/*===--------------------------------------------------------------------------------------------===
 * tq.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "tq.h"
#include <utils/assert.h>


char *tq_get_db_path(const char *root) {
    char *db_path = fs_make_path(root, TQ_DB_NAME, NULL);
    if(fs_file_exists(db_path)) return db_path;
    free(db_path);
    
    char *parent = fs_parent(root);
    char *path = tq_get_db_path(parent);
    free(parent);
    return path;
}

static int task_cmp(const void *a, const void *b) {
    const tq_task_t *ta = a, *tb = b;
    int r = strcmp(ta->id, tb->id);
    if(r < 0) return -1;
    if(r > 0) return 1;
    return 0;
}


void tq_init_new(tq_t *tq, const char *path) {
    ASSERT(tq != NULL);
    ASSERT(path != NULL);
    
    memset(tq, 0, sizeof(*tq));
    
    tq->path = safe_strdup(path);
    list_create(&tq->todo, sizeof(tq_task_t), offsetof(tq_task_t, list_node));
    list_create(&tq->done, sizeof(tq_task_t), offsetof(tq_task_t, list_node));
    avl_create(&tq->tasks, task_cmp, sizeof(tq_task_t), offsetof(tq_task_t, id_node));
}

tq_status_t tq_init(tq_t *tq, const char *path) {
    tq_init_new(tq, path);
    if(!fs_file_exists(path)) return TQ_OK;
    
    return TQ_OK;
}

void tq_fini(tq_t *tq) {
    ASSERT(tq != NULL);
    
    while(list_remove_head(&tq->todo)) {}
    while(list_remove_head(&tq->done)) {}
    
    void *cookie = NULL;
    tq_task_t *task = NULL;
    while((task = avl_destroy_nodes(&tq->tasks, &cookie))) {
        free(task->name);
        free(task->id);
        free(task);
    }
    
    list_destroy(&tq->todo);
    list_destroy(&tq->done);
    avl_destroy(&tq->tasks);
    
    free(tq->path);
}

static void write_task(const tq_task_t *task, FILE *out) {
    fprintf(out, "%s:%s:%s\n", task->done ? "done" : "todo", task->id, task->name);
}

bool tq_write(const tq_t *tq) {
    ASSERT(tq != NULL);
    ASSERT(tq->path != NULL);
    
    FILE *out = fopen(tq->path, "wb");
    if(!out) return false;
    
    for(tq_task_t *t = list_head(&tq->todo); t != NULL; t = list_next(&tq->todo, t)) {
        write_task(t, out);
    }
    
    for(tq_task_t *t = list_head(&tq->done); t != NULL; t = list_next(&tq->done, t)) {
        write_task(t, out);
    }
    
    fclose(out);
    return true;
}

void tq_add(tq_t *tq, const char *name, unsigned at);
void tq_add2(tq_t *tq, const char *name, const char *file);
void tq_delete(tq_t *tq, const char *id);
void tq_done(tq_t *tq, const char *id);

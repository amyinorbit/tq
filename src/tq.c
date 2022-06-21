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
#include <term/printing.h>
#include <term/colors.h>
#include <ctype.h>

#define PARSER_DEBUG

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


#ifdef DEBUG_PARSER
#define FAIL(e) \
    do { \
        fprintf(stderr, "error at line %u: %s\n", linenum, (e)); \
        err = TQ_ERROR_INVALID_DB; \
        goto errout; \
    } while(0)
#else
#define FAIL(e) \
    do { \
        err = TQ_ERROR_INVALID_DB; \
        goto errout; \
    } while(0)
#endif

tq_status_t tq_init(tq_t *tq, const char *path) {
    tq_init_new(tq, path);
    if(!fs_file_exists(path)) return TQ_OK;
    
    FILE *in = fopen(path, "rb");
    if(!in) return TQ_ERROR_IO;
    
    char *line = NULL;
    size_t cap = 0;
    unsigned linenum = 0;
    tq_status_t err = TQ_OK;
    
    while(getline(&line, &cap, in) >= 0) {
        ++linenum;
        str_trim_space(line);
        if(!strlen(line)) continue;
        char *comps[3] = {NULL, NULL, NULL};
        
        if(str_split_inplace(line, ':', comps, 3) != 3) FAIL("not enough components");
        if(strlen(comps[1]) > TQ_ID_LEN || strlen(comps[1]) < 1) FAIL("invalid task ID");
        if(!strlen(comps[2])) FAIL("invalid task description");
        
        bool done = false;
        if(!strcmp(comps[0], "todo")) {
            done = false;
        } else if(!strcmp(comps[0], "done")) {
            done = true;
        } else {
            FAIL("invalid task status");
        }
        
        avl_index_t where;
        tq_task_t search = {.desc = NULL};
        strncpy(search.id, comps[1], sizeof(search.id));
        if(avl_find(&tq->tasks, &search, &where)) FAIL("duplicate task ID");
        
        tq_task_t *task = safe_calloc(1, sizeof(*task));
        strncpy(task->id, comps[1], sizeof(task->id));
        task->desc = safe_strdup(comps[2]);
        task->done = done;
        
        avl_insert(&tq->tasks, task, where);
        if(done) {
            list_insert_tail(&tq->done, task);
        } else {
            list_insert_tail(&tq->todo, task);
        }
    }
    
    if(line) free(line);
    fclose(in);
    return TQ_OK;
errout:
    if(in) fclose(in);
    if(line) free(line);
    return err;
}

void tq_fini(tq_t *tq) {
    ASSERT(tq != NULL);
    
    while(list_remove_head(&tq->todo)) {}
    while(list_remove_head(&tq->done)) {}
    
    void *cookie = NULL;
    tq_task_t *task = NULL;
    while((task = avl_destroy_nodes(&tq->tasks, &cookie))) {
        free(task->desc);
        free(task);
    }
    
    list_destroy(&tq->todo);
    list_destroy(&tq->done);
    avl_destroy(&tq->tasks);
    
    free(tq->path);
}

static void write_task(const tq_task_t *task, FILE *out) {
    fprintf(out, "%s:%s:%s\n", task->done ? "done" : "todo", task->id, task->desc);
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

static avl_index_t unique_id(tq_t *tq, char *id) {
    avl_index_t where;
    tq_task_t search = {0};
    strncpy(search.id, id, sizeof(search.id));
    unsigned disc = 0;
    
    while(avl_find(&tq->tasks, &search, &where)) {
        snprintf(search.id, sizeof(search.id), "%u%s", disc, id);
        ++disc;
        ASSERT(disc < 99);
    }
    
    strncpy(id, search.id, TQ_ID_LEN+1);
    return where;
}

static void create_id(char *id, const char *desc) {
    ASSERT(desc);
    unsigned n = 0;
    bool in_space = true;
    while(*desc && n < TQ_ID_LEN) {
        char c = *(desc++);
        if(isspace(c)) {
            in_space = true;
            continue;
        }
        
        if(in_space) {
            id[n++] = tolower(c);
        }
        in_space = false;
    }
}

static tq_task_t *task_new(tq_t *tq, const char *desc) {
    ASSERT(tq);
    ASSERT(desc);
    ASSERT(strlen(desc) > 0);
    
    char id[TQ_ID_LEN + 1];
    create_id(id, desc);
    avl_index_t where = unique_id(tq, id);
    
    tq_task_t *task = safe_calloc(1, sizeof(*task));
    strncpy(task->id, id, sizeof(task->id));
    task->desc = safe_strdup(desc);
    task->done = false;
    avl_insert(&tq->tasks, task, where);
    return task;
}

tq_task_t *tq_add_front(tq_t *tq, const char *desc) {
    ASSERT(tq);
    ASSERT(desc);
    ASSERT(strlen(desc) > 0);
    
    tq_task_t *task = task_new(tq, desc);
    list_insert_head(&tq->todo, task);
    return task;
}

tq_task_t *tq_add_back(tq_t *tq, const char *desc) {
    ASSERT(tq);
    ASSERT(desc);
    ASSERT(strlen(desc) > 0);
    
    tq_task_t *task = task_new(tq, desc);
    list_insert_tail(&tq->todo, task);
    return task;
}

tq_task_t *tq_add_after(tq_t *tq, const char *desc, const char *node_id) {
    ASSERT(tq);
    ASSERT(desc);
    ASSERT(strlen(desc) > 0);
    tq_task_t *other = avl_find(&tq->tasks, node_id, NULL);
    if(!other || other->done) return NULL;
    
    tq_task_t *task = task_new(tq, desc);
    list_insert_after(&tq->todo, other, task);
    return task;
    
}

tq_task_t *tq_add_before(tq_t *tq, const char *desc, const char *node_id) {
    ASSERT(tq);
    ASSERT(desc);
    ASSERT(strlen(desc) > 0);
    tq_task_t *other = avl_find(&tq->tasks, node_id, NULL);
    if(!other || other->done) return NULL;
    
    tq_task_t *task = task_new(tq, desc);
    list_insert_before(&tq->todo, other, task);
    return task;
}


void tq_print_task(tq_task_t *task, FILE *out) {
    ASSERT(task);
    ASSERT(out);
    
    fprintf(out, " - [");
    term_set_fg(out, TERM_BRIGHT_YELLOW);
    fprintf(out, "%-*s", TQ_ID_LEN, task->id);
    term_style_reset(out);
    fprintf(out, "] %s\n", task->desc);
}

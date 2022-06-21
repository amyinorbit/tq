/*===--------------------------------------------------------------------------------------------===
 * tq.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _TQ_TQ_H_
#define _TQ_TQ_H_

#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include <utils/helpers.h>
#include <utils/avl.h>
#include <utils/list.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TQ_DB_NAME ".tqlist.txt"
#define TQ_ID_LEN (4)

typedef struct tq_task_t {
    char        id[TQ_ID_LEN+1];
    char        *desc;
    bool        done;
    
    avl_node_t  id_node;
    list_node_t list_node;
} tq_task_t;

typedef struct tq_t {
    char        *path;
    
    list_t      todo;
    list_t      done;
    avl_tree_t  tasks;
} tq_t;

typedef enum tq_status_t {
    TQ_OK = 0,
    TQ_ERROR_IO,
    TQ_ERROR_INVALID_DB,
} tq_status_t;


char *tq_get_db_path(const char *current);

void tq_init_new(tq_t *tq, const char *path);
tq_status_t tq_init(tq_t *tq, const char *path);
void tq_fini(tq_t *tq);
bool tq_write(const tq_t *tq);

tq_task_t *tq_add_front(tq_t *tq, const char *desc);
tq_task_t *tq_add_back(tq_t *tq, const char *desc);
tq_task_t *tq_add_after(tq_t *tq, const char *desc, const char *node);
tq_task_t *tq_add_before(tq_t *tq, const char *desc, const char *node);

tq_task_t *tq_mark_done(tq_t *tq, const char *id);

void tq_print_task(tq_task_t *task, FILE *out);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _TQ_TQ_H_ */

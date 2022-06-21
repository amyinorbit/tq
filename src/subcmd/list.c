/*===--------------------------------------------------------------------------------------------===
 * list.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "../cli.h"
#include "../tq.h"
#include <term/colors.h>

static const term_param_t params[] = {
    {'d', 0, "done", TERM_ARG_OPTION, "show tasks already marked as done" },
};
static const int num_params = 1;

static void print_list(const tq_t *tq, bool show_done) {
    term_set_bold(stdout, true);
    printf("Todo:\n");
    term_style_reset(stdout);
    for(tq_task_t *task = list_head(&tq->todo); task; task = list_next(&tq->todo, task)) {
        printf(" - ");
        tq_print_task(task, stdout);
    }
    
    if(!show_done) return;
    
    term_set_bold(stdout, true);
    printf("Done:\n");
    term_style_reset(stdout);
    for(tq_task_t *task = list_head(&tq->done); task; task = list_next(&tq->done, task)) {
        printf(" - ");
        tq_print_task(task, stdout);
    }
}

int subcmd_list(int argc, const char **argv) {
    bool show_done = false;
    
    term_arg_parser_t args;
    term_arg_parser_init(&args, argc, argv);
    
    term_arg_result_t arg = term_arg_parse(&args, params, num_params);
    
    while(arg.name != TERM_ARG_DONE) {
        switch(arg.name) {
        case TERM_ARG_HELP:
            subcmd_use("list", "list [--done]", 
                "list tasks in a queue", params, num_params);
            return 0;
            
        case TERM_ARG_ERROR:
            term_error(tq_prog_name, 1, "%s", args.error);
            return 1;
            
        case 'd':
            show_done = true;
            break;
        }
        arg = term_arg_parse(&args, params, num_params);
    }
    
    tq_t tq;
    get_tq(&tq);
    print_list(&tq, show_done);
    tq_fini(&tq);
    return TQ_OK ? 0 : -1;
}

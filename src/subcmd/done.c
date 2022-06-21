/*===--------------------------------------------------------------------------------------------===
 * done.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "../cli.h"

int subcmd_done(int argc, const char **argv) {
    const char *id = NULL;
    
    term_arg_parser_t args;
    term_arg_parser_init(&args, argc, argv);
    
    term_arg_result_t arg = term_arg_parse(&args, NULL, 0);
    
    while(arg.name != TERM_ARG_DONE) {
        switch(arg.name) {
        case TERM_ARG_HELP:
            subcmd_use("done", "done <task id>", "mark a task as done", NULL, 0);
            return 0;
            
        case TERM_ARG_ERROR:
            term_error(tq_prog_name, 1, "%s", args.error);
            return -1;
            
        case TERM_ARG_POSITIONAL:
            if(!id) {
                id = arg.value;
            } else {
                term_error(tq_prog_name, 0, "too many parameters");
                subcmd_use("done", "done <task id>", "mark a task as done", NULL, 0);
                return -1;
            }
            break;
        }
        arg = term_arg_parse(&args, NULL, 0);
    }
    
    if(!id) {
        term_error(tq_prog_name, 0, "missing task id");
        subcmd_use("done", "done <task id>", "mark a task as done", NULL, 0);
        return -1;
    }
    
    tq_t tq;
    get_tq(&tq);
    tq_task_t *task = tq_mark_done(&tq, id);
    if(task) {
        tq_print_task(task, stdout);
        tq_write(&tq);
    } else {
        term_error(tq_prog_name, 0, "no pending task with ID '%s'", id);
    }
    
    tq_fini(&tq);
    
    return 0;
}

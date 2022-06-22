/*===--------------------------------------------------------------------------------------------===
 * add.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "../cli.h"

static const term_param_t params[] = {
    {'a', 0, "after", TERM_ARG_VALUE, "add the task after an existing one"},
    {'b', 0, "before", TERM_ARG_VALUE, "add the task before an existing one"},
    {0, 'l', "last", TERM_ARG_OPTION, "add the task at the end of the queue"},
};
static const int num_params = 3;

char *my_cat(char *exist, const char *add) {
    if(!exist) return safe_strdup(add);
    
    size_t size = strlen(exist) + strlen(add) + 2;
    char *concat = safe_calloc(size, 1);
    snprintf(concat, size, "%s %s", exist, add);
    free(exist);
    return concat;
}

int subcmd_add(int argc, const char **argv) {
    
    bool last = false;
    const char *after = NULL;
    const char *before = NULL;
    
    term_arg_parser_t args;
    term_arg_parser_init(&args, argc, argv);
    
    term_arg_result_t arg = term_arg_parse(&args, params, num_params);
    
    char *desc = NULL;
    
    while(arg.name != TERM_ARG_DONE) {
        switch(arg.name) {
        case TERM_ARG_HELP:
            subcmd_use("add", "add [--last | --a <id> | --b <id>] <task>", 
                "add a new task to a queue", params, num_params);
            return 0;
        case TERM_ARG_ERROR:
            term_error(tq_prog_name, 1, "%s", args.error);
            return 1;
        case 'l':
            if(after || before) {
                term_error(tq_prog_name, 1, "--last cannot be used with --after or --before");
            }
            last = true;
            break;
            
        case 'a':
            if(last || before) {
                term_error(tq_prog_name, 1, "--after cannot be used with --last or --before");
            }
            after = arg.value;
            break;
        case 'b':
            if(last || after) {
                term_error(tq_prog_name, 1, "--before cannot be used with --last or --after");
            }
            before = arg.value;
            break;
            
        case TERM_ARG_POSITIONAL:
            desc = my_cat(desc, arg.value);
            break;
        }
        arg = term_arg_parse(&args, params, num_params);
    }
    
    if(!desc) {
        term_error(tq_prog_name, 0, "no task description");
        subcmd_use("add", "add [--last | --a <id> | --b <id>] <task>", 
            "add a new task to a queue", params, num_params);
        return -1;
    }
    
    char *path = tq_get_db_path(fs_current_dir());
    if(!path) return -1;
    
    tq_t tq;
    get_tq(&tq);
    
    str_trim_space(desc);
    if(!strlen(desc)) {
        free(desc);
        term_error(tq_prog_name, 0, "empty task description");
        return -1;
    }
    
    tq_task_t *task = NULL;
    if(last) {
        task = tq_add_back(&tq, desc);
    } else if(after) {
        if(!(task = tq_add_after(&tq, desc, after))) {
            term_error(tq_prog_name, 0, "no pending task with ID '%s'", after);
        }
    } else if(before) {
        if(!(task = tq_add_before(&tq, desc, before))) {
            term_error(tq_prog_name, 0, "no pending task with ID '%s'", before);
        }
    } else {
        task = tq_add_front(&tq, desc);
    }
    if(task) tq_print_task(task, stdout);
    
    free(desc);
    tq_write(&tq);
    tq_fini(&tq);
    return 0;
}



/*===--------------------------------------------------------------------------------------------===
 * init.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "../cli.h"
#include "../tq.h"
#include <utils/helpers.h>
#include <term/printing.h>
#include <term/arg.h>

static const term_param_t params[] = {
    {'f', 0, "force", TERM_ARG_OPTION, "override existing task queue" },
    {'q', 0, "quiet", TERM_ARG_OPTION, "execute without printing messages" },
};
static const int num_params = 2;

int subcmd_init(int argc, const char **argv) {
    bool force = false;
    bool quiet = false;
    
    term_arg_parser_t args;
    term_arg_parser_init(&args, argc, argv);
    
    term_arg_result_t arg = term_arg_parse(&args, params, num_params);
    
    while(arg.name != TERM_ARG_DONE) {
        switch(arg.name) {
        case TERM_ARG_HELP:
            subcmd_use("init", "init [--force] [--quiet]", 
                "create a new task queue or reinitialize an existing one", params, num_params);
            return 0;
            
        case TERM_ARG_ERROR:
            term_error(tq_prog_name, -1, "%s", args.error);
            return 1;
            
        case 'f':
            force = true;
            break;
        case 'q':
            quiet = true;
            break;
        }
        arg = term_arg_parse(&args, params, num_params);
    }
    
    char *path = fs_make_path(fs_current_dir(), TQ_DB_NAME, NULL);
    
    bool exists = fs_file_exists(path);
    if(exists && !force) {
        term_error(tq_prog_name, 0,
            "a task queue alread exists at %s.\n"
            "  Use --force to re-initialize it.", path);
        free(path);
        return -1;
    }
    
    tq_t tq;
    tq_init_new(&tq, path);
    tq_write(&tq);
    tq_fini(&tq);
    
    if(!quiet && exists) {
        printf("reinitialised empty task queue in '%s'\n", fs_current_dir());
    } else if(!quiet) {
        printf("initialised empty task queue in '%s'\n", fs_current_dir());
    }
    
    free(path);
    
    return 0;
}

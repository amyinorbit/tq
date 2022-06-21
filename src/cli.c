/*===--------------------------------------------------------------------------------------------===
 * cli.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <term/arg.h>
#include <term/printing.h>
#include <term/colors.h>
#include "cli.h"

typedef struct {
    const char  *cmd;
    const char  *doc;
    int         (*run)(int, const char **);
} subcmd_t;

static const subcmd_t cmds[] = {
    { "init",   "Create a new task queue",      subcmd_init },
    { "add",    "Add new tasks to a queue",     subcmd_add },
    { "list",   "Show tasks in a queue",        subcmd_list },
    { "done",   "Mark tasks as done",           subcmd_done },
    { NULL, NULL, NULL }
};

const char *tq_prog_name = "tq";

static const term_param_t params[] = {
    {0, 0, "version", TERM_ARG_OPTION, "print version number"},
};


static void usage() {
    
    const char *uses[] = {
        "<subcommand> ...",
        "[--help] [--version]"
    };
    term_print_usage(stdout, tq_prog_name, uses, 2);
    puts("");
    
    term_set_bold(stdout, true);
    printf("Subommands\n");
    term_style_reset(stdout);
    for(unsigned i = 0; cmds[i].cmd != NULL; ++i) {
        term_set_bold(stdout, true);
        printf("  %s %-*s ", tq_prog_name, (int)(14 - strlen(tq_prog_name)), cmds[i].cmd);
        term_style_reset(stdout);
        printf("%s\n", cmds[i].doc);
    }
    puts("");
    
    term_print_help(stdout, params, 1);
}

static _Noreturn void version() {
    printf("tq version %s\n", TQ_VERSION);
    exit(EXIT_SUCCESS);
}

static const subcmd_t *find_command(const char *arg) {
    for(unsigned i = 0; cmds[i].cmd != NULL; ++i) {
        if(!strcmp(cmds[i].cmd, arg)) return &cmds[i];
    }
    return NULL;
}

int main(int argc, const char **argv) {
    if(argc < 2) {
        usage();
        exit(EXIT_FAILURE);
    }
    
    const char **cmd_argv = NULL;
    int cmd_argc = 0;
    
    for(int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if(arg[0] == '-') continue;
        cmd_argv = &argv[i];
        cmd_argc = argc - i;
        break;
    }
    
    
    if(cmd_argv != NULL) {
        const subcmd_t *cmd = find_command(cmd_argv[0]);
        if(cmd) return cmd->run(cmd_argc, cmd_argv);
        
        term_error(tq_prog_name, 1, "'%s' is not a tq command", cmd_argv[0]);
        
    } else {
        term_arg_parser_t args;
        term_arg_parser_init(&args, argc, argv);
        
        term_arg_result_t arg = term_arg_parse(&args, params, 1);
        
        while(arg.name != TERM_ARG_DONE) {
            switch(arg.name) {
            case TERM_ARG_HELP:
                usage();
                return 0;
                
            case TERM_ARG_VERSION:
                version();
                return 0;
                
            case TERM_ARG_ERROR:
                term_error(tq_prog_name, 1, "%s", args.error);
                break;
            }
            arg = term_arg_parse(&args, params, 1);
        }
    }
}

void get_tq(tq_t *tq) {
    char *path = tq_get_db_path(fs_current_dir());
    
    if(!path) {
        term_error(tq_prog_name, 1, "unable to access file system");
    }
    
    switch(tq_init(tq, path)) {
    case TQ_OK: break;
    case TQ_ERROR_IO: term_error(tq_prog_name, 1, "unable to open task list at %s", path); break;
    case TQ_ERROR_INVALID_DB: term_error(tq_prog_name, 1, "task list at %s corrupted", path); break;
    }
}


void subcmd_use(
    const char *cmd, const char *use, const char *summary,
    const term_param_t *params, int param_count
) {
    printf(" %s %s – %s\n\n", tq_prog_name, cmd, summary);
    term_print_usage(stdout, tq_prog_name, &use, 1);
    puts("");
    term_print_help(stdout, params, param_count);
}

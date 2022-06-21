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
    (void)argc;
    (void)argv;
    printf("%s done <args>\n", tq_prog_name);
    
    return 0;
}
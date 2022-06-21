/*===--------------------------------------------------------------------------------------------===
 * cli.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _TQ_CLI_H_
#define _TQ_CLI_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define TQ_VERSION "0.1"

extern const char *tq_prog_name;

int subcmd_init(int argc, const char **argv);
int subcmd_list(int argc, const char **argv);
int subcmd_add(int argc, const char **argv);
int subcmd_done(int argc, const char **argv);

#endif /* ifndef _TQ_CLI_H_ */

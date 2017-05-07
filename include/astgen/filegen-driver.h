#pragma once
#include "astgen/ast.h"
#include <stdio.h>

void filegen_init(char *out_dir);
void filegen_add(char *filename, void (*main_func)(struct Config *, FILE *));
void filegen_add_with_userdata(char *filename,
                               void (*main_func)(struct Config *, FILE *,
                                                 void *),
                               void *user_data);
void filegen_all_nodes(char *fileformatter,
                       void (*main_func)(struct Config *, FILE *,
                                         struct Node *));
void filegen_all_nodesets(char *fileformatter,
                          void (*main_func)(struct Config *, FILE *,
                                            struct Nodeset *));
int filegen_generate(struct Config *config);
void filegen_cleanup(void);

#pragma once
#include "astgen/ast.h"
#include <stdio.h>

void filegen_init(char *out_dir);
void filegen_add(char *filename, void (*main_func)(struct Config *, FILE *));
void filegen_all_nodes(char *fileformatter,
                       void (*main_func)(struct Config *, FILE *,
                                         struct Node *));
void filegen_all_nodesets(char *fileformatter,
                          void (*main_func)(struct Config *, FILE *,
                                            struct Nodeset *));

void filegen_all_traversals(char *fileformatter,
                            void (*main_func)(struct Config *, FILE *,
                                              struct Traversal *));
void filegen_all_passes(char *fileformatter,
                        void (*main_func)(struct Config *, FILE *,
                                          struct Pass *));
int filegen_generate(struct Config *config);
void filegen_cleanup(void);

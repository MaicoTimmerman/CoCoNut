#pragma once
#include "astgen/ast.h"
#include <stdio.h>

void filegen_init(char *out_dir);
void filegen_add(char *filename, void (*main_func)(Config *, FILE *));
void filegen_all_nodes(char *fileformatter,
                       void (*main_func)(Config *, FILE *, Node *));
void filegen_all_nodesets(char *fileformatter,
                          void (*main_func)(Config *, FILE *, Nodeset *));

void filegen_all_traversals(char *fileformatter,
                            void (*main_func)(Config *, FILE *, Traversal *));
void filegen_all_passes(char *fileformatter,
                        void (*main_func)(Config *, FILE *, Pass *));
int filegen_generate(Config *config);
void filegen_cleanup(void);

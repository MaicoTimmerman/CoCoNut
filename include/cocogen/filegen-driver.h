#pragma once
#include "cocogen/ast.h"
#include <stdio.h>

void filegen_init(Config *config);
void filegen_dir(char *out_dir);
void filegen_generate(char *filename, void (*func)(Config *, FILE *));
void filegen_all_nodes(char *fileformatter,
                       void (*func)(Config *, FILE *, Node *));
void filegen_all_nodesets(char *fileformatter,
                          void (*func)(Config *, FILE *, Nodeset *));

void filegen_all_traversals(char *fileformatter,
                            void (*func)(Config *, FILE *, Traversal *));
void filegen_all_passes(char *fileformatter,
                        void (*func)(Config *, FILE *, Pass *));

void filegen_cleanup_old_files(void);
void filegen_cleanup(void);

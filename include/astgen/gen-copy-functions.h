#pragma once
#include <stdio.h>

#include "astgen/ast.h"

void generate_copy_node_header(struct Config *c, FILE *fp, struct Node *n);
void generate_copy_node_definitions(struct Config *c, FILE *fp,
                                    struct Node *n);
void generate_copy_nodeset_header(struct Config *c, FILE *fp,
                                  struct Nodeset *n);
void generate_copy_nodeset_definitions(struct Config *c, FILE *fp,
                                       struct Nodeset *n);
void generate_copy_header(struct Config *config, FILE *fp);

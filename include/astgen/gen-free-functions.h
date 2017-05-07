#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "astgen/ast.h"
void generate_free_header(struct Config *, FILE *);
void generate_free_node_header(struct Config *c, FILE *fp, struct Node *n);
void generate_free_node_definitions(struct Config *c, FILE *fp,
                                    struct Node *n);

void generate_free_nodeset_header(struct Config *c, FILE *fp,
                                  struct Nodeset *n);
void generate_free_nodeset_definitions(struct Config *c, FILE *fp,
                                       struct Nodeset *n);

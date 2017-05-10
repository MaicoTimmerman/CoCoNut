#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "astgen/ast.h"
void generate_free_header(Config *, FILE *);
void generate_free_node_header(Config *c, FILE *fp, Node *n);
void generate_free_node_definitions(Config *c, FILE *fp, Node *n);

void generate_free_nodeset_header(Config *c, FILE *fp, Nodeset *n);
void generate_free_nodeset_definitions(Config *c, FILE *fp, Nodeset *n);

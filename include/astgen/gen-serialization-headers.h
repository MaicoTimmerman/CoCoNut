#pragma once
#include "astgen/ast.h"
#include <stdio.h>

void generate_binary_serialization_node_header(Config *config, FILE *fp,
                                               Node *node);
void generate_binary_serialization_nodeset_header(Config *config, FILE *fp,
                                                  Nodeset *nodeset);

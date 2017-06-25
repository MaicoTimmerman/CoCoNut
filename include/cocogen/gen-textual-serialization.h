#pragma once

#include "cocogen/ast.h"
#include <stdio.h>

void generate_textual_serialization_read_node(Config *config, FILE *fp,
                                              Node *node);
void generate_textual_serialization_read_nodeset(Config *config, FILE *fp,
                                                 Nodeset *nodeset);
void generate_textual_serialization_write_node(Config *config, FILE *fp,
                                               Node *node);
void generate_textual_serialization_write_nodeset(Config *config, FILE *fp,
                                                  Nodeset *nodeset);
void generate_textual_serialization_util(Config *config, FILE *fp);
void generate_textual_serialization_util_header(Config *config, FILE *fp);

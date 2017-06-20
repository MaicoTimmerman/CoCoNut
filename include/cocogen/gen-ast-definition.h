#pragma once
#include "cocogen/ast.h"

void generate_enum_definitions(Config *config, FILE *fp);
void generate_ast_definitions(Config *config, FILE *fp);
void generate_ast_node_header(Config *, FILE *, Node *);
void generate_ast_nodeset_header(Config *, FILE *, Nodeset *);

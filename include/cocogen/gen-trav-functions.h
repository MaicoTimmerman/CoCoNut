#pragma once
#include "cocogen/ast.h"
#include <stdio.h>

void generate_trav_header(Config *, FILE *);
void generate_trav_node_header(Config *, FILE *, Node *);
void generate_trav_node_definitions(Config *, FILE *, Node *);

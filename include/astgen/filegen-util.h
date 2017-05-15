#pragma once

#include "astgen/ast.h"
#include "astgen/config.h"
#include <stdio.h>

#define out(...) fprintf(fp, __VA_ARGS__)

void generate_node_header_includes(Config *, FILE *, Node *);

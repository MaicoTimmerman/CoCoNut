#pragma once
#include "astgen/ast.h"

void generate_enum_definitions(struct Config *config, FILE *fp);
void generate_ast_definitions(struct Config *config, FILE *fp);

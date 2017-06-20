#pragma once

#include "cocogen/ast.h"
#include <stdio.h>

void generate_consistency_definitions(Config *c, FILE *fp);

void generate_consistency_header(Config *c, FILE *fp);

#pragma once

#include "ast.h"
#include <stdio.h>

void generate_consistency_definitions(struct Config *c, FILE *fp);

void generate_consistency_header(struct Config *c, FILE *fp);

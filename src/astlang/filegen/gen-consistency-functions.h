#pragma once

#include <stdio.h>
#include "ast.h"

void generate_consistency_definition(struct Config *c, FILE *fp);

void generate_consistency_header(struct Config *c, FILE *fp);

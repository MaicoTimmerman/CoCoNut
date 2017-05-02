#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "astgen/ast.h"
void generate_free_header(struct Config *, FILE *);
void generate_free_definitions(struct Config *, FILE *);

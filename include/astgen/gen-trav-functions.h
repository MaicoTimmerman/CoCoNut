#pragma once
#include "astgen/ast.h"
#include <stdio.h>

void generate_trav_definitions(struct Config *, FILE *);

void generate_trav_header(struct Config *, FILE *);

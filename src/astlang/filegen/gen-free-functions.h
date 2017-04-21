#pragma once
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
void generate_free_header(struct Config *, FILE *);
void generate_free_definitions(struct Config *, FILE *);

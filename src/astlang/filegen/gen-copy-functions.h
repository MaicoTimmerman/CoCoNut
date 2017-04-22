#pragma once
#include <stdio.h>

#include "ast.h"

void generate_copy_definitions(struct Config *, FILE *);

void generate_copy_header(struct Config *, FILE *);

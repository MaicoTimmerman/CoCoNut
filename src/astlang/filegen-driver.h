#pragma once
#include "ast.h"
#include <stdio.h>

void filegen_init(char *out_dir);
void filegen_add(char *filename,
                 void (*main_func)(struct Config *config, FILE *fp));
int filegen_generate(struct Config *config);
void filegen_cleanup(void);

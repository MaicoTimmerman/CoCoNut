#pragma once
#include "ast.h"
#include <stdio.h>

#define out(...) fprintf(info->fp, __VA_ARGS__)

void filegen_init(char *out_dir);
void filegen_add(char *filename,
                 void (*main_func)(struct Config *config, FILE *fp));
int filegen_generate(struct Config *config);
void filegen_cleanup(void);

#pragma once
#include "astgen/ast.h"
#include <stdio.h>

void filegen_init(char *out_dir);
void filegen_add(char *filename, void (*main_func)(struct Config *, FILE *));
void filegen_add_with_userdata(char *filename,
                               void (*main_func)(struct Config *, FILE *,
                                                 void *),
                               void *user_data);
int filegen_generate(struct Config *config);
void filegen_cleanup(void);

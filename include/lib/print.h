#pragma once

#include <stdio.h>
#include <unistd.h>

#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"

#define DIM "\x1B[2m"
#define BOLD "\x1B[1m"
#define RESET_COLOR "\x1B[0m"

#define PRINT_COLOR(c)                                                        \
    {                                                                         \
        if (isatty(STDERR_FILENO))                                            \
            fprintf(stderr, c);                                               \
    }

enum PrintType { PT_error, PT_warning, PT_note };

void print_user_error(const char *header, const char *format, ...);

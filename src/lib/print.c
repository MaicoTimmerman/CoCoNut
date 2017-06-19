#include <stdarg.h>
#include <stdio.h>

#include "lib/print.h"

void print_user_error(const char *header, const char *format, ...) {
    PRINT_COLOR(BOLD MAGENTA);

    fprintf(stderr, "[%s] ", header);

    PRINT_COLOR(BOLD RED);

    fprintf(stderr, "error: ");

    PRINT_COLOR(RESET_COLOR);

    PRINT_COLOR(RESET_COLOR);

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
}

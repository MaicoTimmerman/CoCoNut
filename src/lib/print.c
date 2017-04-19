#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "array.h"
#include "ast.h"

extern array *yy_lines;

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

static void do_print(enum PrintType type, int lineno, int column_start,
                     int column_end, char *line, char *format, va_list ap) {

    char *fn = "(stdin)";

    PRINT_COLOR(BOLD);

    fprintf(stderr, "%s:%d:%d: ", fn, lineno, column_start);

    PRINT_COLOR(RESET_COLOR);

    switch (type) {
    case PT_error:
        PRINT_COLOR(RED BOLD);
        fprintf(stderr, "error: ");
        break;
    case PT_warning:
        PRINT_COLOR(MAGENTA BOLD);
        fprintf(stderr, "warning: ");
        break;
    case PT_note:
        PRINT_COLOR(CYAN BOLD);
        fprintf(stderr, "note: ");
        break;
    }

    PRINT_COLOR(RESET_COLOR);

    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n%s\n", line);

    int c = 1;
    while (c < column_start) {
        fputc(' ', stderr);
        c++;
    }

    PRINT_COLOR(RED BOLD);

    fputc('^', stderr);
    c++;

    while (c < column_end) {
        fputc('~', stderr);
        c++;
    }
    PRINT_COLOR(RESET_COLOR);

    fputc('\n', stderr);
}

void print_error_at(int lineno, int column, char *line, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    do_print(PT_error, lineno, column, column, line, format, ap);
}

void print_error(struct NodeCommonInfo *c, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    char *line = array_get(yy_lines, c->line_start - 1);
    do_print(PT_error, c->line_start, c->column_start, c->column_end, line,
             format, ap);
}

void print_warning_at(int lineno, int column, char *line, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    do_print(PT_warning, lineno, column, column, line, format, ap);
}

void print_warning(struct NodeCommonInfo *c, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    char *line = array_get(yy_lines, c->line_start - 1);
    do_print(PT_warning, c->line_start, c->column_start, c->column_end, line,
             format, ap);
}

void print_note_at(int lineno, int column, char *line, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    do_print(PT_note, lineno, column, column, line, format, ap);
}

void print_note(struct NodeCommonInfo *c, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    char *line = array_get(yy_lines, c->line_start - 1);
    do_print(PT_note, c->line_start, c->column_start, c->column_end, line,
             format, ap);
}

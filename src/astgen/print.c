#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "astgen/ast.h"
#include "lib/array.h"
#include "lib/imap.h"

extern char *yy_filename;
extern array *yy_lines;
extern imap_t *yy_parser_locations;

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

    PRINT_COLOR(BOLD);

    fprintf(stderr, "%s:%d:%d: ", yy_filename, lineno, column_start);

    PRINT_COLOR(RESET_COLOR);

    char *color = "";

    switch (type) {
    case PT_error:
        color = RED BOLD;
        PRINT_COLOR(color);
        fprintf(stderr, "error: ");
        break;
    case PT_warning:
        color = MAGENTA BOLD;
        PRINT_COLOR(color);
        fprintf(stderr, "warning: ");
        break;
    case PT_note:
        color = CYAN BOLD;
        PRINT_COLOR(color);
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

    PRINT_COLOR(color);

    fputc('^', stderr);

    c++;

    while (c <= column_end) {
        fputc('~', stderr);
        c++;
    }
    PRINT_COLOR(RESET_COLOR);

    fputc('\n', stderr);
}

static void do_print_at_loc(enum PrintType type, void *loc_obj, char *format,
                            va_list ap) {
    ParserLocation *loc = imap_retrieve(yy_parser_locations, loc_obj);
    assert(loc != NULL);
    assert(loc->first_line > 0);

    char *line = array_get(yy_lines, loc->first_line - 1);

    do_print(type, loc->first_line, loc->first_column,
             loc->last_line == loc->first_line ? loc->last_column
                                               : loc->first_column,
             line, format, ap);
}

void print_error_at(int lineno, int column, char *line, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    do_print(PT_error, lineno, column, column, line, format, ap);
}

void print_error(void *loc_obj, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    do_print_at_loc(PT_error, loc_obj, format, ap);
}

void print_warning_at(int lineno, int column, char *line, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    do_print(PT_warning, lineno, column, column, line, format, ap);
}

void print_warning(void *loc_obj, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    do_print_at_loc(PT_warning, loc_obj, format, ap);
}

void print_note_at(int lineno, int column, char *line, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    do_print(PT_note, lineno, column, column, line, format, ap);
}

void print_note(void *loc_obj, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    do_print_at_loc(PT_note, loc_obj, format, ap);
}

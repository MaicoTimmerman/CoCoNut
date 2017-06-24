#pragma once

#include <stdio.h>
#include <unistd.h>

#include "lib/array.h"
#include "lib/imap.h"

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

// We define the type for YYLTYPE here,
// to avoid a circular dependency between this header and the parser
typedef struct ParserLocation {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
} ParserLocation;

enum PrintType { PT_error, PT_warning, PT_note };

void print_user_error(char *header, const char *format, ...);

void print_init_compilation_messages(char *header, char *filename,
                                     array *lines, imap_t *parser_locations);

void print_error(void *loc_obj, char *format, ...);
void print_error_at(int lineno, int column, char *line, char *format, ...);
void print_error_range(int lineno, int column_start, int column_end,
                       char *line, char *format, ...);
void print_error_no_loc(char *format, ...);
void print_warning(void *loc_obj, char *format, ...);
void print_warning_at(int lineno, int column, char *line, char *format, ...);
void print_warning_range(int lineno, int column_start, int column_end,
                         char *line, char *format, ...);
void print_warning_no_loc(char *format, ...);
void print_note(void *loc_obj, char *format, ...);
void print_note_at(int lineno, int column, char *line, char *format, ...);
void print_note_range(int lineno, int column_start, int column_end, char *line,
                      char *format, ...);
void print_note_no_loc(char *format, ...);

void _print_internal_error(const char *file, const char *func, int line,
                           const char *format, ...);

#define print_internal_error(...)                                             \
    _print_internal_error(__FILE__, __func__, __LINE__, __VA_ARGS__)

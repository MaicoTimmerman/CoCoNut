#pragma once

#include "astgen/ast.h"
#include "lib/print.h"

void print_error(void *loc_obj, char *format, ...);
void print_error_at(int lineno, int column, char *line, char *format, ...);
void print_error_range(int lineno, int column_start, int column_end,
                       char *line, char *format, ...);
void print_warning(void *loc_obj, char *format, ...);
void print_warning_at(int lineno, int column, char *line, char *format, ...);
void print_warning_range(int lineno, int column_start, int column_end,
                         char *line, char *format, ...);
void print_note(void *loc_obj, char *format, ...);
void print_note_at(int lineno, int column, char *line, char *format, ...);
void print_note_range(int lineno, int column_start, int column_end, char *line,
                      char *format, ...);

void _print_internal_error(const char *file, const char *func, int line,
                           const char *format, ...);

#define print_internal_error(...)                                             \
    _print_internal_error(__FILE__, __func__, __LINE__, __VA_ARGS__)

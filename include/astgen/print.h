#pragma once

#include "astgen/ast.h"

void print_error(void *loc_obj, char *format, ...);
void print_error_at(int lineno, int column, char *line, char *format, ...);
void print_warning(void *loc_obj, char *format, ...);
void print_warning_at(int lineno, int column, char *line, char *format, ...);
void print_note(void *loc_obj, char *format, ...);
void print_note_at(int lineno, int column, char *line, char *format, ...);

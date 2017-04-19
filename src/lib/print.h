#ifndef _PRINT_H
#define _PRINT_H

#include "ast.h"

void print_error(struct NodeCommonInfo *c, char *format, ...);
void print_error_at(int lineno, int column, char *line, char *format, ...);
void print_warning(struct NodeCommonInfo *c, char *format, ...);
void print_warning_at(int lineno, int column, char *line, char *format, ...);
void print_note(struct NodeCommonInfo *c, char *format, ...);
void print_note_at(int lineno, int column, char *line, char *format, ...);

#endif

#pragma once

#include "framework/serialization-binary-format.h"
#include "framework/serialization-txt-ast.h"
#include <stdio.h>

extern char *_serialization_read_fn;

AstBinFile *serialization_read_binfile(FILE *fp);

int _serialization_txt_check_file(AST_TXT_File *file);

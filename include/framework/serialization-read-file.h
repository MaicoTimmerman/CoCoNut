#pragma once

#include "framework/serialization-binary-format.h"
#include <stdio.h>

extern char *_serialization_read_fn;

AstBinFile *serialization_read_binfile(FILE *fp);

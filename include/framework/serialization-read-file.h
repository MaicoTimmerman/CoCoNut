#pragma once

#include "framework/serialization-binary-format.h"
#include <stdio.h>

AstBinFile *serialization_read_binfile(FILE *fp);

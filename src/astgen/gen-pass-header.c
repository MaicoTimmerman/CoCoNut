#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include <stdio.h>

void generate_pass_header(Config *config, FILE *fp, Pass *pass) {

    out("#pragma once\n\n");
    out("#include \"generated/ast.h\"\n");

    out("%s *" PASS_ENTRY_FORMAT "(%s *syntaxtree);\n", config->root_node->id,
        pass->id, config->root_node->id);
}

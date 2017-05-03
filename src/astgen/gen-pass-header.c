#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include <stdio.h>

void generate_pass_header(struct Config *config, FILE *fp, void *userdata) {
    struct Pass *pass = userdata;

    out("#pragma once\n\n");
    out("#include \"generated/ast.h\"\n");

    char *root_node_name = config->root_node != NULL
                               ? config->root_node->id
                               : config->root_nodeset->id;
    out("%s *" PASS_ENTRY_FORMAT "(%s *syntaxtree);\n", root_node_name,
        pass->id, root_node_name);
}

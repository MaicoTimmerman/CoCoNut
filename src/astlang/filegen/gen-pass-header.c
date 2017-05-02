#include "ast.h"
#include "filegen-driver.h"
#include "filegen-util.h"
#include <stdio.h>

void generate_pass_header(struct Config *config, FILE *fp, void *userdata) {
    struct Pass *pass = userdata;

    out("#pragma once\n\n");
    out("#include \"ast.h\"\n");

    char *root_node_name = config->root_node != NULL
                               ? config->root_node->id
                               : config->root_nodeset->id;
    out("%s *pass_%s_entry(%s *syntaxtree);\n", root_node_name, pass->id,
        root_node_name);
}

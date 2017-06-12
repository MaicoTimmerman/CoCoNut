#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include <stdio.h>

static void generate(FILE *fp, char *name) {
    out("#pragma once\n");
    out("#include <stdio.h>\n");
    out("#include \"framework/serialization-binary-format.h\"\n\n");
    out("%s *" SERIALIZE_READ_BIN_FORMAT "(FILE *fp);\n", name, name);
    out("%s *" SERIALIZE_READ_TXT_FORMAT "(FILE *fp);\n", name, name);
    out("\n");
    out("void " SERIALIZE_WRITE_BIN_FORMAT "(%s *syntaxtree, FILE *fp);\n",
        name, name);
    out("void " SERIALIZE_WRITE_TXT_FORMAT "(%s *syntaxtree, FILE *fp);\n",
        name, name);
}

void generate_binary_serialization_node_header(Config *config, FILE *fp,
                                               Node *node) {
    generate(fp, node->id);
}

void generate_binary_serialization_nodeset_header(Config *config, FILE *fp,
                                                  Nodeset *nodeset) {
    generate(fp, nodeset->id);
}

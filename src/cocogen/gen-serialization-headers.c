#include "cocogen/ast.h"
#include "cocogen/filegen-driver.h"
#include "cocogen/filegen-util.h"
#include <stdio.h>

static void generate(FILE *fp, char *name) {
    out("#pragma once\n");
    out("#include <stdio.h>\n");
    out("#include \"framework/serialization-binary-format.h\"\n\n");
    out("%s *" SERIALIZE_READ_BIN_FORMAT "(char *fn);\n", name, name);
    out("%s *" SERIALIZE_READ_TXT_FORMAT "(char *fn);\n", name, name);
    out("\n");
    out("void " SERIALIZE_WRITE_BIN_FORMAT "(%s *syntaxtree, char *fn);\n",
        name, name);
    out("void " SERIALIZE_WRITE_TXT_FORMAT "(%s *syntaxtree, char *fn);\n",
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

void generate_binary_serialization_all_header(Config *config, FILE *fp) {
    out("#pragma once\n");
    for (int i = 0; i < array_size(config->nodes); i++) {
        Node *n = array_get(config->nodes, i);
        out("#include \"generated/serialization-%s.h\"\n", n->id);
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        Nodeset *n = array_get(config->nodesets, i);
        out("#include \"generated/serialization-%s.h\"\n", n->id);
    }
}

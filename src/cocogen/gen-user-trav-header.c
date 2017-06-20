#include "cocogen/ast.h"
#include "cocogen/filegen-driver.h"
#include "cocogen/filegen-util.h"
#include "cocogen/str-ast.h"
#include "lib/memory.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

void generate_user_trav_header(Config *config, FILE *fp, Traversal *trav) {

    out("#pragma once\n\n");
    out("#include \"generated/ast.h\"\n");

    if (trav->nodes == NULL) {
        out("#include \"generated/trav-ast.h\"\n");
    } else {
        for (int i = 0; i < array_size(trav->nodes); ++i) {
            char *node = array_get(trav->nodes, i);
            out("#include \"generated/trav-%s.h\"\n", node);
        }
    }

    out("typedef struct Info Info;\n");
    out("Info *%s_createinfo(void);\n", trav->id);
    out("void %s_freeinfo(Info *);\n", trav->id);

    if (trav->nodes != NULL) {
        for (int i = 0; i < array_size(trav->nodes); i++) {
            char *node = array_get(trav->nodes, i);
            out("void " TRAVERSAL_HANDLER_FORMAT "(%s *node, Info *info);\n",
                trav->id, node, node);
        }

    } else {
        for (int i = 0; i < array_size(config->nodes); i++) {
            Node *n = array_get(config->nodes, i);
            out("void " TRAVERSAL_HANDLER_FORMAT "(%s *node, Info *info);\n",
                trav->id, n->id, n->id);
        }
    }
}

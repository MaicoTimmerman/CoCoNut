#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include "astgen/str-ast.h"
#include "lib/memory.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

void generate_user_trav_header(struct Config *config, FILE *fp,
                               struct Traversal *trav) {

    out("#pragma once\n\n");
    out("#include \"generated/ast.h\"\n");
    out("struct Info;\n");
    out("struct Info *%s_createinfo(void);\n", trav->id);
    out("void %s_freeinfo(struct Info *);\n", trav->id);

    if (trav->nodes != NULL) {
        for (int i = 0; i < array_size(trav->nodes); i++) {
            char *node = array_get(trav->nodes, i);
            out("void " TRAVERSAL_HANDLER_FORMAT
                "(%s *node, struct Info *info);\n",
                trav->id, node, node);
        }

    } else {
        for (int i = 0; i < array_size(config->nodes); i++) {
            struct Node *n = array_get(config->nodes, i);
            out("void " TRAVERSAL_HANDLER_FORMAT
                "(%s *node, struct Info *info);\n",
                trav->id, n->id, n->id);
        }
    }
}

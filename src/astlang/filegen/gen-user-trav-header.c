#include "ast.h"
#include "filegen-driver.h"
#include "filegen-util.h"
#include "memory.h"
#include "str-ast.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define TRAVERSAL_FMT "void %s_%s(%s *node, struct Info *info)"

void generate_user_trav_header(struct Config *config, FILE *fp,
                               void *userdata) {
    struct Traversal *trav = userdata;

    out("#pragma once\n\n");
    out("#include \"ast.h\"\n");
    out("struct Info;\n");
    out("struct Info *%s_createinfo(void);\n", trav->id);
    out("void %s_freeinfo(struct Info *);\n", trav->id);

    if (trav->nodes != NULL) {
        for (int i = 0; i < array_size(trav->nodes); i++) {
            char *node = array_get(trav->nodes, i);
            out(TRAVERSAL_FMT ";\n", trav->id, node, node);
        }

    } else {
        for (int i = 0; i < array_size(config->nodes); i++) {
            struct Node *n = array_get(config->nodes, i);
            out(TRAVERSAL_FMT ";\n", trav->id, n->id, n->id);
        }
    }
}

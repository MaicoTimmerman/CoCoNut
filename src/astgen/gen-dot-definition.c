#include <stdio.h>

#include "astgen/ast.h"
#include "astgen/filegen-util.h"

#include "lib/array.h"

static void generate_nodes(struct Node *node, FILE *fp) {
    out("    \"%s\" [label=\"%s", node->id, node->id);
    for (int i = 0; i < array_size(node->attrs); i++) {
        struct Attr *a = (struct Attr *)array_get(node->attrs, i);
        out("|%s", a->id);
    }
    out("\" shape=\"record\"];\n");
}

static void generate_children(struct Node *node, FILE *fp) {
    for (int i = 0; i < array_size(node->children); i++) {
        struct Child *child = (struct Child *)array_get(node->children, i);
        out("    %s -> %s [label=\"%s\"];\n", node->id, child->type,
            child->id);
    }
}

static void generate_nodesets(struct Nodeset *nodeset, FILE *fp) {
    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        struct Node *node = (struct Node *)array_get(nodeset->nodes, i);
        out("%s -> %s;\n", nodeset->id, node->id);
    }
}

void generate_dot_definition(struct Config *config, FILE *fp) {
    out("digraph ast {\n");

    for (int i = 0; i < array_size(config->nodes); i++) {
        generate_nodes(array_get(config->nodes, i), fp);
    }

    out("\n");
    out("    edge [color=blue]\n");
    for (int i = 0; i < array_size(config->nodesets); i++) {
        generate_nodesets(array_get(config->nodesets, i), fp);
    }

    out("\n");
    out("    edge [color=black]\n");
    for (int i = 0; i < array_size(config->nodes); i++) {
        generate_children(array_get(config->nodes, i), fp);
    }

    out("}");
}

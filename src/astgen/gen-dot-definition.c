#include <stdio.h>

#include "astgen/ast.h"
#include "astgen/filegen-util.h"

#include "lib/array.h"

static void generate_node(Node *node, FILE *fp) {
    out("    \"%s\" [label=\"%s", node->id, node->id);
    for (int i = 0; i < array_size(node->attrs); i++) {
        Attr *a = (Attr *)array_get(node->attrs, i);
        out("|%s", a->id);
    }

    out("\"");
    if (node->root) {
        out(" color=\"darkorchid3\"");
    }
    out(" shape=\"record\"];\n");
}

static void generate_children(Node *node, FILE *fp) {
    for (int i = 0; i < array_size(node->children); i++) {
        Child *child = (Child *)array_get(node->children, i);
        out("    %s -> %s [label=\"%s\"];\n", node->id, child->type,
            child->id);
    }
}

static void generate_nodeset(Nodeset *nodeset, FILE *fp) {
    out("    \"%s\";\n", nodeset->id);
}

static void generate_nodeset_edges(Nodeset *nodeset, FILE *fp) {
    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        Node *node = (Node *)array_get(nodeset->nodes, i);
        out("    %s -> %s;\n", nodeset->id, node->id);
    }
}

void generate_dot_definition(Config *config, FILE *fp) {
    out("digraph ast {\n");

    for (int i = 0; i < array_size(config->nodes); i++) {
        generate_node(array_get(config->nodes, i), fp);
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        generate_nodeset(array_get(config->nodesets, i), fp);
    }

    out("\n");
    out("    edge [color=blue]\n");
    for (int i = 0; i < array_size(config->nodesets); i++) {
        generate_nodeset_edges(array_get(config->nodesets, i), fp);
    }

    out("\n");
    out("    edge [color=black]\n");
    for (int i = 0; i < array_size(config->nodes); i++) {
        generate_children(array_get(config->nodes, i), fp);
    }

    out("}");
}

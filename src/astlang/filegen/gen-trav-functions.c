#include "ast.h"
#include "filegen-driver.h"
#include "filegen-util.h"
#include "memory.h"
#include "str-ast.h"
#include <stdio.h>

// TODO Merge functions for header and definition.
static void generate_trav_node_header(struct Node *node, FILE *fp) {
    out("struct %s *trav_%s(struct %s *, struct Info *);\n", node->id,
        node->id, node->id);
    for (int i = 0; i < array_size(node->children); ++i) {
        struct Child *child = (struct Child *)array_get(node->children, i);
        out("void trav_%s_%s(struct %s *, struct Info *);\n", node->id,
            child->id, node->id);
    }
}

static void generate_trav_nodeset_header(struct Nodeset *nodeset, FILE *fp) {
    out("void trav_%s(struct %s *, struct Info *);\n", nodeset->id,
        nodeset->id);
}

static void generate_trav_node_definitions(struct Node *node, FILE *fp) {
    out("struct %s *trav_%s(struct %s *, struct Info *info) {\n", node->id,
        node->id, node->id);
    out("    return SOME_USER_%s_FUN(struct %s *, struct Info *info);\n",
        node->id, node->id);
    out("}\n");

    for (int i = 0; i < array_size(node->children); ++i) {
        struct Child *child = (struct Child *)array_get(node->children, i);
        out("void trav_%s_%s(struct %s *node, struct Info *info) {\n",
            node->id, child->id, node->id);

        out("    if (!node) return;\n");
        out("    node->%s = trav_%s(node->%s, info);\n", child->id,
            child->type, child->id);
        out("}\n");
    }
}

static void generate_trav_nodeset_definitions(struct Nodeset *nodeset,
                                              FILE *fp) {
    out("struct %s *trav_%s(struct %s *nodeset, struct Info *info) {\n",
        nodeset->id, nodeset->id, nodeset->id);
    out("    switch (nodeset->type) {\n");
    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        char *node = (char *)array_get(nodeset->nodes, i);
        out("    case NS_%s_%s:\n", nodeset->id, node);
        out("        trav_%s(nodeset->value.val_%s, info);\n", node, node);
        out("        break;\n");
    }
    out("    }\n");
    out("}\n");
}

void generate_trav_definitions(struct Config *config, FILE *fp) {

    out("#include \"ast.h\"\n");
    out("#include \"trav-ast.h\"\n");
    out("\n");
    out("// NODES\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        generate_trav_node_definitions(
            (struct Node *)array_get(config->nodes, i), fp);
    }
    out("// NODESETS\n");
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        generate_trav_nodeset_definitions(
            (struct Nodeset *)array_get(config->nodesets, i), fp);
    }
}

void generate_trav_header(struct Config *config, FILE *fp) {

    out("#pragma once\n");
    out("#include \"ast.h\"\n");
    out("\n");
    out("// NODES\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        generate_trav_node_header((struct Node *)array_get(config->nodes, i),
                                  fp);
    }

    out("// NODESETS\n");
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        generate_trav_nodeset_header(
            (struct Nodeset *)array_get(config->nodesets, i), fp);
    }
}

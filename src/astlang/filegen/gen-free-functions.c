#include "ast.h"
#include "filegen-driver.h"
#include "memory.h"
#include "str-ast.h"
#include <stdio.h>

struct Info {
    FILE *fp;
};

static struct Info *create_info(void) {
    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));
    info->fp = NULL;
    return info;
}

static void free_info(struct Info *info) {
    mem_free(info);
}

static void generate_free_node_header(struct Node *node, struct Info *info) {
    out("void free_%s_tree(struct %s*);", node->id, node->id);
    out(" // free children.\n");

    out("void free_%s_node(struct %s*);", node->id, node->id);
    out(" // skip children.\n");
}

static void generate_free_nodeset_header(struct Nodeset *nodeset,
                                         struct Info *info) {
    out("void free_%s_tree(struct %s*);", nodeset->id, nodeset->id);
    out(" // free children.\n");

    out("void free_%s_node(struct %s*);", nodeset->id, nodeset->id);
    out(" // skip children.\n");
}

static void generate_free_nodeset_definition(struct Nodeset *nodeset,
                                             struct Info *info) {

    out("void free_%s_tree(struct %s *nodeset) {", nodeset->id, nodeset->id);
    out(" // free children. \n");

    out("    switch(nodeset->type) {\n");
    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        char *node = (char *)array_get(nodeset->nodes, i);
        out("    case NS_%s_%s:\n", nodeset->id, node);
        out("        free_%s_tree(nodeset->value.val_%s);\n", node, node);
        out("        break;\n");
    }
    out("    }\n");
    out("    mem_free(nodeset);\n");
    out("}\n");

    out("void free_%s_node(struct %s* nodeset) {", nodeset->id, nodeset->id);
    out(" // skip children. \n");
    out("    mem_free(nodeset);\n");
    out("}\n");
}

static void generate_free_node_definition(struct Node *node,
                                          struct Info *info) {
    out("void free_%s_tree(struct %s* node) {\n", node->id, node->id);

    for (int i = 0; i < array_size(node->children); ++i) {
        struct Child *child = (struct Child *)array_get(node->children, i);
        out("    free_%s_tree(node->%s);\n", child->type, child->id);
    }

    // Only need to free strings, as all other attributes are literals or
    // pointers to node's which are not owned by this node.
    for (int i = 0; i < array_size(node->attrs); ++i) {
        struct Attr *attr = (struct Attr *)array_get(node->attrs, i);
        if (attr->type == AT_string) {
            out("    mem_free(node->%s);\n", attr->id);
        }
    }

    /* out("    "); */
    out("    mem_free(node);\n");
    out("}\n");

    out("void free_%s_node(struct %s* node) {", node->id, node->id);
    out(" // skip children.\n");

    // Only need to free strings, as all other attributes are literals or
    // pointers to node's which are not owned by this node.
    for (int i = 0; i < array_size(node->attrs); ++i) {
        struct Attr *attr = (struct Attr *)array_get(node->attrs, i);
        if (attr->type == AT_string) {
            out("    mem_free(node->%s);\n", attr->id);
        }
    }
    out("    mem_free(node);\n");
    out("}\n");
}

void generate_free_definitions(struct Config *config, FILE *fp) {
    struct Info *info = create_info();
    info->fp = fp;

    out("#include \"ast.h\"\n");
    out("#include \"free-ast.h\"\n");
    out("\n");
    out("// NODES\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        generate_free_node_definition(
            (struct Node *)array_get(config->nodes, i), info);
    }
    out("// NODESETS\n");
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        generate_free_nodeset_definition(
            (struct Nodeset *)array_get(config->nodesets, i), info);
    }

    free_info(info);
}
void generate_free_header(struct Config *config, FILE *fp) {
    struct Info *info = create_info();
    info->fp = fp;

    out("#pragma once\n");
    out("#include \"ast.h\"\n");
    out("\n");
    out("// NODES\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        generate_free_node_header((struct Node *)array_get(config->nodes, i),
                                  info);
    }
    out("// NODESETS\n");
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        generate_free_nodeset_header(
            (struct Nodeset *)array_get(config->nodesets, i), info);
    }

    free_info(info);
}

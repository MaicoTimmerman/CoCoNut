#include "ast.h"
#include "filegen-driver.h"
#include "filegen-util.h"
#include "memory.h"
#include "str-ast.h"
#include <stdio.h>

static void generate_free_nodeset(struct Nodeset *nodeset, FILE *fp,
                                  bool header) {

    out("void free_%s_tree(struct %s *nodeset)", nodeset->id, nodeset->id);

    if (header) {
        out(" {\n");
    } else {
        out(";\n");
        return;
    }
    out(" // free children. \n");

    out("    switch(nodeset->type) {\n");
    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        struct Node *node = (struct Node *)array_get(nodeset->nodes, i);
        out("    case " NS_FMT ":\n", nodeset->id, node->id);
        out("        free_%s_tree(nodeset->value.val_%s);\n", node->id,
            node->id);
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

static void generate_free_node(struct Node *node, FILE *fp, bool header) {
    out("void free_%s_tree(struct %s* node)", node->id, node->id);

    if (header) {
        out(" {\n");
    } else {
        out(";\n");
        return;
    }

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

static void generate(struct Config *config, FILE *fp, bool header) {
    out("#include \"ast.h\"\n");

    if (header) {
        out("#include \"memory.h\"\n");
        out("#include \"free-ast.h\"\n");
    } else {
        out("#pragma once\n");
    }

    out("// NODES\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        generate_free_node(array_get(config->nodes, i), fp, header);
    }
    out("// NODESETS\n");
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        generate_free_nodeset(array_get(config->nodesets, i), fp, header);
    }
}

// TODO generate header for free node.
void generate_free_definitions(struct Config *config, FILE *fp) {
    generate(config, fp, true);
}

void generate_free_header(struct Config *config, FILE *fp) {
    generate(config, fp, false);
}

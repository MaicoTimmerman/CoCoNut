#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include "astgen/str-ast.h"
#include "lib/memory.h"
#include <stdio.h>

static void generate_free_nodeset(struct Nodeset *nodeset, FILE *fp,
                                  bool header) {

    out("void " FREE_TREE_FORMAT "(struct %s *nodeset)", nodeset->id,
        nodeset->id);

    if (header) {
        out(";");
    } else {
        out(" {\n");

        out("    switch(nodeset->type) {\n");
        for (int i = 0; i < array_size(nodeset->nodes); ++i) {
            struct Node *node = (struct Node *)array_get(nodeset->nodes, i);
            out("    case " NS_FORMAT ":\n", nodeset->id, node->id);
            out("        " FREE_TREE_FORMAT "(nodeset->value.val_%s);\n",
                node->id, node->id);
            out("        break;\n");
        }
        out("    }\n");
        out("    mem_free(nodeset);\n");
        out("}\n");
    }

    out("void " FREE_NODE_FORMAT "(struct %s* nodeset)", nodeset->id,
        nodeset->id);
    if (header) {
        out(";");
    } else {
        out(" {\n");

        out("    switch(nodeset->type) {\n");
        for (int i = 0; i < array_size(nodeset->nodes); ++i) {
            struct Node *node = (struct Node *)array_get(nodeset->nodes, i);
            out("    case " NS_FORMAT ":\n", nodeset->id, node->id);
            out("        " FREE_NODE_FORMAT "(nodeset->value.val_%s);\n",
                node->id, node->id);
            out("        break;\n");
        }
        out("    }\n");
        out("    mem_free(nodeset);\n");
        out("}\n");
    }
}

static void generate_free_node(struct Node *node, FILE *fp, bool header) {
    out("void " FREE_TREE_FORMAT "(struct %s* node)", node->id, node->id);

    if (header) {
        out(";");
    } else {
        out(" {\n");

        for (int i = 0; i < array_size(node->children); ++i) {
            struct Child *child = (struct Child *)array_get(node->children, i);
            out("    " FREE_TREE_FORMAT "(node->%s);\n", child->type,
                child->id);
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
    }

    out("void " FREE_NODE_FORMAT "(struct %s* node)", node->id, node->id);
    if (header) {
        out(";");
    } else {
        out(" {\n");
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
}

void generate_free_node_header(struct Config *c, FILE *fp, struct Node *n) {
    out("#pragma once\n");
    out("#include \"generated/ast.h\"\n\n");
    generate_free_node(n, fp, true);
}

void generate_free_node_definitions(struct Config *c, FILE *fp,
                                    struct Node *n) {
    out("#include \"lib/memory.h\"\n");
    out("#include \"generated/free-ast.h\"\n");
    out("\n");

    for (int i = 0; i < array_size(n->children); ++i) {
        struct Child *child = (struct Child *)array_get(n->children, i);
        out("#include \"generated/free-%s.h\"\n", child->type);
    }

    generate_free_node(n, fp, false);
}

void generate_free_nodeset_header(struct Config *c, FILE *fp,
                                  struct Nodeset *n) {
    out("#pragma once\n");
    out("#include \"generated/ast.h\"\n\n");
    generate_free_nodeset(n, fp, true);
}

void generate_free_nodeset_definitions(struct Config *c, FILE *fp,
                                       struct Nodeset *n) {
    out("#include \"lib/memory.h\"\n");
    out("#include \"generated/free-ast.h\"\n");
    out("\n");

    for (int i = 0; i < array_size(n->nodes); ++i) {
        struct Node *node = (struct Node *)array_get(n->nodes, i);
        out("#include \"generated/free-%s.h\"\n", node->id);
    }
    generate_free_nodeset(n, fp, false);
}

void generate_free_header(struct Config *config, FILE *fp) {
    out("#pragma once\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        struct Node *node = array_get(config->nodes, i);
        out("#include \"generated/free-%s.h\"\n", node->id);
    }
}

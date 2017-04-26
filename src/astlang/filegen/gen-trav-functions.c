#include "ast.h"
#include "filegen-driver.h"
#include "filegen-util.h"
#include "memory.h"
#include "str-ast.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static void generate_replace_node(struct Node *node, FILE *fp, bool header) {
    // Generate replace functions
    out("void replace_%s(struct %s *node)", node->id, node->id);
    if (header) {
        out(";\n");
    } else {
        out(" {\n");
        out("    node_replacement_type = " NT_FMT ";\n", node->id);
        out("    node_replacement = node;\n");
        out("}\n");
    }
}

static void generate_node_child_node(struct Node *node, struct Child *child,
                                     FILE *fp) {
    out("    trav_%s(node->%s, info);\n", child->type, child->id);

    out("    if (node_replacement != NULL) {\n");
    out("        if (node_replacement_type == " NT_FMT ") {\n", child->type);
    out("            node->%s = node_replacement;\n", child->id);
    out("        } else {\n");
    out("            fprintf(stderr, \"Replacement node for %s->%s is not of "
        "type "
        "%s.\");\n",
        node->id, child->id, child->type);
    out("        }\n");
    out("    }\n");
}

static void generate_node_child_nodeset(struct Node *node, struct Child *child,
                                        FILE *fp) {
    out("    switch (node->%s->type) {\n", child->id);

    struct Nodeset *nodeset = child->nodeset;

    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        struct Node *cnode = (struct Node *)array_get(nodeset->nodes, i);
        out("    case " NS_FMT ":\n", nodeset->id, cnode->id);
        out("        trav_%s(node->%s->value.val_%s, info);\n", cnode->id,
            child->id, cnode->id);
        out("        break;\n");
    }
    out("    }\n");
}

static void generate_trav_node(struct Node *node, FILE *fp, bool header) {
    out("static struct %s *trav_%s(struct %s *node, struct Info *info)",
        node->id, node->id, node->id);

    if (header) {
        out(";\n");
    } else {
        out(" {\n");
        out("    return NULL;// SOME_USER_%s_FUN(struct %s *, struct Info "
            "*info);\n",
            node->id, node->id);
        out("}\n\n");
    }

    for (int i = 0; i < array_size(node->children); ++i) {
        struct Child *child = (struct Child *)array_get(node->children, i);
        out("void trav_%s_%s(struct %s *node, struct Info *info)", node->id,
            child->id, node->id);

        if (header) {
            out(";\n");
        } else {
            out(" {\n");
            out("    if (!node) return;\n");

            out("    node_replacement = NULL;\n");

            if (child->node != NULL) {
                // Child is a node
                generate_node_child_node(node, child, fp);
            } else if (child->nodeset != NULL) {
                // Child is a nodeset
                generate_node_child_nodeset(node, child, fp);
            } else {
                // Should not have passed the context analysis.
                assert(0);
            }

            out("}\n\n");
        }
    }
}

static void generate(struct Config *config, FILE *fp, bool header) {
    if (header) {
        out("#pragma once\n");
    }

    out("#include \"ast.h\"\n");

    if (!header) {
        out("#include \"enum.h\"\n");
        out("#include \"trav-ast.h\"\n");
        out("#include <stdio.h>\n");
    }
    out("\n");

    if (header) {
        out("struct Info;\n");
        out("NodeType node_replacement_type;\n");
        out("void *node_replacement;\n");
    }

    out("\n");
    out("// NODES\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        generate_trav_node(array_get(config->nodes, i), fp, header);
    }

    for (int i = 0; i < array_size(config->nodes); ++i) {
        generate_replace_node(array_get(config->nodes, i), fp, header);
    }
}

void generate_trav_definitions(struct Config *config, FILE *fp) {
    generate(config, fp, false);
}

void generate_trav_header(struct Config *config, FILE *fp) {
    generate(config, fp, true);
}

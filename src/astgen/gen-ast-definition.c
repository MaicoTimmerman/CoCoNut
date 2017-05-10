#include <stdio.h>

#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include "astgen/str-ast.h"

#include "lib/array.h"
#include "lib/memory.h"

static void template_enum(Enum *arg_enum, FILE *fp) {
    out("typedef enum {\n");
    for (int i = 0; i < array_size(arg_enum->values); i++) {
        out("    %s_%s,\n", arg_enum->prefix,
            (char *)array_get(arg_enum->values, i));
    }
    out("} %s;\n\n", arg_enum->id);
}

static void generate_nodetype_enum(Config *config, FILE *fp) {

    out("typedef enum {\n");
    for (int i = 0; i < array_size(config->nodes); i++) {
        Node *n = array_get(config->nodes, i);
        out("   " NT_FORMAT ",\n", n->id);
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        Nodeset *n = array_get(config->nodesets, i);
        out("   " NT_FORMAT ",\n", n->id);
    }

    out("} " NT_ENUM_NAME ";\n\n");
}

static void generate_traversal_enum(Config *config, FILE *fp) {
    out("typedef enum {\n");
    for (int i = 0; i < array_size(config->traversals); i++) {
        Traversal *t = array_get(config->traversals, i);
        out("   " TRAV_FORMAT ",\n", t->id);
    }

    out("} " TRAV_ENUM_NAME ";\n\n");
}

static void template_ast_h(Config *config, FILE *fp) {
    // First do forward declaration of all the structs.
    out("\n// Forward declarations of nodes\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        out("struct %s;\n", ((Node *)array_get(config->nodes, i))->id);
    }
    out("\n// Forward declarations of nodesets\n");
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        out("struct %s;\n", ((Nodeset *)array_get(config->nodesets, i))->id);
    }

    // Print node structs
    out("\n// Definitions of nodes\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        Node *node = (Node *)array_get(config->nodes, i);
        out("typedef struct %s {\n", node->id);

        if (node->children) {
            for (int j = 0; j < array_size(node->children); ++j) {
                Child *child = (Child *)array_get(node->children, j);
                out("    struct %s *%s;\n", child->type, child->id);
            }
        }
        if (node->attrs) {
            for (int j = 0; j < array_size(node->attrs); ++j) {
                Attr *attr = (Attr *)array_get(node->attrs, j);
                out("    %s %s;\n", str_attr_type(attr), attr->id);
            }
        }
        out("} %s;\n\n", node->id);
    }

    // typedef enum {
    //     NS_expr_binop, NS_expr_monop, NS_expr_assign
    // } expr;
    out("\n// Definitions of nodesets\n");
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        Nodeset *nodeset = (Nodeset *)array_get(config->nodesets, i);
        out("// Nodeset %s\n", nodeset->id);
        out("typedef enum { ");
        for (int j = 0; j < array_size(nodeset->nodes); ++j) {
            out(NS_FORMAT ", ", nodeset->id,
                ((Node *)array_get(nodeset->nodes, j))->id);
        }
        out("} " NS_ENUMTYPE_FORMAT ";\n", nodeset->id);

        out("typedef struct %s {\n", nodeset->id);
        out("    union {\n");
        for (int j = 0; j < array_size(nodeset->nodes); ++j) {
            Node *node = (Node *)array_get(nodeset->nodes, j);
            out("        struct %s *val_%s;\n", node->id, node->id);
        }
        out("    } value;\n");
        out("    " NS_ENUMTYPE_FORMAT " type;\n", nodeset->id);
        out("} %s;\n\n", nodeset->id);
    }
}

void generate_enum_definitions(Config *config, FILE *fp) {
    out("#pragma once\n");

    generate_nodetype_enum(config, fp);
    generate_traversal_enum(config, fp);

    for (int i = 0; i < array_size(config->enums); i++) {
        template_enum((Enum *)array_get(config->enums, i), fp);
    }
}

void generate_ast_definitions(Config *config, FILE *fp) {
    out("#pragma once\n");
    out("#include \"generated/enum.h\"\n");
    out("#include <stdint.h>\n");
    out("#include <stdbool.h>\n");
    template_ast_h(config, fp);
}

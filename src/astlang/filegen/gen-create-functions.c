#include <stdbool.h>
#include <stdio.h>

#include "array.h"
#include "ast.h"
#include "filegen-driver.h"
#include "filegen-util.h"
#include "memory.h"
#include "smap.h"
#include "str-ast.h"

static void generate_node(struct Node *node, FILE *fp, bool header) {
    out("struct %s *create_%s(", node->id, node->id);

    int arg_count = 0;
    bool first = true;

    for (int i = 0; i < array_size(node->children); i++) {
        struct Child *c = array_get(node->children, i);
        if (c->construct) {
            if (first) {
                first = false;
                out("\n");
            } else {
                out(",\n");
            }

            out("       struct %s *%s", c->type, c->id);
            arg_count++;
        }
    }

    for (int i = 0; i < array_size(node->attrs); i++) {
        struct Attr *attr = array_get(node->attrs, i);
        if (attr->construct) {
            if (first) {
                first = false;
                out("\n");
            } else {
                out(",\n");
            }

            out("       %s %s", str_attr_type(attr), attr->id);
            arg_count++;
        }
    }

    if (arg_count == 0)
        out("void");

    if (header) {
        out(");\n\n");
    } else {
        out(") {\n");

        out("   struct %s *res = mem_alloc(sizeof(struct %s));\n", node->id,
            node->id);

        for (int i = 0; i < array_size(node->children); i++) {
            struct Child *c = array_get(node->children, i);
            if (c->construct) {
                out("   res->%s = %s;\n", c->id, c->id);
            }
        }

        for (int i = 0; i < array_size(node->attrs); i++) {
            struct Attr *attr = array_get(node->attrs, i);
            if (attr->construct) {
                out("   res->%s = %s;\n", attr->id, attr->id);
            }
        }
        out("   return res;\n");

        out("}\n\n");
    }
}

static void generate_nodeset(struct Nodeset *nodeset, FILE *fp, bool header) {
    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        struct Node *node = array_get(nodeset->nodes, i);

        out("struct %s *create_%s_%s(struct %s *_%s)", nodeset->id,
            nodeset->id, node->id, node->id, node->id);

        if (header) {
            out(";\n\n");
        } else {
            out(" {\n");
            out("   struct %s *res = mem_alloc(sizeof(struct %s));\n",
                nodeset->id, nodeset->id);

            out("   res->type = " NS_FMT ";\n", nodeset->id, node->id);
            out("   res->value.val_%s = _%s;\n", node->id, node->id);
            out("   return res;\n");
            out("}\n\n");
        }
    }
}

static void generate(struct Config *c, FILE *fp, bool header) {
    if (header)
        out("#pragma once\n");

    out("#include \"ast.h\"\n");
    if (!header)
        out("#include \"memory.h\"\n");

    out("\n");

    out("// Node create functions\n");
    for (int i = 0; i < array_size(c->nodes); i++) {
        generate_node(array_get(c->nodes, i), fp, header);
    }

    out("// Nodeset create functions\n");
    for (int i = 0; i < array_size(c->nodesets); i++) {
        generate_nodeset(array_get(c->nodesets, i), fp, header);
    }
}

void generate_create_functions(struct Config *c, FILE *fp) {
    generate(c, fp, false);
}

void generate_create_function_headers(struct Config *c, FILE *fp) {
    generate(c, fp, true);
}

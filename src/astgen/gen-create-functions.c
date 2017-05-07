#include <stdbool.h>
#include <stdio.h>

#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include "astgen/str-ast.h"
#include "lib/array.h"
#include "lib/memory.h"
#include "lib/smap.h"

static void generate_node(struct Node *node, FILE *fp, bool header) {
    out("struct %s *" CREATE_NODE_FORMAT "(", node->id, node->id);

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

        out("struct %s *" CREATE_NODESET_FORMAT "(struct %s *_%s)",
            nodeset->id, nodeset->id, node->id, node->id, node->id);

        if (header) {
            out(";\n\n");
        } else {
            out(" {\n");
            out("   struct %s *res = mem_alloc(sizeof(struct %s));\n",
                nodeset->id, nodeset->id);

            out("   res->type = " NS_FORMAT ";\n", nodeset->id, node->id);
            out("   res->value.val_%s = _%s;\n", node->id, node->id);
            out("   return res;\n");
            out("}\n\n");
        }
    }
}

static void generate(struct Config *c, FILE *fp, bool header) {
    if (header)
        out("#pragma once\n");

    out("#include \"generated/ast.h\"\n");
    if (!header)
        out("#include \"lib/memory.h\"\n");

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

void generate_create_node_header(struct Config *c, FILE *fp, struct Node *n) {
    out("#pragma once\n");
    out("#include <stdbool.h>\n");
    out("#include <string.h>\n");
    out("#include \"generated/ast.h\"\n\n");
    generate_node(n, fp, true);
}

void generate_create_node_definitions(struct Config *c, FILE *fp,
                                      struct Node *n) {
    out("#include \"lib/memory.h\"\n");
    out("#include \"generated/create-%s.h\"\n", n->id);

    for (int i = 0; i < array_size(n->children); ++i) {
        struct Child *child = (struct Child *)array_get(n->children, i);
        out("#include \"generated/create-%s.h\"\n", child->type);
    }
    generate_node(n, fp, false);
}

void generate_create_nodeset_header(struct Config *c, FILE *fp,
                                    struct Nodeset *n) {
    out("#pragma once\n");
    out("#include \"generated/ast.h\"\n\n");
    generate_nodeset(n, fp, true);
}

void generate_create_nodeset_definitions(struct Config *c, FILE *fp,
                                         struct Nodeset *n) {
    out("#include \"lib/memory.h\"\n");
    out("\n");

    for (int i = 0; i < array_size(n->nodes); ++i) {
        struct Node *node = (struct Node *)array_get(n->nodes, i);
        out("#include \"generated/create-%s.h\"\n", node->id);
    }
    generate_nodeset(n, fp, false);
}

void generate_create_header(struct Config *config, FILE *fp) {
    out("#pragma once\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        struct Node *node = array_get(config->nodes, i);
        out("#include \"generated/create-%s.h\"\n", node->id);
    }
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        struct Nodeset *nodeset = array_get(config->nodesets, i);
        out("#include \"generated/create-%s.h\"\n", nodeset->id);
    }
}

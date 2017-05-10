#include <stdbool.h>
#include <stdio.h>

#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include "astgen/str-ast.h"
#include "lib/array.h"
#include "lib/memory.h"
#include "lib/smap.h"

static void generate_node(Node *node, FILE *fp, bool header) {
    out("struct %s *" CREATE_NODE_FORMAT "(", node->id, node->id);

    int arg_count = 0;
    bool first = true;

    for (int i = 0; i < array_size(node->children); i++) {
        Child *c = array_get(node->children, i);
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
        Attr *attr = array_get(node->attrs, i);
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
            Child *c = array_get(node->children, i);
            if (c->construct) {
                out("   res->%s = %s;\n", c->id, c->id);
            }
        }

        for (int i = 0; i < array_size(node->attrs); i++) {
            Attr *attr = array_get(node->attrs, i);
            if (attr->construct) {
                out("   res->%s = %s;\n", attr->id, attr->id);
            }
        }
        out("   return res;\n");

        out("}\n\n");
    }
}

static void generate_nodeset(Nodeset *nodeset, FILE *fp, bool header) {
    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *node = array_get(nodeset->nodes, i);

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

void generate_create_node_header(Config *c, FILE *fp, Node *n) {
    out("#pragma once\n");
    out("#include <stdbool.h>\n");
    out("#include <string.h>\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"generated/ast.h\"\n\n");
    generate_node(n, fp, true);
}

void generate_create_node_definitions(Config *c, FILE *fp, Node *n) {
    out("#include \"generated/create-%s.h\"\n", n->id);

    smap_t *map = smap_init(32);

    for (int i = 0; i < array_size(n->children); ++i) {
        Child *child = (Child *)array_get(n->children, i);
        if (smap_retrieve(map, child->type) == NULL) {
            out("#include \"generated/create-%s.h\"\n", child->type);
            smap_insert(map, child->type, child);
        }
    }

    smap_free(map);

    generate_node(n, fp, false);
}

void generate_create_nodeset_header(Config *c, FILE *fp, Nodeset *n) {
    out("#pragma once\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"generated/ast.h\"\n\n");
    generate_nodeset(n, fp, true);
}

void generate_create_nodeset_definitions(Config *c, FILE *fp, Nodeset *n) {
    out("#include \"generated/create-%s.h\"\n", n->id);
    out("\n");

    smap_t *map = smap_init(32);

    for (int i = 0; i < array_size(n->nodes); ++i) {
        Node *node = (Node *)array_get(n->nodes, i);
        if (smap_retrieve(map, node->id) == NULL) {
            out("#include \"generated/create-%s.h\"\n", node->id);
            smap_insert(map, node->id, node);
        }
    }

    smap_free(map);

    generate_nodeset(n, fp, false);
}

void generate_create_header(Config *config, FILE *fp) {
    out("#pragma once\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        Node *node = array_get(config->nodes, i);
        out("#include \"generated/create-%s.h\"\n", node->id);
    }
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        Nodeset *nodeset = array_get(config->nodesets, i);
        out("#include \"generated/create-%s.h\"\n", nodeset->id);
    }
}

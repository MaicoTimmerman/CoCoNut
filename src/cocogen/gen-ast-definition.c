#include <stdio.h>

#include "cocogen/ast.h"
#include "cocogen/filegen-driver.h"
#include "cocogen/filegen-util.h"
#include "cocogen/str-ast.h"

#include "lib/array.h"
#include "lib/memory.h"
#include "lib/smap.h"

static void generate_enum(Enum *arg_enum, FILE *fp) {
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
    if (array_size(config->traversals) == 0) {
        out("    " TRAV_FORMAT "\n", "PLACEHOLDER");
    } else {
        for (int i = 0; i < array_size(config->traversals); i++) {
            Traversal *t = array_get(config->traversals, i);
            out("   " TRAV_FORMAT ",\n", t->id);
        }
    }

    out("} " TRAV_ENUM_NAME ";\n\n");
}

void generate_enum_definitions(Config *config, FILE *fp) {
    out("#pragma once\n");

    generate_nodetype_enum(config, fp);
    generate_traversal_enum(config, fp);

    for (int i = 0; i < array_size(config->enums); i++) {
        generate_enum((Enum *)array_get(config->enums, i), fp);
    }
}

void generate_ast_definitions(Config *config, FILE *fp) {
    out("#pragma once\n");

    out("// Nodes\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        out("#include \"generated/ast-%s.h\"\n",
            ((Node *)array_get(config->nodes, i))->id);
    }
    out("\n");
    out("// Nodesets\n");
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        out("#include \"generated/ast-%s.h\"\n",
            ((Nodeset *)array_get(config->nodesets, i))->id);
    }
}

void generate_ast_node_header(Config *config, FILE *fp, Node *node) {
    out("#pragma once\n");

    generate_node_header_includes(config, fp, node);

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
    out("} %s;", node->id);
}
void generate_ast_nodeset_header(Config *config, FILE *fp, Nodeset *nodeset) {
    out("#pragma once\n");
    out("\n");

    // Create a map of child node types, so that there aren't duplicate
    // includes.
    smap_t *map = smap_init(32);

    out("// Forward declarations of children.\n");
    for (int j = 0; j < array_size(nodeset->nodes); ++j) {
        Node *node = (Node *)array_get(nodeset->nodes, j);
        if (smap_retrieve(map, node->id) == NULL) {
            out("typedef struct %s %s;\n", node->id, node->id);
            smap_insert(map, node->id, node);
        }
    }

    smap_free(map);

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

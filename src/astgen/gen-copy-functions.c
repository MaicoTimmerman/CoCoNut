#include <stdio.h>

#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"

#include "lib/memory.h"
#include "lib/smap.h"

static void generate_node(Node *node, FILE *fp, bool header) {
    out("struct %s *_copy_%s(struct %s *node, imap_t *imap)", node->id,
        node->id, node->id);

    if (header) {
        out(";\n\n");
    } else {
        out(" {\n");

        out("    struct %s *res = mem_alloc(sizeof(struct %s));\n", node->id,
            node->id);

        out("    imap_insert(imap, node, res);\n");

        for (int i = 0; i < array_size(node->children); i++) {
            Child *c = array_get(node->children, i);
            out("    res->%s = _copy_%s(node->%s, imap);\n", c->id, c->type,
                c->id);
        }

        for (int i = 0; i < array_size(node->attrs); i++) {
            Attr *attr = array_get(node->attrs, i);
            if (attr->type == AT_string) {
                out("    res->%s = strdup(node->%s);\n", attr->id, attr->id);
            } else if (attr->type == AT_link) {
                out("    res->%s = imap_retrieve(imap, node->%s);\n", attr->id,
                    attr->id);
            } else {
                out("    res->%s = node->%s;\n", attr->id, attr->id);
            }
        }
        out("    return res;\n");
        out("}\n\n");
    }

    // Define outsize function.
    out("struct %s *" COPY_NODE_FORMAT "(struct %s *node)", node->id, node->id,
        node->id);

    if (header) {
        out(";\n\n");
    } else {
        out(" {\n");
        out("    if (node == NULL) return NULL; // Cannot copy nothing.\n");
        out("\n");
        out("    imap_t *imap = imap_init(64);\n");
        out("    struct %s * res = _copy_%s(node, imap);\n", node->id,
            node->id);
        out("    imap_free(imap);\n");
        out("    return res;\n");
        out("}\n");
    }
}

static void generate_nodeset(Nodeset *nodeset, FILE *fp, bool header) {

    out("struct %s *_copy_%s(struct %s *nodeset, imap_t *imap)", nodeset->id,
        nodeset->id, nodeset->id);

    if (header) {
        out(";\n\n");
    } else {
        out(" {\n");
        out("    struct %s *res = mem_alloc(sizeof(struct %s));\n",
            nodeset->id, nodeset->id);
        out("    imap_insert(imap, nodeset, res);\n");

        out("    res->type = nodeset->type;\n");
        out("    switch (nodeset->type) {\n");
        for (int i = 0; i < array_size(nodeset->nodes); i++) {
            Node *node = array_get(nodeset->nodes, i);
            out("        case " NS_FORMAT ":\n", nodeset->id, node->id);
            out("            res->value.val_%s = "
                "_copy_%s(nodeset->value.val_%s, imap);\n",
                node->id, node->id, node->id);
            out("            break;\n");
        }
        out("    }\n");
        out("    return res;\n");
        out("}\n\n");
    }

    out("struct %s *" COPY_NODE_FORMAT "(struct %s *nodeset)", nodeset->id,
        nodeset->id, nodeset->id);
    if (header) {
        out(";\n\n");
        return;
    } else {
        out("{\n");
        out("    if (nodeset == NULL) return NULL; // Cannot copy nothing.\n");
        out("    imap_t *imap = imap_init(64);\n");
        out("    struct %s * res = _copy_%s(nodeset, imap);\n", nodeset->id,
            nodeset->id);
        out("    imap_free(imap);\n");
        out("    return res;\n");
        out("}\n");
    }
}

void generate_copy_node_header(Config *c, FILE *fp, Node *n) {
    out("#pragma once\n");
    out("#include <stdbool.h>\n");
    out("#include <string.h>\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"generated/ast.h\"\n");
    out("\n");

    // Create a map of child node types, so that there aren't duplicate
    // includes.
    smap_t *map = smap_init(32);

    for (int i = 0; i < array_size(n->children); ++i) {
        Child *child = (Child *)array_get(n->children, i);
        if (smap_retrieve(map, child->type) == NULL) {
            out("#include \"generated/copy-%s.h\"\n", child->type);
            smap_insert(map, child->type, child);
        }
    }

    smap_free(map);

    generate_node(n, fp, true);
}

void generate_copy_node_definitions(Config *c, FILE *fp, Node *n) {
    out("#include \"generated/copy-%s.h\"\n", n->id);
    out("\n");

    generate_node(n, fp, false);
}

void generate_copy_nodeset_header(Config *c, FILE *fp, Nodeset *n) {
    out("#pragma once\n");
    out("#include <stdbool.h>\n");
    out("#include <string.h>\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"generated/ast.h\"\n");
    out("\n");

    smap_t *map = smap_init(32);
    for (int i = 0; i < array_size(n->nodes); ++i) {
        Node *node = (Node *)array_get(n->nodes, i);

        if (smap_retrieve(map, node->id) == NULL) {
            out("#include \"generated/copy-%s.h\"\n", node->id);
            smap_insert(map, node->id, node);
        }
    }
    smap_free(map);

    generate_nodeset(n, fp, true);
}

void generate_copy_nodeset_definitions(Config *c, FILE *fp, Nodeset *n) {
    out("#include \"generated/copy-%s.h\"\n", n->id);
    out("\n");

    generate_nodeset(n, fp, false);
}

void generate_copy_header(Config *config, FILE *fp) {
    out("#pragma once\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        Node *node = array_get(config->nodes, i);
        out("#include \"generated/copy-%s.h\"\n", node->id);
    }
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        Nodeset *nodeset = array_get(config->nodesets, i);
        out("#include \"generated/copy-%s.h\"\n", nodeset->id);
    }
}

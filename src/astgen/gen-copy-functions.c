#include <stdio.h>

#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"

#include "lib/memory.h"

static void generate_node(struct Node *node, FILE *fp, bool header) {
    out("static struct %s *_copy_%s(struct %s *node, imap_t *imap)", node->id,
        node->id, node->id);

    if (header) {
        out(";\n\n");
    } else {
        out(" {\n");

        out("    struct %s *res = mem_alloc(sizeof(struct %s));\n", node->id,
            node->id);

        out("    imap_insert(imap, node, res);\n");

        for (int i = 0; i < array_size(node->children); i++) {
            struct Child *c = array_get(node->children, i);
            out("    res->%s = _copy_%s(node->%s, imap);\n", c->id, c->type,
                c->id);
        }

        for (int i = 0; i < array_size(node->attrs); i++) {
            struct Attr *attr = array_get(node->attrs, i);
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
    out("struct %s *copy_%s(struct %s *node)", node->id, node->id, node->id);

    if (header) {
        out(";\n\n");
    } else {
        out(" {\n");
        out("    if (node == NULL) return NULL; // Cannot copy nothing.\n");
        out("\n");
        out("    imap_t *imap = imap_init(64);\n");
        out("    _copy_%s(node, imap);\n", node->id);
        out("    imap_free(imap);\n");
        out("}\n");
    }
}

static void generate_nodeset(struct Nodeset *nodeset, FILE *fp, bool header) {

    out("static struct %s *_copy_%s(struct %s *nodeset, imap_t *imap)",
        nodeset->id, nodeset->id, nodeset->id);

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
            struct Node *node = array_get(nodeset->nodes, i);
            out("        case " NS_FMT ":\n", nodeset->id, node->id);
            out("            res->value.val_%s = "
                "_copy_%s(nodeset->value.val_%s, imap);\n",
                node->id, node->id, node->id);
            out("            break;\n");
        }
        out("    }\n");
        out("    return res;\n");
        out("}\n\n");
    }

    out("struct %s *copy_%s(struct %s *nodeset)", nodeset->id, nodeset->id,
        nodeset->id);
    if (header) {
        out(";\n\n");
        return;
    } else {
        out("{\n");
        out("    if (nodeset == NULL) return NULL; // Cannot copy nothing.\n");
        out("    imap_t *imap = imap_init(64);\n");
        out("    _copy_%s(nodeset, imap);\n", nodeset->id);
        out("    imap_free(imap);\n");
        out("}\n");
    }
}

static void generate(struct Config *c, FILE *fp, bool header) {
    if (header)
        out("#pragma once\n");

    out("#include \"generated/ast.h\"\n");
    out("#include \"lib/imap.h\"\n");
    if (!header) {
        out("#include \"lib/memory.h\"\n");
        out("#include \"generated/copy-ast.h\"\n");
        out("#include <string.h>\n");
        out("#include <stdbool.h>\n");
    }

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

void generate_copy_definitions(struct Config *c, FILE *fp) {
    generate(c, fp, false);
}

void generate_copy_header(struct Config *c, FILE *fp) {
    generate(c, fp, true);
}

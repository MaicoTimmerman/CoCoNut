#include <stdbool.h>
#include <stdio.h>

#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include "astgen/str-ast.h"

#include "lib/array.h"
#include "lib/memory.h"
#include "lib/smap.h"

// TODO make enum of mandatory/forbidden/optional.
static const int MANDATORY = 0;
static const int FORBIDDEN = 1;

static void generate_node(struct Node *node, FILE *fp, int node_id,
                          bool header) {
    out("void check_%s(struct %s *node, struct Info *info)", node->id,
        node->id);

    if (header) {
        out(";\n\n");
    } else {
        out(" {\n");

        for (int i = 0; i < array_size(node->children); ++i) {
            struct Child *child = (struct Child *)array_get(node->children, i);
            out("    // Check mandatoryness for %s\n", child->id);
            out("    if (node->%s == NULL && "
                "mandatory_phases[info->phase_id][%d][%d] == %d)\n",
                child->id, node_id, i, MANDATORY);
            out("        info->error = 1;\n");
            out("    if (node->%s != NULL && "
                "mandatory_phases[info->phase_id][%d][%d] == %d)\n",
                child->id, node_id, i, FORBIDDEN);
            out("        info->error = 1;\n");
            out("    check_%s(node->%s, info);\n\n", child->type, child->id);
        }
        out("}\n\n");
    }
}

static void generate_nodeset(struct Nodeset *nodeset, FILE *fp, bool header) {
    out("void check_%s(struct %s *nodeset, struct Info *info)", nodeset->id,
        nodeset->id);

    if (header) {
        out(";\n\n");
        return;
    } else {
        out("{\n");
    }
    out("    switch (nodeset->type) {\n");
    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        struct Node *node = (struct Node *)array_get(nodeset->nodes, i);
        out("    case " NS_FORMAT ":\n", nodeset->id, node->id);
        out("        check_%s(nodeset->value.val_%s, info);\n", node->id,
            node->id);
        out("        break;\n");
    }
    out("    }\n");
    out("}\n");
}

/* For every phase, */
/*     for every node, */
/*         for every child, */
/*         for every manatory phase, */
static void generate_mandatory_array(struct Config *config, FILE *fp) {
    out("bool mandatory_phases[][][] = {\n");
    for (int i = 0; i < array_size(config->phases); ++i) {
        struct Phase *phase = (struct Phase *)array_get(config->phases, i);
        out("// Phase %s\n", phase->id);
        for (int j = 0; j < array_size(config->nodes); ++j) {
            struct Node *node = (struct Node *)array_get(config->nodes, j);
            out("    {\n");
            out("// Node %s\n", node->id);
            for (int k = 0; k < array_size(node->children); ++k) {
                struct Child *c = (struct Child *)array_get(node->children, k);
                out("// Node %s -> Child %s\n", node->id, c->id);
            }
            out("    },\n");
        }
    }
    out("};\n");
}

static void generate_info(FILE *fp) {
    out("struct Info {\n");
    out("    enum phase phase_id;\n");
    out("    bool error;\n");
    out("};\n");
    out("\n");

    out("static struct Info *create_info(enum phase phase_id) {\n");
    out("     struct Info *info = (struct Info *) malloc(sizeof(struct "
        "Info));\n");
    out("     info->phase_id = phase_id;\n");
    out("     info->error = 0;\n");
    out("     return info;\n");
    out("}\n");
    out("\n");

    out("static void free_info(struct Info *info) {\n");
    out("     free(info);\n");
    out("}\n");
}

static void generate(struct Config *c, FILE *fp, bool header) {
    if (header) {
        out("#pragma once\n");
    }

    if (!header) {
        out("#include \"lib/memory.h\"\n");
        out("#include \"generated/consistency-ast.h\"\n");
    }

    out("#include \"generated/ast.h\"\n");
    out("#include <stdbool.h>\n\n");

    if (header) {
        generate_mandatory_array(c, fp);
        out("struct Info;\n");
    } else {
        generate_info(fp);
    }

    out("// Node create functions\n");
    for (int i = 0; i < array_size(c->nodes); i++) {
        generate_node(array_get(c->nodes, i), fp, i, header);
    }

    out("// Nodeset create functions\n");
    for (int i = 0; i < array_size(c->nodesets); i++) {
        generate_nodeset(array_get(c->nodesets, i), fp, header);
    }
}

void generate_consistency_definitions(struct Config *c, FILE *fp) {
    generate(c, fp, false);
}

void generate_consistency_header(struct Config *c, FILE *fp) {
    generate(c, fp, true);
}

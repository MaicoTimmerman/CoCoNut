#include <stdbool.h>
#include <stdio.h>

#include "array.h"
#include "ast.h"
#include "filegen-driver.h"
#include "memory.h"
#include "smap.h"
#include "str-ast.h"

#define out(...) fprintf(fp, __VA_ARGS__)

static const int MANDATORY = 0;
static const int FORBIDDEN = 1;

static void generate_node(struct Node *node, FILE *fp, bool header) {
    out("struct %s *check_%s(struct %s *node, struct Info *info)",
            node->id, node->id, node->id);

    if (header) {
        out(";\n\n");
    } else {
        out(" {\n");

        for (int i = 0; i < array_size(node->children); ++i) {
            struct Child *child = (struct Child*)array_get(node->children, i);
            out("     // Check mandatoryness for %s\n", child->id);
            out("     if (node->%s == NULL && "
                // TODO Fix the indices for the phase and node
                "mandatory_phases[info->phase_id][%d][%d] == %d)\n",
                child->id, 1234234, 23432423, MANDATORY);
            out("          error = 1;\n");
            out("     if (node->%s != NULL && "
                // TODO Fix the indices for the phase and node
                "mandatory_phases[info->phase_id][%d][%d] == %d)\n",
                child->id, 1234234, 23432423, FORBIDDEN);
            out("          error = 1;\n");
        }
        out("}\n\n");
    }
}

static void generate_nodeset(struct Nodeset *nodeset, FILE *fp, bool header) {
    out("struct %s *check_%s(struct %s *nodeset, struct Info *info)",
        nodeset->id, nodeset->id, nodeset->id);

    if (header) {
        out(";\n\n");
        return;
    } else {
        out("{\n");
    }
    out("    switch (nodeset->type) {\n");
    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        char *node = (char *)array_get(nodeset->nodes, i);
        out("    case NS_%s_%s:\n", nodeset->id, node);
        out("        check_%s(nodeset->value.val_%s, info);\n", node, node);
        out("        break;\n");
    }
    out("    }\n");
    out("}\n");
}

static void generate_mandatory(struct Config *config, FILE *fp) {
    out("static bool mandatory_phases[][][] = {\n");
    for (int i = 0; i < array_size(config->phases); ++i) {
        struct Phase *phase = (struct Phase *)array_get(config->phases, i);
        out("// Phase %s\n", phase->id);
        // TODO generate true/false for all children of all nodes.
        out("    {},\n");
    }
    out("}\n");
}

static void generate_info(FILE *fp) {
        out("struct Info {\n");
        out("    enum phase_id phase;\n");
        out("    bool error;\n");
        out("};\n");
        out("\n");

        out("static struct Info *create_info(enum phase_id phase) {\n");
        out("     struct Info *info = (struct info *) malloc(sizeof(struct Info));\n");
        out("     info->phase = phase;\n");
        out("     error = 0;\n");
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
        generate_mandatory(c, fp);
    }


    out("#include \"ast.h\"\n");
    if (!header) {
        out("#include \"memory.h\"\n\n");
        generate_info(fp);
    }

    out("// Node create functions\n");
    for (int i = 0; i < array_size(c->nodes); i++) {
        generate_node(array_get(c->nodes, i), fp, header);
    }

    out("// Nodeset create functions\n");
    for (int i = 0; i < array_size(c->nodesets); i++) {
        generate_nodeset(array_get(c->nodesets, i), fp, header);
    }
}

void generate_consistency_definition(struct Config *c, FILE *fp) {
    generate(c, fp, false);
}

void generate_consistency_header(struct Config *c, FILE *fp) {
    generate(c, fp, true);
}

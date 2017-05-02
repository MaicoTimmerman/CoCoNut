#include "ast.h"
#include "filegen-driver.h"
#include "filegen-util.h"
#include "memory.h"
#include "str-ast.h"
#include <stdbool.h>
#include <stdio.h>

static inline void print_indent(int level, char *single_indent, FILE *fp) {
    for (int i = 0; i < level; i++)
        out(single_indent);
}

static void print_phase_tree(struct Phase *p, int level, FILE *fp,
                             char *root_node_name) {

    if (p->type == PH_subphases) {
        out("    printf(\"");
        print_indent(level, "**", fp);

        out(" %s\\n\");\n", p->info != NULL ? p->info : p->id);
        for (int i = 0; i < array_size(p->subphases); i++) {
            print_phase_tree(array_get(p->subphases, i), level + 1, fp,
                             root_node_name);
        }
    } else {

        for (int i = 0; i < array_size(p->passes); i++) {
            struct PhaseLeaf *leaf = array_get(p->passes, i);

            out("    printf(\"");
            print_indent(level + 1, "--", fp);

            if (leaf->type == PL_traversal) {
                struct Traversal *trav = leaf->value.traversal;
                out(" %s\\n\");\n",
                    trav->info != NULL ? trav->info : trav->id);
                out("    trav_start_%s(syntaxtree, TRAV_%s);\n",
                    root_node_name, trav->id);
            } else {
                struct Pass *pass = leaf->value.pass;
                out(" %s\\n\");\n",
                    pass->info != NULL ? pass->info : pass->id);
                out("    pass_%s_entry(syntaxtree);\n", pass->id);
            }
        }
    }
}

static void generate(struct Config *config, FILE *fp, bool header) {
    char *root_node_name;
    if (config->root_node)
        root_node_name = config->root_node->id;
    else
        root_node_name = config->root_nodeset->id;

    if (header) {
        out("#pragma once\n");
    } else {
        out("#include <stdio.h>\n");
        out("#include \"ast.h\"\n");
        out("#include \"trav-ast.h\"\n");

        for (int i = 0; i < array_size(config->passes); i++) {
            struct Pass *p = array_get(config->passes, i);

            out("#include \"pass-%s.h\"\n", p->id);
        }
    }

    out("\n");

    out("void phasedriver_run(%s *syntaxtree)", root_node_name);

    if (header) {
        out(";\n");
    } else {
        out(" {\n");
        print_phase_tree(config->phase_tree, 1, fp, root_node_name);
        out("}\n");
    }
}

void generate_phase_driver_definitions(struct Config *config, FILE *fp) {
    generate(config, fp, false);
}

void generate_phase_driver_header(struct Config *config, FILE *fp) {
    generate(config, fp, true);
}

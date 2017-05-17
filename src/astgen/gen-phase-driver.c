#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include "astgen/str-ast.h"
#include "lib/memory.h"
#include <stdbool.h>
#include <stdio.h>

static inline void print_indent(int level, char *single_indent, FILE *fp) {
    for (int i = 0; i < level; i++)
        out(single_indent);
}

static void print_phase_tree(Phase *p, int level, FILE *fp,
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
            PhaseLeaf *leaf = array_get(p->passes, i);

            out("    printf(\"");
            print_indent(level + 1, "--", fp);

            if (leaf->type == PL_traversal) {
                Traversal *trav = leaf->value.traversal;
                out(" %s\\n\");\n",
                    trav->info != NULL ? trav->info : trav->id);
                out("    trav_start_%s(syntaxtree, TRAV_%s);\n",
                    root_node_name, trav->id);
            } else {
                Pass *pass = leaf->value.pass;
                out(" %s\\n\");\n",
                    pass->info != NULL ? pass->info : pass->id);
                out("    pass_%s_entry(syntaxtree);\n", pass->id);
            }
        }
    }
}

static void generate(Config *config, FILE *fp, bool header) {
    if (header) {
        out("#pragma once\n");
    } else {
        out("#include <stdio.h>\n");
        out("#include \"generated/ast.h\"\n");
        out("#include \"generated/trav-core.h\"\n");
        out("#include \"generated/trav-%s.h\"\n", config->root_node->id);

        for (int i = 0; i < array_size(config->passes); i++) {
            Pass *p = array_get(config->passes, i);

            out("#include \"generated/pass-%s.h\"\n", p->id);
        }
    }

    out("\n");

    out("void phasedriver_run(%s *syntaxtree)", config->root_node->id);

    if (header) {
        out(";\n");
    } else {
        out(" {\n");
        print_phase_tree(config->phase_tree, 1, fp, config->root_node->id);
        out("}\n");
    }
}

void generate_phase_driver_definitions(Config *config, FILE *fp) {
    generate(config, fp, false);
}

void generate_phase_driver_header(Config *config, FILE *fp) {
    generate(config, fp, true);
}

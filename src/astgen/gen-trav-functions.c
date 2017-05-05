#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include "astgen/str-ast.h"
#include "lib/imap.h"
#include "lib/memory.h"
#include "lib/smap.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Map from node name to index in reachability matrix
static smap_t *node_index = NULL;

// Node reachability matrix
static bool **node_reachability = NULL;

static bool **traversal_node_handles = NULL;

static void compute_reachable_nodes(struct Config *config) {
    node_index = smap_init(32);

    size_t num_nodes = array_size(config->nodes);
    size_t num_nodesets = array_size(config->nodesets);

    // Add nodes to node_index map
    for (int i = 0; i < num_nodes; i++) {
        struct Node *node = array_get(config->nodes, i);
        int *index = mem_alloc(sizeof(int));
        *index = i;
        smap_insert(node_index, node->id, index);
    }

    for (int i = 0; i < num_nodesets; i++) {
        struct Nodeset *nodeset = array_get(config->nodesets, i);
        int *index = mem_alloc(sizeof(int));
        *index = i + num_nodes;
        smap_insert(node_index, nodeset->id, index);
    }

    // Allocate node reachability matrix

    size_t num_total = num_nodes + num_nodesets;

    node_reachability = mem_alloc(sizeof(bool *) * num_total);

    for (int i = 0; i < num_total; i++) {
        node_reachability[i] = mem_alloc(sizeof(bool) * num_total);
        memset(node_reachability[i], 0, sizeof(bool) * num_total);
    }

    // Initialise reachability matrix with adjacency matrix

    for (int i = 0; i < num_nodes; i++) {
        struct Node *node = array_get(config->nodes, i);

        for (int j = 0; j < array_size(node->children); j++) {
            struct Child *child = array_get(node->children, j);

            int *index = smap_retrieve(node_index, child->type);
            node_reachability[*index][i] = true;
        }
    }

    for (int i = 0; i < num_nodesets; i++) {
        struct Nodeset *nodeset = array_get(config->nodesets, i);

        for (int j = 0; j < array_size(nodeset->nodes); j++) {
            struct Node *node = array_get(nodeset->nodes, j);
            int *index = smap_retrieve(node_index, node->id);
            node_reachability[*index][num_nodes + i] = true;
        }
    }

    // Compute reachability of nodes using the Floyd-Warshall algorithm
    for (int k = 0; k < num_total; k++) {
        for (int i = 0; i < num_total; i++) {
            for (int j = 0; j < num_total; j++) {

                if (node_reachability[k][i] && node_reachability[j][k])
                    node_reachability[j][i] = true;
            }
        }
    }

    // Fill traversal_node_handles table

    size_t num_traversals = array_size(config->traversals);

    traversal_node_handles = mem_alloc(sizeof(bool *) * num_traversals);

    for (int i = 0; i < num_traversals; i++) {
        struct Traversal *traversal = array_get(config->traversals, i);

        traversal_node_handles[i] = mem_alloc(sizeof(bool) * num_total);

        // Traversal handles all nodes
        if (traversal->nodes == NULL) {
            for (int j = 0; j < num_total; j++) {
                traversal_node_handles[i][j] = true;
            }
        } else {
            memset(traversal_node_handles[i], 0, sizeof(bool) * num_total);

            for (int j = 0; j < array_size(traversal->nodes); j++) {
                char *node_name = array_get(traversal->nodes, j);
                int *index = smap_retrieve(node_index, node_name);

                // List of nodes from where the handled node can be reached
                bool *reach_nodes = node_reachability[*index];

                // Add the handled node itself to the list of nodes that need
                // to be traversed
                traversal_node_handles[i][*index] = true;

                // Add the nodes in reach_node
                for (int k = 0; k < num_total; k++) {
                    if (reach_nodes[k])
                        traversal_node_handles[i][k] = true;
                }
            }
        }
    }
}

static void generate_replace_node(struct Node *node, FILE *fp, bool header) {
    // Generate replace functions
    out("void " REPLACE_NODE_FORMAT "(struct %s *node)", node->id, node->id);
    if (header) {
        out(";\n");
    } else {
        out(" {\n");
        out("    node_replacement_type = " NT_FORMAT ";\n", node->id);
        out("    node_replacement = node;\n");
        out("}\n");
    }
}

static void generate_start_node(struct Config *config, FILE *fp, bool header) {
    for (int i = 0; i < array_size(config->nodes); ++i) {
        struct Node *node = (struct Node *)array_get(config->nodes, i);
        // Generate start functions
        out("void " TRAV_START_FORMAT "(struct %s *node, TraversalType trav)",
            node->id, node->id);
        if (header) {
            out(";\n");
        } else {
            out(" {\n");
            out("    // Inside of the struct Info* is unknown, thus hide "
                "under void.\n");
            out("    void* info;\n");
            out("\n");
            out("    // Set the new traversal as current traversal.\n");
            out("    " TRAV_PREFIX "push(trav);\n");
            out("\n");
            out("    switch(trav) {\n");
            for (int j = 0; j < array_size(config->traversals); ++j) {
                struct Traversal *trav =
                    (struct Traversal *)array_get(config->traversals, j);
                out("    case " TRAV_FORMAT ":\n", trav->id);
                out("        info = %s_createinfo();\n", trav->id);
                out("        " TRAV_PREFIX "%s(node, info);\n", node->id);
                out("        %s_freeinfo(info);\n", trav->id);
                out("        break;\n");
            }
            out("    }\n");
            out("    " TRAV_PREFIX "pop();\n");
            out("}\n");
        }
    }
}

static void generate_node_child_node(struct Node *node, struct Child *child,
                                     FILE *fp) {
    out("    " TRAV_PREFIX "%s(node->%s, info);\n", child->type, child->id);

    out("    if (node_replacement != NULL) {\n");
    out("        if (node_replacement_type == " NT_FORMAT ") {\n",
        child->type);
    out("            node->%s = node_replacement;\n", child->id);
    out("        } else {\n");
    out("            fprintf(stderr, \"Replacement node for %s->%s is not of "
        "type "
        "%s.\");\n",
        node->id, child->id, child->type);
    out("        }\n");
    out("    }\n");
}

static void generate_node_child_nodeset(struct Node *node, struct Child *child,
                                        FILE *fp) {
    out("    if (!node->%s) return;\n", child->id);
    out("    switch (node->%s->type) {\n", child->id);

    struct Nodeset *nodeset = child->nodeset;

    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        struct Node *cnode = (struct Node *)array_get(nodeset->nodes, i);
        out("    case " NS_FORMAT ":\n", nodeset->id, cnode->id);
        out("        trav_%s(node->%s->value.val_%s, info);\n", cnode->id,
            child->id, cnode->id);
        out("        break;\n");
    }
    out("    }\n");
}

static void generate_trav_node(struct Node *node, FILE *fp,
                               struct Config *config, bool header) {

    if (!header) {
        out("static void " TRAV_PREFIX
            "%s(struct %s *node, struct Info *info) {\n",
            node->id, node->id);
        out("   if (!node) return;\n");
        out("   switch (" TRAV_PREFIX "current()) {\n");
        for (int i = 0; i < array_size(config->traversals); i++) {
            struct Traversal *t = array_get(config->traversals, i);

            bool handles_node = t->nodes == NULL;
            for (int j = 0; j < array_size(t->nodes); j++) {
                char *node_name = array_get(t->nodes, j);
                if (strcmp(node->id, node_name) == 0) {
                    handles_node = true;
                    break;
                }
            }

            out("   case " TRAV_FORMAT ":\n", t->id);

            if (handles_node) {
                out("       " TRAVERSAL_HANDLER_FORMAT "(node, info);\n",
                    t->id, node->id);
            } else {

                for (int j = 0; j < array_size(node->children); j++) {
                    struct Child *c = array_get(node->children, j);

                    int *index = smap_retrieve(node_index, c->type);
                    bool handles_child = traversal_node_handles[i][*index];

                    if (handles_child)
                        out("       trav_%s_%s(node, info);\n", node->id,
                            c->id);
                }
            }
            out("       break;\n");
        }

        out("   default:\n");
        for (int i = 0; i < array_size(node->children); i++) {
            struct Child *c = array_get(node->children, i);
            out("       trav_%s_%s(node, info);\n", node->id, c->id);
        }
        out("       break;\n");
        out("   }\n");
        out("}\n\n");
    }

    for (int i = 0; i < array_size(node->children); ++i) {
        struct Child *child = (struct Child *)array_get(node->children, i);
        out("void " TRAV_PREFIX "%s_%s(struct %s *node, struct Info *info)",
            node->id, child->id, node->id);

        if (header) {
            out(";\n");
        } else {
            out(" {\n");
            out("    if (!node) return;\n");

            out("    node_replacement = NULL;\n");

            if (child->node != NULL) {
                // Child is a node
                generate_node_child_node(node, child, fp);
            } else if (child->nodeset != NULL) {
                // Child is a nodeset
                generate_node_child_nodeset(node, child, fp);
            } else {
                // Should not have passed the context analysis.
                assert(0);
            }

            out("}\n\n");
        }
    }
}

static void generate_stack_functions(FILE *fp, bool header) {
    if (header) {
        out("struct TravStack {\n");
        out("    struct TravStack *prev;\n");
        out("    " TRAV_ENUM_NAME " current;\n");
        out("};\n\n");
    }

    out("void " TRAV_PREFIX "push(" TRAV_ENUM_NAME " trav)");
    if (header) {
        out(";\n");
    } else {
        out(" {\n");
        out("    struct TravStack *new = (struct "
            "TravStack*)mem_alloc(sizeof(struct TravStack));\n");
        out("    new->current = trav;\n");
        out("    new->prev = current_traversal;\n");
        out("    current_traversal = new;\n");
        out("}\n\n");
    }

    out("void " TRAV_PREFIX "pop(void)");
    if (header) {
        out(";\n");
    } else {
        out(" {\n");
        out("    if (current_traversal == NULL) {\n");
        out("        fprintf(stderr, \"Cannot pop of empty stack.\");\n");
        out("        return;\n");
        out("    }\n");
        out("    struct TravStack *prev = current_traversal->prev;\n");
        out("    mem_free(current_traversal);\n");
        out("    current_traversal = prev;\n");
        out("}\n\n");
    }

    out(TRAV_ENUM_NAME " trav_current(void)");
    if (header) {
        out(";\n");
    } else {
        out(" {\n");
        out("    return current_traversal->current;\n");
        out("}\n\n");
    }
}

static void generate(struct Config *config, FILE *fp, bool header) {
    if (header) {
        out("#pragma once\n");
    }

    // TODO: Figure out which node headers are needed, instead of ast.h.
    out("#include \"generated/ast.h\"\n");

    if (!header) {
        out("#include \"generated/enum.h\"\n");
        out("#include \"generated/trav-ast.h\"\n");
        out("#include \"lib/memory.h\"\n");
        out("#include <stdio.h>\n");
    }
    out("\n");

    if (header) {
        out("struct Info;\n");
        generate_stack_functions(fp, header);

        out(NT_ENUM_NAME " node_replacement_type;\n");
        out("void *node_replacement;\n");

    } else {
        for (int i = 0; i < array_size(config->traversals); i++) {
            struct Traversal *t = array_get(config->traversals, i);
            out("#include \"generated/traversal-%s.h\"\n", t->id);
        }
        out("\n");
        out("// Stack of traversals, so that new traversals can be started "
            "inside other traversals. \n");
        out("static struct TravStack *current_traversal;\n");

        generate_stack_functions(fp, header);

        for (int i = 0; i < array_size(config->nodes); ++i) {
            struct Node *node = (struct Node *)array_get(config->nodes, i);
            out("static void trav_%s(struct %s *node, struct Info *info);\n",
                node->id, node->id);
        }
    }

    out("\n");
    out("// NODES\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        generate_trav_node(array_get(config->nodes, i), fp, config, header);
    }

    out("\n");
    out("// Replace functions\n");

    for (int i = 0; i < array_size(config->nodes); ++i) {
        generate_replace_node(array_get(config->nodes, i), fp, header);
    }

    out("\n");
    out("// start functions functions\n");

    generate_start_node(config, fp, header);
}

void generate_trav_definitions(struct Config *config, FILE *fp) {
    compute_reachable_nodes(config);

    generate(config, fp, false);
}

void generate_trav_header(struct Config *config, FILE *fp) {
    generate(config, fp, true);
}

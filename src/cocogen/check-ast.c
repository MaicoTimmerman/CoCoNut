#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cocogen/ast.h"
#include "cocogen/check-ast.h"

#include "lib/array.h"
#include "lib/memory.h"
#include "lib/print.h"
#include "lib/smap.h"

struct Info {
    smap_t *enum_name;
    smap_t *enum_prefix;
    smap_t *node_name;
    smap_t *nodeset_name;
    smap_t *traversal_name;
    smap_t *phase_name;
    smap_t *pass_name;

    Node *root_node;
    Phase *root_phase;
};

static struct Info *create_info(void) {

    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));

    info->enum_name = smap_init(32);
    info->enum_prefix = smap_init(32);
    info->node_name = smap_init(32);
    info->nodeset_name = smap_init(32);
    info->traversal_name = smap_init(32);
    info->phase_name = smap_init(32);
    info->pass_name = smap_init(32);

    info->root_node = NULL;
    info->root_phase = NULL;
    return info;
}

static void free_info(struct Info *info) {
    smap_free(info->enum_name);
    smap_free(info->enum_prefix);
    smap_free(info->node_name);
    smap_free(info->nodeset_name);
    smap_free(info->traversal_name);
    smap_free(info->phase_name);
    smap_free(info->pass_name);
    mem_free(info);
}

static void *check_name_exists(struct Info *info, char *name) {
    Enum *enum_orig;
    Node *node_orig;
    Nodeset *nodeset_orig;
    Traversal *traversal_orig;
    Phase *phase_orig;
    Pass *pass_orig;

    if ((enum_orig = smap_retrieve(info->enum_name, name)) != NULL)
        return enum_orig->id;
    if ((node_orig = smap_retrieve(info->node_name, name)) != NULL)
        return node_orig->id;
    if ((nodeset_orig = smap_retrieve(info->nodeset_name, name)) != NULL)
        return nodeset_orig->id;
    if ((traversal_orig = smap_retrieve(info->traversal_name, name)) != NULL)
        return traversal_orig->id;
    if ((phase_orig = smap_retrieve(info->phase_name, name)) != NULL)
        return phase_orig->id;
    if ((pass_orig = smap_retrieve(info->pass_name, name)) != NULL)
        return pass_orig->id;
    return NULL;
}

static int check_enums(array *enums, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(enums); ++i) {
        Enum *cur_enum = (Enum *)array_get(enums, i);
        void *orig_def;

        // TODO check if name of enum overlaps with autogen enum.
        if ((orig_def = check_name_exists(info, cur_enum->id)) != NULL) {
            print_error(cur_enum->id, "Redefinition of name '%s'",
                        cur_enum->id);
            print_note(orig_def, "Previously declared here");
            error = 1;
        } else {
            smap_insert(info->enum_name, cur_enum->id, cur_enum);
        }

        char *orig_prefix;
        // TODO check if prefix is not forbidden: NS_.
        if ((orig_prefix =
                 smap_retrieve(info->enum_prefix, cur_enum->prefix)) != NULL) {
            print_error(cur_enum->prefix, "Redefinition of prefix '%s'",
                        cur_enum->prefix);
            print_note(orig_prefix, "Previously declared here");
            error = 1;
        } else {
            smap_insert(info->enum_prefix, cur_enum->prefix, cur_enum->prefix);
        }
    }
    return error;
}

static int check_nodes(array *nodes, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(nodes); ++i) {
        Node *cur_node = (Node *)array_get(nodes, i);
        void *orig_def;

        if ((orig_def = check_name_exists(info, cur_node->id))) {
            print_error(cur_node->id, "Redefinition of name '%s'",
                        cur_node->id);
            print_note(orig_def, "Previously declared here");
            error = 1;
        } else {
            smap_insert(info->node_name, cur_node->id, cur_node);
        }

        if (cur_node->root) {
            if (info->root_node != NULL) {
                orig_def = info->root_node->id;
                print_error(cur_node->id,
                            "Duplicate declaration of root node");
                print_note(orig_def, "Previously declared here");
                error = 1;
            } else {
                info->root_node = cur_node;
            }
        }
    }
    return error;
}

static int check_nodesets(array *nodesets, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(nodesets); ++i) {
        Nodeset *cur_nodeset = (Nodeset *)array_get(nodesets, i);
        void *orig_def;

        if ((orig_def = check_name_exists(info, cur_nodeset->id)) != NULL) {
            print_error(cur_nodeset->id, "Redefinition of name '%s'",
                        cur_nodeset->id);
            print_note(orig_def, "Previously declared here");
            error = 1;
        } else {
            smap_insert(info->nodeset_name, cur_nodeset->id, cur_nodeset);
        }
    }
    return error;
}

static int check_phases(array *phases, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(phases); ++i) {
        Phase *cur_phase = (Phase *)array_get(phases, i);
        void *orig_def;

        if ((orig_def = check_name_exists(info, cur_phase->id)) != NULL) {
            print_error(cur_phase->id, "Redefinition of name '%s'",
                        cur_phase->id);
            print_note(orig_def, "Previously declared here");
            error = 1;
        } else {
            smap_insert(info->phase_name, cur_phase->id, cur_phase);
        }
    }
    return error;
}

static int check_passes(array *passes, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(passes); ++i) {
        Pass *cur_pass = (Pass *)array_get(passes, i);
        void *orig_def;

        if ((orig_def = check_name_exists(info, cur_pass->id)) != NULL) {
            print_error(cur_pass->id, "Redefinition of name '%s'",
                        cur_pass->id);
            print_note(orig_def, "Previously declared here");
        } else {
            smap_insert(info->pass_name, cur_pass->id, cur_pass);
        }
    }
    return error;
}

static int check_traversals(array *traversals, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(traversals); ++i) {
        Traversal *cur_traversal = (Traversal *)array_get(traversals, i);
        void *orig_def;

        if ((orig_def = check_name_exists(info, cur_traversal->id)) != NULL) {
            print_error(cur_traversal->id, "Redefinition of name '%s'",
                        cur_traversal->id);
            print_note(orig_def, "Previously declared here");
            error = 1;
        } else {
            smap_insert(info->traversal_name, cur_traversal->id,
                        cur_traversal);
        }
    }
    return error;
}

static int check_mandatory_phase(MandatoryPhase *phase, struct Info *info) {
    int error = 0;

    switch (phase->type) {
    case MP_single:
        if (smap_retrieve(info->phase_name, phase->value.single) == NULL) {
            print_error(phase->value.single, "Unknown mandatory phase '%s'",
                        phase->value.single);
            error = 1;
        }
        break;

    case MP_range:; // Crazy GCC won't allow declaration behind statement.

        PhaseRange *phase_range = phase->value.range;
        if (smap_retrieve(info->phase_name, phase_range->start) == NULL) {
            print_error(phase_range->start,
                        "Unknown mandatory phase range start '%s'",
                        phase_range->start);
            error = 1;
        }
        if (smap_retrieve(info->phase_name, phase_range->end) == NULL) {
            print_error(phase_range->end,
                        "Unknown mandatory phase range end '%s'",
                        phase_range->end);
            error = 1;
        }
        break;
    }

    // TODO: Check if there are no overlapping/duplicate phases.

    return error;
}

static int check_node(Node *node, struct Info *info) {
    int error = 0;

    smap_t *child_name = smap_init(16);

    if (node->children) {
        for (int i = 0; i < array_size(node->children); ++i) {
            Child *child = (Child *)array_get(node->children, i);
            Child *orig_child;

            // Check if there is no duplicate naming.
            if ((orig_child = smap_retrieve(child_name, child->id)) != NULL) {
                print_error(child->id,
                            "Duplicate name '%s' in children of node '%s'",
                            child->id, node->id);
                print_note(orig_child->id, "Previously declared here");
                error = 1;
            } else {
                smap_insert(child_name, child->id, child);
            }

            Node *child_node =
                (Node *)smap_retrieve(info->node_name, child->type);
            Nodeset *child_nodeset =
                (Nodeset *)smap_retrieve(info->nodeset_name, child->type);

            if (!child_node && !child_nodeset) {
                print_error(child->type,
                            "Unknown type '%s' of child '%s' of node '%s'",
                            child->type, child->id, node->id);
                error = 1;
            } else {
                child->node = child_node;
                child->nodeset = child_nodeset;
            }

            if (child_node && child_node == info->root_node) {
                print_error(
                    child->id,
                    "Child '%s' of node '%s' cannot use root node as type",
                    child->id, node->id);
                error = 1;
            }

            for (int i = 0; i < array_size(child->mandatory_phases); ++i) {
                MandatoryPhase *phase =
                    (MandatoryPhase *)array_get(child->mandatory_phases, i);

                error = +check_mandatory_phase(phase, info);
            }
        }
    }

    smap_t *attr_name = smap_init(16);

    if (node->attrs) {
        for (int i = 0; i < array_size(node->attrs); i++) {
            Attr *attr = (Attr *)array_get(node->attrs, i);
            Attr *orig_attr;

            if ((orig_attr = smap_retrieve(attr_name, attr->id)) != NULL) {
                print_error(attr->id,
                            "Duplicate name '%s' in atributes of node '%s'",
                            attr->id, node->id);
                print_note(orig_attr->id, "Previously declared here");
                error = 1;
            } else {
                smap_insert(attr_name, attr->id, attr);
            }

            if (attr->type == AT_link_or_enum) {
                Node *attr_node =
                    (Node *)smap_retrieve(info->node_name, attr->type_id);
                Enum *attr_enum =
                    (Enum *)smap_retrieve(info->enum_name, attr->type_id);

                if (attr_node) {
                    attr->type = AT_link;
                } else if (attr_enum) {
                    attr->type = AT_enum;
                } else {
                    print_error(
                        attr->type_id,
                        "Unknown type '%s' of attribute '%s' of node '%s'",
                        attr->type_id, attr->id, node->id);
                    error = 1;
                }
            }
        }
    }

    smap_free(child_name);
    smap_free(attr_name);

    return error;
}

static int check_nodeset(Nodeset *nodeset, struct Info *info) {
    int error = 0;

    smap_t *node_name = smap_init(16);

    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        char *node = (char *)array_get(nodeset->nodes, i);
        char *orig_node;

        // Check if there is no duplicate naming.
        if ((orig_node = smap_retrieve(node_name, node)) != NULL) {
            print_error(node, "Duplicate name '%s' in nodes of nodeset '%s'",
                        node, nodeset->id);
            print_note(orig_node, "Previously declared here");
            error = 1;
        } else {
            smap_insert(node_name, node, node);
        }

        Node *nodeset_node = (Node *)smap_retrieve(info->node_name, node);

        Nodeset *nodeset_nodeset =
            (Nodeset *)smap_retrieve(info->nodeset_name, node);

        if (nodeset_nodeset) {
            print_error(node, "Nodeset '%s' contains other nodeset '%s'",
                        nodeset->id, nodeset_nodeset->id);
            error = 1;
        } else if (!nodeset_node) {
            print_error(node, "Unknown type of node '%s' in nodeset '%s'",
                        node, nodeset->id);
            error = 1;
        } else {
            mem_free(node);
            array_set(nodeset->nodes, i, nodeset_node);
        }
    }

    smap_free(node_name);

    return error;
}

static int check_enum(Enum *arg_enum, struct Info *info) {

    int error = 0;
    smap_t *value_name = smap_init(16);

    for (int i = 0; i < array_size(arg_enum->values); ++i) {
        char *cur_value = (char *)array_get(arg_enum->values, i);
        char *orig_value;

        // Check if there is no duplicate naming.
        if ((orig_value = smap_retrieve(value_name, cur_value)) != NULL) {
            print_error(cur_value,
                        "Duplicate name '%s' in values of enum '%s'",
                        cur_value, arg_enum->id);
            print_note(orig_value, "Previously declared here");

            error = 1;
        } else {
            smap_insert(value_name, cur_value, cur_value);
        }
    }

    smap_free(value_name);
    return error;
}

static int check_traversal(Traversal *traversal, struct Info *info) {

    // TODO: check collission of func

    int error = 0;

    if (traversal->nodes == NULL)
        return 0;

    smap_t *node_name = smap_init(16);
    smap_t *node_name_expanded = smap_init(16);

    array *nodes_expanded = array_init(16);

    for (int i = 0; i < array_size(traversal->nodes); ++i) {
        char *node = (char *)array_get(traversal->nodes, i);
        char *orig_node;

        // Check if there is no duplicate naming.
        if ((orig_node = smap_retrieve(node_name, node)) != NULL) {
            print_error(node, "Duplicate name '%s' in nodes of traversal '%s'",
                        node, traversal->id);
            print_note(orig_node, "Previously declared here");
            error = 1;
        } else {
            smap_insert(node_name, node, node);
        }

        Node *traversal_node = (Node *)smap_retrieve(info->node_name, node);
        Nodeset *traversal_nodeset =
            (Nodeset *)smap_retrieve(info->nodeset_name, node);

        if (traversal_node) {
            array_append(nodes_expanded, strdup(node));
            smap_insert(node_name_expanded, node, node);
        } else if (traversal_nodeset) {

            // Add every node in the nodeset to the expanded node list
            for (int j = 0; j < array_size(traversal_nodeset->nodes); j++) {
                Node *nodeset_node = array_get(traversal_nodeset->nodes, j);
                if (smap_retrieve(node_name_expanded, nodeset_node->id) ==
                    NULL) {
                    array_append(nodes_expanded, strdup(nodeset_node->id));
                    smap_insert(node_name_expanded, nodeset_node->id,
                                nodeset_node);
                }
            }
        } else {
            print_error(
                node, "Unknown type of node or nodeset '%s' in traversal '%s'",
                node, traversal->id);
            error = 1;
        }
    }

    array_cleanup(traversal->nodes, mem_free);
    traversal->nodes = nodes_expanded;

    smap_free(node_name);
    smap_free(node_name_expanded);

    return error;
}

static int check_pass(Pass *pass, struct Info *info) {

    int error = 0;

    // TODO: check collission of func

    return error;
}

static int check_phase(Phase *phase, struct Info *info, smap_t *phase_order) {

    int error = 0;

    smap_t *pass_name = smap_init(16);
    smap_t *subphase_name = smap_init(16);

    for (int i = 0; i < array_size(phase->passes); ++i) {
        char *pass = (char *)array_get(phase->passes, i);
        char *orig_node;

        // Check if there is no duplicate naming.
        if ((orig_node = smap_retrieve(pass_name, pass)) != NULL) {
            print_error(pass, "Duplicate name '%s' in passes of phase '%s'",
                        pass, phase->id);
            print_note(orig_node, "Previously declared here");
            error = 1;
        } else {
            smap_insert(pass_name, pass, pass);
        }

        Pass *phase_pass = (Pass *)smap_retrieve(info->pass_name, pass);

        Traversal *phase_trav =
            (Traversal *)smap_retrieve(info->traversal_name, pass);

        if (!phase_pass && !phase_trav) {
            print_error(pass,
                        "Unknown type of traversal or pass '%s' in phase '%s'",
                        pass, phase->id);
            error = 1;
        }
    }

    for (int i = 0; i < array_size(phase->subphases); ++i) {
        char *subphase = (char *)array_get(phase->subphases, i);
        char *orig_node;

        // Check if there is no duplicate naming.
        if ((orig_node = smap_retrieve(subphase_name, subphase)) != NULL) {
            print_error(subphase,
                        "Duplicate subphase '%s' in subphases of phase '%s'",
                        subphase, phase->id);
            print_note(orig_node, "Previously declared here");
            error = 1;
        } else {
            smap_insert(subphase_name, subphase, subphase);
        }

        Phase *phase_subphase =
            (Phase *)smap_retrieve(info->phase_name, subphase);

        // Subphase does not exist at all
        if (!phase_subphase) {
            print_error(subphase,
                        "Unknown type of subphase '%s' in phase '%s'",
                        subphase, phase->id);
            error = 1;
        } else {

            // Subphase does exist, but is used before it is declared
            if (smap_retrieve(phase_order, subphase) == NULL) {
                print_error(subphase,
                            "Phase '%s' used as subphase before declaration",
                            subphase);
                error = 1;
            }
        }
    }

    if (phase->root) {
        if (info->root_phase != NULL) {
            print_error(phase->id, "Double declaration of root phase");
            print_note(info->root_phase->id, "Previously declared here");
            error = 1;
        } else {
            info->root_phase = phase;
        }
    }

    smap_insert(phase_order, phase->id, phase);

    smap_free(pass_name);
    smap_free(subphase_name);

    return error;
}

Phase *build_phase_tree(Phase *phase, struct Info *info) {
    Phase *tree_node = mem_alloc(sizeof(Phase));
    tree_node->id = phase->id;
    if (phase->info)
        tree_node->info = phase->info;
    else
        tree_node->info = NULL;
    tree_node->cycle = phase->cycle;

    tree_node->type = phase->type;

    if (phase->type == PH_subphases) {

        tree_node->subphases = array_init(32);

        for (int i = 0; i < array_size(phase->subphases); i++) {
            char *subphase_name = array_get(phase->subphases, i);
            Phase *subphase = smap_retrieve(info->phase_name, subphase_name);

            Phase *subphase_tree = build_phase_tree(subphase, info);

            array_append(tree_node->subphases, subphase_tree);
        }
    } else {

        tree_node->passes = array_init(32);

        for (int i = 0; i < array_size(phase->passes); i++) {
            PhaseLeaf *leaf = mem_alloc(sizeof(PhaseLeaf));

            char *pass_name = array_get(phase->passes, i);
            Traversal *trav = smap_retrieve(info->traversal_name, pass_name);

            if (!trav) {
                Pass *pass = smap_retrieve(info->pass_name, pass_name);
                leaf->type = PL_pass;
                leaf->value.pass = pass;

            } else {
                leaf->type = PL_traversal;
                leaf->value.traversal = trav;
            }
            array_append(tree_node->passes, leaf);
        }
    }

    return tree_node;
}

int check_config(Config *config) {

    int success = 0;
    struct Info *info = create_info();
    smap_t *phase_order = smap_init(16);
    Phase *cur_phase;
    bool root_phase_seen = false;
    bool phase_errors = false;

    success += check_nodes(config->nodes, info);
    success += check_nodesets(config->nodesets, info);
    success += check_enums(config->enums, info);
    success += check_traversals(config->traversals, info);
    success += check_phases(config->phases, info);
    success += check_passes(config->passes, info);

    for (int i = 0; i < array_size(config->nodes); ++i) {
        success += check_node(array_get(config->nodes, i), info);
    }

    for (int i = 0; i < array_size(config->nodesets); ++i) {
        success += check_nodeset(array_get(config->nodesets, i), info);
    }

    for (int i = 0; i < array_size(config->enums); ++i) {
        success += check_enum(array_get(config->enums, i), info);
    }

    for (int i = 0; i < array_size(config->traversals); ++i) {
        success += check_traversal(array_get(config->traversals, i), info);
    }
    for (int i = 0; i < array_size(config->passes); ++i) {
        success += check_pass(array_get(config->passes, i), info);
    }

    for (int i = 0; i < array_size(config->phases); ++i) {
        cur_phase = array_get(config->phases, i);

        // Don't give the warning when we encounter another phase
        // which is declared as root, since it gives an error anyway
        if (root_phase_seen && !cur_phase->root)
            print_warning(cur_phase->id, "Phase is unreachable");

        if (cur_phase->root)
            root_phase_seen = true;

        int res = check_phase(cur_phase, info, phase_order);
        success += res;
        if (res)
            phase_errors = true;
    }

    if (info->root_node == NULL) {
        print_error_no_loc("No root node specified.");
        success++;
    } else {
        config->root_node = info->root_node;
    }

    if (info->root_phase == NULL) {
        print_error_no_loc("No root phase specified.");
        success++;
    } else {
        if (!phase_errors) {
            Phase *tree = build_phase_tree(info->root_phase, info);
            config->phase_tree = tree;
        } else {
            config->phase_tree = NULL;
        }
    }

    smap_free(phase_order);
    free_info(info);

    return success;
}

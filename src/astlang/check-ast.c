#include <stdbool.h>
#include <stdio.h>

#include "array.h"
#include "ast.h"
#include "check-ast.h"
#include "memory.h"
#include "print.h"
#include "smap.h"

struct Info {
    smap_t *enum_name;
    smap_t *enum_prefix;
    smap_t *node_name;
    smap_t *nodeset_name;
    smap_t *traversal_name;
    smap_t *phase_name;
    smap_t *pass_name;
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
    struct Enum *enum_orig;
    struct Node *node_orig;
    struct Nodeset *nodeset_orig;
    struct Traversal *traversal_orig;
    struct Phase *phase_orig;
    struct Pass *pass_orig;

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
        struct Enum *cur_enum = (struct Enum *)array_get(enums, i);
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
        struct Node *cur_node = (struct Node *)array_get(nodes, i);
        void *orig_def;

        if ((orig_def = check_name_exists(info, cur_node->id))) {
            print_error(cur_node->id, "Redefinition of name '%s'",
                        cur_node->id);
            print_note(orig_def, "Previously declared here");
            error = 1;
        } else {
            smap_insert(info->node_name, cur_node->id, cur_node);
        }
    }
    return error;
}

static int check_nodesets(array *nodesets, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(nodesets); ++i) {
        struct Nodeset *cur_nodeset = (struct Nodeset *)array_get(nodesets, i);
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
        struct Phase *cur_phase = (struct Phase *)array_get(phases, i);
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
        struct Pass *cur_pass = (struct Pass *)array_get(passes, i);
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
        struct Traversal *cur_traversal =
            (struct Traversal *)array_get(traversals, i);
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

static int check_mandatory_phase(struct MandatoryPhase *phase,
                                 struct Info *info) {
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

        struct PhaseRange *phase_range = phase->value.range;
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

static int check_node(struct Node *node, struct Info *info) {
    int error = 0;

    smap_t *child_name = smap_init(16);

    if (node->children) {
        for (int i = 0; i < array_size(node->children); ++i) {
            struct Child *child = (struct Child *)array_get(node->children, i);
            struct Child *orig_child;

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

            struct Node *child_node =
                (struct Node *)smap_retrieve(info->node_name, child->type);
            struct Nodeset *child_nodeset = (struct Nodeset *)smap_retrieve(
                info->nodeset_name, child->type);

            if (!child_node && !child_nodeset) {
                print_error(child->type,
                            "Unknown type '%s' of child '%s' of node '%s'",
                            child->type, child->id, node->id);
                error = 1;
            } else {
                child->node = child_node;
                child->nodeset = child_nodeset;
            }


            if (!child->mandatory_phases) {
                for (int i = 0; i < array_size(child->mandatory_phases); ++i) {
                    struct MandatoryPhase *phase =
                        (struct MandatoryPhase *)array_get(
                            child->mandatory_phases, i);

                    error = check_mandatory_phase(phase, info);
                }
            }
        }
    }

    smap_t *attr_name = smap_init(16);

    if (node->attrs) {
        for (int i = 0; i < array_size(node->attrs); i++) {
            struct Attr *attr = (struct Attr *)array_get(node->attrs, i);
            struct Attr *orig_attr;

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
                struct Node *attr_node = (struct Node *)smap_retrieve(
                    info->node_name, attr->type_id);
                struct Enum *attr_enum = (struct Enum *)smap_retrieve(
                    info->enum_name, attr->type_id);

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

static int check_nodeset(struct Nodeset *nodeset, struct Info *info) {
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

        struct Node *nodeset_node =
            (struct Node *)smap_retrieve(info->node_name, node);

        struct Nodeset *nodeset_nodeset =
            (struct Nodeset *)smap_retrieve(info->nodeset_name, node);

        // TODO: create tests for this
        if (nodeset_nodeset) {
            print_error(node, "Nodeset '%s' contains other nodeset '%s'",
                        nodeset->id, nodeset_nodeset->id);
            error = 1;
        } else if (!nodeset_node) {
            print_error(node, "Unknown type of node '%s' in nodeset '%s'",
                        node, nodeset->id);
            error = 1;
        } else {
            array_set(nodeset->nodes, i, nodeset_node);
        }
    }

    smap_free(node_name);

    return error;
}

static int check_enum(struct Enum *arg_enum, struct Info *info) {

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

static int check_traversal(struct Traversal *traversal, struct Info *info) {

    int error = 0;

    if (traversal->nodes == NULL)
        return 0;

    smap_t *node_name = smap_init(16);

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

        struct Node *traversal_node =
            (struct Node *)smap_retrieve(info->node_name, node);
        struct Nodeset *traversal_nodeset =
            (struct Nodeset *)smap_retrieve(info->nodeset_name, node);

        if (!traversal_node && !traversal_nodeset) {
            print_error(node, "Unknown type of node '%s' in traversal '%s'",
                        node, traversal->id);
            error = 1;
        }
    }

    return error;
}

static int check_pass(struct Pass *pass, struct Info *info) {

    int error = 0;

    if (pass->traversal != NULL) {
        struct Traversal *pass_traversal =
            (struct Traversal *)smap_retrieve(info->traversal_name, pass->traversal);

        if (!pass_traversal) {
            print_error(pass->traversal, "Unknown type of traversal '%s' in pass '%s'",
                        pass->traversal, pass->id);
            error = 1;
        }
    }

    return error;
}

static int check_phase(struct Phase *phase, struct Info *info, smap_t *phase_order, smap_t *phase_used) {

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

        struct Pass *phase_pass =
            (struct Pass *)smap_retrieve(info->pass_name, pass);

        if (!phase_pass) {
            print_error(pass, "Unknown type of pass '%s' in phase '%s'",
                        pass, phase->id);
            error = 1;
        }
    }

    for (int i = 0; i < array_size(phase->subphases); ++i) {
        char *subphase = (char *)array_get(phase->subphases, i);
        char *orig_node;

        // Check if there is no duplicate naming.
        if ((orig_node = smap_retrieve(subphase_name, subphase)) != NULL) {
            print_error(subphase, "Duplicate name '%s' in subphases of phase '%s'",
                        subphase, phase->id);
            print_note(orig_node, "Previously declared here");
            error = 1;
        } else {
            smap_insert(subphase_name, subphase, subphase);
        }

        struct Phase *phase_subphase =
            (struct Phase *)smap_retrieve(info->phase_name, subphase);

        if (!phase_subphase) {
            print_error(subphase, "Unknown type of subphase '%s' in phase '%s'",
                        subphase, phase->id);
            error = 1;
        } else {
            if (smap_retrieve(phase_order, subphase) == NULL) {
                print_error(subphase, "Undeclared type of subphase '%s' in phase '%s'",
                            subphase, phase->id);
                error = 1;
            } else {
                if (smap_retrieve(phase_used, subphase) != NULL) {
                    print_error(subphase, "Double use of subphase '%s' in phase '%s'",
                                subphase, phase->id);
                    error = 1;
                } else {
                    smap_insert(phase_used, subphase, phase);
                }
            }
        }
    }

    return error;
}

int check_config(struct Config *config) {

    int success = 0;
    struct Info *info = create_info();
    smap_t *phase_order = smap_init(16);
    smap_t *phase_used = smap_init(16);
    struct Phase *cur_phase;

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
        cur_phase =  array_get(config->phases, i);
        smap_insert(phase_order, cur_phase->id, &cur_phase);
        success += check_phase(cur_phase, info, phase_order, phase_used);
    }

    smap_free(phase_used);
    smap_free(phase_order);
    free_info(info);

    return success;
}

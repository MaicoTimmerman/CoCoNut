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

static bool check_name_exists(struct Info *info, char *name) {
    if (smap_retrieve(info->enum_name, name) != NULL)
        return true;
    if (smap_retrieve(info->node_name, name) != NULL)
        return true;
    if (smap_retrieve(info->nodeset_name, name) != NULL)
        return true;
    if (smap_retrieve(info->traversal_name, name) != NULL)
        return true;
    if (smap_retrieve(info->phase_name, name) != NULL)
        return true;
    if (smap_retrieve(info->pass_name, name) != NULL)
        return true;
    return false;
}

static int check_enums(array *enums, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(enums); ++i) {
        struct Enum *cur_enum = (struct Enum *)array_get(enums, i);

        if (check_name_exists(info, cur_enum->id)) {
            printf("Redefinition of name: %s\n", cur_enum->id);
            error = 1;
        } else {
            smap_insert(info->enum_name, cur_enum->id, cur_enum);
        }

        if (smap_retrieve(info->enum_prefix, cur_enum->prefix) != NULL) {
            printf("Redefinition of prefix: %s\n", cur_enum->prefix);
            error = 1;
        } else {
            smap_insert(info->enum_prefix, cur_enum->prefix, cur_enum);
        }
    }
    return error;
}

static int check_nodes(array *nodes, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(nodes); ++i) {
        struct Node *cur_node = (struct Node *)array_get(nodes, i);

        if (check_name_exists(info, cur_node->id)) {
            printf("Redefinition of name: %s\n", cur_node->id);
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

        if (check_name_exists(info, cur_nodeset->id)) {
            printf("Redefinition of name: %s\n", cur_nodeset->id);
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
        struct Phase *cur_phase =
            (struct Phase *)array_get(phases, i);

        if (check_name_exists(info, cur_phase->id)) {
            printf("Redefinition of name: %s\n", cur_phase->id);
            error = 1;
        } else {
            smap_insert(info->traversal_name, cur_phase->id,
                        cur_phase);
        }
    }
    return error;
}

static int check_passes(array *passes, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(passes); ++i) {
        struct Pass *cur_pass =
            (struct Pass *)array_get(passes, i);

        if (check_name_exists(info, cur_pass->id)) {
            printf("Redefinition of name: %s\n", cur_pass->id);
            error = 1;
        } else {
            smap_insert(info->pass_name, cur_pass->id,
                        cur_pass);
        }
    }
    return error;
}

static int check_traversals(array *traversals, struct Info *info) {

    int error = 0;

    for (int i = 0; i < array_size(traversals); ++i) {
        struct Traversal *cur_traversal =
            (struct Traversal *)array_get(traversals, i);

        if (check_name_exists(info, cur_traversal->id)) {
            printf("Redefinition of name: %s\n", cur_traversal->id);
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
            printf("Unknown mandatory phase '%s'\n", phase->value.single);
            error = 1;
        }
        break;

    case MP_range:; // Crazy GCC won't allow declaration behind statement.

        struct PhaseRange *phase_range = phase->value.range;
        if (smap_retrieve(info->phase_name, phase_range->start) == NULL) {
            printf("Unknown mandatory phase range start '%s'\n",
                    phase_range->start);
            error = 1;
        }
        if (smap_retrieve(info->phase_name, phase_range->end) == NULL) {
            printf("Unknown mandatory phase range end '%s'\n",
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

    for (int i = 0; i < array_size(node->children); ++i) {
        struct Child *child = (struct Child *)array_get(node->children, i);

        // Check if there is no duplicate naming.
        if (smap_retrieve(child_name, child->id)) {
            printf("Duplicate name '%s' in children of node '%s'\n", child->id,
                   node->id);
            error = 1;
        } else {
            smap_insert(child_name, child->id, child);
        }

        struct Node *child_node =
            (struct Node *)smap_retrieve(info->node_name, child->type);
        struct Nodeset *child_nodeset =
            (struct Nodeset *)smap_retrieve(info->nodeset_name, child->type);

        if (!child_node && !child_nodeset) {
            printf("Unknown type '%s' of child '%s' of node '%s'\n",
                   child->type, child->id, node->id);
            error = 1;
        }

        // Test if there are mandatory phases to be checked, if not go
        // to next child.
        if (!child->mandatory_phases)
            continue;

        for (int i = 0; i < array_size(child->mandatory_phases); ++i) {
            struct MandatoryPhase *phase =
                (struct MandatoryPhase *)array_get(child->mandatory_phases, i);

            error = check_mandatory_phase(phase, info);
        }
    }

    smap_free(child_name);

    return error;
}

static int check_nodeset(struct Nodeset *nodeset, struct Info *info) {
    int error = 0;

    smap_t *node_name = smap_init(16);

    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        char *node = (char *)array_get(nodeset->nodes, i);

        // Check if there is no duplicate naming.
        if (smap_retrieve(node_name, node)) {
            printf("Duplicate name '%s' in nodes of nodeset '%s'\n", node,
                   nodeset->id);
            error = 1;
        } else {
            smap_insert(node_name, node, node);
        }

        struct Node *nodeset_node =
            (struct Node *)smap_retrieve(info->node_name, node);
        struct Nodeset *nodeset_nodeset =
            (struct Nodeset *)smap_retrieve(info->nodeset_name, node);

        if (!nodeset_node && !nodeset_nodeset) {
            printf("Unknown type of node '%s' of nodeset '%s'\n", node,
                   nodeset->id);
            error = 1;
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

        // Check if there is no duplicate naming.
        if (smap_retrieve(value_name, cur_value)) {
            printf("Duplicate name '%s' in values of enum '%s'\n", cur_value,
                   arg_enum->id);
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
        //
        // Check if there is no duplicate naming.
        if (smap_retrieve(node_name, node)) {
            printf("Duplicate name '%s' in nodes of traversal '%s'\n", node,
                   traversal->id);
            error = 1;
        } else {
            smap_insert(node_name, node, node);
        }

        struct Node *traversal_node =
            (struct Node *)smap_retrieve(info->node_name, node);
        struct Nodeset *traversal_nodeset =
            (struct Nodeset *)smap_retrieve(info->nodeset_name, node);

        if (!traversal_node && !traversal_nodeset) {
            printf("Unknown type of node '%s' in traversal '%s'\n", node,
                   traversal->id);
            error = 1;
        }
    }

    return error;
}

static int check_pass(struct Pass *pass, struct Info *info) {

    int error = 0;

    smap_t *traversal_name = smap_init(16);

    for (int i = 0; i < array_size(pass->traversals); ++i) {
        char *traversal = (char *)array_get(pass->traversals, i);
        //
        // Check if there is no duplicate naming.
        if (smap_retrieve(traversal_name, traversal)) {
            printf("Duplicate name '%s' in traversals of pass '%s'\n", traversal,
                   pass->id);
            error = 1;
        } else {
            smap_insert(traversal_name, traversal, traversal);
        }

        struct Traversal *pass_traversal =
            (struct Traversal *)smap_retrieve(info->traversal_name, traversal);

        if (!pass_traversal) {
            printf("Unknown type of traversal '%s' in pass '%s'\n", traversal,
                   pass->id);
            error = 1;
        }
    }

    return error;
}

static int check_phase(struct Phase *phase, struct Info *info) {

    int error = 0;

    smap_t *pass_name = smap_init(16);

    for (int i = 0; i < array_size(phase->passes); ++i) {
        char *pass = (char *)array_get(phase->passes, i);
        //
        // Check if there is no duplicate naming.
        if (smap_retrieve(pass_name, pass)) {
            printf("Duplicate name '%s' in passes of phase '%s'\n", pass,
                   phase->id);
            error = 1;
        } else {
            smap_insert(pass_name, pass, pass);
        }

        struct Pass *phase_pass =
            (struct Pass *)smap_retrieve(info->pass_name, pass);

        if (!phase_pass) {
            printf("Unknown type of pass '%s' in phase '%s'\n", pass,
                   phase->id);
            error = 1;
        }
    }

    return error;
}

int check_config(struct Config *config) {

    int success = 0;
    struct Info *info = create_info();

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
        success += check_phase(array_get(config->phases, i), info);
    }

    free_info(info);

    return success;
}

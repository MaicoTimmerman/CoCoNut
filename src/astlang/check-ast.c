#include <stdbool.h>
#include <stdio.h>

#include "array.h"
#include "ast.h"
#include "check-ast.h"
#include "htable.h"
#include "memory.h"

static struct Info *create_info(void) {

    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));

    info->enum_name = htable_init(32);
    info->enum_prefix = htable_init(32);
    info->node_name = htable_init(32);
    info->nodeset_name = htable_init(32);
    info->traversal_name = htable_init(32);
    info->phase_name = htable_init(32);
    return info;
}

static void free_info(struct Info *info) {
    htable_free(info->enum_name);
    htable_free(info->enum_prefix);
    htable_free(info->node_name);
    htable_free(info->nodeset_name);
    htable_free(info->traversal_name);
    htable_free(info->phase_name);
    mem_free(info);
}

static bool check_name_exists(struct Info *info, char *name) {
    if (htable_retrieve(info->enum_name, name) != NULL)
        return true;
    if (htable_retrieve(info->node_name, name) != NULL)
        return true;
    if (htable_retrieve(info->nodeset_name, name) != NULL)
        return true;
    if (htable_retrieve(info->traversal_name, name) != NULL)
        return true;
    if (htable_retrieve(info->phase_name, name) != NULL)
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
            htable_insert(info->enum_name, cur_enum->id, cur_enum);
        }

        if (htable_retrieve(info->enum_prefix, cur_enum->prefix) != NULL) {
            printf("Redefinition of prefix: %s\n", cur_enum->prefix);
            error = 1;
        } else {
            htable_insert(info->enum_prefix, cur_enum->prefix, cur_enum);
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
            htable_insert(info->node_name, cur_node->id, cur_node);
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
            htable_insert(info->nodeset_name, cur_nodeset->id, cur_nodeset);
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
            htable_insert(info->traversal_name, cur_traversal->id,
                          cur_traversal);
        }
    }
    return error;
}

static int check_mandatory_phase(struct MandatoryPhase *phase, struct Info *info) {
    int error = 0;
    // TODO
    return error;
}

static int check_node(struct Node *node, struct Info *info) {
    int error = 0;

    htable_t *child_name = htable_init(16);

    for (int i = 0; i < array_size(node->children); ++i) {
        struct Child *child = (struct Child*)array_get(node->children, i);

        // Check if there is no duplicate naming.
        if (htable_retrieve(child_name, child->id)) {
            printf("Duplicate name '%s' in children of node '%s'\n",
                    child->id, node->id);
            error = 1;
        } else {
            htable_insert(child_name, child->id, child);
        }

        struct Node *child_node =
            (struct Node*)htable_retrieve(info->node_name, child->type);
        struct Nodeset *child_nodeset =
            (struct Nodeset*)htable_retrieve(info->nodeset_name, child->type);

        if (!child_node && !child_nodeset) {
                printf("Unknown type '%s' of child '%s' of node '%s'\n",
                        child->type, child->id, node->id);
                error = 1;
        }

        // Test if there are mandatory phases to be checked, if not go
        // to next child.
        if (!child->mandatory_phases) continue;

        for (int i = 0; i < array_size(child->mandatory_phases); ++i) {
            struct MandatoryPhase *phase =
                (struct MandatoryPhase*) array_get(child->mandatory_phases, i);

            error = check_mandatory_phase(phase, info);
        }
    }

    htable_free(child_name);

    return error;
}

static int check_nodeset(struct Nodeset *nodeset, struct Info *info) {
    int error = 0;

    htable_t *node_name = htable_init(16);

    for (int i = 0; i < array_size(nodeset->nodes); ++i) {
        char *node = (char*)array_get(nodeset->nodes, i);

        // Check if there is no duplicate naming.
        if (htable_retrieve(node_name, node)) {
            printf("Duplicate name '%s' in nodes of nodeset '%s'\n",
                    node, nodeset->id);
            error = 1;
        } else {
            htable_insert(node_name, node, node);
        }

        struct Node *nodeset_node =
            (struct Node*)htable_retrieve(info->node_name, node);
        struct Nodeset *nodeset_nodeset =
            (struct Nodeset*)htable_retrieve(info->nodeset_name, node);

        if (!nodeset_node && !nodeset_nodeset) {
                printf("Unknown type of node '%s' of nodeset '%s'\n",
                        node, nodeset->id);
                error = 1;
        }
    }

    htable_free(node_name);

    return error;
}

static int check_enum(struct Enum *arg_enum, struct Info *info) {

    int error = 0;
    htable_t *value_name = htable_init(16);

    for (int i = 0; i < array_size(arg_enum->values); ++i) {
        char *cur_value = (char*)array_get(arg_enum->values, i);

        // Check if there is no duplicate naming.
        if (htable_retrieve(value_name, cur_value)) {
            printf("Duplicate name '%s' in values of enum '%s'\n",
                    cur_value, arg_enum->id);
            error = 1;
        } else {
            htable_insert(value_name, cur_value, cur_value);
        }
    }

    htable_free(value_name);
    return error;
}

static int check_traversal(struct Traversal *traversal, struct Info *info) {

    int error = 0;
    htable_t *node_name = htable_init(16);

    for (int i = 0; i < array_size(traversal->nodes); ++i) {
        char *node = (char*)array_get(traversal->nodes, i);
        //
        // Check if there is no duplicate naming.
        if (htable_retrieve(node_name, node)) {
            printf("Duplicate name '%s' in nodes of traversal '%s'\n",
                    node, traversal->id);
            error = 1;
        } else {
            htable_insert(node_name, node, node);
        }

        struct Node *traversal_node =
            (struct Node*)htable_retrieve(info->node_name, node);
        struct Nodeset *traversal_nodeset =
            (struct Nodeset*)htable_retrieve(info->nodeset_name, node);

        if (!traversal_node && !traversal_nodeset) {
                printf("Unknown type of node '%s' in traversal '%s'\n",
                        node, traversal->id);
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
    /* success += check_phases(config->traversals, info); */

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
    for (int i = 0; i < array_size(config->traversals); ++i) {
        success += check_traversal(array_get(config->traversals, i), info);
    }

    free_info(info);

    return success;
}

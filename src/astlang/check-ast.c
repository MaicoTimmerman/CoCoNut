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

int check_config(struct Config *config) {

    int success = 0;
    struct Info *info = create_info();

    success += check_enums(config->enums, info);
    success += check_nodes(config->nodes, info);
    success += check_nodesets(config->nodesets, info);
    success += check_traversals(config->traversals, info);
    /* success += check_phases(config->traversals, info); */

    /* success += check_enums(config->enums, info); */
    /* success += check_nodes(config->enums, info); */
    /* success += check_nodesets(config->enums, info); */
    /* success += check_traversals(config->enums, info); */

    free_info(info);

    return success;
}

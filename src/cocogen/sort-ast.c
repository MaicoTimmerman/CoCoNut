#include "cocogen/ast.h"
#include <stdio.h>
#include <string.h>

#include "lib/array.h"
#include "lib/memory.h"
#include "lib/smap.h"

static int compare_nodes(const void *n1, const void *n2) {
    return strcmp((*((Node **)n1))->id, ((*(Node **)n2))->id);
}

static int compare_nodesets(const void *n1, const void *n2) {
    return strcmp((*((Nodeset **)n1))->id, ((*(Nodeset **)n2))->id);
}

static int compare_traversals(const void *n1, const void *n2) {
    return strcmp((*((Traversal **)n1))->id, ((*(Traversal **)n2))->id);
}

static int compare_enums(const void *n1, const void *n2) {
    return strcmp((*((Enum **)n1))->id, ((*(Enum **)n2))->id);
}

static int compare_phases(const void *n1, const void *n2) {
    return strcmp((*((Phase **)n1))->id, ((*(Phase **)n2))->id);
}

static int compare_passes(const void *n1, const void *n2) {
    return strcmp((*((Pass **)n1))->id, ((*(Pass **)n2))->id);
}

static int compare_children(const void *n1, const void *n2) {
    return strcmp((*((Child **)n1))->id, ((*(Child **)n2))->id);
}

static int compare_attributes(const void *n1, const void *n2) {
    return strcmp((*((Attr **)n1))->id, ((*(Attr **)n2))->id);
}

static int compare_strings(const void *n1, const void *n2) {
    return strcmp(*((char **)n1), *((char **)n2));
}

static void sort_node(Node *n) {
    if (n->attrs)
        array_sort(n->attrs, compare_attributes);
    if (n->children)
        array_sort(n->children, compare_children);
}

static void sort_nodesets(Nodeset *n) {
    array_sort(n->nodes, compare_nodesets);
}

static void sort_traversals(Traversal *n) {
    if (n->nodes)
        array_sort(n->nodes, compare_strings);
}

static void sort_enums(Enum *n) {
    array_sort(n->values, compare_strings);
}

void sort_config(Config *config) {
    array_sort(config->nodes, compare_nodes);
    array_sort(config->nodesets, compare_nodesets);
    array_sort(config->traversals, compare_traversals);
    array_sort(config->enums, compare_enums);
    array_sort(config->phases, compare_phases);
    array_sort(config->passes, compare_passes);

    for (int i = 0; i < array_size(config->nodes); i++) {
        sort_node(array_get(config->nodes, i));
    }
    for (int i = 0; i < array_size(config->nodesets); i++) {
        sort_nodesets(array_get(config->nodesets, i));
    }
    for (int i = 0; i < array_size(config->traversals); i++) {
        sort_traversals(array_get(config->traversals, i));
    }
    for (int i = 0; i < array_size(config->enums); i++) {
        sort_enums(array_get(config->enums, i));
    }
}

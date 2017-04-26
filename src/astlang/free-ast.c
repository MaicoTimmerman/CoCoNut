#include "array.h"
#include "ast.h"
#include "memory.h"
#include <stdio.h>

static void free_phase(void *p) {
    struct Phase *phase = p;

    switch (phase->type) {
    case PH_subphases:
        array_cleanup(phase->subphases, mem_free);
        break;
    case PH_passes:
        array_cleanup(phase->passes, mem_free);
        break;
    }

    mem_free(phase->id);
    mem_free(phase);
}

static void free_pass(void *p) {
    struct Pass *pass = p;

    mem_free(pass->id);
    mem_free(pass);
}

static void free_traversal(void *p) {
    struct Traversal *traversal = p;
    if (traversal->nodes != NULL)
        array_cleanup(traversal->nodes, mem_free);

    mem_free(traversal->id);
    mem_free(traversal);
}

static void free_enum(void *p) {
    struct Enum *attr_enum = p;
    if (attr_enum->values != NULL)
        array_cleanup(attr_enum->values, mem_free);

    mem_free(attr_enum->id);
    mem_free(attr_enum->prefix);
    mem_free(attr_enum);
}

static void free_nodeset(void *p) {
    struct Nodeset *nodeset = p;
    if (nodeset->nodes != NULL)
        array_cleanup(nodeset->nodes, NULL);

    mem_free(nodeset->id);
    mem_free(nodeset);
}

static void free_mandatory(void *p) {
    struct MandatoryPhase *ph = p;

    if (ph->type == MP_single) {
        mem_free(ph->value.single);
    } else {
        mem_free(ph->value.range->start);
        mem_free(ph->value.range->end);
        mem_free(ph->value.range);
    }

    mem_free(ph);
}

static void free_child(void *p) {
    struct Child *c = p;
    if (c->mandatory_phases != NULL)
        array_cleanup(c->mandatory_phases, free_mandatory);

    mem_free(c->id);
    if (c->type != NULL)
        mem_free(c->type);

    mem_free(c);
}

static void free_attr(void *p) {
    struct Attr *a = p;
    mem_free(a->id);
    if (a->type_id != NULL)
        mem_free(a->type_id);

    if (a->default_value != NULL) {
        if (a->default_value->type == AV_string ||
            a->default_value->type == AV_id)
            mem_free(a->default_value->value.string_value);

        mem_free(a->default_value);
    }

    mem_free(a);
}

static void free_node(void *p) {
    struct Node *node = p;

    if (node->children != NULL)
        array_cleanup(node->children, free_child);

    if (node->attrs != NULL)
        array_cleanup(node->attrs, free_attr);

    mem_free(node->id);
    mem_free(node);
}

void free_config(struct Config *config) {
    array_cleanup(config->phases, free_phase);
    array_cleanup(config->passes, free_pass);
    array_cleanup(config->traversals, free_traversal);
    array_cleanup(config->enums, free_enum);
    array_cleanup(config->nodesets, free_nodeset);
    array_cleanup(config->nodes, free_node);

    mem_free(config);
}

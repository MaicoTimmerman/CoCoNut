#include "array.h"
#include "ast-internal.h"
#include "memory.h"
#include <stdio.h>

static void freePhase(void *p) {
    struct Phase *phase = p;
}

static void freeTraversal(void *p) {
    struct Traversal *traversal = p;
    if (traversal->nodes != NULL)
        array_cleanup(traversal->nodes, mem_free);

    mem_free(traversal->id);
    mem_free(traversal);
}

static void freeEnum(void *p) {
    struct Enum *attr_enum = p;
    if (attr_enum->values != NULL)
        array_cleanup(attr_enum->values, mem_free);

    mem_free(attr_enum->id);
    mem_free(attr_enum);
}

static void freeNodeset(void *p) {
    struct Nodeset *nodeset = p;
    if (nodeset->nodes != NULL)
        array_cleanup(nodeset->nodes, mem_free);

    mem_free(nodeset->id);
    mem_free(nodeset);
}

static void freeMandatory(void *p) {
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

static void freeChild(void *p) {
    struct Child *c = p;
    if (c->mandatory_phases != NULL)
        array_cleanup(c->mandatory_phases, freeMandatory);

    mem_free(c->id);
    if (c->type != NULL)
        mem_free(c->type);

    mem_free(c);
}

static void freeAttr(void *p) {
    struct Attr *a = p;
    mem_free(a->id);
    if (a->type_id != NULL)
        mem_free(a->type_id);

    if (a->default_value != NULL) {
        mem_free(a->default_value->value);
        mem_free(a->default_value);
    }

    mem_free(a);
}

static void freeFlag(void *p) {
    struct Flag *f = p;
    mem_free(f->id);
    mem_free(f);
}

static void freeNode(void *p) {
    struct Node *node = p;

    if (node->children != NULL)
        array_cleanup(node->children, freeChild);

    if (node->attrs != NULL)
        array_cleanup(node->attrs, freeAttr);

    if (node->flags != NULL)
        array_cleanup(node->flags, freeFlag);

    mem_free(node->id);
    mem_free(node);
}

void freeConfigAST(struct Config *config) {
    array_cleanup(config->phases, freePhase);
    array_cleanup(config->traversals, freeTraversal);
    array_cleanup(config->enums, freeEnum);
    array_cleanup(config->nodesets, freeNodeset);
    array_cleanup(config->nodes, freeNode);

    mem_free(config);
}

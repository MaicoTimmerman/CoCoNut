#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "ast-internal.h"
#include "memory.h"

array *create_array(void) {
    return array_init(32);
}

struct Config *create_config(array *phases, array *traversals, array *enums,
                             array *nodesets, array *nodes) {

    struct Config *c = mem_alloc(sizeof(struct Config));
    c->phases = phases;
    c->traversals = traversals;
    c->enums = enums;
    c->nodesets = nodesets;
    c->nodes = nodes;
    return c;
}

struct Phase *create_phase(void) {

    struct Phase *p = mem_alloc(sizeof(struct Phase));
    return p;
}

struct Traversal *create_traversal(char *id, array *nodes) {

    struct Traversal *t = mem_alloc(sizeof(struct Traversal));
    t->id = id;
    t->nodes = nodes;
    return t;
}

struct Enum *create_enum(char *id, char *prefix, array *values) {

    struct Enum *e = mem_alloc(sizeof(struct Enum));

    e->id = id;
    e->values = values;
    e->prefix = prefix;

    return e;
}

struct Nodeset *create_nodeset(char *id, array *nodes) {

    struct Nodeset *n = mem_alloc(sizeof(struct Nodeset));
    n->id = id;
    n->nodes = nodes;

    return n;
}

struct Node *create_node(char *id, struct Node *nodebody) {
    nodebody->id = id;
    return nodebody;
}

struct Node *create_nodebody(array *children, array *attrs, array *flags) {
    struct Node *n = mem_alloc(sizeof(struct Node));
    n->children = children;
    n->attrs = attrs;
    n->flags = flags;

    return n;
}

struct Child *create_child(int construct, int mandatory,
                           array *mandatory_phases, char *id, char *type) {

    struct Child *c = mem_alloc(sizeof(struct Child));
    c->construct = construct;
    c->mandatory = mandatory;
    c->mandatory_phases = mandatory_phases;
    c->id = id;
    c->type = type;
    return c;
}

struct MandatoryPhase *create_mandatory_singlephase(char *phase,
                                                    int negation) {

    struct MandatoryPhase *p = mem_alloc(sizeof(struct MandatoryPhase));
    p->value.single = phase;
    p->type = MP_single;
    p->negation = negation;
    return p;
}

struct MandatoryPhase *
create_mandatory_phaserange(char *phase_start, char *phase_end, int negation) {

    struct MandatoryPhase *p = mem_alloc(sizeof(struct MandatoryPhase));
    struct PhaseRange *range = mem_alloc(sizeof(struct PhaseRange));
    range->start = phase_start;
    range->end = phase_end;
    p->value.range = range;
    p->type = MP_range;
    p->negation = negation;
    return p;
}

struct Attr *create_attr(struct Attr *attrhead,
                         struct AttrValue *default_value) {
    attrhead->default_value = default_value;
    return attrhead;
}

struct Attr *create_attrhead_primitive(int construct, enum AttrType type,
                                       char *id) {

    struct Attr *a = mem_alloc(sizeof(struct Attr));
    a->construct = construct;
    a->type = type;
    a->type_id = NULL;
    a->id = id;
    return a;
}

struct Attr *create_attrhead_idtype(int construct, char *type, char *id) {

    struct Attr *a = mem_alloc(sizeof(struct Attr));
    a->construct = construct;
    a->type = AT_link_or_enum;
    a->type_id = type;
    a->id = id;
    return a;
}

struct AttrValue *create_attrval_string(char *value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_string;

    // Calculate the position of the quotes remove it by overwriting it with
    // the null char '\0'.
    size_t len = strlen(value);
    value[len - 1] = '\0';

    // strdup creates a new pointer, thus free the old value.
    v->value.string_value = strdup(value + 1);
    mem_free(value);
    return v;
}

struct AttrValue *create_attrval_int(int64_t value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_int;
    v->value.int_value = value;
    return v;
}

struct AttrValue *create_attrval_uint(uint64_t value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_uint;
    v->value.uint_value = value;
    return v;
}

struct AttrValue *create_attrval_float(float value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_float;
    v->value.float_value = value;
    return v;
}

struct AttrValue *create_attrval_double(double value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_double;
    v->value.double_value = value;
    return v;
}

struct AttrValue *create_attrval_bool(bool value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_bool;
    v->value.bool_value = value;
    return v;
}

struct AttrValue *create_attrval_id(char *id) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_id;
    v->value.string_value = id;
    return v;
}

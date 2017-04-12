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
    t->id = strdup(id);
    t->nodes = nodes;
    return t;
}

struct Enum *create_enum(char *id, array *values) {

    struct Enum *e = mem_alloc(sizeof(struct Enum));
    e->id = strdup(id);
    e->values = values;

    return e;
}

struct Nodeset *create_nodeset(char *id, array *nodes) {

    struct Nodeset *n = mem_alloc(sizeof(struct Nodeset));
    n->id = strdup(id);
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
    c->id = strdup(id);
    c->type = type;
    return c;
}

struct MandatoryPhase *create_mandatory_singlephase(char *phase) {

    struct MandatoryPhase *p = mem_alloc(sizeof(struct MandatoryPhase));
    p->value.single = strdup(phase);
    p->type = MP_single;
    return p;
}

struct MandatoryPhase *create_mandatory_phaserange(char *phase_start,
                                                   char *phase_end) {

    struct MandatoryPhase *p = mem_alloc(sizeof(struct MandatoryPhase));
    struct PhaseRange *range = mem_alloc(sizeof(struct PhaseRange));
    range->start = strdup(phase_start);
    range->end = strdup(phase_end);
    p->value.range = range;
    p->type = MP_range;
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
    a->id = strdup(id);
    return a;
}

struct Attr *create_attrhead_idtype(int construct, char *type, char *id) {

    struct Attr *a = mem_alloc(sizeof(struct Attr));
    a->construct = construct;
    a->type = AT_link_or_enum;
    a->type_id = strdup(type);
    a->id = strdup(id);
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
    v->value = strdup(value + 1);
    mem_free(value);
    return v;
}

struct AttrValue *create_attrval_char(char *value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_char;

    char *l = mem_alloc(sizeof(char));
    *l = value[1];
    v->value = l;

    mem_free(value);
    return v;
}

struct AttrValue *create_attrval_int(long long value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_int;

    long long *l = mem_alloc(sizeof(long long));
    *l = value;
    v->value = l;
    return v;
}

struct AttrValue *create_attrval_float(long double value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_float;

    long double *f = mem_alloc(sizeof(long double));
    *f = value;
    v->value = f;
    return v;
}

struct AttrValue *create_attrval_id(char *id) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_id;
    v->value = strdup(id);
    return v;
}
struct Flag *create_flag(int construct, char *id, int default_value) {

    struct Flag *f = mem_alloc(sizeof(struct Flag));
    f->construct = construct;
    f->id = id;
    f->default_value = default_value;
    return f;
}

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astgen/ast.h"
#include "astgen/create-ast.h"

#include "lib/array.h"
#include "lib/memory.h"

extern struct ParserLocation yy_parser_location;

static struct NodeCommonInfo *create_commoninfo() {
    return NULL;
}

struct Config *create_config(array *phases, array *passes, array *traversals,
                             array *enums, array *nodesets, array *nodes) {

    struct Config *c = mem_alloc(sizeof(struct Config));
    c->phases = phases;
    c->passes = passes;
    c->traversals = traversals;
    c->enums = enums;
    c->nodesets = nodesets;
    c->nodes = nodes;

    c->common_info = create_commoninfo();
    return c;
}

struct Phase *create_phase_header(char *id, bool root, bool cycle) {
    struct Phase *p = mem_alloc(sizeof(struct Phase));
    p->id = id;
    p->info = NULL;
    p->root = root;
    p->cycle = cycle;

    p->common_info = create_commoninfo();
    return p;
}

struct Phase *create_phase(struct Phase *phase_header, array *phases,
                           array *passes) {

    struct Phase *p = phase_header;
    if (phases == NULL) {
        p->type = PH_passes;
    } else {
        p->type = PH_subphases;
    }

    p->subphases = phases;
    p->passes = passes;

    return p;
}

struct Pass *create_pass(char *id, char *func) {
    struct Pass *p = mem_alloc(sizeof(struct Pass));

    p->id = id;
    p->func = func;
    p->info = NULL;

    p->common_info = create_commoninfo();
    return p;
}

struct Traversal *create_traversal(char *id, char *func, array *nodes) {

    struct Traversal *t = mem_alloc(sizeof(struct Traversal));
    t->id = id;
    t->func = func;
    t->info = NULL;
    t->nodes = nodes;

    t->common_info = create_commoninfo();
    return t;
}

struct Enum *create_enum(char *id, char *prefix, array *values) {

    struct Enum *e = mem_alloc(sizeof(struct Enum));

    e->id = id;
    e->values = values;
    e->prefix = prefix;
    e->info = NULL;

    e->common_info = create_commoninfo();
    return e;
}

struct Nodeset *create_nodeset(char *id, array *nodes) {

    struct Nodeset *n = mem_alloc(sizeof(struct Nodeset));
    n->id = id;
    n->nodes = nodes;
    n->root = false;
    n->info = NULL;

    n->common_info = create_commoninfo();
    return n;
}

struct Node *create_node(char *id, struct Node *nodebody) {

    nodebody->id = id;
    nodebody->root = false;
    nodebody->common_info = create_commoninfo();
    return nodebody;
}

struct Node *create_nodebody(array *children, array *attrs) {
    struct Node *n = mem_alloc(sizeof(struct Node));
    n->children = children;
    n->attrs = attrs;
    n->info = NULL;

    n->common_info = create_commoninfo();
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

    c->node = NULL;
    c->nodeset = NULL;

    c->common_info = create_commoninfo();
    return c;
}

struct MandatoryPhase *create_mandatory_singlephase(char *phase,
                                                    int negation) {

    struct MandatoryPhase *p = mem_alloc(sizeof(struct MandatoryPhase));
    p->value.single = phase;
    p->type = MP_single;
    p->negation = negation;

    p->common_info = create_commoninfo();
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

    p->common_info = create_commoninfo();
    return p;
}

struct Attr *create_attr(struct Attr *attrhead,
                         struct AttrValue *default_value) {
    attrhead->default_value = default_value;

    attrhead->common_info = create_commoninfo();
    return attrhead;
}

struct Attr *create_attrhead_primitive(int construct, enum AttrType type,
                                       char *id) {

    struct Attr *a = mem_alloc(sizeof(struct Attr));
    a->construct = construct;
    a->type = type;
    a->type_id = NULL;
    a->id = id;

    a->common_info = create_commoninfo();
    return a;
}

struct Attr *create_attrhead_idtype(int construct, char *type, char *id) {

    struct Attr *a = mem_alloc(sizeof(struct Attr));
    a->construct = construct;
    a->type = AT_link_or_enum;
    a->type_id = type;
    a->id = id;

    a->common_info = create_commoninfo();
    return a;
}

struct AttrValue *create_attrval_string(char *value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_string;

    v->value.string_value = value;
    mem_free(value);

    v->common_info = create_commoninfo();
    return v;
}

struct AttrValue *create_attrval_int(int64_t value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_int;
    v->value.int_value = value;

    v->common_info = create_commoninfo();
    return v;
}

struct AttrValue *create_attrval_uint(uint64_t value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_uint;
    v->value.uint_value = value;

    v->common_info = create_commoninfo();
    return v;
}

struct AttrValue *create_attrval_float(float value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_float;
    v->value.float_value = value;

    v->common_info = create_commoninfo();
    return v;
}

struct AttrValue *create_attrval_double(double value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_double;
    v->value.double_value = value;

    v->common_info = create_commoninfo();
    return v;
}

struct AttrValue *create_attrval_bool(bool value) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_bool;
    v->value.bool_value = value;

    v->common_info = create_commoninfo();
    return v;
}

struct AttrValue *create_attrval_id(char *id) {
    struct AttrValue *v = mem_alloc(sizeof(struct AttrValue));
    v->type = AV_id;
    v->value.string_value = id;

    v->common_info = create_commoninfo();
    return v;
}

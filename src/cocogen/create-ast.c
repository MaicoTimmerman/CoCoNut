#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cocogen/ast.h"
#include "cocogen/create-ast.h"

#include "lib/array.h"
#include "lib/memory.h"

extern ParserLocation yy_parser_location;

static NodeCommonInfo *create_commoninfo() {
    NodeCommonInfo *info = (NodeCommonInfo *)mem_alloc(sizeof(NodeCommonInfo));
    info->hash = NULL;
    return info;
}

Config *create_config(array *phases, array *passes, array *traversals,
                      array *enums, array *nodesets, array *nodes) {

    Config *c = mem_alloc(sizeof(Config));
    c->phases = phases;
    c->passes = passes;
    c->traversals = traversals;
    c->enums = enums;
    c->nodesets = nodesets;
    c->nodes = nodes;

    c->common_info = create_commoninfo();
    return c;
}

Phase *create_phase_header(char *id, bool root, bool cycle) {
    Phase *p = mem_alloc(sizeof(Phase));
    p->id = id;
    p->info = NULL;
    p->root = root;
    p->cycle = cycle;

    p->common_info = create_commoninfo();
    return p;
}

Phase *create_phase(Phase *phase_header, array *phases, array *passes) {

    Phase *p = phase_header;
    if (phases == NULL) {
        p->type = PH_passes;
    } else {
        p->type = PH_subphases;
    }

    p->subphases = phases;
    p->passes = passes;

    return p;
}

Pass *create_pass(char *id, char *func) {
    Pass *p = mem_alloc(sizeof(Pass));

    p->id = id;
    p->func = func;
    p->info = NULL;

    p->common_info = create_commoninfo();
    return p;
}

Traversal *create_traversal(char *id, char *func, array *nodes) {

    Traversal *t = mem_alloc(sizeof(Traversal));
    t->id = id;
    t->func = func;
    t->info = NULL;
    t->nodes = nodes;

    t->common_info = create_commoninfo();
    return t;
}

Enum *create_enum(char *id, char *prefix, array *values) {

    Enum *e = mem_alloc(sizeof(Enum));

    e->id = id;
    e->values = values;
    e->prefix = prefix;
    e->info = NULL;

    e->common_info = create_commoninfo();
    return e;
}

Nodeset *create_nodeset(char *id, array *nodes) {

    Nodeset *n = mem_alloc(sizeof(Nodeset));
    n->id = id;
    n->nodes = nodes;
    n->info = NULL;

    n->common_info = create_commoninfo();
    return n;
}

Node *create_node(char *id, Node *nodebody) {

    nodebody->id = id;
    nodebody->root = false;
    return nodebody;
}

Node *create_nodebody(array *children, array *attrs) {
    Node *n = mem_alloc(sizeof(Node));
    n->children = children;
    n->attrs = attrs;
    n->info = NULL;

    n->common_info = create_commoninfo();
    return n;
}

Child *create_child(int construct, int mandatory, array *mandatory_phases,
                    char *id, char *type) {

    Child *c = mem_alloc(sizeof(Child));
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

MandatoryPhase *create_mandatory_singlephase(char *phase, int negation) {

    MandatoryPhase *p = mem_alloc(sizeof(MandatoryPhase));
    p->value.single = phase;
    p->type = MP_single;
    p->negation = negation;

    p->common_info = create_commoninfo();
    return p;
}

MandatoryPhase *create_mandatory_phaserange(char *phase_start, char *phase_end,
                                            int negation) {

    MandatoryPhase *p = mem_alloc(sizeof(MandatoryPhase));
    PhaseRange *range = mem_alloc(sizeof(PhaseRange));
    range->start = phase_start;
    range->end = phase_end;
    p->value.range = range;
    p->type = MP_range;
    p->negation = negation;

    p->common_info = create_commoninfo();
    return p;
}

Attr *create_attr(Attr *a, AttrValue *default_value, int construct) {
    a->default_value = default_value;
    a->construct = construct;
    return a;
}

Attr *create_attrhead_primitive(enum AttrType type, char *id) {

    Attr *a = mem_alloc(sizeof(Attr));
    a->type = type;
    a->type_id = NULL;
    a->id = id;

    a->common_info = create_commoninfo();
    return a;
}

Attr *create_attrhead_idtype(char *type, char *id) {

    Attr *a = mem_alloc(sizeof(Attr));
    a->type = AT_link_or_enum;
    a->type_id = type;
    a->id = id;

    a->common_info = create_commoninfo();
    return a;
}

AttrValue *create_attrval_string(char *value) {
    AttrValue *v = mem_alloc(sizeof(AttrValue));
    v->type = AV_string;

    v->value.string_value = value;

    v->common_info = create_commoninfo();
    return v;
}

AttrValue *create_attrval_int(int64_t value) {
    AttrValue *v = mem_alloc(sizeof(AttrValue));
    v->type = AV_int;
    v->value.int_value = value;

    v->common_info = create_commoninfo();
    return v;
}

AttrValue *create_attrval_uint(uint64_t value) {
    AttrValue *v = mem_alloc(sizeof(AttrValue));
    v->type = AV_uint;
    v->value.uint_value = value;

    v->common_info = create_commoninfo();
    return v;
}

AttrValue *create_attrval_float(float value) {
    AttrValue *v = mem_alloc(sizeof(AttrValue));
    v->type = AV_float;
    v->value.float_value = value;

    v->common_info = create_commoninfo();
    return v;
}

AttrValue *create_attrval_double(double value) {
    AttrValue *v = mem_alloc(sizeof(AttrValue));
    v->type = AV_double;
    v->value.double_value = value;

    v->common_info = create_commoninfo();
    return v;
}

AttrValue *create_attrval_bool(bool value) {
    AttrValue *v = mem_alloc(sizeof(AttrValue));
    v->type = AV_bool;
    v->value.bool_value = value;

    v->common_info = create_commoninfo();
    return v;
}

AttrValue *create_attrval_id(char *id) {
    AttrValue *v = mem_alloc(sizeof(AttrValue));
    v->type = AV_id;
    v->value.string_value = id;

    v->common_info = create_commoninfo();
    return v;
}

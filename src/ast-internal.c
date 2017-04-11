#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "ast-internal.h"

struct Config {
    array *phases;
    array *traversals;
    array *attr_enums;
    array *nodesets;
    array *nodes;
};

struct Phase {
};

struct Traversal {
    char* id;

    // Array of strings (after parsing) or struct node's
    array *nodes;
};

struct Enum {
    char* id;

    array *values;
};

struct Nodeset {
    char* id;

    // Array of strings (after parsing) or struct node's
    array *nodes;
};

struct Node {
    char* id;

    array *children;
    array *attrs;
    array *flags;
};

struct Child {
    int construct;
    int mandatory;
    array *mandatory_phases;
    char* id;
    char* type;

    union {
        struct Node* node;
        struct Nodeset* nodeset;
    } node;

    enum NodeType nodetype;
};

struct PhaseRange {
    char *start;
    char *end;
};

struct MandatoryPhase {
    enum MandatoryPhaseType type;
    union {
        struct PhaseRange* range;
        char *single;
    } value;
};

struct Attr {
    int construct;
    enum AttrType type;
    char *type_id;
    char* id;
    struct AttrValue *default_value;
};

enum AttrValueType { AV_string, AV_int, AV_float, AV_id };

struct AttrValue {
    enum AttrValueType type;
    void* value;
};

struct Flag {
    int construct;
    char* id;
    int has_default_value;
    int default_value;
};

array *create_array(void) {
    return array_init(32);
}

struct Config* create_config(array *phases, array *traversals,
        array *attr_enums, array *nodesets, array *nodes) {

    struct Config *c = malloc(sizeof(struct Config));
    c->phases = phases;
    c->traversals = traversals;
    c->attr_enums = attr_enums;
    c->nodesets = nodesets;
    c->nodes = nodes;
    return c;
}

struct Phase* create_phase(void)  {

    struct Phase *p = malloc(sizeof(struct Phase));
    return p;
}

struct Traversal* create_traversal(char* id, array *nodes) {

    struct Traversal *t = malloc(sizeof(struct Traversal));
    t->id = strdup(id);
    t->nodes = nodes;
    return t;
}

struct Enum* create_enum(char* id, array *values) {

    struct Enum *e = malloc(sizeof(struct Enum));
    e->id = strdup(id);
    e->values = values;

    return e;
}

struct Nodeset* create_nodeset(char* id, array *nodes) {

    struct Nodeset *n = malloc(sizeof(struct Nodeset));
    n->id = strdup(id);
    n->nodes = nodes;

    return n;
}

struct Node* create_node(char *id, struct Node *nodebody) {
    nodebody->id = id;
    return nodebody;
}

struct Node* create_nodebody(array *children, array *attrs, array *flags) {
    struct Node *n = malloc(sizeof(struct Node));
    n->children = children;
    n->attrs = attrs;
    n->flags = flags;

    return n;
}

struct Child* create_child(int construct, int mandatory,
        array *mandatory_phases, char *id, char *type) {

    struct Child *c = malloc(sizeof(struct Child));
    c->construct = construct;
    c->mandatory = mandatory;
    c->mandatory_phases = mandatory_phases;
    c->id = strdup(id);
    c->type = type;
    return c;
}

struct MandatoryPhase* create_mandatory_singlephase(char* phase) {

    struct MandatoryPhase *p = malloc(sizeof(struct MandatoryPhase));
    p->value.single = strdup(phase);
    p->type = MP_single;
    return p;
}

struct MandatoryPhase* create_mandatory_phaserange(
        char* phase_start, char* phase_end) {

    struct MandatoryPhase *p = malloc(sizeof(struct MandatoryPhase));
    struct PhaseRange *range = malloc(sizeof(struct PhaseRange));
    range->start = strdup(phase_start);
    range->end = strdup(phase_end);
    p->value.range = range;
    p->type = MP_range;
    return p;
}

struct Attr* create_attr(struct Attr *attrhead, struct AttrValue *default_value) {
    attrhead->default_value = default_value;
    return attrhead;
}

struct Attr* create_attrhead_primitive(int construct, enum AttrType type,
        char *id) {

    struct Attr* a = malloc(sizeof(struct Attr));
    a->construct = construct;
    a->type = type;
    a->type_id = NULL;
    a->id = strdup(id);
    return a;
}

struct Attr* create_attrhead_idtype(int construct, char *type, char *id) {

    struct Attr *a = malloc(sizeof(struct Attr));
    a->construct = construct;
    a->type = AT_link_or_enum;
    a->type_id = strdup(type);
    a->id = strdup(id);
    return a;
}

struct AttrValue* create_attrval_string(char* value) {
    struct AttrValue *v = malloc(sizeof(struct AttrValue));
    v->type = AV_string;
    v->value = strdup(value);
    return v;
}

struct AttrValue* create_attrval_int(long long value) {
    struct AttrValue *v = malloc(sizeof(struct AttrValue));
    v->type = AV_int;

    long long *l = malloc(sizeof(long long));
    *l = value;
    v->value = l;
    return v;
}

struct AttrValue* create_attrval_float(long double value) {
    struct AttrValue *v = malloc(sizeof(struct AttrValue));
    v->type = AV_float;

    long double *f = malloc(sizeof(long double));
    *f = value;
    v->value = f;
    return v;
}

struct AttrValue* create_attrval_id(char *id) {
    struct AttrValue *v = malloc(sizeof(struct AttrValue));
    v->type = AV_id;
    v->value = strdup(id);
    return v;
}
struct Flag* create_flag(int construct, char *id,
        int has_default_value, int default_value) {

    struct Flag *f = malloc(sizeof(struct Flag));
    f->construct = construct;
    f->id = id;
    f->has_default_value = has_default_value;
    f->default_value = default_value;
    return f;
}

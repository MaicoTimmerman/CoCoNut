#ifndef _AST_H
#define _AST_H

#include "array.h"
#include <stdbool.h>
#include <stdint.h>

enum NodeType { NT_node, NT_nodeset };

enum MandatoryPhaseType { MP_single, MP_range };

enum AttrType {
    AT_int,
    AT_uint,
    AT_int8,
    AT_int16,
    AT_int32,
    AT_int64,
    AT_uint8,
    AT_uint16,
    AT_uint32,
    AT_uint64,

    AT_float,
    AT_double,

    AT_bool,
    AT_string,
    AT_link_or_enum
};

enum AttrValueType {
    AV_string,
    AV_int,
    AV_uint,
    AV_float,
    AV_double,
    AV_bool,
    AV_id
};

struct Config {
    array *phases;
    array *traversals;
    array *enums;
    array *nodesets;
    array *nodes;
};

struct Phase {
    // int dummy;
};

struct Traversal {
    char *id;

    // Array of strings (after parsing) or struct node's
    array *nodes;
};

struct Enum {
    char *id;

    array *values;
};

struct Nodeset {
    char *id;

    // Array of strings (after parsing) or struct node's
    array *nodes;
};

struct Node {
    char *id;

    array *children;
    array *attrs;
    array *flags;
};

struct Child {
    int construct;
    int mandatory;
    array *mandatory_phases;
    char *id;
    char *type;

    union {
        struct Node *node;
        struct Nodeset *nodeset;
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
        struct PhaseRange *range;
        char *single;
    } value;
};

struct Attr {
    int construct;
    enum AttrType type;
    char *type_id;
    char *id;
    struct AttrValue *default_value;
};

struct AttrValue {
    enum AttrValueType type;
    union {
        uint64_t uint_value;
        int64_t int_value;
        float float_value;
        double double_value;
        char *string_value;
        bool bool_value;
    } value;
};

array *create_array(void);

struct Config *create_config(array *phases, array *traversals,
                             array *attr_enums, array *nodesets, array *nodes);

struct Phase *create_phase(void);

struct Traversal *create_traversal(char *id, array *nodes);

struct Enum *create_enum(char *id, char *prefix, array *values);

struct Nodeset *create_nodeset(char *id, array *nodes);

struct Node *create_node(char *id, struct Node *nodebody);

struct Node *create_nodebody(array *children, array *attrs, array *flags);

struct Child *create_child(int construct, int mandatory,
                           array *mandatory_phases, char *id, char *type);

struct MandatoryPhase *create_mandatory_singlephase(char *phase);

struct MandatoryPhase *create_mandatory_phaserange(char *phase_start,
                                                   char *phase_end);

struct Attr *create_attr(struct Attr *attrhead,
                         struct AttrValue *default_value);

struct Attr *create_attrhead_primitive(int construct, enum AttrType type,
                                       char *id);

struct Attr *create_attrhead_idtype(int construct, char *type, char *id);

struct AttrValue *create_attrval_string(char *value);

struct AttrValue *create_attrval_bool(bool value);

struct AttrValue *create_attrval_int(int64_t value);

struct AttrValue *create_attrval_uint(uint64_t value);

struct AttrValue *create_attrval_float(float value);

struct AttrValue *create_attrval_double(double value);

struct AttrValue *create_attrval_id(char *id);

#endif

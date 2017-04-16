#ifndef _AST_H_
#define _AST_H_

#include <stdbool.h>
#include <stdint.h>

#include "array.h"

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
    char *prefix;

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
    int negation;

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

#endif

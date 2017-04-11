#ifndef _AST_H
#define _AST_H

#include "array.h"

struct Config;

struct Phase;

struct Traversal;

struct Enum;

struct Nodeset;

struct Node;

enum NodeType { NT_node, NT_nodeset };

struct Child;

enum MandatoryPhaseType { MP_single, MP_range };

struct PhaseRange;

struct MandatoryPhase;

enum AttrType { AT_char, AT_uchar, AT_short, AT_ushort, AT_int, AT_uint,
                AT_long, AT_ulong, AT_longlong, AT_ulonglong, AT_float,
                AT_double, AT_longdouble, AT_string, AT_link_or_enum };

struct Attr;

struct Flag;

enum  AttrValueType;

struct AttrValue;

array *create_array(void);

struct Config* create_config(array *phases, array *traversals,
        array *attr_enums, array *nodesets, array *nodes);

struct Phase* create_phase(void);

struct Traversal* create_traversal(char* id, array *nodes);

struct Enum* create_enum(char* id, array *values);

struct Nodeset* create_nodeset(char* id, array *nodes);

struct Node* create_node(char *id, struct Node *nodebody);

struct Node* create_nodebody(array *children, array *attrs, array *flags);

struct Child* create_child(int construct, int mandatory,
        array *mandatory_phases, char *id, char *type);

struct MandatoryPhase* create_mandatory_singlephase(char* phase);

struct MandatoryPhase* create_mandatory_phaserange(
        char* phase_start, char* phase_end);

struct Attr* create_attr(struct Attr *attrhead, struct AttrValue *default_value);

struct Attr* create_attrhead_primitive(int construct, enum AttrType type,
        char *id);

struct Attr* create_attrhead_idtype(int construct, char *type, char *id);

struct AttrValue* create_attrval_string(char* value);

struct AttrValue* create_attrval_int(long long value);

struct AttrValue* create_attrval_float(long double value);

struct AttrValue* create_attrval_id(char *id);

struct Flag* create_flag(int construct, char *id,
        int has_default_value, int default_value);

#endif

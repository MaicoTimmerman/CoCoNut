#ifndef _CREATE_AST_H
#define _CREATE_AST_H

#include "lib/array.h"
#include <stdbool.h>
#include <stdint.h>

struct Config *create_config(array *phases, array *passes, array *traversals,
                             array *attr_enums, array *nodesets, array *nodes);

struct Pass *create_pass(char *id, char *func);

struct Traversal *create_traversal(char *id, char *func, array *nodes);

struct Phase *create_phase_header(char *id, bool root, bool cycle);

struct Phase *create_phase(struct Phase *phase_header, array *phases,
                           array *passes);

struct Enum *create_enum(char *id, char *prefix, array *values);

struct Nodeset *create_nodeset(char *id, array *nodes);

struct Node *create_node(char *id, struct Node *nodebody);

struct Node *create_nodebody(array *children, array *attrs);

struct Child *create_child(int construct, int mandatory,
                           array *mandatory_phases, char *id, char *type);

struct MandatoryPhase *create_mandatory_singlephase(char *phase, int negation);

struct MandatoryPhase *
create_mandatory_phaserange(char *phase_start, char *phase_end, int negation);

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
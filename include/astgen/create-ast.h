#ifndef _CREATE_AST_H
#define _CREATE_AST_H

#include "lib/array.h"
#include <stdbool.h>
#include <stdint.h>

Config *create_config(array *phases, array *passes, array *traversals,
                      array *attr_enums, array *nodesets, array *nodes);

Pass *create_pass(char *id, char *func);

Traversal *create_traversal(char *id, char *func, array *nodes);

Phase *create_phase_header(char *id, bool root, bool cycle);

Phase *create_phase(Phase *phase_header, array *phases, array *passes);

Enum *create_enum(char *id, char *prefix, array *values);

Nodeset *create_nodeset(char *id, array *nodes);

Node *create_node(char *id, Node *nodebody);

Node *create_nodebody(array *children, array *attrs);

Child *create_child(int construct, int mandatory, array *mandatory_phases,
                    char *id, char *type);

MandatoryPhase *create_mandatory_singlephase(char *phase, int negation);

MandatoryPhase *create_mandatory_phaserange(char *phase_start, char *phase_end,
                                            int negation);

Attr *create_attr(Attr *attrhead, AttrValue *default_value);

Attr *create_attrhead_primitive(int construct, enum AttrType type, char *id);

Attr *create_attrhead_idtype(int construct, char *type, char *id);

AttrValue *create_attrval_string(char *value);

AttrValue *create_attrval_bool(bool value);

AttrValue *create_attrval_int(int64_t value);

AttrValue *create_attrval_uint(uint64_t value);

AttrValue *create_attrval_float(float value);

AttrValue *create_attrval_double(double value);

AttrValue *create_attrval_id(char *id);

#endif

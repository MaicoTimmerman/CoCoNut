#ifndef _AST_H
#define _AST_H

#include "array.h"

struct Config;

struct Phase;

struct Traversal;

struct Enum;

struct Nodeset;

struct Node;

enum NodeType;

struct Child;

enum MandatoryPhaseType;

struct PhaseRange;

struct MandatoryPhase;

enum AttrType;

struct Attr;

struct Flag;

array *create_array(void);

struct Config* create_config(array *phases, array *traversals,
        array *attr_enums, array *nodesets, array *nodes);

struct Phase* create_phase(void);

struct Traversal* create_traversal(char* id, array *nodes);

struct Enum* create_enum(char* id, array *values);

struct Nodeset* create_nodeset(char* id, array *nodes);

struct Node* create_node(char* id, array *children, array *attrs,
        array *flags);

struct Child* create_child(int construct, int mandatory,
        array *mandatory_phases, char *id, char *type);

struct MandatoryPhase* create_mandatory_singlephase(char* phase);

struct MandatoryPhase* create_mandatory_phaserange(
        char* phase_start, char* phase_end);

struct Attr* create_attr_primitive(int construct, enum AttrType type,
        char *id, void *default_value);

struct Attr* create_attr_idtype(int construct, char *type, char *id,
        void *default_value);

struct Flag* create_flag(int construct, char *id,
        int has_default_value, int default_value);

#endif

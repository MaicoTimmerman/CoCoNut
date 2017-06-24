#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lib/array.h"
#include "lib/imap.h"

#define SERIALIZE_READ_TXT_ERROR_HEADER "textual-serialization-reader"
#define SERIALIZE_WRITE_TXT_ERROR_HEADER "textual-serialization-writer"

typedef struct {
    char *type;
    uint64_t id;
    bool is_root;
    array *children;
    array *attributes;
} AST_TXT_Node;

typedef struct {
    array *nodes;
    AST_TXT_Node *rootnode;
    imap_t *node_id_map;
    imap_t *used_nodes;
} AST_TXT_File;

typedef struct {
    char *type;
    uint64_t id;
    bool is_root;
} AST_TXT_NodeHeader;

typedef struct {
    char *name;
    uint64_t id;
} AST_TXT_Child;

enum AST_TXT_AttributeValueType {
    AST_TXT_int,
    AST_TXT_uint,
    AST_TXT_string,
    AST_TXT_float,
    AST_TXT_id,
    AST_TXT_bool
};

typedef struct {
    enum AST_TXT_AttributeValueType type;
    union {
        int64_t val_int;
        uint64_t val_uint;
        char *val_str;
        double val_float;
        char *val_id;
        bool val_bool;
    } data;
} AST_TXT_AttributeValue;

typedef struct {
    char *name;
    AST_TXT_AttributeValue *value;
} AST_TXT_Attribute;

AST_TXT_File *_serialization_txt_create_file(array *nodes);
AST_TXT_Node *_serialization_txt_create_node(AST_TXT_NodeHeader *header,
                                             AST_TXT_Node *nodebody);
AST_TXT_Node *_serialization_txt_create_nodebody(array *children,
                                                 array *attributes);
AST_TXT_NodeHeader *
_serialization_txt_create_nodeheader(char *type, uint64_t id, bool is_root);
AST_TXT_Child *_serialization_txt_create_child(char *name, uint64_t id);
AST_TXT_Attribute *
_serialization_txt_create_attribute(char *name, AST_TXT_AttributeValue *value);
AST_TXT_AttributeValue *_serialization_txt_create_attrval_int(int64_t value);
AST_TXT_AttributeValue *_serialization_txt_create_attrval_uint(uint64_t value);
AST_TXT_AttributeValue *_serialization_txt_create_attrval_str(char *value);
AST_TXT_AttributeValue *_serialization_txt_create_attrval_float(double value);
AST_TXT_AttributeValue *_serialization_txt_create_attrval_id(char *value);
AST_TXT_AttributeValue *_serialization_txt_create_attrval_bool(bool value);

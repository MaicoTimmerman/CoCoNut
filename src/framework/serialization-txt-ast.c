#include "framework/serialization-txt-ast.h"
#include "lib/memory.h"

AST_TXT_File *_serialization_txt_create_file(array *nodes) {
    AST_TXT_File *res = mem_alloc(sizeof(AST_TXT_File));
    res->nodes = nodes;
    return res;
}

AST_TXT_Node *_serialization_txt_create_node(AST_TXT_NodeHeader *header,
                                             AST_TXT_Node *nodebody) {

    // Create nodebody so that NULL can be passed when constructing a node
    // without children and attributes.
    if (nodebody == NULL) {
        nodebody = mem_alloc(sizeof(AST_TXT_Node));
        nodebody->children = NULL;
        nodebody->attributes = NULL;
    }

    nodebody->type = header->type;
    nodebody->id = header->id;
    nodebody->is_root = header->is_root;
    mem_free(header);
    return nodebody;
}

AST_TXT_Node *_serialization_txt_create_nodebody(array *children,
                                                 array *attributes) {
    AST_TXT_Node *res = mem_alloc(sizeof(AST_TXT_Node));
    res->children = children;
    res->attributes = attributes;
    return res;
}

AST_TXT_NodeHeader *
_serialization_txt_create_nodeheader(char *type, uint64_t id, bool is_root) {
    AST_TXT_NodeHeader *res = mem_alloc(sizeof(AST_TXT_NodeHeader));
    res->type = type;
    res->id = id;
    res->is_root = is_root;
    return res;
}

AST_TXT_Child *_serialization_txt_create_child(char *name, uint64_t id) {
    AST_TXT_Child *res = mem_alloc(sizeof(AST_TXT_Child));
    res->name = name;
    res->id = id;
    return res;
}

AST_TXT_Attribute *
_serialization_txt_create_attribute(char *name,
                                    AST_TXT_AttributeValue *value) {
    AST_TXT_Attribute *res = mem_alloc(sizeof(AST_TXT_Attribute));
    res->name = name;
    res->value = value;
    return res;
}

AST_TXT_AttributeValue *_serialization_txt_create_attrval_int(int64_t value) {
    AST_TXT_AttributeValue *res = mem_alloc(sizeof(AST_TXT_AttributeValue));
    res->type = AST_TXT_int;
    res->data.val_int = value;
    return res;
}

AST_TXT_AttributeValue *
_serialization_txt_create_attrval_uint(uint64_t value) {
    AST_TXT_AttributeValue *res = mem_alloc(sizeof(AST_TXT_AttributeValue));
    res->type = AST_TXT_uint;
    res->data.val_uint = value;
    return res;
}

AST_TXT_AttributeValue *_serialization_txt_create_attrval_str(char *value) {
    AST_TXT_AttributeValue *res = mem_alloc(sizeof(AST_TXT_AttributeValue));
    res->type = AST_TXT_string;
    res->data.val_str = value;
    return res;
}

AST_TXT_AttributeValue *_serialization_txt_create_attrval_float(double value) {
    AST_TXT_AttributeValue *res = mem_alloc(sizeof(AST_TXT_AttributeValue));
    res->type = AST_TXT_float;
    res->data.val_float = value;
    return res;
}

AST_TXT_AttributeValue *_serialization_txt_create_attrval_id(char *value) {
    AST_TXT_AttributeValue *res = mem_alloc(sizeof(AST_TXT_AttributeValue));
    res->type = AST_TXT_id;
    res->data.val_id = value;
    return res;
}

AST_TXT_AttributeValue *_serialization_txt_create_attrval_bool(bool value) {
    AST_TXT_AttributeValue *res = mem_alloc(sizeof(AST_TXT_AttributeValue));
    res->type = AST_TXT_bool;
    res->data.val_bool = value;
    return res;
}

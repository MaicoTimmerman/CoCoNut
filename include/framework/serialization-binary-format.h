#pragma once

#include "lib/array.h"
#include <stdint.h>

typedef enum {
    AT_int = 1,
    AT_uint = 2,
    AT_int8 = 3,
    AT_int16 = 4,
    AT_int32 = 5,
    AT_int64 = 6,
    AT_uint8 = 7,
    AT_uint16 = 8,
    AT_uint32 = 9,
    AT_uint64 = 10,
    AT_float = 11,
    AT_double = 12,
    AT_bool = 13,
    AT_string = 14,
    AT_link = 15,
    AT_enum = 16
} AttributeType;

typedef struct {
    uint32_t magic;
    uint32_t ast_magic;

    array *string_pool;
    array *enum_pool;
    array *nodes;
} AstBinFile;

typedef struct {
    uint32_t name_index;
    uint32_t prefix_index;
    array *values;
} EnumPoolEntry;

typedef struct {
    uint32_t type_index;
    array *children;
    array *attributes;

} Node;

typedef struct {
    uint32_t name_index;
    uint32_t node_index;
} Child;

typedef struct { int value; } Attribute_int_data;

typedef struct { unsigned int value; } Attribute_uint_data;

typedef struct { int8_t value; } Attribute_int8_data;

typedef struct { int16_t value; } Attribute_int16_data;

typedef struct { int32_t value; } Attribute_int32_data;

typedef struct { int64_t value; } Attribute_int64_data;

typedef struct { uint8_t value; } Attribute_uint8_data;

typedef struct { uint16_t value; } Attribute_uint16_data;

typedef struct { uint32_t value; } Attribute_uint32_data;

typedef struct { uint64_t value; } Attribute_uint64_data;

typedef struct { float value; } Attribute_float_data;

typedef struct { double value; } Attribute_double_data;

typedef struct { bool value; } Attribute_bool_data;

typedef struct { uint32_t value_index; } Attribute_string_data;

typedef struct {
    // Index in enum pool
    uint32_t type_index;

    // Index in values array of enum
    uint16_t value_index;
} Attribute_enum_data;

typedef struct { uint32_t node_index; } Attribute_link_data;

typedef struct {
    uint32_t name_index;

    AttributeType type;

    union {
        Attribute_int_data val_int;
        Attribute_uint_data val_uint;
        Attribute_int8_data val_int8;
        Attribute_int16_data val_int16;
        Attribute_int32_data val_int32;
        Attribute_int64_data val_int64;
        Attribute_uint8_data val_uint8;
        Attribute_uint16_data val_uint16;
        Attribute_uint32_data val_uint32;
        Attribute_uint64_data val_uint64;
        Attribute_float_data val_float;
        Attribute_double_data val_double;
        Attribute_bool_data val_bool;
        Attribute_string_data val_string;
        Attribute_link_data val_link;
        Attribute_enum_data val_enum;
    } value;

} Attribute;

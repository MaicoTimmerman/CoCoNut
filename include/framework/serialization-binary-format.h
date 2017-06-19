#pragma once

#include "lib/array.h"
#include <stdbool.h>
#include <stdint.h>

#define SERIALIZE_READ_BIN_ERROR_HEADER "binary-serialization-reader"
#define SERIALIZE_WRITE_BIN_ERROR_HEADER "binary-serialization-writer"

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define HOST_LITTLE_ENDIAN 1
#define FILE_MAGIC 0x00545341
#else
#define HOST_LITTLE_ENDIAN 0
#define FILE_MAGIC 0x41535400
#endif

typedef enum {
    AT_int = 0,
    AT_uint = 1,
    AT_int8 = 2,
    AT_int16 = 3,
    AT_int32 = 4,
    AT_int64 = 5,
    AT_uint8 = 6,
    AT_uint16 = 7,
    AT_uint32 = 8,
    AT_uint64 = 9,
    AT_float = 10,
    AT_double = 11,
    AT_bool = 12,
    AT_string = 13,
    AT_link = 14,
    AT_enum = 15
} AttributeType;

typedef enum {
    AST_LITTLE_ENDIAN = 1 << 15,
    AST_RESERVED_2 = 1 << 14,
    AST_RESERVED_3 = 1 << 13,
    AST_RESERVED_4 = 1 << 12,
    AST_RESERVED_5 = 1 << 11,
    AST_RESERVED_6 = 1 << 10,
    AST_RESERVED_7 = 1 << 9,
    AST_RESERVED_8 = 1 << 8,
    AST_RESERVED_9 = 1 << 7,
    AST_RESERVED_10 = 1 << 6,
    AST_RESERVED_11 = 1 << 5,
    AST_RESERVED_12 = 1 << 4,
    AST_RESERVED_13 = 1 << 3,
    AST_RESERVED_14 = 1 << 2,
    AST_RESERVED_15 = 1 << 1,
    AST_RESERVED_16 = 1

} AstBinFileFlags;

typedef struct {
    uint16_t flags;

    uint8_t *hash;

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

typedef struct { int64_t value; } Attribute_int_data;

typedef struct { uint64_t value; } Attribute_uint_data;

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
    uint16_t type_index;

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

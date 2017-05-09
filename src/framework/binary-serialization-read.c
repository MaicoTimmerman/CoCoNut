#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "framework/serialization-binary-format.h"
#include "lib/array.h"
#include "lib/memory.h"

// TODO: make reading endianess independent

#define READ(N, fp, res)                                                      \
    do {                                                                      \
        fread(&res, N, 1, fp);                                                \
    } while (0)

static int read_int(FILE *fp) {
    int res = 0;
    fread(&res, sizeof(int), 1, fp);
    return res;
}

static unsigned int read_uint(FILE *fp) {
    unsigned int res = 0;
    fread(&res, sizeof(unsigned int), 1, fp);
    return res;
}

static uint32_t read_u4(FILE *fp) {
    uint32_t res = 0;
    fread(&res, 4, 1, fp);
    return res;
}

static uint16_t read_u2(FILE *fp) {
    uint16_t res = 0;
    fread(&res, 2, 1, fp);
    return res;
}

static uint8_t read_u1(FILE *fp) {
    return (uint8_t)fgetc(fp);
}

static char *read_string(FILE *fp) {
    uint16_t length = read_u2(fp);
    char *result = mem_alloc(length);
    fread(result, length, 1, fp);
    return result;
}

static EnumPoolEntry *read_enum_entry(FILE *fp) {
    EnumPoolEntry *entry = mem_alloc(sizeof(EnumPoolEntry));
    entry->name_index = read_u2(fp);
    entry->prefix_index = read_u2(fp);

    uint16_t values_count = read_u2(fp);
    entry->values = array_init(values_count);

    for (int i = 0; i < values_count; i++) {
        uint16_t *index = mem_alloc(sizeof(uint16_t));
        *index = read_u2(fp);

        array_append(entry->values, index);
    }

    return entry;
}

static Child *read_child(FILE *fp) {
    Child *c = mem_alloc(sizeof(Child));
    c->name_index = read_u2(fp);
    c->node_index = read_u2(fp);
    return c;
}

static Attribute *read_attr(FILE *fp) {
    Attribute *attr = mem_alloc(sizeof(Attribute));
    uint8_t type = read_u1(fp);
    switch (type) {
    case AT_int:
        attr->value.val_int.value = read_int(fp);
        break;
    case AT_uint:
        attr->value.val_uint.value = read_uint(fp);
        break;
    case AT_int8:
        READ(1, fp, attr->value.val_int8.value);
        break;
    case AT_int16:
        READ(2, fp, attr->value.val_int16.value);
        break;
    case AT_int32:
        READ(4, fp, attr->value.val_int32.value);
        break;
    case AT_int64:
        READ(8, fp, attr->value.val_int32.value);
        break;
    case AT_uint8:
        READ(1, fp, attr->value.val_uint8.value);
        break;
    case AT_uint16:
        READ(2, fp, attr->value.val_uint16.value);
        break;
    case AT_uint32:
        READ(4, fp, attr->value.val_uint32.value);
        break;
    case AT_uint64:
        READ(8, fp, attr->value.val_uint64.value);
        break;
    case AT_float:
        READ(4, fp, attr->value.val_float.value);
        break;
    case AT_double:
        READ(8, fp, attr->value.val_double.value);
        break;
    case AT_bool:
        READ(1, fp, attr->value.val_bool.value);
        break;
    case AT_string:
        READ(2, fp, attr->value.val_string.value_index);
        break;
    case AT_link:
        READ(4, fp, attr->value.val_link.node_index);
        break;
    case AT_enum:
        READ(2, fp, attr->value.val_enum.type_index);
        break;
    default:
        printf("Invalid attribute type: %d\n", type);
        mem_free(attr);
        return NULL;
    }
    return attr;
}

static Node *read_node(FILE *fp) {
    Node *node = mem_alloc(sizeof(Node));

    uint16_t children_count = read_u2(fp);
    node->children = array_init(children_count);

    for (int i = 0; i < children_count; i++) {
        Child *c = read_child(fp);
        array_append(node->children, c);
    }

    uint16_t attribute_count = read_u2(fp);
    node->attributes = array_init(attribute_count);
    for (int i = 0; i < children_count; i++) {
        Attribute *attr = read_attr(fp);
        array_append(node->attributes, attr);
    }

    return node;
}

AstBinFile *serialization_read_binfile(FILE *fp) {
    AstBinFile *ast = mem_alloc(sizeof(AstBinFile));

    ast->magic = read_u4(fp);
    ast->ast_magic = read_u4(fp);

    uint16_t string_pool_count = read_u2(fp);
    ast->string_pool = array_init(string_pool_count);

    for (int i = 0; i < string_pool_count; i++) {
        char *str = read_string(fp);
        array_append(ast->string_pool, str);
    }

    uint16_t enum_pool_count = read_u2(fp);
    ast->enum_pool = array_init(enum_pool_count);

    for (int i = 0; i < enum_pool_count; i++) {
        EnumPoolEntry *e = read_enum_entry(fp);
        array_append(ast->enum_pool, e);
    }

    uint32_t node_count = read_u4(fp);
    ast->nodes = array_init(node_count);

    for (int i = 0; i < node_count; i++) {
        Node *n = read_node(fp);
        array_append(ast->nodes, n);
    }
    return ast;
}

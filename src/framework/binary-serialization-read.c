#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "framework/serialization-binary-format.h"
#include "lib/array.h"
#include "lib/memory.h"
#include "lib/print.h"

char *_serialization_read_fn = NULL;

static AstBinFile *_ast;

static bool file_read(int N, FILE *fp, void *res) {
    size_t num = fread(res, 1, N, fp);

    // TODO: Make these errors propagate
    if (num < N) {
        if (ferror(fp)) {
            print_user_error(
                SERIALIZE_READ_BIN_ERROR_HEADER,
                "%s: Error reading file: Tried to read %d bytes but "
                "read only %zu bytes.",
                _serialization_read_fn, N, num);
        } else if (feof(fp)) {
            print_user_error(
                SERIALIZE_READ_BIN_ERROR_HEADER,
                "%s: Error reading file: Unexpected end of file reached."
                "Read only %zu of %d bytes",
                _serialization_read_fn, num, N);
        } else {
            print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER,
                             "%s: Error reading file: Unknown error.",
                             _serialization_read_fn);
        }

        return false;
    }

    return true;
}

static uint32_t read_u4(FILE *fp) {
    uint32_t res = 0;
    file_read(4, fp, &res);
    return res;
}

static uint16_t read_u2(FILE *fp) {
    uint16_t res = 0;
    file_read(2, fp, &res);
    return res;
}

static uint8_t read_u1(FILE *fp) {
    uint8_t res = 0;
    file_read(1, fp, &res);
    return res;
}

static char *read_string(FILE *fp) {
    uint16_t length = read_u2(fp);
    char *result = mem_alloc(length + 1);
    file_read(length, fp, result);
    result[length] = '\0';
    return result;
}

static EnumPoolEntry *read_enum_entry(FILE *fp) {
    EnumPoolEntry *entry = mem_alloc(sizeof(EnumPoolEntry));
    entry->name_index = read_u4(fp);
    entry->prefix_index = read_u4(fp);

    uint16_t values_count = read_u2(fp);
    entry->values = array_init(values_count);

    for (int i = 0; i < values_count; i++) {
        uint32_t *index = mem_alloc(sizeof(uint32_t));
        *index = read_u4(fp);

        array_append(entry->values, index);
    }

    return entry;
}

static Child *read_child(FILE *fp) {
    Child *c = mem_alloc(sizeof(Child));
    c->name_index = read_u4(fp);
    c->node_index = read_u4(fp);
    return c;
}

static Attribute *read_attr(FILE *fp) {
    Attribute *attr = mem_alloc(sizeof(Attribute));

    attr->name_index = read_u4(fp);

    uint8_t type = read_u1(fp);

    switch (type) {
    case AT_int:
        file_read(8, fp, &(attr->value.val_int.value));
        break;
    case AT_uint:
        file_read(8, fp, &(attr->value.val_uint.value));
        break;
    case AT_int8:
        file_read(1, fp, &(attr->value.val_int8.value));
        break;
    case AT_int16:
        file_read(2, fp, &(attr->value.val_int16.value));
        break;
    case AT_int32:
        file_read(4, fp, &(attr->value.val_int32.value));
        break;
    case AT_int64:
        file_read(8, fp, &(attr->value.val_int32.value));
        break;
    case AT_uint8:
        file_read(1, fp, &(attr->value.val_uint8.value));
        break;
    case AT_uint16:
        file_read(2, fp, &(attr->value.val_uint16.value));
        break;
    case AT_uint32:
        file_read(4, fp, &(attr->value.val_uint32.value));
        break;
    case AT_uint64:
        file_read(8, fp, &(attr->value.val_uint64.value));
        break;
    case AT_float:
        file_read(4, fp, &(attr->value.val_float.value));
        break;
    case AT_double:
        file_read(8, fp, &(attr->value.val_double.value));
        break;
    case AT_bool:
        file_read(1, fp, &(attr->value.val_bool.value));
        break;
    case AT_string:
        file_read(4, fp, &(attr->value.val_string.value_index));
        break;
    case AT_link:
        file_read(4, fp, &(attr->value.val_link.node_index));
        break;
    case AT_enum:
        file_read(2, fp, &(attr->value.val_enum.type_index));
        file_read(2, fp, &(attr->value.val_enum.value_index));
        break;
    default:
        print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER,
                         "%s: Invalid attribute type: %d",
                         _serialization_read_fn, type);
        mem_free(attr);
        return NULL;
    }

    attr->type = (AttributeType)type;

    return attr;
}

static Node *read_node(FILE *fp) {

    Node *node = mem_alloc(sizeof(Node));
    memset(node, 0, sizeof(Node));

    node->type_index = read_u4(fp);

    uint16_t children_count = read_u2(fp);

    if (children_count > 0) {

        node->children = array_init(children_count);

        for (int i = 0; i < children_count; i++) {
            Child *c = read_child(fp);
            array_append(node->children, c);
        }
    }

    uint16_t attribute_count = read_u2(fp);

    if (attribute_count > 0) {

        node->attributes = array_init(attribute_count);

        for (int i = 0; i < attribute_count; i++) {
            Attribute *attr = read_attr(fp);
            array_append(node->attributes, attr);
        }
    }

    return node;
}

AstBinFile *serialization_read_binfile(FILE *fp) {
    uint32_t magic = read_u4(fp);

    const uint32_t magic_correct = FILE_MAGIC;
    if (memcmp(&magic, &magic_correct, 4) != 0) {
        print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER,
                         "%s: File signature is incorrect.",
                         _serialization_read_fn);
        return NULL;
    }

    AstBinFile *ast = mem_alloc(sizeof(AstBinFile));
    memset(ast, 0, sizeof(AstBinFile));

    _ast = ast;

    uint8_t flags_l = read_u1(fp);
    uint8_t flags_r = read_u1(fp);
    ast->flags = flags_l << 8 | flags_r;

    bool file_endianness = (bool)(ast->flags & AST_LITTLE_ENDIAN);

    if (file_endianness != HOST_LITTLE_ENDIAN) {
        print_user_error(
            SERIALIZE_READ_BIN_ERROR_HEADER,
            "%s: Endianness mismatch: host is %s-endian but file is %s-endian",
            _serialization_read_fn, HOST_LITTLE_ENDIAN ? "little" : "big",
            file_endianness ? "little" : "big");
        mem_free(ast);
        return NULL;
    }

    ast->hash = mem_alloc(16 * sizeof(uint8_t));

    file_read(16, fp, ast->hash);

    uint32_t string_pool_count = read_u4(fp);

    if (string_pool_count > 0) {

        ast->string_pool = array_init(string_pool_count);

        for (int i = 0; i < string_pool_count; i++) {
            char *str = read_string(fp);
            array_append(ast->string_pool, str);
        }
    } else
        ast->string_pool = NULL;

    uint16_t enum_pool_count = read_u2(fp);

    if (enum_pool_count > 0) {
        ast->enum_pool = array_init(enum_pool_count);

        for (int i = 0; i < enum_pool_count; i++) {
            EnumPoolEntry *e = read_enum_entry(fp);

            array_append(ast->enum_pool, e);
        }
    } else
        ast->enum_pool = NULL;

    uint32_t node_count = read_u4(fp);

    if (node_count > 0) {
        ast->nodes = array_init(node_count);

        for (int i = 0; i < node_count; i++) {

            Node *n = read_node(fp);

            array_append(ast->nodes, n);
        }
    } else
        ast->nodes = NULL;
    return ast;
}

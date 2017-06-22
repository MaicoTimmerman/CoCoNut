#include <stdio.h>
#include <string.h>

#include "cocogen/ast.h"
#include "cocogen/filegen-driver.h"
#include "cocogen/filegen-util.h"
#include "cocogen/str-ast.h"

#include "lib/array.h"
#include "lib/imap.h"
#include "lib/memory.h"
#include "lib/print.h"
#include "lib/smap.h"

static smap_t *string_pool_indices;
static array *string_pool_constants;

static smap_t *enum_type_indices;

static void generate_node_gen_traversal(Node *node, FILE *fp) {

    out("void _serialization_gen_node_%s(%s *node, FILE *fp) {\n", node->id,
        node->id);
    out("    if (node == NULL) return;\n\n");

    out("    //  Write index in string pool representing the type of the "
        "node\n");
    out("    uint32_t type_index = %d;\n",
        *((int *)smap_retrieve(string_pool_indices, node->id)));
    out("    WRITE(4, type_index);\n\n");
    out("    uint16_t child_count = 0;\n\n");

    for (int j = 0; j < array_size(node->children); j++) {
        Child *c = array_get(node->children, j);
        out("    if (node->%s != NULL)\n", c->id);
        out("        child_count++;\n");
    }

    out("\n");

    uint16_t attr_count = 0;
    for (int j = 0; j < array_size(node->attrs); j++) {
        Attr *attr = array_get(node->attrs, j);
        if (!(attr->type == AT_string || attr->type == AT_link)) {
            attr_count++;
        }
    }

    out("    uint16_t attr_count = %d;\n", attr_count);

    for (int j = 0; j < array_size(node->attrs); j++) {
        Attr *attr = array_get(node->attrs, j);
        if (attr->type == AT_string || attr->type == AT_link) {
            out("    if (node->%s != NULL)\n", attr->id);
            out("        attr_count++;\n");
        }
    }

    out("\n");
    out("    WRITE(2, child_count);\n");
    if (array_size(node->children) > 0 || array_size(node->attrs) > 0) {
        out("\n");
        out("    uint32_t name_index;\n");
    }

    if (array_size(node->children) > 0) {
        out("    uint32_t node_index;\n\n");

        for (int j = 0; j < array_size(node->children); j++) {
            Child *c = array_get(node->children, j);
            out("    if (node->%s != NULL) {\n", c->id);
            out("        node_index = *((int "
                "*)imap_retrieve(node_indices, node->%s));\n",
                c->id);
            out("        name_index = %d;\n",
                *((int *)smap_retrieve(string_pool_indices, c->id)));
            out("        WRITE(4, name_index);\n");
            out("        WRITE(4, node_index);\n");
            out("    }\n");
        }

        out("\n");
    }

    out("\n");

    out("    WRITE(2, attr_count);\n");
    if (array_size(node->attrs) > 0) {
        out("    uint8_t tag;\n\n");

        for (int j = 0; j < array_size(node->attrs); j++) {
            Attr *attr = array_get(node->attrs, j);

            out("    // Attribute %s\n", attr->id);

            const char *indent = "    ";

            if (attr->type == AT_string || attr->type == AT_link) {
                indent = "        ";
                out("    if (node->%s != NULL) {\n", attr->id);
            }

            out("%sname_index = %d;\n", indent,
                *((int *)smap_retrieve(string_pool_indices, attr->id)));

            switch (attr->type) {
            case AT_int:
                out("    tag = AT_int;\n");
                break;
            case AT_uint:
                out("    tag = AT_uint;\n");
                break;
            case AT_int8:
                out("    tag = AT_int8;\n");
                break;
            case AT_int16:
                out("    tag = AT_int16;\n");
                break;
            case AT_int32:
                out("    tag = AT_int32;\n");
                break;
            case AT_int64:
                out("    tag = AT_int64;\n");
                break;
            case AT_uint8:
                out("    tag = AT_uint8;\n");
                break;
            case AT_uint16:
                out("    tag = AT_uint16;\n");
                break;
            case AT_uint32:
                out("    tag = AT_uint32;\n");
                break;
            case AT_uint64:
                out("    tag = AT_uint64;\n");
                break;
            case AT_float:
                out("    tag = AT_float;\n");
                break;
            case AT_double:
                out("    tag = AT_double;\n");
                break;
            case AT_bool:
                out("    tag = AT_bool;\n");
                break;
            case AT_string:
                out("        tag = AT_string;\n");
                break;
            case AT_link:
                out("        tag = AT_link;\n");
                break;
            case AT_enum:
                out("    tag = AT_enum;\n");
                break;
            default:
                print_internal_error("Invalid attribute type: %d", attr->type);
                break;
            }

            out("%sWRITE(4, name_index);\n", indent);
            out("%sWRITE(1, tag);\n", indent);

            switch (attr->type) {
            case AT_int8:
            case AT_uint8:
                out("    WRITE(1, node->%s);\n", attr->id);
                break;
            case AT_int16:
            case AT_uint16:
                out("    WRITE(2, node->%s);\n", attr->id);
                break;
            case AT_int32:
            case AT_uint32:
                out("    WRITE(4, node->%s);\n", attr->id);
                break;
            case AT_int:
            case AT_uint:
            case AT_int64:
            case AT_uint64:
                out("    WRITE(8, node->%s);\n", attr->id);
                break;
            case AT_float:
                out("    WRITE(sizeof(float), node->%s);\n", attr->id);
                break;
            case AT_double:
                out("    WRITE(sizeof(double), node->%s);\n", attr->id);
                break;
            case AT_bool:
                out("    WRITE(1, node->%s);\n", attr->id);
                break;
            case AT_string:
                out("        const uint32_t value_%s = *((int*) "
                    "smap_retrieve(attrs_index, node->%s));\n",
                    attr->id, attr->id);
                out("        WRITE(4, value_%s);\n", attr->id);
                out("    }\n");
                break;
            case AT_link:
                out("        const uint32_t value_%s = *((int *) "
                    "imap_retrieve(node_indices, node->%s));\n",
                    attr->id, attr->id);
                out("        WRITE(4, value_%s);\n", attr->id);
                out("    }\n");
                break;
            case AT_enum:
                out("    const uint16_t enum_type_%s = %d;\n", attr->id,
                    *((int *)smap_retrieve(enum_type_indices, attr->type_id)));
                out("    const uint16_t value_%s = "
                    "_serialization_enum_get_%s_value_index(node->%s);\n",
                    attr->id, attr->type_id, attr->id);

                out("    WRITE(2, enum_type_%s);\n", attr->id);
                out("    WRITE(2, value_%s);\n", attr->id);
                break;
            default:
                break;
            }
        }
    }

    for (int j = 0; j < array_size(node->children); j++) {
        Child *c = array_get(node->children, j);
        out("    _serialization_gen_node_%s(node->%s, fp);\n", c->type, c->id);
    }

    out("}\n\n");
}

static void generate_nodeset_gen_traversal(Nodeset *nodeset, FILE *fp) {

    out("void _serialization_gen_node_%s(%s *nodeset, FILE *fp) {\n",
        nodeset->id, nodeset->id);
    out("    if (nodeset == NULL) return;\n\n");
    out("    switch (nodeset->type) {\n");

    for (int j = 0; j < array_size(nodeset->nodes); j++) {
        Node *child_node = array_get(nodeset->nodes, j);
        out("    case " NS_FORMAT ":\n", nodeset->id, child_node->id);
        out("        _serialization_gen_node_%s(nodeset->value.val_%s, fp);\n",
            child_node->id, child_node->id);
        out("        break;\n");
    }

    out("    }\n");
    out("}\n\n");
}

static void generate_string_traversal_handler(Config *config, FILE *fp,
                                              Node *node) {

    out("void _serialization_attr_string_trav_%s(%s *node) {\n", node->id,
        node->id);
    out("    if (node == NULL) return;\n");
    for (int j = 0; j < array_size(node->attrs); j++) {
        Attr *attr = array_get(node->attrs, j);

        if (attr->type == AT_string) {
            out("    if (node->%s != NULL)\n", attr->id);
            out("        array_append(string_attrs, node->%s);\n", attr->id);
        }
    }
    for (int j = 0; j < array_size(node->children); j++) {

        Child *c = array_get(node->children, j);
        out("    _serialization_attr_string_trav_%s(node->%s);\n", c->type,
            c->id);
    }
    out("}\n\n");
}

static void generate_string_traversal_handler_nodeset(Config *config, FILE *fp,
                                                      Nodeset *nodeset) {

    out("void _serialization_attr_string_trav_%s(%s *nodeset) {\n",
        nodeset->id, nodeset->id);
    out("    if (nodeset == NULL) return;\n");
    out("    switch (nodeset->type) {\n");

    for (int j = 0; j < array_size(nodeset->nodes); j++) {
        Node *child_node = array_get(nodeset->nodes, j);
        out("    case " NS_FORMAT ":\n", nodeset->id, child_node->id);
        out("        "
            "_serialization_attr_string_trav_%s(nodeset->value.val_%s);\n",
            child_node->id, child_node->id);
        out("        break;\n");
    }

    out("    }\n");
    out("}\n\n");
}

static void populate_static_string_pool(Config *config) {
    // Fills a list of strings that will always be present in the string pool

    string_pool_indices = smap_init(32);
    string_pool_constants = array_init(32);

    enum_type_indices = smap_init(32);

    // Add node names to string pool hashtable
    for (int i = 0; i < array_size(config->nodes); i++) {
        Node *n = array_get(config->nodes, i);
        int *index_ptr = mem_alloc(sizeof(int));
        *index_ptr = array_size(string_pool_constants);
        char *node_name = n->id;
        smap_insert(string_pool_indices, node_name, index_ptr);
        array_append(string_pool_constants, node_name);

        // Add children names of node
        for (int j = 0; j < array_size(n->children); j++) {
            Child *c = array_get(n->children, j);

            if (smap_retrieve(string_pool_indices, c->id) == NULL) {
                int *index_ptr = mem_alloc(sizeof(int));
                *index_ptr = array_size(string_pool_constants);

                smap_insert(string_pool_indices, c->id, index_ptr);
                array_append(string_pool_constants, c->id);
            }
        }

        // Add attribute names of node
        for (int j = 0; j < array_size(n->attrs); j++) {
            Attr *a = array_get(n->attrs, j);

            if (smap_retrieve(string_pool_indices, a->id) == NULL) {
                int *index_ptr = mem_alloc(sizeof(int));
                *index_ptr = array_size(string_pool_constants);

                smap_insert(string_pool_indices, a->id, index_ptr);
                array_append(string_pool_constants, a->id);
            }
        }
    }

    uint16_t string_pool_nodesets_offset = array_size(string_pool_constants);

    // Add nodeset names to string pool hashtable
    for (int i = 0; i < array_size(config->nodesets); i++) {
        int *i_ptr = mem_alloc(sizeof(int));
        *i_ptr = string_pool_nodesets_offset + i;
        char *nodeset_name = ((Nodeset *)array_get(config->nodesets, i))->id;

        smap_insert(string_pool_indices, nodeset_name, i_ptr);
        array_append(string_pool_constants, nodeset_name);
    }

    uint16_t enums_offset = array_size(string_pool_constants);

    for (int i = 0; i < array_size(config->enums); i++) {
        Enum *e = array_get(config->enums, i);

        int *id_ptr = mem_alloc(sizeof(int));
        int *prefix_ptr = mem_alloc(sizeof(int));

        *id_ptr = enums_offset;
        *prefix_ptr = enums_offset + 1;
        enums_offset += 2;

        smap_insert(string_pool_indices, e->id, id_ptr);
        smap_insert(string_pool_indices, e->prefix, prefix_ptr);

        int *index = mem_alloc(sizeof(int));
        *index = i;
        smap_insert(enum_type_indices, e->id, index);

        array_append(string_pool_constants, e->id);
        array_append(string_pool_constants, e->prefix);

        for (int j = 0; j < array_size(e->values); j++) {
            char *value = array_get(e->values, j);

            if (smap_retrieve(string_pool_indices, value) == NULL) {
                int *index_ptr = mem_alloc(sizeof(int));
                *index_ptr = enums_offset;

                smap_insert(string_pool_indices, value, index_ptr);
                array_append(string_pool_constants, value);
                enums_offset++;
            }
        }
    }
}

static void generate_enum_to_index_table(Config *config, FILE *fp) {
    // Generates functions to map enum values to their index

    for (int i = 0; i < array_size(config->enums); i++) {
        Enum *e = array_get(config->enums, i);
        out("int _serialization_enum_get_%s_value_index(%s e) {\n", e->id,
            e->id);
        out("    switch (e) {\n");
        for (int j = 0; j < array_size(e->values); j++) {
            char *value = array_get(e->values, j);
            out("    case %s_%s:\n", e->prefix, value);
            out("        return %d;\n", j);
        }
        out("    default:\n");
        out("        return -1;\n");
        out("    }\n");
        out("}\n\n");
    }
}

static void generate_populate_node_index_map_node(Node *node, FILE *fp) {

    out("void _serialization_populate_node_indices_%s(%s *node) {\n", node->id,
        node->id);

    out("    if (node == NULL) return;\n\n");
    out("    int *index = mem_alloc(sizeof(int));\n");
    out("    *index = node_index_counter++;\n");
    out("    imap_insert(node_indices, node, index);\n\n");

    for (int j = 0; j < array_size(node->children); j++) {
        Child *c = array_get(node->children, j);
        out("    _serialization_populate_node_indices_%s(node->%s);\n",
            c->type, c->id);
    }

    out("}\n\n");
}

static void generate_populate_node_index_map_nodeset(Nodeset *nodeset,
                                                     FILE *fp) {
    out("void _serialization_populate_node_indices_%s(%s *nodeset) {\n",
        nodeset->id, nodeset->id);
    out("    if (nodeset == NULL) return;\n\n");

    out("    // Add the pointer to the nodeset itself with the same index "
        "as the node\n");
    out("    int *index = mem_alloc(sizeof(int));\n");
    out("    *index = node_index_counter;\n");
    out("    imap_insert(node_indices, nodeset, index);\n\n");
    out("    switch (nodeset->type) {\n");

    for (int j = 0; j < array_size(nodeset->nodes); j++) {
        Node *child_node = array_get(nodeset->nodes, j);
        out("    case " NS_FORMAT ":\n", nodeset->id, child_node->id);
        out("        "
            "_serialization_populate_node_indices_%s(nodeset->value.val_%s);"
            "\n",
            child_node->id, child_node->id);
        out("        break;\n");
    }

    out("   }\n");

    out("}\n\n");
}

static void *free_int_index_string(char *key, void *value) {
    mem_free(value);
    return NULL;
}

static void *free_int_index_int(void *key, void *value) {
    mem_free(value);
    return NULL;
}

static void generate_file_header_write_function(Config *config, FILE *fp) {
    out("void _serialization_write_file_header(FILE *fp) {\n");
    out("    // Write magic\n");
    out("    uint32_t magic = FILE_MAGIC;\n");
    out("    WRITE(4, magic);\n\n");

    out("    // Write flags\n");
    out("    uint16_t flags = 0;\n");
    out("#ifdef HOST_LITTLE_ENDIAN\n");
    out("    flags |= AST_LITTLE_ENDIAN;\n");
    out("#endif\n\n");
    out("    const uint8_t flags_l = flags >> 8;\n");
    out("    const uint8_t flags_r = flags & ((1 << 8) - 1);\n");

    out("    WRITE(1, flags_l);\n");
    out("    WRITE(1, flags_r);\n\n");
    out("}\n\n");
}

static void generate_static_string_pool_write_function(Config *config,
                                                       FILE *fp) {
    out("void _serialization_write_static_string_pool(FILE *fp) {\n");
    out("    // Write static strings of nodes, nodesets and enums\n");
    out("    char *string;\n");
    out("    uint16_t string_length;\n");

    for (int i = 0; i < array_size(string_pool_constants); i++) {
        char *str = array_get(string_pool_constants, i);
        out("    string_length = %zu;\n", strnlen(str, UINT16_MAX));
        out("    string = \"%s\";\n", str);
        out("    WRITE(2, string_length);\n");
        out("    fwrite(string, string_length, 1, fp);\n");
    }
    out("}\n\n");
}

static void generate_enum_pool_write_function(Config *config, FILE *fp) {

    out("void _serialization_write_enum_pool(FILE *fp) {\n");
    out("    const uint16_t enum_pool_count = %d;\n",
        array_size(config->enums));
    out("    WRITE(2, enum_pool_count);\n");

    out("    uint32_t name_index, prefix_index, value_index;\n");
    out("    uint16_t values_count;\n");
    out("\n");

    for (int i = 0; i < array_size(config->enums); i++) {
        Enum *e = array_get(config->enums, i);

        out("    // Enum %s\n", e->id);
        out("    name_index = %d;\n",
            *((int *)smap_retrieve(string_pool_indices, e->id)));
        out("    prefix_index = %d;\n",
            *((int *)smap_retrieve(string_pool_indices, e->prefix)));
        out("    values_count = %d;\n", array_size(e->values));

        out("    WRITE(4, name_index);\n");
        out("    WRITE(4, prefix_index);\n");
        out("    WRITE(2, values_count);\n");
        out("\n");

        for (int j = 0; j < array_size(e->values); j++) {
            char *value = array_get(e->values, j);
            out("    // %s\n", value);
            out("    value_index = %d;\n",
                *((int *)smap_retrieve(string_pool_indices, value)));
            out("    WRITE(4, value_index);\n");
        }
        out("\n");
    }
    out("}\n\n");
}

static void generate_enum_read_functions(Config *config, FILE *fp) {
    out("int _serialization_enum_strings_to_enum(AstBinFile *file, int "
        "type_index, int value_index) {\n");
    out("    EnumPoolEntry *e = array_get(file->enum_pool, type_index);\n");
    /* out("    if (e == NULL) {\n"); */
    /* out("        print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER, " */
    /*         "\"%%s: Enum pool index %%d out of range.\", type_index);\n");
     */
    /* out("        return 0;\n"); */
    /* out("    }\n"); */
    out("\n");

    out("    char *type = array_get(file->string_pool, e->name_index);\n");
    out("    int *value_str_index = array_get(e->values, value_index);\n");
    out("    char *value = array_get(file->string_pool, *value_str_index);\n");
    out("\n");

    for (int i = 0; i < array_size(config->enums); i++) {
        out("    ");
        if (i > 0)
            out("else ");

        Enum *e = array_get(config->enums, i);

        out("if (strcmp(type, \"%s\") == 0) {\n", e->id);

        for (int j = 0; j < array_size(e->values); j++) {
            out("        ");
            if (j > 0)
                out("else ");

            char *value = array_get(e->values, j);
            out("if (strcmp(value, \"%s\") == 0) {\n", value);
            out("            return %d;\n", j);
            out("        }\n");
        }
        out("        else {\n");
        out("            print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER, "
            "\"%%s: Unknown value %%s for enum type %s\", value);\n",
            e->id);
        out("            return 0;\n");
        out("        }\n");
        out("    }\n");
    }

    out("    else {\n");
    out("        print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER, \"%%s: "
        "Unknown enum type %%s\", _serialization_read_fn, type);\n");
    out("        return 0;\n");
    out("    }\n");
    out("}\n");
}

static void generate_serialization_function_node(Node *n, FILE *fp) {

    // TODO: put in util
    out("static void *free_int_index_string(char *key, void *value) {\n");
    out("    mem_free(value);\n");
    out("    return NULL;\n");
    out("}\n\n");

    out("void " SERIALIZE_WRITE_BIN_FORMAT "(%s *syntaxtree, char *fn) {\n",
        n->id, n->id);

    out("    FILE *fp = fopen(fn, \"wb\");\n");
    out("    if (fp == NULL) {\n");
    out("        print_user_error(SERIALIZE_WRITE_BIN_ERROR_HEADER, \"%%s: "
        "%%s\", fn, "
        "strerror(errno));\n");
    out("        return;\n");
    out("    }\n\n");
    out("    string_attrs = array_init(32);\n");
    out("    attrs_index = smap_init(32);\n");
    out("    node_indices = imap_init(32);\n");
    out("    node_index_counter = 0;\n");
    out("\n");

    out("    _serialization_populate_node_indices_%s(syntaxtree);\n\n", n->id);

    out("    _serialization_write_file_header(fp);\n\n");

    // TODO: generate real AST hash
    out("    // Write AST hash\n");
    out("    uint8_t ast_hash[16] = "
        "{1,2,3,4,5,6,7,8,9,10,0xA,0xB,0xC,0xD,0xE,0xF};\n");
    out("    WRITE(16, ast_hash);\n\n");

    out("    // Write string pool\n");
    out("    uint32_t string_pool_count = STRING_POOL_STATIC_SIZE;\n\n");

    out("    // Collect string attributes in AST\n");
    out("    _serialization_attr_string_trav_%s(syntaxtree);\n", n->id);
    out("    string_pool_count += array_size(string_attrs);\n");

    out("    // Write string pool count\n");
    out("    WRITE(4, string_pool_count);\n");
    out("    _serialization_write_static_string_pool(fp);\n\n");

    // TODO: fix multiple strings with same value; Gives memory leak
    out("    // Write string attributes\n");
    out("    char *string;\n");
    out("    uint16_t string_length;\n");
    out("    for (int i = 0; i < array_size(string_attrs); i++) {\n");
    out("        string = array_get(string_attrs, i);\n");
    /* out("        if (smap_retrieve(attrs_index, string) != NULL)\n"); */
    /* out("            continue;\n\n"); */
    out("        int *index = mem_alloc(sizeof(int));\n");
    out("        *index = STRING_POOL_STATIC_SIZE + i;\n");
    out("\n");
    out("        smap_insert(attrs_index, string, index);\n");
    out("        string_length = strnlen(string, UINT16_MAX);\n");
    out("        WRITE(2, string_length);\n");
    out("        fwrite(string, string_length, 1, fp);\n");
    out("        index++;\n");

    out("    }\n");

    out("    array_cleanup(string_attrs, NULL);\n");
    out("\n");

    out("    // Write enum pool\n");
    out("    _serialization_write_enum_pool(fp);\n\n");

    out("    // Write nodes\n");
    out("    WRITE(4, node_index_counter);\n");
    out("    _serialization_gen_node_%s(syntaxtree, fp);\n\n", n->id);

    out("    // Cleanup\n");
    out("    smap_map(attrs_index, free_int_index_string);\n");
    out("    smap_free(attrs_index);\n");

    out("}\n\n");
}

static void generate_serialization_function_nodeset(Nodeset *n, FILE *fp) {

    for (int i = 0; i < array_size(n->nodes); i++) {
        Node *node = array_get(n->nodes, i);
        out("void " SERIALIZE_WRITE_BIN_FORMAT "(%s *, char *fn);\n", node->id,
            node->id);
    }
    out("\n");

    out("void " SERIALIZE_WRITE_BIN_FORMAT "(%s *syntaxtree, char *fn) {\n",
        n->id, n->id);

    out("    switch (syntaxtree->type) {\n");

    for (int i = 0; i < array_size(n->nodes); i++) {
        Node *node = array_get(n->nodes, i);

        out("    case " NS_FORMAT ":\n", n->id, node->id);
        out("        " SERIALIZE_WRITE_BIN_FORMAT
            "(syntaxtree->value.val_%s, fn);\n",
            node->id, node->id);
        out("        break;\n");
    }

    out("    }\n");
    out("}\n");
}

void generate_binary_serialization_util(Config *config, FILE *fp) {
    populate_static_string_pool(config);

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <string.h>\n");
    out("#include \"generated/ast.h\"\n");
    out("#include \"framework/serialization-binary-format.h\"\n");
    out("#include \"framework/serialization-read-file.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/print.h\"\n");
    out("\n");

    out("#define WRITE(N, data) do { \\\n"
        "               fwrite(&data, N, 1, fp); \\\n"
        "               fflush(fp); \\\n"
        "              } while(0)\n\n");

    out("#define STRING_POOL_STATIC_SIZE %d\n\n",
        array_size(string_pool_constants));

    out("array *string_attrs = NULL;\n");
    out("smap_t *attrs_index = NULL;\n\n");
    out("imap_t *node_indices = NULL;\n");
    out("int node_index_counter = 0;\n");
    out("\n");

    generate_file_header_write_function(config, fp);

    generate_enum_to_index_table(config, fp);

    generate_static_string_pool_write_function(config, fp);

    generate_enum_pool_write_function(config, fp);

    generate_enum_read_functions(config, fp);

    array_cleanup(string_pool_constants, NULL);
    smap_map(string_pool_indices, free_int_index_string);
    smap_free(string_pool_indices);

    smap_map(enum_type_indices, free_int_index_string);
    smap_free(enum_type_indices);
}

void generate_binary_serialization_node(Config *config, FILE *fp, Node *node) {
    populate_static_string_pool(config);

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <string.h>\n");
    out("#include <errno.h>\n");
    out("#include \"generated/ast.h\"\n");
    out("#include \"generated/binary-serialization-util.h\"\n");
    out("#include \"framework/serialization-binary-format.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/print.h\"\n");
    out("\n");

    out("#define WRITE(N, data) do { \\\n"
        "               fwrite(&data, N, 1, fp); \\\n"
        "               fflush(fp); \\\n"
        "              } while(0)\n\n");

    out("#define STRING_POOL_STATIC_SIZE %d\n\n",
        array_size(string_pool_constants));

    smap_t *node_types = smap_init(32);

    // Generate declarations
    for (int i = 0; i < array_size(node->children); i++) {
        Child *c = array_get(node->children, i);

        if (!smap_retrieve(node_types, c->type)) {

            out("void _serialization_attr_string_trav_%s(%s *node);\n",
                c->type, c->type);
            out("void _serialization_gen_node_%s(%s *node, FILE *fp);\n",
                c->type, c->type);
            out("void _serialization_populate_node_indices_%s(%s *node);\n",
                c->type, c->type);
            smap_insert(node_types, c->type, NULL);
        }
    }

    smap_free(node_types);

    out("\n");

    generate_node_gen_traversal(node, fp);

    generate_populate_node_index_map_node(node, fp);

    generate_string_traversal_handler(config, fp, node);

    generate_serialization_function_node(node, fp);

    array_cleanup(string_pool_constants, NULL);
    smap_map(string_pool_indices, free_int_index_string);
    smap_free(string_pool_indices);

    smap_map(enum_type_indices, free_int_index_string);
    smap_free(enum_type_indices);
}

void generate_binary_serialization_nodeset(Config *config, FILE *fp,
                                           Nodeset *nodeset) {
    populate_static_string_pool(config);

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <string.h>\n");
    out("#include <unistd.h>\n");
    out("#include \"generated/ast.h\"\n");
    out("#include \"generated/binary-serialization-util.h\"\n");
    out("#include \"framework/serialization-binary-format.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/print.h\"\n");
    out("\n");

    out("#define WRITE(N, data) do { \\\n"
        "               fwrite(&data, N, 1, fp); \\\n"
        "               fflush(fp); \\\n"
        "              } while(0)\n\n");

    out("#define STRING_POOL_STATIC_SIZE %d\n\n",
        array_size(string_pool_constants));

    // Generate declarations
    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *n = array_get(nodeset->nodes, i);
        out("void _serialization_attr_string_trav_%s(%s *node);\n", n->id,
            n->id);
        out("void _serialization_gen_node_%s(%s *node, FILE *fp);\n", n->id,
            n->id);
        out("void _serialization_populate_node_indices_%s(%s *node);\n", n->id,
            n->id);
    }
    out("\n");

    generate_nodeset_gen_traversal(nodeset, fp);

    generate_populate_node_index_map_nodeset(nodeset, fp);

    generate_string_traversal_handler_nodeset(config, fp, nodeset);

    generate_serialization_function_nodeset(nodeset, fp);

    array_cleanup(string_pool_constants, NULL);
    smap_map(string_pool_indices, free_int_index_string);
    smap_free(string_pool_indices);

    smap_map(enum_type_indices, free_int_index_string);
    smap_free(enum_type_indices);
}

void generate_binary_serialization_util_header(Config *config, FILE *fp) {
    out("#pragma once\n\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"framework/serialization-binary-format.h\"\n");
    out("\n");

    out("extern array *string_attrs;\n");
    out("extern smap_t *attrs_index;\n");
    out("extern imap_t *node_indices;\n");
    out("extern int node_index_counter;\n");
    out("\n");

    out("void _serialization_write_file_header(FILE *fp);\n");
    out("void _serialization_write_enum_pool(FILE *fp);\n");
    out("void _serialization_write_static_string_pool(FILE *fp);\n");
    out("int _serialization_enum_strings_to_enum(AstBinFile *file, int "
        "type_index, int value_index);\n");

    for (int i = 0; i < array_size(config->enums); i++) {
        Enum *e = array_get(config->enums, i);
        out("int _serialization_enum_get_%s_value_index(%s);\n", e->id, e->id);
    }
}

#include <stdio.h>
#include <string.h>

#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"
#include "astgen/str-ast.h"

#include "lib/array.h"
#include "lib/imap.h"
#include "lib/memory.h"
#include "lib/smap.h"

static smap_t *string_pool_indices;
static array *string_pool_constants;

static void generate_node_gen_traversal(Config *config, FILE *fp) {
    // Generate declarations
    for (int i = 0; i < array_size(config->nodes); i++) {
        Node *n = array_get(config->nodes, i);
        out("static void gen_node_trav_%s(%s *node, FILE *fp);\n", n->id,
            n->id);
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        Nodeset *n = array_get(config->nodesets, i);
        out("static void gen_node_trav_%s(%s *nodeset, FILE *fp);\n", n->id,
            n->id);
    }
    out("\n");

    for (int i = 0; i < array_size(config->nodes); i++) {
        Node *n = array_get(config->nodes, i);
        out("static void gen_node_trav_%s(%s *node, FILE *fp) {\n", n->id,
            n->id);
        out("    if (node == NULL) return;\n\n");

        out("    //  Write index in string pool representing the type of the "
            "node\n");
        out("    uint16_t type_index = %d;\n",
            *((int *)smap_retrieve(string_pool_indices, n->id)));
        out("    WRITE(2, type_index);\n\n");
        out("    uint16_t child_count = 0;\n\n");

        for (int j = 0; j < array_size(n->children); j++) {
            Child *c = array_get(n->children, j);
            out("    if (node->%s != NULL)\n", c->id);
            out("        child_count++;\n");
        }

        out("\n");

        uint16_t attr_count = 0;
        for (int j = 0; j < array_size(n->attrs); j++) {
            Attr *attr = array_get(n->attrs, j);
            if (!(attr->type == AT_string || attr->type == AT_link)) {
                attr_count++;
            }
        }

        out("    uint16_t attr_count = %d;\n", attr_count);

        for (int j = 0; j < array_size(n->attrs); j++) {
            Attr *attr = array_get(n->attrs, j);
            if (attr->type == AT_string || attr->type == AT_link) {
                out("    if (node->%s != NULL)\n", attr->id);
                out("        attr_count++;\n");
            }
        }

        out("\n");
        out("    WRITE(2, child_count);\n");
        if (array_size(n->children) > 0 || array_size(n->attrs) > 0) {
            out("\n");
            out("    uint16_t name_index;\n");
        }

        if (array_size(n->children) > 0) {
            out("    uint32_t node_index;\n\n");

            for (int j = 0; j < array_size(n->children); j++) {
                Child *c = array_get(n->children, j);
                out("    if (node->%s != NULL) {\n", c->id);
                out("        node_index = *((int "
                    "*)imap_retrieve(node_indices, node->%s));\n",
                    c->id);
                out("        name_index = %d;\n",
                    *((int *)smap_retrieve(string_pool_indices, c->id)));
                out("        WRITE(2, name_index);\n");
                out("        WRITE(4, node_index);\n");
                out("    }\n");
            }

            out("\n");
        }

        out("\n");

        out("    WRITE(2, attr_count);\n");
        if (array_size(n->attrs) > 0) {
            out("    uint8_t tag;\n\n");

            for (int j = 0; j < array_size(n->attrs); j++) {
                Attr *attr = array_get(n->attrs, j);

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
                    fprintf(stderr, "%s:%s:%d: Invalid attribute type: %d",
                            __FILE__, __func__, __LINE__, attr->type);
                    break;
                }

                out("%sWRITE(2, name_index);\n", indent);
                out("%sWRITE(1, tag);\n", indent);

                switch (attr->type) {
                case AT_int:
                    out("    WRITE(sizeof(int), node->%s);\n", attr->id);
                    break;
                case AT_uint:
                    out("    WRITE(sizeof(unsigned int), node->%s);\n",
                        attr->id);
                    break;
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
                    out("        const int value_%s = *((int*) "
                        "smap_retrieve(attrs_index, node->%s));\n",
                        attr->id, attr->id);
                    out("        WRITE(2, value_%s);\n", attr->id);
                    out("    }\n");
                    break;
                case AT_link:
                    out("        const int value_%s = *((int *) "
                        "imap_retrieve(node_indices, node->%s));\n",
                        attr->id, attr->id);
                    out("        WRITE(4, value_%s);\n", attr->id);
                    out("    }\n");
                    break;
                case AT_enum:
                    out("    const int value_%s = "
                        "get_%s_value_index(node->%s);\n",
                        attr->id, attr->type_id, attr->id);
                    out("    WRITE(2, value_%s);\n", attr->id);
                    break;
                default:
                    break;
                }
            }
        }

        for (int j = 0; j < array_size(n->children); j++) {
            Child *c = array_get(n->children, j);
            out("    gen_node_trav_%s(node->%s, fp);\n", c->type, c->id);
        }

        out("}\n\n");
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        Nodeset *n = array_get(config->nodesets, i);
        out("static void gen_node_trav_%s(%s *nodeset, FILE *fp) {\n", n->id,
            n->id);
        out("    if (nodeset == NULL) return;\n\n");
        out("    switch (nodeset->type) {\n");

        for (int j = 0; j < array_size(n->nodes); j++) {
            Node *child_node = array_get(n->nodes, j);
            out("    case " NS_FORMAT ":\n", n->id, child_node->id);
            out("        gen_node_trav_%s(nodeset->value.val_%s, fp);\n",
                child_node->id, child_node->id);
            out("        break;\n");
        }

        out("    }\n");
        out("}\n\n");
    }
}

static void generate_string_traversals(Config *config, FILE *fp) {
    // Generate declarations
    for (int i = 0; i < array_size(config->nodes); i++) {
        Node *n = array_get(config->nodes, i);
        out("static void attr_string_trav_%s(%s *node);\n", n->id, n->id);
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        Nodeset *n = array_get(config->nodesets, i);
        out("static void attr_string_trav_%s(%s *nodeset);\n", n->id, n->id);
    }
    out("\n");

    for (int i = 0; i < array_size(config->nodes); i++) {
        Node *n = array_get(config->nodes, i);
        out("static void attr_string_trav_%s(%s *node) {\n", n->id, n->id);
        out("    if (node == NULL) return;\n");
        for (int j = 0; j < array_size(n->attrs); j++) {
            Attr *attr = array_get(n->attrs, j);

            if (attr->type == AT_string) {
                out("    if (node->%s != NULL)\n", attr->id);
                out("        array_append(string_attrs, node->%s);\n",
                    attr->id);
            }
        }

        for (int j = 0; j < array_size(n->children); j++) {
            Child *c = array_get(n->children, j);
            out("    attr_string_trav_%s(node->%s);\n", c->type, c->id);
        }

        out("}\n\n");
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        Nodeset *n = array_get(config->nodesets, i);
        out("static void attr_string_trav_%s(%s *nodeset) {\n", n->id, n->id);
        out("    if (nodeset == NULL) return;\n\n");
        out("    switch (nodeset->type) {\n");

        for (int j = 0; j < array_size(n->nodes); j++) {
            Node *child_node = array_get(n->nodes, j);
            out("    case " NS_FORMAT ":\n", n->id, child_node->id);
            out("        attr_string_trav_%s(nodeset->value.val_%s);\n",
                child_node->id, child_node->id);
            out("        break;\n");
        }

        out("   }\n");
        out("}\n\n");
    }
}

static void populate_static_string_pool(Config *config) {
    string_pool_indices = smap_init(32);
    string_pool_constants = array_init(32);

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

    for (int i = 0; i < array_size(config->enums); i++) {
        Enum *e = array_get(config->enums, i);
        out("static int get_%s_value_index(%s e) {\n", e->id, e->id);
        out("    switch (e) {\n");
        for (int j = 0; j < array_size(e->values); j++) {
            char *value = array_get(e->values, j);
            out("    case %s_%s:\n", e->prefix, value);
            out("        return %d;\n",
                *((int *)smap_retrieve(string_pool_indices, value)));
        }
        out("    default:\n");
        out("        return -1;\n");
        out("    }\n");
        out("}\n\n");
    }
}

static void generate_populate_node_index_map(Config *config, FILE *fp) {

    for (int i = 0; i < array_size(config->nodes); i++) {
        Node *n = array_get(config->nodes, i);
        out("static void populate_node_indices_trav_%s(%s *node);\n", n->id,
            n->id);
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        Nodeset *n = array_get(config->nodesets, i);
        out("static void populate_node_indices_trav_%s(%s *nodeset);\n", n->id,
            n->id);
    }

    out("\n");

    for (int i = 0; i < array_size(config->nodes); i++) {
        Node *n = array_get(config->nodes, i);
        out("static void populate_node_indices_trav_%s(%s *node) {\n", n->id,
            n->id);

        out("    if (node == NULL) return;\n\n");
        out("    int *index = mem_alloc(sizeof(int));\n");
        out("    *index = node_index_counter++;\n");
        out("    imap_insert(node_indices, node, index);\n\n");

        for (int j = 0; j < array_size(n->children); j++) {
            Child *c = array_get(n->children, j);
            out("    populate_node_indices_trav_%s(node->%s);\n", c->type,
                c->id);
        }

        out("}\n\n");
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        Nodeset *n = array_get(config->nodesets, i);
        out("static void populate_node_indices_trav_%s(%s *nodeset) {\n",
            n->id, n->id);
        out("    if (nodeset == NULL) return;\n\n");

        out("    // Add the pointer to the nodeset itself with the same index "
            "as the node\n");
        out("    int *index = mem_alloc(sizeof(int));\n");
        out("    *index = node_index_counter;\n");
        out("    imap_insert(node_indices, nodeset, index);\n\n");
        out("    switch (nodeset->type) {\n");

        for (int j = 0; j < array_size(n->nodes); j++) {
            Node *child_node = array_get(n->nodes, j);
            out("    case " NS_FORMAT ":\n", n->id, child_node->id);
            out("        "
                "populate_node_indices_trav_%s(nodeset->value.val_%s);\n",
                child_node->id, child_node->id);
            out("        break;\n");
        }

        out("   }\n");

        out("}\n\n");
    }
}

static void *free_int_index_string(char *key, void *value) {
    mem_free(value);
    return NULL;
}

static void *free_int_index_int(void *key, void *value) {
    mem_free(value);
    return NULL;
}

void generate_binary_serialization_definitions(Config *config, FILE *fp) {
    char *root_node_name =
        config->root_node ? config->root_node->id : config->root_nodeset->id;

    populate_static_string_pool(config);

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <string.h>\n");
    out("#include \"generated/ast.h\"\n");
    out("#include \"framework/serialization-binary-format.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/memory.h\"\n");
    out("\n");

    out("#define WRITE(N, data) do { \\\n"
        "               fwrite(&data, N, 1, fp); \\\n"
        "               fflush(fp); \\\n"
        "              } while(0)\n\n");

    out("static array *string_attrs;\n");
    out("static smap_t *attrs_index;\n\n");
    out("static imap_t *node_indices;\n");
    out("static int node_index_counter = 0;\n");
    out("\n");

    generate_enum_to_index_table(config, fp);

    generate_string_traversals(config, fp);

    generate_node_gen_traversal(config, fp);

    generate_populate_node_index_map(config, fp);

    out("void serialization_write_binfile(%s *syntaxtree, FILE *fp) {\n",
        root_node_name);

    out("    string_attrs = array_init(32);\n");
    out("    attrs_index = smap_init(32);\n");
    out("    node_indices = imap_init(32);\n\n");

    out("    populate_node_indices_trav_%s(syntaxtree);\n\n", root_node_name);

    out("    // Write magic\n");
    out("    uint32_t magic = 0xAC1DC0DE;\n");
    out("    WRITE(4, magic);\n\n");

    // TODO: generate real AST magic
    out("    // Write AST magic\n");
    out("    uint32_t ast_magic = 0xDEADBEEF;\n");
    out("    WRITE(4, ast_magic);\n\n");

    out("    // Write string pool\n\n");
    out("    uint16_t string_pool_count = %d;\n\n",
        array_size(string_pool_constants));

    out("    // Collect string attributes in AST\n");
    out("    attr_string_trav_%s(syntaxtree);\n", root_node_name);
    out("    string_pool_count += array_size(string_attrs);\n");

    out("    // Write string pool count\n");
    out("    WRITE(2, string_pool_count);\n");

    out("\n");
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
    out("\n");
    out("    // Write string attributes\n");
    out("    for (int i = 0; i < array_size(string_attrs); i++) {\n");
    out("        string = array_get(string_attrs, i);\n");
    out("        int *index = mem_alloc(sizeof(int));\n");
    out("        *index = %d + i;\n", array_size(string_pool_constants));
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
    out("    const uint16_t enum_pool_count = %d;\n",
        array_size(config->enums));
    out("    WRITE(2, enum_pool_count);\n");

    out("    uint16_t name_index, prefix_index, values_count, value_index;\n");
    out("\n");

    for (int i = 0; i < array_size(config->enums); i++) {
        Enum *e = array_get(config->enums, i);
        out("    // Enum %s\n", e->id);
        out("    name_index = %d;\n",
            *((int *)smap_retrieve(string_pool_indices, e->id)));
        out("    prefix_index = %d;\n",
            *((int *)smap_retrieve(string_pool_indices, e->prefix)));
        out("    values_count = %d;\n", array_size(e->values));

        out("    WRITE(2, name_index);\n");
        out("    WRITE(2, prefix_index);\n");
        out("    WRITE(2, values_count);\n");
        out("\n");

        for (int j = 0; j < array_size(e->values); j++) {
            char *value = array_get(e->values, j);
            out("    // %s\n", value);
            out("    value_index = %d;\n",
                *((int *)smap_retrieve(string_pool_indices, value)));
            out("    WRITE(2, value_index);\n");
        }
        out("\n");
    }

    out("    WRITE(4, node_index_counter);\n");

    out("    gen_node_trav_%s(syntaxtree, fp);\n", root_node_name);

    out("    smap_free(attrs_index);\n");

    out("}\n");

    array_cleanup(string_pool_constants, NULL);
    smap_map(string_pool_indices, free_int_index_string);
    smap_free(string_pool_indices);

    /* imap_map(node_indices, free_int_index_int); */
    /* imap_free(node_indices); */
}

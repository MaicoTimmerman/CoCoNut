#include <stdio.h>
#include <string.h>

#include "cocogen/ast.h"
#include "cocogen/filegen-driver.h"
#include "cocogen/filegen-util.h"

static void generate_check_attr_type(char *attr_type, FILE *fp, Attr *attr,
                                     Node *node) {

    out("            if (attr->type != AT_%s) {\n", attr_type);
    out("                print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER, "
        "\"%%s: Invalid type %%d for attribute %s "
        "of node %s\", _serialization_read_fn, attr->type);\n",
        attr->id, node->id);
    out("                continue;\n");
    out("            }\n");
}

static void generate_entry_function(FILE *fp, char *id) {
    out("%s *" SERIALIZE_READ_BIN_FORMAT "(char *fn) {\n", id, id);
    out("    FILE *fp = fopen(fn, \"rb\");\n");
    out("    if (fp == NULL) {\n");
    out("        print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER, \"%%s: "
        "%%s\", fn, strerror(errno));\n");
    out("        return NULL;\n");
    out("    }\n\n");
    out("    _serialization_read_fn = fn;\n");
    out("    AstBinFile *file = serialization_read_binfile(fp);\n");
    out("    if (file == NULL) return NULL;\n\n");

    out("    %s *res = _serialization_read_bin_%s(file, 0);\n", id, id);
    out("    return res;\n");

    out("}\n");
}

void generate_binary_serialization_read_node(Config *config, FILE *fp,
                                             Node *node) {

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <string.h>\n");
    out("#include <errno.h>\n");
    out("#include \"generated/ast.h\"\n");
    out("#include \"framework/serialization-binary-format.h\"\n");
    out("#include \"framework/serialization-read-file.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/print.h\"\n");
    out("\n");

    for (int i = 0; i < array_size(node->children); i++) {
        Child *c = array_get(node->children, i);
        out("%s *_serialization_read_bin_%s(AstBinFile *file, uint32_t "
            "node_index);\n",
            c->type, c->type);
    }
    out("\n");

    out("%s *_serialization_read_bin_%s(AstBinFile *file, uint32_t "
        "node_index) {\n",
        node->id, node->id);

    out("    %s *res = mem_alloc(sizeof(%s));\n", node->id, node->id);
    out("    memset(res, 0, sizeof(%s));\n", node->id);
    out("    Node *node = array_get(file->nodes, node_index);\n");
    out("    const char *type = array_get(file->string_pool, "
        "node->type_index);\n");
    out("\n");
    out("    if(strcmp(type, \"%s\") != 0) {\n", node->id);
    out("        print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER, "
        "\"%%s: Type mismatch: expected node type %s, got "
        "%%s\", _serialization_read_fn, type);\n",
        node->id);
    out("        mem_free(res);\n");
    out("        return NULL;\n");
    out("    }\n\n");

    if (array_size(node->children) > 0) {

        out("    for (int i = 0; i < array_size(node->children); i++) {\n");

        out("        Child *c = array_get(node->children, i);\n");
        out("        const char *child_name = array_get(file->string_pool, "
            "c->name_index);\n");

        for (int i = 0; i < array_size(node->children); i++) {
            Child *c = array_get(node->children, i);
            out("        ");
            if (i > 0)
                out("else ");

            out("if (strcmp(child_name, \"%s\") == 0) {\n", c->id);

            out("            %s *child = _serialization_read_bin_%s(file, "
                "c->node_index);\n",
                c->type, c->type);
            out("            res->%s = child;\n", c->id);

            out("        }\n");
        }

        out("        else {\n");
        out("            print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER, "
            "\"%%s: Invalid child %%s of node %s\", "
            "_serialization_read_fn, child_name);\n",
            node->id);
        out("            mem_free(res);\n");
        out("            return NULL;\n");
        out("        }\n");

        out("    }\n");
    }

    if (array_size(node->attrs) > 0) {
        out("    for (int i = 0; i < array_size(node->attributes); i++) {\n");
        out("        Attribute *attr = array_get(node->attributes, i);\n");
        out("        const char *attr_name = array_get(file->string_pool, "
            "attr->name_index);\n");

        for (int i = 0; i < array_size(node->attrs); i++) {
            Attr *attr = array_get(node->attrs, i);
            out("        ");
            if (i > 0)
                out("else ");
            out("if (strcmp(attr_name, \"%s\") == 0) {\n", attr->id);

            switch (attr->type) {
            case AT_int:
                generate_check_attr_type("int", fp, attr, node);
                out("            res->%s = (int) attr->value.val_int.value;\n",
                    attr->id);
                break;
            case AT_uint:
                generate_check_attr_type("uint", fp, attr, node);
                out("            res->%s = (unsigned int) "
                    "attr->value.val_uint.value;\n",
                    attr->id);
                break;
            case AT_int8:
                generate_check_attr_type("int8", fp, attr, node);
                out("            res->%s = attr->value.val_int8.value;\n",
                    attr->id);
                break;
            case AT_int16:
                generate_check_attr_type("int16", fp, attr, node);
                out("            res->%s = attr->value.val_int16.value;\n",
                    attr->id);
                break;
            case AT_int32:
                generate_check_attr_type("int32", fp, attr, node);
                out("            res->%s = attr->value.val_int32.value;\n",
                    attr->id);
                break;
            case AT_int64:
                generate_check_attr_type("int64", fp, attr, node);
                out("            res->%s = attr->value.val_int64.value;\n",
                    attr->id);
                break;
            case AT_uint8:
                generate_check_attr_type("uint8", fp, attr, node);
                out("            res->%s = attr->value.val_uint8.value;\n",
                    attr->id);
                break;
            case AT_uint16:
                generate_check_attr_type("uint16", fp, attr, node);
                out("            res->%s = attr->value.val_uint16.value;\n",
                    attr->id);
                break;
            case AT_uint32:
                generate_check_attr_type("uint32", fp, attr, node);
                out("            res->%s = attr->value.val_uint32.value;\n",
                    attr->id);
                break;
            case AT_uint64:
                generate_check_attr_type("uint64", fp, attr, node);
                out("            res->%s = attr->value.val_uint64.value;\n",
                    attr->id);
                break;
            case AT_float:
                generate_check_attr_type("float", fp, attr, node);
                out("            res->%s = attr->value.val_float.value;\n",
                    attr->id);
                break;
            case AT_double:
                generate_check_attr_type("double", fp, attr, node);
                out("            res->%s = attr->value.val_double.value;\n",
                    attr->id);
                break;
            case AT_bool:
                generate_check_attr_type("bool", fp, attr, node);
                out("            res->%s = attr->value.val_bool.value;\n",
                    attr->id);
                break;
            case AT_string:
                generate_check_attr_type("string", fp, attr, node);
                out("            // TODO: check out of bounds\n");
                out("            res->%s = array_get(file->string_pool, "
                    "attr->value.val_string.value_index);\n",
                    attr->id);
                break;
            case AT_link:
                generate_check_attr_type("link", fp, attr, node);
                out("            // TODO: check out of bounds\n");
                out("            // TODO: set link\n");
                break;
            case AT_enum:
                generate_check_attr_type("enum", fp, attr, node);
                out("            // TODO: set enum\n");
                break;
            default:
                break;
            }

            out("        }\n");
        }

        out("    }\n");
    }

    out("    return res;\n");

    out("}\n\n");

    generate_entry_function(fp, node->id);
}

void generate_binary_serialization_read_nodeset(Config *config, FILE *fp,
                                                Nodeset *nodeset) {

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <string.h>\n");
    out("#include <errno.h>\n");
    out("#include \"generated/ast.h\"\n");
    out("#include \"framework/serialization-binary-format.h\"\n");
    out("#include \"framework/serialization-read-file.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/print.h\"\n");
    out("#include \"generated/ast.h\"\n");
    out("\n");

    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *n = array_get(nodeset->nodes, i);
        out("%s *_serialization_read_bin_%s(AstBinFile *file, uint32_t "
            "node_index);\n",
            n->id, n->id);
    }
    out("\n");

    out("%s *_serialization_read_bin_%s(AstBinFile *file, uint32_t "
        "node_index) {\n",
        nodeset->id, nodeset->id);

    out("    %s *res = mem_alloc(sizeof(%s));\n", nodeset->id, nodeset->id);
    out("    Node *root = array_get(file->nodes, node_index);\n");
    out("    const char *root_type = array_get(file->string_pool, "
        "root->type_index);\n\n");

    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *n = array_get(nodeset->nodes, i);
        out("    ");
        if (i > 0)
            out("else ");

        out("if (strcmp(root_type, \"%s\") == 0) {\n", n->id);
        out("        %s *node_res = _serialization_read_bin_%s(file, "
            "node_index);\n",
            n->id, n->id);
        out("        if (node_res == NULL) {\n");
        out("            mem_free(res);\n");
        out("            return NULL;\n");
        out("        }\n");
        out("        res->type = " NS_FORMAT ";\n", nodeset->id, n->id);
        out("        res->value.val_%s = node_res;\n", n->id);
        out("        return res;\n");
        out("    }\n");
    }

    out("    else {\n");
    out("        print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER, "
        "\"%%s: Invalid root node type for nodeset %s: "
        "%%s\", _serialization_read_fn, root_type);\n",
        nodeset->id);
    out("        mem_free(res);\n");
    out("        return NULL;\n");
    out("    }\n");

    out("}\n");

    generate_entry_function(fp, nodeset->id);

    /*  */
    /* out("%s *" SERIALIZE_READ_BIN_FORMAT "(char *fn) {\n", nodeset->id,
     * nodeset->id); */
    /* out("    FILE *fp = fopen(fn, \"rb\");\n"); */
    /* out("    if (fp == NULL) {\n"); */
    /* out("        print_user_error(SERIALIZE_READ_BIN_ERROR_HEADER, \"%%s:
     * %%s\", fn, strerror(errno));\n"); */
    /* out("        return NULL;\n"); */
    /* out("    }\n\n"); */
    /* out("    _serialization_read_fn = fn;\n"); */
    /* out("    AstBinFile *file = serialization_read_binfile(fp);\n"); */
    /* out("    fclose(fp);\n"); */
    /* out("    if (file == NULL) return NULL;\n\n"); */
    /*  */
    /* out("    %s *res = _serialization_read_bin_%s(file, 0);\n", nodeset->id,
     */
    /*     nodeset->id); */
    /* out("    return res;\n"); */
    /*  */
    /* out("}\n"); */
    /*
    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *node = array_get(nodeset->nodes, i);
        out("void " SERIALIZE_READ_BIN_FORMAT "(char *fn);\n", node->id);
    }
    out("\n");

    out("void " SERIALIZE_READ_BIN_FORMAT "(char *fn) {\n", nodeset->id);

    out("    switch (syntaxtree->type) {\n");

    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *node = array_get(n->nodes, i);

        out("    case " NS_FORMAT ":\n", n->id, node->id);
        out("        " SERIALIZE_WRITE_BIN_FORMAT
            "(syntaxtree->value.val_%s, fn);\n",
            node->id, node->id);
        out("        break;\n");
    }

    out("    }\n");
    out("}\n");
    */
}

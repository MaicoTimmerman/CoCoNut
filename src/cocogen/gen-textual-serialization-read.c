#include <stdio.h>
#include <string.h>

#include "cocogen/ast.h"
#include "cocogen/filegen-driver.h"
#include "cocogen/filegen-util.h"

static void generate_check_attr_type(char *attr_type, FILE *fp, Attr *attr,
                                     Node *node) {

    out("            if (attr->value->type != AST_TXT_%s) {\n", attr_type);
    out("                print_error(attr, \"Invalid value for attribute '%s' "
        "of node type '%s'\");\n",
        attr->id, node->id);
    out("                continue;\n");
    out("            }\n");
}

static void generate_entry_function(FILE *fp, char *id) {
    out("AST_TXT_File *_serialization_txt_parse_file(char *fn);\n\n");
    out("int _serialization_txt_check_file(AST_TXT_File *file);\n");
    out("%s *" SERIALIZE_READ_TXT_FORMAT "(char *fn) {\n", id, id);
    out("    AST_TXT_File *file = _serialization_txt_parse_file(fn);\n");
    out("    if (file == NULL) {\n");
    out("        return NULL;\n");
    out("    }\n\n");

    out("    if (_serialization_txt_check_file(file) != 0) {\n");
    out("        return NULL;\n");
    out("    }\n");
    out("    %s *res = _serialization_read_txt_%s(file, "
        "file->rootnode->id);\n",
        id, id);

    // TODO free file

    out("    return res;\n");

    out("}\n");
}

void generate_textual_serialization_read_node(Config *config, FILE *fp,
                                              Node *node) {

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <string.h>\n");
    out("#include <errno.h>\n");
    out("#include \"generated/ast-%s.h\"\n", node->id);
    out("#include \"generated/free-%s.h\"\n", node->id);
    out("#include \"framework/serialization-read-file.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/print.h\"\n");
    out("\n");

    for (int i = 0; i < array_size(node->children); i++) {
        Child *c = array_get(node->children, i);
        out("%s *_serialization_read_txt_%s(AST_TXT_File *file, uint64_t "
            "node_id);\n",
            c->type, c->type);
    }
    out("\n");

    out("%s *_serialization_read_txt_%s(AST_TXT_File *file, uint64_t "
        "node_id) {\n",
        node->id, node->id);

    out("    bool error = false;\n");
    out("    %s *res = mem_alloc(sizeof(%s));\n", node->id, node->id);
    out("    memset(res, 0, sizeof(%s));\n", node->id);
    out("    AST_TXT_Node *node = imap_retrieve(file->node_id_map, (void*) "
        "node_id);\n");
    out("\n");
    out("    if(strcmp(node->type, \"%s\") != 0) {\n", node->id);
    out("        print_error(node, \"Type mismatch: expected node type '%s', "
        "got '%%s'\", node->type);\n",
        node->id);
    out("        mem_free(res);\n");
    out("        return NULL;\n");
    out("    }\n\n");

    if (array_size(node->children) > 0) {
        out("    for (int i = 0; i < array_size(node->children); i++) {\n");

        out("        AST_TXT_Child *c = array_get(node->children, i);\n");

        for (int i = 0; i < array_size(node->children); i++) {
            Child *c = array_get(node->children, i);
            out("        ");
            if (i > 0)
                out("else ");

            out("if (strcmp(c->name, \"%s\") == 0) {\n", c->id);

            out("            %s *child = _serialization_read_txt_%s(file, "
                "c->id);\n",
                c->type, c->type);
            out("            res->%s = child;\n", c->id);

            out("        }\n");
        }

        out("        else {\n");
        out("        print_error(c, \"Invalid child %%s of node %s\", "
            "c->name);\n",
            node->id);
        out("            mem_free(res);\n");
        out("            return NULL;\n");

        out("        }\n");
        out("    }\n");
    } else {
        out("    if (array_size(node->children) > 0) {\n");
        out("        print_error(node, \"Children specified while node type "
            "'%s' has no children.\");\n",
            node->id);
        out("        error = true;\n");
        out("    }\n");
    }

    if (array_size(node->attrs) > 0) {
        out("    for (int i = 0; i < array_size(node->attributes); i++) {\n");
        out("        AST_TXT_Attribute *attr = array_get(node->attributes, "
            "i);\n");

        for (int i = 0; i < array_size(node->attrs); i++) {
            Attr *attr = array_get(node->attrs, i);
            out("        ");
            if (i > 0)
                out("else ");
            out("if (strcmp(attr->name, \"%s\") == 0) {\n", attr->id);

            switch (attr->type) {
            case AT_int:
                generate_check_attr_type("int", fp, attr, node);
                out("            res->%s = (int) attr->value->data.val_int;\n",
                    attr->id);
                break;
            case AT_uint:
                generate_check_attr_type("uint", fp, attr, node);
                out("            res->%s = (unsigned int) "
                    "attr->value->data.val_uint;\n",
                    attr->id);
                break;
            case AT_int8:
                generate_check_attr_type("int", fp, attr, node);
                out("            res->%s = (int) attr->value->data.val_int;\n",
                    attr->id);
                break;
            case AT_int16:
                generate_check_attr_type("int", fp, attr, node);
                out("            res->%s = (int) attr->value->data.val_int;\n",
                    attr->id);
                break;
            case AT_int32:
                generate_check_attr_type("int", fp, attr, node);
                out("            res->%s = (int) attr->value->data.val_int;\n",
                    attr->id);
                break;
            case AT_int64:
                generate_check_attr_type("int", fp, attr, node);
                out("            res->%s = (int) attr->value->data.val_int;\n",
                    attr->id);
                break;
            case AT_uint8:
                generate_check_attr_type("uint", fp, attr, node);
                out("            res->%s = (unsigned int) "
                    "attr->value->data.val_uint;\n",
                    attr->id);
                break;
            case AT_uint16:
                generate_check_attr_type("uint", fp, attr, node);
                out("            res->%s = (unsigned int) "
                    "attr->value->data.val_uint;\n",
                    attr->id);
                break;
            case AT_uint32:
                generate_check_attr_type("uint", fp, attr, node);
                out("            res->%s = (unsigned int) "
                    "attr->value->data.val_uint;\n",
                    attr->id);
                break;
            case AT_uint64:
                generate_check_attr_type("uint", fp, attr, node);
                out("            res->%s = (unsigned int) "
                    "attr->value->data.val_uint;\n",
                    attr->id);
                break;
            case AT_float:
                generate_check_attr_type("float", fp, attr, node);
                out("            res->%s = (float) "
                    "attr->value->data.val_float;\n",
                    attr->id);
                break;
            case AT_double:
                generate_check_attr_type("float", fp, attr, node);
                out("            res->%s = (double) "
                    "attr->value->data.val_float;\n",
                    attr->id);
                break;
            case AT_bool:
                generate_check_attr_type("bool", fp, attr, node);
                out("            res->%s = attr->value->data.val_bool;\n",
                    attr->id);
                break;
            case AT_string:
                generate_check_attr_type("string", fp, attr, node);
                out("            res->%s = "
                    "strdup(attr->value->data.val_str);\n",
                    attr->id);
                break;
            case AT_link:
                generate_check_attr_type("uint", fp, attr, node);
                /* out("            res->%s = (int)
                 * attr->value.data.val_int;\n", */
                /* attr->id); */
                break;
            case AT_enum:
                generate_check_attr_type("id", fp, attr, node);
                /* out("            res->%s = (int)
                 * attr->value.data.val_int;\n", */
                /*     attr->id); */
                break;
            default:
                break;
            }

            out("        }\n");
        }

        out("    }\n");
    } else {
        out("    if (array_size(node->attributes) > 0) {\n");
        out("        print_error(node, \"Attributes specified while node type "
            "'%s' has no attributes.\");\n",
            node->id);
        out("        error = true;\n");
        out("    }\n");
    }

    out("\n");
    out("    if (error) {\n");
    out("        " FREE_TREE_FORMAT "(res);\n", node->id);
    out("        return NULL;\n");
    out("    }\n");
    out("    return res;\n");

    out("}\n\n");

    generate_entry_function(fp, node->id);
}

void generate_textual_serialization_read_nodeset(Config *config, FILE *fp,
                                                 Nodeset *nodeset) {

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <string.h>\n");
    out("#include <errno.h>\n");
    out("#include \"generated/ast-%s.h\"\n", nodeset->id);
    out("#include \"generated/free-%s.h\"\n", nodeset->id);
    out("#include \"framework/serialization-read-file.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/print.h\"\n");
    out("\n");

    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *n = array_get(nodeset->nodes, i);
        out("%s *_serialization_read_txt_%s(AST_TXT_File *file, uint64_t "
            "node_id);\n",
            n->id, n->id);
    }
    out("\n");

    out("%s *_serialization_read_txt_%s(AST_TXT_File *file, uint64_t "
        "node_id) {\n",
        nodeset->id, nodeset->id);

    out("    %s *res = mem_alloc(sizeof(%s));\n", nodeset->id, nodeset->id);
    out("    memset(res, 0, sizeof(%s));\n", nodeset->id);
    out("    AST_TXT_Node *node = imap_retrieve(file->node_id_map, (void*) "
        "node_id);\n");
    out("\n");

    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *n = array_get(nodeset->nodes, i);
        out("    ");
        if (i > 0)
            out("else ");

        out("if (strcmp(node->type, \"%s\") == 0) {\n", n->id);
        out("        %s *node_res = _serialization_read_txt_%s(file, "
            "node_id);\n",
            n->id, n->id);
        out("        if (node_res == NULL) {\n");
        out("            " FREE_TREE_FORMAT "(res);\n", nodeset->id);
        out("            return NULL;\n");
        out("        }\n");
        out("        res->type = " NS_FORMAT ";\n", nodeset->id, n->id);
        out("        res->value.val_%s = node_res;\n", n->id);
        out("        return res;\n");
        out("    }\n");
    }

    out("    else {\n");
    out("        print_error(node, \"Invalid node type '%%s' for nodeset type "
        "'%s'\", node->type);\n",
        nodeset->id);
    out("        mem_free(res);\n");
    out("        return NULL;\n");
    out("    }\n");

    out("}\n");

    generate_entry_function(fp, nodeset->id);
}

#include <stdio.h>
#include <string.h>

#include "cocogen/ast.h"
#include "cocogen/filegen-driver.h"
#include "cocogen/filegen-util.h"
#include "lib/print.h"

static void generate_entry_function(FILE *fp, char *id) {
    out("void " SERIALIZE_WRITE_TXT_FORMAT "(%s *syntaxtree, char *fn) {\n",
        id, id);
    out("    FILE *fp = fopen(fn, \"w\");\n");
    out("    if (fp == NULL) {\n");
    out("        print_user_error(SERIALIZE_WRITE_TXT_ERROR_HEADER, \"%%s: "
        "cannot open file: %%s\", fn, strerror(errno));\n");
    out("        return;\n");
    out("    }\n\n");

    out("    imap_t *node_ids = imap_init(64);\n");
    out("    _serialization_write_txt_populate_node_ids_%s(syntaxtree, 0, "
        "node_ids);\n",
        id);
    out("    _serialization_write_txt_%s(syntaxtree, fp, true, node_ids);\n",
        id);
    out("    fclose(fp);\n");

    out("}\n");
}

void generate_textual_serialization_write_node(Config *config, FILE *fp,
                                               Node *node) {

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <inttypes.h>\n");
    out("#include <string.h>\n");
    out("#include <errno.h>\n");
    out("#include \"generated/ast-%s.h\"\n", node->id);
    out("#include \"generated/free-%s.h\"\n", node->id);
    out("#include \"generated/textual-serialization-util.h\"\n");
    out("#include \"framework/serialization-txt-ast.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/print.h\"\n");
    out("\n");

    for (int i = 0; i < array_size(node->children); i++) {
        Child *c = array_get(node->children, i);
        out("void _serialization_write_txt_%s(%s *node, FILE *fp, bool "
            "is_root, imap_t *node_ids);\n",
            c->type, c->type);
        out("uint64_t _serialization_write_txt_populate_node_ids_%s(%s *node, "
            "uint64_t node_id_counter, imap_t *node_ids);\n",
            c->type, c->type);
    }
    out("\n");

    out("uint64_t _serialization_write_txt_populate_node_ids_%s(%s *node, "
        "uint64_t node_id_counter, imap_t *node_ids) {\n",
        node->id, node->id);

    out("    if (node == NULL) return node_id_counter;\n");
    out("    node_id_counter++;\n");
    out("    uint64_t *id = mem_alloc(sizeof(uint64_t));\n");
    out("    *id = node_id_counter;\n");
    out("    imap_insert(node_ids, node, id);\n");
    for (int i = 0; i < array_size(node->children); i++) {
        Child *c = array_get(node->children, i);
        out("    node_id_counter = "
            "_serialization_write_txt_populate_node_ids_%s(node->%s, "
            "node_id_counter, node_ids);\n",
            c->type, c->id);
    }
    out("    return node_id_counter;\n");

    out("}\n");

    out("\n");

    out("void _serialization_write_txt_%s(%s *node, FILE *fp, bool is_root, "
        "imap_t *node_ids) {\n",
        node->id, node->id);

    out("    if (node == NULL) return;\n");
    out("    if (is_root)\n");
    out("        fprintf(fp, \"root \");\n");
    out("    else\n");
    out("        fprintf(fp, \"\\n\\n\");\n");
    out("\n");

    out("    uint64_t *id = imap_retrieve(node_ids, node);\n");
    out("    fprintf(fp, \"%s %%\" PRIu64 \"\", *id);\n", node->id);
    out("\n");

    out("    // Children count\n");
    out("    int childcount_total = 0;\n");

    for (int i = 0; i < array_size(node->children); i++) {
        Child *c = array_get(node->children, i);
        out("    if (node->%s != NULL)\n", c->id);
        out("        childcount_total++;\n");
    }

    out("    // Attributes count\n");
    out("    int attrcount_total = 0;\n");

    int attrcount = 0;
    for (int i = 0; i < array_size(node->attrs); i++) {
        Attr *attr = array_get(node->attrs, i);
        if (attr->type != AT_string && attr->type != AT_link)
            attrcount++;
        else {
            out("    if (node->%s != NULL)\n", attr->id);
            out("        attrcount_total++;\n");
        }
    }

    if (attrcount > 0) {
        out("    attrcount_total += %d;\n", attrcount);
        out("    fprintf(fp, \" {\\n\");\n\n");
    } else {
        out("    if (childcount_total == 0 && attrcount_total == 0) {\n");
        out("        fprintf(fp, \";\");\n");
        out("        return;\n");
        out("    } else {\n");
        out("        fprintf(fp, \" {\\n\");");
        out("    }\n\n");
    }

    out("    // Write children\n");
    out("    if (childcount_total > 0) {\n");
    out("        bool child_set = false;\n");
    out("        fprintf(fp, \"    children {\\n\");\n");
    for (int i = 0; i < array_size(node->children); i++) {
        Child *c = array_get(node->children, i);
        out("        if (node->%s != NULL) {\n", c->id);
        out("            if (child_set)\n");
        out("                fprintf(fp, \",\\n\");\n");

        out("            uint64_t *id = imap_retrieve(node_ids, node->%s);\n",
            c->id);
        out("            fprintf(fp, \"        %s = %%\" PRIu64 , *id);\n",
            c->id);
        out("            child_set = true;\n");
        out("        }\n");
        out("\n");
    }
    out("        fprintf(fp, \"\\n    }\");\n");
    out("    }\n");

    out("\n");
    out("    // Write attributes\n");
    out("    if (attrcount_total > 0) {\n");
    out("        bool attr_set = false;\n");
    out("        if (childcount_total > 0)\n");
    out("            fprintf(fp, \",\\n\");\n");

    out("        fprintf(fp, \"    attributes {\\n\");\n");
    for (int i = 0; i < array_size(node->attrs); i++) {
        Attr *attr = array_get(node->attrs, i);

        if (i > 0 && attr->type != AT_string && attr->type != AT_link) {
            out("        if (attr_set) {\n");
            out("            fprintf(fp, \",\\n\");\n");
            out("            attr_set = false;\n");
            out("        }\n");
        }

        switch (attr->type) {
        case AT_int:
            out("        fprintf(fp, \"        %s = %%d\", node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_uint:
            out("        fprintf(fp, \"        %s = %%u\", node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_int8:
            out("        fprintf(fp, \"        %s = %%\" PRId8, node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_int16:
            out("        fprintf(fp, \"        %s = %%\" PRId16, node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_int32:
            out("        fprintf(fp, \"        %s = %%\" PRId32, node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_int64:
            out("        fprintf(fp, \"        %s = %%\" PRId64, node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_uint8:
            out("        fprintf(fp, \"        %s = %%\" PRIu8, node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_uint16:
            out("        fprintf(fp, \"        %s = %%\" PRIu16, node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_uint32:
            out("        fprintf(fp, \"        %s = %%\" PRIu32, node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_uint64:
            out("        fprintf(fp, \"        %s = %%\" PRIu64, node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_float:
        case AT_double:
            out("        fprintf(fp, \"        %s = %%f\", node->%s);\n",
                attr->id, attr->id);
            break;
        case AT_bool:
            out("        fprintf(fp, \"        %s = %%s\", node->%s ? "
                "\"true\" : \"false\");\n",
                attr->id, attr->id);
            break;
        case AT_string:
            // TODO: escape string
            out("        if (node->%s != NULL) {\n", attr->id);
            if (i > 0) {
                out("            if (attr_set) {\n");
                out("                fprintf(fp, \",\\n\");\n");
                out("                attr_set = false;\n");
                out("            }\n");
            }
            out("            fprintf(fp, \"        %s = \\\"%%s\\\"\", "
                "node->%s);\n",
                attr->id, attr->id);
            out("            attr_set = true;\n");
            out("        }\n");
            break;
        case AT_enum:
            out("        fprintf(fp, \"        %s = %%s\", "
                "_serialization_txt_%s_to_string(node->%s));\n",
                attr->id, attr->type_id, attr->id);
            break;
        case AT_link:
            out("        if (node->%s != NULL) {\n", attr->id);
            if (i > 0) {
                out("            if (attr_set) {\n");
                out("                fprintf(fp, \",\\n\");\n");
                out("                attr_set = false;\n");
                out("            }\n");
            }
            out("            uint64_t *link_id_%s = imap_retrieve(node_ids, "
                "node->%s);\n",
                attr->id, attr->id);
            out("            fprintf(fp, \"        %s = %%\" PRIu64, "
                "*link_id_%s);\n",
                attr->id, attr->id);
            out("            attr_set = true;\n");
            out("        }\n");
            break;
        default:
            print_internal_error("Invalid attribute type: %d\n", attr->type);
            break;
        }

        if (attr->type != AT_string && attr->type != AT_link) {
            out("        attr_set = true;\n");
        }
        out("\n");
    }

    out("        fprintf(fp, \"\\n    }\");\n");
    out("    }\n");

    out("    fprintf(fp, \"\\n};\");\n\n");

    for (int i = 0; i < array_size(node->children); i++) {
        Child *c = array_get(node->children, i);
        out("    _serialization_write_txt_%s(node->%s, fp, false, "
            "node_ids);\n",
            c->type, c->id);
    }

    out("}\n\n");

    generate_entry_function(fp, node->id);
}

void generate_textual_serialization_write_nodeset(Config *config, FILE *fp,
                                                  Nodeset *nodeset) {

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <inttypes.h>\n");
    out("#include <string.h>\n");
    out("#include <errno.h>\n");
    out("#include \"generated/ast-%s.h\"\n", nodeset->id);
    out("#include \"generated/free-%s.h\"\n", nodeset->id);
    out("#include \"generated/textual-serialization-util.h\"\n");
    out("#include \"framework/serialization-txt-ast.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/memory.h\"\n");
    out("#include \"lib/print.h\"\n");
    out("\n");

    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *n = array_get(nodeset->nodes, i);
        out("void _serialization_write_txt_%s(%s *node, FILE *fp, bool "
            "is_root, imap_t *node_ids);\n",
            n->id, n->id);
        out("uint64_t _serialization_write_txt_populate_node_ids_%s(%s *node, "
            "uint64_t node_id_counter, imap_t *node_ids);\n",
            n->id, n->id);
    }
    out("\n");

    out("uint64_t _serialization_write_txt_populate_node_ids_%s(%s *nodeset, "
        "uint64_t node_id_counter, imap_t *node_ids) {\n",
        nodeset->id, nodeset->id);

    out("    if (nodeset == NULL) return node_id_counter;\n");
    out("    uint64_t *id = mem_alloc(sizeof(uint64_t));\n");
    out("    *id = node_id_counter + 1;\n");
    out("    imap_insert(node_ids, nodeset, id);\n");

    out("    switch (nodeset->type) {\n");
    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *n = array_get(nodeset->nodes, i);

        out("    case " NS_FORMAT ":\n", nodeset->id, n->id);

        out("        return "
            "_serialization_write_txt_populate_node_ids_%s(nodeset->value.val_"
            "%s, node_id_counter, node_ids);\n",
            n->id, n->id);
    }

    out("    default:\n");
    out("        print_internal_error(\"Nodeset '%s' has invalid type: "
        "%%d\\n\", nodeset->type);\n",
        nodeset->id);
    out("        return node_id_counter;\n");
    out("    }\n");

    out("}\n\n");

    out("void _serialization_write_txt_%s(%s *nodeset, FILE *fp, bool "
        "is_root, imap_t *node_ids) {\n",
        nodeset->id, nodeset->id);

    out("    if (nodeset == NULL) return;\n");

    out("    switch (nodeset->type) {\n");
    for (int i = 0; i < array_size(nodeset->nodes); i++) {
        Node *n = array_get(nodeset->nodes, i);

        out("    case " NS_FORMAT ":\n", nodeset->id, n->id);

        out("        _serialization_write_txt_%s(nodeset->value.val_%s, fp, "
            "is_root, node_ids);\n",
            n->id, n->id);
        out("        break;\n");
    }

    out("    default:\n");
    out("        print_internal_error(\"Nodeset '%s' has invalid type: "
        "%%d\\n\", nodeset->type);\n",
        nodeset->id);
    out("    }\n");

    out("}\n");

    generate_entry_function(fp, nodeset->id);
}

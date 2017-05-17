#include <stdio.h>
#include <string.h>

#include "astgen/ast.h"
#include "astgen/filegen-driver.h"
#include "astgen/filegen-util.h"

void generate_binary_serialization_read_node(Config *config, FILE *fp,
                                             Node *node) {

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <string.h>\n");
    out("#include \"generated/ast.h\"\n");
    out("#include \"framework/serialization-binary-format.h\"\n");
    out("#include \"framework/serialization-read-file.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/memory.h\"\n");
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
    out("        fprintf(stderr, \"Type mismatch: expected node type %s, got "
        "%%s\\n\", type);\n",
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
        out("            fprintf(stderr, \"Invalid child %%s of node %s\\n\", "
            "child_name);\n",
            node->id);
        out("            mem_free(res);\n");
        out("            return NULL;\n");
        out("        }\n");

        out("    }\n");
    }
    out("    return res;\n");

    out("}\n\n");

    out("%s *" SERIALIZE_READ_BIN_FORMAT "(FILE *fp) {\n", node->id, node->id);
    out("    AstBinFile *file = serialization_read_binfile(fp);\n");
    out("    if (file == NULL) return NULL;\n\n");

    out("    %s *res = _serialization_read_bin_%s(file, 0);\n", node->id,
        node->id);
    out("    return res;\n");

    out("}\n");
}

void generate_binary_serialization_read_nodeset(Config *config, FILE *fp,
                                                Nodeset *nodeset) {

    out("#include <stdio.h>\n");
    out("#include <stdint.h>\n");
    out("#include <string.h>\n");
    out("#include \"generated/ast.h\"\n");
    out("#include \"framework/serialization-binary-format.h\"\n");
    out("#include \"framework/serialization-read-file.h\"\n");
    out("#include \"lib/array.h\"\n");
    out("#include \"lib/imap.h\"\n");
    out("#include \"lib/smap.h\"\n");
    out("#include \"lib/memory.h\"\n");
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
    out("        fprintf(stderr, \"Invalid root node type for nodeset %s: "
        "%%s\\n\", root_type);\n",
        nodeset->id);
    out("        mem_free(res);\n");
    out("        return NULL;\n");
    out("    }\n");

    out("}\n");
}

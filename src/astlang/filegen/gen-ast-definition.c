#include "ast.h"
#include "filegen-driver.h"
#include "memory.h"
#include "str-ast.h"
#include <stdio.h>

struct Info {
    FILE *fp;
};

static struct Info *create_info(void) {

    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));
    info->fp = NULL;

    return info;
}

static void free_info(struct Info *info) {
    mem_free(info);
}

static void template_enum(struct Enum *arg_enum, struct Info *info) {
    out("typedef enum {\n");
    for (int i = 0; i < array_size(arg_enum->values); i++) {
        out("    %s_%s,\n", arg_enum->prefix,
            (char *)array_get(arg_enum->values, i));
    }
    out("} %s;\n\n", arg_enum->id);
}

static void template_ast_h(struct Config *config, struct Info *info) {
    // First do forward declaration of all the structs.
    out("\n// Forward declarations of nodes\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        out("struct %s;\n", ((struct Node *)array_get(config->nodes, i))->id);
    }
    out("\n// Forward declarations of nodesets\n");
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        out("struct %s;\n",
            ((struct Nodeset *)array_get(config->nodesets, i))->id);
    }

    // Print node structs
    out("\n// Definitions of nodes\n");
    for (int i = 0; i < array_size(config->nodes); ++i) {
        struct Node *node = (struct Node *)array_get(config->nodes, i);
        out("typedef struct %s {\n", node->id);

        if (node->children) {
            for (int j = 0; j < array_size(node->children); ++j) {
                struct Child *child =
                    (struct Child *)array_get(node->children, j);
                out("    struct %s *%s;\n", child->type, child->id);
            }
        }
        if (node->attrs) {
            for (int j = 0; j < array_size(node->attrs); ++j) {
                struct Attr *attr = (struct Attr *)array_get(node->attrs, j);
                out("    %s %s;\n", str_attr_type(attr->type, attr->type_id),
                    attr->id);
            }
        }
        out("} %s;\n\n", node->id);
    }

    // typedef enum {
    //     NS_expr_binop, NS_expr_monop, NS_expr_assign
    // } expr;
    out("\n// Definitons of nodesets\n");
    for (int i = 0; i < array_size(config->nodesets); ++i) {
        struct Nodeset *nodeset =
            (struct Nodeset *)array_get(config->nodesets, i);
        out("// Nodeset %s\n", nodeset->id);
        out("typedef enum { ");
        for (int j = 0; j < array_size(nodeset->nodes); ++j) {
            out("NS_%s_%s, ", nodeset->id,
                (char *)array_get(nodeset->nodes, j));
        }
        out("} NS_%s_enum;\n", nodeset->id);

        out("typedef struct %s {\n", nodeset->id);
        out("    union {\n");
        for (int j = 0; j < array_size(nodeset->nodes); ++j) {
            char *node_name = (char *)array_get(nodeset->nodes, j);
            out("        struct %s *val_%s;\n", node_name, node_name);
        }
        out("    } value;\n");
        out("    NS_%s_enum type;\n", nodeset->id);
        out("} %s;\n\n", nodeset->id);
    }
}

void generate_enum_definitions(struct Config *config, FILE *fp) {
    struct Info *info = create_info();
    info->fp = fp;

    out("#pragma once\n");
    for (int i = 0; i < array_size(config->enums); i++) {
        template_enum((struct Enum *)array_get(config->enums, i), info);
    }

    free_info(info);
}

void generate_ast_definitions(struct Config *config, FILE *fp) {
    struct Info *info = create_info();
    info->fp = fp;

    out("#pragma once\n");
    out("#include \"enum.h\"\n");
    template_ast_h(config, info);

    free_info(info);
}

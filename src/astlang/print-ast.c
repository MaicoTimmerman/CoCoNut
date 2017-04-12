#include "array.h"
#include "ast-internal.h"
#include <stdio.h>

static void printPhase(struct Phase *phase) {}

static void printTraversal(struct Traversal *traversal) {
    printf("traversal %s", traversal->id);
    if (traversal->nodes == NULL)
        printf(";\n\n");
    else {
        printf("\n    nodes {\n");
        int num_nodes = array_size(traversal->nodes);
        for (int i = 0; i < num_nodes; i++) {
            printf("        %s", (char *)array_get(traversal->nodes, i));
            if (i < num_nodes - 1)
                printf(",\n");
            else
                printf("\n");
        }
        printf("    }\n};\n\n");
    }
}

static void printEnum(struct Enum *attr_enum) {
    printf("enum %s {\n", attr_enum->id);
    int num_values = array_size(attr_enum->values);
    for (int i = 0; i < num_values; i++) {
        printf("    %s", (char *)array_get(attr_enum->values, i));
        if (i < num_values - 1)
            printf(",\n");
        else
            printf("\n");
    }
    printf("};\n\n");
}

static void printNodeset(struct Nodeset *nodeset) {
    printf("nodeset %s {\n", nodeset->id);
    int num_nodes = array_size(nodeset->nodes);
    for (int i = 0; i < num_nodes; i++) {
        printf("    %s", (char *)array_get(nodeset->nodes, i));
        if (i < num_nodes - 1)
            printf(",\n");
        else
            printf("\n");
    }
    printf("};\n\n");
}

static void printChild(struct Child *c) {
    printf("        ");
    if (c->construct)
        printf("construct ");

    if (c->mandatory) {
        printf("mandatory");
        if (c->mandatory_phases == NULL)
            printf(" ");
        else {
            printf("[");
            int num_mandatory_phases = array_size(c->mandatory_phases);
            for (int j = 0; j < num_mandatory_phases; j++) {

                struct MandatoryPhase *p = array_get(c->mandatory_phases, j);

                if (p->type == MP_single) {
                    printf(p->value.single);
                } else {
                    struct PhaseRange *range = p->value.range;
                    printf("%s to %s", range->start, range->end);
                }

                if (j < num_mandatory_phases - 1)
                    printf(", ");
            }
            printf("] ");
        }
    }

    printf("%s %s", c->type, c->id);
}

static void printAttr(struct Attr *a) {
    printf("        ");
    if (a->construct)
        printf("construct ");

    switch (a->type) {
    case AT_char:
        printf("char ");
        break;
    case AT_uchar:
        printf("unsigned char ");
        break;
    case AT_short:
        printf("short ");
        break;
    case AT_ushort:
        printf("unsigned short ");
        break;
    case AT_int:
        printf("int ");
        break;
    case AT_uint:
        printf("unsigned int ");
        break;
    case AT_long:
        printf("long ");
        break;
    case AT_ulong:
        printf("unsigned long ");
        break;
    case AT_longlong:
        printf("long long ");
        break;
    case AT_ulonglong:
        printf("unsigned long long ");
        break;
    case AT_float:
        printf("float ");
        break;
    case AT_double:
        printf("double ");
        break;
    case AT_longdouble:
        printf("long double ");
        break;
    case AT_string:
        printf("string ");
        break;
    case AT_link_or_enum:
        printf("%s ", a->type_id);
        break;
    }

    printf(a->id);
    if (a->default_value != NULL) {

        printf(" = ");

        switch (a->default_value->type) {
        case AV_string:
            printf("\"%s\"", (char *)(a->default_value->value));
            break;
        case AV_int:
            printf("%lld", *((long long *)(a->default_value->value)));
            break;
        case AV_float:
            printf("%Lf", *((long double *)(a->default_value->value)));
            break;
        case AV_char:
            printf("'%c'", *((char *)(a->default_value->value)));
            break;
        case AV_id:
            printf("%s", (char *)(a->default_value->value));
            break;
        }
        return;
    }

    if (a->type == AT_link_or_enum) {
        printf(" = NULL");
    }
}

static void printFlag(struct Flag *f) {
    printf("        ");
    if (f->construct)
        printf("construct ");

    printf("%s", f->id);
    if (f->default_value != -1) {
        printf(" = %s", f->default_value ? "true" : "false");
    }
}

static void printNode(struct Node *node) {
    int previous_block = 0;

    if (node->children != NULL) {
        printf("node %s {\n", node->id);

        printf("    children {\n");
        int num_children = array_size(node->children);
        for (int i = 0; i < num_children; i++) {
            struct Child *c = array_get(node->children, i);
            printChild(c);
            if (i < num_children - 1)
                printf(",\n");
            else
                printf("\n");
        }
        printf("    }");

        previous_block = 1;
    }

    if (node->attrs) {
        if (previous_block)
            printf(",\n");

        printf("    attributes {\n");

        int num_attrs = array_size(node->attrs);
        for (int i = 0; i < num_attrs; i++) {
            struct Attr *attr = array_get(node->attrs, i);
            printAttr(attr);
            if (i < num_attrs - 1)
                printf(",\n");
            else
                printf("\n");
        }

        printf("    }");

        previous_block = 1;
    }

    if (node->flags) {
        if (previous_block)
            printf(",\n");

        printf("    flags {\n");

        int num_flags = array_size(node->flags);
        for (int i = 0; i < num_flags; i++) {
            struct Flag *f = array_get(node->flags, i);
            printFlag(f);
            if (i < num_flags - 1)
                printf(",\n");
            else
                printf("\n");
        }

        printf("    }");

        previous_block = 1;
    }

    if (previous_block)
        printf("\n");

    printf("};\n\n");
}

void printConfigAST(struct Config *config) {
    for (int i = 0; i < array_size(config->phases); i++) {
        printPhase(array_get(config->phases, i));
    }

    for (int i = 0; i < array_size(config->traversals); i++) {
        printTraversal(array_get(config->traversals, i));
    }

    for (int i = 0; i < array_size(config->enums); i++) {
        printEnum(array_get(config->enums, i));
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        printNodeset(array_get(config->nodesets, i));
    }

    for (int i = 0; i < array_size(config->nodes); i++) {
        printNode(array_get(config->nodes, i));
    }
}

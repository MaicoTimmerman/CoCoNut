#include "array.h"
#include "ast-internal.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
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
    case AT_int:
        printf("int");
        break;
    case AT_uint:
        printf("uint");
        break;
    case AT_int8:
        printf("int8");
        break;
    case AT_int16:
        printf("int16");
        break;
    case AT_int32:
        printf("int32");
        break;
    case AT_int64:
        printf("int64");
        break;
    case AT_uint8:
        printf("uint8");
        break;
    case AT_uint16:
        printf("uint16");
        break;
    case AT_uint32:
        printf("uint32");
        break;
    case AT_uint64:
        printf("uint64");
        break;
    case AT_float:
        printf("float");
        break;
    case AT_double:
        printf("double");
        break;
    case AT_bool:
        printf("bool");
        break;
    case AT_string:
        printf("string");
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
            printf("\"%s\"", a->default_value->value.string_value);
            break;
        case AV_id:
            printf("%s", a->default_value->value.string_value);
            break;
        case AV_int:
            printf("%" PRId64, a->default_value->value.int_value);
            break;
        case AV_uint:
            printf("%" PRIu64, a->default_value->value.uint_value);
            break;
        case AV_float:
            printf("%f", a->default_value->value.float_value);
            break;
        case AV_double:
            printf("%f", a->default_value->value.double_value);
            break;
        case AV_bool:
            printf(a->default_value->value.bool_value ? "true" : "false");
            break;
        }
        return;
    }

    if (a->type == AT_link_or_enum) {
        printf(" = NULL");
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
    }

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

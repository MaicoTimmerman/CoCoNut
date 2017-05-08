#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "astgen/ast.h"
#include "lib/array.h"

#define IND "    "
#define IND2 IND IND
#define IND3 IND IND IND
#define IND4 IND IND IND IND
#define IND5 IND IND IND IND IND

static void print_phase(struct Phase *phase) {
    if (phase->root)
        printf("root ");
    if (phase->cycle)
        printf("cycle ");

    printf("phase %s {\n", phase->id);

    if (phase->info)
        printf(IND "info = %s,\n", phase->info);

    array *elems;
    if (phase->type == PH_subphases) {
        printf(IND "subphases {\n");
        elems = phase->subphases;
    } else {
        printf(IND "passes {\n");
        elems = phase->passes;
    }

    int num_elems = array_size(elems);
    for (int i = 0; i < num_elems; i++) {
        printf(IND2 "%s", (char *)array_get(elems, i));
        if (i < num_elems - 1)
            printf(",\n");
        else
            printf("\n");
    }

    printf(IND "}\n};\n\n");
}

static void print_pass(struct Pass *pass) {
    printf("pass %s", pass->id);
    if (pass->func == NULL) {
        if (pass->info == NULL) {
            printf(";\n\n");
        } else {
            printf(" {\n");
            printf(IND "info = %s\n", pass->info);
            printf("};\n\n");
        }
    } else {
        printf(" {\n");
        printf(IND "func = %s", pass->func);
        if (pass->info) {
            printf(",\n");
            printf(IND "info = %s\n", pass->info);
        } else {
            printf("\n");
        }
        printf("};\n\n");
    }
}

static void print_traversal(struct Traversal *traversal) {
    printf("traversal %s", traversal->id);
    if (traversal->nodes == NULL)
        printf(";\n\n");
    else {
        printf(" {\n" IND "nodes {\n");
        int num_nodes = array_size(traversal->nodes);
        for (int i = 0; i < num_nodes; i++) {
            printf(IND2 "%s", (char *)array_get(traversal->nodes, i));
            if (i < num_nodes - 1)
                printf(",\n");
            else
                printf("\n");
        }
        printf(IND "}\n};\n\n");
    }
}

static void print_enum(struct Enum *attr_enum) {
    printf("enum %s {\n", attr_enum->id);
    printf(IND "prefix = %s,\n", attr_enum->prefix);
    printf(IND "values {\n");

    int num_values = array_size(attr_enum->values);
    for (int i = 0; i < num_values; i++) {
        printf(IND2 "%s", (char *)array_get(attr_enum->values, i));
        if (i < num_values - 1)
            printf(",\n");
        else
            printf("\n");
    }
    printf(IND "}\n};\n\n");
}

static void print_nodeset(struct Nodeset *nodeset) {
    printf("nodeset %s {\n", nodeset->id);
    int num_nodes = array_size(nodeset->nodes);
    for (int i = 0; i < num_nodes; i++) {
        printf(IND "%s", ((struct Node *)array_get(nodeset->nodes, i))->id);
        if (i < num_nodes - 1)
            printf(",\n");
        else
            printf("\n");
    }
    printf("};\n\n");
}

static void print_child(struct Child *c) {
    printf(IND2 "child %s %s", c->type, c->id);
    if (c->construct || c->mandatory) {
        printf(" {\n");

        if (c->construct)
            printf(IND3 "construct%s\n", c->mandatory ? "," : "");

        if (c->mandatory) {
            if (c->mandatory_phases == NULL)
                printf(IND3 "mandatory\n");
            else {
                printf(IND3 "mandatory {\n");
                int num_mandatory_phases = array_size(c->mandatory_phases);
                for (int j = 0; j < num_mandatory_phases; j++) {

                    struct MandatoryPhase *p =
                        array_get(c->mandatory_phases, j);

                    if (p->type == MP_single) {

                        if (p->negation)
                            printf(IND4 "!%s", p->value.single);
                        else
                            printf(IND4 "%s", p->value.single);
                    } else {
                        struct PhaseRange *range = p->value.range;

                        if (p->negation)
                            printf(IND4 "!(%s to %s)", range->start,
                                   range->end);
                        else
                            printf(IND4 "%s to %s", range->start, range->end);
                    }

                    if (j < num_mandatory_phases - 1)
                        printf(",\n");
                }
                printf("\n" IND3 "}\n");
            }
        }

        printf(IND2 "}");
    }
}

static void print_attr(struct Attr *a) {
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
    case AT_link:
    case AT_enum:
    case AT_link_or_enum:
        printf("%s", a->type_id);
        break;
    }

    printf(" %s", a->id);
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

static void print_node(struct Node *node) {
    int previous_block = 0;

    if (node->root)
        printf("root ");

    printf("node %s {\n", node->id);

    if (node->info) {
        printf("info = %s,\n", node->info);
    }

    if (node->children) {

        printf(IND "children {\n");
        int num_children = array_size(node->children);
        for (int i = 0; i < num_children; i++) {
            struct Child *c = array_get(node->children, i);
            print_child(c);
            if (i < num_children - 1)
                printf(",\n");
            else
                printf("\n");
        }
        printf(IND "}");

        previous_block = 1;
    }

    if (node->attrs) {
        if (previous_block)
            printf(",\n");

        printf(IND "attributes {\n");

        int num_attrs = array_size(node->attrs);
        for (int i = 0; i < num_attrs; i++) {
            struct Attr *attr = array_get(node->attrs, i);
            print_attr(attr);
            if (i < num_attrs - 1)
                printf(",\n");
            else
                printf("\n");
        }

        printf(IND "}\n");
    }

    printf("};\n\n");
}

void print_config(struct Config *config) {
    for (int i = 0; i < array_size(config->phases); i++) {
        print_phase(array_get(config->phases, i));
    }

    for (int i = 0; i < array_size(config->passes); i++) {
        print_pass(array_get(config->passes, i));
    }

    for (int i = 0; i < array_size(config->traversals); i++) {
        print_traversal(array_get(config->traversals, i));
    }

    for (int i = 0; i < array_size(config->enums); i++) {
        print_enum(array_get(config->enums, i));
    }

    for (int i = 0; i < array_size(config->nodesets); i++) {
        print_nodeset(array_get(config->nodesets, i));
    }

    for (int i = 0; i < array_size(config->nodes); i++) {
        print_node(array_get(config->nodes, i));
    }
}

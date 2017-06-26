#include <stdio.h>
#include <string.h>

#include "cocogen/ast.h"
#include "cocogen/filegen-driver.h"
#include "cocogen/filegen-util.h"
#include "lib/print.h"

void generate_textual_serialization_util(Config *config, FILE *fp) {
    out("#include <string.h>\n");
    out("#include \"generated/enum.h\"\n\n");

    for (int i = 0; i < array_size(config->enums); i++) {
        Enum *e = array_get(config->enums, i);
        out("char *_serialization_txt_%s_to_string(%s value) {\n", e->id,
            e->id);
        out("    switch (value) {\n");
        for (int j = 0; j < array_size(e->values); j++) {
            char *value = array_get(e->values, j);
            out("        case %s_%s:\n", e->prefix, value);
            out("            return \"%s_%s\";\n", e->prefix, value);
        }
        out("    }\n");
        out("    return \"\";\n");
        out("}\n\n");
    }

    for (int i = 0; i < array_size(config->enums); i++) {
        Enum *e = array_get(config->enums, i);
        out("%s _serialization_txt_string_to_%s(char *value) {\n", e->id,
            e->id);
        for (int j = 0; j < array_size(e->values); j++) {
            char *value = array_get(e->values, j);

            out("    ");
            if (j > 0)
                out("else ");

            out("if (strcmp(value, \"%s_%s\") == 0)\n", e->prefix, value);
            out("        return %s_%s;\n", e->prefix, value);
        }
        out("    return -1;\n");
        out("}\n\n");
    }
}

void generate_textual_serialization_util_header(Config *config, FILE *fp) {
    out("#include \"generated/enum.h\"\n\n");
    for (int i = 0; i < array_size(config->enums); i++) {
        Enum *e = array_get(config->enums, i);
        out("char *_serialization_txt_%s_to_string(%s value);\n", e->id,
            e->id);
        out("%s _serialization_txt_string_to_%s(char *value);\n", e->id,
            e->id);
    }
}

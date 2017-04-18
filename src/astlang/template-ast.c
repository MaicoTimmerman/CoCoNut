#include <stdio.h>
#include "ast.h"
#include "memory.h"
#include "template-ast.h"

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

static void enum_definition(struct Enum *arg_enum, struct Info *info) {
    fprintf(info->fp, "\n");
    fprintf(info->fp, "enum = {\n");
    fprintf(info->fp, "    name = %s;\n", arg_enum->id);
    fprintf(info->fp, "    prefix = %s;\n", arg_enum->prefix);
    for (int i = 0; i < array_size(arg_enum->values); i++) {
        fprintf(info->fp, "    value = %s;\n",
                (char*)array_get(arg_enum->values, i));
    }
    fprintf(info->fp, "};\n");
}

void generate_definitions(struct Config *config) {
    struct Info *info = create_info();

    info->fp = fopen("./src/templates/enum.def", "w");
    /* info->fp = stdout; */
    fprintf(info->fp, "autogen definitions enum;\n");
    for (int i = 0; i < array_size(config->enums); i++) {
        enum_definition((struct Enum *)array_get(config->enums, i), info);
    }
    fclose(info->fp);

    free_info(info);
}

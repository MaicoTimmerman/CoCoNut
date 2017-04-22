#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "check-ast.h"
#include "create-ast.h"
#include "filegen-driver.h"
#include "free-ast.h"
#include "print-ast.h"

#include "gen-ast-definition.h"
#include "gen-copy-functions.h"
#include "gen-create-functions.h"
#include "gen-free-functions.h"
#include "gen-trav-functions.h"
#include "print-ast.h"

extern struct Config *parse(FILE *fp);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s [file]\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        fprintf(stderr, "%s: cannot open file: %s\n", argv[1],
                strerror(errno));
        return 1;
    }

    struct Config *parse_result = parse(f);

    fclose(f);

    if (check_config(parse_result)) {
        fprintf(stderr, "\n\nFound errors\n");
        exit(1);
    }
    /*print_config(parse_result);*/

    // TODO read from command line
    char *out_dir = "src/generated/";

    filegen_init(out_dir);
    filegen_add("enum.h", generate_enum_definitions);
    filegen_add("ast.h", generate_ast_definitions);

    filegen_add("free-ast.h", generate_free_header);
    filegen_add("free-ast.c", generate_free_definitions);
    filegen_add("create-ast.c", generate_create_functions);
    filegen_add("create-ast.h", generate_create_function_headers);

    filegen_add("trav-ast.h", generate_trav_header);
    filegen_add("trav-ast.c", generate_trav_definitions);

    filegen_add("copy-ast.h", generate_copy_header);
    filegen_add("copy-ast.c", generate_copy_definitions);

    int ret = filegen_generate(parse_result);
    filegen_cleanup();

    free_config(parse_result);

    return ret;
}

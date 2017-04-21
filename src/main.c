#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "check-ast.h"
#include "create-ast.h"
#include "filegen-driver.h"
#include "free-ast.h"
#include "gen-ast-definition.h"
#include "gen-free-functions.h"
#include "print-ast.h"

extern struct Config *parse(void);

int main() {
    struct Config *parse_result = parse();

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

    int ret = filegen_generate(parse_result);
    filegen_cleanup();

    free_config(parse_result);

    return ret;
}

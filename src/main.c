#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "check-ast.h"
#include "create-ast.h"
#include "free-ast.h"
#include "print-ast.h"

extern struct Config *parse(void);

int main() {
    struct Config *parse_result = parse();

    if (check_config(parse_result)) {
        fprintf(stderr, "\n\nFound errors\n");
        exit(1);
    }

    print_config(parse_result);
    free_config(parse_result);

    return 0;
}

#include "ast-internal.h"
#include "free-ast.h"
#include "print-ast.h"

extern struct Config *parse(void);

int main() {
    struct Config *parse_result = parse();

    print_config(parse_result);
    free_config(parse_result);

    return 0;
}

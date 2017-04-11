#include "ast-internal.h"
#include "print-ast.h"

extern struct Config* parse(void);

int main() {
    struct Config* parse_result = parse();
    printConfigAST(parse_result); 

    return 0;
}

#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};
struct Info *RenameFor_createinfo(void) {
    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));
    return info;
}
void RenameFor_freeinfo(struct Info *info) {}
void RenameFor_For(For *node, struct Info *info) {}
void RenameFor_VarLet(VarLet *node, struct Info *info) {}
void RenameFor_Var(Var *node, struct Info *info) {}

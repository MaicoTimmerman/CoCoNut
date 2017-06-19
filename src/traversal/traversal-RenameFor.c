#include "generated/traversal-RenameFor.h"
#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};

Info *RenameFor_createinfo(void) {
    Info *info = (Info *)mem_alloc(sizeof(Info));
    return info;
}
void RenameFor_freeinfo(Info *info) {}
void RenameFor_For(For *node, Info *info) {}
void RenameFor_VarLet(VarLet *node, Info *info) {}
void RenameFor_Var(Var *node, Info *info) {}

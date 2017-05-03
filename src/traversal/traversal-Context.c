#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};
struct Info *Context_createinfo(void) {
    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));
    return info;
}
void Context_freeinfo(struct Info *info) {}
void Context_Root(Root *node, struct Info *info) {}
void Context_GlobalDef(GlobalDef *node, struct Info *info) {}
void Context_GlobalDec(GlobalDec *node, struct Info *info) {}
void Context_Stmt(Stmt *node, struct Info *info) {}
void Context_FunDef(FunDef *node, struct Info *info) {}
void Context_Param(Param *node, struct Info *info) {}
void Context_For(For *node, struct Info *info) {}
void Context_VarDec(VarDec *node, struct Info *info) {}
void Context_VarLet(VarLet *node, struct Info *info) {}
void Context_Var(Var *node, struct Info *info) {}
void Context_FunCall(FunCall *node, struct Info *info) {}

#include "generated/traversal-Context.h"
#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};
Info *Context_createinfo(void) {
    Info *info = (Info *)mem_alloc(sizeof(Info));
    return info;
}
void Context_freeinfo(Info *info) {}
void Context_Root(Root *node, Info *info) {}
void Context_GlobalDef(GlobalDef *node, Info *info) {}
void Context_GlobalDec(GlobalDec *node, Info *info) {}
void Context_Stmt(Stmt *node, Info *info) {}
void Context_FunDef(FunDef *node, Info *info) {}
void Context_Param(Param *node, Info *info) {}
void Context_For(For *node, Info *info) {}
void Context_VarDec(VarDec *node, Info *info) {}
void Context_VarLet(VarLet *node, Info *info) {}
void Context_Var(Var *node, Info *info) {}
void Context_FunCall(FunCall *node, Info *info) {}

#include "generated/traversal-Refactor.h"
#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>

struct Info {
    int a;
};
Info *Refactor_createinfo(void) {
    Info *info = (Info *)mem_alloc(sizeof(Info));
    return info;
}
void Refactor_freeinfo(Info *info) {}
void Refactor_Cast(Cast *node, Info *info) {}
void Refactor_While(While *node, Info *info) {}
void Refactor_StmtList(StmtList *node, Info *info) {}
void Refactor_BinOp(BinOp *node, Info *info) {}
void Refactor_IntConst(IntConst *node, Info *info) {}
void Refactor_FloatConst(FloatConst *node, Info *info) {}
void Refactor_BoolConst(BoolConst *node, Info *info) {}
void Refactor_Var(Var *node, Info *info) {}
void Refactor_FunCall(FunCall *node, Info *info) {}

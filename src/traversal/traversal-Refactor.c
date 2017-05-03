#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>

struct Info {
    int a;
};
struct Info *Refactor_createinfo(void) {
    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));
    return info;
}
void Refactor_freeinfo(struct Info *info) {}
void Refactor_Cast(Cast *node, struct Info *info) {}
void Refactor_While(While *node, struct Info *info) {}
void Refactor_StmtList(StmtList *node, struct Info *info) {}
void Refactor_BinOp(BinOp *node, struct Info *info) {}
void Refactor_IntConst(IntConst *node, struct Info *info) {}
void Refactor_FloatConst(FloatConst *node, struct Info *info) {}
void Refactor_BoolConst(BoolConst *node, struct Info *info) {}
void Refactor_Var(Var *node, struct Info *info) {}
void Refactor_FunCall(FunCall *node, struct Info *info) {}

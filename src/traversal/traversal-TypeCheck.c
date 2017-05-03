#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};
struct Info *TypeCheck_createinfo(void) {
    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));
    return info;
}
void TypeCheck_freeinfo(struct Info *info) {}
void TypeCheck_FunDef(FunDef *node, struct Info *info) {}
void TypeCheck_FunBody(FunBody *node, struct Info *info) {}
void TypeCheck_Return(Return *node, struct Info *info) {}
void TypeCheck_VarLet(VarLet *node, struct Info *info) {}
void TypeCheck_For(For *node, struct Info *info) {}
void TypeCheck_DoWhile(DoWhile *node, struct Info *info) {}
void TypeCheck_While(While *node, struct Info *info) {}
void TypeCheck_IfElse(IfElse *node, struct Info *info) {}
void TypeCheck_FunCall(FunCall *node, struct Info *info) {}
void TypeCheck_BinOp(BinOp *node, struct Info *info) {}
void TypeCheck_MonOp(MonOp *node, struct Info *info) {}
void TypeCheck_Cast(Cast *node, struct Info *info) {}
void TypeCheck_Var(Var *node, struct Info *info) {}
void TypeCheck_IntConst(IntConst *node, struct Info *info) {}
void TypeCheck_FloatConst(FloatConst *node, struct Info *info) {}
void TypeCheck_BoolConst(BoolConst *node, struct Info *info) {}

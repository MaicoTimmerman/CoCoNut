#include "generated/traversal-TypeCheck.h"
#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};
Info *TypeCheck_createinfo(void) {
    Info *info = (Info *)mem_alloc(sizeof(Info));
    return info;
}
void TypeCheck_freeinfo(Info *info) {}
void TypeCheck_FunDef(FunDef *node, Info *info) {}
void TypeCheck_FunBody(FunBody *node, Info *info) {}
void TypeCheck_Return(Return *node, Info *info) {}
void TypeCheck_VarLet(VarLet *node, Info *info) {}
void TypeCheck_For(For *node, Info *info) {}
void TypeCheck_DoWhile(DoWhile *node, Info *info) {}
void TypeCheck_While(While *node, Info *info) {}
void TypeCheck_IfElse(IfElse *node, Info *info) {}
void TypeCheck_FunCall(FunCall *node, Info *info) {}
void TypeCheck_BinOp(BinOp *node, Info *info) {}
void TypeCheck_MonOp(MonOp *node, Info *info) {}
void TypeCheck_Cast(Cast *node, Info *info) {}
void TypeCheck_Var(Var *node, Info *info) {}
void TypeCheck_IntConst(IntConst *node, Info *info) {}
void TypeCheck_FloatConst(FloatConst *node, Info *info) {}
void TypeCheck_BoolConst(BoolConst *node, Info *info) {}


#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};
struct Info *Print_createinfo(void) {
    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));
    return info;
}
void Print_freeinfo(struct Info *info) {}
void Print_Root(Root *node, struct Info *info) {}
void Print_Decls(Decls *node, struct Info *info) {}
void Print_LocalFunDef(LocalFunDef *node, struct Info *info) {}
void Print_FunDef(FunDef *node, struct Info *info) {}
void Print_FunHeader(FunHeader *node, struct Info *info) {}
void Print_Param(Param *node, struct Info *info) {}
void Print_GlobalDec(GlobalDec *node, struct Info *info) {}
void Print_GlobalDef(GlobalDef *node, struct Info *info) {}
void Print_FunBody(FunBody *node, struct Info *info) {}
void Print_VarDec(VarDec *node, struct Info *info) {}
void Print_StmtList(StmtList *node, struct Info *info) {}
void Print_VarLet(VarLet *node, struct Info *info) {}
void Print_FunCall(FunCall *node, struct Info *info) {}
void Print_ExprList(ExprList *node, struct Info *info) {}
void Print_IfElse(IfElse *node, struct Info *info) {}
void Print_While(While *node, struct Info *info) {}
void Print_DoWhile(DoWhile *node, struct Info *info) {}
void Print_For(For *node, struct Info *info) {}
void Print_Return(Return *node, struct Info *info) {}
void Print_BinOp(BinOp *node, struct Info *info) {}
void Print_MonOp(MonOp *node, struct Info *info) {}
void Print_Cast(Cast *node, struct Info *info) {}
void Print_Var(Var *node, struct Info *info) {}
void Print_IntConst(IntConst *node, struct Info *info) {}
void Print_FloatConst(FloatConst *node, struct Info *info) {}
void Print_BoolConst(BoolConst *node, struct Info *info) {}
void Print_Ternary(Ternary *node, struct Info *info) {}
void Print_Symbol(Symbol *node, struct Info *info) {}
void Print_FunSymbol(FunSymbol *node, struct Info *info) {}

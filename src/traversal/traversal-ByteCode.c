#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};

struct Info *ByteCode_createinfo(void) {
    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));
    return info;
}

void ByteCode_freeinfo(struct Info *info) {}
void ByteCode_Root(Root *node, struct Info *info) {}
void ByteCode_Decls(Decls *node, struct Info *info) {}
void ByteCode_LocalFunDef(LocalFunDef *node, struct Info *info) {}
void ByteCode_FunDef(FunDef *node, struct Info *info) {}
void ByteCode_FunHeader(FunHeader *node, struct Info *info) {}
void ByteCode_Param(Param *node, struct Info *info) {}
void ByteCode_GlobalDec(GlobalDec *node, struct Info *info) {}
void ByteCode_GlobalDef(GlobalDef *node, struct Info *info) {}
void ByteCode_FunBody(FunBody *node, struct Info *info) {}
void ByteCode_VarDec(VarDec *node, struct Info *info) {}
void ByteCode_StmtList(StmtList *node, struct Info *info) {}
void ByteCode_VarLet(VarLet *node, struct Info *info) {}
void ByteCode_FunCall(FunCall *node, struct Info *info) {}
void ByteCode_ExprList(ExprList *node, struct Info *info) {}
void ByteCode_IfElse(IfElse *node, struct Info *info) {}
void ByteCode_While(While *node, struct Info *info) {}
void ByteCode_DoWhile(DoWhile *node, struct Info *info) {}
void ByteCode_For(For *node, struct Info *info) {}
void ByteCode_Return(Return *node, struct Info *info) {}
void ByteCode_BinOp(BinOp *node, struct Info *info) {}
void ByteCode_MonOp(MonOp *node, struct Info *info) {}
void ByteCode_Cast(Cast *node, struct Info *info) {}
void ByteCode_Var(Var *node, struct Info *info) {}
void ByteCode_IntConst(IntConst *node, struct Info *info) {}
void ByteCode_FloatConst(FloatConst *node, struct Info *info) {}
void ByteCode_BoolConst(BoolConst *node, struct Info *info) {}
void ByteCode_Ternary(Ternary *node, struct Info *info) {}
void ByteCode_Symbol(Symbol *node, struct Info *info) {}
void ByteCode_FunSymbol(FunSymbol *node, struct Info *info) {}

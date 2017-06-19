#include "generated/traversal-ByteCode.h"
#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};

Info *ByteCode_createinfo(void) {
    Info *info = (Info *)mem_alloc(sizeof(Info));
    return info;
}

void ByteCode_freeinfo(Info *info) {}
void ByteCode_Root(Root *node, Info *info) {}
void ByteCode_Decls(Decls *node, Info *info) {}
void ByteCode_LocalFunDef(LocalFunDef *node, Info *info) {}
void ByteCode_FunDef(FunDef *node, Info *info) {}
void ByteCode_FunHeader(FunHeader *node, Info *info) {}
void ByteCode_Param(Param *node, Info *info) {}
void ByteCode_GlobalDec(GlobalDec *node, Info *info) {}
void ByteCode_GlobalDef(GlobalDef *node, Info *info) {}
void ByteCode_FunBody(FunBody *node, Info *info) {}
void ByteCode_VarDec(VarDec *node, Info *info) {}
void ByteCode_StmtList(StmtList *node, Info *info) {}
void ByteCode_VarLet(VarLet *node, Info *info) {}
void ByteCode_FunCall(FunCall *node, Info *info) {}
void ByteCode_ExprList(ExprList *node, Info *info) {}
void ByteCode_IfElse(IfElse *node, Info *info) {}
void ByteCode_While(While *node, Info *info) {}
void ByteCode_DoWhile(DoWhile *node, Info *info) {}
void ByteCode_For(For *node, Info *info) {}
void ByteCode_Return(Return *node, Info *info) {}
void ByteCode_BinOp(BinOp *node, Info *info) {}
void ByteCode_MonOp(MonOp *node, Info *info) {}
void ByteCode_Cast(Cast *node, Info *info) {}
void ByteCode_Var(Var *node, Info *info) {}
void ByteCode_IntConst(IntConst *node, Info *info) {}
void ByteCode_FloatConst(FloatConst *node, Info *info) {}
void ByteCode_BoolConst(BoolConst *node, Info *info) {}
void ByteCode_Ternary(Ternary *node, Info *info) {}
void ByteCode_Symbol(Symbol *node, Info *info) {}
void ByteCode_FunSymbol(FunSymbol *node, Info *info) {}

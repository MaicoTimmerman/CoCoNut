#include "../../src/generated/ast.h"
#include "../../src/generated/trav-ast.h"
#include "../../src/lib/memory.h"
#include <stdio.h>

#define INDENT print_indent(info->indent)

struct Info {
    int indent;
};
struct Info *Print_createinfo(void) {
    struct Info *res = (struct Info*)mem_alloc(sizeof(struct Info));
    res->indent = 0;
    return res;
}

void Print_freeinfo(struct Info *info) {
    mem_free(info);
}

static void print_indent(int n) {
    for (int i = 0; i < n; ++i) {
        printf("    ");
    }
}

static void print_basictype(BasicType type) {
    switch (type) {
        case BT_void:
            printf("void");
            break;
        case BT_float:
            printf("float");
            break;
        case BT_bool:
            printf("bool");
            break;
        case BT_int:
            printf("int");
            break;
        defaut:
            printf("IDK");
            break;
    }
}

void Print_Root(Root *node, struct Info *info) {
    trav_Root_symbol(node, info);
    trav_Root_funsymbol(node, info);
    trav_Root_decls(node, info);
}

void Print_Decls(Decls *node, struct Info *info) {
    trav_Decls_decl(node, info);
}

void Print_LocalFunDef(LocalFunDef *node, struct Info *info) {
    trav_LocalFunDef_decl(node, info);
    trav_LocalFunDef_next(node, info);
}

void Print_FunDef(FunDef *node, struct Info *info) {
    if (node->external) {
        printf("extern ");
        trav_FunDef_funheader(node, info);
        printf(";");
    } else {
        if (node->export) printf("export ");
        trav_FunDef_funheader(node, info);
        printf(" {\n");

        info->indent++;
        trav_FunDef_funbody(node, info);
        info->indent--;

        printf("}\n");
    }
}

void Print_FunHeader(FunHeader *node, struct Info *info) {
    print_basictype(node->rettype);
    printf(" %s(", node->id);
    trav_FunHeader_params(node, info);
}

void Print_Param(Param *node, struct Info *info) {
    print_basictype(node->type);
    printf(" %s", node->id);
    if (node->next != NULL) {
        printf(", ");
        trav_Param_next(node, info);
    }
}

void Print_GlobalDec(GlobalDec *node, struct Info *info) {
    INDENT;
    printf("extern ");
    print_basictype(node->type);
    printf(" %s;", node->id);
}

void Print_GlobalDef(GlobalDef *node, struct Info *info) {
    INDENT;
    if (node->export) {
        printf("export ");
    }
    print_basictype(node->type);
    printf(" %s", node->id);

    if (node->expr != NULL) {
        printf(" = ");
        trav_GlobalDef_expr(node, info);
    }

    printf(";");
}

void Print_FunBody(FunBody *node, struct Info *info) {
    trav_FunBody_vardec(node, info);
    printf("\n");
    trav_FunBody_localfundef(node, info);
    printf("\n");
    trav_FunBody_stmtlist(node, info);
}

void Print_VarDec(VarDec *node, struct Info *info) {
    INDENT;
    print_basictype(node->type);
    printf(" %s", node->id);
    if (node->expr != NULL) {
        printf(" = ");
        trav_VarDec_expr(node, info);
    }

    printf(";");

    if (node->next != NULL) {
        printf("\n");
        trav_VarDec_next(node, info);
    }
}

void Print_StmtList(StmtList *node, struct Info *info) {
    INDENT;
    trav_StmtList_stmt(node, info);

    switch (node->stmt->type) {
        case NS_Stmt_IfElse:
        case NS_Stmt_While:
        case NS_Stmt_For:
            printf("\n");
            break;
        default:
            printf(";\n");
    }

    trav_StmtList_next(node, info);
}

void Print_VarLet(VarLet *node, struct Info *info) {
    printf("%s = ", node->id);
    trav_VarLet_expr(node, info);
}

void Print_FunCall(FunCall *node, struct Info *info) {
    printf("%s(", node->id);
    trav_FunCall_params(node, info);
    printf(")");
}

void Print_ExprList(ExprList *node, struct Info *info){
    trav_ExprList_expr(node, info);
    if (node->next != NULL) {
        printf(", ");
        trav_ExprList_next(node, info);
    }
}

void Print_IfElse(IfElse *node, struct Info *info){
    printf("if (");
    trav_IfElse_condition(node, info);
    printf(") {\n");

    info->indent++;
    trav_IfElse_ifblock;
    info->indent--;

    INDENT;
    printf("}");

    if (node->elseblock != NULL) {
        printf(" else {\n");
        info->indent++;
        trav_IfElse_elseblock;
        info->indent--;

        INDENT;
        printf("}");
    }
}

void Print_While(While *node, struct Info *info) {
    printf("while (");
    trav_While_condition(node, info);
    printf(") {\n");

    info->indent++;
    trav_While_block(node, info);
    info->indent--;

    INDENT;
    printf("}");
}

void Print_DoWhile(DoWhile *node, struct Info *info) {
    printf("do {\n");

    info->indent++;
    trav_DoWhile_block(node, info);
    info->indent--;

    INDENT;
    printf("} (");
    trav_DoWhile_condition(node, info);
    printf(")");
}

void Print_For(For *node, struct Info *info) {

    printf("for (int %s = ", node->id);
    trav_For_initexpr(node, info);

    printf(", ");
    trav_For_boundexpr(node, info);

    if (node->stepexpr != NULL) {
        printf(", ");
        trav_For_stepexpr(node, info);
    }
    printf(") {\n");

    info->indent++;
    trav_For_block(node, info);
    info->indent--;

    INDENT;
    printf("}");
}

void Print_Return(Return *node, struct Info *info) {
    printf("return");
    if (node->expr != NULL) {
        printf(" ");
        trav_Return_expr(node, info);
    }
}

void Print_BinOp(BinOp *node, struct Info *info) {

    printf("(");
    trav_BinOp_left(node, info);

    char *tmp;

    switch (node->op) {
        case BO_add: tmp = "+"; break;
        case BO_sub: tmp = "-"; break;
        case BO_mul: tmp = "*"; break;
        case BO_div: tmp = "/"; break;
        case BO_mod: tmp = "%"; break;
        case BO_lt: tmp = "<"; break;
        case BO_leq: tmp = "<="; break;
        case BO_gt: tmp = ">"; break;
        case BO_geq: tmp = ">="; break;
        case BO_eq: tmp = "=="; break;
        case BO_neq: tmp = "!="; break;
        case BO_or: tmp = "||"; break;
        case BO_and: tmp = "&&"; break;
    }

    printf(" %s ", tmp);

    trav_BinOp_right(node, info);

    printf(")");
}

void Print_MonOp(MonOp *node, struct Info *info) {
    switch (node->op) {
        case MO_not:
            printf("(! ");
            break;
        case BO_sub:
            printf("(- ");
            break;
        default:
            printf("unknown monop detected!");
    }
    trav_MonOp_expr(node, info);
    printf(")");
}

void Print_Cast(Cast *node, struct Info *info) {
    printf("(");
    switch (node->type) {
        case BT_int:
            printf("(int) ");
            break;
        case BT_float:
            printf("(float) ");
            break;
        case BT_bool:
            printf("(bool) ");
            break;
        default:
            printf("unknown basictype detected!");
            exit(1);
    }
    printf("(");
    trav_Cast_expr(node, info);
    printf("))");
}

void Print_Var(Var *node, struct Info *info) {
    printf("%s", node->id);
}

void Print_IntConst(IntConst *node, struct Info *info) {
    printf("%d", node->value);
}

void Print_FloatConst(FloatConst *node, struct Info *info){
    printf("%f", node->value);
}

void Print_BoolConst(BoolConst *node, struct Info *info) {
    if (node->value) {
        printf("true");
    } else {
        printf("false");
    }
}

void Print_Ternary(Ternary *node, struct Info *info) {
    printf("(");
    trav_Ternary_testexpr(node, info);
    printf(" ? ");
    trav_Ternary_trueexpr(node, info);
    printf(" : ");
    trav_Ternary_falseexpr(node, info);
    printf(")");
}

void Print_Symbol(Symbol *node, struct Info *info) {

    INDENT;
    printf(" * %s (Scope: %d, Offset: %d, Extern: %s, Export: %s)\n",
            node->name,
            node->scope,
            node->offset,
            node->external ? "true" : "false",
            node->export? "true" : "false"
            );

    trav_Symbol_next(node, info);
}

void Print_FunSymbol(FunSymbol *node, struct Info *info) {
    INDENT;
    printf(" * function %s: %p (%d params, scope: %d, offset: %d, Extern: %s, Export: %s)\n",
            node->name,
            node,
            node->arity,
            node->scope,
            node->offset,
            node->external ? "true" : "false",
            node->export ? "true" : "false");

    trav_FunSymbol_next(node, info);
}

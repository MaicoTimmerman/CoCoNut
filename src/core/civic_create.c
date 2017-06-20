#include "generated/ast.h"
#include "generated/copy-ast.h"
#include "generated/create-ast.h"
#include "generated/free-ast.h"
#include "generated/phase-driver.h"
#include "generated/trav-ast.h"

#include "generated/serialization-Root.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* void serialization_write_binfile_VarLet(VarLet *syntaxtree, FILE *fp); */
/* VarLet *serialization_read_binfile_VarLet(FILE *fp); */
/* void serialization_write_binfile_Root(Root *syntaxtree, FILE *fp); */
/* Root *serialization_read_binfile_Root(FILE *fp); */

/// export int main() {
///     int a1;
///     int a2;
///     int a3;
///     int a4;
///     a1 = scanInt();
///     a2 = scanInt();
///     a3 = a1 + a2;
///     a4 = a3;
///     return a4;
/// }
int main() {
    // VARDECS
    VarDec *vda1 = create_VarDec(NULL, NULL, strdup("a1"), BT_int);
    VarDec *vda2 = create_VarDec(NULL, NULL, strdup("a2"), BT_int);
    VarDec *vda3 = create_VarDec(NULL, NULL, strdup("a3"), BT_int);
    VarDec *vda4 = create_VarDec(NULL, NULL, strdup("a4"), BT_int);
    vda1->next = vda2;
    vda2->next = vda3;
    vda3->next = vda4;

    // STMTs
    FunCall *scanint = create_FunCall(NULL, strdup("scanInt"));
    FunCall *scanint2 = create_FunCall(NULL, strdup("scanInt"));
    BinOp *plus =
        create_BinOp(create_Expr_Var(create_Var(strdup("a1"))),
                     create_Expr_Var(create_Var(strdup("a2"))), BO_add);

    VarLet *a1 = create_VarLet(create_Expr_FunCall(scanint), strdup("a1"));
    VarLet *a2 = create_VarLet(create_Expr_FunCall(scanint2), strdup("a2"));
    VarLet *a3 = create_VarLet(create_Expr_BinOp(plus), strdup("a3"));
    VarLet *a4 =
        create_VarLet(create_Expr_Var(create_Var(strdup("a3"))), strdup("a4"));
    Return *returna4 =
        create_Return(create_Expr_Var(create_Var(strdup("a4"))));

    StmtList *stmtla1 = create_StmtList(NULL, create_Stmt_VarLet(a1));
    StmtList *stmtla2 = create_StmtList(NULL, create_Stmt_VarLet(a2));
    StmtList *stmtla3 = create_StmtList(NULL, create_Stmt_VarLet(a3));
    StmtList *stmtla4 = create_StmtList(NULL, create_Stmt_VarLet(a4));
    StmtList *stmtreturn = create_StmtList(NULL, create_Stmt_Return(returna4));
    stmtla1->next = stmtla2;
    stmtla2->next = stmtla3;
    stmtla3->next = stmtla4;
    stmtla4->next = stmtreturn;

    FunDef *mainfun =
        create_FunDef(create_FunBody(NULL, stmtla1, vda1),
                      create_FunHeader(NULL, strdup("main"), BT_int), NULL,
                      NULL, true, false);

    Root *program = create_Root(
        create_Decls(create_Decl_FunDef(mainfun), NULL), NULL, NULL);

    // phasedriver_run(program);
    FILE *fp;

    serialization_write_binfile_Root(program, "test-serialization.bin");
    Root *root_ser = serialization_read_binfile_Root("test-serialization.bin");

    trav_start_Root(root_ser, TRAV_Print);

    return 0;
}

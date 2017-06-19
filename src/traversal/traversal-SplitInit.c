#include "generated/traversal-SplitInit.h"
#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};

Info *SplitInit_createinfo(void) {
    Info *info = (Info *)mem_alloc(sizeof(Info));
    return info;
}
void SplitInit_freeinfo(Info *info) {}
void SplitInit_Root(Root *node, Info *info) {}
void SplitInit_GlobalDef(GlobalDef *node, Info *info) {}
void SplitInit_FunBody(FunBody *node, Info *info) {}
void SplitInit_VarDec(VarDec *node, Info *info) {}

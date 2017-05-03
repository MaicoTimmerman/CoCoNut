#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};

struct Info *SplitInit_createinfo(void) {
    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));
    return info;
}
void SplitInit_freeinfo(struct Info *info) {}
void SplitInit_Root(Root *node, struct Info *info) {}
void SplitInit_GlobalDef(GlobalDef *node, struct Info *info) {}
void SplitInit_FunBody(FunBody *node, struct Info *info) {}
void SplitInit_VarDec(VarDec *node, struct Info *info) {}

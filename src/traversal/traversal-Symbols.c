#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};
struct Info *Symbols_createinfo(void) {
    struct Info *info = (struct Info *)mem_alloc(sizeof(struct Info));
    return info;
}
void Symbols_freeinfo(struct Info *info) {}
void Symbols_Root(Root *node, struct Info *info) {}
void Symbols_FunDef(FunDef *node, struct Info *info) {}
void Symbols_FunSymbol(FunSymbol *node, struct Info *info) {}
void Symbols_Symbol(Symbol *node, struct Info *info) {}

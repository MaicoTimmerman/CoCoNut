#include "generated/traversal-Symbols.h"
#include "generated/ast.h"
#include "lib/memory.h"
#include <stdio.h>
struct Info {
    int a;
};
Info *Symbols_createinfo(void) {
    Info *info = (Info *)mem_alloc(sizeof(Info));
    return info;
}
void Symbols_freeinfo(Info *info) {}
void Symbols_Root(Root *node, Info *info) {}
void Symbols_FunDef(FunDef *node, Info *info) {}
void Symbols_FunSymbol(FunSymbol *node, Info *info) {}
void Symbols_Symbol(Symbol *node, Info *info) {}

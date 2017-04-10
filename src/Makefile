all: ast

ast.tab.c ast.tab.h: ast.y
	bison -d ast.y

lex.yy.c: ast.l ast.tab.h
	flex ast.l

ast: lex.yy.c ast.tab.c ast.tab.h
	gcc -o ast ast.tab.c lex.yy.c

clean:
	rm ast ast.tab.c lex.yy.c ast.tab.h

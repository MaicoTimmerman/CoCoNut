CC = gcc
CFLAGS = -Wall -Wextra  -g

all: ast

ast.tab.c ast.tab.h: ast.y
	bison -d ast.y

lex.yy.c: ast.l ast.tab.h
	flex ast.l 

lex.yy.o: lex.yy.c
	$(CC) -c -o $@ $< -g

ast.tab.o: ast.tab.c ast.tab.h
	$(CC) -c -o ast.tab.o ast.tab.c -g

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

ast: lex.yy.o ast.tab.o ast-internal.o main.o array.o
	gcc -o ast $^ -g

clean:
	rm -f ast ast.tab.c lex.yy.c ast.tab.h *.o

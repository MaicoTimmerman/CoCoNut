%{

#include <stdio.h>
#include <stdlib.h>

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s);

int yydebug = 1;
%}

%union {
    long long cint;
    long double cflt;
    char *stringval;
    char *str;
}

%error-verbose
%locations


%token<ival> T_INTVAL
%token<fval> T_FLOATVAL
%token<stringval> T_STRINGVAL

%token T_TRUE T_FALSE

%token T_ATTRIBUTES T_CHILDREN T_CONSTRUCT T_ENUM T_FLAGS T_MANDATORY
%token T_NODE T_NODES T_NODESET T_TO T_TRAVERSAL

%token T_UNSIGNED T_CHAR T_SHORT T_INT T_LONG T_FLOAT T_DOUBLE
%token T_STRING T_ID

%start root

%%

/* root: phases traversals enums nodesets nodes */
root: traversals enums nodesets nodes
/* root: nodesets */
    ;

traversals: traversal traversals
          | %empty
          ;

traversal: T_TRAVERSAL T_ID ';'
         | T_TRAVERSAL T_ID '{' T_NODES '{' idlist '}' '}' ';'
         ;

enums: enum enums
     | %empty
     ;

enum: T_ENUM T_ID '{' idlist '}' ';'
    ;

nodesets: nodeset nodesets
        | %empty
        ;

nodeset: T_NODESET T_ID '{' idlist '}' ';'
       ;

nodes: node nodes
     | %empty
     ;

node: T_NODE T_ID '{' nodebody '}' ';'
    ;

nodebody: children ',' attrs ',' flags
        | attrs ',' flags
        | children ',' flags
        | children ',' attrs
        | children
        | attrs
        | flags
        ;

children: T_CHILDREN '{' childlist '}'
        ;

childlist: child
         | child childlist
         ;

child: T_ID T_ID
     | mandatory T_ID T_ID
     | T_CONSTRUCT T_ID T_ID
     | T_CONSTRUCT mandatory T_ID T_ID
     ;

attrs: T_ATTRIBUTES '{' attrlist '}'
     ;

attrlist: attr attrlist
        | attr
        ;

attr: attrtype T_ID
    | T_CONSTRUCT attrtype T_ID
    | attrtype T_ID '=' attrval
    | T_CONSTRUCT attrtype T_ID '=' attrval
    ;

attrtype: T_CHAR
        | T_UNSIGNED T_CHAR
        | T_SHORT
        | T_UNSIGNED T_SHORT
        | T_INT
        | T_UNSIGNED T_INT
        | T_LONG
        | T_UNSIGNED T_LONG
        | T_LONG T_LONG
        | T_UNSIGNED T_LONG T_LONG
        | T_FLOAT
        | T_DOUBLE
        | T_LONG T_DOUBLE
        | T_STRING
        | T_ID
        ;

attrval: T_STRINGVAL
       | T_INTVAL
       | T_FLOATVAL
       | T_ID
       ;

flags: T_FLAGS '{' flaglist '}'
     ;

flaglist: flag flaglist
        | flag
        ;

flag: T_CONSTRUCT T_ID '=' bool
    | T_CONSTRUCT T_ID
    | T_ID '=' bool
    | T_ID
    ;

mandatory: T_MANDATORY '[' mandatoryarglist ']'
         | T_MANDATORY
         ;

mandatoryarglist: mandatoryarg ',' mandatoryarglist
                | mandatoryarg
                ;

mandatoryarg: T_ID
            | T_ID T_TO T_ID
            ;

bool: T_TRUE
    | T_FALSE
    ;


idlist: idlist ',' T_ID
      | T_ID
      ;


%%

int main() {
    yyin = stdin;

    do {
        yyparse();
    } while(!feof(yyin));

    return 0;
}

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at %d: %s\n", 1, s);
    exit(1);
}

%option noyywrap

%{
#include <stdio.h>
#include "ast.tab.h"

#define YY_DECL int yylex()

int yycolumn = 1;

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng; \
    yylval.str = strdup(yytext);

%}

%option yylineno

%%
"("	{return '(';}
")"	{return ')';}
"["	{return '[';}
"]"	{return ']';}
"{"	{return '{';}
"}"	{return '}';}
","	{return ',';}
"="	{return '=';}
";"	{return ';';}

"attributes" {return T_ATTRIBUTES;}
"children" {return T_CHILDREN;}
"construct" {return T_CONSTRUCT;}
"enum" {return T_ENUM;}
"flags" {return T_FLAGS;}
"mandatory" {return T_MANDATORY;}
"node" {return T_NODE;}
"nodes" {return T_NODES;}
"nodeset" {return T_NODESET;}
"to" {return T_TO;}
"traversal" {return T_TRAVERSAL;}

"char" {return T_CHAR;}
"double" {return T_DOUBLE;}
"float" {return T_FLOAT;}
"int " {return T_INT ;}
"long" {return T_LONG;}
"short" {return T_SHORT;}
"string" {return T_STRING;}
"unsigned" {return T_UNSIGNED;}

"false" {return T_FALSE;}
"true" {return T_TRUE;}

[A-Za-z][A-Za-z_]* {yylval.stringval = strdup(yytext); return T_ID;}
[0-9]+\.[0-9]+ {yylval.cflt = strtold(yytext, NULL); return T_FLOATVAL;}
[0-9]+ {yylval.cint = strtoll(yytext, NULL, 10); return T_INTVAL;}

[ \t]	; // ignore all whitespace
\n { yycolumn = 0; }

. { printf("%s: Normaal. Doen.\n", yytext); exit(1); }
%%

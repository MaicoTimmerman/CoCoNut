%{
#include <stdio.h>
#include "ast-internal.h"
#include "ast.tab.h"

#define YY_DECL int yylex()

char linebuf[500];
int yycolumn = 1;

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng; \
    yylval.str = strdup(yytext);

%}

%option yylineno
%x COMMENT

%%
\/\/[^\n]*                  {  }
"/*"                        { BEGIN(COMMENT); }
<COMMENT>{
    "*/"                    { BEGIN(INITIAL); }
    .                       {  }
}
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

[A-Za-z][A-Za-z_]*  {yylval.string = strdup(yytext); return T_ID;}
[0-9]+\.[0-9]+      {yylval.fval = strtold(yytext, NULL); return T_FLOATVAL;}
[0-9]+              {yylval.ival = strtoll(yytext, NULL, 10); return T_INTVAL;}
\"(\\.|[^\\"])*\"   {yylval.string = strdup(yytext); return T_STRINGVAL;}

[ \t]	; // ignore all whitespace
\n.*  { strncpy(linebuf, yytext+1, sizeof(linebuf)); /* save the next line */
        yycolumn = 0;
        yyless(1);      /* give back all but the \n to rescan */
      }
. { printf("%s: Normaal. Doen.\n", yytext); exit(1); }
%%


int yywrap() {
	return (1);
}

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d, col %d: %s\n",
        yylineno, yycolumn, s);
    fprintf(stderr, "%s", linebuf);
    exit(1);
}

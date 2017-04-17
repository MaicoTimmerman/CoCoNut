%{
#include <stdio.h>
#include "ast.h"
#include "ast.parser.h"
#include "print.h"

#define YY_DECL int yylex()

char linebuf[500];
int yycolumn = 1;

static inline void token_action();
#define YY_USER_ACTION token_action();

%}

%option yylineno
%option nounput
%option noinput

%option stack
%option noyy_top_state

%x COMMENT
%x SAVELINE

%%
<SAVELINE>{
    
    .* {
        // Save the whole line         
        strncpy(linebuf, yytext, sizeof(linebuf));

        // yylineno is set automatically to 0, 
        // however the column needs to be reset.
        yycolumn = 0;

        // give back the line for the real lexing state
        yyless(0);

        // Continue to the previous lexing state
        yy_pop_state();
    }
    \n {
        // Save the empty line
        strncpy(linebuf, "", sizeof(linebuf));
        // yylineno is set automatically to 0, 
        // however the column needs to be reset.
        yycolumn = 0;

        // Stay in the SAVELINE state, to save the next line
    }
}

\/\/[^\n]*      {  }
"/*"            { yy_push_state(COMMENT); }
<COMMENT>{
    "*/"        { yy_push_state(INITIAL); }
    \n          { yy_push_state(SAVELINE); }
    .           {  }
}
"(" {return '(';}
")" {return ')';}
"[" {return '[';}
"]" {return ']';}
"{" {return '{';}
"}" {return '}';}
"," {return ',';}
"=" {return '=';}
";" {return ';';}
"!" {return '!';}

"attributes" {return T_ATTRIBUTES;}
"children" {return T_CHILDREN;}
"child" {return T_CHILD;}
"construct" {return T_CONSTRUCT;}
"enum" {return T_ENUM;}
"mandatory" {return T_MANDATORY;}
"node" {return T_NODE;}
"nodeset" {return T_NODESET;}
"nodes" {return T_NODES;}
"prefix" {return T_PREFIX;}
"to" {return T_TO;}
"traversal" {return T_TRAVERSAL;}
"values" {return T_VALUES;}

"double" {return T_DOUBLE;}
"float" {return T_FLOAT;}
"int" {return T_INT;}
"uint" {return T_UINT;}
"int8" {return T_INT8;}
"int16" {return T_INT16;}
"int32" {return T_INT32;}
"int64" {return T_INT64;}
"uint8" {return T_UINT8;}
"uint16" {return T_UINT16;}
"uint32" {return T_UINT32;}
"uint64" {return T_UINT64;}
"string" {return T_STRING;}
"bool" {return T_BOOL;}

"false" {return T_FALSE;}
"true" {return T_TRUE;}
"NULL" {return T_NULL;}


[A-Za-z][A-Za-z0-9_]*  {
    // Identifier start with an letter and can have numbers and underscores.
    yylval.string = strdup(yytext); return T_ID; }
-?[0-9]+\.[0-9]+ {
    // (negative) floating point numbers
    yylval.fval = strtold(yytext, NULL); return T_FLOATVAL;}
-[0-9]+ {
    // (negative) integer numbers
    yylval.intval = (int64_t)strtoll(yytext, NULL, 10); return T_UINTVAL;}
[0-9]+ {
    // (positive) integer numbers
    yylval.uintval = (uint64_t)strtoull(yytext, NULL, 10); return T_INTVAL;}
\"(\\.|[^\\"])*\"   {
    yylval.string = strdup(yytext); return T_STRINGVAL;}

[ \t\r]   ; // ignore all whitespace

\n  {
    // Switch to the SAVELINE state to read the whole line first
    yy_push_state(SAVELINE);
}
. { printf("Unexpected token found: %s, quit parsing\n", yytext); exit(1); }
%%

void lexer_init(void) {
    // We start by reading the whole first line, and then continue lexing it 
    yy_push_state(SAVELINE);
}

static inline void token_action() {
    if (YY_START != SAVELINE) {
        yylloc.first_line = yylloc.last_line = yylineno; 
        yylloc.first_column = yycolumn;
        yylloc.last_column = yycolumn + yyleng - 1; 
        yycolumn += yyleng;
    }
}

int yywrap() {
    return (1);
}

void yyerror(const char* s) {
    print_error_at(yylineno, yycolumn, linebuf, "Parse error: %s", s);    
    exit(1);
}

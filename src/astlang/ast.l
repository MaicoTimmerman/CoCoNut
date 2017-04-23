%{
#include <stdio.h>
#include <stdbool.h>
#include "ast.h"
#include "ast.parser.h"
#include "print.h"
#include "array.h"

#define YY_DECL int yylex()
#define LEX_KEYWORD(k) if (yy_lex_keywords) \
                                return k;   \
                            else {          \
                                yylval.string = strdup(yytext); \
                                return T_ID; \
                            } \

bool yy_lex_keywords = true;
static char yy_linebuf[500];
int yycolumn = 1;

array *yy_lines;

static inline void token_action();
static inline void lexer_init();

#define YY_USER_INIT lexer_init();
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
        strncpy(yy_linebuf, yytext, sizeof(yy_linebuf));

        array_append(yy_lines, strdup(yy_linebuf));

        // yylineno is set automatically to 0,
        // however the column needs to be reset.
        yycolumn = 1;

        // give back the line for the real lexing state
        yyless(0);

        // Continue to the previous lexing state
        yy_pop_state();
    }
    \n {
        // Save the empty line
        strncpy(yy_linebuf, "", sizeof(yy_linebuf));

        array_append(yy_lines, strdup(yy_linebuf));

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

"attributes"    { LEX_KEYWORD(T_ATTRIBUTES);}
"children"      { LEX_KEYWORD(T_CHILDREN);}
"child"         { LEX_KEYWORD(T_CHILD);}
"construct"     { LEX_KEYWORD(T_CONSTRUCT);}
"cycle"         { LEX_KEYWORD(T_CYCLE);}
"cycles"        { LEX_KEYWORD(T_CYCLES);}
"enum"          { LEX_KEYWORD(T_ENUM);}
"mandatory"     { LEX_KEYWORD(T_MANDATORY);}
"node"          { LEX_KEYWORD(T_NODE);}
"nodeset"       { LEX_KEYWORD(T_NODESET);}
"nodes"         { LEX_KEYWORD(T_NODES);}
"phase"         { LEX_KEYWORD(T_PHASE);}
"prefix"        { LEX_KEYWORD(T_PREFIX);}
"to"            { LEX_KEYWORD(T_TO);}
"traversal"     { LEX_KEYWORD(T_TRAVERSAL);}
"traversals"    { LEX_KEYWORD(T_TRAVERSALS);}
"values"        {
                    if (yy_lex_keywords) {
                        yy_lex_keywords = false;
                        return T_VALUES;
                    } else {
                        yylval.string = strdup(yytext);
                        return T_ID;
                    }
                }

"double"        { LEX_KEYWORD(T_DOUBLE);}
"float"         { LEX_KEYWORD(T_FLOAT);}
"int"           { LEX_KEYWORD(T_INT);}
"uint"          { LEX_KEYWORD(T_UINT);}
"int8"          { LEX_KEYWORD(T_INT8);}
"int16"         { LEX_KEYWORD(T_INT16);}
"int32"         { LEX_KEYWORD(T_INT32);}
"int64"         { LEX_KEYWORD(T_INT64);}
"uint8"         { LEX_KEYWORD(T_UINT8);}
"uint16"        { LEX_KEYWORD(T_UINT16);}
"uint32"        { LEX_KEYWORD(T_UINT32);}
"uint64"        { LEX_KEYWORD(T_UINT64);}
"string"        { LEX_KEYWORD(T_STRING);}
"bool"          { LEX_KEYWORD(T_BOOL);}

"false"         { LEX_KEYWORD(T_FALSE);}
"true"          { LEX_KEYWORD(T_TRUE);}

"NULL"          { return T_NULL;}


[A-Za-z][A-Za-z0-9_]*  {
    // TODO: Make identifiers start with an uppercase letter?
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

static inline void lexer_init(void) {
    // We start by reading the whole first line, and then continue lexing it
    yy_push_state(SAVELINE);

    yy_lines = array_init(32);
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
    print_error_at(yylineno, yycolumn, yy_linebuf, "Parse error: %s", s);
    exit(1);
}

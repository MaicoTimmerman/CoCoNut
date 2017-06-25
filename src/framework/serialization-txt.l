%{
#include <stdio.h>
#include <stdbool.h>

#include "lib/errors.h"
#include "lib/array.h"
#include "lib/print.h"

#include "framework/serialization-txt-ast.h"
#include "serialization-txt.parser.h"

#define AST_SRL_YY_DECL int ast_srl_yylex()

static char ast_srl_yy_linebuf[500];

static int ast_srl_yycolumn = 1;
extern array *ast_srl_yy_lines;

static inline void token_action();
static inline void lexer_init();

#define YY_USER_INIT lexer_init();
#define YY_USER_ACTION token_action();

%}
%option prefix="ast_srl_yy"

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
        strncpy(ast_srl_yy_linebuf, ast_srl_yytext, sizeof(ast_srl_yy_linebuf));

        array_append(ast_srl_yy_lines, strdup(ast_srl_yy_linebuf));

        // ast_srl_yylineno is set automatically to 0,
        // however the column needs to be reset.
        ast_srl_yycolumn = 1;

        // give back the line for the real lexing state
        yyless(0);

        // Continue to the previous lexing state
        yy_pop_state();
    }
    \n {
        // Save the empty line
        strncpy(ast_srl_yy_linebuf, "", sizeof(ast_srl_yy_linebuf));

        array_append(ast_srl_yy_lines, strdup(ast_srl_yy_linebuf));

        // ast_srl_yylineno is set automatically to 0,
        // however the column needs to be reset.
        ast_srl_yycolumn = 0;

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

"{"             { return '{'; }
"}"             { return '}'; }
","             { return ','; }
"="             { return '='; }
";"             { return ';'; }

"root"          { return T_ROOT;        }
"children"      { return T_CHILDREN;    }
"attributes"    { return T_ATTRIBUTES;  }
"false"         { return T_FALSE;       }
"true"          { return T_TRUE;        }


[A-Za-z][A-Za-z0-9_]*  {
    // Identifier start with an letter and can have numbers and underscores.
    ast_srl_yylval.string = strdup(ast_srl_yytext);
    return T_ID; }
-?[0-9]+\.[0-9]+ {
    // (negative) floating point numbers
    ast_srl_yylval.fval = strtold(ast_srl_yytext, NULL); return T_FLOATVAL;}
-[0-9]+ {
    // (negative) integer numbers
    ast_srl_yylval.intval = (int64_t)strtoll(ast_srl_yytext, NULL, 10); return T_INTVAL;}
[0-9]+ {
    // (positive) integer numbers
    ast_srl_yylval.uintval = (uint64_t)strtoull(ast_srl_yytext, NULL, 10); return T_UINTVAL;}

\"(\\.|[^\\"])*\"   {
    // Remove surrounding quotes
    ast_srl_yylval.string = strndup(ast_srl_yytext + 1, ast_srl_yyleng - 2); return T_STRINGVAL;}

[ \t\r]   ; // ignore all whitespace

\n  {
    // Switch to the SAVELINE state to read the whole line first
    yy_push_state(SAVELINE);
}
. {
    print_error_at(ast_srl_yylineno, ast_srl_yycolumn - 1, ast_srl_yy_linebuf,
        "Unexpected token found, quit parsing", ast_srl_yytext);
    exit(INVALID_CONFIG);
}
%%

static inline void lexer_init(void) {
    // We start by reading the whole first line, and then continue lexing it
    yy_push_state(SAVELINE);
}

static inline void token_action() {
    if (YY_START != SAVELINE) {
        ast_srl_yylloc.first_line = ast_srl_yylloc.last_line = ast_srl_yylineno;
        ast_srl_yylloc.first_column = ast_srl_yycolumn;
        ast_srl_yylloc.last_column = ast_srl_yycolumn + ast_srl_yyleng - 1;
        ast_srl_yycolumn += ast_srl_yyleng;
    }
}

int ast_srl_yywrap() {
    return (1);
}

void ast_srl_yyerror(const char* s) {
    print_error_range(ast_srl_yylineno, ast_srl_yycolumn - ast_srl_yyleng, ast_srl_yycolumn - 1,
        ast_srl_yy_linebuf, (char*) s);
    exit(INVALID_CONFIG);
}

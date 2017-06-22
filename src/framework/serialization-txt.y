%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "serialization-txt.lexer.h"
#include "framework/serialization-txt-ast.h"

#include "lib/array.h"
#include "lib/imap.h"
#include "lib/memory.h"

extern void lexer_init();
extern int yylex();
extern int yyparse();
extern int yylineno;
extern int yycolumn;
extern char* yytext;
extern FILE* yyin;

extern bool yy_lex_keywords;

static AST_TXT_File* parse_result = NULL;

imap_t *yy_parser_locations;
void yyerror(const char* s);
int yydebug = 1;

#define AST_SRL_YYLTYPE AST_SRL_YYLTYPE
typedef AST_TXT_ParserLocation AST_SRL_YYLTYPE;

AST_TXT_ParserLocation yy_parser_location;

/* static void new_location(void *ptr, AST_TXT_ParserLocation *loc); */

// Override YYLLOC_DEFAULT so we can set yy_parser_location
// to the current location
#define YYLLOC_DEFAULT(Cur, Rhs, N)                         \
    if (N) {                                                \
        (Cur).first_line   = YYRHSLOC(Rhs, 1).first_line;   \
        (Cur).first_column = YYRHSLOC(Rhs, 1).first_column; \
        (Cur).last_line    = YYRHSLOC(Rhs, N).last_line;    \
        (Cur).last_column  = YYRHSLOC(Rhs, N).last_column;  \
    } else {                                                \
        (Cur).first_line   = (Cur).last_line   =            \
          YYRHSLOC(Rhs, 0).last_line;                       \
        (Cur).first_column = (Cur).last_column =            \
          YYRHSLOC(Rhs, 0).last_column;                     \
    }                                                       \
    yy_parser_location = (Cur);

%}

%union {
    int64_t intval;
    uint64_t uintval;
    long double fval;
    char *string;
    char *str;
    array *array;
    AST_TXT_File *file;
    AST_TXT_Node *node;
    AST_TXT_NodeHeader *nodeheader;
    AST_TXT_Child *child;
    AST_TXT_Attribute *attribute;
    AST_TXT_AttributeValue *attributevalue;
}

%define api.prefix {ast_srl_yy}
%error-verbose
%locations

%token<intval>  T_INTVAL    "integer value"
%token<uintval> T_UINTVAL   "unsigned integer value"
%token<fval>    T_FLOATVAL  "float value"
%token<string>  T_STRINGVAL "string value"
%token<string>  T_ID        "identifier"

%token T_TRUE               "true"
%token T_FALSE              "false"

%token T_ROOT               "root"
%token T_ATTRIBUTES         "attributes"
%token T_CHILDREN           "children"
%token END 0                "End-of-file (EOF)"

%type<file> root
%type<node> node nodebody
%type<nodeheader> nodeheader
%type<child> child
%type<attribute> attribute
%type<attributevalue> attributevalue
%type<array> nodelist children attributes childlist attributelist

%start root

%%

root: nodelist  { $$ = _serialization_txt_create_file($1); }
    ;

nodelist: node nodelist  { $$ = $2;
                           array_append($$, $1); }
        | node           { $$ = create_array();
                           array_append($$, $1);
                         }
        ;

node: nodeheader '{' nodebody '}' optsemicolon { $$ = _serialization_txt_create_node($1, $3); }
    | nodeheader ';'                           { $$ = _serialization_txt_create_node($1, NULL); }
    ;

nodeheader: T_ID T_UINTVAL          { $$ = _serialization_txt_create_nodeheader($1, $2, false); }
          | T_ROOT T_ID T_UINTVAL   { $$ = _serialization_txt_create_nodeheader($2, $3, true); }
          ;

nodebody: children ',' attributes   { $$ = _serialization_txt_create_nodebody($1, $3); }
        | attributes ',' children   { $$ = _serialization_txt_create_nodebody($3, $1); }
        | children                  { $$ = _serialization_txt_create_nodebody($1, NULL); }
        | attributes                { $$ = _serialization_txt_create_nodebody(NULL, $1); }

children: T_CHILDREN '{' childlist '}' { $$ = $3; }
        ;

childlist: child ',' childlist           { $$ = $3;
                                           array_append($$, $1); }
         | child                         { $$ = create_array();
                                           array_append($$, $1);
                                         }
         ;

child: T_ID '=' T_UINTVAL                { $$ = _serialization_txt_create_child($1, $3); }

attributes: T_ATTRIBUTES '{' attributelist '}'  { $$ = $3; }
          ;

attributelist: attribute ',' attributelist  { $$ = $3;
                                              array_append($$, $1); }
             | attribute                    { $$ = create_array();
                                              array_append($$, $1);
                                            }
             ;

attribute: T_ID '=' attributevalue      { $$ = _serialization_txt_create_attribute($1, $3); }
         ;

attributevalue: T_ID            { $$ = _serialization_txt_create_attrval_id($1); }
              | T_STRINGVAL     { $$ = _serialization_txt_create_attrval_str($1); }
              | T_INTVAL        { $$ = _serialization_txt_create_attrval_int($1); }
              | T_UINTVAL       { $$ = _serialization_txt_create_attrval_uint($1); }
              | T_FLOATVAL      { $$ = _serialization_txt_create_attrval_float($1); }
              | T_TRUE          { $$ = _serialization_txt_create_attrval_bool(true); }
              | T_FALSE         { $$ = _serialization_txt_create_attrval_bool(false); }
              ;

optsemicolon: ';'
         | %empty
         ;
%%

/* static void new_location(void *ptr, struct ParserLocation *loc) { */
/*     struct ParserLocation *loc_copy = malloc(sizeof(struct ParserLocation)); */
/*     memcpy(loc_copy, loc, sizeof(struct ParserLocation)); */
/*  */
/*     imap_insert(yy_parser_locations, ptr, loc_copy); */
/* } */

AST_TXT_File *_serialization_txt_parse_file(FILE *fp) {
    ast_srl_yyin = fp;
    ast_srl_yyparse();
    ast_srl_yylex_destroy();
    return parse_result;
}

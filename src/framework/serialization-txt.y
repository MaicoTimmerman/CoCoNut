%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include "serialization-txt.lexer.h"
#include "framework/serialization-txt-ast.h"

#include "lib/array.h"
#include "lib/imap.h"
#include "lib/memory.h"
#include "lib/print.h"

extern void lexer_init();

static AST_TXT_File* parse_result = NULL;
static imap_t *yy_parser_locations;
array *ast_srl_yy_lines;

void yyerror(const char* s);
int yydebug = 1;

#define AST_SRL_YYLTYPE AST_SRL_YYLTYPE
typedef ParserLocation AST_SRL_YYLTYPE;

static ParserLocation yy_parser_location;

static void new_location(void *ptr, ParserLocation *loc);

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

root: nodelist  { parse_result = _serialization_txt_create_file($1); }
    ;

nodelist: node nodelist  { $$ = $2;
                           array_append($$, $1); }
        | node           { $$ = create_array();
                           array_append($$, $1);
                         }
        ;

node: nodeheader '{' nodebody '}' optsemicolon { $$ = _serialization_txt_create_node($1, $3);
                                                 new_location($$, &@$); }
    | nodeheader ';'                           { $$ = _serialization_txt_create_node($1, NULL);
                                                 new_location($$, &@$); }
    ;

nodeheader: T_ID T_UINTVAL          { $$ = _serialization_txt_create_nodeheader($1, $2, false);
                                      new_location($$, &@$);
                                      new_location($1, &@2); }
          | T_ROOT T_ID T_UINTVAL   { $$ = _serialization_txt_create_nodeheader($2, $3, true);
                                      new_location($$, &@$);
                                      new_location($2, &@2); }
          ;

nodebody: children ',' attributes   { $$ = _serialization_txt_create_nodebody($1, $3); }
        | attributes ',' children   { $$ = _serialization_txt_create_nodebody($3, $1); }
        | children                  { $$ = _serialization_txt_create_nodebody($1, NULL); }
        | attributes                { $$ = _serialization_txt_create_nodebody(NULL, $1); }

children: T_CHILDREN '{' childlist '}' { $$ = $3; new_location($$, &@$); }
        ;

childlist: child ',' childlist           { $$ = $3;
                                           array_append($$, $1); }
         | child                         { $$ = create_array();
                                           array_append($$, $1);
                                         }
         ;

child: T_ID '=' T_UINTVAL                { $$ = _serialization_txt_create_child($1, $3);
                                           new_location($$, &@$);
                                           new_location($1, &@1); }

attributes: T_ATTRIBUTES '{' attributelist '}'  { $$ = $3; }
          ;

attributelist: attribute ',' attributelist  { $$ = $3;
                                              array_append($$, $1); }
             | attribute                    { $$ = create_array();
                                              array_append($$, $1);
                                            }
             ;

attribute: T_ID '=' attributevalue      { $$ = _serialization_txt_create_attribute($1, $3);
                                          new_location($$, &@$);
                                          new_location($1, &@1); }
         ;

attributevalue: T_ID            { $$ = _serialization_txt_create_attrval_id($1);        new_location($$, &@$); }
              | T_STRINGVAL     { $$ = _serialization_txt_create_attrval_str($1);       new_location($$, &@$); }
              | T_INTVAL        { $$ = _serialization_txt_create_attrval_int($1);       new_location($$, &@$); }
              | T_UINTVAL       { $$ = _serialization_txt_create_attrval_uint($1);      new_location($$, &@$);}
              | T_FLOATVAL      { $$ = _serialization_txt_create_attrval_float($1);     new_location($$, &@$);}
              | T_TRUE          { $$ = _serialization_txt_create_attrval_bool(true);    new_location($$, &@$);}
              | T_FALSE         { $$ = _serialization_txt_create_attrval_bool(false);   new_location($$, &@$);}
              ;

optsemicolon: ';'
         | %empty
         ;
%%

static void new_location(void *ptr, struct ParserLocation *loc) {
    struct ParserLocation *loc_copy = malloc(sizeof(struct ParserLocation));
    memcpy(loc_copy, loc, sizeof(struct ParserLocation));
    imap_insert(yy_parser_locations, ptr, loc_copy);
}

AST_TXT_File *_serialization_txt_parse_file(char *fn) {
    ast_srl_yyin = fopen(fn ,"r");
    if (!ast_srl_yyin) {
        print_user_error(SERIALIZE_READ_TXT_ERROR_HEADER, "%s: cannot open file: %s",
        fn, strerror(errno));
        return NULL;
    }

    ast_srl_yy_lines = array_init(32);
    yy_parser_locations = imap_init(128);

    print_init_compilation_messages(SERIALIZE_READ_TXT_ERROR_HEADER, fn,
        ast_srl_yy_lines, yy_parser_locations);


    ast_srl_yyparse();
    ast_srl_yylex_destroy();

    return parse_result;
}

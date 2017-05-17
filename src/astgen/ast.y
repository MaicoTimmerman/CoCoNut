%{

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "astgen/ast.h"
#include "astgen/create-ast.h"

#include "ast.lexer.h"

#include "lib/array.h"
#include "lib/imap.h"

extern void lexer_init();
extern int yylex();
extern int yyparse();
extern int yylineno;
extern int yycolumn;
extern char* yytext;
extern FILE* yyin;

extern bool yy_lex_keywords;

/* Array to append config entries to during reducing */
static array *config_phases;
static array *config_passes;
static array *config_enums;
static array *config_traversals;
static array *config_nodesets;
static array *config_nodes;

static struct Config* parse_result = NULL;

imap_t *yy_parser_locations;
void yyerror(const char* s);
int yydebug = 1;

#define YYLTYPE YYLTYPE
typedef struct ParserLocation YYLTYPE;

struct ParserLocation yy_parser_location;

static void new_location(void *ptr, struct ParserLocation *loc);

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
    struct array *array;
    struct Config *config;
    struct Phase *phase;
    struct Pass *pass;
    struct Traversal *traversal;
    struct Enum *attr_enum;
    struct Nodeset *nodeset;
    struct Node *node;
    struct Child *child;
    struct PhaseRange *phaserange;
    struct MandatoryPhase *mandatoryphase;
    enum AttrType attrtype;
    struct Attr *attr;
    struct AttrValue *attrval;
}

%error-verbose
%locations

%token<intval> T_INTVAL "integer value"
%token<uintval> T_UINTVAL "unsigned integer value"
%token<fval> T_FLOATVAL "float value"
%token<string> T_CHARVAL "char value"
%token<string> T_STRINGVAL "string value"
%token<string> T_ID "identifier"

%token T_INT "int"
%token T_UINT "uint"

%token T_INT8 "int8"
%token T_INT16 "int16"
%token T_INT32 "int32"
%token T_INT64 "int64"
%token T_UINT8 "uint8"
%token T_UINT16 "uint16"
%token T_UINT32 "uint32"
%token T_UINT64 "uint64"

%token T_BOOL "bool"
%token T_TRUE "true"
%token T_FALSE "false"

%token T_ATTRIBUTES "attributes"
%token T_CHILDREN "children"
%token T_CONSTRUCT "construct"
%token T_CYCLE "cycle"
%token T_ENUM "enum"
%token T_MANDATORY "mandatory"
%token T_NODE "node"
%token T_NODES "nodes"
%token T_NODESET "nodeset"
%token T_PASS "pass"
%token T_PASSES "passes"
%token T_PHASE "phase"
%token T_PHASES "phases"
%token T_PREFIX "prefix"
%token T_INFO "info"
%token T_FUNC "func"
%token T_ROOT "root"
%token T_SUBPHASES "subphases"
%token T_TO "to"
%token T_TRAVERSAL "traversal"
%token T_FLOAT "float"
%token T_DOUBLE "double"
%token T_STRING "string"
%token T_VALUES "values"
%token T_NULL "NULL"
%token END 0 "End-of-file (EOF)"

%type<string> info func
%type<array> idlist mandatoryarglist mandatory
             attrlist attrs childlist children enumvalues traversalnodes
%type<mandatoryphase> mandatoryarg
%type<attrval> attrval
%type<attrtype> attrprimitivetype
%type<attr> attr attrhead
%type<child> child
%type<pass> pass
%type<node> nodebody node
%type<nodeset> nodeset
%type<phase> phase phaseheader
%type<attr_enum> enum
%type<traversal> traversal
%type<config> root

%start root

%%

/* Root of the config, creating the final config */
root: entry { parse_result = create_config(config_phases,
                                 config_passes,
                                 config_traversals,
                                 config_enums,
                                 config_nodesets,
                                 config_nodes); }
    ;

/* For every entry in the config, append to the correct array. */
entry: entry phase { array_append(config_phases, $2); }
     | entry pass { array_append(config_passes, $2); }
     | entry traversal { array_append(config_traversals, $2); }
     | entry enum { array_append(config_enums, $2); }
     | entry nodeset { array_append(config_nodesets, $2); }
     | entry node { array_append(config_nodes, $2);  }
     | %empty
     ;

phase: phaseheader '{' T_SUBPHASES '{' idlist '}' '}' ';'
     {
         $$ = create_phase($1, $5, NULL);
     }
     | phaseheader '{' T_PASSES '{' idlist '}' '}' ';'
     {
         $$ = create_phase($1, NULL, $5);
     }
     | phaseheader '{' info ',' T_SUBPHASES '{' idlist '}' '}' ';'
     {
         $$ = create_phase($1, $7, NULL);
         $$->info = $3;
     }
     | phaseheader '{' info ',' T_PASSES '{' idlist '}' '}' ';'
     {
         $$ = create_phase($1, NULL, $7);
         $$->info = $3;
     }
     ;

phaseheader: T_PHASE T_ID
           {
               $$ = create_phase_header($2, false, false);
               new_location($$, &@$);
               new_location($2, &@2);
           }
           | T_CYCLE T_PHASE T_ID
           {
               $$ = create_phase_header($3, false, true);
               new_location($$, &@$);
               new_location($3, &@3);
           }
           | T_ROOT T_PHASE T_ID
           {
               $$ = create_phase_header($3, true, false);
               new_location($$, &@$);
               new_location($3, &@3);
           }
           | T_ROOT T_CYCLE T_PHASE T_ID
           {
               $$ = create_phase_header($4, true, true);
               new_location($$, &@$);
               new_location($4, &@4);
           }
           ;

pass: T_PASS T_ID '{' T_FUNC '=' T_ID '}' ';'
    {
        $$ = create_pass($2, $6);
        new_location($$, &@$);
        new_location($2, &@2);
        new_location($6, &@6);
    }
    | T_PASS T_ID '{' info ',' T_FUNC '=' T_ID '}' ';'
    {
        $$ = create_pass($2, $8);
        $$->info = $4;
        new_location($$, &@$);
        new_location($2, &@2);
        new_location($8, &@8);
    }
    | T_PASS T_ID '{' info '}' ';'
    {
        $$ = create_pass($2, NULL);
        $$->info = $4;
        new_location($$, &@$);
        new_location($2, &@2);
    }
    | T_PASS T_ID ';'
    {
        $$ = create_pass($2, NULL);
        new_location($$, &@$);
        new_location($2, &@2);
    }
    ;


traversal: T_TRAVERSAL T_ID ';'
         {
             $$ = create_traversal($2, NULL, NULL);
             new_location($$, &@$);
             new_location($2, &@2);
         }
         | T_TRAVERSAL T_ID '{' func '}' ';'
         {
             $$ = create_traversal($2, $4, NULL);
             new_location($$, &@$);
             new_location($2, &@2);
         }
         | T_TRAVERSAL T_ID '{' func ',' traversalnodes '}' ';'
         {
             $$ = create_traversal($2, $4, $6);
             new_location($$, &@$);
             new_location($2, &@2);
         }
         | T_TRAVERSAL T_ID '{' traversalnodes '}' ';'
         {
             $$ = create_traversal($2, NULL, $4);
             new_location($$, &@$);
             new_location($2, &@2);
         }
         | T_TRAVERSAL T_ID '{' info '}' ';'
         {
             $$ = create_traversal($2, NULL, NULL);
             $$->info = $4;
             new_location($$, &@$);
             new_location($2, &@2);
         }
         | T_TRAVERSAL T_ID '{' info ',' func '}' ';'
         {
             $$ = create_traversal($2, $6, NULL);
             $$->info = $4;
             new_location($$, &@$);
             new_location($2, &@2);
         }
         | T_TRAVERSAL T_ID '{' info ',' func ',' traversalnodes '}' ';'
         {
             $$ = create_traversal($2, $6, $8);
             $$->info = $4;
             new_location($$, &@$);
             new_location($2, &@2);
         }
         | T_TRAVERSAL T_ID '{' info ',' traversalnodes '}' ';'
         {
             $$ = create_traversal($2, NULL, $6);
             $$->info = $4;
             new_location($$, &@$);
             new_location($2, &@2);
         }
         ;

func: T_FUNC '=' T_ID
    {
        $$ = $3;
        new_location($3, &@3);
    }

traversalnodes: T_NODES '{' idlist '}'
              {
                  $$ = $3;
              }

enum: T_ENUM T_ID '{' T_PREFIX '=' T_ID ',' enumvalues '}' ';'
    {
        $$ = create_enum($2, $6, $8);
        new_location($$, &@$);
        new_location($2, &@2);
        new_location($6, &@6);
    }
    | T_ENUM T_ID '{' enumvalues ',' T_PREFIX '=' T_ID '}' ';'
    {
        $$ = create_enum($2, $8, $4);
        new_location($$, &@$);
        new_location($2, &@2);
        new_location($8, &@6);
    }
    | T_ENUM T_ID '{' info ',' T_PREFIX '=' T_ID ',' enumvalues '}' ';'
    {
        $$ = create_enum($2, $8, $10);
        $$->info = $4;
        new_location($$, &@$);
        new_location($2, &@2);
        new_location($8, &@8);
    }
    | T_ENUM T_ID '{' info ',' enumvalues ',' T_PREFIX '=' T_ID '}' ';'
    {
        $$ = create_enum($2, $10, $6);
        $$->info = $4;
        new_location($$, &@$);
        new_location($2, &@2);
        new_location($10, &@10);
    }
    ;


enumvalues: T_VALUES '{'
        {
            yy_lex_keywords = false;
        }
        idlist  '}'
        {
            $$ = $4;
            yy_lex_keywords = true;
        }


nodeset: T_NODESET T_ID '{' T_NODES '{' idlist '}' '}' ';'
       {
           $$ = create_nodeset($2, $6);
           new_location($$, &@$);
           new_location($2, &@2);
       }
       | T_ROOT T_NODESET T_ID '{' T_NODES '{' idlist '}' '}' ';'
       {
           $$ = create_nodeset($3, $7);
           $$->root = true;
           new_location($$, &@$);
           new_location($3, &@3);
       }
       | T_NODESET T_ID '{' info ',' T_NODES '{' idlist '}' '}' ';'
       {
           $$ = create_nodeset($2, $8);
           $$->info = $4;
           new_location($$, &@$);
           new_location($2, &@2);
       }
       | T_ROOT T_NODESET T_ID '{' info ',' T_NODES '{' idlist '}' '}' ';'
       {
           $$ = create_nodeset($3, $9);
           $$->root = true;
           $$->info = $5;
           new_location($$, &@$);
           new_location($3, &@3);
       }
node: T_NODE T_ID '{' nodebody '}' ';'
    {
        $$ = create_node($2, $4);
        new_location($$, &@$);
        new_location($2, &@2);
    }
    | T_ROOT T_NODE T_ID '{' nodebody '}' ';'
    {
        $$ = create_node($3, $5);
        $$->root = true;
        new_location($$, &@$);
        new_location($3, &@3);
    }
    ;

/* All possible combinations of children attrs and flags, with allowing empty. */
nodebody: children ',' attrs
        {
            $$ = create_nodebody($1, $3);
            new_location($$, &@$);
        }
        | children
        {
            $$ = create_nodebody($1, NULL);
            new_location($$, &@$);
        }
        | attrs
        {
            $$ = create_nodebody(NULL, $1);
            new_location($$, &@$);
        }
        | info ',' children ',' attrs
        {
            $$ = create_nodebody($3, $5);
            $$->info = $1;
            new_location($$, &@$);
        }
        | info ',' children
        {
            $$ = create_nodebody($3, NULL);
            $$->info = $1;
            new_location($$, &@$);
        }
        | info ',' attrs
        {
            $$ = create_nodebody(NULL, $3);
            $$->info = $1;
            new_location($$, &@$);
        }
        ;

children: T_CHILDREN '{' childlist '}'
        {
            $$ = $3;
            new_location($$, &@$);
        }
        ;

childlist: childlist ',' child
         {
             array_append($1, $3);
             $$ = $1;
             // $$ is an array and should not be in the locations list
         }
         | child
         {
             array *tmp = create_array();
             array_append(tmp, $1);
             $$ = tmp;
             // $$ is an array and should not be in the locations list
         }
         ;
/* [construct] [mandatory] ID ID */
child: T_ID T_ID
     {
         $$ = create_child(0, 0, NULL, $2, $1);
         new_location($$, &@$);
         new_location($1, &@1);
         new_location($2, &@2);
     }
     | T_ID T_ID '{' T_CONSTRUCT ',' mandatory '}'
     {
         $$ = create_child(1, 1, $6, $2, $1);
         new_location($$, &@$);
         new_location($1, &@1);
         new_location($2, &@2);
     }
     | T_ID T_ID '{' mandatory ',' T_CONSTRUCT '}'
     {
         $$ = create_child(1, 1, $4, $2, $1);
         new_location($$, &@$);
         new_location($1, &@1);
         new_location($2, &@2);
     }
     | T_ID T_ID '{' T_CONSTRUCT '}'
     {
         $$ = create_child(1, 0, NULL, $2, $1);
         new_location($$, &@$);
         new_location($1, &@1);
         new_location($2, &@2);
     }
     | T_ID T_ID '{' mandatory '}'
     {
         $$ = create_child(0, 1, $4, $2, $1);
         new_location($$, &@$);
         new_location($1, &@1);
         new_location($2, &@2);
     }
     ;
attrs: T_ATTRIBUTES '{' attrlist '}'
     { $$ = $3; }
     ;
attrlist: attrlist ',' attr
        {
            array_append($1, $3);
            $$ = $1;
            // $$ is an array and should not be in the locations list
        }
        | attr
        {
            array *tmp = create_array();
            array_append(tmp, $1);
            $$ = tmp;
            // $$ is an array and should not be in the locations list
        }
        ;
attr: attrhead
    {
        $$ = create_attr($1, NULL);
        new_location($$, &@$);
    }
    | attrhead '=' attrval
    {
        $$ = create_attr($1, $3);
        new_location($$, &@$);
    }
    ;
/* Optional [construct] keyword, for adding to constructor. */
attrhead: attrprimitivetype T_ID
        {
            $$ = create_attrhead_primitive(0, $1, $2);
            new_location($$, &@$);
            new_location($2, &@2);
        }
        | T_CONSTRUCT attrprimitivetype T_ID
        {
            $$ = create_attrhead_primitive(1, $2, $3);
            new_location($$, &@$);
            new_location($3, &@3);
        }
        | T_ID T_ID
        {
            $$ = create_attrhead_idtype(0, $1, $2);
            new_location($$, &@$);
            new_location($1, &@1);
            new_location($2, &@2);
        }
        | T_CONSTRUCT T_ID T_ID
        {
            $$ = create_attrhead_idtype(1, $2, $3);
            new_location($$, &@$);
            new_location($2, &@2);
            new_location($3, &@3);
        }
        ;

attrprimitivetype: T_INT
                 { $$ = AT_int; }
                 | T_INT8
                 { $$ = AT_int8; }
                 | T_INT16
                 { $$ = AT_int16; }
                 | T_INT32
                 { $$ = AT_int32; }
                 | T_INT64
                 { $$ = AT_int64; }
                 | T_UINT
                 { $$ = AT_uint; }
                 | T_UINT8
                 { $$ = AT_uint8; }
                 | T_UINT16
                 { $$ = AT_uint16; }
                 | T_UINT32
                 { $$ = AT_uint32; }
                 | T_UINT64
                 { $$ = AT_uint64; }
                 | T_FLOAT
                 { $$ = AT_float; }
                 | T_DOUBLE
                 { $$ = AT_double; }
                 | T_BOOL
                 { $$ = AT_bool; }
                 | T_STRING
                 { $$ = AT_string; }
                 ;
attrval: T_STRINGVAL
       { $$ = create_attrval_string($1); }
       | T_INTVAL
       { $$ = create_attrval_int($1); }
       | T_UINTVAL
       { $$ = create_attrval_uint($1); }
       | T_FLOATVAL
       { $$ = create_attrval_float($1); }
       | T_ID
       { $$ = create_attrval_id($1); }
       |  T_TRUE
       { $$ = create_attrval_bool(true); }
       | T_FALSE
       { $$ = create_attrval_bool(false); }
       | T_NULL
       { $$ = NULL; }
       ;
mandatory: T_MANDATORY '{' mandatoryarglist '}'
         { $$ = $3;      }
         | T_MANDATORY
         { $$ = NULL;    }
         ;
mandatoryarglist: mandatoryarglist ',' mandatoryarg
                { array_append($1, $3); $$ = $1; }
                | mandatoryarg
                { array *tmp = create_array(); array_append(tmp, $1); $$ = tmp; }
                ;
/* Allow single phase or a range of phases. */
mandatoryarg: T_ID
            {
                $$ = create_mandatory_singlephase($1, 0);
                new_location($$, &@$);
                new_location($1, &@1);
            }
            | '!' T_ID
            {
                $$ = create_mandatory_singlephase($2, 1);
                new_location($$, &@$);
                new_location($2, &@2);
            }
            | T_ID T_TO T_ID
            {
                $$ = create_mandatory_phaserange($1, $3, 0);
                new_location($$, &@$);
                new_location($1, &@1);
                new_location($3, &@3);
            }
            | '!' '(' T_ID T_TO T_ID ')'
            {
                $$ = create_mandatory_phaserange($3, $5, 1);
                new_location($$, &@$);
                new_location($3, &@3);
                new_location($5, &@5);
            }
            ;
/* Comma seperated list of identifiers. */
idlist: idlist ',' T_ID
      {
          array_append($1, $3);
          $$ = $1;
          // $$ is an array and should not be added to location list.
          new_location($3, &@3);
      }
      | T_ID
      {
          array *tmp = create_array();
          array_append(tmp, $1);
          $$ = tmp;
          // $$ is an array and should not be added to location list.
          new_location($1, &@1);
      }
      ;

info: T_INFO '=' T_STRINGVAL
    {
        $$ = $3;
        new_location($$, &@$);
        new_location($3, &@3);
    }
%%

static void new_location(void *ptr, struct ParserLocation *loc) {
    struct ParserLocation *loc_copy = malloc(sizeof(struct ParserLocation));
    memcpy(loc_copy, loc, sizeof(struct ParserLocation));

    imap_insert(yy_parser_locations, ptr, loc_copy);
}

struct Config* parse(FILE *fp) {
    yyin = fp;
    config_phases = create_array();
    config_passes = create_array();
    config_enums = create_array();
    config_traversals = create_array();
    config_nodesets = create_array();
    config_nodes = create_array();
    yy_parser_locations = imap_init(128);
    yyparse();
    yylex_destroy();
    return parse_result;;
}

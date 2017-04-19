%{

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "array.h"
#include "imap.h"
#include "ast.h"
#include "create-ast.h"
#include "ast.lexer.h"

extern void lexer_init();
extern int yylex();
extern int yyparse();
extern int yylineno;
extern int yycolumn;
extern char* yytext;
extern FILE* yyin;

/* Array to append config entries to during reducing */
static array *config_phases;
static array *config_cycles;
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
    struct Cycle *cycle;
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
%token T_CHILD "child"
%token T_CONSTRUCT "construct"
%token T_CYCLE "cycle"
%token T_CYCLES "cycles"
%token T_ENUM "enum"
%token T_MANDATORY "mandatory"
%token T_NODE "node"
%token T_NODES "nodes"
%token T_NODESET "nodeset"
%token T_PHASE "phase"
%token T_PREFIX "prefix"
%token T_TO "to"
%token T_TRAVERSAL "traversal"
%token T_TRAVERSALS "traversals"
%token T_FLOAT "float"
%token T_DOUBLE "double"
%token T_STRING "string"
%token T_VALUES "values"
%token T_NULL "NULL"
%token END 0 "End-of-file (EOF)"

%type<array> idlist mandatoryarglist mandatory
             attrlist attrs childlist children
%type<mandatoryphase> mandatoryarg
%type<attrval> attrval
%type<attrtype> attrprimitivetype
%type<attr> attr attrhead
%type<child> child
%type<cycle> cycle
%type<node> nodebody node
%type<nodeset> nodeset
%type<phase> phase
%type<attr_enum> enum
%type<traversal> traversal
%type<config> root

%start root

%%

/* Root of the config, creating the final config */
root: entry { parse_result = create_config(config_phases,
                                 config_cycles,
                                 config_traversals,
                                 config_enums,
                                 config_nodesets,
                                 config_nodes); }
    ;

/* For every entry in the config, append to the correct array. */
entry: phase entry { array_append(config_phases, $1); }
     | cycle entry { array_append(config_cycles, $1); }
     | traversal entry { array_append(config_traversals, $1); }
     | enum entry { array_append(config_enums, $1); }
     | nodeset entry { array_append(config_nodesets, $1); }
     | node entry{ array_append(config_nodes, $1);  }
     | %empty
     ;

phase: T_PHASE T_ID '{' T_TRAVERSALS '{' idlist '}' '}' ';'
     {
         $$ = create_phase($2, $6, NULL);
         new_location($$, &@$);
         new_location($2, &@2);
     }
     | T_PHASE T_ID '{' T_TRAVERSALS '{' idlist '}' ',' T_CYCLES '{' idlist '}' '}' ';'
     {
         $$ = create_phase($2, $6, $11);
         new_location($$, &@$);
         new_location($2, &@2);
     }
     ;

cycle: T_CYCLE T_ID '{' T_TRAVERSALS '{' idlist '}' '}' ';'
     {
         $$ = create_cycle($2, $6, NULL);
         new_location($$, &@$);
         new_location($2, &@2);
     }
     | T_CYCLE T_ID '{' T_TRAVERSALS '{' idlist '}' ',' T_CYCLES '{' idlist '}' '}' ';'
     {
         $$ = create_cycle($2, $6, $11);
         new_location($$, &@$);
         new_location($2, &@2);
     }
     ;

traversal: T_TRAVERSAL T_ID ';'
         {
             $$ = create_traversal($2, NULL);
             new_location($$, &@$);
             new_location($2, &@2);
         }
         | T_TRAVERSAL T_ID '{' T_NODES '{' idlist '}' '}' ';'
         {
             $$ = create_traversal($2, $6);
             new_location($$, &@$);
             new_location($2, &@2);
         }
         ;

enum: T_ENUM T_ID '{' T_PREFIX '=' T_ID ',' T_VALUES '{' idlist  '}' '}' ';'
    {
        $$ = create_enum($2, $6, $10);
        new_location($$, &@$);
        new_location($2, &@2);
        new_location($6, &@6);
    }
    | T_ENUM T_ID '{' T_VALUES '{' idlist  '}' ',' T_PREFIX '=' T_ID '}' ';'
    {
        $$ = create_enum($2, $11, $6);
        new_location($$, &@$);
        new_location($2, &@2);
        new_location($6, &@6);
    }
    ;
nodeset: T_NODESET T_ID '{' idlist '}' ';'
       {
           $$ = create_nodeset($2, $4);
           new_location($$, &@$);
           new_location($2, &@2);
       }
       ;
node: T_NODE T_ID '{' nodebody '}' ';'
    {
        $$ = create_node($2, $4);
        new_location($$, &@$);
        new_location($2, &@2);
    }
    ;
/* All possible combinations of children attrs and flags, with allowing empty. */
nodebody: children ',' attrs
        {
            $$ = create_nodebody($1, $3, NULL);
            new_location($$, &@$);
        }
        | children
        {
            $$ = create_nodebody($1, NULL, NULL);
            new_location($$, &@$);
        }
        | attrs
        {
            $$ = create_nodebody(NULL, $1, NULL);
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
child: T_CHILD T_ID T_ID
     {
         $$ = create_child(0, 0, NULL, $3, $2);
         new_location($$, &@$);
         new_location($2, &@2);
         new_location($3, &@3);
     }
     | T_CHILD T_ID T_ID '{' T_CONSTRUCT ',' mandatory '}'
     {
         $$ = create_child(1, 1, $7, $3, $2);
         new_location($$, &@$);
         new_location($2, &@2);
         new_location($3, &@3);
     }
     | T_CHILD T_ID T_ID '{' mandatory ',' T_CONSTRUCT '}'
     {
         $$ = create_child(1, 1, $5, $3, $2);
         new_location($$, &@$);
         new_location($2, &@2);
         new_location($3, &@3);
     }
     | T_CHILD T_ID T_ID '{' T_CONSTRUCT '}'
     {
         $$ = create_child(1, 0, NULL, $3, $2);
         new_location($$, &@$);
         new_location($2, &@2);
         new_location($3, &@3);
     }
     | T_CHILD T_ID T_ID '{' mandatory '}'
     {
         $$ = create_child(0, 1, $5, $3, $2);
         new_location($$, &@$);
         new_location($2, &@2);
         new_location($3, &@3);
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
%%

static void new_location(void *ptr, struct ParserLocation *loc) {
    struct ParserLocation *loc_copy = malloc(sizeof(struct ParserLocation));
    memcpy(loc_copy, loc, sizeof(struct ParserLocation));

    imap_insert(yy_parser_locations, ptr, loc_copy);
}

struct Config* parse(void) {
    yyin = stdin;
    config_phases = create_array();
    config_cycles = create_array();
    config_enums = create_array();
    config_traversals = create_array();
    config_nodesets = create_array();
    config_nodes = create_array();
    yy_parser_locations = imap_init(128);
    yyparse();
    yylex_destroy();
    return parse_result;;
}

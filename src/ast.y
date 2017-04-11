%{

#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "ast-internal.h"

extern int yylex();
extern int yyparse();
extern int yylineno;
extern int yycolumn;
extern char* yytext;
extern FILE* yyin;

static array *config_phases;
static array *config_enums;
static array *config_traversals;
static array *config_nodesets;
static array *config_nodes;

static struct Config* parse_result = NULL;

void yyerror(const char* s);

int yydebug = 1;

%}

%union {
    long long ival;
    long double fval;
    char *string;
    char *str;
    int boolean;
    struct array *array;
    struct Config *config;
    struct Phase *phase;
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
    struct Flag *flag;
}

%error-verbose
%locations


%token<ival> T_INTVAL "integer value"
%token<fval> T_FLOATVAL "float value"
%token<string> T_STRINGVAL "string value"
%token<string> T_ID "identifier"

%token T_TRUE "true"
       T_FALSE "false"

%token T_ATTRIBUTES "\"attributes\""
       T_CHILDREN "children"
       T_CONSTRUCT "construct"
       T_ENUM "enum"
       T_FLAGS "flags"
       T_MANDATORY "mandatory"
%token T_NODE "node"
       T_NODES "nodes"
       T_NODESET "nodeset"
       T_TO "to"
       T_TRAVERSAL "traversal"

%token T_UNSIGNED "unsigned"
       T_CHAR "char"
       T_SHORT "short"
       T_INT "int"
       T_LONG "long"
       T_FLOAT "float"
       T_DOUBLE "double"
%token T_STRING "string"

%token END 0 "End-of-file (EOF)"

%type<array> idlist mandatoryarglist mandatory flaglist
             flags attrlist attrs childlist children
%type<boolean> bool
%type<mandatoryphase> mandatoryarg
%type<flag> flag
%type<attrval> attrval
%type<attrtype> attrprimitivetype
%type<attr> attr attrhead
%type<child> child
%type<node> nodebody node
%type<nodeset> nodeset
%type<attr_enum> enum
%type<traversal> traversal
%type<config> root

%start root

%%

root: entry { parse_result = create_config(create_array(),
                                 config_traversals,
                                 config_enums,
                                 config_nodesets,
                                 config_nodes); }
    ;

entry: traversal entry { array_append(config_traversals, $1); }
     | enum entry { array_append(config_enums, $1); }
     | nodeset entry { array_append(config_nodesets, $1); }
     | node entry{ array_append(config_nodes, $1);  }
     | %empty
     ;

traversal: T_TRAVERSAL T_ID ';'
         { $$ = create_traversal($2, NULL); }
         | T_TRAVERSAL T_ID '{' T_NODES '{' idlist '}' '}' ';'
         { $$ = create_traversal($2, $6);   }
         ;

enum: T_ENUM T_ID '{' idlist '}' ';'
    { $$ = create_enum($2, $4); }
    ;

nodeset: T_NODESET T_ID '{' idlist '}' ';'
       { $$ = create_nodeset($2, $4); }
       ;

node: T_NODE T_ID '{' nodebody '}' ';'
    { $$ = create_node($2, $4); }
    ;

nodebody: children ',' attrs ',' flags
        { $$ = create_nodebody($1, $3, $5);     }
        | attrs ',' flags
        { $$ = create_nodebody(NULL, $1, $3);   }
        | children ',' flags
        { $$ = create_nodebody($1, NULL, $3);   }
        | children ',' attrs
        { $$ = create_nodebody($1, $3, NULL);   }
        | children
        { $$ = create_nodebody($1, NULL, NULL); }
        | attrs
        { $$ = create_nodebody(NULL, $1, NULL); }
        | flags
        { $$ = create_nodebody(NULL, NULL, $1); }
        ;

children: T_CHILDREN '{' childlist '}'
        { $$ = $3; }
        ;

childlist: childlist ',' child
         {  array_append($1, $3); $$ = $1; }
         | child
         { array *tmp = create_array(); array_append(tmp, $1); $$ = tmp; }
         ;

child: T_ID T_ID
     { $$ = create_child(0, 0, NULL, $2, $1); }
     | mandatory T_ID T_ID
     { $$ = create_child(0, 1, $1, $3, $2);   }
     | T_CONSTRUCT T_ID T_ID
     { $$ = create_child(1, 0, NULL, $3, $2); }
     | T_CONSTRUCT mandatory T_ID T_ID
     { $$ = create_child(1, 1, $2, $4, $3);   }
     ;

attrs: T_ATTRIBUTES '{' attrlist '}'
     { $$ = $3; }
     ;

attrlist: attrlist ',' attr
        { array_append($1, $3); $$ = $1; }
        | attr
        { array *tmp = create_array(); array_append(tmp, $1); $$ = tmp; }
        ;

attr: attrhead
    { $$ = create_attr($1, NULL);}
    | attrhead '=' attrval
    { $$ = create_attr($1, $3);  }
    ;

attrhead: attrprimitivetype T_ID
        { $$ = create_attrhead_primitive(0, $1, $2); }
        | T_CONSTRUCT attrprimitivetype T_ID
        { $$ = create_attrhead_primitive(1, $2, $3); }
        | T_ID T_ID
        { $$ = create_attrhead_idtype(0, $1, $2);    }
        | T_CONSTRUCT T_ID T_ID
        { $$ = create_attrhead_idtype(1, $2, $3);    }
        ;

attrprimitivetype: T_CHAR
                 { $$ = AT_char;   }
                 | T_UNSIGNED T_CHAR
                 { $$ = AT_uchar;  }
                 | T_SHORT
                 { $$ = AT_short;  }
                 | T_UNSIGNED T_SHORT
                 { $$ = AT_ushort; }
                 | T_INT
                 { $$ = AT_int;    }
                 | T_UNSIGNED T_INT
                 { $$ = AT_uint;   }
                 | T_LONG
                 { $$ = AT_long;   }
                 | T_UNSIGNED T_LONG
                 { $$ = AT_ulong;  }
                 | T_LONG T_LONG
                 { $$ = AT_longlong;  }
                 | T_UNSIGNED T_LONG T_LONG
                 { $$ = AT_ulonglong; }
                 | T_FLOAT
                 { $$ = AT_float;  }
                 | T_DOUBLE
                 { $$ = AT_double; }
                 | T_LONG T_DOUBLE
                 { $$ = AT_longdouble; }
                 | T_STRING
                 { $$ = AT_string; }
                 ;

attrval: T_STRINGVAL
       { $$ = create_attrval_string($1); }
       | T_INTVAL
       { $$ = create_attrval_int($1); }
       | T_FLOATVAL
       { $$ = create_attrval_float($1); }
       | T_ID
       { $$ = create_attrval_id($1); }
       ;

flags: T_FLAGS '{' flaglist '}'
     { $$ = $3; }
     ;

flaglist: flaglist ',' flag
        { array_append($1, $3); $$ = $1; }
        | flag
        { array *tmp = create_array(); array_append(tmp, $1); $$ = tmp; }
        ;

flag: T_CONSTRUCT T_ID '=' bool
    { $$ = create_flag(1, $2, 1, $4); }
    | T_CONSTRUCT T_ID
    { $$ = create_flag(1, $2, 0, 0); }
    | T_ID '=' bool
    { $$ = create_flag(0, $1, 1, $3); }
    | T_ID
    { $$ = create_flag(0, $1, 0, 0); }
    ;

mandatory: T_MANDATORY '[' mandatoryarglist ']'
         { $$ = $3;      }
         | T_MANDATORY
         { $$ = NULL;    }
         ;

mandatoryarglist: mandatoryarglist ',' mandatoryarg
                { array_append($1, $3); $$ = $1; }
                | mandatoryarg
                { array *tmp = create_array(); array_append(tmp, $1); $$ = tmp; }
                ;

mandatoryarg: T_ID
            { $$ = create_mandatory_singlephase($1);    }
            | T_ID T_TO T_ID
            { $$ = create_mandatory_phaserange($1, $3); }
            ;

bool: T_TRUE
    { $$ = 1; }
    | T_FALSE
    { $$ = 0; }
    ;


idlist: idlist ',' T_ID
      { array_append($1, $3); $$ = $1; }
      | T_ID
      { array *tmp = create_array(); array_append(tmp, $1); $$ = tmp; }
      ;


%%
struct Config* parse(void) {
    yyin = stdin;

    config_phases = create_array();
    config_enums = create_array();
    config_traversals = create_array();
    config_nodesets = create_array();
    config_nodes = create_array();

    yyparse();

    return parse_result;;
}

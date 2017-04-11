%{

#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "ast-internal.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s);

int yydebug = 1;
extern int yylineno;
extern int yycolumn;
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


%token<ival> T_INTVAL
%token<fval> T_FLOATVAL
%token<string> T_STRINGVAL
%token<string> T_ID

%token T_TRUE T_FALSE

%token T_ATTRIBUTES T_CHILDREN T_CONSTRUCT T_ENUM T_FLAGS T_MANDATORY
%token T_NODE T_NODES T_NODESET T_TO T_TRAVERSAL

%token T_UNSIGNED T_CHAR T_SHORT T_INT T_LONG T_FLOAT T_DOUBLE
%token T_STRING

%type<array> idlist mandatoryarglist mandatory flaglist
             flags attrlist attrs childlist children
             nodes nodesets enums traversals
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

root: traversals enums nodesets nodes   { $$ = create_config(create_array(), $1, $2, $3, $4); }
    ;

traversals: traversal traversals        { array_append($2, $1); $$ = $2; }
          | %empty                      { $$ = create_array(); }
          ;

traversal: T_TRAVERSAL T_ID ';'                                 { $$ = create_traversal($2, NULL); }
         | T_TRAVERSAL T_ID '{' T_NODES '{' idlist '}' '}' ';'  { $$ = create_traversal($2, $6);   }
         ;

enums: enum enums                   { array_append($2, $1); $$ = $2; }
     | %empty                       { $$ = create_array(); }
     ;

enum: T_ENUM T_ID '{' idlist '}' ';'    { $$ = create_enum($2, $4); }
    ;

nodesets: nodeset nodesets      { array_append($2, $1); $$ = $2; }
        | %empty                { $$ = create_array();           }
        ;

nodeset: T_NODESET T_ID '{' idlist '}' ';'  { $$ = create_nodeset($2, $4); }
       ;

nodes: node nodes               { array_append($2, $1); $$ = $2; }
     | %empty                   { $$ = create_array(); }
     ;

node: T_NODE T_ID '{' nodebody '}' ';'      { $$ = create_node($2, $4); }
    ;

nodebody: children ',' attrs ',' flags      { $$ = create_nodebody($1, $3, $5);     }
        | attrs ',' flags                   { $$ = create_nodebody(NULL, $1, $3);   }
        | children ',' flags                { $$ = create_nodebody($1, NULL, $3);   }
        | children ',' attrs                { $$ = create_nodebody($1, $3, NULL);   }
        | children                          { $$ = create_nodebody($1, NULL, NULL); }
        | attrs                             { $$ = create_nodebody(NULL, $1, NULL); }
        | flags                             { $$ = create_nodebody(NULL, NULL, $1); }

        ;

children: T_CHILDREN '{' childlist '}' { $$ = $3; }
        ;

childlist: child childlist      {  array_append($2, $1);
                                   $$ = $2;
                                }
         | child                { array *tmp = create_array();
                                  array_append(tmp, $1);
                                  $$ = tmp; 
                                }
         ;

child: T_ID T_ID                        { $$ = create_child(0, 0, NULL, $2, $1); }
     | mandatory T_ID T_ID              { $$ = create_child(0, 1, $1, $3, $2);   }
     | T_CONSTRUCT T_ID T_ID            { $$ = create_child(1, 0, NULL, $3, $2); }
     | T_CONSTRUCT mandatory T_ID T_ID  { $$ = create_child(1, 1, $2, $4, $3);   }
     ;

attrs: T_ATTRIBUTES '{' attrlist '}'    { $$ = $3; }
     ;

attrlist: attr ',' attrlist         { array_append($3, $1);
                                      $$ = $3; 
                                    }
        | attr                      { array *tmp = create_array();
                                      array_append(tmp, $1);
                                      $$ = tmp; 
                                    }
        ;

attr: attrhead                  { $$ = create_attr($1, NULL);}
    | attrhead '=' attrval      { $$ = create_attr($1, $3);  }

attrhead: attrprimitivetype T_ID                { $$ = create_attrhead_primitive(0, $1, $2); }
        | T_CONSTRUCT attrprimitivetype T_ID    { $$ = create_attrhead_primitive(1, $2, $3); }
        | T_ID T_ID                             { $$ = create_attrhead_idtype(0, $1, $2);    } 
        | T_CONSTRUCT T_ID T_ID                 { $$ = create_attrhead_idtype(1, $2, $3);    }
        ;

attrprimitivetype: T_CHAR                       { $$ = AT_char;   }
                 | T_UNSIGNED T_CHAR            { $$ = AT_uchar;  }
                 | T_SHORT                      { $$ = AT_short;  }
                 | T_UNSIGNED T_SHORT           { $$ = AT_ushort; }
                 | T_INT                        { $$ = AT_int;    }
                 | T_UNSIGNED T_INT             { $$ = AT_uint;   }
                 | T_LONG                       { $$ = AT_long;   }
                 | T_UNSIGNED T_LONG            { $$ = AT_ulong;  }
                 | T_LONG T_LONG                { $$ = AT_longlong;  }
                 | T_UNSIGNED T_LONG T_LONG     { $$ = AT_ulonglong; }
                 | T_FLOAT                      { $$ = AT_float;  }
                 | T_DOUBLE                     { $$ = AT_double; }
                 | T_LONG T_DOUBLE              { $$ = AT_longdouble; }
                 | T_STRING                     { $$ = AT_string; }
                 ;

attrval: T_STRINGVAL    { $$ = create_attrval_string($1); }
       | T_INTVAL       { $$ = create_attrval_int($1); }
       | T_FLOATVAL     { $$ = create_attrval_float($1); }
       | T_ID           { $$ = create_attrval_id($1); }
       ;

flags: T_FLAGS '{' flaglist '}'                         { $$ = $3; }
     ;

flaglist: flag ',' flaglist                             { array_append($3, $1);
                                                          $$ = $3;
                                                        }
        | flag                                          { array *tmp = create_array();
                                                          array_append(tmp, $1);
                                                          $$ = tmp;
                                                        }
        ;

flag: T_CONSTRUCT T_ID '=' bool                         { $$ = create_flag(1, $2, 1, $4); }
    | T_CONSTRUCT T_ID                                  { $$ = create_flag(1, $2, 0, 0); }
    | T_ID '=' bool                                     { $$ = create_flag(0, $1, 1, $3); }
    | T_ID                                              { $$ = create_flag(0, $1, 0, 0); }
    ;

mandatory: T_MANDATORY '[' mandatoryarglist ']'         { $$ = $3;      }
         | T_MANDATORY                                  { $$ = NULL;    }
         ;

mandatoryarglist: mandatoryarg ',' mandatoryarglist     { array_append($3, $1);
                                                          $$ = $3;
                                                        }
                | mandatoryarg                          { array *tmp = create_array();
                                                          array_append(tmp, $1);
                                                          $$ = tmp;
                                                        }
                ;

mandatoryarg: T_ID              { $$ = create_mandatory_singlephase($1);    }
            | T_ID T_TO T_ID    { $$ = create_mandatory_phaserange($1, $3); }
            ;

bool: T_TRUE                { $$ = 1; }
    | T_FALSE               { $$ = 0; }
    ;


idlist: idlist ',' T_ID     {
                                array_append($1, $3);
                                $$ = $1;
                            }
      | T_ID                {
                                array *tmp = create_array();
                                array_append(tmp, $1);
                                $$ = tmp;
                            }

      ;


%%
struct Config* parse(void) {
    yyin = stdin;

    do {
        yyparse();
    } while(!feof(yyin));

    return NULL;
}

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d, col %d: %s\n", yylineno, yycolumn, s);
    exit(1);
}

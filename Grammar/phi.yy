%language "C++"
%defines
%locations

%define parser_class_name {Parser}
%define api.namespace {Phi}

%{
    // C STD
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>

    // Project Headers
    #include "Context.h"

    void yyerror(char *);
    int yylex();
    int yydebug=1;

    extern int yylineno;
    extern char* yytext;
%}

%parse-param { Phi::Context* context }
%lex-param   { Phi::Context* context }

%union {
    char* text;
}

%token ASSIGNMENT

%token WHITESPACE
%token NEWLINE

%token KEYWORD_MODULE
%token KEYWORD_INTERFACE
%token KEYWORD_NAMESPACE
%token KEYWORD_IF
%token KEYWORD_ELSE
%token KEYWORD_WHILE
%token KEYWORD_FOR
%token KEYWORD_IN
%token KEYWORD_WHEN
%token KEYWORD_SWITCH
%token KEYWORD_MUX
%token KEYWORD_CASE
%token KEYWORD_DEFAULT
%token KEYWORD_SW_VAR
%token KEYWORD_WIRE
%token KEYWORD_REGISTER
%token KEYWORD_SYNC
%token KEYWORD_ASYNC
%token KEYWORD_INPUT
%token KEYWORD_OUTPUT
%token KEYWORD_POSEDGE
%token KEYWORD_NEGEDGE
%token ANNOTATION

%token NUMERIC
%token FW_NUMERIC
%token IDENTIFIER

%token OP_RANGE
%token OP_SRL
%token OP_SRA
%token OP_SLL
%token OP_EQ
%token OP_NEQ
%token OP_GTE
%token OP_LTE
%token OP_UNSIGNED_ADD
%token OP_UNSIGNED_SUB
%token OP_UNSIGNED_LT
%token OP_UNSIGNED_LTE
%token OP_UNSIGNED_GT
%token OP_UNSIGNED_GTE

%token LEFT_CAT
%token RIGHT_CAT

%right '=' ':' 

%right '?'
%left OP_RANGE
%left OP_LOGIC_OR
%left OP_LOGIC_AND
%left OP_EQ OP_NEQ OP_GTE OP_LTE '<' '>' OP_UNSIGNED_LT OP_UNSIGNED_LTE OP_UNSIGNED_GT OP_UNSIGNED_GTE
%left OP_UNSIGNED_ADD OP_UNSIGNED_SUB '+' '-' '|' '&' '^'
%left '*' '/' '%'
%left OP_SRL OP_SRA OP_SLL
%left '~' KEYWORD_POSEDGE KEYWORD_NEGEDGE UNARY
%left '.'
%right '['

%type<text> NUMERIC IDENTIFIER

%{
    extern int yylex(Phi::Parser::semantic_type* yylval,
                     Phi::Parser::location_type* yylloc,
                     Phi::Context* context
    );
%}

%initial-action {
    @$.begin.filename = @$.end.filename = &context->files.back();
}

%%

/* Top Level */

description:
    | declaration description
    | KEYWORD_NAMESPACE IDENTIFIER '{' description '}'
    ;

declaration:
    KEYWORD_MODULE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance block
    | KEYWORD_MODULE error '}'
    | KEYWORD_INTERFACE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance ';'
    | KEYWORD_INTERFACE error ';'
    ;

/* Ports */
port_declaration_list:
    | populated_port_declaration_list
    ;
populated_port_declaration_list:
    IDENTIFIER ':' port_declaration ',' populated_port_declaration_list
    | IDENTIFIER ':' port_declaration
    ;
port_declaration:
    port_polarity
    | ANNOTATION port_polarity
    ;
port_polarity:
    KEYWORD_INPUT optional_array_subscript
    | KEYWORD_OUTPUT optional_array_subscript
    ;

/* Templating */
template_declaration:
    | '<' template_declaration_list '>'
    ;
template_declaration_list:
    IDENTIFIER optional_template_assignment template_declaration_list
    | IDENTIFIER optional_template_assignment
    ;
optional_template_assignment:
    | '=' '(' expression ')'
    ;

/* Inheritance */
inheritance:
    | ':' inheritance_list
    ;
inheritance_list:
    expression ',' inheritance_list
    | expression
    ;

/* Statements */

statement:
    subdeclaration ';'
    | nondeclarative_statement ';'
    | block_based
    | error ';'
    ;

/* Blocks */
block_based:
    if
    | KEYWORD_FOR IDENTIFIER KEYWORD_IN range block
    | KEYWORD_NAMESPACE IDENTIFIER block
    | KEYWORD_SWITCH expression switch_block
    | '@' expression block
    | KEYWORD_ASYNC block
    | error '}'
    ;
if:
    KEYWORD_IF expression block else
    ;

else:
    | KEYWORD_ELSE if
    | KEYWORD_ELSE expression block
    ;

switch_block:
    '{' labeled_statement_list '}'
    ;
labeled_statement_list:
    | KEYWORD_CASE number ':' statement_list labeled_statement_list
    | KEYWORD_DEFAULT ':' statement_list
    ;

block:
    '{' statement_list '}'
    ;

statement_list:
    | statement_list statement
    ;

/* Subdeclarations */

subdeclaration:
    dynamic_width optional_array_subscript declaration_list
    | probable_template IDENTIFIER optional_array_subscript optional_ports
    ;

dynamic_width:
    KEYWORD_SW_VAR
    | KEYWORD_WIRE 
    | KEYWORD_REGISTER
    ;
optional_array_subscript:
    | '[' expression ']'
    ;
optional_ports:
    | ports
    ;
declaration_list:
    IDENTIFIER optional_array_subscript optional_assignment ',' declaration_list
    | IDENTIFIER optional_array_subscript optional_assignment
    ;
optional_assignment:
    | '=' expression
    ;

probable_template:
    expression
    | expression '<' template_list '>'
    ;
template_list:
    | IDENTIFIER ':' '(' expression ')' ',' template_list
    | IDENTIFIER ':' '(' expression ')'
    ;
ports:
    '(' ')'
    | '(' port_list ')'
    ;
port_list:
    IDENTIFIER ':' expression ',' port_list
    | IDENTIFIER ':' expression
    ;

/* Nondeclarative Statements */
    
nondeclarative_statement:
    expression '=' expression
    | expression ports
    ;

/* Expressions */

expression:
    expression '?' expression ':' expression
    | expression OP_EQ expression
    | expression OP_NEQ expression
    | expression '>' expression
    | expression '<' expression
    | expression OP_GTE expression
    | expression OP_LTE expression
    | expression OP_UNSIGNED_LT expression
    | expression OP_UNSIGNED_GT  expression
    | expression OP_UNSIGNED_LTE expression
    | expression OP_UNSIGNED_GTE expression
    | expression '+' expression
    | expression '-' expression
    | expression OP_UNSIGNED_ADD expression
    | expression OP_UNSIGNED_SUB expression
    | expression '|' expression
    | expression '&' expression
    | expression '^' expression
    | expression '*' expression
    | expression '/' expression
    | expression '%' expression
    | expression OP_SLL expression
    | expression OP_SRL expression
    | expression OP_SRA expression
    | range
    | '-' expression %prec UNARY
    | '&' expression %prec UNARY
    | '|' expression %prec UNARY
    | '~' expression %prec UNARY
    | KEYWORD_POSEDGE expression
    | KEYWORD_NEGEDGE expression
    | expression '.' expression
    | expression '[' expression ']'
    | '[' concatenation ']'
    | '(' expression ')'
    | '$' IDENTIFIER '(' procedural_call ')'
    | KEYWORD_MUX expression mux_block
    | IDENTIFIER
    | number
    ;

range:
    expression OP_RANGE expression
    ;

mux_block:
    '{' labeled_expression_list '}'
    ;
labeled_expression_list:
    | KEYWORD_CASE number ':' expression ';' labeled_expression_list
    | KEYWORD_DEFAULT ':' expression ';'
    ;

concatenation:
    concatenatable ',' concatenation
    | concatenatable
    ;
concatenatable:
    expression
    | expression LEFT_CAT expression RIGHT_CAT
    ;

procedural_call:
    | procedural_call_list
    ;
procedural_call_list:
    expression ',' procedural_call_list
    | expression
    ;

number:
    NUMERIC
    | FW_NUMERIC
    ;

%%
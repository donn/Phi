%{
    // C STD
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    // Project Headers
    #include "Node.h"
    #include "Errors.h"

    void yyerror(char *);
    int yylex();
    int yydebug=1;

    extern struct Node* head;
    extern int yylineno;
    extern char* yytext;
%}

%union {
    struct Node* node;
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
%token KEYWORD_WHEN
%token KEYWORD_SWITCH
%token KEYWORD_MUX
%token KEYWORD_CASE
%token KEYWORD_SW_VAR
%token KEYWORD_WIRE
%token KEYWORD_REGISTER

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
%token OP_LOGIC_OR
%token OP_LOGIC_AND
%token OP_UNSIGNED_ADD
%token OP_UNSIGNED_SUB
%token OP_UNSIGNED_LT
%token OP_UNSIGNED_LTE
%token OP_UNSIGNED_GT
%token OP_UNSIGNED_GTE

%right '=' ':' 

%right '?'
%left OP_RANGE
%left OP_LOGIC_OR
%left OP_LOGIC_AND
%left OP_EQ OP_NEQ OP_GTE OP_LTE '<' '>' OP_UNSIGNED_LT OP_UNSIGNED_LTE OP_UNSIGNED_GT OP_UNSIGNED_GTE
%left OP_UNSIGNED_ADD OP_UNSIGNED_SUB '+' '-' '~' '|' '&' '^'
%left  '*' '/' '%'
%left OP_SRL OP_SRA OP_SLL
%left '.' '['

%type<text> NUMERIC IDENTIFIER
%%

description:
    | declaration description
    ;

declaration:
    KEYWORD_MODULE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance block
    | KEYWORD_MODULE error '}'
    | KEYWORD_INTERFACE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance ';'
    | KEYWORD_INTERFACE error ';'
    ;

port_declaration_list:
    | populated_port_declaration_list
    ;
populated_port_declaration_list:
    IDENTIFIER ':' lhexpression ',' populated_port_declaration_list
    | IDENTIFIER ':' lhexpression
    ;

template_declaration:
    | '<' template_declaration_list '>'
    ;
template_declaration_list:
    IDENTIFIER '=' number
    | IDENTIFIER
    ;

inheritance:
    | ':' inheritance_list
    ;
inheritance_list:
    IDENTIFIER '.' inheritance_list
    | IDENTIFIER
    ;

block:
    '{' statement_list '}'
    ;
statement_list:
    | statement_list statement
    ;

switch_block:
    '{' labeled_statement_list '}'
    ;
labeled_statement_list:
    | KEYWORD_CASE number ':' statement_list labeled_statement_list
    ;

statement:
    subdeclaration ';'
    | nondeclarative_statement ';'
    | KEYWORD_NAMESPACE IDENTIFIER block
    | '@' expression block
    | KEYWORD_IF expression block
    | KEYWORD_SWITCH expression switch_block
    | KEYWORD_MUX expression switch_block
    | error ';'
    | error '}'
    ;

subdeclaration:
    subscriptable_dynamic_width declaration_list
    | probable_template IDENTIFIER ports
    ;

subscriptable_dynamic_width:
    dynamic_width
    | dynamic_width array_subscript
    ;
dynamic_width:
    KEYWORD_SW_VAR
    | KEYWORD_WIRE 
    | KEYWORD_REGISTER
    ;
declaration_list:
    lhexpression optional_assignment ',' declaration_list
    | lhexpression optional_assignment
    ;
optional_assignment:
    | '=' expression
    ;

probable_template:
    lhexpression
    | lhexpression '<' template_list '>' %prec OP_SRL
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
    
nondeclarative_statement:
    lhexpression '=' expression
    ;

expression:
    expression '?' expression ':' expression
    | expression OP_LOGIC_OR expression
    | expression OP_LOGIC_AND expression
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
    | expression OP_RANGE expression
    | '~' expression
    | '[' concatenation ']'
    | lhexpression
    | '$' IDENTIFIER '(' procedural_call ')'
    | '(' expression ')'
    | number
    ;

concatenation:
    expression ',' concatenation
    | expression '{' expression '}'
    | expression
    ;

procedural_call:
    | procedural_call_list
    ;
procedural_call_list:
    expression ',' procedural_call_list
    | expression
    ;

lhexpression:
    lhexpression '.' lhexpression
    | lhexpression '[' expression ']'
    | IDENTIFIER
    ;
    
array_subscript:
    '[' expression ']'
    ;

number:
    NUMERIC
    | FW_NUMERIC
    ;

%%

void yyerror(char *error) {
    Phi_appendError(yylineno, -1, error, yytext);
}

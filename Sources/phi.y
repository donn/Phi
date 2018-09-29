%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "Node.h"

    void yyerror(char *);
    int yylex();

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

%right '='

%left OP_LOGIC_OR
%left OP_LOGIC_AND
%left OP_EQ OP_NEQ OP_GTE OP_LTE '<' '>'
%left OP_UNSIGNED_ADD OP_UNSIGNED_SUB '+' '-' '~' '|' '&' '^'
%left  '*' '/' '%'
%left OP_SRL OP_SRA OP_SLL
%left ':' '.'

%type<text> NUMERIC IDENTIFIER
%%

description:
    | declaration description
    ;

declaration:
    KEYWORD_MODULE probable_template call inheritance block
    | KEYWORD_INTERFACE probable_template call inheritance ';'
    ;

inheritance:
    | ':' unlabeled_list
    ;

block:
    '{' statement_list '}'
    ;
switch_block:
    '{' labeled_statement_list '}'
    ;

statement_list:
    | statement_list statement
    ;
labeled_statement_list:
    | KEYWORD_CASE number ':' statement_list switch_block
    ;

statement:
    subdeclaration ';'
    | nondeclarative_statement ';'
    | KEYWORD_NAMESPACE IDENTIFIER block
    | '@' expression block
    | KEYWORD_IF expression block
    | KEYWORD_SWITCH expression switch_block
    | KEYWORD_MUX expression switch_block
    ;
subdeclaration:
    dynamic_width array_subscript probable_array optional_assignment
    | dynamic_width probable_array optional_assignment
    | probable_template probable_array optional_call
    ;
nondeclarative_statement:
    lhexpression '=' expression
    ;

dynamic_width:
    KEYWORD_SW_VAR
    | KEYWORD_WIRE 
    | KEYWORD_REGISTER
    ;
optional_assignment:
    | '=' expression
    ;
optional_call:
    | call
    ;
call:
    '(' labeled_list ')'
    ;

unlabeled_list:
    unlabeled_list ',' IDENTIFIER
    | IDENTIFIER
    ;
unlabeled_expression_list:
    unlabeled_list ',' expression
    | expression
    ;
labeled_list:
    | IDENTIFIER ':' expression ',' labeled_list
    | IDENTIFIER ':' expression
    ;

expression:
    expression OP_LOGIC_OR expression
    | expression OP_LOGIC_AND expression
    | expression OP_EQ expression
    | expression OP_NEQ expression
    | expression '>' expression
    | expression '<' expression
    | expression OP_GTE expression
    | expression OP_LTE expression
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
    | expression ':' expression
    | lhexpression optional_call
    | '$' IDENTIFIER '(' unlabeled_expression_list ')'
    | number
    ;
lhexpression:
    | lhexpression '.' lhexpression
    | probable_array
    ;


number:
    NUMERIC
    | FW_NUMERIC
    ;

probable_template:
    IDENTIFIER
    | IDENTIFIER '<' unlabeled_list '>'
    ;
probable_array:
    IDENTIFIER
    | IDENTIFIER '[' expression ']'
    ;

array_subscript:
    '[' expression ']'
    ;

%%

void yyerror(char *error) {
    fprintf(stderr, "Line %d: %s: '%s'\n", yylineno, error, yytext);
}

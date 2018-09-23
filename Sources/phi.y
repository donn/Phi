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

%token KEYWORD_MODULE
%token KEYWORD_INTERFACE
%token KEYWORD_IF
%token KEYWORD_ELSE
%token KEYWORD_WHILE
%token KEYWORD_WHEN
%token KEYWORD_MUX
%token KEYWORD_SW_VAR
%token KEYWORD_WIRE
%token KEYWORD_REGISTER

%token NUMERIC
%token IDENTIFIER

%token LBRACE
%token RBRACE
%token LBRACKET
%token RBRACKET
%token LPAREN
%token RPAREN

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

%type<text> NUMERIC IDENTIFIER
%%

description:
        declaration description
        |
        ;

declaration:
        KEYWORD_MODULE IDENTIFIER inheritance block
        | KEYWORD_INTERFACE IDENTIFIER inheritance block
        ;

inheritance:
        ':' inheritance_list
        |
        ;

inheritance_list:
        IDENTIFIER ',' inheritance_list 
        | IDENTIFIER
        ;

block:
    LBRACE statement RBRACE
    ;

statement:
    declaration
    | subdeclaration
    | if
    ;

if:
    KEYWORD_IF expression block elif
    ;

elif:
    KEYWORD_ELSE block
    | KEYWORD_ELSE if
    |
    ;


subdeclaration:
    IDENTIFIER IDENTIFIER arraysubscript call
    | IDENTIFIER IDENTIFIER arraysubscript
    | declarables IDENTIFIER arraysubscript '=' expression
    ;
declarables:
    KEYWORD_SW_VAR
    | KEYWORD_WIRE arraysubscript
    | KEYWORD_REGISTER arraysubscript
    ;

call:
    LPAREN call_list RPAREN
    | LPAREN RPAREN
    ;
call_list:
    IDENTIFIER ':' IDENTIFIER ',' call_list
    | IDENTIFIER ':' IDENTIFIER
    ;

expression:
    expression_logic_or
    ;

expression_logic_or:
    expression_logic_and
    | expression_logic_or OP_LOGIC_OR expression_logic_and
    ;

expression_logic_and:
    expression_comparison
    | expression_logic_and OP_LOGIC_AND expression_comparison
    ;

expression_comparison:
    expression_addition
    | expression_comparison OP_EQ expression_addition
    | expression_comparison OP_NEQ expression_addition
    | expression_comparison '>' expression_addition
    | expression_comparison '<' expression_addition
    | expression_comparison OP_GTE expression_addition
    | expression_comparison OP_LTE expression_addition
    ;

expression_addition:
    expression_multiplication
    | expression_addition '+' expression_multiplication
    | expression_addition '-' expression_multiplication
    | expression_addition OP_UNSIGNED_ADD expression_multiplication
    | expression_addition OP_UNSIGNED_SUB expression_multiplication
    | expression_addition '|' expression_multiplication
    | expression_addition '&' expression_multiplication
    | expression_addition '^' expression_multiplication
    ;

expression_multiplication:
    expression_shift
    | expression_multiplication '*' expression_shift
    | expression_multiplication '/' expression_shift
    ;

expression_shift:
    ;
    
arraysubscript:
    LBRACKET expression RBRACKET
    |
    ;
%%

void yyerror(char *error) {
    fprintf(stderr, "Line %d: %s: '%s'\n", yylineno, error, yytext);
}

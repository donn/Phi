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
%token FW_NUMERIC
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

%left OP_LOGIC_OR
%left OP_LOGIC_AND
%left OP_EQ OP_NEQ OP_GTE OP_LTE '<' '>'
%left OP_UNSIGNED_ADD OP_UNSIGNED_SUB '+' '-' '~' '|' '&' '^'
%left  '*' '/' '%'
%left OP_SRL OP_SRA OP_SLL
%left ':'

%type<text> NUMERIC IDENTIFIER
%%

description:
        declaration ';' description
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
    declaration statement ';'
    | subdeclaration ';' statement
    | if statement
    |
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
    IDENTIFIER subscriptable call
    | IDENTIFIER subscriptable
    | declarables subscriptable '=' expression
    ;
declarables:
    KEYWORD_SW_VAR arraysubscript
    | KEYWORD_WIRE arraysubscript
    | KEYWORD_REGISTER arraysubscript
    | KEYWORD_SW_VAR
    | KEYWORD_WIRE
    | KEYWORD_REGISTER
    ;

subscriptable:
    IDENTIFIER arraysubscript
    | IDENTIFIER
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
    | number
    | expression OP_LOGIC_OR expression
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
    | IDENTIFIER
    ;

number:
    NUMERIC
    | FW_NUMERIC
    ;

arraysubscript:
    LBRACKET expression RBRACKET
    ;
%%

void yyerror(char *error) {
    fprintf(stderr, "Line %d: %s: '%s'\n", yylineno, error, yytext);
}

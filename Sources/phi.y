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

%right '='
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
        declaration statement_delimiter description
        |
        ;

declaration:
        KEYWORD_MODULE _ IDENTIFIER _ inheritance block
        | KEYWORD_INTERFACE _ IDENTIFIER _ inheritance block
        ;

inheritance:
        ':' _ inheritance_list
        |
        ;

inheritance_list:
        IDENTIFIER _ ',' _ inheritance_list 
        | IDENTIFIER
        ;

block:
    LBRACE _ statement _ RBRACE
    ;

statement:
    subdeclaration WHITESPACE statement_delimiter
    ;

subdeclaration:
    dynamic_width _ array_subscript _ probable_array optional_assignment
    ;

dynamic_width:
    KEYWORD_SW_VAR
    | KEYWORD_WIRE 
    | KEYWORD_REGISTER
    ;
optional_assignment:
    _ '=' _ expression
    |
    ;
optional_call:
    _ LPAREN _ call_list RPAREN
    | _ LPAREN _ RPAREN
    |
    ;
call_list:
    IDENTIFIER _ ':' _ IDENTIFIER _ ',' call_list
    | IDENTIFIER _ ':' _ IDENTIFIER
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
    | probable_array
    | number
    ;

number:
    NUMERIC
    | FW_NUMERIC
    ;
probable_array:
    IDENTIFIER 
    ;
array_subscript:
    LBRACKET _ expression _ RBRACKET
    ;

_:
    WHITESPACE _
    | NEWLINE _
    |
    ;
statement_delimiter:
    NEWLINE
    | ';'
    ;
%%

void yyerror(char *error) {
    fprintf(stderr, "Line %d: %s: '%s'\n", yylineno, error, yytext);
}

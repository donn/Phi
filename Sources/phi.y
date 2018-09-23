%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "Node.h"

    void yyerror(char *);
    int yylex();

    extern struct Node* head;
%}

%union {
    struct Node* node;
    char* text;
}

%token KEYWORD_MODULE
%token KEYWORD_INTERFACE
%token NUMERIC
%token IDENTIFIER
%token SEMICOLON

%type<node> integer
%type<text> NUMERIC IDENTIFIER
%type<semicolon> SEMICOLON
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
    ;

integer:
        NUMERIC {
            $$ = makeNode(NODE_TYPE_INT, &(struct Int){NODE_INT_DEFAULT_WIDTH,$1}, NULL, NULL);
        }
        ;

%%

void yyerror(char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

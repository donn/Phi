%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "Node.h"

    void yyerror(char *);
    int yylex();
%}

%union {
    struct Node* node;
    char* numeric;
}

%token NUMERIC_TOKEN
%token SEMICOLON

%type<node> integer
%type<numeric> NUMERIC_TOKEN
%type<semicolon> SEMICOLON
%%

description:
        description integer { traverse($2); }
        |
        ;

integer:
        NUMERIC_TOKEN {
            $$ = makeNode(NODE_TYPE_INT, &(struct Int){NODE_INT_DEFAULT_WIDTH,$1}, NULL, NULL);
        }
        ;

%%

void yyerror(char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

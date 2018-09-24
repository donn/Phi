%{
    #include "phi.y.h"
    #include <stdlib.h>

    void yyerror(char *);
%}

%%
[ \t]+ {}
[\n] { 
    yylineno += 1;
}

(module) {
    return KEYWORD_MODULE;
}
(interface) {
    return KEYWORD_INTERFACE;
}
(if) {
    return KEYWORD_IF;
}
(else) {
    return KEYWORD_ELSE;
}
(while) {
    return KEYWORD_WHILE;
}
(mux) {
    return KEYWORD_WHILE;
}
(when) {
    return KEYWORD_WHEN;
}
(Var) {
    return KEYWORD_SW_VAR;
}
(Wire) {
    return KEYWORD_WIRE;
}
(Register) {
    return KEYWORD_REGISTER;
}

[0-9]+ {
    yylval.text = yytext;
    return NUMERIC;
}
[0-9]+[bodx][0-9]+ {
    yylval.text = yytext;
    return FW_NUMERIC;
}
[_A-Za-z][_A-Za-z0-9]+ {
    yylval.text = yytext;
    return IDENTIFIER;
}

[\{] {
    return LBRACE;
}
[\}] {
    return RBRACE;
}
[\[] {
    return LBRACKET;
}
[\]] {
    return RBRACKET;
}
[\(] {
    return LPAREN;
}
[\)] {
    return RPAREN;
}

(>>>) {
    return OP_SRL;
}
(>>) {
    return OP_SRA;
}

(==) {
    return OP_EQ;
} 
(!=) {
    return OP_NEQ;
}
(>=) {
    return OP_GTE;
}
(<=) {
    return OP_LTE;
}
(\|\|) {
    return OP_LOGIC_OR;
}
(&&) {
    return OP_LOGIC_AND;
}

(&\+) {
    return OP_UNSIGNED_ADD;
}
(&\-) {
    return OP_UNSIGNED_SUB;
}

[!~+\-*<>/%\|&^\?:;,=] {
    return yytext[0];
}


%%

int yywrap(void) {
    return 1;
}
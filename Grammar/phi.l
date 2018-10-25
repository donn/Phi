%{
    #include "phi.y.h"
    #include <stdlib.h>
    void yyerror(char *);
    int cCommentNesting = 0;
%}


%x C_COMMENT
%%
(\/\*) { cCommentNesting += 1 ; BEGIN(C_COMMENT); }
<C_COMMENT>(\/\*) { cCommentNesting += 1 ; }
<C_COMMENT>(\*\/) { cCommentNesting -= 1 ; if (!cCommentNesting) { BEGIN(INITIAL); } }
<C_COMMENT>\n   { yylineno += 1; } 
<C_COMMENT>. { }

(\/\/.*) { 
    // C99/C++-style comments
}

[ \t]+ {
    // Ignore whitespace
}
\n { 
    yylineno += 1;
}
(module) {
    return KEYWORD_MODULE;
}
(interface) {
    return KEYWORD_INTERFACE;
}
(namespace) {
    return KEYWORD_NAMESPACE;
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
(for) {
    return KEYWORD_FOR;
}
(in) {
    return KEYWORD_IN;
}
(switch) {
    return KEYWORD_SWITCH;
}
(mux) {
    return KEYWORD_MUX;
}
(case) {
    return KEYWORD_CASE;
}
(default) {
    return KEYWORD_DEFAULT;
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
(async) {
    return KEYWORD_ASYNC;
}
(sync) {
    return KEYWORD_SYNC;
}
(Input) {
    return KEYWORD_INPUT;
}
(Output) {
    return KEYWORD_OUTPUT;
}
(posedge) {
    return KEYWORD_POSEDGE;
}
(negedge) {
    return KEYWORD_NEGEDGE;
}

(@[_A-Za-z][_A-Za-z0-9]*) {
    return ANNOTATION;
}
[_A-Za-z][_A-Za-z0-9]* {
    yylval.text = yytext;
    return IDENTIFIER;
}
[0-9]+[bodx][0-9zx]+ {
    yylval.text = yytext;
    return FW_NUMERIC;
}
[0-9]+ {
    yylval.text = yytext;
    return NUMERIC;
}

(\.\.) {
    return OP_RANGE;
}

(>>>) {
    return OP_SRL;
}
(>>) {
    return OP_SRA;
}
(<<) {
    return OP_SLL;
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

(&\+) {
    return OP_UNSIGNED_ADD;
}
(&\-) {
    return OP_UNSIGNED_SUB;
}
(&<) {
    return OP_UNSIGNED_LT;
}
(&<=) {
    return OP_UNSIGNED_LTE;
}
(&>) {
    return OP_UNSIGNED_GT;
}
(&>=) {
    return OP_UNSIGNED_LTE;
}

(\[\[) {
    return LEFT_CAT;
}
(\]\]) {
    return RIGHT_CAT;
}

[@\$!~+\-*<>/%\|&^\?:;,=\.\{\}\[\]\(\)] {
    return yytext[0];
}

%%

int yywrap(void) {
    return 1;
}
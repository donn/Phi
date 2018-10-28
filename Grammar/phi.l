%option noyywrap
%{
    #include <cstdlib>

    #include "Context.h"
    #include "phi.yy.hh"

    #define YY_DECL int yylex(Phi::Parser::semantic_type* yylval, \
    Phi::Parser::location_type* yylloc, Phi::Context* context)

    # define YY_USER_ACTION  do { yylloc->columns(yyleng); } while (0);
    
    int cCommentNesting = 0;

    using token = Phi::Parser::token;
%}


%x C_COMMENT
%%
%{
    yylloc->step();
%}
(\/\*) { cCommentNesting += 1 ; BEGIN(C_COMMENT); }
<C_COMMENT>(\/\*) { cCommentNesting += 1 ; }
<C_COMMENT>(\*\/) { cCommentNesting -= 1 ; if (!cCommentNesting) { BEGIN(INITIAL); } }
<C_COMMENT>\n   { yylloc->lines(1); } 
<C_COMMENT>. { }

(\/\/.*) { 
    // C99/C++-style comments
}

[ \t]+ {
    // Ignore whitespace
}
\n { 
    yylloc->lines(1);
}
(module) {
    return token::KEYWORD_MODULE;
}
(interface) {
    return token::KEYWORD_INTERFACE;
}
(namespace) {
    return token::KEYWORD_NAMESPACE;
}
(if) {
    return token::KEYWORD_IF;
}
(else) {
    return token::KEYWORD_ELSE;
}
(while) {
    return token::KEYWORD_WHILE;
}
(for) {
    return token::KEYWORD_FOR;
}
(in) {
    return token::KEYWORD_IN;
}
(switch) {
    return token::KEYWORD_SWITCH;
}
(mux) {
    return token::KEYWORD_MUX;
}
(case) {
    return token::KEYWORD_CASE;
}
(default) {
    return token::KEYWORD_DEFAULT;
}
(Var) {
    return token::KEYWORD_SW_VAR;
}
(Wire) {
    return token::KEYWORD_WIRE;
}
(Register) {
    return token::KEYWORD_REGISTER;
}
(async) {
    return token::KEYWORD_ASYNC;
}
(sync) {
    return token::KEYWORD_SYNC;
}
(Input) {
    return token::KEYWORD_INPUT;
}
(Output) {
    return token::KEYWORD_OUTPUT;
}
(posedge) {
    return token::KEYWORD_POSEDGE;
}
(negedge) {
    return token::KEYWORD_NEGEDGE;
}

(@[_A-Za-z][_A-Za-z0-9]*) {
    return token::ANNOTATION;
}
[_A-Za-z][_A-Za-z0-9]* {
    yylval->text = yytext;
    return token::IDENTIFIER;
}
[0-9]+[bodx][0-9zx]+ {
    yylval->text = yytext;
    return token::FW_NUMERIC;
}
[0-9]+ {
    yylval->text = yytext;
    return token::NUMERIC;
}

(\.\.) {
    return token::OP_RANGE;
}

(>>>) {
    return token::OP_SRL;
}
(>>) {
    return token::OP_SRA;
}
(<<) {
    return token::OP_SLL;
}

(==) {
    return token::OP_EQ;
} 
(!=) {
    return token::OP_NEQ;
}
(>=) {
    return token::OP_GTE;
}
(<=) {
    return token::OP_LTE;
}

(&\+) {
    return token::OP_UNSIGNED_ADD;
}
(&\-) {
    return token::OP_UNSIGNED_SUB;
}
(&<) {
    return token::OP_UNSIGNED_LT;
}
(&<=) {
    return token::OP_UNSIGNED_LTE;
}
(&>) {
    return token::OP_UNSIGNED_GT;
}
(&>=) {
    return token::OP_UNSIGNED_LTE;
}

(\[\[) {
    return token::LEFT_CAT;
}
(\]\]) {
    return token::RIGHT_CAT;
}

[@\$!~+\-*<>/%\|&^\?:;,=\.\{\}\[\]\(\)] {
    return yytext[0];
}

%%
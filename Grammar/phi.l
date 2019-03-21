%option noyywrap
%{
    #include <cstdlib>
    #include <regex>

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

[ \t\r]+ {
    // Ignore whitespace
}
\n { 
    yylloc->lines(1);
}
(`[_A-Za-z][_A-Za-z0-9]*`) {
    yylval->text = strdup(yytext + 1);
    yylval->text[yyleng - 2] = 0;
    return token::IDENTIFIER;
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
(Latch) {
    return token::KEYWORD_LATCH;
}
(comb) {
    return token::KEYWORD_COMB;
}
(Input) {
    return token::KEYWORD_INPUT;
}
(Output) {
    return token::KEYWORD_OUTPUT;
}

\@[_A-Za-z][_A-Za-z0-9]* {
    yylval->text = strdup(yytext);
    return token::ANNOTATION;
}
[_A-Za-z][_A-Za-z0-9]* {
    yylval->text = strdup(yytext);
    return token::IDENTIFIER;
}
[0-9]+[bodxh][A-Fa-f0-9z]+ {
    yylval->text = strdup(yytext);
    auto temp = std::string(yylval->text);
    auto pureRegex = std::regex("^([0-9]+)([bodxh])([A-F0-9]+)$");
    auto match = std::smatch();
    if (std::regex_match(temp, match, pureRegex)) {
        return token::FW_NUMERIC;
    } else {
        return token::FW_SPECIAL;
    }
}
[0-9]+ {
    yylval->text = strdup(yytext);
    return token::NUMERIC;
}

\"(\\.|[^\\"])*\" {
    yylval->text = strdup(yytext);
    return token::STRING;
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
    return token::LEFT_REPEAT_CAT;
}

[\$!~+\-*<>/%\|&^\?:;,=\.\{\}\[\]\(\)] {
    return yytext[0];
}

%%
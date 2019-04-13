%option flex
%option unicode 
%option namespace=Phi 
%option lexer=Lexer
%option bison-cc
%option bison-locations
%option bison-cc-namespace=Phi
%option bison-cc-parser=Parser 

%top {
    #include "phi.yy.hh"
%}
%{
    #include <cstdlib>
    #include <regex>

    #include "Context.h"
    
    int cCommentNesting = 0;
    using token = Phi::Parser::token;
%}


%x C_COMMENT

ender 0-9\u{0300}-\u{036F}\u{1DC0}-\u{1DFF}\u{20D0}-\u{20FF}\u{FE20}-\u{FE2F}
starter _A-Za-z\u{00A8}\u{00AA}\u{00AD}\u{00AF}\u{00B2}-\u{00B5}\u{00B7}-\u{00BA}\u{00BC}-\u{00BE}\u{00C0}-\u{00D6}\u{00D8}-\u{00F6}\u{00F8}-\u{00FF}\u{0100}-\u{02FF}\u{0370}-\u{167F}\u{1681}-\u{180D}\u{180F}-\u{1DBF}\u{1E00}-\u{1FFF}\u{200B}-\u{200D}\u{202A}-\u{202E}\u{203F}-\u{2040}\u{2054}\u{2060}-\u{206F}\u{2070}-\u{20CF}\u{2100}-\u{218F}\u{2460}-\u{24FF}\u{2776}-\u{2793}\u{2C00}-\u{2DFF}\u{2E80}-\u{2FFF}\u{3004}-\u{3007}\u{3021}-\u{302F}\u{3031}-\u{303F}\u{3040}-\u{D7FF}\u{F900}-\u{FD3D}\u{FD40}-\u{FDCF}\u{FDF0}-\u{FE1F}\u{FE30}-\u{FE44}\u{FE47}-\u{FFFD}\u{10000}-\u{1FFFD}\u{20000}-\u{2FFFD}\u{30000}-\u{3FFFD}\u{40000}-\u{4FFFD}\u{50000}-\u{5FFFD}\u{60000}-\u{6FFFD}\u{70000}-\u{7FFFD}\u{80000}-\u{8FFFD}\u{90000}-\u{9FFFD}\u{A0000}-\u{AFFFD}\u{B0000}-\u{BFFFD}\u{C0000}-\u{CFFFD}\u{D0000}-\u{DFFFD}\u{E0000}-\u{EFFFD}

%%

(\/\*) { cCommentNesting += 1 ; BEGIN(C_COMMENT); }
<C_COMMENT>(\/\*) { cCommentNesting += 1 ; }
<C_COMMENT>(\*\/) { cCommentNesting -= 1 ; if (!cCommentNesting) { BEGIN(INITIAL); } }
<C_COMMENT>\n   { /* yylloc->lines(1); */ } 
<C_COMMENT>. { }

(\/\/.*) { 
    // C99/C++-style comments
}

[ \t\r]+ {
    // Ignore whitespace
}
\n { 
    //yylloc->lines(1);
}
(`[_A-Za-z\u{00A8}\u{00AA}\u{00AD}\u{00AF}\u{00B2}-\u{00B5}\u{00B7}-\u{00BA}\u{00BC}-\u{00BE}\u{00C0}-\u{00D6}\u{00D8}-\u{00F6}\u{00F8}-\u{00FF}\u{0100}-\u{02FF}\u{0370}-\u{167F}\u{1681}-\u{180D}\u{180F}-\u{1DBF}\u{1E00}-\u{1FFF}\u{200B}-\u{200D}\u{202A}-\u{202E}\u{203F}-\u{2040}\u{2054}\u{2060}-\u{206F}\u{2070}-\u{20CF}\u{2100}-\u{218F}\u{2460}-\u{24FF}\u{2776}-\u{2793}\u{2C00}-\u{2DFF}\u{2E80}-\u{2FFF}\u{3004}-\u{3007}\u{3021}-\u{302F}\u{3031}-\u{303F}\u{3040}-\u{D7FF}\u{F900}-\u{FD3D}\u{FD40}-\u{FDCF}\u{FDF0}-\u{FE1F}\u{FE30}-\u{FE44}\u{FE47}-\u{FFFD}\u{10000}-\u{1FFFD}\u{20000}-\u{2FFFD}\u{30000}-\u{3FFFD}\u{40000}-\u{4FFFD}\u{50000}-\u{5FFFD}\u{60000}-\u{6FFFD}\u{70000}-\u{7FFFD}\u{80000}-\u{8FFFD}\u{90000}-\u{9FFFD}\u{A0000}-\u{AFFFD}\u{B0000}-\u{BFFFD}\u{C0000}-\u{CFFFD}\u{D0000}-\u{DFFFD}\u{E0000}-\u{EFFFD}][_A-Za-z\u{00A8}\u{00AA}\u{00AD}\u{00AF}\u{00B2}-\u{00B5}\u{00B7}-\u{00BA}\u{00BC}-\u{00BE}\u{00C0}-\u{00D6}\u{00D8}-\u{00F6}\u{00F8}-\u{00FF}\u{0100}-\u{02FF}\u{0370}-\u{167F}\u{1681}-\u{180D}\u{180F}-\u{1DBF}\u{1E00}-\u{1FFF}\u{200B}-\u{200D}\u{202A}-\u{202E}\u{203F}-\u{2040}\u{2054}\u{2060}-\u{206F}\u{2070}-\u{20CF}\u{2100}-\u{218F}\u{2460}-\u{24FF}\u{2776}-\u{2793}\u{2C00}-\u{2DFF}\u{2E80}-\u{2FFF}\u{3004}-\u{3007}\u{3021}-\u{302F}\u{3031}-\u{303F}\u{3040}-\u{D7FF}\u{F900}-\u{FD3D}\u{FD40}-\u{FDCF}\u{FDF0}-\u{FE1F}\u{FE30}-\u{FE44}\u{FE47}-\u{FFFD}\u{10000}-\u{1FFFD}\u{20000}-\u{2FFFD}\u{30000}-\u{3FFFD}\u{40000}-\u{4FFFD}\u{50000}-\u{5FFFD}\u{60000}-\u{6FFFD}\u{70000}-\u{7FFFD}\u{80000}-\u{8FFFD}\u{90000}-\u{9FFFD}\u{A0000}-\u{AFFFD}\u{B0000}-\u{BFFFD}\u{C0000}-\u{CFFFD}\u{D0000}-\u{DFFFD}\u{E0000}-\u{EFFFD}0-9\u{0300}-\u{036F}\u{1DC0}-\u{1DFF}\u{20D0}-\u{20FF}\u{FE20}-\u{FE2F}]*`) {
    yylval.text = strdup(yytext + 1);
    yylval.text[yyleng - 2] = 0;
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
    yylval.text = strdup(yytext);
    return token::ANNOTATION;
}
[_A-Za-z\u{00A8}\u{00AA}\u{00AD}\u{00AF}\u{00B2}-\u{00B5}\u{00B7}-\u{00BA}\u{00BC}-\u{00BE}\u{00C0}-\u{00D6}\u{00D8}-\u{00F6}\u{00F8}-\u{00FF}\u{0100}-\u{02FF}\u{0370}-\u{167F}\u{1681}-\u{180D}\u{180F}-\u{1DBF}\u{1E00}-\u{1FFF}\u{200B}-\u{200D}\u{202A}-\u{202E}\u{203F}-\u{2040}\u{2054}\u{2060}-\u{206F}\u{2070}-\u{20CF}\u{2100}-\u{218F}\u{2460}-\u{24FF}\u{2776}-\u{2793}\u{2C00}-\u{2DFF}\u{2E80}-\u{2FFF}\u{3004}-\u{3007}\u{3021}-\u{302F}\u{3031}-\u{303F}\u{3040}-\u{D7FF}\u{F900}-\u{FD3D}\u{FD40}-\u{FDCF}\u{FDF0}-\u{FE1F}\u{FE30}-\u{FE44}\u{FE47}-\u{FFFD}\u{10000}-\u{1FFFD}\u{20000}-\u{2FFFD}\u{30000}-\u{3FFFD}\u{40000}-\u{4FFFD}\u{50000}-\u{5FFFD}\u{60000}-\u{6FFFD}\u{70000}-\u{7FFFD}\u{80000}-\u{8FFFD}\u{90000}-\u{9FFFD}\u{A0000}-\u{AFFFD}\u{B0000}-\u{BFFFD}\u{C0000}-\u{CFFFD}\u{D0000}-\u{DFFFD}\u{E0000}-\u{EFFFD}][_A-Za-z\u{00A8}\u{00AA}\u{00AD}\u{00AF}\u{00B2}-\u{00B5}\u{00B7}-\u{00BA}\u{00BC}-\u{00BE}\u{00C0}-\u{00D6}\u{00D8}-\u{00F6}\u{00F8}-\u{00FF}\u{0100}-\u{02FF}\u{0370}-\u{167F}\u{1681}-\u{180D}\u{180F}-\u{1DBF}\u{1E00}-\u{1FFF}\u{200B}-\u{200D}\u{202A}-\u{202E}\u{203F}-\u{2040}\u{2054}\u{2060}-\u{206F}\u{2070}-\u{20CF}\u{2100}-\u{218F}\u{2460}-\u{24FF}\u{2776}-\u{2793}\u{2C00}-\u{2DFF}\u{2E80}-\u{2FFF}\u{3004}-\u{3007}\u{3021}-\u{302F}\u{3031}-\u{303F}\u{3040}-\u{D7FF}\u{F900}-\u{FD3D}\u{FD40}-\u{FDCF}\u{FDF0}-\u{FE1F}\u{FE30}-\u{FE44}\u{FE47}-\u{FFFD}\u{10000}-\u{1FFFD}\u{20000}-\u{2FFFD}\u{30000}-\u{3FFFD}\u{40000}-\u{4FFFD}\u{50000}-\u{5FFFD}\u{60000}-\u{6FFFD}\u{70000}-\u{7FFFD}\u{80000}-\u{8FFFD}\u{90000}-\u{9FFFD}\u{A0000}-\u{AFFFD}\u{B0000}-\u{BFFFD}\u{C0000}-\u{CFFFD}\u{D0000}-\u{DFFFD}\u{E0000}-\u{EFFFD}0-9\u{0300}-\u{036F}\u{1DC0}-\u{1DFF}\u{20D0}-\u{20FF}\u{FE20}-\u{FE2F}]* {
    // This feels dirty
    yylval.text = strdup(yytext);
    return token::IDENTIFIER;
}
[0-9]+[bodxh][A-Fa-f0-9?]+ {
    yylval.text = strdup(yytext);
    auto temp = std::string(yylval.text);
    auto pureRegex = std::regex("^([0-9]+)([bodxh])([A-F0-9]+)$");
    auto match = std::smatch();
    if (std::regex_match(temp, match, pureRegex)) {
        return token::FW_NUMERIC;
    } else {
        return token::FW_SPECIAL;
    }
}
[0-9]+ {
    yylval.text = strdup(yytext);
    return token::NUMERIC;
}

\"(\\.|[^\\"])*\" {
    yylval.text = strdup(yytext);
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

(\{\{) {
    return token::LEFT_REPEAT_CAT;
}

[\$!~+\-*<>/%\|&^\?:;,=\.\{\}\[\]\(\)] {
    return yytext[0];
}

%%

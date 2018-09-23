%{
    #include "phi.y.h"
    #include <stdlib.h>
    void yyerror(char *);
%}

%%
[ \t\n]+ {}
(module) {
    return KEYWORD_MODULE;
}
(interface) {
    return KEYWORD_INTERFACE;
}
[_A-Za-z][_A-Za-z0-9]+ {
    yylval.text = yytext;
    return IDENTIFIER;
}
[0-9]+ {
    yylval.text = yytext;
    return NUMERIC;
}
[;] {
    return SEMICOLON;
}

%%

int yywrap(void) {
    return 1;
}
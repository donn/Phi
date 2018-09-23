%{
    #include "phi.y.h"
    #include <stdlib.h>
    void yyerror(char *);
%}

%%
[ \t]+ {}
[0-9]+ {
    yylval.numeric = yytext;
    return NUMERIC_TOKEN;
}
[;] {
    return SEMICOLON;
}

%%

int yywrap(void) {
    return 1;
}
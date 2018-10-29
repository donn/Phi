%language "C++"
%defines
%locations

%define parser_class_name {Parser}
%define api.namespace {Phi}

%{
    // C STD
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>

    // CPP STL
    #include <sstream>

    // Project Headers
    #include "Context.h"
    #include "Node.h"
    #include "Utils.h"

    void yyerror(char *);
    int yylex();
    int yydebug=1;

    extern int yylineno;
    extern char* yytext;

    using namespace Phi::Node;

    #define catchIntoContext catch (const char* error) { context->errorList.push_back({yylhs.location, std::string(error)}); };
    #define tcc(stmt) try { stmt } catchIntoContext
    #define cst std::stringstream stream
    #define ε strdup("")
    #define err_placeholder strdup("/* PHI TRANSLATOR: ERROR */")
    #define TODO strdup("")
    #define dup strdup(stream.str().c_str())
%}

%parse-param { Phi::Context* context }
%lex-param   { Phi::Context* context }

%union {
    char* text;
    void* node;
}

%token ASSIGNMENT

%token WHITESPACE
%token NEWLINE

%token KEYWORD_MODULE
%token KEYWORD_INTERFACE
%token KEYWORD_NAMESPACE
%token KEYWORD_IF
%token KEYWORD_ELSE
%token KEYWORD_WHILE
%token KEYWORD_FOR
%token KEYWORD_IN
%token KEYWORD_WHEN
%token KEYWORD_SWITCH
%token KEYWORD_MUX
%token KEYWORD_CASE
%token KEYWORD_DEFAULT
%token KEYWORD_SW_VAR
%token KEYWORD_WIRE
%token KEYWORD_REGISTER
%token KEYWORD_SYNC
%token KEYWORD_ASYNC
%token KEYWORD_INPUT
%token KEYWORD_OUTPUT
%token KEYWORD_POSEDGE
%token KEYWORD_NEGEDGE

%token NUMERIC
%token FW_NUMERIC
%token IDENTIFIER

%token OP_RANGE
%token OP_SRL
%token OP_SRA
%token OP_SLL
%token OP_EQ
%token OP_NEQ
%token OP_GTE
%token OP_LTE
%token OP_UNSIGNED_ADD
%token OP_UNSIGNED_SUB
%token OP_UNSIGNED_LT
%token OP_UNSIGNED_LTE
%token OP_UNSIGNED_GT
%token OP_UNSIGNED_GTE

%token LEFT_CAT
%token RIGHT_CAT

%right '=' ':' 

%right '?'
%left OP_RANGE
%left OP_LOGIC_OR
%left OP_LOGIC_AND
%left OP_EQ OP_NEQ OP_GTE OP_LTE '<' '>' OP_UNSIGNED_LT OP_UNSIGNED_LTE OP_UNSIGNED_GT OP_UNSIGNED_GTE
%left OP_UNSIGNED_ADD OP_UNSIGNED_SUB '+' '-' '|' '&' '^'
%left '*' '/' '%'
%left OP_SRL OP_SRA OP_SLL
%left '~' KEYWORD_POSEDGE KEYWORD_NEGEDGE UNARY
%left '.'
%right '['

%type<text> NUMERIC FW_NUMERIC IDENTIFIER

// Silly conversion
%type<text> description declaration port_declaration_list populated_port_declaration_list port_declaration port_polarity template_declaration template_declaration_list optional_template_assignment inheritance inheritance_list statement block_based if else switch_block labeled_statement_list block statement_list subdeclaration dynamic_width optional_array_subscript optional_ports declaration_list optional_assignment probable_template template_list ports port_list nondeclarative_statement expression range mux_block labeled_expression_list concatenation concatenatable procedural_call procedural_call_list number

%{
    extern int yylex(Phi::Parser::semantic_type* yylval,
                     Phi::Parser::location_type* yylloc,
                     Phi::Context* context
    );
%}

%initial-action {
    @$.begin.filename = @$.end.filename = &context->files.back();
}

%%

/* Top Level */

description:
    { $$ = ε; }
    | declaration description  {
        cst; stream << $1 << std::endl << std::endl << $2;
        $$ = dup;
        context->top = $$;
    }
    | KEYWORD_NAMESPACE IDENTIFIER '{' description '}' {
        $$ = TODO;
    }
    ;

declaration:
    KEYWORD_MODULE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance block  {
        cst; stream << "module " << $2 << TODO << '(' << std::endl << $5 << ')' << TODO << $8;
        auto string = stream.str();
        auto index = string.find("begin");
        string.replace(index, 5, ";");
        index = string.rfind("end");
        string.replace(index, 3, "endmodule");
        $$ = strdup(string.c_str());
    }
    | KEYWORD_MODULE error '}' {
        $$ = err_placeholder;
    }
    | KEYWORD_INTERFACE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance ';' {
        $$ = TODO;
    }
    | KEYWORD_INTERFACE error ';' {
        $$ = err_placeholder;
    }
    ;

/* Ports */
port_declaration_list:
    { $$ = ε; }
    | populated_port_declaration_list {
        $$ = $1;
    }
    ;
populated_port_declaration_list:
    IDENTIFIER ':' port_declaration ',' populated_port_declaration_list {
        cst; stream << "    " << $3 << " " << $1 << ',' << std::endl << $5;
        $$ = dup;
    }
    | IDENTIFIER ':' port_declaration {
        cst; stream << "    " << $3 << $1 << std::endl;
        $$ = dup;
    }
    ;
port_declaration:
    port_polarity {
        $$ = $1;
    }
    | '@' IDENTIFIER port_polarity {
        $$ = $3;
    }
    ;
port_polarity:
    KEYWORD_INPUT optional_array_subscript {
        cst; stream << "input " << $2;
        $$ = dup;
    }
    | KEYWORD_OUTPUT optional_array_subscript {
        cst; stream << "output " << $2;
        $$ = dup;
    }
    ;

/* Templating */
template_declaration:
    { $$ = ε; }
    | '<' template_declaration_list '>' {
        $$ = $2;
    }
    ;
template_declaration_list:
    IDENTIFIER optional_template_assignment template_declaration_list {
        cst; stream << "parameter " << $1 << $2 << ";" << std::endl << $3;
        $$ = dup;
    }
    | IDENTIFIER optional_template_assignment {
        cst; stream << "parameter " << $1 << $2 << ";";
        $$ = dup;
    }
    ;
optional_template_assignment:
    { $$ = ε; }
    | '=' '(' expression ')' {
        cst; stream << '=' << $3;
        $$ = dup;
    }
    ;

/* Inheritance */
inheritance:
    { $$ = ε; }
    | ':' inheritance_list {
        $$ = TODO;
    }
    ;
inheritance_list:
    expression ',' inheritance_list {
        cst; stream << $1 << ',' << $3;
        $$ = dup;
    }
    | expression {
        $$ = $1;
    }
    ;

/* Statements */

statement:
    subdeclaration ';' {
        cst; stream << $1 << ";";
        $$ = dup;
    }
    | nondeclarative_statement ';' {
        cst; stream << $1 << ";";
        $$ = dup;
    }
    | block_based {
        $$ = $1;
    }
    ;

/* Blocks */
block_based:
    if {
        $$ = $1;
    }
    | KEYWORD_FOR IDENTIFIER KEYWORD_IN range block {
        $$ = TODO;
    }
    | KEYWORD_NAMESPACE IDENTIFIER block {
        $$ = TODO;
    }
    | KEYWORD_SWITCH expression switch_block {
        cst; stream << "switch (" << $2 << ") " << $3;
        $$ = dup;
    }
    | '@' expression block {
        cst; stream << "always @ (" << $2 << ") " << $3;
        $$ = dup;
    }
    | KEYWORD_ASYNC block {
        cst; stream << "always @ (*) " << $2;
        $$ = dup;
    }
    | error '}' {
        $$ = err_placeholder;
    }
    ;
if:
    KEYWORD_IF expression block else {
        cst; stream << "if (" << $2 << ")" << " " << $3 << " " << $4;
        $$ = dup;
    }
    ;

else:
    { $$ = ε; }
    | KEYWORD_ELSE if {
        cst; stream << "else " << $2;
        $$ = dup;
    }
    | KEYWORD_ELSE block {
        cst; stream << "else " << $2;
        $$ = dup;
    }
    ;

switch_block:
    '{' labeled_statement_list '}' {
        cst; stream << "begin" << std::endl << $2 << std::endl << "end";
        $$ = dup;
    }
    ;
labeled_statement_list:
    { $$ = ε; }
    | KEYWORD_CASE expression ':' statement_list labeled_statement_list {
        cst; stream << $2 << ": " << std::endl << $4 << std::endl << $5;
        $$ = dup;
    }
    | KEYWORD_DEFAULT ':' statement_list {
        cst; stream << "default: " << std::endl << $3;
        $$ = dup;
    }
    ;

block:
    '{' statement_list '}' {
        cst; stream << "begin" << std::endl << $2 << std::endl << "end";
        $$ = dup;
    }
    ;

statement_list:
    { $$ = ε; }
    | statement_list statement {
        cst; stream << $1 << std::endl << $2;
        $$ = dup;
    }
    ;

/* Subdeclarations */

subdeclaration:
    dynamic_width optional_array_subscript declaration_list {
        cst; stream << $1 << " " << $2 << " " << $3;
        $$ = dup;
    }
    | probable_template IDENTIFIER optional_array_subscript optional_ports {
        cst; stream << $1 << $2 << " /* PHI: Unsupported array of components. */" << $4;
        $$ = dup;
    }
    ;

dynamic_width:
    KEYWORD_SW_VAR {
        $$ = strdup("var");
    }
    | KEYWORD_WIRE {
        $$ = strdup("wire");
    }
    | KEYWORD_REGISTER {
        $$ = strdup("reg");
    }
    ;
optional_array_subscript:
    { $$ = ε; }
    | '[' expression ']' {
        cst; stream << '[' << $2 << ']';
        $$ = dup;
    }
    ;
optional_ports:
    { $$ = ε; }
    | ports {
        cst; stream << $1;
        $$ = dup;
    }
    ;
declaration_list:
    IDENTIFIER optional_array_subscript optional_assignment ',' declaration_list {
        cst; stream << $1 << $2 << $3 << ',';
        $$ = dup;
    }
    | IDENTIFIER optional_array_subscript optional_assignment {
        cst; stream << $1 << $2 << $3;
        $$ = dup;
    }
    ;
optional_assignment:
    { $$ = ε; }
    | '=' expression {
        cst; stream << '=' << $2;
        $$ = dup;
    }
    ;

probable_template:
    expression {
        $$ = $1;
    }
    | expression '<' template_list '>' {
        cst; stream << $1 << "#(" << $3 << ")";
        $$ = dup;
    }
    ;
template_list:
    { $$ = ε; }
    | IDENTIFIER ':' '(' expression ')' ',' template_list {
        cst; stream << '.' << $1 << '(' << $4 << ')' << ',' << $7;
        $$ = dup;
    }
    | IDENTIFIER ':' '(' expression ')' {
        cst; stream << '.' << $1 << '(' << $4 << ')';
        $$ = dup;
    }
    ;
ports:
    '(' ')' {
        $$ = strdup("()");
    }
    | '(' port_list ')' {
        cst; stream << '(' << $2 << ')';
        $$ = dup;
    }
    ;
port_list:
    IDENTIFIER ':' expression ',' port_list {
        cst; stream << '.' << $1 << '(' << $3 << ')' << ',' << $5;
        $$ = dup;
    }
    | IDENTIFIER ':' expression {
        cst; stream << '.' << $1 << '(' << $3 << ')';
        $$ = dup;
    }
    ;

/* Nondeclarative Statements */
    
nondeclarative_statement:
    expression '=' expression {
        cst;
        auto assignee = std::string($1);
        auto op = "=";
        size_t pos = assignee.rfind(".next");
        if (pos != std::string::npos) {
            assignee.replace(pos, 5, "");
            op = "<=";
        }
        stream << assignee << ' ' << op << ' ' << $3;
        $$ = dup;
    }
    | expression ports {
        cst; stream << $1 << $2;
        $$ = dup;
    }
    ;

/* Expressions */

expression:
    expression '?' expression ':' expression 
    | expression OP_EQ expression {
        cst; stream << $1 << "==" << $3;
        $$ = dup;
    }
    | expression OP_NEQ expression {
        cst; stream << $1 << "!=" << $3;
        $$ = dup;
    }
    | expression '>' expression {
        cst; stream << $1 << '>' << $3;
        $$ = dup;
    }
    | expression '<' expression {
        cst; stream << $1 << '<' << $3;
        $$ = dup;
    }
    | expression OP_GTE expression {
        cst; stream << $1 << ">=" << $3;
        $$ = dup;
    }
    | expression OP_LTE expression {
        cst; stream << $1 << "<=" << $3;
        $$ = dup;
    }
    | expression OP_UNSIGNED_LT expression {
        $$ = TODO;
    }
    | expression OP_UNSIGNED_GT  expression {
        $$ = TODO;
    } 
    | expression OP_UNSIGNED_LTE expression {
        $$ = TODO;
    }
    | expression OP_UNSIGNED_GTE expression {
        $$ = TODO;
    }
    | expression '+' expression {
        cst; stream << $1 << '+' << $3;
        $$ = dup;
    }
    | expression '-' expression {
        cst; stream << $1 << '-' << $3;
        $$ = dup;
    }
    | expression OP_UNSIGNED_ADD expression{
        $$ = TODO;
    }
    | expression OP_UNSIGNED_SUB expression {
        $$ = TODO;
    }
    | expression '|' expression {
        cst; stream << $1 << '|' << $3;
        $$ = dup;
    }
    | expression '&' expression {
        cst; stream << $1 << '&' << $3;
        $$ = dup;
    }
    | expression '^' expression {
        cst; stream << $1 << '^' << $3;
        $$ = dup;
    }
    | expression '*' expression {
        cst; stream << $1 << '*' << $3;
        $$ = dup;
    }
    | expression '/' expression {
        cst; stream << $1 << '/' << $3;
        $$ = dup;
    }
    | expression '%' expression {
        cst; stream << $1 << '%' << $3;
        $$ = dup;
    }
    | expression OP_SLL expression {
        cst; stream << $1 << "<<" << $3;
        $$ = dup;
    }
    | expression OP_SRL expression {
        cst; stream << $1 << ">>" << $3;
        $$ = dup;
    }
    | expression OP_SRA expression {
        cst; stream << "$signed(" << $1 << ") >> $signed(" << $3 << ")";
        $$ = dup;
    }
    | range {
        $$ = $1;
    }
    | '-' expression %prec UNARY {
        cst; stream << '-' << $2;
        $$ = dup;
    }
    | '&' expression %prec UNARY {
        $$ = TODO;
    }
    | '|' expression %prec UNARY {
        $$ = TODO;
    }
    | '~' expression %prec UNARY {
        cst; stream << '~' << $2;
        $$ = dup;
    }
    | KEYWORD_POSEDGE expression {
        cst; stream << "posedge " << $2;
        $$ = dup;
    }
    | KEYWORD_NEGEDGE expression {
        cst; stream << "negedge " << $2;
        $$ = dup;
    }
    | expression '.' expression {
        cst; stream << $1 << '.' << $3; // TODO PROPERLY
        $$ = dup;
    }
    | expression '[' expression ']' {
        cst; stream << $1 << '[' << $3 << ']';
        $$ = dup;
    }
    | '[' concatenation ']' {
        cst; stream << '{' << $2 << '}';
        $$ = dup;
    }
    | '(' expression ')' {
        cst; stream << '(' << $2 << ')';
        $$ = dup;
    }
    | '$' IDENTIFIER '(' procedural_call ')' {
        $$ = TODO;
    }
    | KEYWORD_MUX expression mux_block {
        $$ = TODO;
    }
    | IDENTIFIER {
        $$ = strdup(yytext);
    }
    | number {
        $$ = $1;
    }
    ;

range:
    expression OP_RANGE expression {
        cst; stream << $1 << ':' << $3;
        $$ = dup;
    }
    ;

mux_block:
    '{' labeled_expression_list '}' {
        cst; stream << '{' << $2 << '}';
        $$ = dup;
    }
    ;
labeled_expression_list:
    { $$ = ε; }
    | KEYWORD_CASE expression ':' expression ';' labeled_expression_list {
        cst; stream << $2 << ": " << std::endl;
        stream << "__PHI__MUX__LAST = " << $4 << ';' << std::endl;
        stream << "break; " << std::endl;
        stream << $6;
        $$ = dup;
    }
    | KEYWORD_DEFAULT ':' expression ';' {
        cst; stream << "default: " << std::endl;
        stream << "__PHI__MUX__LAST = " << $3 << ';' << std::endl;
        $$ = dup;
    }
    ;

concatenation:
    concatenatable ',' concatenation {
        cst; stream << $1 << ", " << $3;
        $$ = dup;
    }
    | concatenatable {
        $$ = $1;
    }
    ;
concatenatable:
    expression {
        $$ = $1;
    }
    | expression LEFT_CAT expression RIGHT_CAT {
        cst; stream << '{' << $1 << '{' << $3 << '}' << '}';
        $$ = dup;
    }
    ;

procedural_call: {
        $$ = ε;
    }
    | procedural_call_list {
        $$ = $1;
    }
    ;
procedural_call_list:
    expression ',' procedural_call_list {
        cst; stream << $1 << ", " << $3;
        $$ = dup;
    }
    | expression {
        $$ = $1;
    }
    ;

number:
    NUMERIC { 
        cst; stream << "64'd" << $1;
        $$ = dup;
    }
    | FW_NUMERIC {
        cst; Number* number;
        tcc(number = new Number(yytext);)
        stream << int(number->width) << '\'';
        switch (number->radix) {
            case 2:
                stream << 'b';
                break;
            case 8:
                stream << 'o';
                break;
            case 16:
                stream << 'h';
                break;
            default:
                stream << 'd';
        }
        stream << number->literal;
        delete number;
        $$ = dup;
    }  
    ;

%%
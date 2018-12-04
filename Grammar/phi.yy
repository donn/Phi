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

    extern int yylineno;
    extern char* yytext;

    using namespace Phi::Node;

    #define catchIntoContext catch (const char* error) { context->errorList.push_back({yylhs.location, std::string(error)}); };
    #define ε nullptr
%}

%parse-param { Phi::Context* context }
%lex-param   { Phi::Context* context }

%union {
    char* text;
    void* node;
}

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
%token KEYWORD_ASYNC
%token KEYWORD_INPUT
%token KEYWORD_OUTPUT
%token KEYWORD_POSEDGE
%token KEYWORD_NEGEDGE

%token NUMERIC
%token FW_NUMERIC
%token ANNOTATION
%token IDENTIFIER
%token STRING

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

%token LEFT_REPEAT_CAT

%right '=' ':' 

%right '?'
%left OP_RANGE
%left OP_EQ OP_NEQ OP_GTE OP_LTE '<' '>' OP_UNSIGNED_LT OP_UNSIGNED_LTE OP_UNSIGNED_GT OP_UNSIGNED_GTE
%left OP_UNSIGNED_ADD OP_UNSIGNED_SUB '+' '-' '|' '&' '^'
%left '*' '/' '%'
%left OP_SRL OP_SRA OP_SLL
%left '~' KEYWORD_POSEDGE KEYWORD_NEGEDGE UNARY
%left '.'
%right '['

%type<text> NUMERIC FW_NUMERIC IDENTIFIER ANNOTATION STRING

// Silly conversion
%type<node> description declaration port_declaration_list populated_port_declaration_list port_declaration port_polarity template_declaration template_declaration_list optional_template_assignment inheritance inheritance_list statement block_based if else switch_block labeled_statement_list block statement_list subdeclaration dynamic_width optional_bus_declaration optional_array_declaration subscript optional_ports declaration_list optional_assignment probable_template template_list ports port_list nondeclarative_statement expression range mux_block labeled_expression_list concatenation concatenatable procedural_call procedural_call_list optional_annotation

%{
    extern int yylex(Phi::Parser::semantic_type* yylval,
                     Phi::Parser::location_type* yylloc,
                     Phi::Context* context
    );
%}

%initial-action {
#if YYDEBUG
    this->set_debug_level(context->trace);
    @$.begin.filename = @$.end.filename = &context->files.back();
#endif
}

%%

/* Top Level */

description:
    { $$ = ε; }
    | declaration description  {
        $$ = ε;
    }
    | KEYWORD_NAMESPACE IDENTIFIER '{' description '}' {
        $$ = ε;
    }
    ;

declaration:
    KEYWORD_MODULE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance block  {
        $$ = ε;
    }
    | KEYWORD_MODULE error '}' {
        $$ = ε;
    }
    | KEYWORD_INTERFACE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance ';' {
        $$ = ε;
    }
    | KEYWORD_INTERFACE error ';' {
        $$ = ε;
    }
    ;

/* Ports */
port_declaration_list:
    { $$ = ε; }
    | populated_port_declaration_list {
        $$ = ε;
    }
    ;
populated_port_declaration_list:
    IDENTIFIER ':' port_declaration ',' populated_port_declaration_list {
        $$ = ε;
    }
    | IDENTIFIER ':' port_declaration {
        $$ = ε;
    }
    ;
port_declaration:
    port_polarity {
        $$ = ε;
    }
    | ANNOTATION port_polarity {
        $$ = ε;
    }
    ;
port_polarity:
    KEYWORD_INPUT optional_bus_declaration {
        $$ = ε;
    }
    | KEYWORD_OUTPUT optional_bus_declaration {
        $$ = ε;
    }
    ;

/* Templating */
template_declaration:
    { $$ = ε; }
    | '<' template_declaration_list '>' {
        $$ = ε;
    }
    ;
template_declaration_list:
    IDENTIFIER optional_template_assignment template_declaration_list {
        $$ = ε;
    }
    | IDENTIFIER optional_template_assignment {
        $$ = ε;
    }
    ;
optional_template_assignment:
    { $$ = ε; }
    | '=' '(' expression ')' {
        $$ = ε;
    }
    ;

/* Inheritance */
inheritance:
    { $$ = ε; }
    | ':' inheritance_list {
        $$ = ε;
    }
    ;
inheritance_list:
    expression ',' inheritance_list {
        $$ = ε;
    }
    | expression {
        $$ = ε;
    }
    ;

/* Statements */

statement:
    optional_annotation subdeclaration ';' {
        $$ = ε;
    }
    | optional_annotation nondeclarative_statement ';' {
        $$ = ε;
    }
    | optional_annotation block_based {
        $$ = ε;
    }
    ;

optional_annotation:
    { $$ = ε; }
    | ANNOTATION {
        $$ = ε;
    }
    ;

/* Blocks */
block_based:
    if {
        $$ = ε;
    }
    | KEYWORD_FOR IDENTIFIER KEYWORD_IN range block {
        $$ = ε;
    }
    | KEYWORD_NAMESPACE IDENTIFIER block {
        $$ = ε;
    }
    | KEYWORD_SWITCH expression switch_block {
        $$ = ε;
    }
    | KEYWORD_ASYNC block {
        $$ = ε;
    }
    | error '}' {
        $$ = ε;
    }
    ;
if:
    KEYWORD_IF expression block else {
        $$ = ε;
    }
    ;

else:
    { $$ = ε; }
    | KEYWORD_ELSE if {
        $$ = ε;
    }
    | KEYWORD_ELSE block {
        $$ = ε;
    }
    ;

switch_block:
    '{' labeled_statement_list '}' {
        $$ = ε;
    }
    ;
labeled_statement_list:
    { $$ = ε; }
    | KEYWORD_CASE expression ':' statement_list labeled_statement_list {
        $$ = ε;
    }
    | KEYWORD_DEFAULT ':' statement_list {
        $$ = ε;
    }
    ;

block:
    '{' statement_list '}' {
        $$ = ε;
    }
    ;

statement_list:
    { $$ = ε; }
    | statement_list statement {
        $$ = ε;
    }
    ;

/* Subdeclarations */

subdeclaration:
    dynamic_width optional_bus_declaration declaration_list {
        $$ = ε;
    }
    | probable_template IDENTIFIER optional_array_declaration optional_ports {
        $$ = ε;
    }
    ;

dynamic_width:
    KEYWORD_SW_VAR {
        $$ = ε;
    }
    | KEYWORD_WIRE {
        $$ = ε;
    }
    | KEYWORD_REGISTER {
        $$ = ε;
    }
    ;
optional_bus_declaration:
    { $$ = ε; }
    | '[' range ']' {
        $$ = ε;
    }
    ;
optional_array_declaration:
    { $$ = ε; }
    | '[' expression ']' {
        $$ = ε;
    }
    ;

optional_ports:
    { $$ = ε; }
    | ports {
        $$ = ε;
    }
    ;
declaration_list:
    IDENTIFIER optional_array_declaration optional_assignment ',' declaration_list {
        $$ = ε;
    }
    | IDENTIFIER optional_array_declaration optional_assignment {
        $$ = ε;
    }
    ;
optional_assignment:
    { $$ = ε; }
    | '=' expression {
        $$ = ε;
    }
    ;

probable_template:
    expression {
        $$ = ε;
    }
    | expression '<' template_list '>' {
        $$ = ε;
    }
    ;
template_list:
    { $$ = ε; }
    | IDENTIFIER ':' '(' expression ')' ',' template_list {
        $$ = ε;
    }
    | IDENTIFIER ':' '(' expression ')' {
        $$ = ε;
    }
    ;
ports:
    '(' ')' {
        $$ = ε;
    }
    | '(' port_list ')' {
        $$ = ε;
    }
    ;
port_list:
    IDENTIFIER ':' expression ',' port_list {
        $$ = ε;
    }
    | IDENTIFIER ':' expression {
        $$ = ε;
    }
    ;

/* Nondeclarative Statements */
    
nondeclarative_statement:
    expression '=' expression {
        $$ = ε;
    }
    | expression ports {
        $$ = ε;
    }
    ;

/* Expressions */

expression:
    expression '?' expression ':' expression {
        $$ = ε;
    }
    | expression OP_EQ expression {
        $$ = ε;
    }
    | expression OP_NEQ expression {
        $$ = ε;
    }
    | expression '>' expression {
        $$ = ε;
    }
    | expression '<' expression {
        $$ = ε;
    }
    | expression OP_GTE expression {
        $$ = ε;
    }
    | expression OP_LTE expression {
        $$ = ε;
    }
    | expression OP_UNSIGNED_LT expression {
        $$ = ε;
    }
    | expression OP_UNSIGNED_GT  expression {
        $$ = ε;
    } 
    | expression OP_UNSIGNED_LTE expression {
        $$ = ε;
    }
    | expression OP_UNSIGNED_GTE expression {
        $$ = ε;
    }
    | expression '+' expression {
        $$ = ε;
    }
    | expression '-' expression {
        $$ = ε;
    }
    | expression OP_UNSIGNED_ADD expression{
        $$ = ε;
    }
    | expression OP_UNSIGNED_SUB expression {
        $$ = ε;
    }
    | expression '|' expression {
        $$ = ε;
    }
    | expression '&' expression {
        $$ = ε;
    }
    | expression '^' expression {
        $$ = ε;
    }
    | expression '*' expression {
        $$ = ε;
    }
    | expression '/' expression {
        $$ = ε;
    }
    | expression '%' expression {
        $$ = ε;
    }
    | expression OP_SLL expression {
        $$ = ε;
    }
    | expression OP_SRL expression {
        $$ = ε;
    }
    | expression OP_SRA expression {
        $$ = ε;
    }
    | '-' expression %prec UNARY {
        $$ = ε;
    }
    | '&' expression %prec UNARY {
        $$ = ε;
    }
    | '|' expression %prec UNARY {
        $$ = ε;
    }
    | '~' expression %prec UNARY {
        $$ = ε;
    }
    | KEYWORD_POSEDGE expression {
        $$ = ε;
    }
    | KEYWORD_NEGEDGE expression {
        $$ = ε;
    }
    | expression '.' expression {
        $$ = ε;
    }
    | expression subscript {
        $$ = ε;
    }
    | '[' concatenation ']' {
        $$ = ε;
    }
    | '(' expression ')' {
        $$ = ε;
    }
    | '$' IDENTIFIER '(' procedural_call ')' {
        $$ = ε;
    }
    | KEYWORD_MUX expression mux_block {
        $$ = ε;
    }
    | IDENTIFIER {
        $$ = ε;
    }
    | FW_NUMERIC {
        $$ = ε;
    }
    | NUMERIC  {
        $$ = ε;
    }
    ;

subscript:
    '[' range ']' {
        $$ = ε;
    }
    | '[' expression ']' {
        $$ = ε;
    }
    ;


range:
    expression OP_RANGE expression {
        $$ = ε;
    }
    ;

mux_block:
    '{' labeled_expression_list '}' {
        $$ = ε;
    }
    ;
labeled_expression_list:
    { $$ = ε; }
    | expression ':' expression ',' labeled_expression_list {
        $$ = ε;
    }
    | expression ':' expression {
        $$ = ε;
    }
    | KEYWORD_DEFAULT ':' expression {
        $$ = ε;
    }
    ;

concatenation:
    concatenatable ',' concatenation {
        $$ = ε;
    }
    | concatenatable {
        $$ = ε;
    }
    ;
concatenatable:
    expression {
        $$ = ε;
    }
    | expression LEFT_REPEAT_CAT expression ']' ']' {
        $$ = ε;
    }
    ;

procedural_call: {
        $$ = ε;
    }
    | procedural_call_list {
        $$ = ε;
    }
    ;
procedural_call_list:
    expression ',' procedural_call_list {
        $$ = ε;
    }
    | STRING ',' procedural_call_list {
        $$ = ε;
    }
    | expression {
        $$ = ε;
    }
    | STRING {
        $$ = ε;
    }
    ;

%%
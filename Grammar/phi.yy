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
    #define epsilon nullptr
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
    @$.begin.filename = @$.end.filename = &context->files.back();
#if YYDEBUG
    this->set_debug_level(context->trace);
#endif
}

%%

/* Top Level */

description:
    { $$ = epsilon; }
    | declaration description  {
        $$ = epsilon;
    }
    | KEYWORD_NAMESPACE IDENTIFIER '{' description '}' {
        $$ = epsilon;
    }
    ;

declaration:
    KEYWORD_MODULE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance block  {
        $$ = epsilon;
    }
    | KEYWORD_MODULE error '}' {
        $$ = epsilon;
    }
    | KEYWORD_INTERFACE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance ';' {
        $$ = epsilon;
    }
    | KEYWORD_INTERFACE error ';' {
        $$ = epsilon;
    }
    ;

/* Ports */
port_declaration_list:
    { $$ = epsilon; }
    | populated_port_declaration_list {
        $$ = epsilon;
    }
    ;
populated_port_declaration_list:
    IDENTIFIER ':' port_declaration ',' populated_port_declaration_list {
        $$ = epsilon;
    }
    | IDENTIFIER ':' port_declaration {
        $$ = epsilon;
    }
    ;
port_declaration:
    port_polarity {
        $$ = epsilon;
    }
    | ANNOTATION port_polarity {
        $$ = epsilon;
    }
    ;
port_polarity:
    KEYWORD_INPUT optional_bus_declaration {
        $$ = epsilon;
    }
    | KEYWORD_OUTPUT optional_bus_declaration {
        $$ = epsilon;
    }
    ;

/* Templating */
template_declaration:
    { $$ = epsilon; }
    | '<' template_declaration_list '>' {
        $$ = epsilon;
    }
    ;
template_declaration_list:
    IDENTIFIER optional_template_assignment template_declaration_list {
        $$ = epsilon;
    }
    | IDENTIFIER optional_template_assignment {
        $$ = epsilon;
    }
    ;
optional_template_assignment:
    { $$ = epsilon; }
    | '=' '(' expression ')' {
        $$ = epsilon;
    }
    ;

/* Inheritance */
inheritance:
    { $$ = epsilon; }
    | ':' inheritance_list {
        $$ = epsilon;
    }
    ;
inheritance_list:
    expression ',' inheritance_list {
        $$ = epsilon;
    }
    | expression {
        $$ = epsilon;
    }
    ;

/* Statements */

statement:
    optional_annotation subdeclaration ';' {
        $$ = epsilon;
    }
    | optional_annotation nondeclarative_statement ';' {
        $$ = epsilon;
    }
    | optional_annotation block_based {
        $$ = epsilon;
    }
    ;

optional_annotation:
    { $$ = epsilon; }
    | ANNOTATION {
        $$ = epsilon;
    }
    ;

/* Blocks */
block_based:
    if {
        $$ = epsilon;
    }
    | KEYWORD_FOR IDENTIFIER KEYWORD_IN range block {
        $$ = epsilon;
    }
    | KEYWORD_NAMESPACE IDENTIFIER block {
        $$ = epsilon;
    }
    | KEYWORD_SWITCH expression switch_block {
        $$ = epsilon;
    }
    | KEYWORD_ASYNC block {
        $$ = epsilon;
    }
    | error '}' {
        $$ = epsilon;
    }
    ;
if:
    KEYWORD_IF expression block else {
        $$ = epsilon;
    }
    ;

else:
    { $$ = epsilon; }
    | KEYWORD_ELSE if {
        $$ = epsilon;
    }
    | KEYWORD_ELSE block {
        $$ = epsilon;
    }
    ;

switch_block:
    '{' labeled_statement_list '}' {
        $$ = epsilon;
    }
    ;
labeled_statement_list:
    { $$ = epsilon; }
    | KEYWORD_CASE expression ':' statement_list labeled_statement_list {
        $$ = epsilon;
    }
    | KEYWORD_DEFAULT ':' statement_list {
        $$ = epsilon;
    }
    ;

block:
    '{' statement_list '}' {
        $$ = epsilon;
    }
    ;

statement_list:
    { $$ = epsilon; }
    | statement_list statement {
        $$ = epsilon;
    }
    ;

/* Subdeclarations */

subdeclaration:
    dynamic_width optional_bus_declaration declaration_list {
        $$ = epsilon;
    }
    | probable_template IDENTIFIER optional_array_declaration optional_ports {
        $$ = epsilon;
    }
    ;

dynamic_width:
    KEYWORD_SW_VAR {
        $$ = epsilon;
    }
    | KEYWORD_WIRE {
        $$ = epsilon;
    }
    | KEYWORD_REGISTER {
        $$ = epsilon;
    }
    ;
optional_bus_declaration:
    { $$ = epsilon; }
    | '[' range ']' {
        $$ = epsilon;
    }
    ;
optional_array_declaration:
    { $$ = epsilon; }
    | '[' expression ']' {
        $$ = epsilon;
    }
    ;

optional_ports:
    { $$ = epsilon; }
    | ports {
        $$ = epsilon;
    }
    ;
declaration_list:
    IDENTIFIER optional_array_declaration optional_assignment ',' declaration_list {
        $$ = epsilon;
    }
    | IDENTIFIER optional_array_declaration optional_assignment {
        $$ = epsilon;
    }
    ;
optional_assignment:
    { $$ = epsilon; }
    | '=' expression {
        $$ = epsilon;
    }
    ;

probable_template:
    expression {
        $$ = epsilon;
    }
    | expression '<' template_list '>' {
        $$ = epsilon;
    }
    ;
template_list:
    { $$ = epsilon; }
    | IDENTIFIER ':' '(' expression ')' ',' template_list {
        $$ = epsilon;
    }
    | IDENTIFIER ':' '(' expression ')' {
        $$ = epsilon;
    }
    ;
ports:
    '(' ')' {
        $$ = epsilon;
    }
    | '(' port_list ')' {
        $$ = epsilon;
    }
    ;
port_list:
    IDENTIFIER ':' expression ',' port_list {
        $$ = epsilon;
    }
    | IDENTIFIER ':' expression {
        $$ = epsilon;
    }
    ;

/* Nondeclarative Statements */
    
nondeclarative_statement:
    expression '=' expression {
        $$ = epsilon;
    }
    | expression ports {
        $$ = epsilon;
    }
    ;

/* Expressions */

expression:
    expression '?' expression ':' expression {
        $$ = epsilon;
    }
    | expression OP_EQ expression {
        $$ = epsilon;
    }
    | expression OP_NEQ expression {
        $$ = epsilon;
    }
    | expression '>' expression {
        $$ = epsilon;
    }
    | expression '<' expression {
        $$ = epsilon;
    }
    | expression OP_GTE expression {
        $$ = epsilon;
    }
    | expression OP_LTE expression {
        $$ = epsilon;
    }
    | expression OP_UNSIGNED_LT expression {
        $$ = epsilon;
    }
    | expression OP_UNSIGNED_GT  expression {
        $$ = epsilon;
    } 
    | expression OP_UNSIGNED_LTE expression {
        $$ = epsilon;
    }
    | expression OP_UNSIGNED_GTE expression {
        $$ = epsilon;
    }
    | expression '+' expression {
        $$ = epsilon;
    }
    | expression '-' expression {
        $$ = epsilon;
    }
    | expression OP_UNSIGNED_ADD expression{
        $$ = epsilon;
    }
    | expression OP_UNSIGNED_SUB expression {
        $$ = epsilon;
    }
    | expression '|' expression {
        $$ = epsilon;
    }
    | expression '&' expression {
        $$ = epsilon;
    }
    | expression '^' expression {
        $$ = epsilon;
    }
    | expression '*' expression {
        $$ = epsilon;
    }
    | expression '/' expression {
        $$ = epsilon;
    }
    | expression '%' expression {
        $$ = epsilon;
    }
    | expression OP_SLL expression {
        $$ = epsilon;
    }
    | expression OP_SRL expression {
        $$ = epsilon;
    }
    | expression OP_SRA expression {
        $$ = epsilon;
    }
    | '-' expression %prec UNARY {
        $$ = epsilon;
    }
    | '&' expression %prec UNARY {
        $$ = epsilon;
    }
    | '|' expression %prec UNARY {
        $$ = epsilon;
    }
    | '~' expression %prec UNARY {
        $$ = epsilon;
    }
    | KEYWORD_POSEDGE expression {
        $$ = epsilon;
    }
    | KEYWORD_NEGEDGE expression {
        $$ = epsilon;
    }
    | expression '.' expression {
        $$ = epsilon;
    }
    | expression subscript {
        $$ = epsilon;
    }
    | '[' concatenation ']' {
        $$ = epsilon;
    }
    | '(' expression ')' {
        $$ = epsilon;
    }
    | '$' IDENTIFIER '(' procedural_call ')' {
        $$ = epsilon;
    }
    | KEYWORD_MUX expression mux_block {
        $$ = epsilon;
    }
    | IDENTIFIER {
        $$ = epsilon;
    }
    | FW_NUMERIC {
        $$ = epsilon;
    }
    | NUMERIC  {
        $$ = epsilon;
    }
    ;

subscript:
    '[' range ']' {
        $$ = epsilon;
    }
    | '[' expression ']' {
        $$ = epsilon;
    }
    ;


range:
    expression OP_RANGE expression {
        $$ = epsilon;
    }
    ;

mux_block:
    '{' labeled_expression_list '}' {
        $$ = epsilon;
    }
    ;
labeled_expression_list:
    { $$ = epsilon; }
    | expression ':' expression ',' labeled_expression_list {
        $$ = epsilon;
    }
    | expression ':' expression {
        $$ = epsilon;
    }
    | KEYWORD_DEFAULT ':' expression {
        $$ = epsilon;
    }
    ;

concatenation:
    concatenatable ',' concatenation {
        $$ = epsilon;
    }
    | concatenatable {
        $$ = epsilon;
    }
    ;
concatenatable:
    expression {
        $$ = epsilon;
    }
    | expression LEFT_REPEAT_CAT expression ']' ']' {
        $$ = epsilon;
    }
    ;

procedural_call: {
        $$ = epsilon;
    }
    | procedural_call_list {
        $$ = epsilon;
    }
    ;
procedural_call_list:
    expression ',' procedural_call_list {
        $$ = epsilon;
    }
    | STRING ',' procedural_call_list {
        $$ = epsilon;
    }
    | expression {
        $$ = epsilon;
    }
    | STRING {
        $$ = epsilon;
    }
    ;

%%

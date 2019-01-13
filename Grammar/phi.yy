%language "C++"
%defines
%locations

%define parser_class_name {Parser}
%define api.namespace {Phi}

%code requires {
    #include "Node.h"
    using namespace Phi::Node;
}

%{
    // C STD
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>

    // CPP STL
    #include <sstream>

    // Project Headers
    #include "Context.h"
    
    #include "Utils.h"

    void yyerror(char *);
    int yylex();

    extern int yylineno;
    extern char* yytext;

    #define catchIntoContext catch (const char* error) { context->errorList.push_back({yylhs.location, std::string(error)}); };
    #define epsilon nullptr
%}

%parse-param { Phi::Context* context }
%lex-param   { Phi::Context* context }

%union {
    char* text;
    struct Node* node;
    bool bin;
    VariableLengthDeclaration::Type vldt;
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
%token KEYWORD_LATCH
%token KEYWORD_COMB
%token KEYWORD_INPUT
%token KEYWORD_OUTPUT

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
%left '~' UNARY
%left '.'
%right '['

%type<text> NUMERIC FW_NUMERIC IDENTIFIER ANNOTATION STRING optional_annotation

%type<bin> port_polarity

%type<vldt> dynamic_width

// Silly conversion
%type<node> description declaration port_declaration_list populated_port_declaration_list template_declaration template_declaration_list optional_template_assignment inheritance inheritance_list statement block_based if else  labeled_statement_list block statement_list subdeclaration  optional_bus_declaration optional_array_declaration optional_ports declaration_list optional_assignment optional_template template_list ports port_list nondeclarative_statement expression range mux_block labeled_expression_list concatenation concatenatable procedural_call procedural_call_list 

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
        auto node = $1;
        node->right = $2;
        $$ = node;
    }
    | KEYWORD_NAMESPACE IDENTIFIER '{' description '}' description {
        auto node = new TopLevelNamespace($2, $4);
        node->right = $6;
        $$ = node;
    }
    ;

declaration:
    KEYWORD_MODULE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance block  {
        $$ = new TopLevelDeclaration($2, TopLevelDeclaration::Type::module, (Port*)$5, (Expression*)$7, (Statement*)$8);
    }
    | KEYWORD_MODULE error '}' {
        $$ = epsilon;
    }
    | KEYWORD_INTERFACE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance ';' {
        $$ = new TopLevelDeclaration($2, TopLevelDeclaration::Type::interface, (Port*)$5, (Expression*)$7);
    }
    | KEYWORD_INTERFACE error ';' {
        $$ = epsilon;
    }
    ;

/* Ports */
port_declaration_list:
    { $$ = epsilon; }
    | populated_port_declaration_list {
        $$ = $1;
    }
    ;
populated_port_declaration_list:
    IDENTIFIER ':' optional_annotation port_polarity optional_bus_declaration ',' populated_port_declaration_list {
        auto node = new Port($1, $4, (Range*)$5, $3);
        node->right = $7;
        $$ = node;
    }
    | IDENTIFIER ':' optional_annotation port_polarity optional_bus_declaration {
        $$ = new Port($1, $4, (Range*)$5, $3);
    }
    ;
port_polarity:
    KEYWORD_INPUT {
        $$ = true;
    }
    | KEYWORD_OUTPUT {
        $$ = false;
    }
    ;

/* Templating */
template_declaration:
    { $$ = epsilon; }
    | '<' template_declaration_list '>' {
        $$ = $2;
    }
    ;
template_declaration_list:
    IDENTIFIER optional_template_assignment template_declaration_list {
        auto node = new TemplateDeclaration($1, (Expression*)$2);
        node->assignment = (Expression*)$3;
        $$ = node;
    }
    | IDENTIFIER optional_template_assignment {
        $$ = new TemplateDeclaration($1, (Expression*)$2);
    }
    ;
optional_template_assignment:
    { $$ = epsilon; }
    | '=' '(' expression ')' {
        $$ = $3;
    }
    ;

/* Inheritance */
inheritance:
    { $$ = epsilon; }
    | ':' inheritance_list {
        $$ = $2;
    }
    ;
inheritance_list:
    expression ',' inheritance_list {
        auto node = $1;
        node->right = $3;
        $$ = node;
    }
    | expression {
        $$ = $1;
    }
    ;

/* Statements */

statement:
    optional_annotation subdeclaration ';' {
        auto node = (Statement*)$2;
        if ($1) {
            node->annotation = $1;
        }
        $$ = node;
    }
    | optional_annotation nondeclarative_statement ';' {
        auto node = (Statement*)$2;
        if ($1) {
            node->annotation = $1;
        }
        $$ = node;
    }
    | optional_annotation block_based {
        auto node = (Statement*)$2;
        if ($1) {
            node->annotation = $1;
        }
        $$ = node;
    }
    ;

optional_annotation:
    { $$ = epsilon; }
    | ANNOTATION {
        $$ = $1;
    }
    ;

/* Blocks */
block_based:
    if {
        $$ = $1;
    }
    | KEYWORD_FOR IDENTIFIER KEYWORD_IN range block {
        $$ = new ForLoop((Statement*)$5, (Range*)$4, $2);
    }
    | KEYWORD_NAMESPACE IDENTIFIER block {
        $$ = new Namespace((Statement*)$3, $2);
    }
    | KEYWORD_SWITCH expression '{' labeled_statement_list '}' {
        $$ = new Switch((Expression*)$2, (LabeledStatementList*)$4);
    }
    | KEYWORD_COMB block {
        $$ = new Combinational((Statement*)$2);
    }
    | error '}' {
        $$ = epsilon;
    }
    ;
if:
    KEYWORD_IF expression block else {
        $$ = new If((Statement*)$3, (Expression*)$2, (If*)$4);
    }
    ;

else:
    { $$ = epsilon; }
    | KEYWORD_ELSE if {
        $$ = $2;
    }
    | KEYWORD_ELSE block {
        $$ = new If((Statement*)$2, nullptr, nullptr);
    }
    ;

labeled_statement_list:
    { $$ = epsilon; }
    | KEYWORD_CASE expression ':' statement_list labeled_statement_list {
        auto node = new LabeledStatementList(false, (Expression*)$2, (Statement*)$4);
        node->right = $5;
        $$ = node;
    }
    | KEYWORD_DEFAULT ':' statement_list {
        $$ = new LabeledStatementList(false, nullptr, (Statement*)$3);;
    }
    ;

block:
    '{' statement_list '}' {
        $$ = $2;
    }
    ;

statement_list:
    { $$ = epsilon; }
    | statement_list statement {
        auto node = $1;
        $1->right = $2;
        $$ = $1;
    }
    ;

/* Subdeclarations */
subdeclaration:
    dynamic_width optional_bus_declaration declaration_list {
        $$ = VariableLengthDeclaration::flattenedList($1, (Range*)$2, (DeclarationListItem*)$3);
    }
    | expression optional_template IDENTIFIER optional_array_declaration optional_ports {
        $$ = new InstanceDeclaration($3, (Expression*)$1, (ExpressionIDPair*)$2, (Expression*)$4, (ExpressionIDPair*)$5);
    }
    ;

dynamic_width:
    KEYWORD_SW_VAR {
        $$ = VariableLengthDeclaration::Type::var;
    }
    | KEYWORD_WIRE {
        $$ = VariableLengthDeclaration::Type::wire;
    }
    | KEYWORD_REGISTER {
        $$ = VariableLengthDeclaration::Type::reg;
    }
    | KEYWORD_LATCH {
        $$ = VariableLengthDeclaration::Type::latch;
    }
    ;
optional_bus_declaration:
    { $$ = epsilon; }
    | '[' range ']' {
        $$ = $2;
    }
    ;

optional_array_declaration:
    { $$ = epsilon; }
    | '[' expression ']' {
        $$ = $2;
    }
    ;
    
declaration_list:
    IDENTIFIER optional_array_declaration optional_assignment ',' declaration_list {
        auto node = new DeclarationListItem($1, (Expression*)$2, (Expression*)$3);
        node->right = $5;
        $$ = node;
    }
    | IDENTIFIER optional_array_declaration optional_assignment {
        $$ = new DeclarationListItem($1, (Expression*)$2, (Expression*)$3);
    }
    ;
optional_assignment:
    { $$ = epsilon; }
    | '=' expression {
        $$ = $2;
    }
    ;
optional_template:
    { $$ = epsilon; }
    | '<' template_list '>' {
        $$ = $2;
    }
    ;

template_list:
    { $$ = epsilon; }
    | IDENTIFIER ':' '(' expression ')' ',' template_list {
        auto node = new ExpressionIDPair($1, (Expression*)$4);
        node->right = $7;
        $$ = node;
    }
    | IDENTIFIER ':' '(' expression ')' {
        $$ = new ExpressionIDPair($1, (Expression*)$4);
    }
    ;


optional_ports:
    { $$ = epsilon; }
    | ports {
        $$ = $1;
    }
    ;

/* Note: Ports separated from optional_ports because its used elsewhere */
ports:
    '(' port_list ')' {
        $$ = $2;
    }
    ;


port_list:
    IDENTIFIER ':' expression ',' port_list {
        auto node = new ExpressionIDPair($1, (Expression*)$3);
        node->right = $5;
        $$ = node;
    }
    | IDENTIFIER ':' expression {
        $$ = new ExpressionIDPair($1, (Expression*)$3);
    }
    ;

/* Nondeclarative Statements */
    
nondeclarative_statement:
    expression '=' expression {
        $$ = new NondeclarativeAssignment((Expression*)$1, (Expression*)$3);
    }
    | expression ports {
        $$ = new NondeclarativePorts((Expression*)$1, (ExpressionIDPair*)$2);
    }
    ;

/* Expressions */

expression:
    expression '?' expression ':' expression {
        $$ = new Ternary((Expression*)$1, (Expression*)$3, (Expression*)$5);
    }
    | expression OP_EQ expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::equal, (Expression*)$3);
    }
    | expression OP_NEQ expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::notEqual, (Expression*)$3);
    }
    | expression '>' expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::greaterThan, (Expression*)$3);
    }
    | expression '<' expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::lessThan, (Expression*)$3);
    }
    | expression OP_GTE expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::greaterThanOrEqual, (Expression*)$3);
    }
    | expression OP_LTE expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::lessThanOrEqual, (Expression*)$3);
    }
    | expression OP_UNSIGNED_LT expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::unsignedLessThan, (Expression*)$3);
    }
    | expression OP_UNSIGNED_GT  expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::unsignedGreaterThan, (Expression*)$3);
    } 
    | expression OP_UNSIGNED_LTE expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::unsignedLessThanOrEqual, (Expression*)$3);
    }
    | expression OP_UNSIGNED_GTE expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::unsignedGreaterThanOrEqual, (Expression*)$3);
    }
    | expression '+' expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::plus, (Expression*)$3);
    }
    | expression '-' expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::minus, (Expression*)$3);
    }
    | expression OP_UNSIGNED_ADD expression{
        $$ = new Binary((Expression*)$1, Binary::Operation::unsignedPlus, (Expression*)$3);
    }
    | expression OP_UNSIGNED_SUB expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::unsignedMinus, (Expression*)$3);
    }
    | expression '|' expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::bitwiseOr, (Expression*)$3);
    }
    | expression '&' expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::bitwiseAnd, (Expression*)$3);
    }
    | expression '^' expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::bitwiseXor, (Expression*)$3);
    }
    | expression '*' expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::mul, (Expression*)$3);
    }
    | expression '/' expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::div, (Expression*)$3);
    }
    | expression '%' expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::modulo, (Expression*)$3);
    }
    | expression OP_SLL expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::shiftLeftLogical, (Expression*)$3);
    }
    | expression OP_SRL expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::shiftRightLogical, (Expression*)$3);
    }
    | expression OP_SRA expression {
        $$ = new Binary((Expression*)$1, Binary::Operation::shiftRightArithmetic, (Expression*)$3);
    }
    | '-' expression %prec UNARY {
        $$ = new Unary(Unary::Operation::negate, (Expression*)$2);
    }
    | '&' expression %prec UNARY {
        $$ = new Unary(Unary::Operation::allAnd, (Expression*)$2);
    }
    | '|' expression %prec UNARY {
        $$ = new Unary(Unary::Operation::allOr, (Expression*)$2);
    }
    | '~' expression %prec UNARY {
        $$ = new Unary(Unary::Operation::bitwiseNot, (Expression*)$2);
    }
    | expression '.' expression {
        $$ = epsilon;
    }
    | expression '[' range ']' {
        $$ = epsilon;
    }
    | expression '[' expression ']' {
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

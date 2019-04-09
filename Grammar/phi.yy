%language "C++"
%locations
%require "3.2"

%define api.parser.class {Parser}
%define api.namespace {Phi}

%code requires {
    namespace Phi {
        class Lexer;
    }
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

    //Lex
    #include "phi.l.hh"

    // Project Headers
    #include "Context.h"
    #include "Utils.h"

    #undef yylex
    #define yylex lexer->yylex

    #define catchIntoContext catch (const char* error) { context->errorList.push_back({yylhs.location, std::string(error)}); };
    #define epsilon nullptr
%}

%parse-param { Phi::Lexer* lexer } { Phi::Context* context }

%union {
    char* text;
    struct Node* node;
    struct LHExpression* lhexpr;
    struct Expression* expr;
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
%token FW_SPECIAL
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

%left OP_RANGE
%left OP_EQ OP_NEQ OP_GTE OP_LTE '<' '>' OP_UNSIGNED_LT OP_UNSIGNED_LTE OP_UNSIGNED_GT OP_UNSIGNED_GTE
%left OP_UNSIGNED_ADD OP_UNSIGNED_SUB '+' '-' '|' '&' '^'
%left '*' '/' '%'
%left OP_SRL OP_SRA OP_SLL
%left '~' UNARY
%right '.'
%right '['

%type<text> NUMERIC FW_NUMERIC FW_SPECIAL IDENTIFIER ANNOTATION STRING optional_annotation

%type<bin> port_polarity

%type<vldt> dynamic_width

%type<node> description declaration port_declaration_list populated_port_declaration_list template_declaration template_declaration_list statement block_based if else labeled_statement_list block statement_list subdeclaration  optional_bus_declaration  optional_ports declaration_list optional_template template_list ports port_list nondeclarative_statement range mux_block labeled_expression_list   procedural_call procedural_call_list special_number

%type<lhexpr> lhexpression
%type<expr> expression inheritance inheritance_list optional_template_assignment optional_array_declaration optional_assignment concatenation concatenatable mux

// %{
//     extern int yylex(Phi::Parser::semantic_type* yylval,
//                      Phi::Parser::location_type* yylloc,
//                      Phi::Context* context
//     );
// %}

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
        context->head = node;
    }
    | KEYWORD_NAMESPACE IDENTIFIER '{' description '}' description {
        auto node = new TopLevelNamespace($2, $4);
        node->right = $6;
        $$ = node;
        context->head = node;
    }
    ;

declaration:
    KEYWORD_MODULE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance block  {
        $$ = new TopLevelDeclaration($2, TopLevelDeclaration::Type::module, (Port*)$5, $7, (Statement*)$8);
    }
    | KEYWORD_MODULE error '}' {
        $$ = new ErrorNode();
    }
    | KEYWORD_INTERFACE IDENTIFIER template_declaration '(' port_declaration_list ')' inheritance ';' {
        $$ = new TopLevelDeclaration($2, TopLevelDeclaration::Type::interface, (Port*)$5, $7);
    }
    | KEYWORD_INTERFACE error ';' {
        $$ = new ErrorNode();
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
        $$ = false;
    }
    | KEYWORD_OUTPUT {
        $$ = true;
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
        auto node = new TemplateDeclaration($1, $2);
        node->right = $3;
        $$ = node;
    }
    | IDENTIFIER optional_template_assignment {
        $$ = new TemplateDeclaration($1, $2);
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
    lhexpression ',' inheritance_list {
        auto node = $1;
        node->right = $3;
        $$ = node;
    }
    | lhexpression {
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
special_number:
    FW_SPECIAL {
        $$ = new SpecialNumber($1);
    }
    | '(' FW_SPECIAL ')' {
        $$ = new SpecialNumber($2);
    }
    ;
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
        $$ = new Switch($2, (LabeledStatementList*)$4);
    }
    | KEYWORD_COMB block {
        $$ = new Combinational((Statement*)$2);
    }
    | error '}' {
        $$ = new ErrorNode();
    }
    ;
if:
    KEYWORD_IF expression block else {
        $$ = new If((Statement*)$3, $2, (If*)$4);
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
        auto node = new LabeledStatementList(false, (Expression*)$2, nullptr, (Statement*)$4);
        node->right = $5;
        $$ = node;
    }
    | KEYWORD_CASE special_number ':' statement_list labeled_statement_list {
        auto node = new LabeledStatementList(false, nullptr, (SpecialNumber*)$2, (Statement*)$4);
        node->right = $5;
        $$ = node;
    }
    | KEYWORD_DEFAULT ':' statement_list {
        $$ = new LabeledStatementList(false, nullptr, nullptr, (Statement*)$3);;
    }
    ;

block:
    '{' statement_list '}' {
        $$ = $2;
    }
    ;

statement_list:
    { $$ = epsilon; }
    | statement statement_list {
        auto node = $1;
        node->right = $2;
        $$ = $1;
    }
    ;

/* Subdeclarations */
subdeclaration:
    dynamic_width optional_bus_declaration declaration_list {
        $$ = new VariableLengthDeclaration($1, (Range*)$2, (DeclarationListItem*)$3);
    }
    | lhexpression optional_template IDENTIFIER optional_array_declaration optional_ports {
        $$ = new InstanceDeclaration($3, $1, (ExpressionIDPair*)$2, $4, (ExpressionIDPair*)$5);
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
        auto node = new DeclarationListItem($1, $2, $3);
        node->right = $5;
        $$ = node;
    }
    | IDENTIFIER optional_array_declaration optional_assignment {
        $$ = new DeclarationListItem($1, $2, $3);
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
        auto node = new ExpressionIDPair($1, $4);
        node->right = $7;
        $$ = node;
    }
    | IDENTIFIER ':' '(' expression ')' {
        $$ = new ExpressionIDPair($1, $4);
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
        auto node = new ExpressionIDPair($1, $3);
        node->right = $5;
        $$ = node;
    }
    | IDENTIFIER ':' expression {
        $$ = new ExpressionIDPair($1, $3);
    }
    ;

/* Nondeclarative Statements */
    
nondeclarative_statement:
    lhexpression '=' expression {
        $$ = new NondeclarativeAssignment($1, $3);
    }
    | lhexpression ports {
        $$ = new NondeclarativePorts($1, (ExpressionIDPair*)$2);
    }
    ;

/* Expressions */
lhexpression:
    IDENTIFIER {
        $$ = new IdentifierExpression($1);
    } 
    | lhexpression '.' lhexpression {
        $$ = new PropertyAccess($1, $3);
    }
    | lhexpression '[' range ']' {
        $$ = new RangeAccess($1, (Range*)$3);
    }
    | lhexpression '[' expression ']' {
        $$ = new ArrayAccess($1, $3);;
    }
    ;

expression:
    expression OP_EQ expression {
        $$ = new Binary($1, Binary::Operation::equal, $3);
    }
    | expression OP_NEQ expression {
        $$ = new Binary($1, Binary::Operation::notEqual, $3);
    }
    | expression '>' expression {
        $$ = new Binary($1, Binary::Operation::greaterThan, $3);
    }
    | expression '<' expression {
        $$ = new Binary($1, Binary::Operation::lessThan, $3);
    }
    | expression OP_GTE expression {
        $$ = new Binary($1, Binary::Operation::greaterThanOrEqual, $3);
    }
    | expression OP_LTE expression {
        $$ = new Binary($1, Binary::Operation::lessThanOrEqual, $3);
    }
    | expression OP_UNSIGNED_LT expression {
        $$ = new Binary($1, Binary::Operation::unsignedLessThan, $3);
    }
    | expression OP_UNSIGNED_GT  expression {
        $$ = new Binary($1, Binary::Operation::unsignedGreaterThan, $3);
    } 
    | expression OP_UNSIGNED_LTE expression {
        $$ = new Binary($1, Binary::Operation::unsignedLessThanOrEqual, $3);
    }
    | expression OP_UNSIGNED_GTE expression {
        $$ = new Binary($1, Binary::Operation::unsignedGreaterThanOrEqual, $3);
    }
    | expression '+' expression {
        $$ = new Binary($1, Binary::Operation::plus, $3);
    }
    | expression '-' expression {
        $$ = new Binary($1, Binary::Operation::minus, $3);
    }
    | expression OP_UNSIGNED_ADD expression{
        $$ = new Binary($1, Binary::Operation::unsignedPlus, $3);
    }
    | expression OP_UNSIGNED_SUB expression {
        $$ = new Binary($1, Binary::Operation::unsignedMinus, $3);
    }
    | expression '|' expression {
        $$ = new Binary($1, Binary::Operation::bitwiseOr, $3);
    }
    | expression '&' expression {
        $$ = new Binary($1, Binary::Operation::bitwiseAnd, $3);
    }
    | expression '^' expression {
        $$ = new Binary($1, Binary::Operation::bitwiseXor, $3);
    }
    | expression '*' expression {
        $$ = new Binary($1, Binary::Operation::mul, $3);
    }
    | expression '/' expression {
        $$ = new Binary($1, Binary::Operation::div, $3);
    }
    | expression '%' expression {
        $$ = new Binary($1, Binary::Operation::modulo, $3);
    }
    | expression OP_SLL expression {
        $$ = new Binary($1, Binary::Operation::shiftLeftLogical, $3);
    }
    | expression OP_SRL expression {
        $$ = new Binary($1, Binary::Operation::shiftRightLogical, $3);
    }
    | expression OP_SRA expression {
        $$ = new Binary($1, Binary::Operation::shiftRightArithmetic, $3);
    }
    | '-' expression %prec UNARY {
        $$ = new Unary(Unary::Operation::negate, $2);
    }
    | '&' expression %prec UNARY {
        $$ = new Unary(Unary::Operation::allAnd, $2);
    }
    | '|' expression %prec UNARY {
        $$ = new Unary(Unary::Operation::allOr, $2);
    }
    | '~' expression %prec UNARY {
        $$ = new Unary(Unary::Operation::bitwiseNot, $2);
    }
    | '[' concatenation ']' {
        $$ = $2;
    }
    | '(' expression ')' {
        $$ = $2;
    }
    | '$' expression '(' procedural_call ')' {
        $$ = new ProceduralCall($2, (Argument*)$4);
    }
    | mux {
        $$ = $1;
    }
    | FW_NUMERIC {
        $$ = new Literal($1, true);
    }
    | NUMERIC  {
        $$ = new Literal($1, false);
    }
    | lhexpression {
        $$ = $1;
    }
    ;

range:
    expression OP_RANGE expression {
        $$ = new Range($1, $3);
    }
    ;

concatenation:
    concatenatable ',' concatenation {
        $$ = new Concatenation($1, $3);
    }
    | concatenatable {
        $$ = $1;
    }
    ;
concatenatable:
    expression {
        $$ = $1;
    }
    | expression LEFT_REPEAT_CAT concatenation ']' ']' {
        $$ = new RepeatConcatenation($1, $3);
    }
    ;

mux:
    KEYWORD_MUX expression mux_block {
        $$ = epsilon;
    }
    | KEYWORD_MUX FW_SPECIAL mux_block {
        $$ = epsilon;
    }
    ;

procedural_call:
    { $$ = epsilon; }
    | procedural_call_list {
        $$ = $1;
    }
    ;
procedural_call_list:
    expression ',' procedural_call_list {
        auto node = new ExpressionArgument($1);
        node->right = $3;
        $$ = node;
    }
    | STRING ',' procedural_call_list {
        auto node = new StringArgument($1);
        node->right = $3;
        $$ = node;
    }
    | expression {
        $$ = new ExpressionArgument($1);
    }
    | STRING {
        $$ = new StringArgument($1);
    }
    ;

mux_block:
    '(' labeled_expression_list ')' {
        $$ = $2;
    }
    ;
labeled_expression_list:
    { $$ = epsilon; }
    | expression ':' expression ',' labeled_expression_list {
        auto node = new ExpressionPair($1, $3);
        node->right = $5;
        $$ = node;
    }
    | expression ':' expression {
        $$ = new ExpressionPair($1, $3);;
    }
    | KEYWORD_DEFAULT ':' expression {
        $$ = new ExpressionPair(nullptr, $3);;
    }
    ;

%%

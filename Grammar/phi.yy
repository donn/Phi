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

%right '.' '['
%left OP_RANGE
%left OP_EQ OP_NEQ OP_GTE OP_LTE '<' '>' OP_UNSIGNED_LT OP_UNSIGNED_LTE OP_UNSIGNED_GT OP_UNSIGNED_GTE
%left OP_UNSIGNED_ADD OP_UNSIGNED_SUB '+' '-' '|' '&' '^'
%left '*' '/' '%'
%left OP_SRL OP_SRA OP_SLL
%left '~' UNARY

%type<text> NUMERIC FW_NUMERIC FW_SPECIAL IDENTIFIER ANNOTATION STRING optional_annotation

%type<bin> port_polarity

%type<vldt> dynamic_width

%type<node> description declaration port_declaration_list populated_port_declaration_list template_declaration template_declaration_list statement block_based if else labeled_statement_list block statement_list subdeclaration  optional_bus_declaration  optional_ports declaration_list optional_template template_list ports port_list nondeclarative_statement range mux_block labeled_expression_list   procedural_call procedural_call_list special_number identifier

%type<lhexpr> lhexpression lhconcatenation
%type<expr> expression inheritance inheritance_list optional_template_assignment optional_array_declaration optional_assignment concatenation concatenatable mux

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
    | KEYWORD_NAMESPACE identifier '{' description '}' description {
        auto node = new TopLevelNamespace(static_cast<Identifier*>($2), $4);
        node->right = $6;
        $$ = node;
        context->head = node;
    }
    ;

optional_semicolon:
    | ';'
    ;

declaration:
    KEYWORD_MODULE identifier template_declaration '(' port_declaration_list ')' inheritance block optional_semicolon {
        $$ = new TopLevelDeclaration(static_cast<Identifier*>($2), TopLevelDeclaration::Type::module, static_cast<Port*>($5), $7, static_cast<Statement*>($8));
    }
    | KEYWORD_MODULE error '}' {
        $$ = new ErrorNode();
    }
    | KEYWORD_INTERFACE identifier template_declaration '(' port_declaration_list ')' inheritance optional_semicolon {
        $$ = new TopLevelDeclaration(static_cast<Identifier*>($2), TopLevelDeclaration::Type::interface, static_cast<Port*>($5), $7);
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
    identifier ':' optional_annotation port_polarity optional_bus_declaration ',' populated_port_declaration_list {
        auto node = new Port(static_cast<Identifier*>($1), $4, static_cast<Range*>($5), $3);
        node->right = $7;
        $$ = node;
    }
    | identifier ':' optional_annotation port_polarity optional_bus_declaration {
        $$ = new Port(static_cast<Identifier*>($1), $4, static_cast<Range*>($5), $3);
        $$->right = nullptr;
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
        context->addError(nullopt, "phi.parametersUnsupported"); // UNSUPPORTED
        $$ = $2;
    }
    ;
template_declaration_list:
    identifier optional_template_assignment template_declaration_list {
        auto node = new TemplateDeclaration(static_cast<Identifier*>($1), $2);
        node->right = $3;
        $$ = node;
    }
    | identifier optional_template_assignment {
        $$ = new TemplateDeclaration(static_cast<Identifier*>($1), $2);
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
    optional_annotation subdeclaration optional_semicolon {
        auto node = static_cast<Statement*>($2);
        if ($1) {
            node->annotation = $1;
        }
        $$ = node;
    }
    | optional_annotation nondeclarative_statement optional_semicolon {
        auto node = static_cast<Statement*>($2);
        if ($1) {
            node->annotation = $1;
        }
        $$ = node;
    }
    | optional_annotation block_based {
        auto node = static_cast<Statement*>($2);
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
    | KEYWORD_FOR identifier KEYWORD_IN range block {
        $$ = new ForLoop(static_cast<Statement*>($5), static_cast<Range*>($4), static_cast<Identifier*>($2));
    }
    | KEYWORD_NAMESPACE identifier block {
        $$ = new Namespace(static_cast<Statement*>($3), static_cast<Identifier*>($2));
    }
    | KEYWORD_SWITCH expression '{' labeled_statement_list '}' {
        $$ = new Switch($2, static_cast<LabeledStatementList*>($4));
    }
    | KEYWORD_COMB block {
        $$ = new Combinational(static_cast<Statement*>($2));
    }
    | error '}' {
        $$ = new ErrorNode();
    }
    ;
if:
    KEYWORD_IF expression block else {
        $$ = new If(static_cast<Statement*>($3), $2, static_cast<If*>($4));
    }
    ;

else:
    { $$ = epsilon; }
    | KEYWORD_ELSE if {
        $$ = $2;
    }
    | KEYWORD_ELSE block {
        $$ = new If(static_cast<Statement*>($2), nullptr, nullptr);
    }
    ;

labeled_statement_list:
    { $$ = epsilon; }
    | KEYWORD_CASE expression ':' statement_list labeled_statement_list {
        auto node = new LabeledStatementList(false, static_cast<Expression*>($2), nullptr, static_cast<Statement*>($4));
        node->right = $5;
        $$ = node;
    }
    | KEYWORD_CASE special_number ':' statement_list labeled_statement_list {
        auto node = new LabeledStatementList(false, nullptr, static_cast<SpecialNumber*>($2), static_cast<Statement*>($4));
        node->right = $5;
        $$ = node;
    }
    | KEYWORD_DEFAULT ':' statement_list {
        $$ = new LabeledStatementList(false, nullptr, nullptr, static_cast<Statement*>($3));;
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
        $$ = new VariableLengthDeclaration($1, static_cast<Range*>($2), static_cast<DeclarationListItem*>($3));
    }
    | lhexpression optional_template identifier optional_array_declaration optional_ports {
        $$ = new InstanceDeclaration(static_cast<Identifier*>($3), $1, static_cast<ExpressionIDPair*>($2), $4, static_cast<ExpressionIDPair*>($5));
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
    identifier optional_array_declaration optional_assignment ',' declaration_list {
        auto node = new DeclarationListItem(static_cast<Identifier*>($1), $2, $3);
        node->right = $5;
        $$ = node;
    }
    | identifier optional_array_declaration optional_assignment {
        $$ = new DeclarationListItem(static_cast<Identifier*>($1), $2, $3);
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
    | identifier ':' '(' expression ')' ',' template_list {
        auto node = new ExpressionIDPair(static_cast<Identifier*>($1), $4);
        node->right = $7;
        $$ = node;
    }
    | identifier ':' '(' expression ')' {
        $$ = new ExpressionIDPair(static_cast<Identifier*>($1), $4);
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
    identifier ':' expression ',' port_list {
        auto node = new ExpressionIDPair(static_cast<Identifier*>($1), $3);
        node->right = $5;
        $$ = node;
    }
    | identifier ':' expression {
        $$ = new ExpressionIDPair(static_cast<Identifier*>($1), $3);
    }
    ;

/* Nondeclarative Statements */
    
nondeclarative_statement:
    lhexpression '=' expression {
        $$ = new NondeclarativeAssignment($1, $3);
    }
    | '{' lhconcatenation '}' '=' expression {
        $$ = new NondeclarativeAssignment($2, $5); 
    }
    | lhexpression ports {
        $$ = new NondeclarativePorts($1, static_cast<ExpressionIDPair*>($2));
    }
    ;

/* Expressions */
lhexpression:
    identifier {
        $$ = new IdentifierExpression(static_cast<Identifier*>($1));
    } 
    | lhexpression '.' lhexpression {
        $$ = new PropertyAccess($1, $3);
    }
    | lhexpression '[' range ']' {
        $$ = new RangeAccess($1, static_cast<Range*>($3));
    }
    | lhexpression '[' expression ']' {
        $$ = new ArrayAccess($1, $3);
    }
    ;

lhconcatenation:
    lhexpression ',' lhconcatenation {
        $$ = new LHConcatenation($1, $3);
    }
    | lhexpression {
        $$ = $1;
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
    | '{' concatenation '}' {
        $$ = $2;
    }
    | '(' expression ')' {
        $$ = $2;
    }
    | '$' lhexpression '(' procedural_call ')' {
        $$ = new ProceduralCall($2, (Node::Argument*)$4);
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
    | expression LEFT_REPEAT_CAT concatenation '}' '}' {
        $$ = new RepeatConcatenation($1, $3);
    }
    ;

mux:
    KEYWORD_MUX expression mux_block {
        $$ = new Multiplexer($2, static_cast<ExpressionPair*>($3));
    }
    | KEYWORD_MUX special_number mux_block {
        $$ = new Multiplexer($2, static_cast<ExpressionPair*>($3));
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
        auto node = new ExpressionPair($1, nullptr, $3);
        node->right = $5;
        $$ = node;
    }
    | special_number ':' expression ',' labeled_expression_list {
        auto node = new ExpressionPair(nullptr, static_cast<SpecialNumber*>($1), $3);
        node->right = $5;
        $$ = node;
    }
    | expression ':' expression {
        $$ = new ExpressionPair($1, nullptr, $3);
    }
    | special_number ':' expression {
        $$ = new ExpressionPair(nullptr, static_cast<SpecialNumber*>($1), $3);
    }
    | KEYWORD_DEFAULT ':' expression {
        $$ = new ExpressionPair(nullptr, nullptr, $3);
    }
    ;

identifier:
    IDENTIFIER {
        $$ = new Identifier($1);
    }
    ;

%%

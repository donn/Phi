%language "C++"
%locations

%define parser_class_name {Parser}
%define api.namespace {Phi}

%define api.value.type {std::shared_ptr<Node::Node>}

%code requires {
    namespace Phi {
        class Lexer;
    }
    #include <memory>
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
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($2);
        $$ = node;
        context->head = node;
    }
    | KEYWORD_NAMESPACE identifier '{' description '}' description {
        auto node = std::make_shared<TopLevelNamespace>(@$, std::static_pointer_cast<Identifier>($2), $4);
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($6);
        $$ = node;
        context->head = node;
    }
    ;

optional_semicolon:
    | ';'
    ;

declaration:
    KEYWORD_MODULE identifier template_declaration '(' port_declaration_list ')' inheritance block optional_semicolon {
        $$ = std::make_shared<TopLevelDeclaration>(@$, std::static_pointer_cast<Identifier>($2), TopLevelDeclaration::Type::module, std::static_pointer_cast<Port>($5), std::static_pointer_cast<InheritanceListItem>($7), std::static_pointer_cast<Statement>($8));
    }
    | KEYWORD_MODULE error '}' {
        $$ = std::make_shared<ErrorNode>(@$);
    }
    | KEYWORD_INTERFACE identifier template_declaration '(' port_declaration_list ')' inheritance optional_semicolon {
        $$ = std::make_shared<TopLevelDeclaration>(@$, std::static_pointer_cast<Identifier>($2), TopLevelDeclaration::Type::interface, std::static_pointer_cast<Port>($5), std::static_pointer_cast<InheritanceListItem>($7));
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
        auto cast = std::static_pointer_cast<Token>($3);
        optional<std::string> annotation;
        if (cast) {
            annotation = cast->text;
        }

        auto polarityCast = std::static_pointer_cast<Token>($4);
        auto polarity = polarityCast->text[0] == 'O';

        auto node = std::make_shared<Port>(@$, std::static_pointer_cast<Identifier>($1), polarity, std::static_pointer_cast<Range>($5), annotation);
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($7);
        $$ = node;
    }
    | identifier ':' optional_annotation port_polarity optional_bus_declaration {
        auto cast = std::static_pointer_cast<Token>($3);
        optional<std::string> annotation;
        if (cast) {
            annotation = cast->text;
        }

        auto polarityCast = std::static_pointer_cast<Token>($4);
        auto polarity = polarityCast->text[0] == 'O';

        $$ = std::make_shared<Port>(@$, std::static_pointer_cast<Identifier>($1), polarity, std::static_pointer_cast<Range>($5), annotation);
    }
    ;
port_polarity:
    KEYWORD_INPUT {
        $$ = std::make_shared<Token>(@$, "I");
    }
    | KEYWORD_OUTPUT {
        $$ = std::make_shared<Token>(@$, "O");
    }
    ;

/* Templating */
template_declaration:
    { $$ = epsilon; }
    | '<' template_declaration_list '>' {
        context->addError(@$, "phi.parametersUnsupported"); // UNSUPPORTED
        $$ = $2;
    }
    ;
template_declaration_list:
    identifier optional_template_assignment template_declaration_list {
        auto node = std::make_shared<TemplateDeclaration>(@$, std::static_pointer_cast<Identifier>($1), std::static_pointer_cast<Expression>($2));
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($3);
        $$ = node;
    }
    | identifier optional_template_assignment {
        $$ = std::make_shared<TemplateDeclaration>(@$, std::static_pointer_cast<Identifier>($1), std::static_pointer_cast<Expression>($2));
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
    inheritance_list_item ',' inheritance_list {
        auto node = $1;
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($3);
        $$ = node;
    }
    | inheritance_list_item {
        $$ = $1;
    }
    ;
inheritance_list_item:
    lhexpression {
        $$ = std::make_shared<InheritanceListItem>(@$, std::static_pointer_cast<LHExpression>($1));
    }
    ;

/* Statements */
statement:
    optional_annotation subdeclaration optional_semicolon {
        auto node = std::static_pointer_cast<Statement>($2);
        if ($1) {
            auto cast = std::static_pointer_cast<Token>($1);
            node->annotation = cast->text;
        }
        $$ = node;
    }
    | optional_annotation nondeclarative_statement optional_semicolon {
        auto node = std::static_pointer_cast<Statement>($2);
        if ($1) {
                auto cast = std::static_pointer_cast<Token>($1);
            node->annotation = cast->text;
        }
        $$ = node;
    }
    | optional_annotation block_based {
        auto node = std::static_pointer_cast<Statement>($2);
        if ($1) {
            auto cast = std::static_pointer_cast<Token>($1);
            node->annotation = cast->text;
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
        auto cast = std::static_pointer_cast<Token>($1);
        try {
            $$ = std::make_shared<SpecialNumber>(@$, cast->text);
        } catch (const char* error) {
            context->addError(@1, error);
        }
    }
    | '(' FW_SPECIAL ')' {
        auto cast = std::static_pointer_cast<Token>($2);
        try {
            $$ = std::make_shared<SpecialNumber>(@$, cast->text);
        } catch (const char* error) {
            context->addError(@2, error);
        }
    }
    ;
block_based:
    if {
        $$ = $1;
    }
    | KEYWORD_FOR identifier KEYWORD_IN range block {
        $$ = std::make_shared<ForLoop>(@$, std::static_pointer_cast<Statement>($5), std::static_pointer_cast<Range>($4), std::static_pointer_cast<Identifier>($2));
    }
    | KEYWORD_NAMESPACE identifier block {
        $$ = std::make_shared<Namespace>(@$, std::static_pointer_cast<Statement>($3), std::static_pointer_cast<Identifier>($2));
    }
    | KEYWORD_SWITCH expression '{' labeled_statement_list '}' {
        $$ = std::make_shared<Switch>(@$, std::static_pointer_cast<Expression>($2), std::static_pointer_cast<LabeledStatementList>($4));
    }
    | KEYWORD_COMB block {
        $$ = std::make_shared<Combinational>(@$, std::static_pointer_cast<Statement>($2));
    }
    | error '}' {
        $$ = std::make_shared<ErrorNode>(@$);
    }
    ;
if:
    KEYWORD_IF expression block else {
        $$ = std::make_shared<If>(@$, std::static_pointer_cast<Statement>($3), std::static_pointer_cast<Expression>($2), std::static_pointer_cast<If>($4));
    }
    ;

else:
    { $$ = epsilon; }
    | KEYWORD_ELSE if {
        $$ = $2;
    }
    | KEYWORD_ELSE block {
        $$ = std::make_shared<If>(@$, std::static_pointer_cast<Statement>($2), nullptr, nullptr);
    }
    ;

labeled_statement_list:
    { $$ = epsilon; }
    | KEYWORD_CASE expression ':' statement_list labeled_statement_list {
        auto node = std::make_shared<LabeledStatementList>(@$, false, std::static_pointer_cast<Expression>($2), nullptr, std::static_pointer_cast<Statement>($4));
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($5);
        $$ = node;
    }
    | KEYWORD_CASE special_number ':' statement_list labeled_statement_list {
        auto node = std::make_shared<LabeledStatementList>(@$, false, nullptr, std::static_pointer_cast<SpecialNumber>($2), std::static_pointer_cast<Statement>($4));
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($5);
        $$ = node;
    }
    | KEYWORD_DEFAULT ':' statement_list {
        $$ = std::make_shared<LabeledStatementList>(@$, false, nullptr, nullptr, std::static_pointer_cast<Statement>($3));;
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
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($2);
        $$ = $1;
    }
    ;

/* Subdeclarations */
subdeclaration:
    dynamic_width optional_bus_declaration declaration_list {
        using VLDT = VariableLengthDeclaration::Type;
        auto cast = std::static_pointer_cast<Token>($1);
        VLDT type;
        switch (cast->text[0]) {
            case 'W':
                type = VLDT::wire;
                break;
            case 'R':
                type = VLDT::reg;
                break;
            case 'L':
                type = VLDT::latch;
                break;
            case 'V':
            default:
                type = VLDT::var;
        }
        $$ = std::make_shared<VariableLengthDeclaration>(@$, type, std::static_pointer_cast<Range>($2), std::static_pointer_cast<DeclarationListItem>($3));
    }
    | lhexpression optional_template identifier optional_array_declaration optional_ports {
        $$ = std::make_shared<InstanceDeclaration>(@$, std::static_pointer_cast<Identifier>($3), std::static_pointer_cast<LHExpression>($1), std::static_pointer_cast<ExpressionIDPair>($2), std::static_pointer_cast<Expression>($4), std::static_pointer_cast<ExpressionIDPair>($5));
    }
    ;

dynamic_width:
    KEYWORD_SW_VAR {
        $$ = std::make_shared<Token>(@$, "V");
    }
    | KEYWORD_WIRE {
        $$ = std::make_shared<Token>(@$, "W");
    }
    | KEYWORD_REGISTER {
        $$ = std::make_shared<Token>(@$, "R");
    }
    | KEYWORD_LATCH {
        $$ = std::make_shared<Token>(@$, "L");
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
        auto node = std::make_shared<DeclarationListItem>(@$, std::static_pointer_cast<Identifier>($1), std::static_pointer_cast<Expression>($2), std::static_pointer_cast<Expression>($3));
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($5);
        $$ = node;
    }
    | identifier optional_array_declaration optional_assignment {
        $$ = std::make_shared<DeclarationListItem>(@$, std::static_pointer_cast<Identifier>($1), std::static_pointer_cast<Expression>($2), std::static_pointer_cast<Expression>($3));
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
        auto node = std::make_shared<ExpressionIDPair>(@$, std::static_pointer_cast<Identifier>($1), std::static_pointer_cast<Expression>($4));
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($7);
        $$ = node;
    }
    | identifier ':' '(' expression ')' {
        $$ = std::make_shared<ExpressionIDPair>(@$, std::static_pointer_cast<Identifier>($1), std::static_pointer_cast<Expression>($4));
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
        auto node = std::make_shared<ExpressionIDPair>(@$, std::static_pointer_cast<Identifier>($1), std::static_pointer_cast<Expression>($3));
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($5);
        $$ = node;
    }
    | identifier ':' expression {
        $$ = std::make_shared<ExpressionIDPair>(@$, std::static_pointer_cast<Identifier>($1), std::static_pointer_cast<Expression>($3));
    }
    ;

/* Nondeclarative Statements */
    
nondeclarative_statement:
    lhexpression '=' expression {
        $$ = std::make_shared<NondeclarativeAssignment>(@$, std::static_pointer_cast<LHExpression>($1), std::static_pointer_cast<Expression>($3));
    }
    | '{' lhconcatenation '}' '=' expression {
        $$ = std::make_shared<NondeclarativeAssignment>(@$, std::static_pointer_cast<LHExpression>($2), std::static_pointer_cast<Expression>($5)); 
    }
    | lhexpression ports {
        $$ = std::make_shared<NondeclarativePorts>(@$, std::static_pointer_cast<LHExpression>($1), std::static_pointer_cast<ExpressionIDPair>($2));
    }
    ;

/* Expressions */
lhexpression:
    identifier {
        $$ = std::make_shared<IdentifierExpression>(@$, std::static_pointer_cast<Identifier>($1));
    } 
    | lhexpression '.' lhexpression {
        $$ = std::make_shared<PropertyAccess>(@$, std::static_pointer_cast<LHExpression>($1), std::static_pointer_cast<LHExpression>($3));
    }
    | lhexpression '[' range ']' {
        $$ = std::make_shared<RangeAccess>(@$, std::static_pointer_cast<LHExpression>($1), std::static_pointer_cast<Range>($3));
    }
    | lhexpression '[' expression ']' {
        $$ = std::make_shared<ArrayAccess>(@$, std::static_pointer_cast<LHExpression>($1), std::static_pointer_cast<Expression>($3));
    }
    ;

lhconcatenation:
    lhexpression ',' lhconcatenation {
        $$ = std::make_shared<LHConcatenation>(@$, std::static_pointer_cast<LHExpression>($1), std::static_pointer_cast<LHExpression>($3));
    }
    | lhexpression {
        $$ = $1;
    }
    ;

expression:
    expression OP_EQ expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::equal, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_EQ special_number {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::equal, std::static_pointer_cast<SpecialNumber>($3));
    }
    | expression OP_NEQ expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::notEqual, std::static_pointer_cast<Expression>($3));
    }
    | expression '>' expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::greaterThan, std::static_pointer_cast<Expression>($3));
    }
    | expression '<' expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::lessThan, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_GTE expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::greaterThanOrEqual, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_LTE expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::lessThanOrEqual, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_UNSIGNED_LT expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::unsignedLessThan, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_UNSIGNED_GT  expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::unsignedGreaterThan, std::static_pointer_cast<Expression>($3));
    } 
    | expression OP_UNSIGNED_LTE expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::unsignedLessThanOrEqual, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_UNSIGNED_GTE expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::unsignedGreaterThanOrEqual, std::static_pointer_cast<Expression>($3));
    }
    | expression '+' expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::plus, std::static_pointer_cast<Expression>($3));
    }
    | expression '-' expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::minus, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_UNSIGNED_ADD expression{
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::unsignedPlus, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_UNSIGNED_SUB expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::unsignedMinus, std::static_pointer_cast<Expression>($3));
    }
    | expression '|' expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::bitwiseOr, std::static_pointer_cast<Expression>($3));
    }
    | expression '&' expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::bitwiseAnd, std::static_pointer_cast<Expression>($3));
    }
    | expression '^' expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::bitwiseXor, std::static_pointer_cast<Expression>($3));
    }
    | expression '*' expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::mul, std::static_pointer_cast<Expression>($3));
    }
    | expression '/' expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::div, std::static_pointer_cast<Expression>($3));
    }
    | expression '%' expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::modulo, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_SLL expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::shiftLeftLogical, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_SRL expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::shiftRightLogical, std::static_pointer_cast<Expression>($3));
    }
    | expression OP_SRA expression {
        $$ = std::make_shared<Binary>(@$, std::static_pointer_cast<Expression>($1), Binary::Operation::shiftRightArithmetic, std::static_pointer_cast<Expression>($3));
    }
    | '-' expression %prec UNARY {
        $$ = std::make_shared<Unary>(@$, Unary::Operation::negate, std::static_pointer_cast<Expression>($2));
    }
    | '&' expression %prec UNARY {
        $$ = std::make_shared<Unary>(@$, Unary::Operation::allAnd, std::static_pointer_cast<Expression>($2));
    }
    | '|' expression %prec UNARY {
        $$ = std::make_shared<Unary>(@$, Unary::Operation::allOr, std::static_pointer_cast<Expression>($2));
    }
    | '~' expression %prec UNARY {
        $$ = std::make_shared<Unary>(@$, Unary::Operation::bitwiseNot, std::static_pointer_cast<Expression>($2));
    }
    | '{' concatenation '}' {
        $$ = $2;
    }
    | '(' expression ')' {
        $$ = $2;
    }
    | '$' lhexpression '(' procedural_call ')' {
        $$ = std::make_shared<ProceduralCall>(@$, std::static_pointer_cast<LHExpression>($2), std::static_pointer_cast<Phi::Node::Argument>($4));
    }
    | mux {
        $$ = $1;
    }
    | FW_NUMERIC {
        auto cast = std::static_pointer_cast<Token>($1);
        try {
            $$ = std::make_shared<Literal>(@$, cast->text, true);
        } catch (const char *error) {
            context->addError(@1, error);
        }
    }
    | NUMERIC  {
        auto cast = std::static_pointer_cast<Token>($1);
        $$ = std::make_shared<Literal>(@$, cast->text, false);
    }
    | lhexpression {
        $$ = std::make_shared<LHExpressionEncapsulator>(@$, std::static_pointer_cast<LHExpression>($1));
    }
    ;

range:
    expression OP_RANGE expression {
        $$ = std::make_shared<Range>(@$, std::static_pointer_cast<Expression>($1), std::static_pointer_cast<Expression>($3));
    }
    ;

concatenation:
    concatenatable ',' concatenation {
        $$ = std::make_shared<Concatenation>(@$, std::static_pointer_cast<Expression>($1), std::static_pointer_cast<Expression>($3));
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
        $$ = std::make_shared<RepeatConcatenation>(@$, std::static_pointer_cast<Expression>($1), std::static_pointer_cast<Expression>($3));
    }
    ;

mux:
    KEYWORD_MUX expression mux_block {
        $$ = std::make_shared<Multiplexer>(@$, std::static_pointer_cast<Expression>($2), std::static_pointer_cast<ExpressionPair>($3));
    }
    | KEYWORD_MUX special_number mux_block {
        $$ = std::make_shared<Multiplexer>(@$, std::static_pointer_cast<Expression>($2), std::static_pointer_cast<ExpressionPair>($3));
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
        auto node = std::make_shared<ExpressionArgument>(@$, std::static_pointer_cast<Expression>($1));
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($3);
        $$ = node;
    }
    | STRING ',' procedural_call_list {
        auto cast = std::static_pointer_cast<Token>($1);
        auto node = std::make_shared<StringArgument>(@$, cast->text);
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($3);
        $$ = node;
    }
    | expression {
        $$ = std::make_shared<ExpressionArgument>(@$, std::static_pointer_cast<Expression>($1));
    }
    | STRING {
        auto cast = std::static_pointer_cast<Token>($1);
        $$ = std::make_shared<StringArgument>(@$, cast->text);
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
        auto node = std::make_shared<ExpressionPair>(@$, std::static_pointer_cast<Expression>($1), nullptr, std::static_pointer_cast<Expression>($3));
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($5);
        $$ = node;
    }
    | special_number ':' expression ',' labeled_expression_list {
        auto node = std::make_shared<ExpressionPair>(@$, nullptr, std::static_pointer_cast<SpecialNumber>($1), std::static_pointer_cast<Expression>($3));
        auto ns = std::static_pointer_cast<Statement>(node);
        ns->next = std::static_pointer_cast<Statement>($5);
        $$ = node;
    }
    | expression ':' expression {
        $$ = std::make_shared<ExpressionPair>(@$, std::static_pointer_cast<Expression>($1), nullptr, std::static_pointer_cast<Expression>($3));
    }
    | special_number ':' expression {
        $$ = std::make_shared<ExpressionPair>(@$, nullptr, std::static_pointer_cast<SpecialNumber>($1), std::static_pointer_cast<Expression>($3));
    }
    | KEYWORD_DEFAULT ':' expression {
        $$ = std::make_shared<ExpressionPair>(@$, nullptr, nullptr, std::static_pointer_cast<Expression>($3));
    }
    ;

identifier:
    IDENTIFIER {
        auto cast = std::static_pointer_cast<Token>($1);
        $$ = std::make_shared<Identifier>(@$, cast->text);
    }
    ;

%%

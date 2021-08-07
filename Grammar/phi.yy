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

    #define mk std::make_shared
    #define c std::static_pointer_cast
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
%token KEYWORD_DEC
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

%left OP_RANGE
%left OP_EQ OP_NEQ OP_GTE OP_LTE OP_UNSIGNED_LT OP_UNSIGNED_LTE OP_UNSIGNED_GT OP_UNSIGNED_GTE '<' '>'
%left OP_UNSIGNED_ADD OP_UNSIGNED_SUB '+' '-' '|' '&' '^'
%left '*' '/' '%'
%left OP_SRL OP_SRA OP_SLL
%left UNARY

%right '=' ':' 
%right '.' '['

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
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($2);
        $$ = node;
        context->head = node;
    }
    | KEYWORD_NAMESPACE identifier '{' description '}' description {
        auto node = mk<TopLevelNamespace>(@$, c<Identifier>($2), $4);
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($6);
        $$ = node;
        context->head = node;
    }
    ;

optional_semicolon:
    | ';'
    ;

declaration:
    KEYWORD_MODULE identifier template_declaration '(' port_declaration_list ')' inheritance block optional_semicolon {
        $$ = mk<TopLevelDeclaration>(@$, c<Identifier>($2), TopLevelDeclaration::Type::module, c<Port>($5), c<InheritanceListItem>($7), c<Statement>($8));
    }
    | KEYWORD_MODULE error '}' {
        $$ = mk<ErrorNode>(@$);
    }
    | KEYWORD_INTERFACE identifier template_declaration '(' port_declaration_list ')' inheritance optional_semicolon {
        $$ = mk<TopLevelDeclaration>(@$, c<Identifier>($2), TopLevelDeclaration::Type::interface, c<Port>($5), c<InheritanceListItem>($7));
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
        auto cast = c<Token>($3);
        optional<std::string> annotation;
        if (cast) {
            annotation = cast->text;
        }

        auto polarityCast = c<Token>($4);
        auto polarity = polarityCast->text[0] == 'O';

        auto node = mk<Port>(@$, c<Identifier>($1), polarity, c<Range>($5), annotation);
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($7);
        $$ = node;
    }
    | identifier ':' optional_annotation port_polarity optional_bus_declaration {
        auto cast = c<Token>($3);
        optional<std::string> annotation;
        if (cast) {
            annotation = cast->text;
        }

        auto polarityCast = c<Token>($4);
        auto polarity = polarityCast->text[0] == 'O';

        $$ = mk<Port>(@$, c<Identifier>($1), polarity, c<Range>($5), annotation);
    }
    ;
port_polarity:
    KEYWORD_INPUT {
        $$ = mk<Token>(@$, "I");
    }
    | KEYWORD_OUTPUT {
        $$ = mk<Token>(@$, "O");
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
        auto node = mk<TemplateDeclaration>(@$, c<Identifier>($1), c<Expression>($2));
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($3);
        $$ = node;
    }
    | identifier optional_template_assignment {
        $$ = mk<TemplateDeclaration>(@$, c<Identifier>($1), c<Expression>($2));
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
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($3);
        $$ = node;
    }
    | inheritance_list_item {
        $$ = $1;
    }
    ;
inheritance_list_item:
    lhexpression {
        $$ = mk<InheritanceListItem>(@$, c<LHExpression>($1));
    }
    ;

/* Statements */
statement:
    optional_annotation subdeclaration optional_semicolon {
        auto node = c<Statement>($2);
        if ($1) {
            auto cast = c<Token>($1);
            node->annotation = cast->text;
        }
        $$ = node;
    }
    | optional_annotation nondeclarative_statement optional_semicolon {
        auto node = c<Statement>($2);
        if ($1) {
                auto cast = c<Token>($1);
            node->annotation = cast->text;
        }
        $$ = node;
    }
    | optional_annotation block_based {
        auto node = c<Statement>($2);
        if ($1) {
            auto cast = c<Token>($1);
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
        auto cast = c<Token>($1);
        try {
            $$ = mk<SpecialNumber>(@$, cast->text);
        } catch (const char* error) {
            context->addError(@1, error);
        }
    }
    | '(' FW_SPECIAL ')' {
        auto cast = c<Token>($2);
        try {
            $$ = mk<SpecialNumber>(@$, cast->text);
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
        $$ = mk<ForLoop>(@$, c<Statement>($5), c<Range>($4), c<Identifier>($2));
    }
    | KEYWORD_NAMESPACE identifier block {
        $$ = mk<Namespace>(@$, c<Statement>($3), c<Identifier>($2));
    }
    | KEYWORD_SWITCH expression '{' labeled_statement_list '}' {
        $$ = mk<Switch>(@$, c<Expression>($2), c<LabeledStatementList>($4));
    }
    | KEYWORD_COMB block {
        $$ = mk<Combinational>(@$, c<Statement>($2));
    }
    | error '}' {
        $$ = mk<ErrorNode>(@$);
    }
    ;
if:
    KEYWORD_IF expression block else {
        $$ = mk<If>(@$, c<Statement>($3), c<Expression>($2), c<If>($4));
    }
    ;

else:
    { $$ = epsilon; }
    | KEYWORD_ELSE if {
        $$ = $2;
    }
    | KEYWORD_ELSE block {
        $$ = mk<If>(@$, c<Statement>($2), nullptr, nullptr);
    }
    ;

labeled_statement_list:
    { $$ = epsilon; }
    | KEYWORD_CASE expression ':' statement_list labeled_statement_list {
        auto node = mk<LabeledStatementList>(@$, false, c<Expression>($2), nullptr, c<Statement>($4));
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($5);
        $$ = node;
    }
    | KEYWORD_CASE special_number ':' statement_list labeled_statement_list {
        auto node = mk<LabeledStatementList>(@$, false, nullptr, c<SpecialNumber>($2), c<Statement>($4));
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($5);
        $$ = node;
    }
    | KEYWORD_DEFAULT ':' statement_list {
        $$ = mk<LabeledStatementList>(@$, false, nullptr, nullptr, c<Statement>($3));;
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
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($2);
        $$ = $1;
    }
    ;

/* Subdeclarations */
subdeclaration:
    dynamic_width optional_bus_declaration declaration_list {
        using VLDT = VariableLengthDeclaration::Type;
        auto cast = c<Token>($1);
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
        $$ = mk<VariableLengthDeclaration>(@$, type, c<Range>($2), c<DeclarationListItem>($3));
    }
    | lhexpression optional_template identifier optional_array_declaration optional_ports {
        $$ = mk<InstanceDeclaration>(@$, c<Identifier>($3), c<LHExpression>($1), c<ExpressionIDPair>($2), c<Expression>($4), c<ExpressionIDPair>($5));
    }
    ;

dynamic_width:
    KEYWORD_SW_VAR {
        $$ = mk<Token>(@$, "V");
    }
    | KEYWORD_WIRE {
        $$ = mk<Token>(@$, "W");
    }
    | KEYWORD_REGISTER {
        $$ = mk<Token>(@$, "R");
    }
    | KEYWORD_LATCH {
        $$ = mk<Token>(@$, "L");
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
        auto node = mk<DeclarationListItem>(@$, c<Identifier>($1), c<Expression>($2), c<Expression>($3));
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($5);
        $$ = node;
    }
    | identifier optional_array_declaration optional_assignment {
        $$ = mk<DeclarationListItem>(@$, c<Identifier>($1), c<Expression>($2), c<Expression>($3));
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
        auto node = mk<ExpressionIDPair>(@$, c<Identifier>($1), c<Expression>($4));
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($7);
        $$ = node;
    }
    | identifier ':' '(' expression ')' {
        $$ = mk<ExpressionIDPair>(@$, c<Identifier>($1), c<Expression>($4));
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
        auto node = mk<ExpressionIDPair>(@$, c<Identifier>($1), c<Expression>($3));
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($5);
        $$ = node;
    }
    | identifier ':' expression {
        $$ = mk<ExpressionIDPair>(@$, c<Identifier>($1), c<Expression>($3));
    }
    ;

/* Nondeclarative Statements */
    
nondeclarative_statement:
    lhexpression '=' expression {
        auto lhx = c<LHExpression>($1);
        $$ = mk<NondeclarativeAssignment>(@$, mk<LHExpressionEncapsulator>(@1, lhx), c<Expression>($3));
    }
    | '{' lhconcatenation '}' '=' expression {
        auto lhc = c<LHConcatenation>($2);
        $$ = mk<NondeclarativeAssignment>(@$, mk<LHExpressionEncapsulator>(@2, lhc), c<Expression>($5)); 
    }
    | lhexpression ports {
        auto lhx = c<LHExpression>($1);
        $$ = mk<NondeclarativePorts>(@$, mk<LHExpressionEncapsulator>(lhx->location, lhx), c<ExpressionIDPair>($2));
    }
    ;

/* Expressions */
lhexpression:
    identifier {
        $$ = mk<IdentifierExpression>(@$, c<Identifier>($1));
    } 
    | lhexpression '.' lhexpression {
        $$ = mk<PropertyAccess>(@$, c<LHExpression>($1), c<LHExpression>($3));
    }
    | lhexpression '[' range ']' {
        $$ = mk<RangeAccess>(@$, c<LHExpression>($1), c<Range>($3));
    }
    | lhexpression '[' expression ']' {
        $$ = mk<IndexAccess>(@$, c<LHExpression>($1), c<Expression>($3));
    }
    ;

lhconcatenation:
    lhexpression ',' lhconcatenation {
        auto lhx = c<LHExpression>($1);
        $$ = mk<LHConcatenation>(@$, mk<LHExpressionEncapsulator>(@1, lhx), c<LHExpressionEncapsulator>($3));
    }
    | lhexpression {
        auto lhx = c<LHExpression>($1);
        $$ = mk<LHExpressionEncapsulator>(lhx->location, lhx);
    }
    ;

expression:
    expression OP_EQ expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::equal, c<Expression>($3));
    }
    | expression OP_EQ special_number {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::equal, c<SpecialNumber>($3));
    }
    | expression OP_NEQ expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::notEqual, c<Expression>($3));
    }
    | expression '>' expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::greaterThan, c<Expression>($3));
    }
    | expression '<' expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::lessThan, c<Expression>($3));
    }
    | expression OP_GTE expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::greaterThanOrEqual, c<Expression>($3));
    }
    | expression OP_LTE expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::lessThanOrEqual, c<Expression>($3));
    }
    | expression OP_UNSIGNED_LT expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::unsignedLessThan, c<Expression>($3));
    }
    | expression OP_UNSIGNED_GT  expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::unsignedGreaterThan, c<Expression>($3));
    } 
    | expression OP_UNSIGNED_LTE expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::unsignedLessThanOrEqual, c<Expression>($3));
    }
    | expression OP_UNSIGNED_GTE expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::unsignedGreaterThanOrEqual, c<Expression>($3));
    }
    | expression '+' expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::plus, c<Expression>($3));
    }
    | expression '-' expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::minus, c<Expression>($3));
    }
    | expression OP_UNSIGNED_ADD expression{
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::unsignedPlus, c<Expression>($3));
    }
    | expression OP_UNSIGNED_SUB expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::unsignedMinus, c<Expression>($3));
    }
    | expression '|' expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::bitwiseOr, c<Expression>($3));
    }
    | expression '&' expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::bitwiseAnd, c<Expression>($3));
    }
    | expression '^' expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::bitwiseXor, c<Expression>($3));
    }
    | expression '*' expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::mul, c<Expression>($3));
    }
    | expression '/' expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::div, c<Expression>($3));
    }
    | expression '%' expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::modulo, c<Expression>($3));
    }
    | expression OP_SLL expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::shiftLeftLogical, c<Expression>($3));
    }
    | expression OP_SRL expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::shiftRightLogical, c<Expression>($3));
    }
    | expression OP_SRA expression {
        $$ = mk<Binary>(@$, c<Expression>($1), Binary::Operation::shiftRightArithmetic, c<Expression>($3));
    }
    | '-' expression %prec UNARY {
        $$ = mk<Unary>(@$, Unary::Operation::negate, c<Expression>($2));
    }
    | '&' expression %prec UNARY {
        $$ = mk<Unary>(@$, Unary::Operation::allAnd, c<Expression>($2));
    }
    | '|' expression %prec UNARY {
        $$ = mk<Unary>(@$, Unary::Operation::allOr, c<Expression>($2));
    }
    | '~' expression %prec UNARY {
        $$ = mk<Unary>(@$, Unary::Operation::bitwiseNot, c<Expression>($2));
    }
    | KEYWORD_DEC expression %prec UNARY {
        $$ = mk<Decoder>(@$, c<Expression>($2));
    }
    | '{' concatenation '}' {
        $$ = $2;
    }
    | '(' expression ')' {
        $$ = $2;
    }
    | '$' lhexpression '(' procedural_call ')' {
        $$ = mk<ProceduralCall>(@$, c<LHExpression>($2), c<Phi::Node::Argument>($4));
    }
    | mux {
        $$ = $1;
    }
    | FW_NUMERIC {
        auto cast = c<Token>($1);
        try {
            $$ = mk<Literal>(@$, cast->text, true);
        } catch (const char *error) {
            context->addError(@1, error);
        }
    }
    | NUMERIC  {
        auto cast = c<Token>($1);
        $$ = mk<Literal>(@$, cast->text, false);
    }
    | lhexpression {
        auto lhxs = mk<LHExpressionEncapsulator>(@$, c<LHExpression>($1));
        $$ = mk<LHExpressionEvaluator>(@$, lhxs);
    }
    ;

range:
    expression OP_RANGE expression {
        $$ = mk<Range>(@$, c<Expression>($1), c<Expression>($3));
    }
    ;

concatenation:
    concatenatable ',' concatenation {
        $$ = mk<Concatenation>(@$, c<Expression>($1), c<Expression>($3));
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
        $$ = mk<RepeatConcatenation>(@$, c<Expression>($1), c<Expression>($3));
    }
    ;

mux:
    KEYWORD_MUX expression mux_block {
        $$ = mk<Multiplexer>(@$, $2, $3);
    }
    | KEYWORD_MUX expression lhexpression {
        // $$ = mk<Multiplexer>(@$, $2, $3)
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
        auto node = mk<ExpressionArgument>(@$, c<Expression>($1));
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($3);
        $$ = node;
    }
    | STRING ',' procedural_call_list {
        auto cast = c<Token>($1);
        auto node = mk<StringArgument>(@$, cast->text);
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($3);
        $$ = node;
    }
    | expression {
        $$ = mk<ExpressionArgument>(@$, c<Expression>($1));
    }
    | STRING {
        auto cast = c<Token>($1);
        $$ = mk<StringArgument>(@$, cast->text);
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
        auto node = mk<ExpressionPair>(@$, c<Expression>($1), nullptr, c<Expression>($3));
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($5);
        $$ = node;
    }
    | special_number ':' expression ',' labeled_expression_list {
        auto node = mk<ExpressionPair>(@$, nullptr, c<SpecialNumber>($1), c<Expression>($3));
        auto ns = c<Statement>(node);
        ns->next = c<Statement>($5);
        $$ = node;
    }
    | expression ':' expression {
        $$ = mk<ExpressionPair>(@$, c<Expression>($1), nullptr, c<Expression>($3));
    }
    | special_number ':' expression {
        $$ = mk<ExpressionPair>(@$, nullptr, c<SpecialNumber>($1), c<Expression>($3));
    }
    | KEYWORD_DEFAULT ':' expression {
        $$ = mk<ExpressionPair>(@$, nullptr, nullptr, c<Expression>($3));
    }
    ;

identifier:
    IDENTIFIER {
        auto cast = c<Token>($1);
        $$ = mk<Identifier>(@$, cast->text);
    }
    ;

%%

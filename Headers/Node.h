#ifndef _node_h
#define _node_h
#include "Context.h"
#include "Types.h"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>
#include <typeinfo>

#include <ostream>

// Elaboration Macros
#define MACRO_ELAB_PARAMS Phi::Context* context
#define MACRO_ELAB_SIG_IMP elaborate (MACRO_ELAB_PARAMS)
#define MACRO_ELAB_SIG_HDR virtual void MACRO_ELAB_SIG_IMP;

// Translation Macros
#define MACRO_TRANS_PARAMS std::ostream* stream, std::string namespaceSoFar, int* indent
#define MACRO_TRANS_SIG_IMP translate (MACRO_TRANS_PARAMS)
#define MACRO_TRANS_SIG_HDR virtual void MACRO_TRANS_SIG_IMP;

#define MACRO_INDENT std::string(*indent * 4, ' ')
#define MACRO_EOL std::endl << MACRO_INDENT

// Debug Macros
#if YYDEBUG
    #define MACRO_DEBUGLABEL_PARAMS
    #define MACRO_DEBUGLABEL_SIG_IMP debugLabel (MACRO_DEBUGLABEL_PARAMS)
    #define MACRO_DEBUGLABEL_SIG_HDR virtual std::string MACRO_DEBUGLABEL_SIG_IMP;

    #define MACRO_GRAPHPRINT_PARAMS std::ostream* stream, int* node
    #define MACRO_GRAPHPRINT_SIG_IMP graphPrint (MACRO_GRAPHPRINT_PARAMS)
    #define MACRO_GRAPHPRINT_SIG_HDR virtual int MACRO_GRAPHPRINT_SIG_IMP;
#else
    #define MACRO_DEBUGLABEL_PARAMS
    #define MACRO_DEBUGLABEL_SIG_IMP
    #define MACRO_DEBUGLABEL_SIG_HDR

    #define MACRO_GRAPHPRINT_PARAMS
    #define MACRO_GRAPHPRINT_SIG_IMP 
    #define MACRO_GRAPHPRINT_SIG_HDR 
#endif

namespace Phi {
    class Context;

    // This namespace contains all AST nodes
    // Some of them are Abstract: i.e. there are no actual objects of that type.
    // None of these are allowed for stack allocation.
    namespace Node {
        struct Node: public std::enable_shared_from_this<Node> {
            std::shared_ptr<Node> left = nullptr;
            std::shared_ptr<Node> right = nullptr;

            Node() {}
            Node(std::shared_ptr<Node> right): right(right) {}
            virtual ~Node() {}

            MACRO_DEBUGLABEL_SIG_HDR
            MACRO_GRAPHPRINT_SIG_HDR

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        void tryElaborate(std::shared_ptr<Node> node, MACRO_ELAB_PARAMS);
        void tryTranslate(std::shared_ptr<Node> node, MACRO_TRANS_PARAMS);

        struct ErrorNode: public Node {
            MACRO_TRANS_SIG_HDR
        };

        struct Token: public Node {
            std::string text;

            Token(const char* text): text(text) {}
        };

        // Molecular
        struct Identifier: public Node {
            std::string idString;

            MACRO_DEBUGLABEL_SIG_HDR

            Identifier(std::string identifier);

            // No elaboration needed.
            MACRO_TRANS_SIG_HDR
        };

        // Some forward declarations
        struct Expression;
        struct LHExpression;
        struct Range;

        // Declarations
        struct Declaration: public Node { // Abstract
            std::shared_ptr<Identifier> identifier;

            Declaration(std::shared_ptr<Identifier> identifier): identifier(identifier) {}
        };

        struct Port: public Declaration {
            enum class Polarity {
                input = 0,
                output,
                output_reg
            };
            Polarity polarity;
            std::shared_ptr<Range> bus;
            
            optional<std::string> annotation;

            MACRO_DEBUGLABEL_SIG_HDR
            MACRO_GRAPHPRINT_SIG_HDR

            Port(std::shared_ptr<Identifier> identifier, bool polarity, std::shared_ptr<Range> bus, std::optional<std::string> annotation): Declaration(identifier), polarity(polarity ? Polarity::output : Polarity::input), bus(bus), annotation(annotation) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct TopLevelNamespace: public Declaration {
            std::shared_ptr<Node> contents;

            MACRO_DEBUGLABEL_SIG_HDR
            MACRO_GRAPHPRINT_SIG_HDR

            TopLevelNamespace(std::shared_ptr<Identifier> identifier, std::shared_ptr<Node> contents): Declaration(identifier), contents(contents) {}
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct Statement;
        struct DeclarationListItem;
        struct TopLevelDeclaration: public Declaration {
            // For elaborative use
            std::shared_ptr<Declaration> preambles = nullptr;
            std::shared_ptr<Statement> addenda = nullptr;

            enum class Type {
                module = 0,
                interface = 1
            };
            constexpr static SymbolSpace::Type declTypeMap[] = {SymbolSpace::Type::module, SymbolSpace::Type::other};

            Type type;
            std::shared_ptr<Port> ports;
            std::shared_ptr<LHExpression> inheritance;
            std::shared_ptr<Statement> contents;

            MACRO_DEBUGLABEL_SIG_HDR
            MACRO_GRAPHPRINT_SIG_HDR

            TopLevelDeclaration(std::shared_ptr<Identifier> identifier, Type type, std::shared_ptr<Port> ports, std::shared_ptr<LHExpression> inheritance, std::shared_ptr<Statement> contents = nullptr): Declaration(identifier), type(type), ports(ports), inheritance(inheritance), contents(contents) {}
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

            std::shared_ptr<DeclarationListItem> propertyDeclaration(std::string container, std::string property, std::shared_ptr<Range> bus); 
            void propertyAssignment(std::string container, std::string property, std::string rightHandSide);
        };

        // Templating
        struct TemplateDeclaration: public Declaration {
            std::shared_ptr<Expression> assignment;

            TemplateDeclaration(std::shared_ptr<Identifier> identifier, std::shared_ptr<Expression> assignment): Declaration(identifier), assignment(assignment) {}
        };

        // Statements
        struct Statement: public Node { // Abstract
            optional<std::string> annotation = nullopt;
            bool inComb = false;
        };

        // Block-Based Statements
        struct BlockBased: public Statement { // Abstract
            std::shared_ptr<Statement> contents;
            BlockBased(std::shared_ptr<Statement> contents): contents(contents) {}
        };

        struct If: public BlockBased {
            std::shared_ptr<Expression> expression;
            std::shared_ptr<If> elseBlock;

            If(std::shared_ptr<Statement> contents, std::shared_ptr<Expression> expression, std::shared_ptr<If> elseBlock): BlockBased(contents), expression(expression), elseBlock(elseBlock) {}
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct ForLoop: public BlockBased {
            std::shared_ptr<Range> range;
            std::shared_ptr<Identifier> identifier;

            ForLoop(std::shared_ptr<Statement> contents, std::shared_ptr<Range> range, std::shared_ptr<Identifier> identifier): BlockBased(contents), range(range), identifier(identifier) {}
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct Namespace: public BlockBased {
            std::shared_ptr<Identifier> identifier;

            Namespace(std::shared_ptr<Statement> contents, std::shared_ptr<Identifier> identifier): BlockBased(contents), identifier(identifier) {}
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

        };

        struct LabeledStatementList;
        struct SpecialNumber;

        struct Switch: public BlockBased {
            std::shared_ptr<Expression> expression;
            std::shared_ptr<LabeledStatementList> list;

            Switch(std::shared_ptr<Expression> expression, std::shared_ptr<LabeledStatementList> list): BlockBased(nullptr), expression(expression), list(list) {}
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct LabeledStatementList: public Node {
            bool isDefault; // Is this the default case in a switch statement?
            std::shared_ptr<Expression> label;
            std::shared_ptr<SpecialNumber> specialNumber;

            std::shared_ptr<Statement> statements;

            LabeledStatementList(bool isDefault, std::shared_ptr<Expression> label, std::shared_ptr<SpecialNumber> specialNumber, std::shared_ptr<Statement> statements): isDefault(isDefault), label(label), specialNumber(specialNumber), statements(statements) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct Combinational: public BlockBased {
            std::vector< std::function<void()> > conclusionTriggers;
            
            Combinational(std::shared_ptr<Statement> contents): BlockBased(contents) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        // Subdeclarations
        struct VariableLengthDeclaration: public Node {
            enum class Type {
                var = 0,
                wire, reg, latch,

                wire_reg, // For things that are Wires in Phi and Registers in Verilog (i.e. assigned to inside a comb block),
                undefined
            };
            Type type = Type::undefined;
            std::shared_ptr<Range> bus;
            std::shared_ptr<DeclarationListItem> declarationList;

            MACRO_GRAPHPRINT_SIG_HDR
            MACRO_DEBUGLABEL_SIG_HDR

            VariableLengthDeclaration(Type type, std::shared_ptr<Range> bus, std::shared_ptr<DeclarationListItem> declarationList): type(type), bus(bus), declarationList(declarationList) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct DeclarationListItem: public Declaration {
            // CHECK IF THIS EXISTS FIRST: IF IT DOES, THEN TRANSLATE *THIS* INSTEAD OF IDENTIFIER
            // THIS HAS BEEN A HACKY PSA
            // For elaborative use
            std::shared_ptr<LHExpression> trueIdentifier = nullptr;
            
            optional< std::weak_ptr<Range> > bus;

            VariableLengthDeclaration::Type type;
            std::shared_ptr<Expression> array;
            std::shared_ptr<Expression> optionalAssignment;

            bool hasEnable = false;

            MACRO_DEBUGLABEL_SIG_HDR

            DeclarationListItem(std::shared_ptr<Identifier> identifier, std::shared_ptr<Expression> array, std::shared_ptr<Expression> optionalAssignment): Declaration(identifier), array(array), optionalAssignment(optionalAssignment) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

        };

        struct ExpressionIDPair;
        struct InstanceDeclaration: public Declaration {
            // For elaborative use
            std::optional< std::weak_ptr<SymbolSpace> > symSpace;

            std::shared_ptr<LHExpression> module;
            std::shared_ptr<ExpressionIDPair> parameters;

            std::shared_ptr<Expression> array;
            std::shared_ptr<ExpressionIDPair> ports;

            InstanceDeclaration(std::shared_ptr<Identifier> identifier, std::shared_ptr<LHExpression> module, std::shared_ptr<ExpressionIDPair> parameters, std::shared_ptr<Expression> array, std::shared_ptr<ExpressionIDPair> ports): Declaration(identifier), module(module), parameters(parameters), array(array), ports(ports) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

            void elaboratePorts(Context* context);
        };

        struct ExpressionIDPair: public Declaration {
            std::shared_ptr<Expression> expression;

            ExpressionIDPair(std::shared_ptr<Identifier> identifier, std::shared_ptr<Expression> expression): Declaration(identifier), expression(expression) {}
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        // Nondeclarative Statements
        struct Nondeclarative: public Statement { // Abstract
            std::shared_ptr<LHExpression> lhs;

            Nondeclarative(std::shared_ptr<LHExpression> lhs): lhs(lhs) {}
        };

        struct NondeclarativeAssignment: public Nondeclarative {
            std::shared_ptr<Expression> expression;
            bool skipTranslation = false;

            MACRO_GRAPHPRINT_SIG_HDR

            NondeclarativeAssignment(std::shared_ptr<LHExpression> lhs, std::shared_ptr<Expression> expression): Nondeclarative(lhs), expression(expression) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

            static void drivingAssignment(Context* context, std::shared_ptr<LHExpression> lhs, std::shared_ptr<Expression> expression, bool* skipTranslation, bool* inComb);
        };
        
        struct NondeclarativePorts: public Nondeclarative {
            std::shared_ptr<ExpressionIDPair> ports;

            NondeclarativePorts(std::shared_ptr<LHExpression> lhs, std::shared_ptr<ExpressionIDPair> ports): Nondeclarative(lhs), ports(ports) {}

            MACRO_ELAB_SIG_HDR
            // No translation needed.
        };

        // Expression
        struct Expression: public Node { //Abstract
            enum class Type {
                // In ascending order of precedence
                compileTime = 0,
                parameterSensitive,
                runTime,
                error = 0xFF
            };

            Type type = Type::error;
            AccessWidth numBits = 0;
            optional<llvm::APInt> value = nullopt; // Value iff compileTime
 
            static std::shared_ptr<Expression> abstract(Type type, AccessWidth numBits = 0, optional<llvm::APInt> value = nullopt) {
                auto abstractExpression = std::make_shared<Expression>();
                abstractExpression->type = type; abstractExpression->numBits = numBits; abstractExpression->value = value;
                return abstractExpression;
            }
        };

        // Range
        struct Range: public Node {
            std::shared_ptr<Expression> from;
            std::shared_ptr<Expression> to;

            Range(std::shared_ptr<Expression> from, std::shared_ptr<Expression> to): from(from), to(to) {}
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

            void getValues(AccessWidth* from, AccessWidth* to); // CALL ONLY AFTER ELABORATION!!
        };      

        // Left Hand Expressions
        struct LHExpression: public Expression { // Abstract
            std::vector<SymbolTable::Access> accessList(optional<AccessWidth>* from, optional<AccessWidth>* to);
            static void lhDrivenProcess(std::shared_ptr<Node> suspect, Phi::SymbolTable* table);

            MACRO_ELAB_SIG_HDR
        };

        
        struct IdentifierExpression: public LHExpression {
            std::shared_ptr<Identifier> identifier;

            MACRO_GRAPHPRINT_SIG_HDR

            IdentifierExpression(std::shared_ptr<Identifier> identifier): identifier(identifier) {}

            // No elaboration needed.
            MACRO_TRANS_SIG_HDR
        };

        struct PropertyAccess: public LHExpression {
            PropertyAccess(std::shared_ptr<LHExpression> object, std::shared_ptr<LHExpression> property) {
                this->left = object; this->right = property;
            }

            // No elaboration needed.
            MACRO_TRANS_SIG_HDR
        };

        struct ArrayAccess: public LHExpression {
            bool index = true; // If false, translation should treat this as a namespace array access

            ArrayAccess(std::shared_ptr<LHExpression> object, std::shared_ptr<Expression> width) {
                this->left = object; this->right = width;
            }

            // Inherits elaboration.
            MACRO_TRANS_SIG_HDR
        };
        
        struct RangeAccess: public LHExpression {
            RangeAccess(std::shared_ptr<LHExpression> object, std::shared_ptr<Range> range) {
                this->left = object; this->right = (std::shared_ptr<Node>)range;
            }

            // Inherits elaboration.
            MACRO_TRANS_SIG_HDR
        };

        struct LHConcatenation: public LHExpression {
            LHConcatenation(std::shared_ptr<LHExpression> of, std::shared_ptr<LHExpression> with) {
                this->left = of; this->right = with;
            }

            //Inherits elaboration.
            MACRO_TRANS_SIG_HDR
        };

        
        struct Literal: public Expression {
            std::string interpretableSaved;
            Literal(std::string interpretable, bool widthIncluded = true);

            // No elaboration needed.
            MACRO_TRANS_SIG_HDR
        };

        struct SpecialNumber: public Node {
            unsigned int numBits;
            uint8 radix;
            std::string number;
            
            SpecialNumber(std::string interpretable);

            MACRO_TRANS_SIG_HDR
        };

        struct Unary: public Expression {
            enum class Operation {
                negate = 0,
                bitwiseNot,
                allAnd,
                allOr
            };
            Operation operation;
            Unary(Operation operation, std::shared_ptr<Expression> right): operation(operation) { this->right = right; }

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct Binary: public Expression {
            enum class Operation {
                equal = 0,
                notEqual,
                greaterThan,
                lessThan,
                greaterThanOrEqual,
                lessThanOrEqual,
                unsignedLessThan,
                unsignedGreaterThan,
                unsignedLessThanOrEqual,
                unsignedGreaterThanOrEqual,

                plus,
                minus,
                unsignedPlus,
                unsignedMinus,

                mul,
                div,
                modulo,

                bitwiseOr,
                bitwiseAnd,
                bitwiseXor,

                shiftLeftLogical,
                shiftRightLogical,
                shiftRightArithmetic
            };
            Operation operation;

            Binary(std::shared_ptr<Expression> left,  Operation operation, std::shared_ptr<Expression> right): operation(operation) {
                this->left = left; this->right = right;
            }

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        // Concatenation
        struct RepeatConcatenation: public Expression {
            RepeatConcatenation(std::shared_ptr<Expression> repeatCount, std::shared_ptr<Expression> repeatable) {
                this->left = repeatCount; this->right = repeatable;
            }
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct Concatenation: public Expression {
            Concatenation(std::shared_ptr<Expression> of, std::shared_ptr<Expression> with) {
                this->left = of; this->right = with;
            }
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };
        
        // Procedural Call
        struct Argument: public Node {}; // Abstract

        struct StringArgument: public Argument {
            std::string argument;
            StringArgument(std::string argument): argument(argument) {
                this->argument.erase(0, 1);
                this->argument.erase(this->argument.length() - 1, 1);
            }
        };
        
        struct ExpressionArgument: public Argument {
            std::shared_ptr<Expression> argument;
            ExpressionArgument(std::shared_ptr<Expression> argument): argument(argument) {}

            MACRO_ELAB_SIG_HDR
        };

        struct ProceduralCall: public Expression {
            ProceduralCall(std::shared_ptr<LHExpression> function, std::shared_ptr<Argument> argument) {
                this->left = function; this->right = argument;
            }

            MACRO_ELAB_SIG_HDR
        };

        // Multiplexer
        struct ExpressionPair: public Node {
            // If both of the following are nullptr, it's default
            std::shared_ptr<Expression> label;
            std::shared_ptr<SpecialNumber> specialNumber;

            std::shared_ptr<Expression> result;
            
            ExpressionPair(std::shared_ptr<Expression> label, std::shared_ptr<SpecialNumber> specialNumber, std::shared_ptr<Expression> result): label(label), specialNumber(specialNumber), result(result) {}
            MACRO_ELAB_SIG_HDR
            // Translation not needed due to PII by Multiplexer
        };

        struct Multiplexer: public Expression {
            // std::shared_ptr<Node> selection could either be specialNumber or expression!!
            bool inComb = false;

            Multiplexer(std::shared_ptr<Node> selection, std::shared_ptr<ExpressionPair> options) {
                this->left = selection; this->right = options;
            }

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };
    }
}
#endif

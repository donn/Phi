#ifndef _node_h
#define _node_h
#include "Context.h"
#include "Types.h"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>
#include <typeinfo>

#include <ostream>
#include <tuple>

// Elaboration Macros
#define MACRO_ELAB_PARAMS Phi::Context* context
#define MACRO_ELAB_SIG_IMP elaborate (MACRO_ELAB_PARAMS)
#define MACRO_ELAB_SIG_HDR virtual void MACRO_ELAB_SIG_IMP;

// Translation Macros
#define MACRO_TRANS_PARAMS std::ostream* stream, std::string namespaceSoFar, int* indent
#define MACRO_TRANS_SIG_IMP translate (MACRO_TRANS_PARAMS)
#define MACRO_TRANS_SIG_HDR virtual void MACRO_TRANS_SIG_IMP;

#define MACRO_INDENTATION std::string(*indent * 4, ' ')
#define MACRO_EOL std::endl << MACRO_INDENTATION
#define MACRO_INDENT do { *indent += 1; *stream << MACRO_EOL; } while (0)
#define MACRO_DEDENT do { *indent -= 1; *stream << MACRO_EOL; } while (0)

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
            Location location;

            Node(Location location): location(location) {}
            virtual ~Node() {}

            MACRO_DEBUGLABEL_SIG_HDR
            MACRO_GRAPHPRINT_SIG_HDR

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        void tryElaborate(std::shared_ptr<Node> node, MACRO_ELAB_PARAMS);
        void tryTranslate(std::shared_ptr<Node> node, MACRO_TRANS_PARAMS);

        struct ErrorNode: public Node {
            ErrorNode(Location location): Node(location) {}
            MACRO_TRANS_SIG_HDR
        };

        struct Token: public Node {
            std::string text;

            Token(Location location, const char* text):  Node(location),  text(text) {}
        };

        // Molecular
        struct Identifier: public Node, std::string {
            MACRO_DEBUGLABEL_SIG_HDR

            Identifier(Location location, std::string identifier): Node(location), std::string(identifier) {}

            // No elaboration needed.
            MACRO_TRANS_SIG_HDR
        };

        // Some forward declarations
        struct Expression;
        struct LHExpression;
        struct Range;

        // Statements
        struct Statement: public Node { // Abstract
            optional<std::string> annotation = nullopt;
            std::shared_ptr<Statement> next = nullptr;
            
            Statement(Location location, optional<std::string> annotation = nullopt): Node(location), annotation(annotation) {}

            MACRO_GRAPHPRINT_SIG_HDR
        };

        // Declarations
        struct Declaration: public Statement { // Abstract
            std::shared_ptr<Identifier> identifier;

            Declaration(Location location, std::shared_ptr<Identifier> identifier, optional<std::string> annotation = nullopt):  Statement(location, annotation),  identifier(identifier) {}

            std::vector<SymbolTable::Access> immediateAccessList();
        };

        struct Port: public Declaration, public PortObject {
            PortObject::Polarity polarity;
            std::shared_ptr<Range> bus;

            MACRO_DEBUGLABEL_SIG_HDR
            MACRO_GRAPHPRINT_SIG_HDR

            Port(Location location, std::shared_ptr<Identifier> identifier, bool polarity, std::shared_ptr<Range> bus, optional<std::string> annotation):  Declaration(location, identifier, annotation), polarity(polarity ? PortObject::Polarity::output : PortObject::Polarity::input), bus(bus) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

            void addInCurrentContext(MACRO_ELAB_PARAMS, bool addOutputCheck);

            virtual Polarity getPolarity() { return polarity; }
            virtual std::string getName();
            virtual AccessWidth getWidth();

            bool operator==(const Port& rhs);
            bool operator!=(const Port& rhs);
            
            // For elaborative use
            bool assignedInComb = false;
        };

        struct TopLevelNamespace: public Declaration {
            std::shared_ptr<Node> contents;

            MACRO_DEBUGLABEL_SIG_HDR
            MACRO_GRAPHPRINT_SIG_HDR

            TopLevelNamespace(Location location, std::shared_ptr<Identifier> identifier, std::shared_ptr<Node> contents): Declaration(location,  identifier), contents(contents) {}
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct DeclarationListItem;
        struct InheritanceListItem;
        struct TopLevelDeclaration: public Declaration {
            enum class Type {
                module = 0,
                interface = 1
            };
            constexpr static Space::Type declTypeMap[] = {Space::Type::module, Space::Type::interface};

            Type type;
            std::shared_ptr<Port> ports;
            std::shared_ptr<InheritanceListItem> inheritance;
            std::shared_ptr<Statement> contents;

            MACRO_DEBUGLABEL_SIG_HDR
            MACRO_GRAPHPRINT_SIG_HDR

            TopLevelDeclaration(Location location, std::shared_ptr<Identifier> identifier, Type type, std::shared_ptr<Port> ports, std::shared_ptr<InheritanceListItem> inheritance, std::shared_ptr<Statement> contents = nullptr): Declaration(location, identifier), type(type), ports(ports), inheritance(inheritance), contents(contents) {}
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
            
            // For elaborative use
            static std::shared_ptr<LHExpression> lhx(Context* context, std::string property);
            std::shared_ptr<DeclarationListItem> propertyDeclaration(Context* context, std::string container, std::string property, std::shared_ptr<Range> bus); 
            void propertyAssignment(Context* context, std::shared_ptr<DeclarationListItem> dli, std::string rightHandSide);
            std::shared_ptr<Declaration> preambles = nullptr;
            std::shared_ptr<Statement> addenda = nullptr;
            std::weak_ptr<SpaceWithPorts> space;
        };

        // Templating
        struct TemplateDeclaration: public Declaration {
            std::shared_ptr<Expression> assignment;

            TemplateDeclaration(Location location, std::shared_ptr<Identifier> identifier, std::shared_ptr<Expression> assignment): Declaration(location,  identifier), assignment(assignment) {}
        };

        // Block-Based
        struct BlockBased: public Statement { // Abstract
            std::shared_ptr<Statement> contents;
            BlockBased(Location location, std::shared_ptr<Statement> contents):  Statement(location),  contents(contents) {}
        };

        struct If: public BlockBased {
            std::shared_ptr<Expression> expression;
            std::shared_ptr<If> elseBlock;

            If(Location location, std::shared_ptr<Statement> contents, std::shared_ptr<Expression> expression, std::shared_ptr<If> elseBlock):  BlockBased(location, contents), expression(expression), elseBlock(elseBlock) {}
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

            // For elaborative use
            bool inComb = false;
        };

        struct ForLoop: public BlockBased {
            std::shared_ptr<Range> range;
            std::shared_ptr<Identifier> identifier;

            ForLoop(Location location, std::shared_ptr<Statement> contents, std::shared_ptr<Range> range, std::shared_ptr<Identifier> identifier):  BlockBased(location, contents), range(range), identifier(identifier) {}
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct Namespace: public BlockBased {
            std::shared_ptr<Identifier> identifier;

            Namespace(Location location, std::shared_ptr<Statement> contents, std::shared_ptr<Identifier> identifier):  BlockBased(location, contents), identifier(identifier) {}
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

        };

        struct LabeledStatementList;
        struct SpecialNumber;

        struct Switch: public BlockBased {
            std::shared_ptr<Expression> expression;
            std::shared_ptr<LabeledStatementList> list;

            Switch(Location location, std::shared_ptr<Expression> expression, std::shared_ptr<LabeledStatementList> list):  BlockBased(location, nullptr), expression(expression), list(list) {}
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct LabeledStatementList: public Statement {
            bool isDefault; // Is this the default case in a switch statement?
            std::shared_ptr<Expression> label;
            std::shared_ptr<SpecialNumber> specialNumber;

            std::shared_ptr<Statement> statements;

            LabeledStatementList(Location location, bool isDefault, std::shared_ptr<Expression> label, std::shared_ptr<SpecialNumber> specialNumber, std::shared_ptr<Statement> statements):  Statement(location),  isDefault(isDefault), label(label), specialNumber(specialNumber), statements(statements) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct Combinational: public BlockBased {
            std::vector< std::function<void()> > conclusionTriggers;
            
            Combinational(Location location, std::shared_ptr<Statement> contents):  BlockBased(location, contents) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        // Subdeclarations
        struct VariableLengthDeclaration: public Statement {
            enum class Type {
                var = 0,
                wire, reg, latch,
                undefined
            };
            Type type = Type::undefined;
            std::shared_ptr<Range> bus;
            std::shared_ptr<DeclarationListItem> declarationList;

            MACRO_GRAPHPRINT_SIG_HDR
            MACRO_DEBUGLABEL_SIG_HDR

            VariableLengthDeclaration(Location location, Type type, std::shared_ptr<Range> bus, std::shared_ptr<DeclarationListItem> declarationList):  Statement(location),  type(type), bus(bus), declarationList(declarationList) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct LHExpressionEncapsulator;
        struct DeclarationListItem: public Declaration {

            VariableLengthDeclaration::Type type;
            std::shared_ptr<Expression> array;
            std::shared_ptr<Expression> optionalAssignment;

            MACRO_DEBUGLABEL_SIG_HDR

            DeclarationListItem(Location location, std::shared_ptr<Identifier> identifier, std::shared_ptr<Expression> array, std::shared_ptr<Expression> optionalAssignment): Declaration(location,  identifier), array(array), optionalAssignment(optionalAssignment) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
            void elaborationAssistant(MACRO_ELAB_PARAMS);
            
            // For elaborative use
            std::shared_ptr<LHExpressionEncapsulator> accessor = nullptr; 
            AccessWidth size = 1;

            bool hasEnable = false;
            bool assignedInComb = false;
            
            optional< std::weak_ptr<Range> > bus;

        };

        struct ExpressionIDPair;
        struct InstanceDeclaration: public Declaration {
            std::shared_ptr<LHExpression> module;
            std::shared_ptr<ExpressionIDPair> parameters;

            std::shared_ptr<Expression> array;
            std::shared_ptr<ExpressionIDPair> ports;

            InstanceDeclaration(Location location, std::shared_ptr<Identifier> identifier, std::shared_ptr<LHExpression> module, std::shared_ptr<ExpressionIDPair> parameters, std::shared_ptr<Expression> array, std::shared_ptr<ExpressionIDPair> ports): Declaration(location,  identifier), module(module), parameters(parameters), array(array), ports(ports) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

            void elaboratePorts(Context* context);
            
            // For elaborative use
            optional< std::weak_ptr<SpaceWithPorts> > symSpace;
        };

        struct ExpressionIDPair: public Declaration {
            std::shared_ptr<Expression> expression;

            ExpressionIDPair(Location location, std::shared_ptr<Identifier> identifier, std::shared_ptr<Expression> expression): Declaration(location,  identifier), expression(expression) {}
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        // Nondeclarative Statements
        struct Nondeclarative: public Statement { // Abstract
            std::shared_ptr<LHExpressionEncapsulator> lhxe;

            Nondeclarative(Location location, std::shared_ptr<LHExpressionEncapsulator> lhxe):  Statement(location),  lhxe(lhxe) {}
        };

        struct NondeclarativeAssignment: public Nondeclarative {
            std::shared_ptr<Expression> expression;
            MACRO_GRAPHPRINT_SIG_HDR

            NondeclarativeAssignment(Location location, std::shared_ptr<LHExpressionEncapsulator> lhxe, std::shared_ptr<Expression> expression): Nondeclarative(location,  lhxe), expression(expression) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

            static void drivingAssignment(Context* context, std::shared_ptr<LHExpressionEncapsulator> lhxe, std::shared_ptr<Expression> expression, bool* skipTranslation, bool* inComb);

            // For elaborative use
            bool inComb = false;
            bool skipTranslation = false;
        };
        
        struct NondeclarativePorts: public Nondeclarative {
            std::shared_ptr<ExpressionIDPair> ports;

            NondeclarativePorts(Location location, std::shared_ptr<LHExpressionEncapsulator> lhxe, std::shared_ptr<ExpressionIDPair> ports): Nondeclarative(location,  lhxe), ports(ports) {}

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
            
            std::shared_ptr<Node> left = nullptr;
            std::shared_ptr<Node> right = nullptr;

            Expression(Location location): Node(location) {}
 
            static std::shared_ptr<Expression> abstract(Type type, AccessWidth numBits = 0, optional<llvm::APInt> value = nullopt) {
                static std::string ice = "Internal Compiler Error";
                auto abstractExpression = std::make_shared<Expression>(Location(&ice, 0, 0));
                abstractExpression->type = type; abstractExpression->numBits = numBits; abstractExpression->value = value;
                return abstractExpression;
            }

            MACRO_GRAPHPRINT_SIG_HDR

            // For elaborative use
            Type type = Type::error;
            AccessWidth numBits = 0;
            optional<llvm::APInt> value = nullopt; // Value iff compileTime
        };

        // Left Hand Expressions
        struct LHExpression: public Expression { // Abstract
            std::tuple< std::vector<SymbolTable::Access>, optional<AccessWidth>, optional<AccessWidth> > accessList();
            LHExpression(Location location): Expression(location) {}
            MACRO_ELAB_SIG_HDR
        };

        // Range
        struct Range: public LHExpression {
            std::shared_ptr<Expression> from;
            std::shared_ptr<Expression> to;

            Range(Location location, std::shared_ptr<Expression> from, std::shared_ptr<Expression> to):  LHExpression(location),  from(from), to(to) {}
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

            // For elaborative use
            std::pair<AccessWidth, AccessWidth> getValues();
            AccessWidth getWidth(); 
        };      

        struct InheritanceListItem: public Statement {
            std::shared_ptr<LHExpression> lhx;

            InheritanceListItem(Location location, std::shared_ptr<LHExpression> contained): Statement(location), lhx(contained) { }
        };

        struct LHExpressionEncapsulator: public LHExpression {
            std::shared_ptr<LHExpression> lhx;

            LHExpressionEncapsulator(Location location, std::shared_ptr<LHExpression> lhx):  LHExpression(location),  lhx(lhx) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct LHExpressionEvaluator: public Expression {
            std::shared_ptr<LHExpressionEncapsulator> lhxe;

            LHExpressionEvaluator(Location location, std::shared_ptr<LHExpressionEncapsulator> lhxe): Expression(location),  lhxe(lhxe) {}

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };
        
        struct IdentifierExpression: public LHExpression {
            std::shared_ptr<Identifier> identifier;

            MACRO_GRAPHPRINT_SIG_HDR

            IdentifierExpression(Location location, std::shared_ptr<Identifier> identifier):  LHExpression(location),  identifier(identifier) {}

            // No elaboration needed.
            MACRO_TRANS_SIG_HDR
        };

        struct PropertyAccess: public LHExpression {
            PropertyAccess(Location location, std::shared_ptr<LHExpression> object, std::shared_ptr<LHExpression> property): LHExpression(location) {
                this->left = object; this->right = property;
            }

            // Inherits elaboration.
            // Translated via PII.
        };

        struct IndexAccess: public LHExpression {
            IndexAccess(Location location, std::shared_ptr<LHExpression> object, std::shared_ptr<Expression> width): LHExpression(location) {
                this->left = object; this->right = width;
            }

            // Inherits elaboration.
            // Translated via PII.

            // For elaborative use
            bool array = false; // If true, translation should treat this as an array access
        };
        
        struct RangeAccess: public LHExpression {
            RangeAccess(Location location, std::shared_ptr<LHExpression> object, std::shared_ptr<Range> range): LHExpression(location) {
                this->left = object; this->right = (std::shared_ptr<Node>)range;
            }

            // Inherits elaboration.
            // No translation necessary.
        };

        struct LHConcatenation: public LHExpression {
            LHConcatenation(Location location, std::shared_ptr<LHExpressionEncapsulator> of, std::shared_ptr<LHExpressionEncapsulator> with): LHExpression(location) {
                this->left = of; this->right = with;
            }

            //Inherits elaboration.
            MACRO_TRANS_SIG_HDR
        };

        
        struct Literal: public Expression {
            std::string interpretableSaved;
            Literal(Location location, std::string interpretable, bool widthIncluded = true);

            // No elaboration needed.
            MACRO_TRANS_SIG_HDR
        };

        struct SpecialNumber: public Expression {
            uint8 radix;
            std::string number;
            
            SpecialNumber(Location location, std::string interpretable);

            MACRO_TRANS_SIG_HDR
        };

        struct Decoder: public Expression {
            Decoder(Location location, std::shared_ptr<Expression> decodable): Expression(location) { this->right = decodable; }

            MACRO_ELAB_SIG_HDR
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
            Unary(Location location, Operation operation, std::shared_ptr<Expression> right):  Expression(location),  operation(operation) { this->right = right; }

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

            Binary(Location location, std::shared_ptr<Expression> left,  Operation operation, std::shared_ptr<Expression> right):  Expression(location),  operation(operation) {
                this->left = left; this->right = right;
            }

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        // Concatenation
        struct RepeatConcatenation: public Expression {
            RepeatConcatenation(Location location, std::shared_ptr<Expression> repeatCount, std::shared_ptr<Expression> repeatable): Expression(location) {
                this->left = repeatCount; this->right = repeatable;
            }
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };

        struct Concatenation: public Expression {
            Concatenation(Location location, std::shared_ptr<Expression> of, std::shared_ptr<Expression> with): Expression(location) {
                this->left = of; this->right = with;
            }
            
            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR
        };
        
        // Procedural Call
        struct Argument: public Statement { // Abstract
            Argument(Location location): Statement(location) {}
        };

        struct StringArgument: public Argument {
            std::string argument;
            StringArgument(Location location, std::string argument):  Argument(location),  argument(argument) {
                this->argument.erase(0, 1);
                this->argument.erase(this->argument.length() - 1, 1);
            }
        };
        
        struct ExpressionArgument: public Argument {
            std::shared_ptr<Expression> argument;
            ExpressionArgument(Location location, std::shared_ptr<Expression> argument):  Argument(location),  argument(argument) {}

            MACRO_ELAB_SIG_HDR
        };

        struct ProceduralCall: public Expression {
            ProceduralCall(Location location, std::shared_ptr<LHExpression> function, std::shared_ptr<Argument> argument): Expression(location) {
                this->left = function; this->right = argument;
            }

            MACRO_ELAB_SIG_HDR
        };

        // Multiplexer
        struct ExpressionPair: public Statement {
            // If both of the following are nullptr, it's default
            std::shared_ptr<Expression> label;
            std::shared_ptr<SpecialNumber> specialNumber;

            std::shared_ptr<Expression> result;
            
            ExpressionPair(Location location, std::shared_ptr<Expression> label, std::shared_ptr<SpecialNumber> specialNumber, std::shared_ptr<Expression> result):  Statement(location),  label(label), specialNumber(specialNumber), result(result) {}
            
            MACRO_ELAB_SIG_HDR
            // Translation not needed due to PII by Multiplexer
        };

        struct Multiplexer: public Expression {
            // Left: Selection expression
            // Right: Something multiplexable: either a list of expression pairs or an lhexpression of an array
            Multiplexer(Location location, std::shared_ptr<Node> selection, std::shared_ptr<Node> multiplexable): Expression(location) {
                this->left = selection; this->right = multiplexable;
            }

            MACRO_ELAB_SIG_HDR
            MACRO_TRANS_SIG_HDR

            // For elaborative use
            bool inComb = false;
        };
    }
}
#endif

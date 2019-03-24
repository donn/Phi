#ifndef _node_h
#define _node_h
// Project Headers
#include "Types.h"
#include <ostream>

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>

// Elaboration Macros
#define MACRO_ELAB_PARAMS SymbolTable* table, Context* context
#define MACRO_ELAB_ARGS table, context
#define MACRO_ELAB_SIG_IMP elaborate (MACRO_ELAB_PARAMS)
#define MACRO_ELAB_SIG_HDR virtual void MACRO_ELAB_SIG_IMP

#if YYDEBUG
#define DEBUGLABEL virtual std::string debugLabel()
#define GRAPHPRINT virtual void graphPrint(std::ostream* stream, int* node)
#else
#define DEBUGLABEL 
#define GRAPHPRINT
#endif

namespace Phi {
    // Forward declarations
    class SymbolTable;
    class Context;

    namespace Node {
        using Width = uint16;
        static const Width maxWidth = UINT16_MAX;

        struct Node {
            Node* left = nullptr;
            Node* right = nullptr;

            Node() {}
            Node(Node* right): right(right) {}
            virtual ~Node() {}

            DEBUGLABEL;
            GRAPHPRINT;

            MACRO_ELAB_SIG_HDR {}
            virtual void translate(std::ofstream* stream) {}
        };

        inline void tryElaborate(Node* node, MACRO_ELAB_PARAMS) {
            if (node) {
                node->elaborate(MACRO_ELAB_ARGS);
            }
        }

        inline void tryTranslate(Node* node, std::ofstream* stream) {
            if (node) {
                node->translate(stream);
            }
        }

        struct ErrorNode: public Node {
            virtual void translate(std::ofstream* stream);
        };

        // Declarations
        struct Expression; // Fwd Declaration
        struct Range; // Fwd Declaration

        struct Declaration: public Node {
            std::string identifier;

            Declaration(std::string identifier): identifier(identifier) {}
        };

        struct Port: public Declaration {
            bool polarity; // polarity ? Input: Output
            Range* bus;

            optional<std::string> annotation;

            Port(const char* identifier, bool polarity, Range* bus, const char* annotation): Declaration(identifier), polarity(polarity), bus(bus) {
                if (annotation) {
                    this->annotation = std::string(annotation);
                }
            }

            MACRO_ELAB_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        struct TopLevelNamespace: public Declaration {
            Node* contents;

            TopLevelNamespace(const char* identifier, Node* contents): Declaration(identifier), contents(contents) {}
            
            DEBUGLABEL ;

            virtual void translate(std::ofstream* stream);

            MACRO_ELAB_SIG_HDR;
        };

        struct Statement;
        struct TopLevelDeclaration: public Declaration {
            enum class Type {
                module = 0,
                interface
            };

            Type type;
            Port* ports;
            Expression* inheritance;
            Statement* contents;

            TopLevelDeclaration(std::string identifier, Type type, Port* ports, Expression* inheritance, Statement* contents = nullptr): Declaration(identifier), type(type), ports(ports), inheritance(inheritance), contents(contents) {}
            
            MACRO_ELAB_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        // Templating
        struct TemplateDeclaration: public Declaration {
            Expression* assignment;

            TemplateDeclaration(const char* identifier, Expression* assignment): Declaration(identifier) {}
        };

        // Statements
        struct Statement: public Node {
            optional<std::string> annotation = nullopt;
        };

        // Block-Based Statements
        struct BlockBased: public Statement {
            Statement* contents;

            BlockBased(Statement* contents): contents(contents) {}
        };

        struct If: public BlockBased {
            Expression* expression;
            If* elseBlock;

            If(Statement* contents, Expression* expression, If* elseBlock): BlockBased(contents), expression(expression), elseBlock(elseBlock) {}
            MACRO_ELAB_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        struct ForLoop: public BlockBased {
            Range* range;
            std::string identifier;

            ForLoop(Statement* contents, Range* range, const char* identifier): BlockBased(contents), range(range), identifier(identifier) {}
            MACRO_ELAB_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        struct Namespace: public BlockBased {
            std::string identifier;

            Namespace(Statement* contents, const char* identifier): BlockBased(contents), identifier(identifier) {}

            MACRO_ELAB_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        struct LabeledStatementList;
        struct SpecialNumber: public Node {
            unsigned int numBits;
            uint8 radix;
            std::string number;
            
            SpecialNumber(const char* interpretable);

            virtual void translate(std::ofstream* stream);
        };
        struct Switch: public BlockBased {
            Expression* expression;
            LabeledStatementList* list;

            Switch(Expression* expression, LabeledStatementList* list): BlockBased(nullptr), expression(expression), list(list) {}

            virtual void translate(std::ofstream* stream);
        };

        struct LabeledStatementList: public Node {
            bool isDefault; // Is this the default case in a switch statement?
            Expression* expression;
            SpecialNumber* specialNumber;

            Statement* statements;

            LabeledStatementList(bool isDefault, Expression* expression, SpecialNumber* specialNumber, Statement* statements): isDefault(isDefault), expression(expression), specialNumber(specialNumber), statements(statements) {}

            virtual void translate(std::ofstream* stream);
        };

        struct Combinational: public BlockBased {
            
            Combinational(Statement* contents): BlockBased(contents) {}

            MACRO_ELAB_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        // Subdeclarations
        struct DeclarationListItem;
        struct VariableLengthDeclaration: public Declaration {
            enum class Type {
                var = 0,
                wire, reg, latch
            };
            Type type;
            Range* bus;
            Expression* array;
            Expression* optionalAssignment;

            static VariableLengthDeclaration* flattenedList(Type type, Range* bus, DeclarationListItem* list);

            VariableLengthDeclaration(std::string identifier, Type type, Range* bus, Expression* array, Expression* optionalAssignment): Declaration(identifier), type(type), bus(bus), array(array), optionalAssignment(optionalAssignment) {}

            MACRO_ELAB_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        struct DeclarationListItem: public Declaration { // TEMP: Flattened in VLD constructor!!
            Expression* array;
            Expression* optionalAssignment;

            DeclarationListItem(const char* identifier, Expression* array, Expression* optionalAssignment): Declaration(identifier), array(array), optionalAssignment(optionalAssignment) {}

        };

        struct ExpressionIDPair;
        struct InstanceDeclaration: public Declaration {
            Expression* module;
            ExpressionIDPair* parameters;

            Expression* array;
            ExpressionIDPair* ports;

            InstanceDeclaration(const char* identifier, Expression* module, ExpressionIDPair* parameters, Expression* array, ExpressionIDPair* ports): Declaration(identifier), module(module), parameters(parameters), array(array), ports(ports) {}

            MACRO_ELAB_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        struct ExpressionIDPair: public Declaration {
            Expression* expression;

            ExpressionIDPair(const char* identifier, Expression* expression): Declaration(identifier), expression(expression) {}
            
            MACRO_ELAB_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        // Nondeclarative Statements
        struct Nondeclarative: public Statement {
            Expression* lhs;

            Nondeclarative(Expression* lhs): lhs(lhs) {}
        };
        struct NondeclarativeAssignment: public Nondeclarative {
            Expression* expression;

            NondeclarativeAssignment(Expression* lhs, Expression* expression): Nondeclarative(lhs), expression(expression) {}

            MACRO_ELAB_SIG_HDR;

             virtual void translate(std::ofstream* stream);
        };
        struct NondeclarativePorts: public Nondeclarative {
            ExpressionIDPair* ports;

            NondeclarativePorts(Expression* lhs, ExpressionIDPair* ports): Nondeclarative(lhs), ports(ports) {}

            MACRO_ELAB_SIG_HDR;
        };

        // Expression
        struct Expression: public Node {
            enum class Type {
                CompileTime = 0,
                ParameterSensitive,
                RunTime,

                Undefined = 0xFF
            };
            Type type = Type::Undefined;

            unsigned int numBits = 0;
            optional<llvm::APInt> value = nullopt;

            bool leftHandExpression = true;
        };

        struct Literal: public Expression {
            Literal(const char* interpretable, bool widthIncluded = true);

            virtual void translate (std::ofstream* stream);
        };

        struct Identifier: public Expression {
            std::string identifier;

            Identifier(const char* identifier): identifier(identifier) {}

            virtual void translate (std::ofstream* stream);
        };

        struct Unary: public Expression {
            enum class Operation {
                negate = 0,
                bitwiseNot,
                allAnd,
                allOr
            };
            Operation operation;
            Unary(Operation operation, Expression* right): operation(operation) { this->right = right; }

            virtual void translate (std::ofstream* stream);
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

            Binary(Expression* left,  Operation operation, Expression* right): operation(operation) {
                this->left = left; this->right = right;
            }

            virtual void translate (std::ofstream* stream);
        };

        struct Ternary: public Expression {
            Expression* condition;

            Ternary(Expression* condition, Expression* left, Expression* right): condition(condition) {
                this->left = left; this->right = right;
            }

            virtual void translate (std::ofstream* stream);
        };

        struct Access: public Expression {};

        struct PropertyAccess: public Access {
            PropertyAccess(Expression* object, Expression* property) {
                this->left = object; this->right = property;
            }

            virtual void translate (std::ofstream* stream);
        };

        struct ArrayAccess: public Access {
            ArrayAccess(Expression* object, Expression* width) {
                this->left = object; this->right = width;
            }

            virtual void translate (std::ofstream* stream);
        };
        
        struct Range: public Node {
            Range(Expression* from, Expression* to) {
                this->left = from; this->right = to;
            }
            
            virtual void translate (std::ofstream* stream);
        };
        struct RangeAccess: public Access {
            RangeAccess(Expression* object, Range* range) {
                this->left = object; this->right = range;
            }

            virtual void translate (std::ofstream* stream);
        };

        struct RepeatConcatenation: public Expression {
            RepeatConcatenation(Expression* repeatCount, Expression* repeatable) {
                this->left = repeatCount; this->right = repeatable;
            }

            virtual void translate (std::ofstream* stream);
        };

        struct Concatenation: public Expression {
            Concatenation(Expression* of, Expression* with) {
                this->left = of; this->right = with;
            }

            virtual void translate (std::ofstream* stream);
        };
        
        struct Argument: public Node {
        };

        struct StringArgument: public Node {
            std::string argument;
            StringArgument(const char* argument): argument(argument) {}
        };
        
        struct ExpressionArgument: public Node {
            Expression* argument;
            ExpressionArgument(Expression* argument): argument(argument) {}
        };

        struct ProceduralCall: public Expression {
            ProceduralCall(Expression* function, Argument* argument) {
                this->left = function; this->right = argument;
            }
        };

        struct ExpressionPair: public Node {
            Expression* label; // If nullptr, default
            Expression* result;
            ExpressionPair(Expression* label, Expression* result): label(label), result(result) {}
        };

        struct Multiplexer: public Expression {
            Multiplexer(Expression* selection, ExpressionPair* options) {}
        };
    }
}
#endif

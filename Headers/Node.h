#ifndef _node_h
#define _node_h
// Project Headers
#include "Types.h"
#include <ostream>

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>

// Elaboration Macros
#define MACRO_ELAB_PARAMS SymbolTable* table, Context* context
#define MACRO_ELAB_SIG_IMP elaborate (MACRO_ELAB_PARAMS)
#define MACRO_ELAB_SIG_HDR virtual void MACRO_ELAB_SIG_IMP

#if YYDEBUG

#define MACRO_DEBUGLABEL_PARAMS
#define MACRO_DEBUGLABEL_SIG_IMP debugLabel (MACRO_DEBUGLABEL_PARAMS)
#define MACRO_DEBUGLABEL_SIG_HDR virtual std::string MACRO_DEBUGLABEL_SIG_IMP

#define MACRO_GRAPHPRINT_PARAMS std::ostream* stream, int* node
#define MACRO_GRAPHPRINT_SIG_IMP graphPrint (MACRO_GRAPHPRINT_PARAMS)
#define MACRO_GRAPHPRINT_SIG_HDR virtual int MACRO_GRAPHPRINT_SIG_IMP
#else
#define MACRO_DEBUGLABEL_PARAMS
#define MACRO_DEBUGLABEL_SIG_IMP
#define MACRO_DEBUGLABEL_SIG_HDR 

#define MACRO_GRAPHPRINT_PARAMS
#define MACRO_GRAPHPRINT_SIG_IMP 
#define MACRO_GRAPHPRINT_SIG_HDR
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

            MACRO_DEBUGLABEL_SIG_HDR;
            MACRO_GRAPHPRINT_SIG_HDR;

            MACRO_ELAB_SIG_HDR;
            virtual void translate(std::ofstream* stream);
        };

        inline void tryElaborate(Node* node, MACRO_ELAB_PARAMS) {
            if (node) {
                node->elaborate(table, context);
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

        // Molecular
        struct Identifier: public Node {
            std::string idString;

            Identifier(const char* identifier);

            MACRO_DEBUGLABEL_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        // Some forward declarations
        struct Expression;
        struct LHExpression;
        struct Range;

        // Declarations

        struct Declaration: public Node {
            Identifier* identifier;

            Declaration(const char* identifier): identifier(new Identifier(identifier)) {}
        };

        struct Port: public Declaration {
            enum class Polarity {
                input = 0,
                output,
                output_reg
            };
            Polarity polarity;
            Range* bus;

            optional<std::string> annotation;

            MACRO_DEBUGLABEL_SIG_HDR;
            MACRO_GRAPHPRINT_SIG_HDR;

            Port(const char* identifier, bool polarity, Range* bus, const char* annotation): Declaration(identifier), polarity(polarity ? Polarity::output : Polarity::input), bus(bus) {
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
            
            MACRO_DEBUGLABEL_SIG_HDR;
            MACRO_GRAPHPRINT_SIG_HDR;

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

            MACRO_DEBUGLABEL_SIG_HDR;
            MACRO_GRAPHPRINT_SIG_HDR;

            TopLevelDeclaration(const char* identifier, Type type, Port* ports, Expression* inheritance, Statement* contents = nullptr): Declaration(identifier), type(type), ports(ports), inheritance(inheritance), contents(contents) {}
            
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
            bool inComb = false;
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
            Identifier* identifier;

            ForLoop(Statement* contents, Range* range, const char* identifier): BlockBased(contents), range(range), identifier(new Identifier(identifier)) {}
            MACRO_ELAB_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        struct Namespace: public BlockBased {
            Identifier* identifier;

            Namespace(Statement* contents, const char* identifier): BlockBased(contents), identifier(new Identifier(identifier)) {}

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
        struct VariableLengthDeclaration: public Node {
            enum class Type {
                var = 0,
                wire, reg, latch,

                wire_reg, // For things that are Wires in Phi and Registers in Verilog (i.e. assigned to inside a comb block),
                undefined
            };
            Type type = Type::undefined;
            Range* bus;
            DeclarationListItem* declarationList;

            MACRO_GRAPHPRINT_SIG_HDR;
            MACRO_DEBUGLABEL_SIG_HDR;

            VariableLengthDeclaration(Type type, Range* bus, DeclarationListItem* declarationList): type(type), bus(bus), declarationList(declarationList) {}

            MACRO_ELAB_SIG_HDR;
            virtual void translate(std::ofstream* stream);
        };

        struct DeclarationListItem: public Declaration {
            VariableLengthDeclaration::Type type;
            Range* bus;
            Expression* array;
            Expression* optionalAssignment;

            MACRO_DEBUGLABEL_SIG_HDR;

            DeclarationListItem(const char* identifier, Expression* array, Expression* optionalAssignment): Declaration(identifier), array(array), optionalAssignment(optionalAssignment) {}

            MACRO_ELAB_SIG_HDR;
            virtual void translate(std::ofstream* stream);

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
            LHExpression* lhs;

            Nondeclarative(LHExpression* lhs): lhs(lhs) {}
        };
        struct NondeclarativeAssignment: public Nondeclarative {
            Expression* expression;

            NondeclarativeAssignment(LHExpression* lhs, Expression* expression): Nondeclarative(lhs), expression(expression) {}

            MACRO_ELAB_SIG_HDR;
            MACRO_GRAPHPRINT_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };
        struct NondeclarativePorts: public Nondeclarative {
            ExpressionIDPair* ports;

            NondeclarativePorts(LHExpression* lhs, ExpressionIDPair* ports): Nondeclarative(lhs), ports(ports) {}

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

        };
        // Range
        struct Range: public Node {
            Range(Expression* from, Expression* to) {
                this->left = from; this->right = to;
            }
            
            virtual void translate (std::ofstream* stream);
        };


        // Left Hand Expressions
        struct LHExpression: public Expression {};

        struct IdentifierExpression: public LHExpression {
            Identifier* identifier;

            IdentifierExpression(const char* identifier): identifier(new Identifier(identifier)) {}

            MACRO_ELAB_SIG_HDR;
            MACRO_GRAPHPRINT_SIG_HDR;

            virtual void translate(std::ofstream* stream);
        };

        struct PropertyAccess: public LHExpression {
            PropertyAccess(Expression* object, Expression* property) {
                this->left = object; this->right = property;
            }

            virtual void translate (std::ofstream* stream);
        };

        struct ArrayAccess: public LHExpression {
            ArrayAccess(Expression* object, Expression* width) {
                this->left = object; this->right = width;
            }

            virtual void translate (std::ofstream* stream);
        };
        
        struct RangeAccess: public LHExpression {
            RangeAccess(Expression* object, Range* range) {
                this->left = object; this->right = (Node*)range;
            }

            virtual void translate (std::ofstream* stream);
        };

        struct Literal: public Expression {
            Literal(const char* interpretable, bool widthIncluded = true);

            virtual void translate(std::ofstream* stream);
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

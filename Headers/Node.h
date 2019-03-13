#ifndef _node_h
#define _node_h

// Project Headers
#include "Types.h"
#include "Context.h"

namespace Phi {
    namespace Node {
        using Width = uint16;
        static const Width maxWidth = UINT16_MAX;

        struct Node {
            Node* left = nullptr;
            Node* right = nullptr;

            Node() {}
            Node(Node* right): right(right) {}
            virtual ~Node() {}

            virtual Node* elaborate() { return nullptr; }
            virtual bool semanticCheck() { return false; }
        };

        struct ErrorNode: public Node {};

        // Declarations
        struct Expression; // Fwd Declaration
        struct Range; // Fwd Declaration

        struct Declaration: public Node {
            std::string name;

            Declaration(std::string name): name(name) {}
        };

        struct Port: public Node {
            std::string name;
            bool polarity; // polarity ? Input: Output
            Range* bus;

            optional<std::string> annotation;

            Port(const char* name, bool polarity, Range* bus, const char* annotation): name(name), polarity(polarity), bus(bus) {
                if (annotation) {
                    this->annotation = std::string(annotation);
                }
            }
        };

        struct TopLevelNamespace: public Declaration {
            Node* contents;

            TopLevelNamespace(const char* name, Node* contents): Declaration(name), contents(contents) {}
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

            TopLevelDeclaration(std::string name, Type type, Port* ports, Expression* inheritance, Statement* contents = nullptr): Declaration(name), type(type), ports(ports), inheritance(inheritance), contents(contents) {}
        };

        // Templating
        struct TemplateDeclaration: public Declaration {
            Expression* assignment;

            TemplateDeclaration(const char* name, Expression* assignment): Declaration(name) {}
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
        };

        struct ForLoop: public BlockBased {
            Range* range;
            std::string identifier;

            ForLoop(Statement* contents, Range* range, const char* identifier): BlockBased(contents), range(range), identifier(identifier) {}
        };

        struct Namespace: public BlockBased {
            std::string identifier;

            Namespace(Statement* contents, const char* identifier): BlockBased(contents), identifier(identifier) {}
        };

        struct LabeledStatementList;
        struct Switch: public BlockBased {
            Expression* expression;
            LabeledStatementList* list;

            Switch(Expression* expression, LabeledStatementList* list): BlockBased(nullptr), expression(expression), list(list) {}
        };

        struct LabeledStatementList: public Node {
            bool isDefault;
            Expression* expression;
            Statement* statements;

            LabeledStatementList(bool isDefault, Expression* expression, Statement* statements): isDefault(isDefault), expression(expression), statements(statements) {}
        };

        struct Combinational: public BlockBased {
            Combinational(Statement* contents): BlockBased(contents) {}
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

            VariableLengthDeclaration(std::string name, Type type, Range* bus, Expression* array, Expression* optionalAssignment): Declaration(name), type(type), bus(bus), array(array), optionalAssignment(optionalAssignment) {}
        };

        struct DeclarationListItem: public Declaration { // TEMP: Flattened in VLD constructor!!
            Expression* array;
            Expression* optionalAssignment;

            DeclarationListItem(const char* name, Expression* array, Expression* optionalAssignment): Declaration(name), array(array), optionalAssignment(optionalAssignment) {}

        };

        struct ExpressionIDPair;
        struct InstanceDeclaration: public Declaration {
            Expression* module;
            ExpressionIDPair* parameters;

            Expression* array;
            ExpressionIDPair* ports;

            InstanceDeclaration(const char* name, Expression* module, ExpressionIDPair* parameters, Expression* array, ExpressionIDPair* ports): Declaration(name), module(module), parameters(parameters), array(array), ports(ports) {}
        };

        struct ExpressionIDPair: Declaration {
            Expression* expression;

            ExpressionIDPair(const char* name, Expression* expression): Declaration(name), expression(expression) {}
        };

        // Nondeclarative Statements
        struct Nondeclarative: public Statement {
            Expression* lhs;

            Nondeclarative(Expression* lhs): lhs(lhs) {}
        };
        struct NondeclarativeAssignment: public Nondeclarative {
            Expression* expression;

            NondeclarativeAssignment(Expression* lhs, Expression* expression): Nondeclarative(lhs), expression(expression) {}
        };
        struct NondeclarativePorts: public Nondeclarative {
            ExpressionIDPair* ports;

            NondeclarativePorts(Expression* lhs, ExpressionIDPair* ports): Nondeclarative(lhs), ports(ports) {}
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
        };

        struct Literal: public Expression {
            std::string literal;
            std::string width;
            uint8 radix;

            Literal(const char* interpretable, bool widthIncluded = true);
        };

        struct Identifier: public Expression {
            std::string identifier;

            Identifier(const char* identifier): identifier(identifier) {}
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
        };

        struct Ternary: public Expression {
            Expression* condition;

            Ternary(Expression* condition, Expression* left, Expression* right): condition(condition) {
                this->left = left; this->right = right;
            }
        };

        struct Access: public Expression {};

        struct PropertyAccess: public Access {
            PropertyAccess(Expression* object, Expression* property) {
                this->left = object; this->right = property;
            }
        };

        struct ArrayAccess: public Access {
            ArrayAccess(Expression* object, Expression* width) {
                this->left = object; this->right = width;
            }
        };
        
        struct Range: public Node {
            Range(Expression* from, Expression* to) {
                this->left = from; this->right = to;
            }
        };
        struct RangeAccess: public Access {
            RangeAccess(Expression* object, Range* range) {
                this->left = object; this->right = range;
            }
        };

        struct RepeatConcatenation: public Expression {
            RepeatConcatenation(Expression* repeatCount, Expression* repeatable) {
                this->left = repeatCount; this->right = repeatable;
            }
        };

        struct Concatenation: public Expression {
            Concatenation(Expression* of, Expression* with) {
                this->left = of; this->right = with;
            }
        };
        
        struct Argument: public Node {};

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

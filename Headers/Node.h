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
            virtual Node* traverse() {
                return NULL;
            }
            Node() {}
            Node(Node* right): right(right) {}
            virtual ~Node() {}
        };

        // Declarations
        struct Expression; // Fwd Declaration
        struct Range; // Fwd Declaration

        struct Declaration: public Node {
            String name;

            Declaration(String name): name(name) {}
        };

        struct Port: public Node {
            String name;
            bool polarity; // polarity ? Input: Output
            Range* bus;

            std::optional<String> annotation;

            Port(const char* name, bool polarity, Range* bus, const char* annotation): name(name), polarity(polarity), bus(bus) {
                if (annotation) {
                    this->annotation = String(annotation);
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

            TopLevelDeclaration(String name, Type type, Port* ports, Expression* inheritance, Statement* contents = nullptr): Declaration(name), type(type), ports(ports), inheritance(inheritance), contents(contents) {}
        };

        // Templating
        struct TemplateDeclaration: public Declaration {
            Expression* assignment;

            TemplateDeclaration(const char* name, Expression* assignment): Declaration(name) {}
        };

        // Statements
        struct Statement: public Node {
            std::optional<String> annotation = std::nullopt;
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
            String identifier;

            ForLoop(Statement* contents, Range* range, const char* identifier): BlockBased(contents), range(range), identifier(identifier) {}
        };

        struct Namespace: public BlockBased {
            String identifier;

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

            VariableLengthDeclaration(String name, Type type, Range* bus, Expression* array, Expression* optionalAssignment): Declaration(name), type(type), bus(bus), array(array), optionalAssignment(optionalAssignment) {}
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
            String literal;
            String width;

            Literal(String interpretable);
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

        struct PropertyAccess: public Expression {
        };

        struct ArrayAccess: public Expression {
        };

        struct Range: public Node {
            
        };
    }
}
#endif

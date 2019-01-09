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

        struct InstanceDeclaration: public Declaration {
            Expression* array;
            Expression* moduleName;
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


        // Nondeclarative Statements
        struct Nondeclarative: public Statement {

        };

        // Expression
        struct Expression: public Node {
            ExpType expType;
            String operation;

            virtual Node* traverse() override;

            Width width;
            String literal;

            Expression(String operation, Node* left, Node* right);
            Expression(String numericInterpretable);
        };
    }
}
#endif

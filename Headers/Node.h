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
            Node* left;
            Node* right;
            virtual Node* traverse() {
                return NULL;
            }
        };

        // Declarations
        struct Expression; // Fwd Declaration
        struct Declaration: public Node {
            String name;
        };

        struct Port {
            String name;
            bool polarity; // Input: Output

            std::optional<bool> clockPort; // Active Low: Active High
            std::optional<bool> resetPort; // Active Low: Active High
            std::optional<bool> enablePort; // Active Low: Active High
        };

        struct Namespace: public Declaration {
            Node* content;
        };
        
        struct TopLevelDeclaration: public Declaration {
            enum class Type {
                module = 0,
                interface
            };
            std::vector<Port> ports;
        };

        struct InstanceDeclaration: public Declaration {
            Expression* array;
            Expression* moduleName;
        };

        struct VariableLengthDecalaration: public Declaration {
            enum class Type {
                var = 0,
                wire, reg, latch
            };
            Type type;
            Expression* array;
            Expression* bus;
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

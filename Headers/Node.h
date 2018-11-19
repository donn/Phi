#ifndef _node_h
#define _node_h

#include <string>

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

        struct Declarative: public Node {
        };

        struct Expression: public Node {
            ExpType::Enum expType;
            std::string operation;

            virtual Node* traverse();

            Width width;
            std::string literal;

            Expression(std::string operation, Node* left, Node* right);
            Expression(std::string numericInterpretable);
        };
    };
};
#endif
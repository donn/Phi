#ifndef _node_h
#define _node_h

#include <string>

#include "Types.h"
#include "Context.h"

namespace Phi {
    namespace Node {
        struct Node {
            Node* left;
            Node* right;
            virtual bool traverse() {
                return true;
            }
        };

        struct Declarative: public Node {
        };
        
        struct Number: public Node {
            using Width = uint8;
            static const Width maxWidth = UINT8_MAX;
            
            uint8 radix;
            uint8 width;
            std::string literal;
            int number;

            Number(uint8 radix, uint8 width, std::string literal): radix(radix), width(width), literal(literal) {}
            Number(std::string interpretable);
        };
    };
};
#endif
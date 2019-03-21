#include "Node.h"
#include <fstream>

using namespace Phi::Node;

void SpecialNumber::translate(std::ofstream* stream){
    //why it called SpecialNumber?
    //because of the existence of x in number like 0101010x0101

    // example
    // case 32b0101010x.. 
    // description --> 32: numBits, 2 equivalent to b:radix, 0101010x..: number


    //note about radix:
    // radix is uint. So, it needs to be converted to character
    // 2 --> b
    // 8 --> O
    // 10 --> d
    // 16 --> h

    // struct SpecialNumber: public Node {
    //             unsigned int numBits;
    //             uint8 radix;
    //             std::string number;
    // }

    *stream << numBits;
    *stream << "'";
    switch(radix){
        case 2:
            *stream << "b";
            break;
        case 8:
            *stream << "O";
            break;
        case 10:
            *stream << "d";
            break;
        case 16:
            *stream << "h";
            break;
        default:
            break;
    }
    *stream << number;
    *stream << ";";
}

void Range::translate(std::ofstream* stream){
    // example
    //  3:4 
    //  description--> 3: from, 4: to

    // Range(Expression* from, Expression* to) {
    //                 this->left = from; this->right = to;
    //             }

    left->translate(stream); //from
    *stream << ":";
    right->translate(stream); //to
}
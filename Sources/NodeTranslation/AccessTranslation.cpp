#include "Node.h"
#include <fstream>

using namespace Phi::Node;

void PropertyAccess::translate (std::ofstream* stream){
    //example: 
    
    // namespace Gamma{
    //     wire x;
    // }
    // Gamma.x; // PropertyAccess: Gamma --> object, x --> property


    // PropertyAccess(Expression* object, Expression* property) {
    //             this->left = object; this->right = property;
    //         }

    left->translate(stream); //object
    *stream << ".";
    right->translate(stream); //property
    
    tryTranslate(right, stream);
}

void ArrayAccess::translate (std::ofstream* stream){
    //example
    // barrelShifter bs[4]; // bs --> object , 4 --> width

    // ArrayAccess(Expression* object, Expression* width) {
    //             this->left = object; this->right = width;
    //         }

    left->translate(stream); //object
    *stream << "[";
    right->translate(stream); //width
    *stream << "]";
    
    tryTranslate(right, stream);
}

void RangeAccess::translate (std::ofstream* stream){
    //example : 
    // arr[3:4]; //arr--> object , 3:4-->range

    // RangeAccess(Expression* object, Range* range) {
    //             this->left = object; this->right = range;
    //         }


    left->translate(stream); //object
    *stream << "[";
    right->translate(stream); //range
    *stream << "]";
    
    tryTranslate(right, stream);

}
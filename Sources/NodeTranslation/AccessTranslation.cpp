#include "Node.h"
#include <fstream>

using namespace Phi::Node;

void PropertyAccess::MACRO_TRANS_SIG_IMP {
    //example: 
    
    // namespace Gamma{
    //     wire x;
    // }
    // Gamma.x; // PropertyAccess: Gamma --> object, x --> property


    // PropertyAccess(Expression* object, Expression* property) {
    //             this->left = object; this->right = property;
    //         }

    tryTranslate(left, stream); //object
    *stream << ".";
    tryTranslate(right, stream); //property
}

void ArrayAccess::MACRO_TRANS_SIG_IMP {
    //example
    // barrelShifter bs[4]; // bs --> object , 4 --> width

    // ArrayAccess(Expression* object, Expression* width) {
    //             this->left = object; this->right = width;
    //         }

    tryTranslate(left, stream); //object
    *stream << "[";
    tryTranslate(right, stream); //width
    *stream << "]";
}

void RangeAccess::MACRO_TRANS_SIG_IMP {
    //example : 
    // arr[3:4]; //arr--> object , 3:4-->range

    // RangeAccess(Expression* object, Range* range) {
    //             this->left = object; this->right = range;
    //         }


    tryTranslate(left, stream); //object
    tryTranslate(right, stream); //range
}
#include "Node.h"

#include <string>
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

    tryTranslate(left, stream, namespace_so_far); //object
    *stream << ".";
    tryTranslate(right, stream, namespace_so_far); //property
}

void ArrayAccess::MACRO_TRANS_SIG_IMP {
    //example
    // barrelShifter bs[4]; // bs --> object , 4 --> width

    // ArrayAccess(Expression* object, Expression* width) {
    //             this->left = object; this->right = width;
    //         }

    tryTranslate(left, stream, namespace_so_far); //object
    *stream << "[";
    tryTranslate(right, stream, namespace_so_far); //width
    *stream << "]";
}

void RangeAccess::MACRO_TRANS_SIG_IMP {
    //example : 
    // arr[3:4]; //arr--> object , 3:4-->range

    // RangeAccess(Expression* object, Range* range) {
    //             this->left = object; this->right = range;
    //         }


    tryTranslate(left, stream, namespace_so_far); //object
    tryTranslate(right, stream, namespace_so_far); //range
}
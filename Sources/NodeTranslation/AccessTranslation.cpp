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

    tryTranslate(left, stream, namespaceSoFar); //object
    *stream << ".";
    tryTranslate(right, stream, namespaceSoFar); //property
}

void ArrayAccess::MACRO_TRANS_SIG_IMP {
    //example 1
    //barrelShifter bs[4]; // bs --> object , 4 --> width

    // bool index = true; --> // If false, translation should treat this as a namespace array access
    // ArrayAccess(Expression* object, Expression* width) {
    //             this->left = object; this->right = width;
    //         }

    //example 2
    //a[3].k[0] --> ns_0A3 k[0]

    if(index==false){
        //array -> treat this as a namespace
        auto width = static_cast<Expression*>(right);
        auto value = width->value.value();
        namespaceSoFar = namespaceSoFar + "_0A" + std::to_string(value.getLimitedValue());
        tryTranslate(right, stream, namespaceSoFar); 
    }else{
        tryTranslate(left, stream, namespaceSoFar); //object
        *stream << "[";
        tryTranslate(right, stream, namespaceSoFar); //width
        *stream << "]";
    }
    
}

void RangeAccess::MACRO_TRANS_SIG_IMP {
    //example : 
    // arr[3:4]; //arr--> object , 3:4-->range

    // RangeAccess(Expression* object, Range* range) {
    //             this->left = object; this->right = range;
    //         }


    tryTranslate(left, stream, namespaceSoFar); //object
    tryTranslate(right, stream, namespaceSoFar); //range
}
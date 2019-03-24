#include "Node.h"
#include <string>
#include <fstream>

using namespace Phi::Node;

void NondeclarativeAssignment::translate (std::ofstream* stream){
    // example 
    // a = b;
    // a: Nondeclarative::lhs, b: expression

    // struct Nondeclarative: public Statement {
    //         Expression* lhs;
    // }

    // struct NondeclarativeAssignment: public Nondeclarative {
    //         Expression* expression;
    // }

    tryTranslate(lhs, stream); 
    *stream << "=";
    tryTranslate(expression, stream); 
    *stream << ";";


    tryTranslate(right, stream); 
}


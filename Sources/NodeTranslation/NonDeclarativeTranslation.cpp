#include "Node.h"
#include <string>
#include <fstream>

using namespace Phi::Node;

void NondeclarativeAssignment::translate (std::ofstream* stream){
    // example 
    // a = b;
    // a: Nondeclarative::lhs, b: expression

    // struct Statement: public Node {
    //         optional<std::string> annotation = nullopt;
    //         bool insideCombBlock = false;
    //     };

    // struct Nondeclarative: public Statement {
    //         Expression* lhs;
    // }

    // struct NondeclarativeAssignment: public Nondeclarative {
    //         Expression* expression;
    // }

    
    unless (Statement::inComb) {
        *stream << "assign ";
    }

    tryTranslate(lhs, stream); 
    *stream << " = ";
    tryTranslate(expression, stream); 
    *stream << ";";
    *stream << std::endl;

    tryTranslate(right, stream); 
}


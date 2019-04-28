#include "Node.h"

#include <string>
#include <fstream>

using namespace Phi::Node;

void NondeclarativeAssignment::MACRO_TRANS_SIG_IMP {
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

    
    if (!Statement::inComb) {
        *stream << "assign ";
    }

    tryTranslate(lhs, stream, namespace_so_far); 
    *stream << " = ";
    tryTranslate(expression, stream, namespace_so_far); 
    *stream << ";";
    *stream << std::endl;

    tryTranslate(right, stream, namespace_so_far); 
}


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

    if (!skipTranslation) {
        if (!Statement::inComb) {
            *stream << "assign ";
        }

        tryTranslate(lhs, stream, namespaceSoFar); 
        *stream << " = ";
        tryTranslate(expression, stream, namespaceSoFar); 
        *stream << ";";
        *stream << std::endl;
    }

    tryTranslate(right, stream, namespaceSoFar); 
}

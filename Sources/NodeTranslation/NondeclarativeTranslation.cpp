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
        if (!inComb) {
            *stream << "assign ";
        }

        tryTranslate(lhxe, stream, namespaceSoFar, indent); 
        *stream << " = ";
        tryTranslate(expression, stream, namespaceSoFar, indent); 
        *stream << ";";
        *stream << MACRO_EOL;
    }
}


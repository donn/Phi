#include "Node.h"

#include <string>
#include <fstream>

using namespace Phi::Node;

void If::MACRO_TRANS_SIG_IMP {
}

void ForLoop::MACRO_TRANS_SIG_IMP {
}

void Namespace::MACRO_TRANS_SIG_IMP {
}

void Switch::MACRO_TRANS_SIG_IMP {
}

void Combinational::MACRO_TRANS_SIG_IMP {
    // always_comb in systemverilog

    // struct BlockBased: public Statement {
    //         Statement* contents;
    // }

    *stream << "always @* \n";
    *stream << "begin \n";
    tryTranslate(contents, stream);
    *stream << "end \n";

    
    tryTranslate(right, stream);
}

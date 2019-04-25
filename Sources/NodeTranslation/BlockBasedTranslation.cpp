#include "Node.h"

#include <string>
#include <fstream>

using namespace Phi::Node;

void If::MACRO_TRANS_SIG_IMP {
    /*
    struct Statement: public Node {
            optional<std::string> annotation = nullopt;
            bool inComb = false;
    };
    struct BlockBased: public Statement {
            Statement* contents;
    };
    struct If: public BlockBased {
            Expression* expression;
            If* elseBlock;
    };
    */

    //think of it as if{} else, not if{}else{}
    //translate iff inside comb block
    if(Statement::inComb){
        if((expression!=NULL) )
        {
            *stream << "if ( ";
            tryTranslate(expression, stream);
            *stream << " ) ";
        }   
        *stream << "{";
        tryTranslate(contents, stream);
        *stream << "}";

        if (elseBlock!=NULL) {
            *stream << " else ";
            tryTranslate(elseBlock, stream);
        }
    }
}

void ForLoop::MACRO_TRANS_SIG_IMP {
    //translate iff inside comb block
    if(Statement::inComb){

    }
}

void Namespace::MACRO_TRANS_SIG_IMP {
}

void Switch::MACRO_TRANS_SIG_IMP {
    //always inside comb block
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

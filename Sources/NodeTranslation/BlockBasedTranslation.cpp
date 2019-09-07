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
    if (Statement::inComb) {
        if (expression) {
            *stream << "if ( ";
            tryTranslate(expression, stream, namespaceSoFar, indent);
            *stream << " ) ";
        }   
        *stream << "begin ";
        *indent += 1;
        *stream << MACRO_EOL;
        tryTranslate(contents, stream, namespaceSoFar, indent);
        *indent -= 1;
        *stream << "end ";

        if (elseBlock) {
            *stream << "else ";
            tryTranslate(elseBlock, stream, namespaceSoFar, indent);
        }
    }
    

    tryTranslate(right, stream, namespaceSoFar, indent);
}

void ForLoop::MACRO_TRANS_SIG_IMP {
    tryTranslate(right, stream, namespaceSoFar, indent);
}

void Namespace::MACRO_TRANS_SIG_IMP {
    //adjust namespace
    namespaceSoFar = namespaceSoFar.length() == 0 ?
        identifier->idString :
        namespaceSoFar + "." + identifier->idString
    ;
    tryTranslate(contents, stream, namespaceSoFar, indent);
    tryTranslate(right, stream, namespaceSoFar, indent);
}

void Switch::MACRO_TRANS_SIG_IMP {
    /*
    struct LabeledStatementList: public Node {
            bool isDefault; // Is this the default case in a switch statement?
            Expression* expression;
            SpecialNumber* specialNumber;
            Statement* statements;
    };

    struct Switch: public BlockBased {
            Expression* expression;
            LabeledStatementList* list;
    };
    */

   
   /* 
   example in verilog:
    
    case (address) //address -> expression

    //list 
    2'b00 : statement1; 
    2'b01, 2'b10 : statement2; 
    default : statement3; 

    endcase
   */


    //note:
    //http://www.sunburst-design.com/papers/CummingsSNUG1999Boston_FullParallelCase.pdf
    //Guideline: Do not use casex for synthesizable code [2].
    //So, we decided to use casez instead of using casex
    

    //always inside comb block
    *stream << "casez";
    *stream << "(";
    tryTranslate(expression, stream, namespaceSoFar, indent);
    *stream << " ) ";
    *indent += 1;
    *stream << MACRO_EOL;
    tryTranslate(list, stream, namespaceSoFar, indent);
    *indent -= 1;
    *stream << "endcase";

    tryTranslate(right, stream, namespaceSoFar, indent);
}

void Combinational::MACRO_TRANS_SIG_IMP {
    // always_comb in systemverilog

    // struct BlockBased: public Statement {
    //         Statement* contents;
    // }

    *stream << MACRO_EOL;
    *stream << MACRO_EOL;

    *indent += 1;
    *stream << "always @* begin " << MACRO_EOL;
        tryTranslate(contents, stream, namespaceSoFar, indent);
    *indent -= 1;
    *stream << "end ";

    *stream << MACRO_EOL;
    *stream << MACRO_EOL;
    
    tryTranslate(right, stream, namespaceSoFar, indent);
}
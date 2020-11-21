#include "Node.h"

#include <string>
#include <fstream>

using namespace Phi::Node;

void If::MACRO_TRANS_SIG_IMP {
    /*
    struct Statement: public Node {
            optional<std::string> annotation = nullopt;
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
    if (inComb) {
        if (expression) {
            *stream << "if ( ";
            tryTranslate(expression, stream, namespaceSoFar, indent);
            *stream << " ) ";
        }   
        *stream << "begin ";
        
        MACRO_INDENT;
            tryTranslate(contents, stream, namespaceSoFar, indent);
        MACRO_DEDENT;

        *stream << "end ";

        if (elseBlock) {
            *stream << "else ";
            tryTranslate(elseBlock, stream, namespaceSoFar, indent);
        }
    }
}

void ForLoop::MACRO_TRANS_SIG_IMP {} // Notto Yetto

void Namespace::MACRO_TRANS_SIG_IMP {
    //adjust namespace
    namespaceSoFar = namespaceSoFar.length() == 0 ?
        *identifier :
        namespaceSoFar + "." + *identifier
    ;
    tryTranslate(contents, stream, namespaceSoFar, indent);
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

    MACRO_INDENT;
        tryTranslate(list, stream, namespaceSoFar, indent);
    MACRO_DEDENT;

    *stream << "endcase";
}

void Combinational::MACRO_TRANS_SIG_IMP {
    // always_comb in systemverilog

    // struct BlockBased: public Statement {
    //         Statement* contents;
    // }

    *stream << MACRO_EOL;
    *stream << MACRO_EOL;

    *stream << "always @* begin ";
    
    MACRO_INDENT;
        tryTranslate(contents, stream, namespaceSoFar, indent);
    MACRO_DEDENT;
    
    *stream << "end ";

    *stream << MACRO_EOL;
    *stream << MACRO_EOL;
}
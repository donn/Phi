#include "Node.h"
#include <iostream>
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
            tryTranslate(expression, stream, namespaceSoFar);
            *stream << " ) ";
        }   
        *stream << "begin";
        tryTranslate(contents, stream, namespaceSoFar);
        *stream << "end";

        if (elseBlock!=NULL) {
            *stream << " else ";
            tryTranslate(elseBlock, stream, namespaceSoFar);
        }
    }

    tryTranslate(right, stream, namespaceSoFar);
}

void ForLoop::MACRO_TRANS_SIG_IMP {
    //translate iff inside comb block
    if(Statement::inComb){

    }

    tryTranslate(right, stream, namespaceSoFar);
}

void Namespace::MACRO_TRANS_SIG_IMP {
    //adjust namespace
    namespaceSoFar = namespaceSoFar + "_" + std::to_string((identifier->idString).length()) + identifier->idString; 
    tryTranslate(contents, stream, namespaceSoFar);
    tryTranslate(right, stream, namespaceSoFar);
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
    tryTranslate(expression, stream, namespaceSoFar);
    *stream << " ) ";
    *stream << "\n";
    tryTranslate(list, stream, namespaceSoFar);
    *stream << "endcase";
    *stream << "\n";
    

    tryTranslate(right, stream, namespaceSoFar);
}

void Combinational::MACRO_TRANS_SIG_IMP {
    // always_comb in systemverilog

    // struct BlockBased: public Statement {
    //         Statement* contents;
    // }

    *stream << "always @* \n";
    *stream << "begin \n";
    tryTranslate(contents, stream, namespaceSoFar);
    *stream << "end \n";

    
    tryTranslate(right, stream, namespaceSoFar);
}
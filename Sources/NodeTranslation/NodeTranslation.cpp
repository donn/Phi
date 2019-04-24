#include "Node.h"

#include <fstream>

using namespace Phi::Node;

void ErrorNode::MACRO_TRANS_SIG_IMP {
}

void SpecialNumber::MACRO_TRANS_SIG_IMP {
    //why it called SpecialNumber?
    //because of the existence of z in number like 0101010z0101

    // example
    // case 32b0101010z.. 
    // description --> 32: numBits, 2 equivalent to b:radix, 0101010z..: number


    //note about radix:
    // radix is uint. So, it needs to be converted to character
    // 2 --> b
    // 8 --> o
    // 10 --> d
    // 16 --> h

    // struct SpecialNumber: public Node {
    //             unsigned int numBits;
    //             uint8 radix;
    //             std::string number;
    // }

    *stream << numBits;
    *stream << "'";
    switch(SpecialNumber::radix){
        case 2:
            *stream << "b";
            break;
        case 8:
            *stream << "o";
            break;
        case 10:
            *stream << "d";
            break;
        case 16:
            *stream << "h";
            break;
        default:
            break;
    }
    *stream << number;
    *stream << ";";
    
    tryTranslate(right, stream);
}

void LabeledStatementList::MACRO_TRANS_SIG_IMP {
    // note about LabeledStatementList --> relate to CASE only

    //example1:
    //case expression: 
    // statement1 
    // statment2

    //example2:
    //case 3b010100x00:  // 3b010100x00 --> SpecialNumber
    // statement1 
    // statment2

    // struct LabeledStatementList: public Node {
    //         bool isDefault; // Is this the default case in a switch statement?
    //         Expression* expression;
    //         SpecialNumber* specialNumber;

    //         Statement* statements;
    // }

    //note:
    //http://www.sunburst-design.com/papers/CummingsSNUG1999Boston_FullParallelCase.pdf
    //Guideline: Do not use casex for synthesizable code [2].
    //So, we decided to use casez instead of using casex
    
    /*
    example of case in verilog:
        case (case_expression)
            case_item1 : case_item_statement1;
            case_item2 : case_item_statement2;
            case_item3 : case_item_statement3;
            case_item4 : case_item_statement4;
            default    : case_item_statement5;
        endcase
    */

    if(isDefault){
        //default case 
        *stream << "default: ";
    }else{
        //not default case
        if(LabeledStatementList::specialNumber!=nullptr){
            //casez specialNumber
            *stream << "casez: ";
        }else{
            //case expression
            *stream << "case: ";
        }
    }
    
    tryTranslate(statements, stream); 
}

void Range::MACRO_TRANS_SIG_IMP {
    // example
    //  3:4 
    //  description--> 3: from, 4: to

    // Range(Expression* from, Expression* to) {
    //                 this->left = from; this->right = to;
    //             }

    *stream << "[";
    tryTranslate(from, stream); //from
    *stream << ":";
    tryTranslate(to, stream); //to
    *stream << "]";
}



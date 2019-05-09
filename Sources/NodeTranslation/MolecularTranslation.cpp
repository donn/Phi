#include "Node.h"

#include <string>
#include <fstream>

using namespace Phi::Node;

void Identifier::MACRO_TRANS_SIG_IMP {
    *stream << "\\" << namespaceSoFar << idString << " ";
}

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
    
    tryTranslate(right, stream, namespaceSoFar);
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

    
    /*
    example of case in verilog:
        
        example 1) case_item1 : case_item_statement1;
        example 2) default    : case_item_statement5;
    */

    if (isDefault) {
        //default case 
        *stream << "default:";
        *stream << "\n";
    } else {
        //not default case
        if(LabeledStatementList::specialNumber!=nullptr){
            //special number 
            
            //convert each ? to z
            std::string modified_specialNumber = specialNumber->number;
            for(size_t i = 0; i<(specialNumber->number).size(); i++){
                if(specialNumber->number[i]=='?'){
                    modified_specialNumber[i]='z';
                }else{
                    modified_specialNumber[i]= specialNumber->number[i];
                }
            }
            *stream << modified_specialNumber;
            *stream << ":";
            *stream << "\n";
        }else{
            //expression 
            tryTranslate(expression, stream, namespaceSoFar); 
            *stream << ":";
            *stream << "\n";
        }
    }
    tryTranslate(statements, stream, namespaceSoFar); 
    tryTranslate(right, stream, namespaceSoFar); 
}

void Range::MACRO_TRANS_SIG_IMP {
    // example
    //  3:4 
    //  description--> 3: from, 4: to

    // Range(Expression* from, Expression* to) {
    //                 this->left = from; this->right = to;
    //             }

    *stream << "[";
    tryTranslate(from, stream, namespaceSoFar); //from
    *stream << ":";
    tryTranslate(to, stream, namespaceSoFar); //to
    *stream << "]";
}

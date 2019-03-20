#include "Node.h"
#include <string>
#include <fstream>

using namespace Phi::Node;

void Literal::translate (std::ofstream* stream){
    // examples: 0, 21, 32b1010010101.., 32d3, ...

    //assume: all literals will be represented in hexadecimal
    *stream << numBits << "'h" <<  value.value().toString(16, false);
}

void Identifier::translate (std::ofstream* stream){
    //variable name 

    *stream << identifier;
}

void Unary::translate (std::ofstream* stream){

    if(operation == Unary::Operation::negate){
            //two's complement
            *stream << "-";
            *stream << " ( ";
            right->translate(stream);
            *stream << " ) ";
    }else if(operation == Unary::Operation::bitwiseNot){
            //one's complement 
            *stream << "~";
            *stream << " ( ";
            right->translate(stream);
            *stream << " ) ";
    }else if (operation == Unary::Operation::allAnd){
            //handled in elaboration phase
            //do nothing 
    }else if(operation == Unary::Operation::allOr){
            //handled in elaboration phase
            //do nothing 
    }
}

void Binary::translate (std::ofstream* stream){

// note:
//     $Signed in :
//     1- signed shift: shiftRightArithmetic
//     2- signed comparsions: greaterThan, lessThan, greaterThanOrEqual, lessThanOrEqual

    if( 
        (operation == Binary::Operation::shiftRightArithmetic) || 
        (operation == Binary::Operation::greaterThan) || 
        (operation == Binary::Operation::lessThan) ||
        (operation == Binary::Operation::greaterThanOrEqual) ||
        (operation == Binary::Operation::lessThanOrEqual) 
    ){
        //handle if the operation is for signed numbers
        *stream << "$Signed(";
        left->translate(stream);
        *stream << ")";
    }else{
        left->translate(stream);
    }
    

    if(operation == Binary::Operation::equal){
        *stream << "==";
    }else if(operation == Binary::Operation::notEqual){
        *stream << "!=";
    }else if(operation == Binary::Operation::greaterThan){
        *stream << ">";
    }else if(operation == Binary::Operation::lessThan){
        *stream << "<";
    }else if(operation == Binary::Operation::greaterThanOrEqual){
        *stream << ">=";
    }else if(operation == Binary::Operation::lessThanOrEqual){
        *stream << "<=";
    }else if(operation == Binary::Operation::unsignedLessThan){
        *stream << "<";
    }else if(operation == Binary::Operation::unsignedGreaterThan){
        *stream << ">";
    }else if(operation == Binary::Operation::unsignedLessThanOrEqual){
        *stream << "<=";
    }else if(operation == Binary::Operation::unsignedGreaterThanOrEqual){
        *stream << ">=";
    }else if(operation == Binary::Operation::plus){
        *stream << "+";
    }else if(operation == Binary::Operation::minus){
        *stream << "-";
    }else if(operation == Binary::Operation::unsignedPlus){
        *stream << "+";
    }else if(operation == Binary::Operation::unsignedMinus){
        *stream << "-";
    }else if(operation == Binary::Operation::mul){
        *stream << "*";
    }else if(operation == Binary::Operation::div){
        *stream << "/";
    }else if(operation == Binary::Operation::modulo){
        *stream << "%";
    }else if(operation == Binary::Operation::bitwiseOr){
        *stream << "|";
    }else if(operation == Binary::Operation::bitwiseAnd){
        *stream << "&";
    }else if(operation == Binary::Operation::bitwiseXor){
        *stream << "^";
    }else if(operation == Binary::Operation::shiftLeftLogical){
        *stream << "<<";
    }else if(operation == Binary::Operation::shiftRightLogical){
        *stream << ">>>";
    }else if(operation == Binary::Operation::shiftRightArithmetic){
        *stream << ">>";
    }


    if( 
        (operation == Binary::Operation::shiftRightArithmetic) || 
        (operation == Binary::Operation::greaterThan) || 
        (operation == Binary::Operation::lessThan) ||
        (operation == Binary::Operation::greaterThanOrEqual) ||
        (operation == Binary::Operation::lessThanOrEqual) 
    ){
        //handle if the operation is for signed numbers
        *stream << "$Signed(";
        right->translate(stream);
        *stream << ")";
    }else{
        right->translate(stream);
    }
    

}

void Ternary::translate (std::ofstream* stream){

     condition->translate(stream);
     *stream << "?";
     left->translate(stream);
     *stream << ":";
     right->translate(stream);
     *stream << " ) ";
     *stream << " ; ";
}

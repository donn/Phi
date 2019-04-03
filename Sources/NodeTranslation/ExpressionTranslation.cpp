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
            *stream << "(";
            tryTranslate(right, stream);
            *stream << ")";
    }else if(operation == Unary::Operation::bitwiseNot){
            //one's complement
            *stream << "~";
            *stream << "(";
            tryTranslate(right, stream);
            *stream << ")";
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
        *stream << "$signed(";
        tryTranslate(left, stream);
        *stream << ")";
    }else{
        tryTranslate(left, stream);
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
        *stream << "$signed(";
        tryTranslate(right, stream);
        *stream << ")";
    }else{
        tryTranslate(right, stream);
    }
}

void Ternary::translate (std::ofstream* stream){

     *stream << "(";
     tryTranslate(condition, stream);
     *stream << ")";

     *stream << " ";
     *stream << "?";
     *stream << " ";
     *stream << "(";
     tryTranslate(left, stream);
     *stream << ")";
     
     *stream << " ";
     *stream << ":";
     *stream << " ";
     *stream << "(";
     tryTranslate(right, stream);
     *stream << ")";
}

void RepeatConcatenation::translate (std::ofstream* stream){
    //example in phi : [2 [[3'b110]] ]
    //example in verilog : {2 {3'b110} }

    *stream << "{";
    tryTranslate(left, stream); //repeatCount

    *stream << "{";
    tryTranslate(right, stream); // repeatable
    *stream << "}";

    *stream << "}";
}

void Concatenation::translate (std::ofstream* stream){
    //example: {2'b11, 4'h8}

    *stream << "{";
    tryTranslate(left, stream); 
    *stream << ",";
    tryTranslate(right, stream); 
    *stream << "}";
}
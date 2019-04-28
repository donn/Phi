#include "Node.h"

#include <string>
#include <fstream>

using namespace Phi::Node;

void Literal::MACRO_TRANS_SIG_IMP {
    // examples: 0, 21, 32b1010010101.., 32d3, ...

    //assume: all literals will be represented in hexadecimal
    *stream << numBits << "'h" <<  value.value().toString(16, false);
}

void IdentifierExpression::MACRO_TRANS_SIG_IMP {
    //variable name
    tryTranslate(identifier, stream, namespace_so_far);
}

void Unary::MACRO_TRANS_SIG_IMP {

    if(operation == Unary::Operation::negate){
            //two's complement
            *stream << "-";
            *stream << "(";
            tryTranslate(right, stream, namespace_so_far);
            *stream << ")";
    }else if(operation == Unary::Operation::bitwiseNot){
            //one's complement
            *stream << "~";
            *stream << "(";
            tryTranslate(right, stream, namespace_so_far);
            *stream << ")";
    }else if (operation == Unary::Operation::allAnd){
            for(int i=0; i<Expression::numBits; i++){
            tryTranslate(right, stream, namespace_so_far);
            *stream << "[";
            *stream << i;
            *stream << "]";
            if(i<(Expression::numBits-1))
                *stream << "&";
        }
    }else if(operation == Unary::Operation::allOr){
        for(int i=0; i<Expression::numBits; i++){
            tryTranslate(right, stream, namespace_so_far);
            *stream << "[";
            *stream << i;
            *stream << "]";
            if(i<(Expression::numBits-1))
                *stream << "|";
        }
            
    }

}

void Binary::MACRO_TRANS_SIG_IMP {

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
        tryTranslate(left, stream, namespace_so_far);
        *stream << ")";
    }else{
        tryTranslate(left, stream, namespace_so_far);
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
        tryTranslate(right, stream, namespace_so_far);
        *stream << ")";
    }else{
        tryTranslate(right, stream, namespace_so_far);
    }
}

void Ternary::MACRO_TRANS_SIG_IMP {

     *stream << "(";
     tryTranslate(condition, stream, namespace_so_far);
     *stream << ")";

     *stream << " ";
     *stream << "?";
     *stream << " ";
     *stream << "(";
     tryTranslate(left, stream, namespace_so_far);
     *stream << ")";
     
     *stream << " ";
     *stream << ":";
     *stream << " ";
     *stream << "(";
     tryTranslate(right, stream, namespace_so_far);
     *stream << ")";
}

void RepeatConcatenation::MACRO_TRANS_SIG_IMP {
    //example in phi : [2 [[3'b110]] ]
    //example in verilog : {2 {3'b110} }

    *stream << "{";
    tryTranslate(left, stream, namespace_so_far); //repeatCount

    *stream << "{";
    tryTranslate(right, stream, namespace_so_far); // repeatable
    *stream << "}";

    *stream << "}";
}

void Multiplexer::MACRO_TRANS_SIG_IMP{
    /*
    Multiplexer(Expression* selection, ExpressionPair* options) {
                this->left = selection; this->right = options;
            }
    */

    /*
    assign q = ( select == 0 )? d[0] : ( select == 1 )? d[1] : ( select == 2 )? d[2] : d[3];
    */
   
    // PII
    ExpressionPair* cur = (ExpressionPair*)right;
    Expression* selection = (Expression*)left;
    while(cur != NULL) {
        *stream << "(";
        tryTranslate(selection, stream, namespace_so_far); 
        *stream << "=";
        tryTranslate(cur->label, stream, namespace_so_far);
        *stream << ")";
        *stream << "?";
        tryTranslate(cur->result, stream, namespace_so_far);
        *stream << ":";

        //get next node
        cur = (ExpressionPair*)cur->right;
    }
    //cur == NULL
    //last node
    tryTranslate(cur->result, stream, namespace_so_far); 
}

#define LOCAL_CONCATDEF(x) void x::MACRO_TRANS_SIG_IMP {\
    \
    *stream << "{";\
    tryTranslate(left, stream, namespace_so_far); \
    *stream << ",";\
    tryTranslate(right, stream, namespace_so_far); \
    *stream << "}";\
}

LOCAL_CONCATDEF(LHConcatenation);
LOCAL_CONCATDEF(Concatenation);
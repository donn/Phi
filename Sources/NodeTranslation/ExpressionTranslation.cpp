#include "Node.h"

#include <string>
#include <fstream>
#include <ios>
#include <sstream>

using namespace Phi::Node;

void Literal::MACRO_TRANS_SIG_IMP {
    // examples: 0, 21, 32b1010010101.., 32d3, ...

    //assume: all literals will be represented in hexadecimal
    *stream << numBits << "'h" <<  value.value().toString(16, false);
}

void IdentifierExpression::MACRO_TRANS_SIG_IMP {
    //variable name
    tryTranslate(identifier, stream, namespaceSoFar, indent);
}

void Unary::MACRO_TRANS_SIG_IMP {

    if (operation == Unary::Operation::negate) {
        //two's complement
        *stream << "-";
        *stream << "(";
        tryTranslate(right, stream, namespaceSoFar, indent);
        *stream << ")";
    } else if (operation == Unary::Operation::bitwiseNot) {
        //one's complement
        *stream << "~";
        *stream << "(";
        tryTranslate(right, stream, namespaceSoFar, indent);
        *stream << ")";
    } else if (operation == Unary::Operation::allAnd) {
        *stream << "(";
        *stream << "~(";
        tryTranslate(right, stream, namespaceSoFar, indent);
        *stream << ") == 0";
        *stream << ")";
    } else if (operation == Unary::Operation::allOr) {
        *stream << "((";
        tryTranslate(right, stream, namespaceSoFar, indent);
        *stream << ") != 0";
        *stream << ")";
    }

}

void Binary::MACRO_TRANS_SIG_IMP {

    // note:
    //     $Signed in :
    //     1- signed shift: shiftRightArithmetic
    //     2- signed comparsions: greaterThan, lessThan, greaterThanOrEqual, lessThanOrEqual

    if ( 
        (operation == Binary::Operation::shiftRightArithmetic) || 
        (operation == Binary::Operation::greaterThan) || 
        (operation == Binary::Operation::lessThan) ||
        (operation == Binary::Operation::greaterThanOrEqual) ||
        (operation == Binary::Operation::lessThanOrEqual) 
    ) {
        //handle if the operation is for signed numbers
        *stream << "$signed(";
        tryTranslate(left, stream, namespaceSoFar, indent);
        *stream << ")";
    } else{
        tryTranslate(left, stream, namespaceSoFar, indent);
    }
    

    if (operation == Binary::Operation::equal) {
        *stream << "==";
    } else if (operation == Binary::Operation::notEqual) {
        *stream << "!=";
    } else if (operation == Binary::Operation::greaterThan) {
        *stream << ">";
    } else if (operation == Binary::Operation::lessThan) {
        *stream << "<";
    } else if (operation == Binary::Operation::greaterThanOrEqual) {
        *stream << ">=";
    } else if (operation == Binary::Operation::lessThanOrEqual) {
        *stream << "<=";
    } else if (operation == Binary::Operation::unsignedLessThan) {
        *stream << "<";
    } else if (operation == Binary::Operation::unsignedGreaterThan) {
        *stream << ">";
    } else if (operation == Binary::Operation::unsignedLessThanOrEqual) {
        *stream << "<=";
    } else if (operation == Binary::Operation::unsignedGreaterThanOrEqual) {
        *stream << ">=";
    } else if (operation == Binary::Operation::plus) {
        *stream << "+";
    } else if (operation == Binary::Operation::minus) {
        *stream << "-";
    } else if (operation == Binary::Operation::unsignedPlus) {
        *stream << "+";
    } else if (operation == Binary::Operation::unsignedMinus) {
        *stream << "-";
    } else if (operation == Binary::Operation::mul) {
        *stream << "*";
    } else if (operation == Binary::Operation::div) {
        *stream << "/";
    } else if (operation == Binary::Operation::modulo) {
        *stream << "%";
    } else if (operation == Binary::Operation::bitwiseOr) {
        *stream << "|";
    } else if (operation == Binary::Operation::bitwiseAnd) {
        *stream << "&";
    } else if (operation == Binary::Operation::bitwiseXor) {
        *stream << "^";
    } else if (operation == Binary::Operation::shiftLeftLogical) {
        *stream << "<<";
    } else if (operation == Binary::Operation::shiftRightLogical) {
        *stream << ">>>";
    } else if (operation == Binary::Operation::shiftRightArithmetic) {
        *stream << ">>";
    }


    if ( 
        (operation == Binary::Operation::shiftRightArithmetic) || 
        (operation == Binary::Operation::greaterThan) || 
        (operation == Binary::Operation::lessThan) ||
        (operation == Binary::Operation::greaterThanOrEqual) ||
        (operation == Binary::Operation::lessThanOrEqual) 
    ) {
        //handle if the operation is for signed numbers
        *stream << "$signed(";
        tryTranslate(right, stream, namespaceSoFar, indent);
        *stream << ")";
    } else{
        tryTranslate(right, stream, namespaceSoFar, indent);
    }
}

void RepeatConcatenation::MACRO_TRANS_SIG_IMP {
    //example in phi : [2 [[3'b110]] ]
    //example in verilog : {2 {3'b110} }

    *stream << "{";
    tryTranslate(left, stream, namespaceSoFar, indent); //repeatCount

    *stream << "{";
    tryTranslate(right, stream, namespaceSoFar, indent); // repeatable
    *stream << "}";

    *stream << "}";
}

void Multiplexer::MACRO_TRANS_SIG_IMP{
    /*
    Multiplexer(Expression* selection, ExpressionPair* options) {
                this->left = selection; this->right = options;
            }
    */
   
    // PII
    auto cur = std::static_pointer_cast<ExpressionPair>(right);
    auto selection = std::static_pointer_cast<Expression>(left);
    while (cur != NULL) {
        if (cur->right) {
            *stream << "(";
            if (cur->label) {
                tryTranslate(selection, stream, namespaceSoFar, indent);
                *stream << " == ";
                tryTranslate(cur->label, stream, namespaceSoFar, indent);
            } else {
                // We need to account for all possibilities...
                // The problem is, "inside", which is a part SystemVerilog doesn't work in iverilog
                // Thus, for expressions with less than 256 possibilities, we will MANUALLY expand them
                // PII
                auto number = cur->specialNumber->number;
                auto radix = cur->specialNumber->radix;
                auto radixString = (radix == 2) ? "b" : (radix == 16) ? "h" : "o";
                size_t count = 0;
                for (size_t i = 0; i < number.size(); i += 1) {
                    if (number[i] == '?') {
                        count += 1;
                    }
                }

                #define output_sn(numStr) *stream << cur->specialNumber->numBits << "'" << radixString << numStr << " "

                double possibilitiesF = pow(cur->specialNumber->radix, count);
                if (possibilitiesF > 256) {
                    tryTranslate(selection, stream, namespaceSoFar, indent);
                    *stream << "inside ";
                    *stream << "{ ";
                    output_sn(number);
                    *stream << "} ";
                } else {
                    auto positions = std::vector<size_t>();
                    for (size_t i = 0; i < number.size(); i += 1) {
                        if (number[i] == '?') {
                            positions.push_back(i);
                        }
                    }
                    size_t possibilities = possibilitiesF;
                    for (size_t i = 0; i < possibilities; i += 1) {
                        auto currentPossibility = llvm::APInt(count, i).toString(radix, false);
                        if (currentPossibility.length() < count) {
                            currentPossibility = std::string(count - currentPossibility.length(), '0') + currentPossibility;
                        }
                        auto numberCopy = number;
                        for (size_t j = 0; j < currentPossibility.size(); j += 1) {
                            numberCopy[positions[j]] = currentPossibility[j];
                        }
                        *stream << "( ";
                        tryTranslate(selection, stream, namespaceSoFar, indent);
                        *stream << "== ";
                        output_sn(numberCopy);
                        *stream << ") ";
                        if (i + 1 != possibilities) {
                            *stream << "|| ";
                        }
                    }
                }
            }
            *stream << ") ";
            *stream << "? ";
        }
        tryTranslate(cur->result, stream, namespaceSoFar, indent);
        if (cur->right) {
            *stream << ": ";
        }
        
        cur = std::static_pointer_cast<ExpressionPair>(cur->right);
    }
}

#define LOCAL_CONCATDEF(x) void x::MACRO_TRANS_SIG_IMP {\
    \
    *stream << "{ ";\
    tryTranslate(left, stream, namespaceSoFar, indent); \
    *stream << ", ";\
    tryTranslate(right, stream, namespaceSoFar, indent); \
    *stream << "} ";\
}

LOCAL_CONCATDEF(LHConcatenation)
LOCAL_CONCATDEF(Concatenation)
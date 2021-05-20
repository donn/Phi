#include "Node.h"

#include <string>
#include <fstream>
#include <ios>
#include <sstream>
#include <cmath>
#include <unordered_map>

using namespace Phi::Node;

static void expandSpecialNumberEquivalence(std::shared_ptr<Expression> lhs, std::shared_ptr<SpecialNumber> specialNumber, MACRO_TRANS_PARAMS) {
    // We need to account for all possibilities...
    // The problem is "inside", which is a part of canon SystemVerilog
    // doesn't work in iverilog, the defacto standard OS Verilog compiler.

    // Thus, for expressions with less than 256 possibilities,
    // we will MANUALLY expand them.

    // Else, we will just give up and use inside.
    
    // Use a better SV compiler. Or something.

    // PII
    auto number = specialNumber->number;
    auto radix = specialNumber->radix;
    auto radixString = (radix == 2) ? "b" : (radix == 16) ? "h" : "o";
    size_t count = 0;
    for (size_t i = 0; i < number.size(); i += 1) {
        if (number[i] == '?') {
            count += 1;
        }
    }

    #define output_sn(numStr) *stream << specialNumber->numBits << "'" << radixString << numStr << " "

    double possibilitiesF = pow(specialNumber->radix, count);
    if (possibilitiesF > 256) {
        tryTranslate(lhs, stream, namespaceSoFar, indent);
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
            auto currentPossibility = llvm::APInt(log2(radix), i).toString(radix, false);
            if (currentPossibility.length() < count) {
                currentPossibility = std::string(count - currentPossibility.length(), '0') + currentPossibility;
            }
            auto numberCopy = number;
            for (size_t j = 0; j < currentPossibility.size(); j += 1) {
                numberCopy[positions[j]] = currentPossibility[j];
            }
            *stream << "( ";
            tryTranslate(lhs, stream, namespaceSoFar, indent);
            *stream << "== ";
            output_sn(numberCopy);
            *stream << ") ";
            if (i + 1 != possibilities) {
                *stream << "|| ";
            }
        }
    }

    #undef output_sn
}

void Literal::MACRO_TRANS_SIG_IMP {
    // examples: 0, 21, 32b1010010101.., 32d3, ...

    // all literals will be represented in hexadecimal.
    *stream << numBits << "'h" <<  value.value().toString(16, false);
}

void IdentifierExpression::MACRO_TRANS_SIG_IMP {
    tryTranslate(identifier, stream, namespaceSoFar, indent);
}

void LHExpressionEncapsulator::MACRO_TRANS_SIG_IMP {
    // PII
    auto accessTuple = lhx->accessList();
    auto accesses = std::get<0>(accessTuple);
    auto accessFrom = std::get<1>(accessTuple);
    auto accessTo = std::get<2>(accessTuple);

    std::stringstream tempStream;
    tempStream << "\\";
    
    for (auto it = accesses.begin(); it != accesses.end(); it++) {
        auto& access= *it;
        if (access.type == SymbolTable::Access::Type::index) {
            if (*access.arrayIndex) {
                tempStream << ".";
                tempStream << access.index;
            } else {
                tempStream << " ";
                tempStream << "[";
                tempStream << access.index;
                tempStream << "]";
            }
        }
        if (access.type == SymbolTable::Access::Type::id) {
            if (it != accesses.begin()) {
                tempStream << ".";
            }
            tempStream << access.id;
        }
    }
  
    tempStream << " ";
    if (accessFrom.has_value()) {
        tempStream << "[" << accessFrom.value() << ":" << accessTo.value() << "]";
    tempStream << " ";
    }

    *stream << tempStream.str();
}

void LHExpressionEvaluator::MACRO_TRANS_SIG_IMP {
    tryTranslate(lhxe, stream, namespaceSoFar, indent);
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
    using O = Operation;

    if (operation == O::equal) {
        auto specialNumber = std::dynamic_pointer_cast<SpecialNumber>(right);
        if (specialNumber != nullptr) {
            expandSpecialNumberEquivalence(std::static_pointer_cast<Expression>(left), specialNumber, stream, namespaceSoFar, indent);
            return;
        }
    }

    // note:
    //     $Signed in :
    //     1- signed shift: shiftRightArithmetic
    //     2- signed comparsions: greaterThan, lessThan, greaterThanOrEqual, lessThanOrEqual
    std::set<O> signedOperations = {
        O::shiftRightArithmetic,
        O::greaterThan,
        O::lessThan,
        O::greaterThanOrEqual,
        O::lessThanOrEqual
    };

    auto signedOperation = signedOperations.find(operation) != signedOperations.end();

    if (signedOperation) {
        //handle if the operation is for signed numbers
        *stream << "$signed(";
        tryTranslate(left, stream, namespaceSoFar, indent);
        *stream << ")";
    } else{
        tryTranslate(left, stream, namespaceSoFar, indent);
    }

    std::unordered_map<
        O,
        std::string
    > verilogOperator = {
        {O::equal, "=="},
        {O::notEqual, "!="},
        {O::greaterThan, ">"},
        {O::lessThan, "<"},
        {O::greaterThanOrEqual, ">="},
        {O::lessThanOrEqual, "<="},
        {O::unsignedLessThan, "<"},
        {O::unsignedGreaterThan, ">"},
        {O::unsignedLessThanOrEqual, "<="},
        {O::unsignedGreaterThanOrEqual, ">="},
        {O::plus, "+"},
        {O::minus, "-"},
        {O::unsignedPlus, "+"},
        {O::unsignedMinus, "-"},
        {O::mul, "*"},
        {O::div, "/"},
        {O::modulo, "%"},
        {O::bitwiseAnd, "&"},
        {O::bitwiseOr, "|"},
        {O::bitwiseXor, "^"},
        {O::shiftLeftLogical, "<<"},
        {O::shiftRightLogical, ">>"},
        {O::shiftRightArithmetic, ">>"}
    };
    *stream << verilogOperator[operation];

    if (signedOperation) {
        //handle if the operation is for signed numbers
        *stream << "$signed(";
        tryTranslate(right, stream, namespaceSoFar, indent);
        *stream << ")";
    } else {
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
        if (cur->next) {
            *stream << "(";
            if (cur->label) {
                tryTranslate(selection, stream, namespaceSoFar, indent);
                *stream << " == ";
                tryTranslate(cur->label, stream, namespaceSoFar, indent);
            } else {
                expandSpecialNumberEquivalence(selection, cur->specialNumber, stream, namespaceSoFar, indent);
            }
            *stream << ") ";
            *stream << "? ";
        }
        tryTranslate(cur->result, stream, namespaceSoFar, indent);
        if (cur->next) {
            *stream << ": ";
        }
        
        cur = std::static_pointer_cast<ExpressionPair>(cur->next);
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

#undef LOCAL_CONCATDEF
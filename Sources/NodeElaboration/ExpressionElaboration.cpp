#include "Node.h"

#include "SymbolTable.h"

#include <stack>
#include <regex>
#include <sstream>

using namespace Phi::Node;

// Numbers
SpecialNumber::SpecialNumber(const char* interpretablePtr) {
    auto interpretable = std::string(interpretablePtr);
    auto regex = std::regex("([0-9]+)([bodxh])([A-F0-9x]+)");

    auto match = std::smatch();
    std::regex_match(interpretable, match, regex); // If it doesn't match, the regex here and in the .l file are mismatched.

    auto prospectiveWidth = std::stoi(match[1]);
    if (prospectiveWidth < 0 || prospectiveWidth > maxAccessWidth) {
        throw "expr.tooWide";
    }

    std::string radixCharacter = match[2];
    uint8_t radix;

    switch(radixCharacter[0]) {
        case 'b':
            radix = 2;
            break;
        case 'o':
            radix = 8;
            break;
        case 'd':
            throw "specialNumber.decimalNotAllowed";
            break;
        case 'x':
            radix = 16;
            break;
        case 'h':
            throw "fixedNumber.usedVerilogH";
            break;
        default:
            throw "FATAL";
    }
}

Literal::Literal(const char* interpretablePtr, bool widthIncluded) {
    auto interpretable = std::string(interpretablePtr);
    type = Type::CompileTime;

    if (!widthIncluded) {
        auto ref = llvm::StringRef(interpretable);
        numBits = 32;
        value = llvm::APInt(32, ref, 10);
    } else {
        auto regex = std::regex("([0-9]+)([bodxh])([A-F0-9]+)");
        
        auto match = std::smatch();
        std::regex_match(interpretable, match, regex); // If it doesn't match, the regex here and in the .l file are mismatched.

        auto prospectiveWidth = std::stoi(match[1]);
        if (prospectiveWidth < 0 || prospectiveWidth > maxAccessWidth) {
            throw "expr.tooWide";
        }

        numBits = prospectiveWidth;

        std::string radixCharacter = match[2];
        uint8_t radix;

        switch(radixCharacter[0]) {
            case 'b':
                radix = 2;
                break;
            case 'o':
                radix = 8;
                break;
            case 'd':
                radix = 10;
                break;
            case 'x':
                radix = 16;
                break;
            case 'h':
                throw "fixedNumber.usedVerilogH";
                break;
            default:
                throw "FATAL";
        }
        auto ref = llvm::StringRef(match[3]);
        value = llvm::APInt(prospectiveWidth, ref, radix);
    }
}

void LHExpression::MACRO_ELAB_SIG_IMP {
    
}

std::vector<Phi::SymbolTable::Access> LHExpression::accessList(AccessWidth* from, AccessWidth* to) {
    using PSA = Phi::SymbolTable::Access;
    std::vector<PSA> vector;

    std::stack<LHExpression*> lhStack;
    lhStack.push(this);

    while (!lhStack.empty()) {
        auto top = lhStack.top();
        lhStack.pop();
        if (top->left && top->right) {
            lhStack.push((LHExpression*)top->right);
            lhStack.push((LHExpression*)top->left);
            continue;
        }
        assert(!top->left && !top->right);
        if (auto pointer = dynamic_cast<IdentifierExpression*>(top)) {
            vector.push_back(PSA::ID(&pointer->identifier->idString));
        } else if (auto pointer = dynamic_cast<Expression*>(top)) {
            if (pointer->type == Type::Error) {
                throw "access.errorValue";
            }
            if (pointer->type == Type::RunTime) {
                throw "access.runTimeValue";
            }
            AccessWidth value = pointer->value.value().getLimitedValue();
            if (value > maxAccessWidth) {
                throw "access.maxWidthExceeded";
            }
            vector.push_back(PSA::Index(value));
        } else if (auto pointer = dynamic_cast<Range*>(top)) {
            // Elaborating on a range should have checked this by now
            assert(!(pointer->from->type == Expression::Type::RunTime || pointer->to->type == Expression::Type::RunTime));
            assert(!(pointer->from->type == Expression::Type::Error || pointer->to->type == Expression::Type::Error));

            auto toValue = pointer->to->value.value().getLimitedValue();
            auto fromValue = pointer->from->value.value().getLimitedValue();

            // Elaborating on a range should have checked this as well
            assert(toValue <= maxAccessWidth);
            assert(fromValue <= maxAccessWidth);

            if (!lhStack.empty()) {
                throw "driven.rangeAccessIsFinal";
            }

            if (from) {
                *from = fromValue;
            }
            if (to) {
                *to = toValue;
            }

            // vector.push_back(PSA::Range(toValue, fromValue));
        }
    }

    return vector;
}
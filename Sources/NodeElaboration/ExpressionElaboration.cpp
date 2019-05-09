#include "Node.h"
#include "Context.h"

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

std::vector<Phi::SymbolTable::Access> LHExpression::accessList(optional<AccessWidth>* from, optional<AccessWidth>* to) {
    using PSA = Phi::SymbolTable::Access;
    std::vector<PSA> vector;

    std::stack< std::pair<Node*, Node*> > lhStack; // pair<child, parent>
    lhStack.push(std::pair(this, nullptr));

    while (!lhStack.empty()) {
        auto topWithParent = lhStack.top();
        auto top = topWithParent.first;
        auto parent = topWithParent.second;
        lhStack.pop();
        if (top->left && top->right) {
            // Traverse leaves from left to right, so left goes onto the stack last
            lhStack.push(std::pair(top->right, top));
            lhStack.push(std::pair(top->left, top));
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
            auto parentAsArrayAccess = static_cast<ArrayAccess*>(parent);
            vector.push_back(PSA::Index(value, &parentAsArrayAccess->index));
        } else if (auto pointer = dynamic_cast<Range*>(top)) {
            // Elaborating on a range should have checked this by now
            AccessWidth fromValue, toValue;
            pointer->getValues(&fromValue, &toValue);

            if (!lhStack.empty()) {
                throw "driven.rangeAccessIsFinal";
            }

            *from = fromValue;
            *to = toValue;

            // vector.push_back(PSA::Range(toValue, fromValue));
        }
    }

    return vector;
}

void LHExpression::MACRO_ELAB_SIG_IMP {
    tryElaborate(left, context);
    tryElaborate(right, context);
}

void LHExpression::lhDrivenProcess(Node* suspect, Phi::SymbolTable* table) {
    auto lh = dynamic_cast<LHExpression*>(suspect);
    if (!lh) { return; }

    // Get and verify symbol
    optional<AccessWidth> from, to;
    auto accesses = lh->accessList(&from, &to);
    auto symbol = table->find(&accesses, &from, &to);

    if (!symbol.has_value()) {
        throw "symbol.dne";
    }

    auto unwrapped = symbol.value();
    auto driven = std::dynamic_pointer_cast<Phi::Driven>(unwrapped);

    if (!driven) {
        throw "symbol.notADriven";
    }

    if (!from.has_value() || !to.has_value()) {
        assert(!from.has_value() && !to.has_value());

        from = driven->from;
        to = driven->to;
    }

    auto fromUnwrapped = from.value();
    auto toUnwrapped = to.value();

    lh->numBits = driven->msbFirst ? (fromUnwrapped - toUnwrapped + 1) : (toUnwrapped - fromUnwrapped + 1);

    // Check if RunTime
    if (dynamic_cast<Port*>(driven->declarator)) {
        lh->type = Expression::Type::RunTime;
        return;
    }
    if (auto dli = dynamic_cast<DeclarationListItem*>(driven->declarator)) {
        if (dli->type != VariableLengthDeclaration::Type::var) {
            lh->type = Expression::Type::RunTime;
            return;
        }
    }

    if (lh->numBits == 1) {
        auto index = fromUnwrapped;
        auto range = driven->checkRangeCoverage(index);
        if (!range.has_value()) {
            throw "driven.usedBeforeInitializing";
        }
        auto unwrapped = range.value();
        lh->type = unwrapped.expression->type;
        if (lh->type == Expression::Type::CompileTime) {
            auto trueIndex = driven->msbFirst ? index - range->to : index - range->from;
            auto value = unwrapped.expression->value.value();
            lh->value = value.extractBits(1, trueIndex);
        }
    } else {
        auto rangeList = driven->checkRangeCoverage(fromUnwrapped, toUnwrapped);
        if (rangeList.size() == 0) {
            throw "driven.usedBeforeInitializing";
        }

        lh->type = Expression::Type::CompileTime;
        
        AccessWidth fromSoFar = fromUnwrapped, toSoFar = toUnwrapped;
        AccessWidth bitCount = 0;
        AccessWidth fullCount;
        llvm::APInt value;

        // TODO: Transform to concatenation!
        if (driven->msbFirst) {
            fullCount = fromSoFar - toSoFar + 1;
            value = llvm::APInt(fullCount, 0);

            for (auto& r: rangeList) {
                if (r.expression->type > lh->type) {
                    lh->type = r.expression->type;
                }
                if (r.expression->type == Expression::Type::CompileTime &&
                    fromSoFar <= r.from && fromSoFar >= r.to) {
                    auto cap = std::max(toSoFar, r.to);
                    auto trueFrom = fromSoFar - r.to;
                    auto trueTo = cap - r.to;
                    auto trueCount = trueFrom - trueTo + 1;
                    
                    auto extractedBits = r.expression->value.value().
                        extractBits(trueCount, trueTo).zext(fullCount);
                    
                    value <<= bitCount;
                    value |= extractedBits;
                    bitCount += trueCount;
                    fromSoFar = cap - 1;
                }
            }
        } else {
            fullCount = toSoFar - fromSoFar + 1;
            value = llvm::APInt(fullCount, 0);

            for (auto& r: rangeList) {
                if (r.expression->type > lh->type) {
                    lh->type = r.expression->type;
                }
                if (r.expression->type == Expression::Type::CompileTime &&
                    fromSoFar >= r.from && fromSoFar <= r.to) {
                    auto cap = std::min(toSoFar, r.to);
                    auto trueFrom = fromSoFar - r.from;
                    auto trueTo = cap - r.from;
                    auto trueCount = trueTo - trueFrom + 1;

                    auto extractedBits = r.expression->value.value().
                        extractBits(trueCount, trueFrom).zext(fullCount);
                    
                    value |= extractedBits << bitCount;
                    bitCount += trueCount;
                    fromSoFar = cap + 1;
                }
            }
        }

        if (lh->type == Expression::Type::CompileTime) {
            lh->value = value;
        }
    }
}

void Unary::MACRO_ELAB_SIG_IMP {
    tryElaborate(right, context);
    LHExpression::lhDrivenProcess(right, context->table);
    auto rightExpr = static_cast<Expression*>(right);

    // Abort if error
    if (rightExpr->type == Expression::Type::Error) {
        type = Expression::Type::Error;
        return;
    }

    numBits = rightExpr->numBits;

    if (rightExpr->type == Expression::Type::CompileTime) {
        auto rightUnwrapped = rightExpr->value.value();
        switch (operation) {
            case Operation::negate:
                rightUnwrapped.negate();
                break;
            case Operation::bitwiseNot:
                rightUnwrapped.flipAllBits();
                break;
            case Operation::allAnd:
                numBits = 1;
                rightUnwrapped = rightUnwrapped.isAllOnesValue() ? llvm::APInt(1, 1) : llvm::APInt(1, 0);
                break;
            case Operation::allOr:
                numBits = 1;
                rightUnwrapped = rightUnwrapped.isNullValue() ? llvm::APInt(1, 0) : llvm::APInt(1, 1);
                break;
        }
        type = Expression::Type::CompileTime;
        value = rightUnwrapped;
    }
    if (rightExpr->type == Expression::Type::RunTime) {
        type = Expression::Type::RunTime;
    }
}

void Binary::MACRO_ELAB_SIG_IMP {
    tryElaborate(left, context);
    tryElaborate(right, context);
    LHExpression::lhDrivenProcess(left, context->table);
    LHExpression::lhDrivenProcess(right, context->table);
    auto leftExpr = static_cast<Expression*>(left);
    auto rightExpr = static_cast<Expression*>(right);

    // Abort if error
    if (leftExpr->type == Expression::Type::Error || rightExpr->type == Expression::Type::Error) {
        type = Expression::Type::Error;
        return;
    }

    // Calculate numbits
    if (operation >= Operation::equal && operation <= Operation::unsignedGreaterThanOrEqual) {
        numBits = 1;
    } else {
        switch (operation) {
            case Operation::plus:
            case Operation::minus:
                numBits = leftExpr->numBits + 1;
                break;
            case Operation::mul:
                numBits = leftExpr->numBits + rightExpr->numBits;
            default:
                numBits = leftExpr->numBits;
        }
    }

    if (leftExpr->type == Expression::Type::CompileTime && rightExpr->type == Expression::Type::CompileTime) {
        auto leftUnwrapped = leftExpr->value.value();
        auto rightUnwrapped = rightExpr->value.value();
        if (leftExpr->numBits != rightExpr->numBits) {
            if (
                // Exceptions to the rule
                operation != Operation::mul &&
                !(operation >= Operation::shiftLeftLogical && operation <= Operation::shiftRightArithmetic)
            ) {
                throw "expr.widthMismatch";
            }
        }

        if (operation >= Operation::equal && operation <= Operation::unsignedGreaterThanOrEqual) {
            bool condition = false;
            switch (operation) {
                case Operation::equal:
                    condition = (leftUnwrapped == rightUnwrapped);
                    break;
                case Operation::notEqual:
                    condition = (leftUnwrapped != rightUnwrapped);
                    break;
                case Operation::greaterThan:
                    condition = (leftUnwrapped.sgt(rightUnwrapped));
                    break;
                case Operation::lessThan:
                    condition = (leftUnwrapped.slt(rightUnwrapped));
                    break;
                case Operation::greaterThanOrEqual:
                    condition = (leftUnwrapped.sge(rightUnwrapped));
                    break;
                case Operation::lessThanOrEqual:
                    condition = (leftUnwrapped.sle(rightUnwrapped));
                    break;
                case Operation::unsignedLessThan:
                    condition = (leftUnwrapped.ult(rightUnwrapped));
                    break;
                case Operation::unsignedGreaterThan:
                    condition = (leftUnwrapped.ugt(rightUnwrapped));
                    break;
                case Operation::unsignedLessThanOrEqual:
                    condition = (leftUnwrapped.ule(rightUnwrapped));
                    break;
                case Operation::unsignedGreaterThanOrEqual:
                    condition = (leftUnwrapped.uge(rightUnwrapped));
                    break;
                default:
                    assert(false);
            }
            value = condition ? llvm::APInt(1, 1) : llvm::APInt(1, 0);
        } else {
            auto leftCopy  = leftUnwrapped.zext(leftExpr->numBits + 1);
            auto rightCopy = rightUnwrapped.zext(rightExpr->numBits + 1);

            auto leftCopyMul = leftUnwrapped.zext(leftExpr->numBits + rightExpr->numBits);

            llvm::APInt store, garbage;
            switch (operation) {
                case Operation::plus:
                    value = leftCopy + rightCopy;
                    break;
                case Operation::minus:
                    value = leftCopy - rightCopy;
                    break;
                case Operation::unsignedPlus:
                    value = leftUnwrapped + rightUnwrapped;
                    break;
                case Operation::unsignedMinus:
                    value = leftUnwrapped - rightUnwrapped;
                    break;
                    
                case Operation::mul:
                    value = leftCopyMul * rightUnwrapped;
                    break;
                case Operation::div:
                    llvm::APInt::sdivrem(leftUnwrapped, rightUnwrapped, store, garbage);
                    value = store;
                    break;
                case Operation::modulo:
                    llvm::APInt::sdivrem(leftUnwrapped, rightUnwrapped, garbage, store);
                    value = store;
                    break;

                case Operation::bitwiseOr:
                    value = leftUnwrapped | rightUnwrapped;
                    break;
                case Operation::bitwiseAnd:
                    value = leftUnwrapped & rightUnwrapped;
                    break;
                case Operation::bitwiseXor:
                    value = leftUnwrapped ^ rightUnwrapped;
                    break;
                    
                case Operation::shiftLeftLogical:
                    value = leftUnwrapped << rightUnwrapped;
                    break;
                case Operation::shiftRightLogical:
                    value = leftUnwrapped.lshr(rightUnwrapped);
                    break;
                case Operation::shiftRightArithmetic:
                    value = leftUnwrapped.ashr(rightUnwrapped);
                    break;
                default:
                    assert(false);
            }
        }
        
        type = Expression::Type::CompileTime;
    }
    if (rightExpr->type == Expression::Type::RunTime) {
        type = Expression::Type::RunTime;
    }
}

void Concatenation::MACRO_ELAB_SIG_IMP {
    tryElaborate(left, context);
    LHExpression::lhDrivenProcess(left, context->table);
    tryElaborate(right, context);
    LHExpression::lhDrivenProcess(right, context->table);
    
    auto leftExpr = static_cast<Expression*>(left);
    auto rightExpr = static_cast<Expression*>(right);

    type = std::max(leftExpr->type, rightExpr->type);

    numBits = leftExpr->numBits + rightExpr->numBits;
}

void RepeatConcatenation::MACRO_ELAB_SIG_IMP {
    tryElaborate(left, context);
    LHExpression::lhDrivenProcess(left, context->table);
    tryElaborate(right, context);
    LHExpression::lhDrivenProcess(right, context->table);
    
    auto leftExpr = static_cast<Expression*>(left);
    auto rightExpr = static_cast<Expression*>(right);
    type = rightExpr->type;
    
    if (leftExpr->type == Type::RunTime) {
        type = Type::Error;
        throw "concat.cannotRepeatOnHardwareExpression";
    }

    numBits = leftExpr->value.value().getLimitedValue() * rightExpr->numBits;
}
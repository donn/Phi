#include "Node.h"

#include <stack>
#include <regex>
#include <sstream>
using namespace Phi::Node;

// Numbers
SpecialNumber::SpecialNumber(Location location, std::string interpretableSent): Expression(location) {
    auto interpretable = interpretableSent;
    auto regex = std::regex("([0-9]+)([bodxh])([A-F0-9?]+)");

    auto match = std::smatch();
    std::regex_match(interpretable, match, regex); // If it doesn't match, the regex here and in the .l file are mismatched.

    auto prospectiveWidth = std::stoi(match[1]);
    if (prospectiveWidth < 0 || prospectiveWidth > maxAccessWidth) {
        throw "expr.maxWidthExceeded";
    }

    numBits = prospectiveWidth;

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
    
    number = match[3];
}

Literal::Literal(Location location, std::string interpretableSent, bool widthIncluded): Expression(location) {
    auto interpretable = interpretableSent;
    type = Type::compileTime;

    if (!widthIncluded) {
        numBits = 32;
        interpretableSaved = interpretable; 
        value = llvm::APInt(32, interpretableSaved.c_str(), 10);
    } else {
        auto regex = std::regex("([0-9]+)([bodxh])([A-F0-9]+)");
        
        auto match = std::smatch();
        std::regex_match(interpretable, match, regex); // If it doesn't match, the regex here and in the .l file are mismatched.

        auto prospectiveWidth = std::stoi(match[1]);
        if (prospectiveWidth < 0 || prospectiveWidth > maxAccessWidth) {
            throw "expr.maxWidthExceeded";
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
        interpretableSaved = match[3];

        value = llvm::APInt(prospectiveWidth, interpretableSaved.c_str(), radix);
    }
}

std::tuple< std::vector<Phi::SymbolTable::Access>, optional<AccessWidth>, optional<AccessWidth> >  LHExpression::accessList() {
    using PSA = Phi::SymbolTable::Access;
    std::vector<PSA> vector;

    std::stack< std::pair< std::shared_ptr<Node> , std::shared_ptr<Node> > > lhStack; // pair<child, parent>
    lhStack.push(std::pair(shared_from_this(), nullptr));

    optional<AccessWidth> from, to;

    while (!lhStack.empty()) {
        auto topWithParent = lhStack.top();
        auto top = std::static_pointer_cast<LHExpression>(topWithParent.first);
        if (auto evaluator = std::dynamic_pointer_cast<LHExpressionEvaluator>(top)) {
            top = evaluator->lhxe;
        }
        if (auto encapsulator = std::dynamic_pointer_cast<LHExpressionEncapsulator>(top)) {
            top = encapsulator->lhx;
        }
        auto parent = topWithParent.second;
        lhStack.pop();
        if (top->left && top->right) {
            // Traverse leaves from left to right, so left goes onto the stack last
            lhStack.push(std::pair(top->right, top));
            lhStack.push(std::pair(top->left, top));
            continue;
        }
        assert(!top->left && !top->right);
        if (auto pointer = std::dynamic_pointer_cast<IdentifierExpression>(top)) {
            vector.push_back(PSA::ID(*pointer->identifier));
        } else if (auto pointer = std::dynamic_pointer_cast<Range>(top)) {
            // Elaborating on a range should have checked a number of errors by now
            auto pair = pointer->getValues();
            AccessWidth fromValue = pair.first, toValue = pair.second;

            if (!lhStack.empty()) {
                throw "driven.rangeAccessIsFinal";
            }

            from = fromValue;
            to = toValue;
        } else if (auto pointer = std::dynamic_pointer_cast<Expression>(top)) {
            if (pointer->type == Type::error) {
                throw "access.errorValue";
            }
            if (pointer->type == Type::runTime) {
                throw "access.runTimeValue";
            }
            AccessWidth value = pointer->value.value().getLimitedValue();
            if (value > maxAccessWidth) {
                throw "access.maxWidthExceeded";
            }
            auto parentAsIndexAccess = std::static_pointer_cast<IndexAccess>(parent);
            vector.push_back(PSA::Index(value, &parentAsIndexAccess->array));
        }
    }

    return {vector, from, to};
}

void LHExpression::MACRO_ELAB_SIG_IMP {
    tryElaborate(left, context);
    tryElaborate(right, context);
}

void LHExpressionEncapsulator::MACRO_ELAB_SIG_IMP {
    tryElaborate(lhx, context);
}

// This is for LHExpressions that are about to be USED AS EXPRESSIONS
void LHExpressionEvaluator::MACRO_ELAB_SIG_IMP {
    tryElaborate(lhxe, context);
    auto potential = lhxe->lhx;
    std::vector< std::shared_ptr<LHExpression> > expressions;

    AccessWidth widthSum = 0;
    Type totalType = Type::compileTime;

    if (auto concatenation = std::dynamic_pointer_cast<LHConcatenation>(potential)) {
        // Same algorithm as above, but implemented slightly differently
        std::stack< std::shared_ptr<LHExpression> > lhStack;
        lhStack.push(concatenation);
        while (!lhStack.empty()) {
            auto top = lhStack.top();
            lhStack.pop();
            if (top->left && top->right) {
                lhStack.push(std::static_pointer_cast<LHExpression>(top->right));
                lhStack.push(std::static_pointer_cast<LHExpression>(top->left));
                continue;
            }
            expressions.push_back(top);
        }
        // TO-DO: LHConcatenation runTime vs compileTime
        totalType = Type::runTime;
    } else {
        expressions.push_back(potential);
    }
    
    for (auto& lh: expressions) {
        // Get and verify symbol
        auto accessTuple = lh->accessList();
        auto accesses = std::get<0>(accessTuple);
        auto accessFrom = std::get<1>(accessTuple);
        auto accessTo = std::get<2>(accessTuple);

        auto symbolInfo = context->table->find(&accesses, accessFrom, accessTo);

        auto symbolOptional = std::get<0>(symbolInfo);
        auto from = std::get<1>(symbolInfo);
        auto to = std::get<2>(symbolInfo);

        if (!symbolOptional.has_value()) {
            throw "symbol.dne";
        }


        auto unwrapped = symbolOptional.value();
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
        if (std::dynamic_pointer_cast<Port>(driven->declarator)) {
            type = lh->type = Expression::Type::runTime;
            numBits = lh->numBits;
            return;
        }
        if (auto dli = std::dynamic_pointer_cast<DeclarationListItem>(driven->declarator)) {
            if (dli->type != VariableLengthDeclaration::Type::var) {
                type = lh->type = Expression::Type::runTime;
                numBits = lh->numBits;
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
            if (lh->type == Expression::Type::compileTime) {
                auto trueIndex = driven->msbFirst ? index - range->to : index - range->from;
                auto value = unwrapped.expression->value.value();
                lh->value = value.extractBits(1, trueIndex);
            }
        } else {
            auto rangeList = driven->checkRangeCoverage(fromUnwrapped, toUnwrapped);
            if (rangeList.size() == 0) {
                throw "driven.usedBeforeInitializing";
            }

            lh->type = Expression::Type::compileTime;
            
            AccessWidth fromSoFar = fromUnwrapped, toSoFar = toUnwrapped;
            AccessWidth bitCount = 0;
            AccessWidth fullCount;
            llvm::APInt value;

            if (driven->msbFirst) {
                fullCount = fromSoFar - toSoFar + 1;
                value = llvm::APInt(fullCount, 0);

                for (auto& r: rangeList) {
                    if (r.expression->type > lh->type) {
                        lh->type = r.expression->type;
                    }
                    if (r.expression->type == Expression::Type::compileTime &&
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
                    if (r.expression->type == Expression::Type::compileTime &&
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

            if (lh->type > totalType) {
                totalType = lh->type;
            }

            if (lh->type == Expression::Type::compileTime) {
                lh->value = value;
            }
        }

        widthSum += lh->numBits;
    }

    numBits = potential->numBits = widthSum; 
    type = potential->type;
    value = potential->value;
}



void Unary::MACRO_ELAB_SIG_IMP {
    tryElaborate(right, context);

    auto rightExpr = std::static_pointer_cast<Expression>(right);

    // Abort if error
    if (rightExpr->type == Expression::Type::error) {
        type = Expression::Type::error;
        return;
    }
    switch (operation) {
        case Operation::allAnd:
        case Operation::allOr:
            numBits = 1;
            break;
        default:
            numBits = rightExpr->numBits;
    }

    if (rightExpr->type == Expression::Type::compileTime) {
        auto rightUnwrapped = rightExpr->value.value();
        switch (operation) {
            case Operation::negate:
                rightUnwrapped.negate();
                break;
            case Operation::bitwiseNot:
                rightUnwrapped.flipAllBits();
                break;
            case Operation::allAnd:
                rightUnwrapped = rightUnwrapped.isAllOnesValue() ? llvm::APInt(1, 1) : llvm::APInt(1, 0);
                break;
            case Operation::allOr:
                rightUnwrapped = rightUnwrapped.isNullValue() ? llvm::APInt(1, 0) : llvm::APInt(1, 1);
                break;
        }
        type = Expression::Type::compileTime;
        value = rightUnwrapped;
    }
    if (rightExpr->type == Expression::Type::runTime) {
        type = Expression::Type::runTime;
    }
}

void Binary::MACRO_ELAB_SIG_IMP {
    tryElaborate(left, context);
    tryElaborate(right, context);


    auto leftExpr = std::static_pointer_cast<Expression>(left);
    auto rightExpr = std::static_pointer_cast<Expression>(right);

    // Abort if error
    if (leftExpr->type == Expression::Type::error || rightExpr->type == Expression::Type::error) {
        type = Expression::Type::error;
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
                break;
            default:
                numBits = leftExpr->numBits;
        }
    }

    if (leftExpr->type == Expression::Type::compileTime && rightExpr->type == Expression::Type::compileTime) {
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
        
        type = Expression::Type::compileTime;
    }
    type = std::max(leftExpr->type, rightExpr->type);
}

void Concatenation::MACRO_ELAB_SIG_IMP {
    tryElaborate(left, context);

    tryElaborate(right, context);

    
    auto leftExpr = std::static_pointer_cast<Expression>(left);
    auto rightExpr = std::static_pointer_cast<Expression>(right);

    type = std::max(leftExpr->type, rightExpr->type);

    // TODO: Concatenation runtime vs compiletime
    type = Type::runTime;

    numBits = leftExpr->numBits + rightExpr->numBits;
}

void RepeatConcatenation::MACRO_ELAB_SIG_IMP {
    tryElaborate(left, context);

    tryElaborate(right, context);

    
    auto leftExpr = std::static_pointer_cast<Expression>(left);
    auto rightExpr = std::static_pointer_cast<Expression>(right);
    type = rightExpr->type;
    
    if (leftExpr->type == Type::runTime) {
        type = Type::error;
        throw "concat.cannotRepeatOnHardwareExpression";
    }

    // TODO: Concatenation runtime vs compiletime
    type = Type::runTime;

    numBits = leftExpr->value.value().getLimitedValue() * rightExpr->numBits;
}

void Multiplexer::MACRO_ELAB_SIG_IMP {
    // TODO: Multiplexer runtime vs compiletime: Probably requires Selection Algorithm
    // TODO: Exhaustivity guarantees
    tryElaborate(left, context);
    tryElaborate(right, context);
    type = Type::runTime;
    inComb = context->table->findNearest(Space::Type::comb) != nullptr;

    // PII
    // Width checks on EPs
    optional<AccessWidth> labelNumbits = nullopt;
    optional<AccessWidth> resultNumbits = nullopt;
    auto c = std::static_pointer_cast<ExpressionPair>(right);
    while (c) {
        auto currentNumbits = c->specialNumber ? c->specialNumber->numBits : c->label->numBits;
        if (labelNumbits.has_value() && labelNumbits.value() != currentNumbits) {
            throw "expr.widthMismatch.conditions";
            return;
        } else {
            labelNumbits = currentNumbits;
        }
        auto currentResultNumbits = c->result->numBits;
        if (resultNumbits.has_value() && resultNumbits.value() != currentResultNumbits) {
            throw "expr.widthMismatch.results";
            return;
        } else {
            resultNumbits = currentResultNumbits;
        }
        c = std::static_pointer_cast<ExpressionPair>(c->next);
    }
    numBits = resultNumbits.value();        
}

void ExpressionPair::MACRO_ELAB_SIG_IMP {
    tryElaborate(label, context);
    tryElaborate(specialNumber, context);
    tryElaborate(result, context);
}

void ExpressionArgument::MACRO_ELAB_SIG_IMP {
    tryElaborate(argument, context);
}

void ProceduralCall::MACRO_ELAB_SIG_IMP {
    tryElaborate(left, context);
    auto function = std::static_pointer_cast<LHExpression>(left);
    auto accesses = std::get<0>(function->accessList());
    auto symbolOptional = std::get<0>(context->table->find(&accesses));
    if (!symbolOptional.has_value()) {
        throw "symbol.dne";
    }
    auto symbol = std::dynamic_pointer_cast<Function>(symbolOptional.value());

    tryElaborate(right, context);

    // PII
    std::vector<Phi::Argument> args;
    auto head = std::static_pointer_cast<Argument>(right);
    while (head) {
        if (auto stringArgument = std::dynamic_pointer_cast<StringArgument>(head)) {
            args.push_back({Phi::Argument::Type::string, stringArgument->argument, nullopt});
        } else {
            auto expressionArgument = std::static_pointer_cast<ExpressionArgument>(head);
            auto expression = expressionArgument->argument;
            if (expression->type == Type::runTime) {
                throw "arg.hardwareExpression";
            }
            if (expression->type == Type::error) {
                return;
            }
            
            args.push_back({Phi::Argument::Type::expression, nullopt, std::pair(expression->value.value(), expression->numBits)});
        }
        head = std::static_pointer_cast<Argument>(head->next);
    }
    
    auto result = symbol->call(&args);

    type = Type::compileTime;
    value = result.first;
    numBits = result.second;
}
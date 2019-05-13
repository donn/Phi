#include "Node.h"
using namespace Phi::Node;

void If::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, context);
    LHExpression::lhDrivenProcess(expression, context->table);

    auto comb = context->table->findNearest(SymbolSpace::Type::comb);
    if (comb) {
        inComb = true;
    }

    if (expression) {
        if (expression->type == Expression::Type::error) {
            return;
        }

        if (expression->numBits != 1) {
            context->addError(nullopt, "expr.notACondition");
            return;
        }

        if (!inComb) {
            if (expression->type == Expression::Type::runTime) {
                throw "expr.hardwareInIf";
                return;
            }

            auto value = expression->value.value();

            if (value == llvm::APInt(1, 1)) {
                tryElaborate(contents, context);
            } else {
                tryElaborate(elseBlock, context);
            }
        } else {
            tryElaborate(contents, context);
            tryElaborate(elseBlock, context);
        }
    } else {
        tryElaborate(contents, context);
        tryElaborate(elseBlock, context);
    }
    tryElaborate(right, context);
}

void ForLoop::MACRO_ELAB_SIG_IMP {
    context->addError(nullopt, "phi.forLoopUnsupported"); // UNSUPPORTED
    range->elaborate(context);
    if (auto comb = context->table->findNearest(SymbolSpace::Type::comb)) {
        context->addError(nullopt, "comb.forLoopNotAllowed");
    }
    // PII
    // TODO
    tryElaborate(right, context);
}

void Combinational::MACRO_ELAB_SIG_IMP {
    if (auto comb = context->table->findNearest(SymbolSpace::Type::comb)) {
        context->addError(nullopt, "comb.nestingNotAllowed");
    } else {
        context->table->stepIntoComb(this);
        tryElaborate(contents, context);
        context->table->stepOut();
        for (auto& fn: conclusionTriggers) {
            fn();
        }
    }
    tryElaborate(right, context);
}

void Namespace::MACRO_ELAB_SIG_IMP {
    if (auto comb = context->table->findNearest(SymbolSpace::Type::comb)) {
        context->addError(nullopt, "comb.declarationNotAllowed");
    } else {
        context->table->stepIntoAndCreate(identifier->idString, this);
        tryElaborate(contents, context);
        context->table->stepOut();
    }
    tryElaborate(right, context);
}

void Switch::MACRO_ELAB_SIG_IMP {
    if (auto comb = context->table->findNearest(SymbolSpace::Type::comb)) {
        tryElaborate(expression, context);
        LHExpression::lhDrivenProcess(expression, context->table);
        tryElaborate(list, context);
    } else {
        context->addError(nullopt, "decl.switchOutsideComb");
    }
}

void LabeledStatementList::MACRO_ELAB_SIG_IMP {
    tryElaborate(label, context);
    LHExpression::lhDrivenProcess(label, context->table);
    tryElaborate(specialNumber, context);
    tryElaborate(statements, context);

    tryElaborate(right, context);

    // PII
    if (right) {
        auto rightLSL = static_cast<LabeledStatementList*>(right);
        if (rightLSL->label || rightLSL->specialNumber) { // We don't need to check the current node because if it has a right, it must not be default
            auto numBitsHere = specialNumber ? specialNumber->numBits : label->numBits;
            auto numBitsThere = rightLSL->specialNumber ?
                rightLSL->specialNumber->numBits:
                rightLSL->label->numBits;
            if (numBitsHere != numBitsThere) {
                throw "expr.widthMismatch";
            }
        }

    }
}
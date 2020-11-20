#include "Node.h"
using namespace Phi::Node;

void If::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, context);
    auto comb = context->table->findNearest(Space::Type::comb);
    if (comb) {
        inComb = true;
    }

    if (expression) {
        if (expression->type == Expression::Type::error) {
            return;
        }

        if (expression->numBits != 1) {
            context->addError(location, "expr.notACondition");
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
}

void ForLoop::MACRO_ELAB_SIG_IMP {
    context->addError(location, "phi.forLoopUnsupported"); // UNSUPPORTED
    range->elaborate(context);
    if (auto comb = context->table->findNearest(Space::Type::comb)) {
        context->addError(location, "comb.forLoopNotAllowed");
    }
    // PII
    // TODO
}

void Combinational::MACRO_ELAB_SIG_IMP {
    if (auto comb = context->table->findNearest(Space::Type::comb)) {
        context->addError(location, "comb.nestingNotAllowed");
    } else {
        context->table->stepIntoComb(shared_from_this());
        tryElaborate(contents, context);
        context->table->stepOut();
        for (auto& fn: conclusionTriggers) {
            fn();
        }
    }
}

void Namespace::MACRO_ELAB_SIG_IMP {
    if (auto comb = context->table->findNearest(Space::Type::comb)) {
        context->addError(location, "comb.declarationNotAllowed");
    } else {
        context->table->stepIntoAndCreate(identifier->idString, shared_from_this());
        tryElaborate(contents, context);
        context->table->stepOut();
    }
}

void Switch::MACRO_ELAB_SIG_IMP {
    if (auto comb = context->table->findNearest(Space::Type::comb)) {
        tryElaborate(expression, context);        tryElaborate(list, context);
    } else {
        context->addError(location, "decl.switchOutsideComb");
    }
}

void LabeledStatementList::MACRO_ELAB_SIG_IMP {
    tryElaborate(label, context);
    tryElaborate(specialNumber, context);
    tryElaborate(statements, context);

    return;
    
    // THIS PART HAS TO HAPPEN *AFTER* AND I STILL HAVE NOT FIGURED OUT HOW
    // PII
    if (next) {
        auto nextLSL = std::static_pointer_cast<LabeledStatementList>(next);
        if (nextLSL->label || nextLSL->specialNumber) { // We don't need to check the current node because if it has a right, it must not be default
            auto numBitsHere = specialNumber ? specialNumber->numBits : label->numBits;
            auto numBitsThere = nextLSL->specialNumber ?
                nextLSL->specialNumber->numBits:
                nextLSL->label->numBits;
            if (numBitsHere != numBitsThere) {
                throw "expr.widthMismatch";
            }
        }

    }
}
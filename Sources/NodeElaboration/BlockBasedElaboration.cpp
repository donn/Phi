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
            throw "expr.notACondition";
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
    throw "phi.forLoopUnsupported"; // UNSUPPORTED
    //TODO
}

void Combinational::MACRO_ELAB_SIG_IMP {
    if (auto comb = context->table->findNearest(Space::Type::comb)) {
        throw "comb.nestingNotAllowed";
    }
    context->table->stepIntoComb(shared_from_this());
    tryElaborate(contents, context);
    context->table->stepOut();
    for (auto& fn: conclusionTriggers) {
        fn();
    }
}

void Namespace::MACRO_ELAB_SIG_IMP {
    if (auto comb = context->table->findNearest(Space::Type::comb)) {
       throw "comb.declarationNotAllowed";
    }
    context->table->stepIntoAndCreate(*identifier, shared_from_this());
    tryElaborate(contents, context);
    context->table->stepOut();
}

void Switch::MACRO_ELAB_SIG_IMP {
    if (auto comb = context->table->findNearest(Space::Type::comb)) {
        tryElaborate(expression, context);
        tryElaborate(list, context);

        // PII
        // Width checks on LSL
        optional<AccessWidth> labelNumbits = nullopt;
        auto c = list;
        while (c) {
            auto currentNumbits = c->specialNumber ? c->specialNumber->numBits : c->label->numBits;
            if (labelNumbits.has_value() && labelNumbits.value() != currentNumbits) {
                throw "expr.widthMismatch";
            }
            labelNumbits = currentNumbits;
            c = std::static_pointer_cast<LabeledStatementList>(c->next);
        }        
    } else {
        throw "decl.switchOutsideComb";
    }
}

void LabeledStatementList::MACRO_ELAB_SIG_IMP {
    tryElaborate(label, context);
    tryElaborate(specialNumber, context);
    tryElaborate(statements, context);
}
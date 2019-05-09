#include "Node.h"
#include "Context.h"
using namespace Phi::Node;

void If::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, context);
    LHExpression::lhDrivenProcess(expression, context->table);
    
    // If NOT in a comb block, we do need to elaborate. Otherwise... nyet.
    if (context->table->inComb()) {
        return;
    }

    if (expression->type == Expression::Type::Error) {
        //evaluation prolly failed
        return;
    }

    if (expression->type == Expression::Type::RunTime) {
        throw "elaboration.softwareExpr";
        return;
    }

    auto value = expression->value.value();

    if (expression->numBits != 1) {
        context->addError(nullopt, "expr.notACondition");
        return;
    }

    if (value == llvm::APInt(1, 1)) {
        tryElaborate(contents, context);
    } else {
        tryElaborate(elseBlock, context);
    }
    tryElaborate(right, context);
}

void ForLoop::MACRO_ELAB_SIG_IMP {
    range->elaborate(context);
    // PII
    // TODO
    tryElaborate(right, context);
}

void Combinational::MACRO_ELAB_SIG_IMP {
    context->table->stepIntoComb(this);
    tryElaborate(contents, context);
    context->table->stepOut();
    tryElaborate(right, context);
}

void Namespace::MACRO_ELAB_SIG_IMP {
    context->table->stepIntoAndCreate(identifier->idString, this);
    tryElaborate(contents, context);
    context->table->stepOut();
    tryElaborate(right, context);
}
#include <regex>
#include <sstream>

#include "Node.h"
#include "SymbolTable.h"

using namespace Phi::Node;

void Port::MACRO_ELAB_SIG_IMP {
    table->add(identifier, this);
    tryElaborate(right, table, context);
}

void TopLevelNamespace::MACRO_ELAB_SIG_IMP {
    table->stepIntoAndCreate(identifier, this);
    tryElaborate(contents, table, context);
    table->stepOut();
    tryElaborate(right, table, context);
}

void TopLevelDeclaration::MACRO_ELAB_SIG_IMP {
    table->stepIntoAndCreate(identifier, this);
    tryElaborate(ports, table, context);
    tryElaborate(contents, table, context);
    table->stepOut();
    tryElaborate(right, table, context);
}

void If::MACRO_ELAB_SIG_IMP {
    expression->elaborate(table, context);
    if (expression->type == Expression::Type::ParameterSensitive) {
        // Translate to assert/generate
        return;
    }
    
    // If NOT in a comb block, we do need to elaborate. Otherwise... nyet.
    if (table->inComb()) {
        return;
    }

    if (expression->type == Expression::Type::RunTime) {
        context->errorList.push_back({Phi::Error::emptyLocation, "elaboration.softwareExpr"});
        return;
    }

    if (!expression->value.has_value()) {
        //evaluation prolly failed
        return;
    }

    auto value = expression->value.value();
    if (expression->numBits != 1) {
        context->errorList.push_back({Phi::Error::emptyLocation, "expr.notACondition"});
        return;
    }

    if (value == llvm::APInt(1, 1, false)) {
        tryElaborate(contents, table, context);
    } else {
        tryElaborate(elseBlock, table, context);
    }
}

void ForLoop::MACRO_ELAB_SIG_IMP {
    range->elaborate(table, context);
    // NOTE: POLITICALLY INCORRECT INSPECTION
    // TODO
}

void Combinational::MACRO_ELAB_SIG_IMP {
    table->stepIntoComb(this);
    tryElaborate(contents, table, context);
    table->stepOut();
}

void Namespace::MACRO_ELAB_SIG_IMP {
    table->stepIntoAndCreate(identifier, this);
    tryElaborate(contents, table, context);
    table->stepOut();
    tryElaborate(right, table, context);
}

void VariableLengthDeclaration::MACRO_ELAB_SIG_IMP {
    tryElaborate(optionalAssignment, table, context);
    tryElaborate(array, table, context);
    tryElaborate(bus, table, context);
    table->add(identifier, this, optionalAssignment);
}
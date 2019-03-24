#include <regex>
#include <sstream>

#include "Node.h"
#include "SymbolTable.h"

using namespace Phi::Node;

void Port::MACRO_ELAB_SIG_IMP {
    table->add(identifier, this);
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void TopLevelNamespace::MACRO_ELAB_SIG_IMP {
    table->stepIntoAndCreate(identifier, this);
    tryElaborate(contents, MACRO_ELAB_ARGS);
    table->stepOut();
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void TopLevelDeclaration::MACRO_ELAB_SIG_IMP {
    table->stepIntoAndCreate(identifier, this);
    tryElaborate(ports, MACRO_ELAB_ARGS);
    tryElaborate(contents, MACRO_ELAB_ARGS);
    table->stepOut();
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void If::MACRO_ELAB_SIG_IMP {
    expression->elaborate(MACRO_ELAB_ARGS);
    
    // If NOT in a comb block, we do need to elaborate. Otherwise... nyet.
    if (table->inComb()) {
        return;
    }
    
    if (expression->type == Expression::Type::ParameterSensitive) {
        // Translate to assert/generate
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
        tryElaborate(contents, MACRO_ELAB_ARGS);
    } else {
        tryElaborate(elseBlock, MACRO_ELAB_ARGS);
    }
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void ForLoop::MACRO_ELAB_SIG_IMP {
    range->elaborate(MACRO_ELAB_ARGS);
    // NOTE: POLITICALLY INCORRECT INSPECTION
    // TODO
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void Combinational::MACRO_ELAB_SIG_IMP {
    table->stepIntoComb(this);
    tryElaborate(contents, MACRO_ELAB_ARGS);
    table->stepOut();
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void Namespace::MACRO_ELAB_SIG_IMP {
    table->stepIntoAndCreate(identifier, this);
    tryElaborate(contents, MACRO_ELAB_ARGS);
    table->stepOut();
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void VariableLengthDeclaration::MACRO_ELAB_SIG_IMP {
    tryElaborate(bus, MACRO_ELAB_ARGS);
    tryElaborate(declarationList, MACRO_ELAB_ARGS);
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void DeclarationListItem::MACRO_ELAB_SIG_IMP {
    tryElaborate(array, MACRO_ELAB_ARGS);
    tryElaborate(optionalAssignment, MACRO_ELAB_ARGS);
    table->add(identifier, this, optionalAssignment);
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void InstanceDeclaration::MACRO_ELAB_SIG_IMP {
    if (array) {
        tryElaborate(array, MACRO_ELAB_ARGS);
        if (array->type == Expression::Type::ParameterSensitive) {
            //Translate to assert and generate
        } else if (array->type == Expression::Type::RunTime) {
            context->errorList.push_back({Phi::Error::emptyLocation, "elaboration.softwareExpr"});
        } else {
            //unroll
        }
    }
    tryElaborate(module, MACRO_ELAB_ARGS);
    unless (module->leftHandExpression) {
        context->errorList.push_back({Phi::Error::emptyLocation, "identifier.rightHand"});
    }
    
    tryElaborate(parameters, MACRO_ELAB_ARGS);
    tryElaborate(ports, MACRO_ELAB_ARGS);

    tryElaborate(right, MACRO_ELAB_ARGS);
}

void ExpressionIDPair::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, MACRO_ELAB_ARGS);

    tryElaborate(right, MACRO_ELAB_ARGS);
}

void NondeclarativeAssignment::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, MACRO_ELAB_ARGS);
}

void NondeclarativePorts::MACRO_ELAB_SIG_IMP {
    tryElaborate(ports, MACRO_ELAB_ARGS);
}
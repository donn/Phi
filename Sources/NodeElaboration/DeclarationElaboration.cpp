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
    // PII
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
    // PII
    tryElaborate(bus, MACRO_ELAB_ARGS);
    declarationList->type = type;
    declarationList->bus = bus;
    tryElaborate(declarationList, MACRO_ELAB_ARGS);
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void DeclarationListItem::MACRO_ELAB_SIG_IMP {
    tryElaborate(array, MACRO_ELAB_ARGS);
    tryElaborate(optionalAssignment, MACRO_ELAB_ARGS);
    table->add(identifier, this, optionalAssignment);
    if (right) {
        auto rightDLI = (DeclarationListItem*)right;
        rightDLI->type = type;
        rightDLI->bus = bus;
        tryElaborate(right, MACRO_ELAB_ARGS);
    }
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
    // Declaration block because I'm using goto here
    std::vector<std::string> ids;
    auto pointer = lhs;
    auto identifierPointer = dynamic_cast<Identifier*>(pointer);
    auto propertyAccessPointer = dynamic_cast<PropertyAccess*>(pointer);
    Identifier* left;
    std::shared_ptr<Symbol> symbol;
    DeclarationListItem* dliAttache;
    Port* portAttache;

    tryElaborate(lhs, MACRO_ELAB_ARGS);

    unless (lhs->leftHandExpression) {
        context->errorList.push_back({Phi::Error::emptyLocation, "identifier.rightHand"});
        goto exit;
    }
    while (pointer) {
        if (identifierPointer) {
            ids.push_back(identifierPointer->identifier);
            pointer = nullptr;
        } else if (propertyAccessPointer) {
            left = (Identifier*)identifierPointer->left; // LHExpression association should mandate this
            ids.push_back(left->identifier);
            pointer = (Expression*)identifierPointer->right;
        } else {
            context->errorList.push_back({Phi::Error::emptyLocation, "identifier.invalidAccess"});
            goto exit;
        }
    }

    symbol = table->checkExistence(ids);
    unless (symbol) {
        context->errorList.push_back({Phi::Error::emptyLocation, "symbol.dne"});
        goto exit;
    }


    dliAttache = dynamic_cast<DeclarationListItem*>(symbol->attached);
    portAttache = dynamic_cast<Port*>(symbol->attached);
    unless (dliAttache && dliAttache->type == VariableLengthDeclaration::Type::wire) {
        unless (portAttache && portAttache->polarity == Port::Polarity::output) {
            context->errorList.push_back({Phi::Error::emptyLocation, "symbol.notAWire"});
            goto exit;
        }
    } 

    if (symbol->driver) {
        context->errorList.push_back({Phi::Error::emptyLocation, "symbol.driverExists"});
        goto exit;
    }

    symbol->driver = this;

    if (table->inComb()) {
        if (dliAttache) {
            dliAttache->type = VariableLengthDeclaration::Type::wire_reg;
        }
        if (portAttache) {
            portAttache->polarity = Port::Polarity::output_reg;
        }
        inComb = true;
    }

    tryElaborate(expression, MACRO_ELAB_ARGS);

exit:
    tryElaborate(right, MACRO_ELAB_ARGS);
}

void NondeclarativePorts::MACRO_ELAB_SIG_IMP {
    tryElaborate(ports, MACRO_ELAB_ARGS);
}
#include <sstream>

#include "Node.h"
#include "SymbolTable.h"

using namespace Phi::Node;

void Port::MACRO_ELAB_SIG_IMP {
    table->add(identifier->idString, this);
    tryElaborate(right, table, context);
}

void TopLevelNamespace::MACRO_ELAB_SIG_IMP {
    table->stepIntoAndCreate(identifier->idString, this);
    tryElaborate(contents, table, context);
    table->stepOut();
    tryElaborate(right, table, context);
}

void TopLevelDeclaration::MACRO_ELAB_SIG_IMP {
    table->stepIntoAndCreate(identifier->idString, this);
    table->stepIntoAndCreate("_tldPrivate", this);
    tryElaborate(ports, table, context);
    tryElaborate(contents, table, context);
    table->stepOut();
    table->stepOut();
    tryElaborate(right, table, context);
}

void If::MACRO_ELAB_SIG_IMP {
    expression->elaborate(table, context);
    
    // If NOT in a comb block, we do need to elaborate. Otherwise... nyet.
    if (table->inComb()) {
        return;
    }
    
    if (expression->type == Expression::Type::ParameterSensitive) {
        // Translate to assert/generate
        return;
    }

    if (expression->type == Expression::Type::RunTime) {
        context->addError(nullopt, "elaboration.softwareExpr");
        return;
    }

    if (!expression->value.has_value()) {
        //evaluation prolly failed
        return;
    }

    auto value = expression->value.value();
    if (expression->numBits != 1) {
        context->addError(nullopt, "expr.notACondition");
        return;
    }

    if (value == llvm::APInt(1, 1, false)) {
        tryElaborate(contents, table, context);
    } else {
        tryElaborate(elseBlock, table, context);
    }
    tryElaborate(right, table, context);
}

void ForLoop::MACRO_ELAB_SIG_IMP {
    range->elaborate(table, context);
    // PII
    // TODO
    tryElaborate(right, table, context);
}

void Combinational::MACRO_ELAB_SIG_IMP {
    table->stepIntoComb(this);
    tryElaborate(contents, table, context);
    table->stepOut();
    tryElaborate(right, table, context);
}

void Namespace::MACRO_ELAB_SIG_IMP {
    table->stepIntoAndCreate(identifier->idString, this);
    tryElaborate(contents, table, context);
    table->stepOut();
    tryElaborate(right, table, context);
}

void VariableLengthDeclaration::MACRO_ELAB_SIG_IMP {
    // PII
    tryElaborate(bus, table, context);
    declarationList->type = type;
    declarationList->bus = bus;
    tryElaborate(declarationList, table, context);
    tryElaborate(right, table, context);
}

void DeclarationListItem::MACRO_ELAB_SIG_IMP {
    tryElaborate(array, table, context);
    tryElaborate(optionalAssignment, table, context);
    table->add(identifier->idString, this, optionalAssignment);
    if (right) {
        auto rightDLI = (DeclarationListItem*)right;
        rightDLI->type = type;
        rightDLI->bus = bus;
        tryElaborate(right, table, context);
    }
}

void InstanceDeclaration::MACRO_ELAB_SIG_IMP {
    if (array) {
        tryElaborate(array, table, context);
        if (array->type == Expression::Type::ParameterSensitive) {
            //Translate to assert and generate
        } else if (array->type == Expression::Type::RunTime) {
            context->addError(nullopt, "elaboration.softwareExpr");
        } else {
            //unroll
        }
    }
    tryElaborate(module, table, context);
    
    tryElaborate(parameters, table, context);
    tryElaborate(ports, table, context);

    tryElaborate(right, table, context);
}

void ExpressionIDPair::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, table, context);

    tryElaborate(right, table, context);
}

void NondeclarativeAssignment::MACRO_ELAB_SIG_IMP {
    // Declaration block because I'm using goto here
    std::vector<std::string> ids;
    auto pointer = lhs;
    auto identifierPointer = dynamic_cast<IdentifierExpression*>(pointer);
    auto propertyAccessPointer = dynamic_cast<PropertyAccess*>(pointer);
    IdentifierExpression* left;
    std::shared_ptr<Symbol> symbol;
    DeclarationListItem* dliAttache;
    Port* portAttache;

    tryElaborate(lhs, table, context);

    while (pointer) {
        identifierPointer = dynamic_cast<IdentifierExpression*>(pointer);
        propertyAccessPointer = dynamic_cast<PropertyAccess*>(pointer);
        if (identifierPointer) {
            ids.push_back(identifierPointer->identifier->idString);
            pointer = nullptr;
        } else if (propertyAccessPointer) {
            left = (IdentifierExpression*)propertyAccessPointer->left; // LHExpression association should mandate this
            ids.push_back(left->identifier->idString);
            pointer = (LHExpression*)propertyAccessPointer->right;
        } else {
            context->addError(nullopt, "identifier.invalidAccess");
            goto exit;
        }
    }

    symbol = table->checkExistence(ids);
    if (!symbol) {
        context->addError(nullopt, "symbol.dne");
        goto exit;
    }


    dliAttache = dynamic_cast<DeclarationListItem*>(symbol->attached);
    portAttache = dynamic_cast<Port*>(symbol->attached);
    if (!(dliAttache && dliAttache->type == VariableLengthDeclaration::Type::wire)) {
        if (!(portAttache && portAttache->polarity == Port::Polarity::output)) {
            context->addError(nullopt, "symbol.notAWire");
            goto exit;
        }
    } 

    if (symbol->driver) {
        context->addError(nullopt, "symbol.driverExists");
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

    tryElaborate(expression, table, context);

exit:
    tryElaborate(right, table, context);
}

void NondeclarativePorts::MACRO_ELAB_SIG_IMP {
    tryElaborate(ports, table, context);
}
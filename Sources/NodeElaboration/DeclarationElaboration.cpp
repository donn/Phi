#include "Node.h"
#include "SymbolTable.h"
#include "Context.h"

#include <sstream>

using namespace Phi::Node;

void Port::MACRO_ELAB_SIG_IMP {
    optional<AccessWidth> to = nullopt, from = nullopt;
    
    auto pointer = std::make_shared<Driven>(identifier->idString, this);
    table->add(identifier->idString, pointer);
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
    tryElaborate(ports, table, context);
    tryElaborate(contents, table, context);
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
    DeclarationListItem* rightDLI;
    std::shared_ptr<Symbol> pointer;
    std::shared_ptr<Driven> pointerAsDriven;
    std::shared_ptr<SymbolArray> pointerAsArray;

    tryElaborate(array, table, context);
    AccessWidth size = 1;
    AccessWidth width = 1;

    bool msbFirst = true;

    optional<AccessWidth> from = nullopt;
    optional<AccessWidth> to = nullopt;

    if (array) {
        switch (array->type) {
        case Expression::Type::ParameterSensitive:
            size = 0;
            break;
        case Expression::Type::CompileTime:
            if (!Utils::apIntCheck(&array->value.value(), maxAccessWidth)) {
                context->addError(nullopt, "array.maximumExceeded");
                goto exit; 
            }
            size = array->value.value().getLimitedValue();
            if (size == 0) {
                context->addError(nullopt, "array.cannotBeZero");
                goto exit; 
            }
            break;
        case Expression::Type::RunTime:
            context->addError(nullopt, "array.hardwareExpression");
            [[fallthrough]];
        case Expression::Type::Error:
            goto exit;
            break;
        default:
            throw "FATAL";
            goto exit;
        }
    }
    tryElaborate(bus, table, context);
    if (bus) {
        if (bus->from->type == Expression::Type::Error) {
            goto exit;
        }
        from = bus->from->value.value().getLimitedValue();
        to = bus->to->value.value().getLimitedValue();

        msbFirst = from > to;

        width = (msbFirst ? (from.value() - to.value()) : (from.value() + to.value())) + 1;
    }

    tryElaborate(optionalAssignment, table, context);

    if (size == 1) {
        pointerAsDriven = std::make_shared<Driven>(identifier->idString, this, from.value(), to.value(), msbFirst);
        pointer = pointerAsDriven;
        if (optionalAssignment) {
            if (width != optionalAssignment->numBits) {
                throw "assign.widthMismatch";
            }
            pointerAsDriven->drive(optionalAssignment);
        } 
    } else {
        pointerAsArray = std::make_shared<SymbolArray>(identifier->idString, this, size);
        for (AccessWidth i = 0; i < size; i += 1) {
            pointerAsArray->array.push_back(std::make_shared<Driven>(identifier->idString, this, from.value(), to.value(), msbFirst));
        }
        pointer = pointerAsArray;
        if (optionalAssignment) {
            throw "array.inlineInitialization";
        }
    }
    table->add(identifier->idString, pointer);

exit:
    //PII
    if (right) {
        rightDLI = (DeclarationListItem*)right;
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
    //TODO: Port Checking

    tryElaborate(right, table, context);
}

void ExpressionIDPair::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, table, context);

    tryElaborate(right, table, context);
}

void NondeclarativeAssignment::MACRO_ELAB_SIG_IMP {
    // Declaration block because I'm using goto here
    std::vector<SymbolTable::Access> accesses;
    
    optional< std::shared_ptr<Symbol> > symbolOptional;
    std::shared_ptr<Symbol> symbol;
    std::shared_ptr<Driven> driven;

    DeclarationListItem* dliAttache;
    Port* portAttache;
    optional<AccessWidth> from = nullopt, to = nullopt;

    tryElaborate(lhs, table, context);

    accesses = lhs->accessList(&from, &to);
    symbolOptional = table->find(&accesses);

    if (!symbolOptional.has_value()) {
        context->addError(nullopt, "symbol.dne");
        goto exit;
    }

    symbol = symbolOptional.value();

    dliAttache = dynamic_cast<DeclarationListItem*>(symbol->declarator);
    portAttache = dynamic_cast<Port*>(symbol->declarator);
    if (!(dliAttache && dliAttache->type == VariableLengthDeclaration::Type::wire)) {
        if (!(portAttache && portAttache->polarity == Port::Polarity::output)) {
            context->addError(nullopt, "symbol.notAWire");
            goto exit;
        }
    } 

    driven = std::dynamic_pointer_cast<Driven>(symbol);

    if (!driven) {
        context->addError(nullopt, "assignment.leftHandNotDriven");
        goto exit;
    }

    if (!driven->drive(expression, from, to)) {
        context->addError(nullopt, "assignment.alreadyDriven");
    } else if (table->inComb()) {
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
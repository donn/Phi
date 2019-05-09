#include "Node.h"
#include "Context.h"
using namespace Phi::Node;

void Port::MACRO_ELAB_SIG_IMP {
    std::shared_ptr<Driven> pointer;

    optional<AccessWidth> from = nullopt;
    optional<AccessWidth> to = nullopt;
    bool msbFirst = true;

    tryElaborate(bus, context);
    if (bus) {
        if (bus->from->type == Expression::Type::Error) {
            goto exit;
        }
        from = bus->from->value.value().getLimitedValue();
        to = bus->to->value.value().getLimitedValue();

        msbFirst = from > to;
    } else {
        from = to = 0;
    }

    pointer = std::make_shared<Driven>(identifier->idString, this, from.value(), to.value(), msbFirst);
    context->table->add(identifier->idString, pointer);

exit:
    tryElaborate(right, context);
}

void TopLevelNamespace::MACRO_ELAB_SIG_IMP {
    context->table->stepIntoAndCreate(identifier->idString, this);
    tryElaborate(contents, context);
    context->table->stepOut();
    tryElaborate(right, context);
}

void TopLevelDeclaration::MACRO_ELAB_SIG_IMP {
    context->table->stepIntoAndCreate(identifier->idString, this);
    tryElaborate(ports, context);
    tryElaborate(contents, context);
    context->table->stepOut();
    tryElaborate(right, context);
}

void VariableLengthDeclaration::MACRO_ELAB_SIG_IMP {
    tryElaborate(bus, context);
    // PII
    declarationList->type = type;
    declarationList->bus = bus;
    tryElaborate(declarationList, context);
    tryElaborate(right, context);
}

void DeclarationListItem::MACRO_ELAB_SIG_IMP {
    DeclarationListItem* rightDLI;
    std::shared_ptr<Symbol> pointer;
    std::shared_ptr<Driven> pointerAsDriven;
    std::shared_ptr<SymbolArray> pointerAsArray;

    tryElaborate(array, context);
    LHExpression::lhDrivenProcess(array, context->table);

    AccessWidth size = 1;
    AccessWidth width = 1;

    bool msbFirst = true;

    optional<AccessWidth> from = nullopt;
    optional<AccessWidth> to = nullopt;

    if (array) {
        if (optionalAssignment) {
            throw "array.inlineInitialization";
        }
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
    tryElaborate(bus, context);
    if (bus) {
        if (bus->from->type == Expression::Type::Error) {
            goto exit;
        }
        from = bus->from->value.value().getLimitedValue();
        to = bus->to->value.value().getLimitedValue();

        msbFirst = from > to;

        width = (msbFirst ? (from.value() - to.value()) : (from.value() + to.value())) + 1;
    } else {
        from = to = 0;
    }

    tryElaborate(optionalAssignment, context);
    LHExpression::lhDrivenProcess(optionalAssignment, context->table);

    if (size == 1) {
        pointerAsDriven = std::make_shared<Driven>(identifier->idString, this, from.value(), to.value(), msbFirst);
        pointer = pointerAsDriven;
        if (optionalAssignment) {
            if (width != optionalAssignment->numBits) {
                throw "driving.widthMismatch";
            }
            pointerAsDriven->drive(optionalAssignment);
        } 
    } else {
        pointerAsArray = std::make_shared<SymbolArray>(identifier->idString, this, size);
        for (AccessWidth i = 0; i < size; i += 1) {
            pointerAsArray->array.push_back(std::make_shared<Driven>(identifier->idString, this, from.value(), to.value(), msbFirst));
        }
        pointer = pointerAsArray;
    }
    context->table->add(identifier->idString, pointer);

exit:
    //PII
    if (right) {
        rightDLI = (DeclarationListItem*)right;
        rightDLI->type = type;
        rightDLI->bus = bus;
        tryElaborate(right, context);
    }
}

void InstanceDeclaration::MACRO_ELAB_SIG_IMP {
    if (array) {
        tryElaborate(array, context);
        if (array->type == Expression::Type::ParameterSensitive) {
            //Translate to assert and generate
        } else if (array->type == Expression::Type::RunTime) {
            context->addError(nullopt, "elaboration.softwareExpr");
        } else {
            //unroll
        }
    }
    tryElaborate(module, context);
    
    tryElaborate(parameters, context);
    tryElaborate(ports, context);
    //TODO: Port Checking

    tryElaborate(right, context);
}

void ExpressionIDPair::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, context);

    tryElaborate(right, context);
}
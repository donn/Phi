#include "Node.h"
using namespace Phi::Node;

void NondeclarativeAssignment::MACRO_ELAB_SIG_IMP {
    using VLD = VariableLengthDeclaration;
    // Declaration block because I'm using goto here
    std::vector<SymbolTable::Access> accesses;
    
    optional< std::shared_ptr<Symbol> > symbolOptional;
    std::shared_ptr<Symbol> symbol;
    std::shared_ptr<Driven> driven;

    DeclarationListItem* dliAttache;
    Port* portAttache;
    optional<AccessWidth> from = nullopt, to = nullopt;
    AccessWidth width;

    // Elaborate left hand side
    tryElaborate(lhs, context);

    // Find and get symbol
    accesses = lhs->accessList(&from, &to);
    symbolOptional = context->table->find(&accesses, &from, &to);

    if (!symbolOptional.has_value()) {
        context->addError(nullopt, "symbol.dne");
        goto exit;
    }

    symbol = symbolOptional.value();

    // Perform checks on lhs
    dliAttache = dynamic_cast<DeclarationListItem*>(symbol->declarator);
    portAttache = dynamic_cast<Port*>(symbol->declarator);

    driven = std::dynamic_pointer_cast<Driven>(symbol);

    if (!driven) {
        context->addError(nullopt, "driving.notDrivable");
        goto exit;
    }

    // Elaborate on right hand side
    tryElaborate(expression, context);
    LHExpression::lhDrivenProcess(expression, context->table);

    if (dliAttache) {
        if (dliAttache->type == VLD::Type::var) {
            skipTranslation = true;
            if (expression->type != Expression::Type::CompileTime) {
                context->addError(nullopt, "driving.hardwareDominance");
                goto exit;
            }
        } else if (dliAttache->type == VLD::Type::reg) {
            skipTranslation = true;
        } else if (dliAttache->type == VLD::Type::latch) {
            context->addError(nullopt, "driving.latchNoReset");
            goto exit;
        }
    }

    if (portAttache) {
        if (portAttache->polarity == Port::Polarity::input) {
            context->addError(nullopt, "driving.inputWire");
            goto exit;
        }
    }
    
    if (!from.has_value() || !to.has_value()) {
        assert(!from.has_value() && !to.has_value());

        from = driven->from;
        to = driven->to;
    }

    width = driven->msbFirst ? from.value() - to.value() + 1 : to.value() - from.value() + 1;

    if (width != expression->numBits) {
        context->addError(nullopt, "driving.widthMismatch");
    }
    if (!driven->drive(expression, from, to)) {
        context->addError(nullopt, "assignment.alreadyDriven");
    } else if (context->table->inComb()) {
        if (dliAttache) {
            dliAttache->type = VLD::Type::wire_reg;
        }
        if (portAttache) {
            portAttache->polarity = Port::Polarity::output_reg;
        }
        inComb = true;
    }

    if (dliAttache && dliAttache->type == VLD::Type::reg) {
        dliAttache->optionalAssignment = expression;
    }

exit:
    tryElaborate(right, context);
}

void NondeclarativePorts::MACRO_ELAB_SIG_IMP {
    optional<AccessWidth> trash;
    std::vector<SymbolTable::Access> accesses;
    
    optional< std::shared_ptr<Symbol> > symbolOptional;
    std::shared_ptr<Symbol> symbol;

    InstanceDeclaration* declarator;

    tryElaborate(lhs, context);
    accesses = lhs->accessList(&trash, &trash);
    symbolOptional = context->table->find(&accesses, &trash, &trash);
    if (!symbolOptional.has_value()) {
        context->addError(nullopt, "symbol.dne");
        goto exit;
    }
    symbol = symbolOptional.value();

    declarator = dynamic_cast<InstanceDeclaration*>(symbol->declarator);

    if (!declarator) {
        context->addError(nullopt, "driving.notAnInstance");
        goto exit;
    }

    tryElaborate(ports, context);
    declarator->ports = ports;

exit:
    tryElaborate(right, context);
}
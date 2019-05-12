#include "Node.h"
using namespace Phi::Node;

void NondeclarativeAssignment::MACRO_ELAB_SIG_IMP {
    using VLD = VariableLengthDeclaration;
    // Declaration block because I'm using goto here
    std::vector<SymbolTable::Access> accesses;
    
    optional< std::shared_ptr<Symbol> > symbolOptional;
    std::shared_ptr<Symbol> symbol;
    std::shared_ptr<SymbolSpace> comb;
    std::shared_ptr<Driven> driven;
    std::shared_ptr<Container> container = nullptr;

    Combinational* combDeclarator;
    DeclarationListItem* dliAttache;
    Port* portAttache;

    optional<AccessWidth> from = nullopt, to = nullopt;
    AccessWidth width;

    bool elevate = false;

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
            if (expression->type != Expression::Type::compileTime) {
                context->addError(nullopt, "driving.hardwareDominance");
                goto exit;
            }
        } else if (dliAttache->type == VLD::Type::reg) {
            if ((container = std::dynamic_pointer_cast<Container>(driven))){
                skipTranslation = true;
                driven = std::dynamic_pointer_cast<Driven>(container->space["_0R"]);
                elevate = true;
            }
        } else if (dliAttache->type == VLD::Type::latch) {
            if ((container = std::dynamic_pointer_cast<Container>(driven))){
                context->addError(nullopt, "driving.latchNoReset");
            }
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

    if (!driven->drive(expression, from, to, true)) {
        context->addError(nullopt, "assignment.alreadyDriven");
        goto exit;
    } 

    if ((comb = context->table->findNearest(SymbolSpace::Type::comb))) {
        combDeclarator = static_cast<Combinational*>(comb->declarator);
        if (dliAttache) {
            dliAttache->type = VLD::Type::wire_reg;
        }
        if (portAttache) {
            portAttache->polarity = Port::Polarity::output_reg;
        }
        inComb = true;
        combDeclarator->conclusionTriggers.push_back([=](){
            //Unsafe allocation
            auto exp = Expression::abstract(Expression::Type::runTime, width);
            if (!driven->drive(exp, from, to)) { // If we accidentally created another unneeded one for this comb block, just deallocate
                delete exp;
            }
        });
    } else {
        driven->drive(expression, from, to);
    }

    if (elevate) {
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

    if (auto comb = context->table->findNearest(SymbolSpace::Type::comb)) {
        context->addError(nullopt, "comb.declarationNotAllowed");
    } else {
        tryElaborate(ports, context);
    }

    
    declarator->ports = ports;

exit:
    tryElaborate(right, context);
}
#include "Node.h"
using namespace Phi::Node;

void NondeclarativeAssignment::drivingAssignment(Context* context, std::shared_ptr<LHExpression> lhs, std::shared_ptr<Expression> expression, bool* skipTranslation, bool* inComb) {
    using VLD = VariableLengthDeclaration;
    std::vector<SymbolTable::Access> accesses;
    
    optional< std::shared_ptr<Symbol> > symbolOptional;
    std::shared_ptr<Symbol> symbol;
    std::shared_ptr<SymbolSpace> comb;
    std::shared_ptr<Driven> driven;
    std::shared_ptr<Container> container = nullptr;

    std::shared_ptr<Combinational> combDeclarator;
    std::shared_ptr<DeclarationListItem> dliAttache;
    std::shared_ptr<Port> portAttache;

    optional<AccessWidth> from = nullopt, to = nullopt;
    AccessWidth width;

    bool elevate = false;

    // Find and get symbol
    accesses = lhs->accessList(&from, &to);
    symbolOptional = context->table->find(&accesses, &from, &to);

    if (!symbolOptional.has_value()) {
        throw "symbol.dne";
    }

    symbol = symbolOptional.value();

    // Perform checks on lhs
    dliAttache = std::dynamic_pointer_cast<DeclarationListItem>(symbol->declarator);
    portAttache = std::dynamic_pointer_cast<Port>(symbol->declarator);

    driven = std::dynamic_pointer_cast<Driven>(symbol);

    if (!driven) {
        throw "driving.notDrivable";
    }

    // Elaborate on right hand side

    if (dliAttache) {
        if (dliAttache->type == VLD::Type::var) {
            *skipTranslation = true;
            if (expression->type != Expression::Type::compileTime) {
                throw "driving.hardwareDominance";
            }
        } else if (dliAttache->type == VLD::Type::reg) {
            if ((container = std::dynamic_pointer_cast<Container>(driven))){
                *skipTranslation = true;
                driven = std::dynamic_pointer_cast<Driven>(container->space["_0R"]);
                elevate = true;
            }
        } else if (dliAttache->type == VLD::Type::latch) {
            if ((container = std::dynamic_pointer_cast<Container>(driven))){
                throw "driving.latchNoReset";
            }
        }
    }

    if (portAttache) {
        if (portAttache->polarity == Port::Polarity::input) {
            throw "driving.inputWire";
        }
    }
    
    if (!from.has_value() || !to.has_value()) {
        assert(!from.has_value() && !to.has_value());

        from = driven->from;
        to = driven->to;
    }

    width = driven->msbFirst ? from.value() - to.value() + 1 : to.value() - from.value() + 1;

    if (width != expression->numBits) {
        throw "driving.widthMismatch";
    }

    if (!driven->drive(expression, from, to, true)) {
        throw "assignment.alreadyDriven";
    } 

    if ((comb = context->table->findNearest(SymbolSpace::Type::comb))) {
        combDeclarator = std::static_pointer_cast<Combinational>(comb->declarator);
        if (dliAttache) {
            dliAttache->type = VLD::Type::wire_reg;
        }
        if (portAttache) {
            portAttache->polarity = Port::Polarity::output_reg;
        }
        *inComb = true;
        combDeclarator->conclusionTriggers.push_back([=](){
            //Unsafe allocation
            auto exp = Expression::abstract(Expression::Type::runTime, width);
            driven->drive(exp, from, to);
            // if (!driven->drive(exp, from, to)) { // If we accidentally created another unneeded one for this comb block, just deallocate
            //     delete exp;
            // }
        });
    } else {
        driven->drive(expression, from, to);
    }

    if (elevate) {
        dliAttache->optionalAssignment = expression;
    }
}

void NondeclarativeAssignment::MACRO_ELAB_SIG_IMP {
    tryElaborate(lhs, context);
    tryElaborate(expression, context);
    LHExpression::lhDrivenProcess(expression, context->table);
    drivingAssignment(context, lhs, expression, &skipTranslation, &inComb);
    tryElaborate(right, context);
}

void NondeclarativePorts::MACRO_ELAB_SIG_IMP {
    optional<AccessWidth> trash;
    std::vector<SymbolTable::Access> accesses;
    
    optional< std::shared_ptr<Symbol> > symbolOptional;
    std::shared_ptr<Symbol> symbol;

    std::shared_ptr<InstanceDeclaration> declarator;

    tryElaborate(lhs, context);
    accesses = lhs->accessList(&trash, &trash);
    symbolOptional = context->table->find(&accesses, &trash, &trash);
    if (!symbolOptional.has_value()) {
        throw "symbol.dne";
    }
    symbol = symbolOptional.value();

    declarator = std::dynamic_pointer_cast<InstanceDeclaration>(symbol->declarator);

    if (!declarator) {
        throw "driving.notAnInstance";
    }

    if (auto comb = context->table->findNearest(SymbolSpace::Type::comb)) {
        throw "comb.declarationNotAllowed";
    } else {
        tryElaborate(ports, context);
    }

    
    declarator->ports = ports;
    declarator->elaboratePorts(context);

    tryElaborate(right, context);
}
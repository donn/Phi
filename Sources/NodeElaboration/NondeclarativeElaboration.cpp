#include "Node.h"
using namespace Phi::Node;

void NondeclarativeAssignment::drivingAssignment(Context* context, std::shared_ptr<LHExpression> lhs, std::shared_ptr<Expression> expression, bool* skipTranslation, bool* inComb) {
    using VLD = VariableLengthDeclaration;

    std::shared_ptr<Symbol> symbol;
    std::shared_ptr<Space> comb;
    std::shared_ptr<Driven> driven;
    std::shared_ptr<Container> container = nullptr;

    std::shared_ptr<Combinational> combDeclarator;
    std::shared_ptr<DeclarationListItem> dliAttache;
    std::shared_ptr<Port> portAttache;

    AccessWidth width;

    bool elevate = false;

    // Get and verify symbol
    auto accessTuple = lhs->accessList();
    auto accesses = std::get<0>(accessTuple);
    // Phi::SymbolTable::Access::representList(&std::cout, &accesses);
    auto accessFrom = std::get<1>(accessTuple);
    auto accessTo = std::get<2>(accessTuple);

    auto symbolInfo = context->table->find(&accesses, accessFrom, accessTo);

    auto symbolOptional = std::get<0>(symbolInfo);
    auto from = std::get<1>(symbolInfo);
    auto to = std::get<2>(symbolInfo);


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
                driven = std::dynamic_pointer_cast<Driven>(container->space["resetValue"]);
                elevate = true;
            }
        } else if (dliAttache->type == VLD::Type::latch) {
            if ((container = std::dynamic_pointer_cast<Container>(driven))){
                throw "driving.latchNoReset";
            }
        }
    }

    if (portAttache) {
        if (portAttache->polarity ==PortObject::Polarity::input) {
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

    if ((comb = context->table->findNearest(Space::Type::comb))) {
        combDeclarator = std::static_pointer_cast<Combinational>(comb->declarator);
        if (dliAttache) {
            dliAttache->assignedInComb = true;
        }
        if (portAttache) {
            portAttache->assignedInComb = true;
        }
        *inComb = true;
        combDeclarator->conclusionTriggers.push_back([=](){
            auto exp = Expression::abstract(Expression::Type::runTime, width);
            driven->drive(exp, from, to);
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

    drivingAssignment(context, lhs, expression, &skipTranslation, &inComb);
}

void NondeclarativePorts::MACRO_ELAB_SIG_IMP {
    tryElaborate(lhs, context);
    
    auto accesses = std::get<0>(lhs->accessList());
    auto symbolOptional = std::get<0>(context->table->find(&accesses));
    if (!symbolOptional.has_value()) {
        throw "symbol.dne";
    }
    auto symbol = symbolOptional.value();

    auto declarator = std::dynamic_pointer_cast<InstanceDeclaration>(symbol->declarator);

    if (!declarator) {
        throw "driving.notAnInstance";
    }

    if (auto comb = context->table->findNearest(Space::Type::comb)) {
        throw "comb.declarationNotAllowed";
    } else {
        tryElaborate(ports, context);
    }
    
    declarator->ports = ports;
    declarator->elaboratePorts(context);
}
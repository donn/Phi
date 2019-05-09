#include "Node.h"
#include "SymbolTable.h"
#include "Context.h"
using namespace Phi::Node;

void NondeclarativeAssignment::MACRO_ELAB_SIG_IMP {
    // Declaration block because I'm using goto here
    std::vector<SymbolTable::Access> accesses;
    
    optional< std::shared_ptr<Symbol> > symbolOptional;
    std::shared_ptr<Symbol> symbol;
    std::shared_ptr<Driven> driven;

    DeclarationListItem* dliAttache;
    Port* portAttache;
    optional<AccessWidth> from = nullopt, to = nullopt;

    tryElaborate(lhs, context);

    accesses = lhs->accessList(&from, &to);
    symbolOptional = context->table->find(&accesses, &from, &to);

    if (!symbolOptional.has_value()) {
        context->addError(nullopt, "symbol.dne");
        goto exit;
    }

    symbol = symbolOptional.value();

    dliAttache = dynamic_cast<DeclarationListItem*>(symbol->declarator);
    portAttache = dynamic_cast<Port*>(symbol->declarator);

    if (dliAttache && dliAttache->type == VariableLengthDeclaration::Type::var) {
        skipTranslation = true;
    }

    driven = std::dynamic_pointer_cast<Driven>(symbol);

    if (!driven) {
        context->addError(nullopt, "assignment.leftHandNotDriven");
        goto exit;
    }

    tryElaborate(expression, context);
    LHExpression::lhDrivenProcess(expression, context->table);

    if (lhs->numBits != expression->numBits) {
        throw "driving.widthMismatch";
    }
    if (!driven->drive(expression, from, to)) {
        context->addError(nullopt, "assignment.alreadyDriven");
    } else if (context->table->inComb()) {
        if (dliAttache) {
            dliAttache->type = VariableLengthDeclaration::Type::wire_reg;
        }
        if (portAttache) {
            portAttache->polarity = Port::Polarity::output_reg;
        }
        inComb = true;
    }

exit:
    tryElaborate(right, context);
}

void NondeclarativePorts::MACRO_ELAB_SIG_IMP {
    tryElaborate(ports, context);
}
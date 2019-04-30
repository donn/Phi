#include "Node.h"

#include <regex>
#include <sstream>

using namespace Phi::Node;

Identifier::Identifier(const char* identifier) {
    idString = std::regex_replace(identifier, std::regex("_"), "__");
}

void Range::MACRO_ELAB_SIG_IMP {
    tryElaborate(from, table, context);
    tryElaborate(to, table, context);
    LHExpression::lhDrivenProcess(from, table);
    LHExpression::lhDrivenProcess(to, table);

    auto from = static_cast<Expression*>(this->from);
    auto to = static_cast<Expression*>(this->to);
    
    if (from->type == Expression::Type::RunTime || to->type == Expression::Type::RunTime) {
        throw "range.runTimeValue";
    }
    if (from->type == Expression::Type::Error || to->type == Expression::Type::Error) {
        return;
    }

    AccessWidth fromValue = from->value.value().getLimitedValue();
    AccessWidth toValue = to->value.value().getLimitedValue();

    if (toValue > maxAccessWidth || fromValue > maxAccessWidth) {
        throw "range.maxWidthExceeded";
    }
}

void Range::getValues(AccessWidth* fromRef, AccessWidth* toRef) {
    auto from = static_cast<Expression*>(this->from);
    auto to = static_cast<Expression*>(this->to);

    assert(!(from->type == Expression::Type::RunTime || to->type == Expression::Type::RunTime));
    if (from->type == Expression::Type::Error || to->type == Expression::Type::Error) {
        throw "driven.rangeError";
    }

    auto fromValue = from->value.value().getLimitedValue();
    auto toValue   = to->value.value().getLimitedValue();

    assert(toValue <= maxAccessWidth);
    assert(fromValue <= maxAccessWidth);

    *fromRef = fromValue;
    *toRef = toValue;
}
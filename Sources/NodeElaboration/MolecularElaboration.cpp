#include "Node.h"

#include <regex>
using namespace Phi::Node;

void Range::MACRO_ELAB_SIG_IMP {
    tryElaborate(from, context);
    tryElaborate(to, context);

    auto from = std::static_pointer_cast<Expression>(this->from);
    auto to = std::static_pointer_cast<Expression>(this->to);
    
    if (from->type == Expression::Type::runTime || to->type == Expression::Type::runTime) {
        throw "range.runTimeValue";
    }
    if (from->type == Expression::Type::error || to->type == Expression::Type::error) {
        return;
    }

    AccessWidth fromValue = from->value.value().getLimitedValue();
    AccessWidth toValue = to->value.value().getLimitedValue();

    if (toValue > maxAccessWidth || fromValue > maxAccessWidth) {
        throw "range.maxWidthExceeded";
    }
}

std::pair<AccessWidth, AccessWidth> Range::getValues() {
    auto from = std::static_pointer_cast<Expression>(this->from);
    auto to = std::static_pointer_cast<Expression>(this->to);

    assert(!(from->type == Expression::Type::runTime || to->type == Expression::Type::runTime));
    if (from->type == Expression::Type::error || to->type == Expression::Type::error) {
        throw "driven.rangeError";
    }

    auto fromValue = from->value.value().getLimitedValue();
    auto toValue   = to->value.value().getLimitedValue();

    assert(toValue <= maxAccessWidth);
    assert(fromValue <= maxAccessWidth);

    return std::pair(fromValue, toValue);
}

AccessWidth Range::getWidth() {
    auto pair = getValues();
    AccessWidth from = pair.first, to = pair.second;
    
    return (from < to) ? to - from + 1 : from - to + 1;
}
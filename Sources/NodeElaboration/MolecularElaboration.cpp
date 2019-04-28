#include "Node.h"

#include <regex>
#include <sstream>

using namespace Phi::Node;

Identifier::Identifier(const char* identifier) {
    idString = std::regex_replace(identifier, std::regex("_"), "__");
}

void Range::MACRO_ELAB_SIG_IMP {
    tryElaborate(left, context, table);
    tryElaborate(right, context, table);

    auto from = static_cast<Expression*>(left);
    auto to = static_cast<Expression*>(right);
    
    if (from->type == Expression::Type::RunTime || to->type == Expression::Type::RunTime) {
        throw "range.runTimeValue";
    }
    if (from->type == Expression::Type::Error || to->type == Expression::Type::Error) {
        return;
    }
    if (toValue > maxAccessWidth || fromValue > maxAccessWidth) {
        throw "range.maxWidthExceeded";
    }
}

void Range::getValues(AccessWidth* from, AccessWidth* to) {
    assert(!(pointer->from->type == Expression::Type::RunTime || pointer->to->type == Expression::Type::RunTime));
    if (pointer->from->type == Expression::Type::Error || pointer->to->type == Expression::Type::Error) {
        throw "driven.rangeError";
    }

    auto fromValue = from->value.value().getLimitedValue();
    auto toValue   = to->value.value().getLimitedValue();

    assert(toValue <= maxAccessWidth);
    assert(fromValue <= maxAccessWidth);

    *from = fromValue;
    *to = toValue;
}
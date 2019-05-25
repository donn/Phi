#include "Node.h"
#include "Context.h"

using namespace Phi::Node;

void Node::MACRO_ELAB_SIG_IMP {
    tryElaborate(right, context);
}

void Node::MACRO_TRANS_SIG_IMP {
    tryTranslate(right, stream, namespaceSoFar, indent);
}

void tryElaborate(std::shared_ptr<Node> node, MACRO_ELAB_PARAMS) {
    if (node) {
        try {
            node->elaborate(context);  
        } catch (const char* e) {
            context->addError(nullopt, e);
        }
    }
}

void tryTranslate(std::shared_ptr<Node> node, MACRO_TRANS_PARAMS) {
    if (node) {
        auto asExpr = std::dynamic_pointer_cast<Expression>(node);
        if (asExpr && asExpr->type == Expression::Type::compileTime) {
            auto str = asExpr->value.value().toString(16, false);
            *stream << asExpr->numBits << "'h" << str;
        } else {
            node->translate(stream, namespaceSoFar, indent);
        }
    }
}
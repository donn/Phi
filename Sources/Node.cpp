#include "Node.h"
#include "Context.h"

#include <llvm/ADT/StringExtras.h>

using namespace Phi::Node;

void Node::MACRO_ELAB_SIG_IMP {}
void Node::MACRO_TRANS_SIG_IMP {}

void Phi::Node::tryElaborate(std::shared_ptr<Phi::Node::Node> node, MACRO_ELAB_PARAMS) {
    while (node) {
        try {
            node->elaborate(context);  
        } catch (const char* e) {
            context->addError(node->location, e);
        }
        auto asStmt = std::dynamic_pointer_cast<Statement>(node);
        if (asStmt) {
            node = asStmt->next;
        } else {
            node = nullptr;
        }
    }
}

void Phi::Node::tryTranslate(std::shared_ptr<Phi::Node::Node> node, MACRO_TRANS_PARAMS) {
    while (node) {
        auto asExpr = std::dynamic_pointer_cast<Expression>(node);
        if (asExpr && asExpr->type == Expression::Type::compileTime) {
            auto str = llvm::toString(asExpr->value.value(), 16, false);
            *stream << asExpr->numBits << "'h" << str;
        } else {
            node->translate(stream, namespaceSoFar, indent);
        }
        auto asStmt = std::dynamic_pointer_cast<Statement>(node);
        if (asStmt) {
            node = asStmt->next;
        } else {
            node = nullptr;
        }
    }
}

std::string Port::getName() {
    return *identifier;
}

AccessWidth Port::getWidth() {
    if (bus) {
        return bus->getWidth();
    }
    return 1;
}
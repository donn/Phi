#include "Node.h"
#include "Context.h"

using namespace Phi::Node;

void Node::MACRO_ELAB_SIG_IMP {
    tryElaborate(right, table, context);
}

void Node::MACRO_TRANS_SIG_IMP {
    tryTranslate(right, stream, namespace_so_far);
}

void Phi::Node::tryElaborate(Phi::Node::Node* node, MACRO_ELAB_PARAMS)  {
    if (node) {
        try {
            node->elaborate(table, context);  
        } catch (const char* e) {
            context->addError(nullopt, e);
        }
    }
}
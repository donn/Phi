#include "Node.h"
using namespace Phi::Node;

void Node::MACRO_ELAB_SIG_IMP {
    tryElaborate(right, table, context);
}

void Node::MACRO_TRANS_SIG_IMP {
    tryTranslate(right, stream);
}
#include "Node.h"
using namespace Phi::Node;

void Node::MACRO_ELAB_SIG_IMP {
    tryElaborate(right, table, context);
}

void Node::translate(std::ofstream* stream) {
    tryTranslate(right, stream);
}
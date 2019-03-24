#if YYDEBUG
#include "Node.h"
using namespace Phi::Node;

#include <typeinfo>
#include <cxxabi.h>

std::string Node::debugLabel() {
    int status;
    return abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
}

std::string TopLevelNamespace::debugLabel() {
    return Node::debugLabel() + "\\n" + identifier;
}

void Node::graphPrint(std::ostream* stream, int* node) {
    auto current = *node;
    *node = current + 1;
    
    *stream << current << " " << "[label=\"" << debugLabel() << "\"]" << ";" << std::endl;

    if (left) {
        left->graphPrint(stream, node);
    }
    if (right) {
        right->graphPrint(stream, node);
    }
}


void TopLevelNamespace::graphPrint(std::ostream* stream, int* node) {
    auto current = *node;
    *node = current + 1;

    *stream << current << " " << "[label=\"" << debugLabel() << "\\n" << identifier << "\"]" << ";" << std::endl;

    if (left) {
        left->graphPrint(stream, node);
    }
    if (right) {
        right->graphPrint(stream, node);
    }
}

#endif
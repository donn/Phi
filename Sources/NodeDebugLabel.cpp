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

std::string TopLevelDeclaration::debugLabel() {
    std::string typeString;
    switch (type) {
        case TopLevelDeclaration::Type::module:
            typeString = "Module";
        case TopLevelDeclaration::Type::interface:
            typeString = "Interface";
        default:
            typeString = "Unknown";
    }
    return Node::debugLabel() + "\\n" + typeString + "\\n" + identifier;
}

int Node::graphPrint(std::ostream* stream, int* node) {
    auto current = *node;
    *node = current + 1;
    
    *stream << current << " " << "[label=\"" << debugLabel() << "\"]" << ";" << std::endl;

    if (left) {
        auto nodeID = left->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    if (right) {
        auto nodeID = right->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }

    return current;
}

int TopLevelNamespace::graphPrint(std::ostream* stream, int* node) {
    auto current = Node::graphPrint(stream, node);
    if (contents) {
        auto nodeID = contents->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    return current;
}

int TopLevelDeclaration::graphPrint(std::ostream* stream, int* node) {
    auto current = Node::graphPrint(stream, node);
    if (contents) {
        auto nodeID = contents->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    return current;
}

#endif
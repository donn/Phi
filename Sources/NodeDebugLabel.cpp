#if YYDEBUG
#include "Node.h"
using namespace Phi::Node;

#include <typeinfo>
#include <cxxabi.h>

std::string Node::debugLabel() {
    int status;
    std::string string =  abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
    return string.substr(string.find_last_of(':') + 1);
}

std::string Port::debugLabel() {
    std::string polarityString;
    switch (polarity) {
        case Port::Polarity::input:
            polarityString = "Input";
            break;
        case Port::Polarity::output:
            polarityString = "Output";
            break;
        case Port::Polarity::output_reg:
            polarityString = "Output (Register in Verilog)";
            break;
        default:
            polarityString = "Unknown";
    }
    return Node::debugLabel() + "\\n" + polarityString + "\\n" + identifier;
    
}

std::string TopLevelNamespace::debugLabel() {
    return Node::debugLabel() + "\\n" + identifier;
}

std::string TopLevelDeclaration::debugLabel() {
    using TLD = TopLevelDeclaration;
    std::string typeString;
    switch (type) {
        case TLD::Type::module:
            typeString = "Module";
            break;
        case TLD::Type::interface:
            typeString = "Interface";
            break;
        default:
            typeString = "Unknown";
    }
    return Node::debugLabel() + "\\n" + typeString + "\\n" + identifier;
}

std::string VariableLengthDeclaration::debugLabel() {
    using VLD = VariableLengthDeclaration;
    std::string typeString;
    switch (type) {
        case VLD::Type::wire:
            typeString = "Wire";
            break;
        case VLD::Type::wire_reg:
            typeString = "Wire (Register in Verilog)";
            break;
        case VLD::Type::reg:
            typeString = "Register";
            break;
        default:
            typeString = "Unknown";
    }
    return Node::debugLabel() + "\\n" + typeString;
}

std::string DeclarationListItem::debugLabel() {
    using VLD = VariableLengthDeclaration;
    std::string typeString;
    switch (type) {
        case VLD::Type::wire:
            typeString = "Wire";
            break;
        case VLD::Type::wire_reg:
            typeString = "Wire (Register in Verilog)";
            break;
        case VLD::Type::reg:
            typeString = "Register";
            break;
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

int Port::graphPrint(std::ostream* stream, int* node) {
    auto current = Node::graphPrint(stream, node);
    if (bus) {
        auto nodeID = bus->graphPrint(stream, node);
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
    if (ports) {
        auto nodeID = ports->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    if (contents) {
        auto nodeID = contents->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    return current;
}

int VariableLengthDeclaration::graphPrint(std::ostream* stream, int* node) {
    auto current = Node::graphPrint(stream, node);
    if (bus) {
        auto nodeID = bus->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    auto nodeID = declarationList->graphPrint(stream, node);
    *stream << current << " -- " << nodeID << ";" << std::endl;
    return current;
}
#endif
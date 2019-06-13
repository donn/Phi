#if YYDEBUG
#include "Node.h"

#include <typeinfo>
#include <cxxabi.h>

using namespace Phi::Node;

std::string Node::debugLabel() {
    int status;
    std::string string =  abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
    return string.substr(string.find_last_of(':') + 1);
}

std::string Port::debugLabel() {
    std::string polarityString;
    switch (polarity) {
        case PortObject::Polarity::input:
            polarityString = "Input";
            break;
        case PortObject::Polarity::output:
            polarityString = "Output";
            break;
        case PortObject::Polarity::output_reg:
            polarityString = "Output (Register in Verilog)";
            break;
        default:
            polarityString = "Unknown";
    }
    return Node::debugLabel() + "\\n" + polarityString + "\\n" + identifier->idString;
    
}

std::string TopLevelNamespace::debugLabel() {
    return Node::debugLabel() + "\\n" + identifier->idString;
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
    return Node::debugLabel() + "\\n" + typeString + "\\n" + identifier->idString;
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
    return Node::debugLabel() + "\\n" + typeString + "\\n" + identifier->idString;
}

std::string Identifier::debugLabel() {
    return Node::debugLabel() + "\\n" + idString;
}

int Node::MACRO_GRAPHPRINT_SIG_IMP {
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

int Port::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Node::graphPrint(stream, node);
    if (bus) {
        auto nodeID = bus->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    return current;
}

int TopLevelNamespace::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Node::graphPrint(stream, node);
    if (contents) {
        auto nodeID = contents->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    return current;
}

int TopLevelDeclaration::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Node::graphPrint(stream, node);
    if (ports) {
        auto nodeID = ports->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    if (preambles) {
        auto nodeID = preambles->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    if (contents) {
        auto nodeID = contents->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    if (addenda) {
        auto nodeID = addenda->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    return current;
}

int VariableLengthDeclaration::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Node::graphPrint(stream, node);
    if (bus) {
        auto nodeID = bus->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    auto nodeID = declarationList->graphPrint(stream, node);
    *stream << current << " -- " << nodeID << ";" << std::endl;
    return current;
}

int NondeclarativeAssignment::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Node::graphPrint(stream, node);
    auto lhsID = lhs->graphPrint(stream, node);
    *stream << current << " -- " << lhsID << ";" << std::endl;
    auto exprID = expression->graphPrint(stream, node);
    *stream << current << " -- " << exprID << ";" << std::endl;
    return current;
}

int IdentifierExpression::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Node::graphPrint(stream, node);
    auto nodeID = identifier->graphPrint(stream, node);
    *stream << current << " -- " << nodeID << ";" << std::endl;
    return current;
}

#endif
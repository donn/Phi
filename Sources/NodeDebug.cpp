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
        default:
            polarityString = "Unknown";
    }
    return Node::debugLabel() + "\\n" + polarityString + "\\n" + *identifier;
    
}

std::string TopLevelNamespace::debugLabel() {
    return Node::debugLabel() + "\\n" + *identifier;
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
    return Node::debugLabel() + "\\n" + typeString + "\\n" + *identifier;
}

std::string VariableLengthDeclaration::debugLabel() {
    using VLD = VariableLengthDeclaration;
    std::string typeString;
    switch (type) {
        case VLD::Type::wire:
            typeString = "Wire";
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
        case VLD::Type::reg:
            typeString = "Register";
            break;
        default:
            typeString = "Unknown";
    }
    return Node::debugLabel() + "\\n" + typeString + "\\n" + *identifier;
}

std::string Identifier::debugLabel() {
    return Node::debugLabel() + "\\n" + *this;
}

int Node::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = *node;
    *node = current + 1;
    
    *stream << current << " " << "[label=\"" << debugLabel() << "\"]" << ";" << std::endl;
    return current;
}

int Expression::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Node::graphPrint(stream, node);
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

int Statement::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Node::graphPrint(stream, node);
    if (next) {
        auto nodeID = next->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    return current;
}

int Port::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Statement::graphPrint(stream, node);
    if (bus) {
        auto nodeID = bus->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    return current;
}

int TopLevelNamespace::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Statement::graphPrint(stream, node);
    if (contents) {
        auto nodeID = contents->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    return current;
}

int TopLevelDeclaration::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Statement::graphPrint(stream, node);
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
    auto current = Statement::graphPrint(stream, node);
    if (bus) {
        auto nodeID = bus->graphPrint(stream, node);
        *stream << current << " -- " << nodeID << ";" << std::endl;
    }
    auto nodeID = declarationList->graphPrint(stream, node);
    *stream << current << " -- " << nodeID << ";" << std::endl;
    return current;
}

int NondeclarativeAssignment::MACRO_GRAPHPRINT_SIG_IMP {
    auto current = Statement::graphPrint(stream, node);
    auto lhsID = lhxe->lhx->graphPrint(stream, node);
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
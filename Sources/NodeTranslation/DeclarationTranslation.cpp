#include "Node.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace Phi::Node;

static std::string adjustNamespace(std::string soFar, std::string adding) {
    return soFar == "" ? adding : soFar + "." + adding;
}

// NOTE: This translation does NOT GO RIGHT! EVER!!
void Port::MACRO_TRANS_SIG_IMP {
    switch (polarity) {
        case PortObject::Polarity::input:
            *stream << "input";
            break;
        case PortObject::Polarity::output:
            *stream << "output";
            if (assignedInComb) {
                *stream << " reg";
            }
            break;
    }
    
    // bus --> bus points to null ? no range : range [from:to] ;
    tryTranslate(bus, stream, namespaceSoFar, indent);

    // identifier 
    *stream << " ";
    tryTranslate(identifier, stream, namespaceSoFar, indent);

    *stream << ";";
    *stream << MACRO_EOL;
}

void TopLevelNamespace::MACRO_TRANS_SIG_IMP {
    //adjust namespaceSoFar
    namespaceSoFar = adjustNamespace(namespaceSoFar, identifier->idString);
    tryTranslate(contents, stream, namespaceSoFar, indent);
    tryTranslate(right, stream, namespaceSoFar, indent);
}

void TopLevelDeclaration::MACRO_TRANS_SIG_IMP {
    
    if (type == TopLevelDeclaration::Type::module) {
        
        *stream << "module ";
        tryTranslate(identifier, stream, namespaceSoFar, indent);
        //adjust namespaceSoFar after entering the module to be nothing 
        namespaceSoFar = "";
        *stream << " (";

        MACRO_INDENT;
            // PII
            for (auto& abstractPort: space.lock()->ports) {
                auto actualPort = std::static_pointer_cast<Port>(abstractPort);
                tryTranslate(actualPort->identifier, stream, namespaceSoFar, indent);
                *stream << ", " << MACRO_EOL;
            }
        MACRO_DEDENT;

        *stream << ");" << MACRO_EOL;

        MACRO_INDENT;
            *stream << MACRO_EOL;

            // Get ready for ports
            for (auto& abstractPort: space.lock()->ports) {
                auto actualPort = std::static_pointer_cast<Port>(abstractPort);;
                tryTranslate(actualPort, stream, namespaceSoFar, indent);
            }
            *stream << MACRO_EOL;

            // Contents
            tryTranslate(preambles, stream, namespaceSoFar, indent);
            *stream << MACRO_EOL;

            // Contents
            tryTranslate(contents, stream, namespaceSoFar, indent);
            *stream << MACRO_EOL;

            // Addenda
            tryTranslate(addenda, stream, namespaceSoFar, indent);

        MACRO_DEDENT;
        *stream << "endmodule" << MACRO_EOL;
        *stream << MACRO_EOL << MACRO_EOL;

    } else if (type == TopLevelDeclaration::Type::interface){
        // Interfaces are elaboration-only
    }


    tryTranslate(right, stream, namespaceSoFar, indent);
}

void VariableLengthDeclaration::MACRO_TRANS_SIG_IMP {

    tryTranslate(declarationList, stream, namespaceSoFar, indent);
    
    tryTranslate(right, stream, namespaceSoFar, indent);
}

void DeclarationListItem::MACRO_TRANS_SIG_IMP {
    using VLD = VariableLengthDeclaration;

    if (type == VLD::Type::wire && assignedInComb) {
        *stream << "reg ";
    } else if (type != VLD::Type::var) {
        *stream << "wire ";
    } else {
        // Var is immutable and is handled at compile time.
        tryTranslate(right, stream, namespaceSoFar, indent);
        return;
    }

    if (bus.has_value()) {
        tryTranslate(bus.value().lock(), stream, namespaceSoFar, indent);
    }

    if (trueIdentifier) {
        tryTranslate(trueIdentifier, stream, namespaceSoFar, indent);
    } else {
        tryTranslate(identifier, stream, namespaceSoFar, indent);
    }

    //add wires,regs, always @ block, inside always @ block
    auto nsfLocal = adjustNamespace(namespaceSoFar, identifier->idString);
    if (type == VLD::Type::reg || type == VLD::Type::latch) {
        *stream << ";" << MACRO_EOL;
        *stream << MACRO_EOL;

        auto verilogPrimitive = type == VLD::Type::reg ? "\\Phi.Common.Register " : "\\Phi.Common.Latch";

        *stream << verilogPrimitive << " ";
        if (bus.has_value()) {
            auto busStrong = bus.value().lock();
            if (busStrong) {
                *stream << "#(.from(";
                tryTranslate(busStrong->from, stream, namespaceSoFar, indent);
                *stream << "), .to(";
                tryTranslate(busStrong->to, stream, namespaceSoFar, indent);
                *stream << ")) ";
            }
        }
        *stream <<  ("\\" + nsfLocal + ".module ");
        
        *stream << "(";
        MACRO_INDENT;
            *stream << ".data(" << ("\\" + nsfLocal + ".data ") << ")," << MACRO_EOL;
            *stream << ".out("; tryTranslate(identifier, stream, namespaceSoFar, indent); *stream << ")," << MACRO_EOL;
            if (type == VLD::Type::reg) {
                *stream << ".resetValue("; tryTranslate(optionalAssignment, stream, nsfLocal, indent); *stream << "), " << MACRO_EOL;
                *stream << ".reset(" << ("\\" + nsfLocal + ".reset ") << ")," << MACRO_EOL;
                *stream << ".clock(" << ("\\" + nsfLocal + ".clock ") << ")," << MACRO_EOL;
            } else {
                *stream << ".condition(" << ("\\" + nsfLocal + ".condition ") << ")," << MACRO_EOL; 
            }
            *stream << ".enable(" << (hasEnable ? "\\" + nsfLocal + ".enable " : "1'b1") << ")";
        MACRO_DEDENT;
        *stream << "); " << MACRO_EOL << MACRO_EOL;
    } else {
        if (optionalAssignment) {
            *stream << " ";
            *stream << "=";
            *stream << " ";
            *stream << "(";
            tryTranslate(optionalAssignment, stream, namespaceSoFar, indent);
            *stream << ")";
        }

        *stream << ";" << MACRO_EOL;
    }

    tryTranslate(right, stream, namespaceSoFar, indent);
}

void InstanceDeclaration::MACRO_TRANS_SIG_IMP {

    //example
    // flipflop #(.max(4000), .width(18)) id(.b(b), ...);
    // flipflop:module, .max(4000), .width(18), ..:parameters, 
    // id:identifier, .b(b), ...:ports

    // struct InstanceDeclaration: public Declaration {
    //         Expression* module;
    //         ExpressionIDPair* parameters;

    //         Expression* array; 
    //         ExpressionIDPair* ports;
    // }

    *stream << MACRO_EOL;

    tryTranslate(module, stream, namespaceSoFar, indent);
    if (parameters) {
        *stream << " #(";
        tryTranslate(parameters, stream, namespaceSoFar, indent);
        *stream << ")";
    }

    *stream << " ";
    tryTranslate(identifier, stream, namespaceSoFar, indent);
    *stream << "(";

    MACRO_INDENT;
        tryTranslate(ports, stream, namespaceSoFar, indent);
    MACRO_DEDENT;

    *stream << MACRO_EOL;
    *stream << "); " << MACRO_EOL;

    tryTranslate(right, stream, namespaceSoFar, indent);
}

void ExpressionIDPair::MACRO_TRANS_SIG_IMP {

    //example 
    //calling another module
    // A (.b(b_in), ... ) 
    // b: Declaration::identifier, b_in: expression

    // struct ExpressionIDPair: public Declaration {
    //         Expression* expression;
    // }
    
    *stream << ".";
    tryTranslate(identifier, stream, namespaceSoFar, indent);
    *stream << "(";
    tryTranslate(expression, stream, namespaceSoFar, indent);
    *stream << ")";

    if (right) {
        *stream << "," << MACRO_EOL;
        tryTranslate(right, stream, namespaceSoFar, indent);
    }
}

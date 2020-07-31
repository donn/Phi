#include "Node.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace Phi::Node;

// NOTE: This translation does NOT GO RIGHT! EVER!!
void Port::MACRO_TRANS_SIG_IMP {
    switch (polarity) {
        case PortObject::Polarity::input:
            *stream << "input";
            break;
        case PortObject::Polarity::output:
            *stream << "output";
            break;
        case PortObject::Polarity::output_reg:
            *stream << "output reg";
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
    namespaceSoFar = namespaceSoFar + "." + std::to_string((identifier->idString).length()) + identifier->idString;
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

    switch(type){
        case VLD::Type::wire_reg:
        case VLD::Type::reg:
        case VLD::Type::latch:
            *stream << "reg ";
            break;

        case VLD::Type::wire:
            //wire
            *stream << "wire ";
            break;
        default:
            // Var handled during elaboration
            tryTranslate(right, stream, namespaceSoFar, indent);
            return;
    };

    if (bus.has_value()) {
        tryTranslate(bus.value().lock(), stream, namespaceSoFar, indent);
    }

    if (trueIdentifier) {
        tryTranslate(trueIdentifier, stream, namespaceSoFar, indent);
    } else {
        tryTranslate(identifier, stream, namespaceSoFar, indent);
    }

    // Expression* array;
    if (array) {
        *stream << "[";
        tryTranslate(array, stream, namespaceSoFar, indent);
        *stream << "]";
    }

    if (optionalAssignment) {
        *stream << " ";
        *stream << "=";
        *stream << " ";
        *stream << "(";
        tryTranslate(optionalAssignment, stream, namespaceSoFar, indent);
        *stream << ")";
    }

    *stream << ";" << MACRO_EOL;

    //add wires,regs, always @ block, inside always @ block
    auto nsfLocal = namespaceSoFar.length() == 0 ? identifier->idString : namespaceSoFar + "." + identifier->idString;
    if (type==VLD::Type::reg) {
        *stream << MACRO_EOL;
        *stream << MACRO_EOL;

        *stream << "// Declaration stub for " << nsfLocal << MACRO_EOL;
        *stream << "always @ (posedge \\" + nsfLocal + ".clock  or posedge \\" + nsfLocal + ".reset ) begin " << MACRO_EOL;
        MACRO_INDENT;
            *stream << "if (\\" + nsfLocal + ".reset ) begin" << MACRO_EOL;
            MACRO_INDENT;
                *stream << "\\" << identifier->idString + " <= ";
                tryTranslate(optionalAssignment, stream, nsfLocal, indent);
                *stream << "; " << MACRO_EOL;
            MACRO_DEDENT;
            *stream << "end " << MACRO_EOL;
            MACRO_INDENT;
                *stream << "else begin" << MACRO_EOL;
                if (hasEnable) {
                    MACRO_INDENT;
                    *stream << "if (\\" << nsfLocal + ".enable " << ") begin" << MACRO_EOL;
                }
                *stream << "\\" << identifier->idString + " <= \\" + nsfLocal + ".data ; " << MACRO_EOL;
                if (hasEnable) {
                    MACRO_DEDENT;
                    *stream << "end" << MACRO_EOL;
                }
            MACRO_DEDENT;
            *stream << "end " << MACRO_EOL;
        MACRO_DEDENT;
        *stream << "end ";

        *stream << MACRO_EOL;
        *stream << MACRO_EOL;
    } else if (type==VLD::Type::latch) {
        *stream << MACRO_EOL;
        *stream << MACRO_EOL;

        *stream << "// Declaration stub for " << nsfLocal << MACRO_EOL;
        *stream << "always @ (\\" + nsfLocal +".condition ) begin " << MACRO_EOL;
        MACRO_INDENT;
            *stream << "if (\\" + nsfLocal + ".condition ) begin" << MACRO_EOL;
            MACRO_INDENT;  
                if (hasEnable) {
                    MACRO_INDENT;
                    *stream << "if (\\" << nsfLocal + ".enable" << ") begin" << MACRO_EOL;
                }
                *stream << "\\" << identifier->idString + " <= \\" + nsfLocal+ ".data ; " << MACRO_EOL;
                if (hasEnable) {
                    MACRO_DEDENT;
                    *stream << "end" << MACRO_EOL;
                }
            MACRO_DEDENT;
            *stream << "end " << MACRO_EOL;
        MACRO_DEDENT;
        *stream << "end ";

        *stream << MACRO_EOL;
        *stream << MACRO_EOL;
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

#include "Node.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace Phi::Node;

void Port::MACRO_TRANS_SIG_IMP {
    // example
    // a: Input [width-1 .. 0]; 
    // input [width-1 ..0] a;

    switch (polarity) {
        case Port::Polarity::input:
            *stream << "input";
            break;
        case Port::Polarity::output:
            *stream << "output";
            break;
        case Port::Polarity::output_reg:
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

    tryTranslate(right, stream, namespaceSoFar, indent);

}

void TopLevelNamespace::MACRO_TRANS_SIG_IMP {
    //adjust namespaceSoFar
    namespaceSoFar = namespaceSoFar + "_" + std::to_string((identifier->idString).length()) + identifier->idString;
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
        *indent += 1;
        *stream << MACRO_EOL;
        auto pointer = ports;
        
        // PII
        while (pointer) {
            tryTranslate(pointer->identifier, stream, namespaceSoFar, indent);
            *stream << ", " << MACRO_EOL;
            pointer = (Port*)pointer->right;
        }
        *indent -= 1;
        *stream << ")";


        *stream << ";" << MACRO_EOL;

        *indent += 1;

        *stream << MACRO_EOL;

        // Get ready for ports
        tryTranslate(ports, stream, namespaceSoFar, indent);
        *stream << MACRO_EOL;

        // Contents
        tryTranslate(preambles, stream, namespaceSoFar, indent);
        *stream << MACRO_EOL;

        // Contents
        tryTranslate(contents, stream, namespaceSoFar, indent);
        *stream << MACRO_EOL;

        // Addenda
        tryTranslate(addenda, stream, namespaceSoFar, indent);

        *indent -= 1;
        *stream << MACRO_EOL;
        *stream << "endmodule" << MACRO_EOL;

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


    tryTranslate(bus, stream, namespaceSoFar, indent);

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
    if(type==VLD::Type::reg) {
        namespaceSoFar = namespaceSoFar +  "_" + std::to_string((identifier->idString).length()) + identifier->idString;

        *stream << MACRO_EOL;
        *stream << MACRO_EOL;

        *stream << " // Declaration stub for " << namespaceSoFar << MACRO_EOL;
        *indent += 1;
        *stream << "always @ (posedge " + namespaceSoFar +"_clock or posedge " + namespaceSoFar +"_reset) begin " << MACRO_EOL;
            *indent += 1;
            *stream << "if("+namespaceSoFar+"_reset) begin" << MACRO_EOL;
                *stream << MACRO_EOL;
                *stream << identifier->idString + "<= ";
                tryTranslate(optionalAssignment, stream, namespaceSoFar, indent);
                *stream << "; " << MACRO_EOL;
            *indent -= 1;
            *stream << "end " << MACRO_EOL;
            *indent += 1;
            *stream << "else begin" << MACRO_EOL;
                if (hasEnable) {
                    *indent += 1;
                    *stream << "if (" << namespaceSoFar+"_enable" << ") begin" << MACRO_EOL;
                }
                *stream << identifier->idString + "<=" + namespaceSoFar+ "_data; " << MACRO_EOL;
                if (hasEnable) {
                    *indent -= 1;
                    *stream << "end" << MACRO_EOL;
                }
            *indent -= 1;
            *stream << "end " << MACRO_EOL;
        *indent -= 1;
        *stream << "end ";

        *stream << MACRO_EOL;
        *stream << MACRO_EOL;

        tryTranslate(right, stream, namespaceSoFar, indent);
    }else if(type==VLD::Type::latch){
        namespaceSoFar = namespaceSoFar +  "_" + std::to_string((identifier->idString).length()) + identifier->idString;

        *stream << MACRO_EOL;
        *stream << MACRO_EOL;

        *stream << " // Declaration stub for " << namespaceSoFar << MACRO_EOL;
        *indent += 1;
        *stream << "always @ (" + namespaceSoFar +"_condition) begin " << MACRO_EOL;
        *indent += 1;
            *stream << "if("+namespaceSoFar+"_condition) begin" << MACRO_EOL;  
                if (hasEnable) {
                    *indent += 1;
                    *stream << "if (" << namespaceSoFar+"_enable" << ") begin" << MACRO_EOL;
                }
                *stream << identifier->idString + "<=" + namespaceSoFar+ "_data; " << MACRO_EOL;
                if (hasEnable) {
                    *indent -= 1;
                    *stream << "end" << MACRO_EOL;
                }
            *indent -= 1;
            *stream << "end " << MACRO_EOL;
        *indent -= 1;
        *stream << "end ";

        *stream << MACRO_EOL;
        *stream << MACRO_EOL;
        
        tryTranslate(right, stream, namespaceSoFar, indent);
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
    tryTranslate(ports, stream, namespaceSoFar, indent);
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

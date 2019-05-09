#include "Node.h"

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
    tryTranslate(bus, stream, namespaceSoFar);

    // identifier 
    *stream << " ";
    tryTranslate(identifier, stream, namespaceSoFar);

    *stream << ";";
    *stream << std::endl;

    tryTranslate(right, stream, namespaceSoFar);

}

void TopLevelNamespace::MACRO_TRANS_SIG_IMP {
    //adjust namespaceSoFar
    namespaceSoFar = namespaceSoFar + "_" + std::to_string((identifier->idString).length()) + identifier->idString;

    tryTranslate(contents, stream, namespaceSoFar);
    tryTranslate(identifier, stream, namespaceSoFar);
}

void TopLevelDeclaration::MACRO_TRANS_SIG_IMP {
    
    //adjust namespaceSoFar
    namespaceSoFar = namespaceSoFar +  "_" + std::to_string((identifier->idString).length()) + identifier->idString;
    
    if (type == TopLevelDeclaration::Type::module) {
        *stream << "module ";
        tryTranslate(identifier, stream, namespaceSoFar);
        *stream << " (" << std::endl;
        auto pointer = ports;
        
        // PII
        while (pointer) {
            tryTranslate(pointer->identifier, stream, namespaceSoFar);
            *stream << ", " << std::endl;
            pointer = (Port*)pointer->right;
        }
        *stream << ")";


        *stream << ";" << std::endl;
        // Get ready for ports
        tryTranslate(ports, stream, namespaceSoFar);
        *stream << std::endl;

        // Contents
        tryTranslate(contents, stream, namespaceSoFar);

        *stream <<std::endl;
        *stream << "endmodule" << std::endl;

    } else if (type == TopLevelDeclaration::Type::interface){
        // Interfaces are elaboration-only
    }
    
    tryTranslate(right, stream, namespaceSoFar);
}

void VariableLengthDeclaration::MACRO_TRANS_SIG_IMP {

    tryTranslate(declarationList, stream, namespaceSoFar);
    tryTranslate(right, stream, namespaceSoFar);
}

void DeclarationListItem::MACRO_TRANS_SIG_IMP {
    using VLD = VariableLengthDeclaration;

    //adjust namespaceSoFar
    if(type==VLD::Type::reg || type==VLD::Type::latch){
        namespaceSoFar = namespaceSoFar +  "_" + std::to_string((identifier->idString).length()) + identifier->idString;
    }else{
        //do nothing
    }

    switch(type){
        case VLD::Type::wire:
            //wire
            *stream << "wire";
            break;

        case VLD::Type::reg:
        case VLD::Type::wire_reg:
            //reg
            *stream << "reg";
            break;

        case VLD::Type::latch:
            //latch --> leave for final presentation ??
            break;

        default:
            // Var handled during elaboration
            tryTranslate(right, stream, namespaceSoFar);
            return;
    };

    //add wires,regs, always @ block, inside always @ block
    if(type==VLD::Type::reg){
        *stream << "wire "+ namespaceSoFar+"_clock; \n";
        *stream << "wire "+namespaceSoFar+"_reset; \n";
        
        *stream << "wire ";
        tryTranslate(bus, stream, namespaceSoFar);
        *stream << " ";
        *stream <<namespaceSoFar+"_data; \n";
        
        *stream << "reg ";
        tryTranslate(bus, stream, namespaceSoFar);
        *stream << " ";
        *stream << identifier->idString + "; \n";

        *stream << "always @ (posedge " + namespaceSoFar +"_clock or posedge " + namespaceSoFar +"_reset) begin";
        
        *stream << "if("+namespaceSoFar+"_reset) begin\n";
        *stream << identifier->idString + "<= 0; \n";
        *stream << "end \n";

        *stream << "else begin\n";
        *stream << identifier->idString + "<=" + namespaceSoFar+ "_data; \n";
        *stream << "end \n";

        *stream << "end \n";

        tryTranslate(right, stream, namespaceSoFar);
    }else if(type==VLD::Type::latch){
        *stream << "wire "+ namespaceSoFar+"_condition; \n";
        
        *stream << "wire ";
        tryTranslate(bus, stream, namespaceSoFar);
        *stream << " ";
        *stream <<namespaceSoFar+"_data; \n";

        *stream << "reg ";
        tryTranslate(bus, stream, namespaceSoFar);
        *stream << " ";
        *stream <<identifier->idString+ "; \n";

        *stream << "always @ (" + namespaceSoFar +"_condition) begin";
        
        *stream << "if("+namespaceSoFar+"_condition) begin\n";
        *stream << identifier->idString + "<=" + namespaceSoFar +"_data; \n";
        *stream << "end \n";
        
        *stream << "end \n";
        
        tryTranslate(right, stream, namespaceSoFar);
    }else{
        tryTranslate(bus, stream, namespaceSoFar);
        *stream << " ";
        tryTranslate(identifier, stream, namespaceSoFar);

        // Expression* array;
        if (array) {
            *stream << "[";
            tryTranslate(array, stream, namespaceSoFar);
            *stream << "]";
        }

        if (optionalAssignment) {
            *stream << " ";
            *stream << "=";
            *stream << " ";
            *stream << "(";
            tryTranslate(optionalAssignment, stream, namespaceSoFar);
            *stream << ")";
        }

        *stream << "; " << std::endl;

        tryTranslate(right, stream, namespaceSoFar);
    }
    

    
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

    tryTranslate(module, stream, namespaceSoFar);

    if (parameters) {
        *stream << " #(";
        tryTranslate(parameters, stream, namespaceSoFar);
        *stream << ")";
    }

    *stream << " ";
    tryTranslate(identifier, stream, namespaceSoFar);
    *stream << "(";
    tryTranslate(ports, stream, namespaceSoFar);
    *stream << ");";
    tryTranslate(right, stream, namespaceSoFar);
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
    tryTranslate(identifier, stream, namespaceSoFar);
    *stream << "(";
    tryTranslate(expression, stream, namespaceSoFar);
    *stream << ")";

    if (right) {
        *stream << "," << std::endl;
        tryTranslate(right, stream, namespaceSoFar);
    }
}

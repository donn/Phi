#include "Node.h"
#include <string>
#include <fstream>

using namespace Phi::Node;

void Port::translate(std::ofstream* stream) {
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
    tryTranslate(bus, stream);

    // identifier 
    *stream << " ";
    tryTranslate(identifier, stream);

    *stream << ";";
    *stream << std::endl;
    
    tryTranslate(right, stream);

}

void TopLevelNamespace::translate(std::ofstream* stream) {
    tryTranslate(contents, stream);
    tryTranslate(right, stream);
}

void TopLevelDeclaration::translate(std::ofstream* stream) {
    
    if (type == TopLevelDeclaration::Type::module) {
        *stream << "module ";
        tryTranslate(identifier, stream);
        *stream << " (" << std::endl;
        auto pointer = ports;
        // PII
        while (pointer) {
            tryTranslate(pointer->identifier, stream);
            *stream << ", " << std::endl;
            pointer = (Port*)pointer->right;
        }
        *stream << ")";


        *stream << ";" << std::endl;

        // Parameters
        // TODO

        // Get ready for ports
        tryTranslate(ports, stream);
        *stream << std::endl;

        // Contents
        tryTranslate(contents, stream);

        *stream <<std::endl;
        *stream << "endmodule" << std::endl;

    } else if (type == TopLevelDeclaration::Type::interface){
        // Interfaces are elaboration-only
        // *stream << "interface " << Declaration::identifier << ";" << std::endl;

        // // Parameters
        // // TODO

        // // Get ready for ports
        // *stream << "(";
        // tryTranslate(ports, stream);
        // *stream << ");";
        // *stream << std::endl;

        // // Contents
        // tryTranslate(contents, stream);

        // *stream << "endinterface" << std::endl;
    }
    
    tryTranslate(right, stream);
}

void VariableLengthDeclaration::translate(std::ofstream* stream){

    tryTranslate(declarationList, stream);
    tryTranslate(right, stream);
}

void DeclarationListItem::translate(std::ofstream* stream) {
    using VLD = VariableLengthDeclaration;

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
            *stream << "/* phi error */";
            break;
    };

    tryTranslate(bus, stream);

    *stream << " ";
    tryTranslate(identifier, stream);

    // Expression* array;
    if (array) {
        *stream << "[";
        tryTranslate(array, stream);
        *stream << "]";
    }

    if (optionalAssignment) {
        *stream << " ";
        *stream << "=";
        *stream << " ";
        *stream << "(";
        tryTranslate(optionalAssignment, stream);
        *stream << ")";
    }

    *stream << "; " << std::endl;

    tryTranslate(right, stream);
}

void InstanceDeclaration::translate(std::ofstream* stream){

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

    tryTranslate(module, stream);

    if (parameters) {
        *stream << " #(";
        tryTranslate(parameters, stream);
        *stream << ")";
    }

    *stream << " ";
    tryTranslate(identifier, stream);
    *stream << "(";
    tryTranslate(ports, stream);
    *stream << ");";
    tryTranslate(right, stream);
}

void ExpressionIDPair::translate(std::ofstream* stream){

    //example 
    //calling another module
    // A (.b(b_in), ... ) 
    // b: Declaration::identifier, b_in: expression

    // struct ExpressionIDPair: public Declaration {
    //         Expression* expression;
    // }

    *stream << ".";
    tryTranslate(identifier, stream);
    *stream << "(";
    tryTranslate(expression, stream);
    *stream << ")";

    if (right) {
        *stream << "," << std::endl;
        tryTranslate(right, stream);
    }
}
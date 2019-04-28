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
    tryTranslate(bus, stream, namespace_so_far);

    // identifier 
    *stream << " ";
    tryTranslate(identifier, stream, namespace_so_far);

    *stream << ";";
    *stream << std::endl;

    tryTranslate(right, stream, namespace_so_far);

}

void TopLevelNamespace::MACRO_TRANS_SIG_IMP {
    //adjust namespace_so_far
    namespace_so_far = namespace_so_far + "_" + std::to_string((identifier->idString).length()) + identifier->idString;

    tryTranslate(contents, stream, namespace_so_far);
    tryTranslate(identifier, stream, namespace_so_far);
}

void TopLevelDeclaration::MACRO_TRANS_SIG_IMP {
    
    //adjust namespace_so_far
    namespace_so_far = namespace_so_far +  "_" + std::to_string((identifier->idString).length()) + identifier->idString;
    
    if (type == TopLevelDeclaration::Type::module) {
        *stream << "module ";
        tryTranslate(identifier, stream, namespace_so_far);
        *stream << " (" << std::endl;
        auto pointer = ports;
        
        // PII
        while (pointer) {
            tryTranslate(pointer->identifier, stream, namespace_so_far);
            *stream << ", " << std::endl;
            pointer = (Port*)pointer->right;
        }
        *stream << ")";


        *stream << ";" << std::endl;

        // Parameters
        // TODO

        // Get ready for ports
        tryTranslate(ports, stream, namespace_so_far);
        *stream << std::endl;

        // Contents
        tryTranslate(contents, stream, namespace_so_far);

        *stream <<std::endl;
        *stream << "endmodule" << std::endl;

    } else if (type == TopLevelDeclaration::Type::interface){
        // Interfaces are elaboration-only
    }
    
    tryTranslate(right, stream, namespace_so_far);
}

void VariableLengthDeclaration::MACRO_TRANS_SIG_IMP {

    tryTranslate(declarationList, stream, namespace_so_far);
    tryTranslate(right, stream, namespace_so_far);
}

void DeclarationListItem::MACRO_TRANS_SIG_IMP {
    using VLD = VariableLengthDeclaration;

    //adjust namespace_so_far
    if(type==VLD::Type::reg || type==VLD::Type::latch){
        namespace_so_far = namespace_so_far +  "_" + std::to_string((identifier->idString).length()) + identifier->idString;
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
            *stream << "/* phi error */";
            break;
    };

    //add wires,regs, always @ block, inside always @ block
    if(type==VLD::Type::reg){
        *stream << "wire "+ namespace_so_far+"_clock; \n";
        *stream << "wire "+namespace_so_far+"_reset; \n";
        
        *stream << "wire ";
        tryTranslate(bus, stream, namespace_so_far);
        *stream << " ";
        *stream <<namespace_so_far+"_data; \n";
        
        *stream << "reg ";
        tryTranslate(bus, stream, namespace_so_far);
        *stream << " ";
        *stream << identifier->idString + "; \n";

        *stream << "always @ (posedge " + namespace_so_far +"_clock or posedge " + namespace_so_far +"_reset) begin";
        
        *stream << "if("+namespace_so_far+"_reset) begin\n";
        *stream << identifier->idString + "<= 0; \n";
        *stream << "end \n";

        *stream << "else begin\n";
        *stream << identifier->idString + "<=" + namespace_so_far+ "_data; \n";
        *stream << "end \n";

        *stream << "end \n";

        tryTranslate(right, stream, namespace_so_far);
    }else if(type==VLD::Type::latch){
        *stream << "wire "+ namespace_so_far+"_condition; \n";
        
        *stream << "wire ";
        tryTranslate(bus, stream, namespace_so_far);
        *stream << " ";
        *stream <<namespace_so_far+"_data; \n";

        *stream << "reg ";
        tryTranslate(bus, stream, namespace_so_far);
        *stream << " ";
        *stream <<identifier->idString+ "; \n";

        *stream << "always @ (" + namespace_so_far +"_condition) begin";
        
        *stream << "if("+namespace_so_far+"_condition) begin\n";
        *stream << identifier->idString + "<=" + namespace_so_far +"_data; \n";
        *stream << "end \n";
        
        *stream << "end \n";
        
        tryTranslate(right, stream, namespace_so_far);
    }else{
        tryTranslate(bus, stream, namespace_so_far);
        *stream << " ";
        tryTranslate(identifier, stream, namespace_so_far);

        // Expression* array;
        if (array) {
            *stream << "[";
            tryTranslate(array, stream, namespace_so_far);
            *stream << "]";
        }

        if (optionalAssignment) {
            *stream << " ";
            *stream << "=";
            *stream << " ";
            *stream << "(";
            tryTranslate(optionalAssignment, stream, namespace_so_far);
            *stream << ")";
        }

        *stream << "; " << std::endl;

        tryTranslate(right, stream, namespace_so_far);
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

    tryTranslate(module, stream, namespace_so_far);

    if (parameters) {
        *stream << " #(";
        tryTranslate(parameters, stream, namespace_so_far);
        *stream << ")";
    }

    *stream << " ";
    tryTranslate(identifier, stream, namespace_so_far);
    *stream << "(";
    tryTranslate(ports, stream, namespace_so_far);
    *stream << ");";
    tryTranslate(right, stream, namespace_so_far);
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
    tryTranslate(identifier, stream, namespace_so_far);
    *stream << "(";
    tryTranslate(expression, stream, namespace_so_far);
    *stream << ")";

    if (right) {
        *stream << "," << std::endl;
        tryTranslate(right, stream, namespace_so_far);
    }
}

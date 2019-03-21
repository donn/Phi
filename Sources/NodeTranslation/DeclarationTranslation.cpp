#include "Node.h"
#include <string>
#include <fstream>

using namespace Phi::Node;

void Port::translate(std::ofstream* stream) {
    // example
    // a: Input [width-1 .. 0]; 
    // input [width-1 ..0] a;

    // struct Port: public Declaration {
    //         bool polarity; // polarity ? Input: Output
    //         Range* bus;

    //         optional<std::string> annotation;
    // }

    // polarity --> polarity ? Input: Output
    if(polarity==true){
        *stream << "input ";
    } else {
        *stream << "output ";
    }

    // bus --> bus points to null ? no range : range [from:to] ;
    tryTranslate(bus, stream);

    // identifier 
    *stream << Declaration::identifier;
    
     tryTranslate(right, stream);

}

void TopLevelNamespace::translate(std::ofstream* stream) {

    //example of package in verilog 
    //  package my_pkg;
    //   typedef enum bit [1:0] { RED, YELLOW, GREEN, RSVD } e_signal;
    //   typedef struct { bit [3:0] signal_id;
    //                      bit       active;
    //                      bit [1:0] timeout; 
    //                    } e_sig_param;
    
    //   function common ();
    //       $display ("Called from somewhere");
    //      endfunction
    
    //     task run ( ... );
    //       ...
    //     endtask
    // endpackage

    *stream << "package " << Declaration::identifier << ";" << std::endl;
    tryTranslate(contents, stream);
    *stream  << "endpackage" << std::endl;

    tryTranslate(right, stream);
}

void TopLevelDeclaration::translate(std::ofstream* stream) {
    
    if (type == TopLevelDeclaration::Type::module) {
        *stream << "module " << Declaration::identifier << ";" << std::endl;

        // Parameters
        // TODO

        // Get ready for ports
        *stream << "(";
        tryTranslate(ports, stream);
        *stream << ");";
        *stream << std::endl;

        // Contents
        tryTranslate(contents, stream);

        *stream << "endmodule" << std::endl;

    } else if (type == TopLevelDeclaration::Type::interface){
        
        *stream << "interface " << Declaration::identifier << ";" << std::endl;

        // Parameters
        // TODO

        // Get ready for ports
        *stream << "(";
        tryTranslate(ports, stream);
        *stream << ");";
        *stream << std::endl;

        // Contents
        tryTranslate(contents, stream);

        *stream << "endinterface" << std::endl;
    }
    
    tryTranslate(right, stream);
}


void VariableLengthDeclaration::translate(std::ofstream* stream){

    // struct VariableLengthDeclaration: public Declaration {
    //         enum class Type {
    //             var = 0,
    //             wire, reg, latch
    //         };
    //         Type type;
    //         Range* bus;
    //         Expression* array;
    //         Expression* optionalAssignment;
    // }

    switch(type){
        case Type::var:    
            //var --> leave for final presentation ?? 
            *stream << "integer";
            break;
        case Type::wire:
            //wire
            *stream << "wire";
            break;
        case Type::reg:
            //reg
            *stream << "reg";
            break;
        case Type::latch:
            //latch --> leave for final presentation ??
            break;
    };
    *stream << Declaration::identifier;

    tryTranslate(bus, stream);

    // leave those two parts for the final pres. 
    // Expression* array;
    // Expression* optionalAssignment;

    tryTranslate(right, stream);

}
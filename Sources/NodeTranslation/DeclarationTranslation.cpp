#include "Node.h"
#include <string>
#include <fstream>

using namespace Phi::Node;

void TopLevelNamespace::translate(std::ofstream* stream) {
    *stream << "package " << identifier << ";" << std::endl;
    tryTranslate(contents, stream);
    *stream  << "endpackage" << std::endl;
    tryTranslate(right, stream);
}

void TopLevelDeclaration::translate(std::ofstream* stream) {
    if (type == TopLevelDeclaration::Type::module) {
        *stream << "module " << identifier;

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

    }
    tryTranslate(right, stream);
}


void Port::translate(std::ofstream* stream) {
    // example
    // a: Input [width-1 .. 0]; 
    // input [width-1 ..0] a;

    // polarity --> polarity ? Input: Output
    if(polarity==true){
        *stream << "input ";
    } else {
        *stream << "output ";
    }

    // bus --> bus points to nuclll ? no range : range [from:to] ;
    if(bus == nullptr){
        // do nothing
    } else {
        *stream << " [ ";
        //bus->left->translate(stream);
        *stream << " : ";
        //bus->right->translate(stream); 
        *stream << " ] ";
    }

    // identifier 
    *stream << identifier;
    *stream << " ; ";

}
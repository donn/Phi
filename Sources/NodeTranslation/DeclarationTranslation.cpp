#include "Node.h"

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
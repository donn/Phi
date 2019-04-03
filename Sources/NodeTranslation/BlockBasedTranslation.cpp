#include "Node.h"
#include <string>
#include <fstream>

using namespace Phi::Node;

void If::translate (std::ofstream* stream){
}

void ForLoop::translate (std::ofstream* stream){
}

void Namespace::translate (std::ofstream* stream){
}

void Switch::translate (std::ofstream* stream){
}

void Combinational::translate (std::ofstream* stream){
    // always_comb in systemverilog

    // struct BlockBased: public Statement {
    //         Statement* contents;
    // }

    *stream << "always @* \n";
    *stream << "begin \n";
    tryTranslate(contents, stream);
    *stream << "end \n";

    
    tryTranslate(right, stream);
}

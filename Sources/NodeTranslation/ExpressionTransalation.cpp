#include "Node.h"
#include <string>
#include <fstream>

using namespace Phi::Node;

void Literal::translate (std::ofstream* stream){
    // examples: 0, 21, 32b1010010101.., 32d3, ...

    //assume: all literals will be represented in hexadecimal
    *stream << numBits << "'h" <<  value.value().toString(16, false);
}


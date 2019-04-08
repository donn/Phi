#include "Node.h"
#include <string>
#include <fstream>

using namespace Phi::Node;

void Identifier::translate(std::ofstream* stream) {
    *stream << "\\" << idString << " ";
}
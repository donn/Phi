#include "Node.h"

#include <string>
#include <fstream>

using namespace Phi::Node;

void Identifier::MACRO_TRANS_SIG_IMP {
    *stream << "\\" << idString << " ";
}
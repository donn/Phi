#include "Node.h"

#include <regex>
#include <sstream>
#include <iostream>
using namespace Phi::Node;

Identifier::Identifier(const char* identifier) {
    idString = std::regex_replace(identifier, std::regex("_"), "__");
}
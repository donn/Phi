#include "Node.h"

#include <regex>
#include <sstream>

using namespace Phi::Node;

// Number
Expression::Expression(String interpretable) {
    left = NULL;
    right = NULL;
    expType = ExpType::RunTime;
    
    auto regex = std::regex("([0-9]+)([bodxh])([A-F0-9zx]+)");
    auto match = std::smatch();
    std::regex_match(interpretable, match, regex); // If it doesn't match, the regex here and in the .l file are mismatched.
    auto prospectiveWidth = std::stoi(match[1]);
    if (prospectiveWidth < 0 || prospectiveWidth > maxWidth) {
        throw "Width out of range.";
    }

    width = prospectiveWidth;
    String radix = match[2];

    switch(radix[0]) {
        case 'b':
            radix = 2;
            break;
        case 'o':
            radix = 8;
            break;
        case 'd':
            radix = 10;
            break;
        case 'x':
            radix = 16;
            break;
        case 'h':
            throw "Phi doesn't support 'h'. Use 'x'.";
            break;
        default:
            throw "Unknown radix.";
    }
    literal = match[3];
}

Node* Expression::traverse() {
    return nullptr;
}


#include "Node.h"

#include <regex>
#include <sstream>

using namespace Phi::Node;

// Number
Literal::Literal(const char* interpretablePtr, bool widthIncluded) {
    auto interpretable = std::string(interpretablePtr);
    if (!widthIncluded) {
        auto ref = llvm::StringRef(interpretable);
        integer = llvm::APInt(32, ref, false);
    } else {
        std::string interpretable = std::string(interpretablePtr);
        type = Type::CompileTime;
        
        auto regex = std::regex("([0-9]+)([bodxh])([A-F0-9]+)");
        auto match = std::smatch();
        std::regex_match(interpretable, match, regex); // If it doesn't match, the regex here and in the .l file are mismatched.
        auto prospectiveWidth = std::stoi(match[1]);
        if (prospectiveWidth < 0 || prospectiveWidth > maxWidth) {
            throw "Width out of range.";
        }

        std::string radixCharacter = match[2];
        uint8_t radix;

        switch(radixCharacter[0]) {
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
        auto ref = llvm::StringRef(match[3]);
        integer = llvm::APInt(prospectiveWidth, ref, radix);
    }
}
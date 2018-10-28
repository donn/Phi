// Project Headers
#include "Context.h"
#include "Utils.h"

// Flex/Bison
#include <phi.yy.hh>

// Libraries
#include <termcolor/termcolor.hpp>

// CPP STL
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

void Phi::Parser::error(Location const& loc, const std::string& string) {
    context->errorList.push_back({loc, string});
}

void Phi::Context::printErrors() {
    auto errorCount = errorList.size();
    if (errorCount > 0) {
        for (auto i = 0; i < errorCount; i += 1) {
            auto& error = errorList[i];
            auto& loc = error.loc;
            auto& message = error.message;

            std::cerr << termcolor::bold << *loc.begin.filename << ":" << loc.begin.line << ":" << loc.begin.column << ": " << termcolor::red << message << termcolor::reset << std::endl;
            std::cerr << currentFileLines[loc.begin.line - 1] << std::endl;
            std::cerr << termcolor::bold << termcolor::green <<
                std::setw(loc.begin.column + 1) << "^" <<
                std::string(loc.end.column - loc.begin.column - 2, '~') <<
                termcolor::reset << std::endl;
        }
        std::cerr << errorCount << ((errorCount == 1) ? " error" : " errors") << " generated." << std::endl;
    }
}

std::string Phi::Context::setFile(std::string currentFile)  {
    files.push_back(currentFile);

    auto file = std::ifstream(currentFile);
    if (file.fail()) {
        std::cerr << "Could not open file '" << currentFile << "'." << std::endl;
        throw "couldNotOpen";
    }
    
    std::stringstream stringstream;
    stringstream << file.rdbuf();
    file.close();

    auto dump = stringstream.str();
    currentFileLines = Phi::Utils::split(&dump, '\n');

    return dump;

}

bool Phi::Context::error() {
    return errorList.size() > 0;
}
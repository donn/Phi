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
#include <exception>

void Phi::Parser::error(Location const& loc, const std::string& string) {
    auto copy = string;
    if (copy == "syntax error") {
        copy = "parser.syntaxError";
    }
    
    context->errorList.push_back({loc, copy});
}

void Phi::Context::printErrors() {
    auto errorCount = errorList.size();
    if (errorCount > 0) {
        for (size_t i = 0; i < errorCount; i += 1) {
            auto error = errorList[i];
            auto loc = error.loc;
            auto message = error.message;

            std::string highlight = "";
            if (loc.end.column > loc.begin.column) {
                highlight = std::string(loc.end.column - loc.begin.column - 2, '~');
            }

            std::cerr << termcolor::bold << *loc.begin.filename;
            unless (loc.begin.line == 0) {
                std::cerr << ":" << loc.begin.line << ":" << loc.begin.column;
            }
            std::cerr << ": " << termcolor::red << message << termcolor::reset << std::endl;
            unless (loc.begin.line == 0) {
                std::cerr << currentFileLines[loc.begin.line - 1] << std::endl;
                std::cerr << termcolor::bold << termcolor::green <<
                    std::setw(loc.begin.column + 1) << "^" <<
                    highlight << termcolor::reset << std::endl;
            }
        }
        std::cerr << errorCount << ((errorCount == 1) ? " error" : " errors") << " generated." << std::endl;
    }
}

optional<std::string> Phi::Context::setFile(std::string currentFile)  {
    files.push_back(currentFile);

    auto file = std::ifstream(currentFile);
    if (file.fail()) {
        errorList.push_back({Location(&currentFile, 0, 0), "io.fileNotFound"});
        return nullopt;
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

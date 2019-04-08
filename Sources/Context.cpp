// Project Headers
#include "Context.h"
#include "Utils.h"

// Bison
#include <phi.yy.hh>

// Libraries
#include <termcolor/termcolor.hpp>

// CPP STL
#include <iomanip>
#include <sstream>
#include <fstream>
#include <exception>

using namespace Phi;

std::string Error::emptyLocationFileName = "/";
Location Error::emptyLocation = Location(&Error::emptyLocationFileName, 0, 0);

void Parser::error(Location const& loc, const std::string& string) {
    auto copy = string;
    if (copy == "syntax error") {
        copy = "parser.syntaxError";
    }

    auto copyloc = loc;
    
    context->errorList.push_back({copyloc, copy});
}

void Context::prettyPrintErrors(std::ostream* out) {
    auto errorCount = errorList.size();
    if (errorCount > 0) {
        for (size_t i = 0; i < errorCount; i += 1) {
            auto error = errorList[i];
            auto loc = error.loc;
            auto message = error.message;

            std::string highlight = "";
            if (loc.end.column > loc.begin.column + 1) {
                highlight = std::string(loc.end.column - loc.begin.column - 2, '~');
            }

            *out << termcolor::bold << *loc.begin.filename;
            unless (loc.begin.line == 0) {
                *out << ":" << loc.begin.line << ":" << loc.begin.column;
            }
            *out << ": " << termcolor::red << message << termcolor::reset << std::endl;
            unless (loc.begin.line == 0) {
                *out << currentFileLines[loc.begin.line - 1] << std::endl;
                *out << termcolor::bold << termcolor::green <<
                    std::setw(loc.begin.column + 1) << "^" <<
                    highlight << termcolor::reset << std::endl;
            }
        }
        *out << errorCount << ((errorCount == 1) ? " error" : " errors") << " generated." << std::endl;
    }
}

void Context::setFile(std::string currentFile)  {
    files.push_back(currentFile);

    auto file = std::ifstream(currentFile);
    if (file.fail()) {
        errorList.push_back({Location(&files.back(), 0, 0), "io.fileNotFound"});
        return;
    }

    std::stringstream stringstream;
    stringstream << file.rdbuf();
    file.close();

    auto dump = stringstream.str();
    currentFileLines = Utils::split(&dump, '\n');
}

bool Context::error() {
    return errorList.size() > 0;
}

void Context::elaborate(SymbolTable* table) {
    head->elaborate(table, this);
}

void Context::translate(std::ofstream* stream) {
    head->translate(stream);
}

#if YYDEBUG
void Context::graphPrint(std::ofstream* stream) {
    *stream << "strict graph tree {" << std::endl;
    int counter = 0;
    head->graphPrint(stream, &counter);
    *stream << "}" << std::endl;
}
#endif
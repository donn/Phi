#include "Context.h"
#include "Utils.h"

#include <phi.yy.hh>

#include <iomanip>
#include <sstream>
#include <fstream>
#include <exception>

using namespace Phi;

void Parser::error(Location const& location, const std::string& string) {
    auto copy = string;
    if (copy == "syntax error") {
        copy = "parser.syntaxError";
    }

    auto copyloc = location;
    
    context->addError(copyloc, copy);
}

void Context::addError(const optional<Location> location, const std::string message) {
    auto trueLocation = location.has_value() ? location.value() : Location(&files.back(), 0, 0);
    errorList.push_back({trueLocation, message});
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

            *out << *loc.begin.filename;
            if (loc.begin.line != 0) {
                *out << ":" << loc.begin.line << ":" << loc.begin.column;
            }
            *out << ": " << message << std::endl;
            if (loc.begin.line != 0) {
                *out << currentFileLines[loc.begin.line - 1] << std::endl;
                *out <<
                    std::setw(loc.begin.column + 1) << "^" <<
                    highlight << std::endl;
            }
        }
        *out << errorCount << ((errorCount == 1) ? " error" : " errors") << " generated." << std::endl;
    }
}

void Context::setFile(std::string currentFile)  {
    files.push_back(currentFile);

    auto file = std::ifstream(currentFile);
    if (file.fail()) {
        addError(nullopt, "io.fileNotFound");
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

void Context::elaborate(SymbolTable* tablePtr) {
    table = tablePtr;
    head->elaborate(this);
    table = nullptr;
}

void Context::driveChecks() {
    for (auto& check: checks) {
        auto from = check.from.has_value() ? check.from.value() : check.target->from;
        auto to = check.to.has_value() ? check.to.value() : check.target->to;

        auto coverage = check.target->checkRangeCoverage(from, to);

        if (coverage.size() == 0) {
            check.effect();
        }
    }
}

void Context::translate(std::ostream* stream) {
    int indent = 0;
    std::string namespaceSoFar = "";
    head->translate(stream, namespaceSoFar, &indent);
}

#if YYDEBUG
void Context::graphPrint(std::ofstream* stream) {
    *stream << "strict graph tree {" << std::endl;
    int counter = 0;
    head->graphPrint(stream, &counter);
    *stream << "}" << std::endl;
}
#endif

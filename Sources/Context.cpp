// Project Headers
#include "Context.h"

// Flex/Bison
#include <phi.yy.hh>

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
            auto& content = "IMPLEMENT_TOKEN";

            std::cerr << *loc.begin.filename << ":" << loc.begin.line << ":" << loc.begin.column << ": " << message << ": unexpected " << content<< std::endl;
        }
        std::cerr << errorCount << ((errorCount == 1) ? " error" : " errors") << " generated." << std::endl;
    }
}
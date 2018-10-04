// CPP STL
#include <iostream>
#include <fstream>
#include <sstream>

// Project headers
#include "sysexits.h"
#include "Node.h"
#include "Errors.h"

extern "C" {
    #include <phi.l.h>
    #include <phi.y.h>
    
    extern int yynerrs;
    extern int yyparse();
}

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cout << "Invocation: ./phi <input.phi>" << std::endl;
        return EX_USAGE;
    }

    // Read input file
    std::stringstream stringstream;
    auto file = std::ifstream(argv[1]);
    if (file.fail()) {
        std::cerr << "Could not open file '" << argv[1] << "'." << std::endl;
        return EX_NOINPUT;
    }

    stringstream << file.rdbuf();

    auto input = stringstream.str();
    yy_scan_string(input.c_str());

    // Parse
    auto parsingResult = yyparse();
    auto errorCount = Error::list.size();
    if (errorCount > 0) {
        for (auto i = 0; i < errorCount; i += 1) {
            auto& error = Error::list[i];

            std::cout << argv[1] << ":" << error.line << ": " << error.message << ": unexpected " << error.content << std::endl;
        }
        std::cerr << errorCount << ((errorCount == 1) ? " error" : " errors") << " generated." << std::endl;
        return EX_DATAERR;
    }
    
    // Construct AST
    traverse(head);


    return EX_OK;
}

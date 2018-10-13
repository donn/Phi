// CPP STL
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

// Project headers
#include "sysexits.h"
#include "Node.h"
#include "Errors.h"

#include <git_version.h>

// Libraries
#include <StupidSimpleCPPOpts.h>

extern "C" {
    #include <phi.l.h>
    #include <phi.y.h>
    
    extern int yynerrs;
    extern int yyparse();
}

void printVersion() {
    std::string dev = "";
    if (std::string(GIT_TAG) != std::string(GIT_VER_STRING)) {
        dev = "-dev";
    }
    std::cout << "Phi Compiler version " << GIT_TAG << dev << " (" << GIT_VER_STRING << ")" << std::endl;
    std::cout << "All rights reserved. Licensed under the Apache License 2.0." << std::endl;
}

int main(int argc, char* argv[]) {
    // CLI Option parsing
    SSCO::Options options({
        {"help", 'h', "Show this message and exit.", false, [&](){ options.printHelp(); exit(0); }},
        {"version", 'V', "Show the current version of Phi.", false, [&](){ printVersion(); exit(0); }},
    });

    auto opts = options.process(argc, argv);

    if (!opts.has_value()) {
        options.printHelp();
        return EX_USAGE;
    }

    auto& arguments = opts.value().arguments;

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

            std::cerr << argv[1] << ":" << error.line << ": " << error.message << ": unexpected " << error.content << std::endl;
        }
        std::cerr << errorCount << ((errorCount == 1) ? " error" : " errors") << " generated." << std::endl;
        return EX_DATAERR;
    }
    
    // Construct AST
    traverse(head);


    return EX_OK;
}

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
#include <Flags.hh>
#include <getopt.h>

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
    if (argc != 2) {
        std::cout << "Invocation: ./phi <input.phi>" << std::endl;
        return EX_USAGE;
    }

    // CLI Option parsing
    Flags flags;

    bool help;
    flags.Bool(help, 'h', "help", "Show this message and exit.", "Operation Modes");

    bool version;
    flags.Bool(version, 'v', "version", "Show the version.", "Operation Modes");

    if (!flags.Parse(argc, argv)) {
        flags.PrintHelp(argv[0], std::cerr);
        return EX_USAGE;
    } else if (help) {
        flags.PrintHelp(argv[0]);
        return EX_OK;
    } else if (version) {
        printVersion();
        return EX_OK;
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

            std::cerr << argv[1] << ":" << error.line << ": " << error.message << ": unexpected " << error.content << std::endl;
        }
        std::cerr << errorCount << ((errorCount == 1) ? " error" : " errors") << " generated." << std::endl;
        return EX_DATAERR;
    }
    
    // Construct AST
    traverse(head);


    return EX_OK;
}

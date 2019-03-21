// CPP STL
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>

// Project headers
#include "git_version.h"

#include "Context.h"
#include "BSDExits.h"
#include "SymbolTable.h"

// Flex/Bison Headers
#include <phi.l.hh>
#include <phi.yy.hh>

// Libraries
#include <StupidSimpleCPPOpts.h>
#include <termcolor/termcolor.hpp>

std::string versionString() {
    std::string dev = "";
    if (std::string(Phi::GIT_TAG) != std::string(Phi::GIT_VER_STRING)) {
        dev = "-dev";
    }
    return std::string(Phi::GIT_TAG) + dev + " (" + Phi::GIT_VER_STRING + ")";
}

void printVersion() {
    std::cout << "Phi Compiler version " << versionString() << std::endl;
    std::cout << "All rights reserved. Licensed under the Apache License 2.0." << std::endl;
}

int main(int argc, char* argv[]) {
    // CLI Option parsing
    SSCO::Options getOpt({
        {"help", 'h', "Show this message and exit.", false, [&](){ getOpt.printHelp(); exit(0); }},
        {"version", 'V', "Show the current version of Phi.", false, [&](){ printVersion(); exit(0); }},
#if YYDEBUG
        {"trace", 'T', "Trace GNU Bison/Phi semantic analysis operation. (Debug builds only.)", false, nullopt},
        {"astGraph", std::nullopt, "Filename to output graphviz of syntax tree. (Debug builds only.)", true, nullopt},
        {"elabGraph", std::nullopt, "Filename to output graphviz of post-elaboration tree. (Debug builds only.)", true, nullopt},
        {"symGraph", std::nullopt, "Filename to output graphviz of symbol table. (Debug builds only.)", true, nullopt}
#endif
    });
    auto opts = getOpt.process(argc, argv);

    if (!opts.has_value()) {
        getOpt.printHelp();
        return EX_USAGE;
    }

    auto& options = opts.value().options;
    auto& arguments = opts.value().arguments;

    // Read input file
    if (arguments.size() < 1) {
        std::cout << "No files provided." << std::endl;
        getOpt.printHelp();
        return EX_USAGE;
    }

    auto filename = arguments[0];
    auto extensionPosition = filename.rfind(".phi");
    if (extensionPosition == std::string::npos) {
        std::cerr << "File must end with .phi." << std::endl;
        return EX_DATAERR;
    }

    auto outputFilename = filename;
    outputFilename.replace(extensionPosition, 4, ".v");

    auto context = Phi::Context(argv[0]);
    
#if YYDEBUG
    if (options.find("trace") != options.end()) {
        context.trace = 1;
    }
#endif

    optional<std::string> input = context.setFile(filename);
    unless (input.has_value()) {
        context.printErrors();
        return EX_NOINPUT;
    }

    yy_scan_string(input.value().c_str());

    // Parse
    auto parser = Phi::Parser(&context);
    parser.parse();

    if (context.error()) {
        context.printErrors();
        return EX_DATAERR;
    }

 #if YYDEBUG
    auto astGraphFile = options.find("astGraph");
    if (astGraphFile != options.end()) {
        std::ofstream output(astGraphFile->second);
        if (output.fail()) {
            return EX_CANTCREAT;
        }
        context.graphPrint(&output);
    }
#endif   

    Phi::SymbolTable table;
    context.elaborate(&table);

    std::ofstream output;
    output.open(arguments[0] + ".sv");
    
    //context.translate(&output);

#if YYDEBUG
    auto symTableFile = options.find("symGraph");
    if (symTableFile != options.end()) {
        std::ofstream output(symTableFile->second);
        if (output.fail()) {
            return EX_CANTCREAT;
        }
        table.represent(&output);
    }
#endif
    return EX_OK;
}
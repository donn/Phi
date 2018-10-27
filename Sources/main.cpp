// CPP STL
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

// Project headers
#include "git_version.h"

#include "Context.h"
#include "BSDExits.h"

// Flex/Bison Headers
#include <phi.l.hh>
#include <phi.yy.hh>

// Libraries
#include <StupidSimpleCPPOpts.h>

void printVersion() {
    std::string dev = "";
    if (std::string(Phi::GIT_TAG) != std::string(Phi::GIT_VER_STRING)) {
        dev = "-dev";
    }
    std::cout << "Phi Compiler version " << Phi::GIT_TAG << dev << " (" << Phi::GIT_VER_STRING << ")" << std::endl;
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
    if (arguments.size() < 1) {
        std::cout << "No files provided." << std::endl;
    }

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
    auto context = Phi::Context();
    auto parser = Phi::Parser(&context);
    
    context.setFile(argv[1]);
    auto parsingResult = parser.parse();

    if (context.error()) {
        context.printErrors();
        return EX_DATAERR;
    }

    // for (auto k: context.hell) {
    //     std::cout << k << std::endl;
    // }

    return EX_OK;
}

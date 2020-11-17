#include "Context.h"
#include "BSDExits.h"
#include "SymbolTable.h"

#include "git_version.h"

#include <phi.yy.hh>
#include <phi.l.hh>

#include <SSCO/StupidSimpleCPPOpts.h>

#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>


#define VERSION_STRING(name, a, b, c) (std::string(name) + " " + std::to_string(a) + "." + std::to_string(b) + "." + std::to_string(c))

std::string compilerVersionString() {
    #ifdef __clang__
    return VERSION_STRING("clang", __clang_major__, __clang_minor__, __clang_patchlevel__);
    #elif __GNUC__
    return VERSION_STRING("GCC", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    #elif _MSC_VER
    return std::string("Microsoft Visual C++ ") + _MSC_VER;
    #else
    return "unknown compiler"
    #endif
}


std::string versionString() {
    std::string dev = "";
    if (std::string(BuildInfo::GIT_TAG) != std::string(BuildInfo::GIT_VER_STRING)) {
        dev = "-dev";
    }
    return std::string(BuildInfo::GIT_TAG) + dev + " (" + BuildInfo::GIT_VER_STRING + " " + compilerVersionString() + ")";
}

#ifdef _WIN32
    #include <iostream>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    auto& stdoutStream = std::cout;
    auto& stderrStream = std::cerr;
#else
    auto stdoutStream = std::ofstream("/dev/stdout");
    auto stderrStream = std::ofstream("/dev/stderr");
#endif

    // CLI Option parsing
    SSCO::Options getOpt({
        {"help", 'h', "Show this message and exit.", false, [&](){ getOpt.printHelp(stdoutStream); exit(0); }},
        {"version", 'V', "Show the current version of Phi.", false, [&](){   
            stdoutStream << "Phi " << versionString() << std::endl;
            stdoutStream << "All rights reserved. Licensed under the Apache License 2.0." << std::endl;
            exit(0);
        }},
        {"metadataJSON", nullopt, "Data to output JSON information about processed modules.", true, nullopt},
        {"outFile", 'o', "Output file.", true, nullopt},
        {"ignoreErrors", nullopt, "Attempt best translation despite errors.", false, nullopt},
#if YYDEBUG
        {"trace", 'T', "Trace GNU Bison/Phi semantic analysis operation. (Debug builds only.)", false, nullopt},
        {"astGraph", nullopt, "Filename to output graphviz of syntax tree. (Debug builds only.)", true, nullopt},
        {"elabGraph", nullopt, "Filename to output graphviz of post-elaboration tree. (Debug builds only.)", true, nullopt},
        {"symGraph", nullopt, "Filename to output graphviz of symbol table. (Debug builds only.)", true, nullopt}
#endif
    });
    auto opts = getOpt.process(argc, argv);

    if (!opts.has_value()) {
        getOpt.printHelp(stdoutStream);
        return EX_USAGE;
    }

    auto& options = opts.value().options;
    auto& arguments = opts.value().arguments;

    // Read input file
    if (arguments.size() < 1) {
        stdoutStream << "No files provided." << std::endl;
        getOpt.printHelp(stdoutStream);
        return EX_USAGE;
    }

    auto filename = arguments[0];
    auto extensionPosition = filename.rfind(".phi");
    if (extensionPosition == std::string::npos) {
        std::cerr << "File must end with .phi." << std::endl;
        return EX_DATAERR;
    }
    
    auto context = Phi::Context(argv[0]);
    
#if YYDEBUG
    if (options.find("trace") != options.end()) {
        context.trace = 1;
    }
#endif

    context.setFile(filename);
    if (context.error()) {
        context.prettyPrintErrors(&stderrStream);
        return EX_NOINPUT;
    }

    // Lex
    auto file = std::ifstream(filename);
    auto lexer = Phi::Lexer(file);

    // Parse
    auto parser = Phi::Parser(&lexer, &context);
    try {
        parser.parse();
    } catch (const char* error) {
        stderrStream << "[CRITICAL] Unhandled parse issue: " << error << std::endl;
        stderrStream << "Please report this issue." << std::endl;
        exit(EX_SOFTWARE);
    }

    if (context.error()) {
        context.prettyPrintErrors(&stderrStream);
        if (options.find("ignoreErrors") == options.end()) {
            return EX_DATAERR;
        }
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
    context.driveChecks();
    if (context.error()) {
        context.prettyPrintErrors(&stderrStream);
        if (options.find("ignoreErrors") == options.end()) {
            return EX_DATAERR;
        }
    }

#if YYDEBUG
    auto elabGraphFile = options.find("elabGraph");
    if (elabGraphFile != options.end()) {
        std::ofstream output(elabGraphFile->second);
        if (output.fail()) {
            return EX_CANTCREAT;
        }
        context.graphPrint(&output);
    }
#endif   

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

    auto metadataJSONFile = options.find("metadataJSON");
    if (metadataJSONFile != options.end()) {
        std::ofstream output(metadataJSONFile->second);
        if (output.fail()) {
            return EX_CANTCREAT;
        }
        output << table.moduleMetadata();
    }

    std::string outputFilename = arguments[0] + ".sv";
    if (options.find("outFile") != options.end()) {
        outputFilename = options["outFile"];
    }

    std::ofstream output;
    std::string namespaceSoFar = "";
    
    output.open(outputFilename);
    if (output.fail()) {
        std::cerr << "Could not open file '" << filename << "'." << std::endl;
        return EX_CANTCREAT;
    }

    output << "/*" << std::endl;
    output << "   Automatically generated by the Phi Compiler " << versionString() << std::endl;
    output << "   Do not modify." << std::endl;
    auto timeObject = std::time(nullptr);
    auto currentTime = *std::localtime(&timeObject);
    output << "   Generated on: " <<  std::put_time(&currentTime, "%Y-%m-%d %H:%M:%S") << std::endl;
    output << "*/" << std::endl << std::endl;
    context.translate(&output);
    output.close();

    return EX_OK;
}
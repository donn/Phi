#include "Context.h"
#include "BSDExits.h"
#include "SymbolTable.h"
#include "BuildInfo.h"

#include <phi.yy.hh>
#include <phi.l.hh>

#include <SSCO/StupidSimpleCPPOpts.h>

#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>

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
        {"help", 'h', "Show this message and exit.", false, [&](){ getOpt.printHelp(stdoutStream); exit(EX_OK); }},
        {"version", 'V', "Show the current version of Phi.", false, [&](){   
            stdoutStream << "Phi " << Phi::BuildInfo::versionString() << std::endl;
            stdoutStream << "©2019-2021 The Phi Authors. Licensed under the Apache License 2.0." << std::endl;
            exit(EX_OK);
        }},
        {"metadataJSON", nullopt, "Data to output JSON information about processed modules.", true, nullopt},
        {"outFile", 'o', "Output file.", true, nullopt},
#if YYDEBUG
        {"ignoreErrors", nullopt, "Attempt best translation despite errors. (Debug builds only.)", false, nullopt},
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
        return EX_SOFTWARE;
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
    context.driveChecks(&table);
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

    context.translate(&output);
    output.close();

    return EX_OK;
}
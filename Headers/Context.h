#ifndef _context_h
#define _context_h
#include "Types.h"
#include "SymbolTable.h"

#include <location.hh>

#include <vector>
#include <list>

namespace Phi {
    using Location = Phi::location;
    namespace Node {
        class Node;
    }

    class Context {
        struct Error {
            Location loc;
            std::string message;
        };

        std::string executableName;
        std::vector<Error> errorList;
    public:
        Context(const char* argv0): executableName(argv0) {
            files.push_back("\\");
        }

        std::list<std::string> files;
        std::vector<std::string> currentFileLines;
        Node::Node* head = nullptr;

        void setFile(std::string currentFile);

        void addError(const optional<Location> location, const std::string message);
        bool error();
        void prettyPrintErrors(std::ostream* out);

        void elaborate(SymbolTable* table);
        SymbolTable* table = nullptr;

        void translate(std::ostream* stream, std::string namespaceSoFar);

#if YYDEBUG
        int trace = 0;
        void graphPrint(std::ofstream* stream);
#endif
    };
}

#endif

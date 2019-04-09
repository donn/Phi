#ifndef _context_h
#define _context_h
// C++ STL
#include <vector>
#include <list>

// Flex/Bison
#include <location.hh>

// Project Headers
#include "Types.h"
#include "Node.h"

using Location = Phi::location;

namespace Phi {
    class SymbolTable;

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
        using Node = Phi::Node::Node;
        Node* head = nullptr;

        void setFile(std::string currentFile);

        void addError(const optional<Location> location, const std::string message);
        bool error();
        void prettyPrintErrors(std::ostream* out);

        void elaborate(SymbolTable* table);
        void translate(std::ofstream* stream);

#if YYDEBUG
        int trace = 0;
        void graphPrint(std::ofstream* stream);
#endif
    };
}

#endif

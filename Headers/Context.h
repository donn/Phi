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
        struct Node;
    }
    class Context {
        friend class Parser;
        struct Error {
            Location loc;
            std::string message;
        };

        std::string executableName;
        std::vector<Error> errorList;

        std::list<std::string> files;
        std::vector<std::string> currentFileLines;
        Node::Node* head = nullptr;
    public:
        struct DriveCheck {
            std::shared_ptr<Driven> target;
            optional<AccessWidth> from;
            optional<AccessWidth> to;
            std::function<void()> effect;

            DriveCheck(std::shared_ptr<Driven> target, optional<AccessWidth> from, optional<AccessWidth> to, std::function<void()> effect): target(target), from(from), to(to), effect(effect) {}
        };
        std::vector<DriveCheck> checks;
        SymbolTable* table = nullptr;
        
        Context(const char* argv0): executableName(argv0) {
            files.push_back("\\");
        }

        void setFile(std::string currentFile);

        void addError(const optional<Location> location, const std::string message);
        bool error();
        void prettyPrintErrors(std::ostream* out);

        void elaborate(SymbolTable* table);
        void driveChecks();
        void translate(std::ostream* stream, std::string namespaceSoFar);

#if YYDEBUG
        int trace = 0;
        void graphPrint(std::ofstream* stream);
#endif
    };
}

#endif

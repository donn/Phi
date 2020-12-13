#ifndef _context_h
#define _context_h
#include "Types.h"
#include "SymbolTable.h"
#include "BuildInfo.h"

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
        std::shared_ptr<Node::Node> head = nullptr;
    public:
        Location noLocation() {
            return Location(&files.back(), 0, 0);
        }

        Location location(int start, int end) {
            return Location(&files.back(), start, end);
        }

        struct DriveCheck {
            std::shared_ptr<Driven> target;
            optional<AccessWidth> from;
            optional<AccessWidth> to;
            optional< std::function<void()> > ifUndriven;
            optional< std::function<void()> > ifDriven;

            DriveCheck(
                std::shared_ptr<Driven> target,
                optional<AccessWidth> from,
                optional<AccessWidth> to,
                optional< std::function<void()> > ifUndriven = nullopt,
                optional< std::function<void()> > ifDriven = nullopt
            ): target(target), from(from), to(to), ifUndriven(ifUndriven), ifDriven(ifDriven) {}
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
        void driveChecks(SymbolTable* tablePtr); // Must use same table as elaboration or you can happily risk a segfault.
        void translate(std::ostream* stream);

#if YYDEBUG
        int trace = 0;
        void graphPrint(std::ofstream* stream);
#endif
    };
}

#endif

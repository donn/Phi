#ifndef _context_h
#define _context_h
// C++ STL
#include <vector>

// Flex/Bison
#include <location.hh>

// Project Headers
#include "Types.h"

using Location = Phi::location;

namespace Phi {
    struct Error {
        Location loc;
        std::string message;
    };

    class Context {
        std::string executableName;
    public:
        Context(const char* argv0): executableName(argv0) {}

        std::vector<Error> errorList;
        std::vector<std::string> files;
        std::vector<std::string> currentFileLines;
        char* top = nullptr;

        optional<std::string> setFile(std::string currentFile);
        bool error();
        void printErrors();

#if YYDEBUG
        int trace = 0;
#endif
    };
}

#endif

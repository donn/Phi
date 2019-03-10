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
        String message;
    };

    class Context {
        String executableName;
    public:
        Context(const char* argv0): executableName(argv0) {}

        std::vector<Error> errorList;
        std::vector<String> files;
        std::vector<String> currentFileLines;
        char* top = nullptr;

        optional<String> setFile(String currentFile);
        bool error();
        void printErrors();

#if YYDEBUG
        int trace = 0;
#endif
    };
}

#endif

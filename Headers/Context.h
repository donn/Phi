#ifndef _context_h
#define _context_h
// CPP STL
#include <string>
#include <vector>

// Flex/Bison
#include <location.hh>

using Location = Phi::location;

namespace Phi {
    struct Error {
        Location loc;
        std::string message;
    };

    namespace ExpType {
        enum Enum {
            CompileTime = 0,
            ParameterSensitive,
            RunTime,

            Undefined = 0xFF
        };
    };

    class Context {
        std::string executableName;
    public:
        Context(const char* argv0): executableName(argv0) {}

        std::vector<Error> errorList;
        std::vector<std::string> files;
        std::vector<std::string> currentFileLines;
        char* top = NULL;

        std::string setFile(std::string currentFile);
        bool error();
        void printErrors();

#if YYDEBUG
        int trace = 0;
#endif
    };
}

#endif
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

    class Context {
    public:
        std::vector<Error> errorList;
        std::vector<std::string> files;
        std::vector<std::string> currentFileLines;
        char* top = NULL;
        bool _HACK_FOR_ALWAYS = false;

        std::string setFile(std::string currentFile);
        bool error();
        void printErrors();
    };
}

#endif
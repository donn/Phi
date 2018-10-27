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

        void setFile(std::string currentFile) { files.push_back(currentFile); }
        bool error();
        void printErrors();
    };
}

#endif
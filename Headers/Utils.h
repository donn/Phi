#ifndef _utils_h
#define _utils_h

// C++ STL
#include <vector>

// Project Headers
#include "Types.h"

namespace Phi {
    namespace Utils {
        std::vector<std::string> split(const std::string* string, char delimiter);
        std::string join(std::vector<std::string>* arrays, char delimiter);
    }
}
#endif
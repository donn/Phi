#ifndef _utils_h
#define _utils_h

// C++ STL
#include <vector>

// Project Headers
#include "Types.h"

namespace Phi {
    namespace Utils {
        std::vector<String> split(const String* string, char delimiter);
    }
}
#endif
#ifndef _utils_h
#define _utils_h

#include <string>
#include <vector>

namespace Phi {
    namespace Utils {
        std::vector<std::string> split(const std::string* string, char delimiter);
    }
}
#endif
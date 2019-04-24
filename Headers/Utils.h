#ifndef _utils_h
#define _utils_h

#include "Types.h"

#include <vector>
#include <llvm/ADT/APInt.h>


namespace Phi {
    namespace Utils {
        std::vector<std::string> split(const std::string* string, char delimiter);
        std::string join(std::vector<std::string>* arrays, char delimiter);
        bool apIntCheck(llvm::APInt* integer, uint64 limit);
    }
}
#endif
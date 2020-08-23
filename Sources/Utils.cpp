#include "Utils.h"

#include <sstream>

using namespace Phi;

std::vector<std::string> Utils::split(const std::string* string, char delimiter) {
    std::vector<std::string> returnValue;
    std::stringstream ss;
    ss.str(*string);

    std::string item;
    while (std::getline(ss, item, delimiter)) {
        returnValue.push_back(item);
    }

    return returnValue;
}

std::string Utils::join(std::vector<std::string>* array, char delimiter) {
    std::stringstream ss;

    for (auto& string: *array) {
        ss << string << delimiter;
    }
    
    return ss.str();
}

bool Utils::apIntCheck(llvm::APInt* integer, uint64 limit) {
    return integer->ult(llvm::APInt(64, limit));
}
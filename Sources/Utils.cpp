#include "Utils.h"

#include <sstream>

std::vector<std::string> Phi::Utils::split(const std::string* string, char delimiter) {
    std::vector<std::string> returnValue;
    std::stringstream ss;
    ss.str(*string);

    std::string item;
    while (std::getline(ss, item, delimiter)) {
        returnValue.push_back(item);
    }

    return returnValue;
}

std::string Phi::Utils::join(std::vector<std::string>* arrays, char delimiter) {
    std::stringstream ss;
    for (auto& string: *arrays) {
        ss << string << delimiter;
    }
    return ss.str();
}
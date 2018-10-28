#include "Utils.h"

#include <iostream>
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
#include "Utils.h"

#include <iostream>
#include <sstream>

std::vector<std::string> Phi::Utils::split(const std::string* string, std::string delimiter) {
    std::vector<std::string> returnValue;
    std::stringstream ss;
    ss.str(*string);

    std::string item;
    while (std::getline(ss, item, delimiter[0])) {
        returnValue.push_back(item);
    }

    return returnValue;
}
#include "Utils.h"

#include <iostream>
#include <sstream>

std::vector<String> Phi::Utils::split(const String* string, char delimiter) {
    std::vector<String> returnValue;
    std::stringstream ss;
    ss.str(*string);

    String item;
    while (std::getline(ss, item, delimiter)) {
        returnValue.push_back(item);
    }

    return returnValue;
}
#include "Errors.h"

std::vector<Error> Error::list = std::vector<Error>();

void Phi_appendError(int line, int column, const char* message, const char* content) {
    Error::list.push_back(Error(line, column, message, content));
}
#ifndef _errors_h
#define _errors_h

#ifdef __cplusplus
#include <string>
#include <vector>

struct Error {
    int line;
    int column;
    std::string message;
    std::string content;

    static std::vector<Error> list;

    Error(int line, int column, const char* message, const char* content) : line(line), column(column), message(std::string(message)), content(std::string(content)) {}
};

extern "C" {
#endif

void Phi_appendError(int line, int column, const char* type, const char* content);

#ifdef __cplusplus
}
#endif

#endif // _errors_h

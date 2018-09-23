#include <iostream>
#include <fstream>
#include <sstream>
extern "C" {
    #include <phi.l.h>
    #include <phi.y.h>
    #include "Node.h"
    
    int yyparse();
    struct Node* head;
}


int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cout << "Invocation: ./phi <input.phi>" << std::endl;
        return 64;
    }

    std::stringstream stringstream;
    stringstream << std::ifstream(argv[1]).rdbuf();
    auto input = stringstream.str();

    yy_scan_string(input.c_str());
    yyparse();
    traverse(head);
    
}
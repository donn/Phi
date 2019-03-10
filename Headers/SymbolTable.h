#ifndef _symboltable_h
#define _symboltable_h

// CPP STL
#include <map>
#include <vector>

// Project Headers
#include "Types.h"
#include "Context.h"

enum class SymbolType {
    nspace = 0,
    module
};

struct Symbol {
    String id;
    SymbolType type;

    Symbol(String id, SymbolType type): id(id), type(type) {}
};

struct SymbolSpace: public Symbol {
    std::map<String, Symbol> space;

    SymbolSpace(String id, SymbolType type): Symbol(id, type) {
        space = std::map<String, Symbol>();
    }
};

class SymbolTable {
    SymbolSpace* head = nullptr;
    std::vector<SymbolSpace*> stack;

public: 
    SymbolTable();
    ~SymbolTable();


    void add(Symbol symbol);
    void checkExistence(std::vector<String> ids, SymbolType type);
    void nestInto(SymbolSpace newSpace);
    void nestOut();
};

#endif
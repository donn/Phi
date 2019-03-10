#include "SymbolTable.h"

#define top stack.back()

SymbolTable::SymbolTable() {
    head = new SymbolSpace("", SymbolType::nspace);
    stack.push_back(head);
}

SymbolTable::~SymbolTable() {
    delete head;
}

void SymbolTable::add(Symbol symbol) {
    auto& name = symbol.id;
    auto insertResult = top->space.insert(std::pair(name, symbol));
    if (insertResult.second == false) {
        throw "symbol.redefinition(`name`)";
    }
}

void SymbolTable::nestInto(SymbolSpace space) {
    auto& name = space.id;
    auto insertResult = top->space.insert(std::pair(name, space));
    if (insertResult.second == false) {
        throw "symbol.redefinition(`name`)";
    }

    auto iterator = insertResult.first;
    auto& pair = *iterator;
    auto& symbol = pair.second;
    stack.push_back((SymbolSpace*)&symbol);
}

void SymbolTable::checkExistence(std::vector<String> ids, SymbolType type) {
    bool found = false;
    for (auto i = stack.rbegin(); i != stack.rend() && !found; i++) {
        auto currentSpace = *i;
        
        bool flag = true;
        for (auto j = ids.begin(); j != ids.end() && flag; j++) {
            auto k = currentSpace->space.find(*j);
            if (k == currentSpace->space.end()) {
                flag = false;
            } else {
                currentSpace = (SymbolSpace*)&*k;
            }
        }
        found = flag;
    }
    unless (found) {
        throw "symbol.dne(`name`)";
    }
}

void SymbolTable::nestOut() {
    stack.pop_back();
}
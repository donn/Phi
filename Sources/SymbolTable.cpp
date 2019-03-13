#include "SymbolTable.h"
using namespace Phi;

#define top stack.back()

SymbolTable::SymbolTable() {
    head = std::make_shared<SymbolSpace>("", SymbolType::nspace);
    stack.push_back(head);
}

SymbolTable::~SymbolTable() {
}

void SymbolTable::add(std::string id, SymbolType type, bool space) {

    if (top->space.find(id) != top->space.end()) {
        throw std::string("symbol.redefinition(`") + id + "`)";
    }
    
    if (space) {
        top->space[id] = std::make_shared<SymbolSpace>(id, type);
    } else {
        top->space[id] = std::make_shared<Symbol>(id, type);
    }
}

void SymbolTable::nestInto(std::string space) {
    auto iterator = top->space.find(space);
    if (iterator == top->space.end()) {
        throw std::string("symbol.dne(`") + space + "`)";
    }
    auto& object = *iterator;
    stack.push_back(std::dynamic_pointer_cast<SymbolSpace>(object.second));
}

void SymbolTable::nestIntoAndCreate(std::string space, SymbolType type) {
    add(space, type, true);
    nestInto(space);
}

void SymbolTable::checkExistence(std::vector<std::string> ids, SymbolType type) {
    bool found = false;
    for (auto i = stack.rbegin(); i != stack.rend() && !found; i++) {
        auto pointer = *i;
        bool flag = true;
        for (auto j = ids.begin(); j != ids.end() && flag; j++) {
            auto next = pointer->space.find(*j);
            auto& target = *next;
#if 0 // Fix later
            std::cout << (*j) << " in " << (pointer->id) << ": " << (next != pointer->space.end() ? "Found" : "Not Found") << std::endl;
#endif
            if (next == pointer->space.end()) {
                flag = false;
            } else {
                pointer = std::dynamic_pointer_cast<SymbolSpace>(target.second);
                if (pointer == nullptr && std::next(j) != ids.end()) {
                    flag = false;
                }
            }
        }
        found = flag;
    }
    unless (found) {
        throw std::string("symbol.dne(`") + Utils::join(&ids, '.') + "`)";
    }
}

void SymbolTable::nestOut() {
    stack.pop_back();
}
#include "SymbolTable.h"
using namespace Phi;

#define top stack.back()

SymbolTable::SymbolTable() {
    head = std::make_shared<SymbolSpace>("", nullptr);
    stack.push_back(head);
}

SymbolTable::~SymbolTable() {
}

void SymbolTable::add(std::string id, Node::Node* attached, bool space) {

    if (top->space.find(id) != top->space.end()) {
        throw std::string("symbol.redefinition(`") + id + "`)";
    }
    
    if (space) {
        top->space[id] = std::make_shared<SymbolSpace>(id, attached);
    } else {
        top->space[id] = std::make_shared<Symbol>(id, attached);
    }
}

void SymbolTable::stepInto(std::string space) {
    auto iterator = top->space.find(space);
    if (iterator == top->space.end()) {
        throw std::string("symbol.dne(`") + space + "`)";
    }
    auto& object = *iterator;
    stack.push_back(std::dynamic_pointer_cast<SymbolSpace>(object.second));
}

void SymbolTable::stepIntoAndCreate(std::string space, Node::Node* attached) {
    add(space, attached, true);
    stepInto(space);
}

std::shared_ptr<Symbol>  SymbolTable::checkExistence(std::vector<std::string> ids, Node::Node* attached) {
    std::shared_ptr<Symbol> found = nullptr;
    for (auto i = stack.rbegin(); i != stack.rend() && (found == nullptr); i++) {
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
        found = flag ? pointer : NULL;
    }
    return found;
}

void SymbolTable::stepOut() {
    stack.pop_back();
}

// Debug
#include <iomanip>

void SymbolSpace::represent(std::ostream* stream, int nesting) {
    auto& out = *stream;
    for (auto& symbol: space) {
        out << std::setw(nesting * 4) << ' ' << symbol.first << std::endl;
        if (auto spacePointer = std::dynamic_pointer_cast<SymbolSpace>(symbol.second)) {
            spacePointer->represent(stream, nesting + 1);
        }
    }
}

bool SymbolSpace::inComb() {
    return top->isComb();
}

void SymbolTable::represent(std::ostream* stream) {
    head->represent(stream);
}
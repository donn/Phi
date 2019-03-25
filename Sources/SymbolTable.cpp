#include "SymbolTable.h"
using namespace Phi;

#define top stack.back()

SymbolTable::SymbolTable() {
    head = std::make_shared<SymbolSpace>("", nullptr);
    stack.push_back(head);
}

SymbolTable::~SymbolTable() {
}

void SymbolTable::add(std::string id, Node::Node* attached, bool space, Node::Node* driver) {

    if (top->space.find(id) != top->space.end()) {
        throw std::string("symbol.redefinition(`") + id + "`)";
    }
    
    if (space) {
        if (driver) {
            throw std::string("symbol.drivingANamespace");
        }
        top->space[id] = std::make_shared<SymbolSpace>(id, attached);
    } else {
        top->space[id] = std::make_shared<Symbol>(id, attached, driver);
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

std::shared_ptr<Symbol>  SymbolTable::checkExistence(std::vector<std::string> ids) {
    for (auto i = stack.rbegin(); i != stack.rend(); i++) {
        auto pointer = *i;
        bool flag = true;
        for (auto j = ids.begin(); j != ids.end() && flag; j++) {
            auto next = pointer->space.find(*j);
            auto& target = *next;
            // std::cout << (*j) << " in " << (pointer->id) << ": " << (next != pointer->space.end() ? "Found" : "Not Found") << std::endl;
            if (next == pointer->space.end()) {
                flag = false;
            } else {
                if (std::next(j) == ids.end()) {
                    return next->second;
                }
                pointer = std::dynamic_pointer_cast<SymbolSpace>(target.second);
                if (pointer == nullptr) {
                    flag = false;
                }
            }
        }
    }
    return nullptr;
}

void SymbolTable::stepOut() {
    stack.pop_back();
}

void SymbolTable::stepIntoComb(Node::Node* attached) {
    top->space["\\comb"] = std::make_shared<SymbolSpace>("", attached, true);
    stepInto("\\comb");
}

bool SymbolTable::inComb() {
    return top->isComb;
}

#if YYDEBUG
void SymbolTable::represent(std::ostream* stream) {
    *stream << "strict graph symbolTable {" << std::endl;
    int counter = 0;
    *stream << "0 [label=\"Global Namespace\"];" << std::endl;
    head->represent(stream, &counter);
    *stream << "}" << std::endl;
}

// Debug
#include <iomanip>

int SymbolSpace::represent(std::ostream* stream, int* node) {
    auto current = *node;
    for (auto& symbol: space) {
        *node = *node + 1;
        *stream << *node << " " << "[label=\"" << symbol.first << "\"]" << ";" << std::endl;
        auto connection = *node;
        if (auto spacePointer = std::dynamic_pointer_cast<SymbolSpace>(symbol.second)) {
            connection = spacePointer->represent(stream, node);
        }
        *stream << current << " -- " << connection << ";" << std::endl;
    }
    return current;
}
#endif
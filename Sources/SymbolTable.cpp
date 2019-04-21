#include "SymbolTable.h"
using namespace Phi;

#include <stack>

#define tableTop stack.back()

SymbolTable::SymbolTable() {
    head = std::make_shared<SymbolSpace>("", nullptr);
    stack.push_back(head);
}

SymbolTable::~SymbolTable() {
}

void SymbolTable::add(std::string id, std::shared_ptr<Symbol> symbol) {
    if (tableTop->space.find(id) != tableTop->space.end()) {
        throw std::string("symbol.redefinition(`") + id + "`)";
    }

    tableTop->space[id] = symbol;
}

void SymbolTable::stepInto(std::string space) {
    auto iterator = tableTop->space.find(space);
    if (iterator == tableTop->space.end()) {
        throw std::string("symbol.dne(`") + space + "`)";
    }
    auto& object = *iterator;
    stack.push_back(std::dynamic_pointer_cast<SymbolSpace>(object.second));
}

void SymbolTable::stepIntoAndCreate(std::string space, Node::Node* declarator) {
    if (tableTop->space.find(space) != tableTop->space.end()) {
        throw std::string("symbol.redefinition(`") + space + "`)";
    }

    tableTop->space[space] = std::make_shared<SymbolSpace>(space, declarator);
    stepInto(space);
}

optional< std::shared_ptr<Symbol> > SymbolTable::find(Node::LHExpression* findable) {
    using namespace Phi::Node;

    for (auto i = stack.rbegin(); i != stack.rend(); i++) {
        std::shared_ptr<Symbol> stackPointer = *i;
        bool flag = true;

        // LHExpression-based approach
        std::stack<LHExpression*> lhStack;
        lhStack.push(findable);
        while (!lhStack.empty()) {
            auto top = lhStack.top();
            lhStack.pop();
            if (top->left && top->right) {
                lhStack.push((LHExpression*)top->right);
                lhStack.push((LHExpression*)top->left);
            } else {
                assert(!top->left && !top->right);
                if (auto pointer = dynamic_cast<IdentifierExpression*>(top)) {
                    // Namespaced Access
                    // First: Make sure we're in symbol space
                    auto space = std::dynamic_pointer_cast<SymbolSpace>(stackPointer);
                    if (space == nullptr) {
                        continue;
                    }

                    // Second: Check if symbol exists
                    auto next = space->space.find(pointer->identifier->idString);
                    if (next == space->space.end()) {
                        // If not, we just leave gracefully. There might be a similarly named space at a higher level.
                        continue;
                    }

                    // Third: What to do next...
                    if (stack.empty()) {
                        return next->second;
                    }

                    stackPointer = next->second;
                } else if (auto pointer = dynamic_cast<Expression*>(top)) {
                    // Array Access
                    // First: Make sure we're in an array of symbols, and check that it's not parameter sensitive
                    auto array = std::dynamic_pointer_cast<SymbolArray>(stackPointer);
                    if (array == nullptr) {
                        continue;
                    }

                    if (array->array.size() == 0) {
                        // PARSEN
                        throw "parsenAssert";
                    } 

                    if (pointer->type == Expression::Type::Error) {
                        return nullopt;
                    }

                    // Second: Process the expression
                    if (pointer->type == Expression::Type::ParameterSensitive) {
                        // PARSEN
                        throw "parsenAssert";
                    }

                    if (pointer->type == Expression::Type::RunTime) {
                        return array;
                    }

                    auto& apInt = pointer->value.value();
                    if (!Utils::apIntCheck(&apInt, Expression::maxWidth)) {
                        throw "expr.exceedsSize";
                    }

                } else if (auto pointer = dynamic_cast<Range*>(top)) {
                    // Range Access
                }
            }

        }

        // // Vector-based approach (Removed)
        // for (auto j = ids.begin(); j != ids.end() && flag; j++) {
        //     auto next = pointer->space.find(*j);
        //     auto& target = *next;
        //     // std::cout << (*j) << " in " << (pointer->id) << ": " << (next != pointer->space.end() ? "Found" : "Not Found") << std::endl;
        //     if (next == pointer->space.end()) {
        //         flag = false;
        //     } else {
        //         if (std::next(j) == ids.end()) {
        //             return next->second;
        //         }
        //         pointer = std::dynamic_pointer_cast<SymbolSpace>(target.second);
        //         if (pointer == nullptr) {
        //             flag = false;
        //         }
        //     }
        // }
    }
    return nullptr;
}

void SymbolTable::stepOut() {
    stack.pop_back();
}

void SymbolTable::stepIntoComb(Node::Node* attached) {
    tableTop->space["_comb"] = std::make_shared<SymbolSpace>("_comb", attached, true);
    stepInto("_comb");
}

bool SymbolTable::inComb() {
    return tableTop->isComb;
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
#include "SymbolTable.h"
#include "Node.h"

#include <stack>

using namespace Phi;

#define tableTop stack.back()

bool Driven::drive(Node::Expression* expression, optional<AccessWidth> fromOptional, optional<AccessWidth> toOptional) {
    AccessWidth from = fromOptional.has_value() ? fromOptional.value() : this->from;
    AccessWidth to = toOptional.has_value() ? toOptional.value() : this->to;

    AccessWidth width;
    // Check for any crossover
    if (from <= to) {
        width = to - from + 1;
        for (auto& range: driveRanges) {
            if (from >= range.from && from <= range.to) {
                return false;
            }
        }
    } else {
        width = from - to + 1;
        for (auto& range: driveRanges) {
            if (from <= range.from && from >= range.to) {
                return false;
            }
        }
    }

    if (expression->numBits != width) {
        throw "driving.widthMismatch";
    }

    driveRanges.emplace(DriveRange(expression, from, to));
    return true;
}

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

static bool checkRangeCoverage(std::multiset<DriveRange>* driveRanges, bool msbFirst, AccessWidth from, AccessWidth to) {
    auto& ranges = *driveRanges;
    if (from <= to) {
        for (auto& range: ranges) {
            if (from >= range.from && from <= range.to) {
                if (to <= range.to) {
                    return true;
                } else {
                    from = range.to + 1;
                }
            }
        }
    } else {
        for (auto& range: ranges) {
            if (from <= range.from && from >= range.to) {
                if (to >= range.to) {
                    return true;
                } else {
                    from = range.to - 1;
                }
            }
        }
    }
    return false;
}

static bool checkRangeCoverage(std::multiset<DriveRange>* driveRanges, bool msbFirst, AccessWidth unit) {
    auto& ranges = *driveRanges;
    if (msbFirst) {
        for (auto& range: ranges) {
            if (unit >= range.from && unit <= range.to) {
                return true;
            }
        }
    } else {
        for (auto& range: ranges) {
            if (unit <= range.from && unit >= range.to) {
                return true;
            }
        }
    }
    return false;
}

#include <iostream>

optional< std::shared_ptr<Symbol> > SymbolTable::find(std::vector<Access>* accessesPtr) {
    auto& accesses = *accessesPtr;
    for (auto i = stack.rbegin(); i != stack.rend(); i++) {
        std::shared_ptr<Symbol> pointer = *i;
        bool flag = true;
        for (auto j = accesses.begin(); j != accesses.end() && flag; j++) {
            auto& access = *j;

            if (access.type == Access::Type::id) {
                auto pointerAsSpace = std::dynamic_pointer_cast<SymbolSpace>(pointer);
                if (pointerAsSpace == nullptr) {
                    flag = false;
                    continue;
                }
                auto& id = *j->id;
                auto next = pointerAsSpace->space.find(id);
                // std::cout << (*j) << " in " << (pointer->id) << ": " << (next != pointer->space.end() ? "Found" : "Not Found") << std::endl;
                if (next == pointerAsSpace->space.end()) {
                    flag = false;
                    continue;
                }
                if (std::next(j) == accesses.end()) {
                    return next->second;
                }
                pointer = next->second;
            } else if (access.type == Access::Type::index) {
                if (auto pointerAsArray = std::dynamic_pointer_cast<SymbolArray>(pointer)) {
                    if (access.index >= pointerAsArray->array.size()) {
                        throw "symbol.outOfRangeAccess";
                    }

                    if (std::next(j) == accesses.end()) {
                        return pointerAsArray->array[access.index];
                    }

                    pointer = pointerAsArray->array[access.index];
                } else if (auto pointerAsDriven = std::dynamic_pointer_cast<Driven>(pointer)) {
                    if (
                    (pointerAsDriven->msbFirst && (access.index > pointerAsDriven->from || access.index < pointerAsDriven->to))
                    || (access.index < pointerAsDriven->from || access.index > pointerAsDriven->to)
                    ) {
                        throw "symbol.outOfRangeAccess";
                    }

                    if (std::next(j) != accesses.end()) {
                        throw "symbol.accessIsFinal";
                    }

                    return pointerAsDriven;
                } else {
                    throw "symbol.notIndexable";
                } 
            }
            // else if (access.type == Access::Type::range) {
            //     auto pointerAsDriven = std::dynamic_pointer_cast<Driven>(pointer);

            //     if (
            //         (pointerAsDriven->msbFirst && (access.range.from > pointerAsDriven->from || access.range.to < pointerAsDriven->to))
            //         || (access.range.from < pointerAsDriven->from || access.range.to > pointerAsDriven->to)
            //     ) {
            //         throw "driven.outOfRangeAccess";
            //     }

            //     if (std::next(j) != accesses.end()) {
            //         throw "driven.accessIsFinal";
            //     }
                
            //     return pointerAsDriven;
            // } 
        }
    }
    return nullopt;
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

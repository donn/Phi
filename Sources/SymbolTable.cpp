#include "SymbolTable.h"
#include "Node.h"

#include <stack>
using namespace Phi;

#define tableTop stack.back()

bool Driven::drive(Node::Expression* expression, optional<AccessWidth> fromOptional, optional<AccessWidth> toOptional) {
    AccessWidth from = fromOptional.has_value() ? fromOptional.value() : this->from;
    AccessWidth to = toOptional.has_value() ? toOptional.value() : this->to;
    
    // Check for any crossover
    if (from <= to) {
        for (auto& range: driveRanges) {
            if (from >= range.from && from <= range.to) {
                return false;
            }
        }
    } else {
        for (auto& range: driveRanges) {
            if (from <= range.from && from >= range.to) {
                return false;
            }
        }
    }

    driveRanges.emplace(DriveRange(expression, from, to));
    return true;
}

Argument::FunctionValue Function::call(Argument::List* listPtr) {
    // First: Check
    auto& list = *listPtr;
    if (list.size() != parameterList.size()){
        throw "function.unexpectedParameterCount";
    }
    for (size_t i = 0; i < list.size(); i += 1) {
        if (list[i].type != parameterList[i]) {
            throw "function.invalidParameters";
        }
    }
    // Second: Execute
    return behavior(listPtr);
}

SymbolTable::SymbolTable() {
    head = std::make_shared<SymbolSpace>("", nullptr);
    stack.push_back(head);

    // Create functions
    stepIntoAndCreate("Sys", NULL);
    using Parameter = Argument::Type;
    auto sysPow = std::shared_ptr<Function>(new Function("pow", {Parameter::expression, Parameter::expression}, [](Argument::List* argList) {
        auto& list = *argList;

        // Check list
        auto number = list[0].expression.value();
        auto exponent = list[1].expression.value();

        if (
            number.second > 52
            || exponent.second > 52
        ) {
            throw "pow.52bitexceeded";
        }
        
        double numberD = number.first.getLimitedValue();
        double exponentD = exponent.first.getLimitedValue();

        double returnD;
        
        try {
            returnD = std::pow(numberD, exponentD);
        } catch (int error) {
            returnD = INFINITY;
        }

        uint64 returnedValue = returnD;

        return std::pair(llvm::APInt(number.second, returnedValue), number.second);
    }));
    add("pow", sysPow);
    stepOut();
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

std::vector<DriveRange> Driven::checkRangeCoverage(AccessWidth from, AccessWidth to) {
    std::vector<DriveRange> returnValues;

    bool covered = false;
    
    if (msbFirst) {
        if (to > from) {
            throw "symbol.rangeOrderViolation";
        }
        for (auto& range: driveRanges) {
            if (from <= range.from && from >= range.to) {
                returnValues.push_back(range);
                if (to >= range.to) {
                    covered = true;
                    break;
                } else {
                    from = range.to - 1;
                }
            }
        }
    } else {
        if (from > to) {
            throw "symbol.rangeOrderViolation";
        }

        for (auto& range: driveRanges) {
            if (from >= range.from && from <= range.to) {
                returnValues.push_back(range);
                if (to <= range.to) {
                    covered = true;
                    break;
                } else {
                    from = range.to + 1;
                }
            }
        }
    }
    return covered ? returnValues : std::vector<DriveRange>();
}

optional<DriveRange> Driven::checkRangeCoverage(AccessWidth unit) {
    if (msbFirst) {
        for (auto& range: driveRanges) {
            if (unit <= range.from && unit >= range.to) {
                return range;
            }
        }
    } else {
        for (auto& range: driveRanges) {
            if (unit >= range.from && unit <= range.to) {
                return range;
            }
        }
    }
    return nullopt;
}
optional< std::shared_ptr<Symbol> > SymbolTable::find(std::vector<Access>* accessesPtr, optional<AccessWidth>* from, optional<AccessWidth>* to) {
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
                    *access.trueIndex = false;
                    
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
                    || (!pointerAsDriven->msbFirst && (access.index < pointerAsDriven->from || access.index > pointerAsDriven->to))
                    ) {
                        throw "symbol.outOfRangeAccess";
                    }

                    *from = access.index;
                    *to = access.index;

                    if (std::next(j) != accesses.end()) {
                        throw "symbol.accessIsFinal";
                    }

                    return pointerAsDriven;
                } else {
                    throw "symbol.notIndexable";
                } 
            }
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
    for (auto iterator = stack.rbegin(); iterator != stack.rend(); iterator++) {
        if ((*iterator)->isComb) {
            return true;
        }
    }
    return false;
} 

#if YYDEBUG
void SymbolTable::represent(std::ostream* stream) {
    *stream << "strict graph symbolTable {" << std::endl;
    int counter = 0;
    *stream << "0 [label=\"Global Namespace\"];" << std::endl;
    head->represent(stream, &counter);
    *stream << "}" << std::endl;
}

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

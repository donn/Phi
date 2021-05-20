#include "SymbolTable.h"
#include "Node.h"

#include <fstream>
#include <stack>
#include <regex>
#include <cmath>
#include <string>
using namespace Phi;

#define tableTop stack.back()

Function::Value Function::call(Argument::List* listPtr) {
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
    head = std::make_shared<Space>("", nullptr);
    stack.push_back(head);

    // Create functions
    stepIntoAndCreate("Phi", NULL);
    stepIntoAndCreate("Sys", NULL);
    using Parameter = Function::Argument::Type;

    auto sysAbs= std::shared_ptr<Function>(new Function("abs", {Parameter::expression}, [](Function::Argument::List* argList) {
        auto& list = *argList;

        // Check list
        auto number = list[0].expression.value();

        return std::pair(number.first.abs(), number.second);
    }));
    add("abs", sysAbs);


    auto sysLog= std::shared_ptr<Function>(new Function("log", {Parameter::expression, Parameter::expression}, [](Function::Argument::List* argList) {
        auto& list = *argList;

        // Check list
        auto number = list[0].expression.value();
        auto base = list[1].expression.value();

        double numberD = number.first.roundToDouble();
        double baseD = base.first.roundToDouble();

        double returnD;
            
        try {
            returnD = std::log(numberD) / std::log(baseD);
        } catch (int error) {
            returnD = INFINITY;
        }

        uint64 returnedValue = returnD;

        return std::pair(llvm::APInt(number.second, returnedValue), number.second);
    }));
    add("log", sysLog);


    auto sysPow = std::shared_ptr<Function>(new Function("pow", {Parameter::expression, Parameter::expression}, [](Function::Argument::List* argList) {
        auto& list = *argList;

        // Check list
        auto number = list[0].expression.value();
        auto exponent = list[1].expression.value();

        llvm::APInt returnedValue;

        if (
            number.second > 52
            || exponent.second > 52
        ) {
            returnedValue = number.first;
            for (llvm::APInt i(exponent.second, 1); i.ult(exponent.first); i += llvm::APInt(exponent.second, 1)) {
                returnedValue *= number.first;
            }
        } else {
            double numberD = number.first.getLimitedValue();
            double exponentD = exponent.first.getLimitedValue();

            double returnD;
            
            try {
                returnD = std::pow(numberD, exponentD);
            } catch (int error) {
                returnD = INFINITY;
            }

            uint64 returnInt = returnD;
            returnedValue = llvm::APInt(number.second, returnInt);
        }

        return std::pair(returnedValue, number.second);
    }));
    add("pow", sysPow);


    auto sysinterpretFromFile = std::shared_ptr<Function>(new Function("interpretFromFile", {Parameter::string, Parameter::expression}, [](Function::Argument::List* argList) {
        auto& list = *argList;

        // Check list
        auto fileName = list[0].string.value();
        auto lineNumberAPInt = list[1].expression.value().first;
        auto lineNumber = lineNumberAPInt.getLimitedValue();

        //open file 
        std::ifstream f;
        f.open(fileName);
        if (!f) {
            throw "function.fileNotFound";
        }
        
        //read from file
        std::string line;
        uint counter = 1;
        while(getline(f, line)) {
            if (counter == lineNumber) {
                break;
            }
            counter += 1;
        }

        //close file
        f.close();
        

        //convert line from string in to llvm::APInt
        auto interpretable = line;

        auto regex = std::regex("([0-9]+)([bodxh])([A-F0-9]+)");
        
        auto match = std::smatch();
        std::regex_match(interpretable, match, regex); // If it doesn't match, the regex here and in the .l file are mismatched.

        AccessWidth prospectiveWidth;
        try {
            prospectiveWidth = std::stoi(match[1]);
        } catch (std::invalid_argument& error) {
            throw "function.notANumber";
        }

        if (prospectiveWidth < 0 || prospectiveWidth > maxAccessWidth) {
            throw "expr.maxWidthExceeded";
        }

        std::string radixCharacter = match[2];
        uint8_t radix;

        switch(radixCharacter[0]) {
            case 'b':
                radix = 2;
                break;
            case 'o':
                radix = 8;
                break;
            case 'd':
                radix = 10;
                break;
            case 'x':
                radix = 16;
                break;
            case 'h':
                throw "fixedNumber.usedVerilogH";
                break;
            default:
                throw "FATAL";
        }

        auto numeric = std::string(match[3]);
        auto ref = llvm::StringRef(numeric);
        auto value = llvm::APInt(prospectiveWidth, ref, radix);
        
        return std::pair(value, prospectiveWidth);
    }));
    add("interpretFromFile", sysinterpretFromFile);

    auto sysfromFile = std::shared_ptr<Function>(new Function("fromFile", {Parameter::string, Parameter::expression, Parameter::expression, Parameter::expression}, [](Function::Argument::List* argList) {
        auto& list = *argList;


        // Check list
        auto fileName = list[0].string.value();
        auto offsetAPInt = list[1].expression.value().first;
        auto offset = offsetAPInt.getLimitedValue();
        auto bytesAPIInt = list[2].expression.value().first;
        auto bytes = bytesAPIInt.getLimitedValue();
        auto endianAPIInt = list[3].expression.value().first;
        auto endian = endianAPIInt.getLimitedValue(); // (1b0: Little endian, 1b1: big endian)

        //open binary file 
        std::ifstream binaryFile;
        binaryFile.open(fileName, std::ios::in | std::ios::binary);
        if (!binaryFile) {
            throw "function.fileNotFound";
        }

        // Seek to offset from the beginning of the file 
        uint start = offset;
        binaryFile.seekg(start , std::ios::beg); 

        //read from file
        uint length = bytes;
        std::vector<char> buffer(length);
        binaryFile.read(&buffer[0], length);

        // (1b0: Little endian, 1b1: big endian)
        if (endian == 0) {
            // little endian
            // swaping
            std::vector<char> tempBufferContiguous(length);
            char* tempBuffer = &tempBufferContiguous[0];
            int j = 0;
            for (uint i = length; i > 0; i -= 1) {
                tempBuffer[i] = buffer[j++];
            }
            //adjust buffer
            for (uint i = 0; i < length; i++) {
                buffer[i] = tempBuffer[i];
            }
        } else if (endian == 1) {
            // big endian: Do nothing
        } else {
            throw "function.invalidEndianness";
        }
        
        //close file
        binaryFile.close();

        auto value = llvm::APInt(bytes * 8, 0);
        for (uint i = 0; i < length; i++) {
            value = value | buffer[i];
            if (i != (length - 1)) {
                value = value << 8;
            }
        }

        return std::pair(value, bytes * 8);
    }));
    add("fromFile", sysfromFile);

    stepOut();
    stepOut();
}

SymbolTable::~SymbolTable() {
}

void SymbolTable::add(std::string id, std::shared_ptr<Symbol> symbol) {
    if (tableTop->space.find(id) != tableTop->space.end()) {
        throw "symbol.redefinition";
    }

    tableTop->space[id] = symbol;
}

void SymbolTable::stepInto(std::string space) {
    auto iterator = tableTop->space.find(space);
    if (iterator == tableTop->space.end()) {
        throw "symbol.dne";
    }
    auto& object = *iterator;
    stack.push_back(std::dynamic_pointer_cast<Space>(object.second));
}

std::shared_ptr<Space> SymbolTable::stepIntoAndCreate(std::string space, std::shared_ptr<Node::Node> declarator, Space::Type type) {
    if (tableTop->space.find(space) != tableTop->space.end()) {
        throw "symbol.redefinition";
    }

    std::shared_ptr<Space> createdSpace;
    if ((int)type < 10) { // Less than 10 are things with ports: i.e. interfaces and modules
        createdSpace = std::make_shared<SpaceWithPorts>(space, declarator, type);
    } else {
        createdSpace = std::make_shared<Space>(space, declarator, type);
    }
    tableTop->space[space] = createdSpace;
    stepInto(space);
    return createdSpace;
}

std::vector<Driver> Driven::checkRangeCoverage(AccessWidth from, AccessWidth to) {
    std::vector<Driver> returnValues;

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
    return covered ? returnValues : std::vector<Driver>();
}

optional<Driver> Driven::checkRangeCoverage(AccessWidth unit) {
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

bool Driven::drive(
    std::shared_ptr<Node::Expression> expression,
    optional<AccessWidth> fromOptional,
    optional<AccessWidth> toOptional,
    bool dry
) {
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

    if (!dry) {
        driveRanges.emplace(Driver(expression, from, to));
    }
    return true;
}

#if YYDEBUG
void SymbolTable::Access::representList(std::ostream* ostream, std::vector<Access>* list) {
    auto& accesses = *list;
    for (auto j = accesses.begin(); j != accesses.end(); j++) {
        auto& access = *j;
        if (access.type == Access::Type::id) {
            *ostream << ((j != accesses.begin()) ? "." : "") << access.id;
        } else if (access.type == Access::Type::index) {
            *ostream << "[" << access.index << "]";
        }
    }
    *ostream << std::endl;
}
#endif

std::tuple< optional< std::shared_ptr<Symbol> >, optional<AccessWidth>, optional<AccessWidth> > SymbolTable::find(
    std::vector<Access>* accessesPtr,
    optional<AccessWidth> from,
    optional<AccessWidth> to
) {
    auto& accesses = *accessesPtr;
    for (auto i = stack.rbegin(); i != stack.rend(); i++) {
        std::shared_ptr<Symbol> pointer = *i;
        bool flag = true;
        for (auto j = accesses.begin(); j != accesses.end() && flag; j++) {
            auto& access = *j;

            if (access.type == Access::Type::id) {
                auto pointerAsSpace = std::dynamic_pointer_cast<Space>(pointer);
                if (pointerAsSpace == nullptr) {
                    flag = false;
                    continue;
                }
                auto& id = j->id;
                auto next = pointerAsSpace->space.find(id);
                
                if (next == pointerAsSpace->space.end()) {
                    flag = false;
                    continue;
                }
                if (std::next(j) == accesses.end()) {
                    return { next->second, from, to } ;
                }
                pointer = next->second;
            } else if (access.type == Access::Type::index) {
                auto pointerAsSpace = std::dynamic_pointer_cast<Space>(pointer);
                if (pointerAsSpace && pointerAsSpace->type == Space::Type::array) {
                    *access.arrayIndex = true;
                    auto indexString = std::to_string(access.index);

                    auto& sp = pointerAsSpace->space;

                    if (sp.find(indexString) == sp.end()) {
                        throw "array.outOfRangeAccess";
                    }

                    if (std::next(j) == accesses.end()) {
                        return { pointerAsSpace->space[indexString], from, to };
                    }

                    pointer = pointerAsSpace->space[indexString];
                } else if (auto pointerAsDriven = std::dynamic_pointer_cast<Driven>(pointer)) {
                    if (
                    (pointerAsDriven->msbFirst && (access.index > pointerAsDriven->from || access.index < pointerAsDriven->to))
                    || (!pointerAsDriven->msbFirst && (access.index < pointerAsDriven->from || access.index > pointerAsDriven->to))
                    ) {
                        throw "symbol.outOfRangeAccess";
                    }

                    if (std::next(j) != accesses.end() || from.has_value() || to.has_value()) {
                        throw "symbol.accessIsFinal";
                    }

                    return { pointerAsDriven, access.index, access.index };
                } else {
                    throw "symbol.notIndexable";
                } 
            }
        }
    }
    return {nullopt, nullopt, nullopt};
}

void SymbolTable::stepOut() {
    stack.pop_back();
}

void SymbolTable::stepIntoComb(std::shared_ptr<Node::Node> attached) {
    tableTop->space["_comb"] = std::make_shared<Space>("_comb", attached, Space::Type::comb);
    stepInto("_comb");
}

std::shared_ptr<Space> SymbolTable::findNearest(Space::Type type) {
    for (auto iterator = stack.rbegin(); iterator != stack.rend(); iterator++) {
        if ((*iterator)->type == type) {
            return *iterator;
        }
    }
    return nullptr;
} 

std::vector<std::string> SymbolTable::whereAmI(optional<Space::Type> until) {
    std::vector<std::string> elements; 
    for (auto iterator = stack.rbegin(); iterator != stack.rend(); iterator++) {
        auto& el = *iterator;
        if (el->type == until) {
            break;
        }
        elements.push_back(el->id);
    }
    std::reverse(std::begin(elements), std::end(elements));
    return elements;
}


void Space::moduleMetadata(std::stringstream* ssptr) {
    auto& ss = *ssptr;
    if (id != "") {
        ss << "\"" << id << "\":{";    
        ss << "\"members\":{";
    }
    for (auto it = space.begin(); it != space.end(); it++) {
        auto& element = *it;
        if (
            auto sp = std::dynamic_pointer_cast<Space>(element.second)
        ) {
            if (sp->id != "Phi") {
                if (it != space.begin()) {
                    ss << ",";
                }
                sp->moduleMetadata(ssptr);
            }
        }
    }
    if (id != "") {
        ss << "}";
        ss << "}";
    }
}

void SpaceWithPorts::moduleMetadata(std::stringstream* ssptr) {
    auto& ss = *ssptr;
    ss << "\"" << id << "\":{";
    
    ss << "\"ports\":{";
    for (auto it = ports.begin(); it != ports.end(); it++) {
        auto& port = *it;

        if (it != ports.begin()) {
            ss << ",";
        }

        ss << "\"" << port->getName() << "\":{";
        ss << "\"width\": " << port->getWidth();

        ss << ",";

        auto polarity = port->getPolarity();
        auto polarityS = polarity == PortObject::Polarity::input ? "input" : polarity == PortObject::Polarity::output ? "output" : "other";
        
        ss << "\"polarity\": \"" << polarityS << "\"";
        ss << "}";
    }
    ss << "}";
    ss << "}";
}

std::string SymbolTable::moduleMetadata() {
    std::stringstream ss;
    ss << "{";
    head->moduleMetadata(&ss);
    ss << "}";
    return ss.str();
}

#if YYDEBUG
void SymbolTable::represent(std::ostream* stream) {
    *stream << "strict graph symbolTable {" << std::endl;
    int counter = 0;
    *stream << "0 [label=\"Global Namespace\"];" << std::endl;
    head->represent(stream, &counter);
    *stream << "}" << std::endl;
}

int Space::represent(std::ostream* stream, int* node) {
    auto current = *node;
    for (auto& symbol: space) {
        *node = *node + 1;
        *stream << *node << " " << "[label=\"" << symbol.first << "\"]" << ";" << std::endl;
        auto connection = *node;
        if (auto spacePointer = std::dynamic_pointer_cast<Space>(symbol.second)) {
            connection = spacePointer->represent(stream, node);
        }
        *stream << current << " -- " << connection << ";" << std::endl;
    }
    return current;
}
#endif
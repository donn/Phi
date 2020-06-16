#include "SymbolTable.h"
#include "Node.h"
#include "JSON.h"

#include <fstream>
#include <stack>
#include <regex>
#include <cmath>
#include <string>
using namespace Phi;

#define tableTop stack.back()

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
    head = std::make_shared<Space>("", nullptr);
    stack.push_back(head);

    // Create functions
    stepIntoAndCreate("Sys", NULL);
    using Parameter = Argument::Type;


    auto sysAbs= std::shared_ptr<Function>(new Function("abs", {Parameter::expression}, [](Argument::List* argList) {
        auto& list = *argList;

        // Check list
        auto number = list[0].expression.value();

        return std::pair(number.first.abs(), number.second);
    }));
    add("abs", sysAbs);


    auto sysLog= std::shared_ptr<Function>(new Function("log", {Parameter::expression, Parameter::expression}, [](Argument::List* argList) {
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


    auto sysPow = std::shared_ptr<Function>(new Function("pow", {Parameter::expression, Parameter::expression}, [](Argument::List* argList) {
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


    auto sysinterpretFromFile = std::shared_ptr<Function>(new Function("interpretFromFile", {Parameter::string, Parameter::expression}, [](Argument::List* argList) {
        auto& list = *argList;

        // Check list
        auto fileName = list[0].string.value();
        auto lineNumberAPInt = list[1].expression.value().first;
        auto lineNumber = lineNumberAPInt.getLimitedValue();

        //open file 
        std::ifstream f;
        f.open(fileName);
        if (!f) {
            throw "io.fileNotFound";
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
            throw "interpretFromFile.notANumber";
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
        auto ref = llvm::StringRef(match[3]);
        auto value = llvm::APInt(prospectiveWidth, ref, radix);
        
        return std::pair(value, prospectiveWidth);
    }));
    add("interpretFromFile", sysinterpretFromFile);



    auto sysfromFile = std::shared_ptr<Function>(new Function("fromFile", {Parameter::string, Parameter::expression, Parameter::expression, Parameter::expression}, [](Argument::List* argList) {
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
            throw "io.fileNotFound";
        }

        // Seek to offset from the beginning of the file 
        uint start = offset;
        binaryFile.seekg(start , std::ios::beg); 

        //read from file
        uint length = bytes;
        std::vector<char> buffer(length);
        binaryFile.read(&buffer[0], length);

        // (1b0: Little endian, 1b1: big endian)
        if (endian == 0){
            // little endian
            // swaping
            std::vector<char> tempBufferContiguous(length);
            char* tempBuffer = &tempBufferContiguous[0];
            int j=0;
            for(uint i=length; i>0; i=i-1){
                tempBuffer[i] = buffer[j];
                //increment j
                j++;
            }
            //adjust buffer
            for(uint i=0; i<length; i++){
                buffer[i] = tempBuffer[i];
            }
        }else if (endian == 1){
            // big endian
            // do nothing 
        } else {
            throw "fromFile.invalidEndianness";
        }
        
        //close file
        binaryFile.close();

        auto value = llvm::APInt(bytes * 8, 0);
        for(uint i=0; i<length; i++){
            value = value | buffer[i];
            if (i!=length-1)
                value = value << 8;
        }

        return std::pair(value, bytes * 8);
    }));
    add("fromFile", sysfromFile);

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

void SymbolTable::stepIntoAndCreate(std::string space, std::shared_ptr<Node::Node> declarator, Space::Type type) {
    if (tableTop->space.find(space) != tableTop->space.end()) {
        throw "symbol.redefinition";
    }

    if ((int)type < 10) { // Less than 10 are things with ports: i.e. interfaces and modules
        tableTop->space[space] = std::make_shared<SpaceWithPorts>(space, declarator, type);
    } else {
        tableTop->space[space] = std::make_shared<Space>(space, declarator, type);
    }
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
        driveRanges.emplace(DriveRange(expression, from, to));
    }
    return true;
}


std::tuple< optional< std::shared_ptr<Symbol> >, optional<AccessWidth>, optional<AccessWidth> > SymbolTable::find(
    std::vector<Access>* accessesPtr
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
                    return { next->second, nullopt, nullopt } ;
                }
                pointer = next->second;
            } else if (access.type == Access::Type::index) {
                if (auto pointerAsArray = std::dynamic_pointer_cast<SymbolArray>(pointer)) {
                    *access.trueIndex = false;
                    
                    if (access.index >= pointerAsArray->array.size()) {
                        throw "symbol.outOfRangeAccess";
                    }

                    if (std::next(j) == accesses.end()) {
                        return { pointerAsArray->array[access.index], nullopt, nullopt };
                    }

                    pointer = pointerAsArray->array[access.index];
                } else if (auto pointerAsDriven = std::dynamic_pointer_cast<Driven>(pointer)) {
                    if (
                    (pointerAsDriven->msbFirst && (access.index > pointerAsDriven->from || access.index < pointerAsDriven->to))
                    || (!pointerAsDriven->msbFirst && (access.index < pointerAsDriven->from || access.index > pointerAsDriven->to))
                    ) {
                        throw "symbol.outOfRangeAccess";
                    }

                    if (std::next(j) != accesses.end()) {
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

void Space::moduleMetadata(void* array) {
    auto& json = *static_cast<nlohmann::json*>(array);
    auto current = nlohmann::json::object();
    current["subs"] = nlohmann::json::object();
    for (auto& element: space) {
        if (auto sp = std::dynamic_pointer_cast<Space>(element.second)) {
            sp->moduleMetadata(&current["subs"]);
        }
    }
    json[id] = current;
}

void SpaceWithPorts::moduleMetadata(void* array) {
    auto& json = *static_cast<nlohmann::json*>(array);
    auto current = nlohmann::json::object();
    current["ports"] = nlohmann::json::object();
    for (auto& port: ports) {
        auto object = nlohmann::json::object();
        object["width"] = port->getWidth();
        auto pol = port->getPolarity();
        object["polarity"] = pol == PortObject::Polarity::input ? "input" : pol == PortObject::Polarity::output ? "output" : "other";
        current["ports"][port->getName()] = object;
    }
    json[id] = current;
}

std::string SymbolTable::moduleMetadata() {
    nlohmann::json json = nlohmann::json::object();
    for (auto& element: head->space) {
        if (auto sp = std::dynamic_pointer_cast<Space>(element.second)) {
            sp->moduleMetadata(&json);
        }
    }
    return json.dump();
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
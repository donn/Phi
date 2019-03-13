#ifndef _symboltable_h
#define _symboltable_h

// CPP STL
#include <map>
#include <vector>
#include <memory>

// Project Headers
#include "Types.h"
#include "Context.h"
#include "Utils.h"

namespace Phi {
    enum class SymbolType {
        invalid = 0,
        nspace,
        module,
        variable
    };

    struct Symbol {
        std::string id;
        SymbolType type;

        Symbol(std::string id, SymbolType type): id(id), type(type) {}
        Symbol(const Symbol& copyable): id(copyable.id), type(copyable.type) {} // Copy Constructor

        virtual ~Symbol() = default;
    };

    struct SymbolSpace: public Symbol {
        std::map< std::string, std::shared_ptr<Symbol> > space;

        SymbolSpace(std::string id, SymbolType type): Symbol(id, type) {
            space = std::map<std::string, std::shared_ptr<Symbol> >();
        }
        SymbolSpace(const SymbolSpace& copyable): Symbol(copyable.id, copyable.type) { // Copy Constructor
            space = copyable.space;
        }
    };

    class SymbolTable {
        std::shared_ptr<SymbolSpace> head = nullptr;
        std::vector< std::shared_ptr<SymbolSpace> > stack;

    public: 
        SymbolTable();
        ~SymbolTable();


        void add(std::string id, SymbolType type, bool space = false);
        void checkExistence(std::vector<std::string> ids, SymbolType type);
        void nestInto(std::string id);
        void nestIntoAndCreate(std::string id, SymbolType type);
        void nestOut();
    };
};

#endif
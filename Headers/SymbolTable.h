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
#include "Node.h"

namespace Phi {
    struct Symbol {
        std::string id;
        Node::Node* attached;
        Node::Node* driver = nullptr;

        Symbol(std::string id, Node::Node* attached, Node::Node* driver = nullptr): id(id), attached(attached), driver(nullptr) {}
        void drive(Node::Node* newDriver) {
            if (driver) {
                throw driver;
            }
            driver = newDriver;
        }

        virtual ~Symbol() = default;
    };

    struct SymbolSpace: public Symbol {
        std::map< std::string, std::shared_ptr<Symbol> > space;
        bool isComb;

        SymbolSpace(std::string id, Node::Node* attached, bool isComb = false): Symbol(id, attached), isComb(isComb) {
            space = std::map<std::string, std::shared_ptr<Symbol> >();
        }
#if YYDEBUG
        int represent(std::ostream* stream, int* node);
#endif
    };

    class SymbolTable {
        std::shared_ptr<SymbolSpace> head = nullptr;
        std::vector< std::shared_ptr<SymbolSpace> > stack;

    public: 
        SymbolTable();
        ~SymbolTable();

        void add(std::string id, Node::Node* attached, bool space = false, Node::Node* driver = nullptr);
        void add(std::string id, Node::Node* attached, Node::Node* driver) {
            add(id, attached, false, driver);
        }
        std::shared_ptr<Symbol> checkExistence(std::vector<std::string> ids);
        void stepInto(std::string id);
        void stepIntoComb(Node::Node* attached);
        void stepIntoAndCreate(std::string id, Node::Node* attached);
        void stepOut();

        bool inComb();

#if YYDEBUG
        void represent(std::ostream* stream);
#endif
    };
};

#endif
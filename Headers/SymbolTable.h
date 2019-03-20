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

        Symbol(std::string id, Node::Node* attached): id(id), attached(attached) {}

        virtual ~Symbol() = default;
    };

    struct SymbolSpace: public Symbol {
        std::map< std::string, std::shared_ptr<Symbol> > space;
        bool isComb;

        SymbolSpace(std::string id, Node::Node* attached, bool isComb = false): Symbol(id, attached), isComb(isComb) {
            space = std::map<std::string, std::shared_ptr<Symbol> >();
        }

        void represent(std::ostream* stream, int nesting = 1);
    };

    class SymbolTable {
        std::shared_ptr<SymbolSpace> head = nullptr;
        std::vector< std::shared_ptr<SymbolSpace> > stack;

    public: 
        SymbolTable();
        ~SymbolTable();


        void add(std::string id, Node::Node* attached, bool space = false);
        std::shared_ptr<Symbol>  checkExistence(std::vector<std::string> ids, Node::Node* attached);
        void stepInto(std::string id);
        void stepIntoComb(Node::Node* attached);
        void stepIntoAndCreate(std::string id, Node::Node* attached);
        void stepOut();

        bool inComb();

        void represent(std::ostream* stream);
    };
};

#endif
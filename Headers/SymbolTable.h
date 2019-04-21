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
        Node::Node* declarator;

        Symbol(std::string id, Node::Node* declarator): id(id), declarator(declarator) {}
        virtual ~Symbol() = default;
    };

    struct DriveRange {
        Node::Expression* expression;

        // Relative to expression being driven
        Node::Expression::Width from;
        Node::Expression::Width to;

        DriveRange(Node::Expression* expression, Node::Expression::Width from, Node::Expression::Width to): expression(expression), from(from), to(to) {}
    };

    struct Driven: public Symbol {
        std::vector<DriveRange> driveRanges;

        Driven(std::string id, Node::Node* declarator): Symbol(id, declarator) {}

        bool drive(Node::Expression* expression, optional<Node::Expression::Width> from = nullopt, optional<Node::Expression::Width> to = nullopt);
    };

    struct SymbolSpace: public Symbol {
        std::map< std::string, std::shared_ptr<Symbol> > space;
        bool isComb;

        SymbolSpace(std::string id, Node::Node* declarator, bool isComb = false): Symbol(id, declarator), isComb(isComb) {}
#if YYDEBUG
        int represent(std::ostream* stream, int* node);
#endif
    };

    struct SymbolArray: public Symbol {
        std::vector <std::shared_ptr<Symbol> > array;

        SymbolArray(std::string id, Node::Node* declarator, Node::Expression::Width size = 1): Symbol(id, declarator) {}
#if YYDEBUG
        //int represent(std::ostream* stream, int* node);
#endif
    };

    class SymbolTable {
        std::shared_ptr<SymbolSpace> head = nullptr;
        std::vector< std::shared_ptr<SymbolSpace> > stack;

    public:
        SymbolTable();
        ~SymbolTable();

        void add(std::string id, std::shared_ptr<Symbol> symbol);
        optional< std::shared_ptr<Symbol> > find(Node::LHExpression* findable);
        void stepInto(std::string id);
        void stepIntoComb(Node::Node* attached);
        void stepIntoAndCreate(std::string id, Node::Node* declarator);
        void stepOut();

        bool inComb();

#if YYDEBUG
        void represent(std::ostream* stream);
#endif
    };
};

#endif
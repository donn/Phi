#ifndef _symboltable_h
#define _symboltable_h
#include "Types.h"
#include "Utils.h"

#include <map>
#include <vector>
#include <memory>
#include <set>

namespace Phi {
    namespace Node {
        struct Node;
        struct Expression;
        struct LHExpression;
        struct Range;
    }

    struct Symbol {
        std::string id;
        Node::Node* declarator;

        Symbol(std::string id, Node::Node* declarator): id(id), declarator(declarator) {}
        virtual ~Symbol() = default;
    };

    struct DriveRange {
        Node::Expression* expression;

        // Relative to expression being driven
        AccessWidth from;
        AccessWidth to;

        DriveRange(Node::Expression* expression, AccessWidth from, AccessWidth to): expression(expression), from(from), to(to) {}

        bool operator<(const DriveRange& rhs) const {
            if (to > from) {
                return from < rhs.from;
            } else {
                return from > rhs.from;
            }
        }
    };

    struct Driven: public Symbol {
        std::multiset<DriveRange> driveRanges;

        AccessWidth from;
        AccessWidth to;

        bool msbFirst; // e.g. true: [31..0], false: [0..31]

        Driven(std::string id, Node::Node* declarator, AccessWidth from = 1, AccessWidth to = 1, bool msbFirst = true): Symbol(id, declarator), from(from), to(to), msbFirst(msbFirst) {}

        bool drive(Node::Expression* expression, optional<AccessWidth> from = nullopt, optional<AccessWidth> to = nullopt);
        bool checkRangeCoverage(AccessWidth from, AccessWidth to);
        bool checkRangeCoverage(AccessWidth unit);
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

        SymbolArray(std::string id, Node::Node* declarator, AccessWidth size = 1): Symbol(id, declarator) {}
#if YYDEBUG
        //int represent(std::ostream* stream, int* node);
#endif
    };

    class SymbolTable {
        std::shared_ptr<SymbolSpace> head = nullptr;
        std::vector< std::shared_ptr<SymbolSpace> > stack;

    public:
        struct Access {
            enum class Type {
                id = 0,
                range,
                index
            };
            Type type;
            std::string* id;

            AccessWidth index;
            bool* trueIndex;

            inline static Access ID(std::string* id) { return {Type::id, id, 0, nullptr}; }
            inline static Access Index(AccessWidth access, bool* trueIndex) { return {Type::index, nullptr, access, trueIndex}; }
        };
        SymbolTable();
        ~SymbolTable();

        void add(std::string id, std::shared_ptr<Symbol> symbol);
        optional< std::shared_ptr<Symbol> > find(std::vector<Access>* accesses);
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
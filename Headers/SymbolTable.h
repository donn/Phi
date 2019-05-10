#ifndef _symboltable_h
#define _symboltable_h
#include "Types.h"
#include "Utils.h"

#include <map>
#include <vector>
#include <memory>
#include <set>
#include <functional>

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

    struct Argument {
        typedef std::pair<llvm::APInt, AccessWidth> FunctionValue;
        typedef std::vector<Argument> List;
        enum class Type {
            string = 0,
            expression
        };

        Type type;
        optional<std::string> string;
        optional<FunctionValue> expression; // pair<value, numBits>
    };

    struct Function: public Symbol {
    private:
        std::function<Argument::FunctionValue(Argument::List*)> behavior;
    public:
        std::vector<Argument::Type> parameterList;

        Argument::FunctionValue call(Argument::List* list);

        Function(std::string id,
                std::vector<Argument::Type> parameterList,
                std::function<Argument::FunctionValue(Argument::List*)> behavior
        ): Symbol(id, nullptr), behavior(behavior), parameterList(parameterList) {}
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

        Driven(std::string id, Node::Node* declarator, AccessWidth from = 0, AccessWidth to = 0, bool msbFirst = true): Symbol(id, declarator), from(from), to(to), msbFirst(msbFirst) {}

        bool drive(Node::Expression* expression, optional<AccessWidth> from = nullopt, optional<AccessWidth> to = nullopt);

        std::vector<DriveRange> checkRangeCoverage(AccessWidth from, AccessWidth to);
        optional<DriveRange> checkRangeCoverage(AccessWidth unit);
    };

    struct SymbolSpace: public Symbol {
        std::map< std::string, std::shared_ptr<Symbol> > space;
        bool isComb;

        SymbolSpace(std::string id, Node::Node* declarator, bool isComb = false): Symbol(id, declarator), isComb(isComb) {}
#if YYDEBUG
        int represent(std::ostream* stream, int* node);
#endif
    };

    struct Container: public SymbolSpace, public Driven {
        Container(std::string id, Node::Node* declarator, AccessWidth from = 0, AccessWidth to = 0, bool msbFirst = true): SymbolSpace(id, declarator, false), Driven(id, declarator, from, to, msbFirst) {} 
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
        optional< std::shared_ptr<Symbol> > find(std::vector<Access>* accesses, optional<AccessWidth>* from, optional<AccessWidth>* to);
        void stepInto(std::string id);
        void stepIntoComb(Node::Node* attached);
        void stepIntoAndCreate(std::string id, Node::Node* declarator);
        void stepOut();

        bool inComb();

#if YYDEBUG
        void represent(std::ostream* stream);
#endif
    };
}

#endif
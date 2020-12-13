#ifndef _symboltable_h
#define _symboltable_h
#include "Types.h"
#include "Utils.h"

#include <map>
#include <vector>
#include <memory>
#include <set>
#include <functional>
#include <sstream>

namespace Phi {
    namespace Node {
        struct Node;
        struct Expression;
        struct LHExpression;
        struct Range;
    }

    struct Symbol {
        std::string id;
        std::shared_ptr<Node::Node> declarator;

        Symbol(std::string id, std::shared_ptr<Node::Node> declarator): id(id), declarator(declarator) {}
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
        std::shared_ptr<Node::Expression> expression;

        // Relative to expression being driven
        AccessWidth from;
        AccessWidth to;

        DriveRange(std::shared_ptr<Node::Expression> expression, AccessWidth from, AccessWidth to): expression(expression), from(from), to(to) {}

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

        Driven(std::string id, std::shared_ptr<Node::Node> declarator, AccessWidth from = 0, AccessWidth to = 0, bool msbFirst = true): Symbol(id, declarator), from(from), to(to), msbFirst(msbFirst) {}

        std::vector<DriveRange> checkRangeCoverage(AccessWidth from, AccessWidth to);
        optional<DriveRange> checkRangeCoverage(AccessWidth unit);
        bool drive(std::shared_ptr<Node::Expression> expression, optional<AccessWidth> from = nullopt, optional<AccessWidth> to = nullopt, bool dry = false);
    };

    struct Space: public Symbol {
        enum class Type {
            // Port-havers
            module = 0,
            interface = 1,

            // Blocks
            comb = 10,
            other
        };

        Type type;
        std::map< std::string, std::shared_ptr<Symbol> > space;

        Space(std::string id, std::shared_ptr<Node::Node> declarator, Type type = Type::other): Symbol(id, declarator), type(type) {}
#if YYDEBUG
        int represent(std::ostream* stream, int* node);
#endif

        virtual void moduleMetadata(std::stringstream* jsonObject);
    };

    struct PortObject { // Abstract
        enum class Polarity {
            input = 0,
            output
        };

        virtual Polarity getPolarity() = 0;
        virtual std::string getName() = 0;
        virtual AccessWidth getWidth() = 0;
    };

    struct SpaceWithPorts: public Space {
        SpaceWithPorts(std::string id, std::shared_ptr<Node::Node> declarator, Type type = Type::module): Space(id, declarator, type) {}

        std::vector< std::shared_ptr<PortObject> > ports = {};
        std::map< std::string, std::shared_ptr<Node::Node> > annotations;

        virtual void moduleMetadata(std::stringstream* jsonObject);
    };

    struct Container: public Space, public Driven {
        Container(std::string id, std::shared_ptr<Node::Node> declarator, AccessWidth from = 0, AccessWidth to = 0, bool msbFirst = true): Space(id, declarator), Driven(id, declarator, from, to, msbFirst) {} 
    };

    struct SymbolArray: public Space {
        AccessWidth size;

        SymbolArray(std::string id, std::shared_ptr<Node::Node> declarator, AccessWidth size = 1): Space(id, declarator), size(size) {}
#if YYDEBUG
        //int represent(std::ostream* stream, int* node);
#endif
    };

    class SymbolTable {
        std::shared_ptr<Space> head = nullptr;
        std::vector< std::shared_ptr<Space> > stack;

    public:
        struct Access {
            enum class Type {
                id = 0,
                range,
                index
            };
            Type type;
            std::string id;

            AccessWidth index;
            bool* trueIndex;

            inline static Access ID(std::string id) { return {Type::id, id, 0, nullptr}; }
            inline static Access Index(AccessWidth access, bool* trueIndex) { return {Type::index, "", access, trueIndex}; }
#if YYDEBUG
            static void representList(std::ostream* ostream, std::vector<Access>* list);
#endif
        };
        SymbolTable();
        ~SymbolTable();

        std::tuple< optional< std::shared_ptr<Symbol> >, optional<AccessWidth>, optional<AccessWidth> > find(std::vector<Access>* accesses, optional<AccessWidth> from = nullopt, optional<AccessWidth> to = nullopt);
        void add(std::string id, std::shared_ptr<Symbol> symbol);
        void stepInto(std::string id);
        void stepIntoComb(std::shared_ptr<Node::Node> attached);
        std::shared_ptr<Space> stepIntoAndCreate(std::string id, std::shared_ptr<Node::Node> declarator, Space::Type type = Space::Type::other);
        void stepOut();
        

        std::vector<std::string> whereAmI(optional<Space::Type> until=nullopt);
        std::shared_ptr<Space> findNearest(Space::Type type);
        std::string moduleMetadata();

#if YYDEBUG
        void represent(std::ostream* stream);
#endif
    };
}

#endif
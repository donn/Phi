#include "Node.h"
#include <iostream>
#include <string>
#include <fstream>

using namespace Phi::Node;

static std::string adjustNamespace(std::string soFar, std::string adding) {
    return soFar == "" ? adding : soFar + "." + adding;
}

// NOTE: This translation does NOT GO RIGHT! EVER!!
void Port::MACRO_TRANS_SIG_IMP {
    switch (polarity) {
        case PortObject::Polarity::input:
            *stream << "input";
            break;
        case PortObject::Polarity::output:
            *stream << "output";
            if (assignedInComb) {
                *stream << " reg";
            }
            break;
    }
    
    // bus --> bus points to null ? no range : range [from:to] ;
    tryTranslate(bus, stream, namespaceSoFar, indent);

    // identifier 
    *stream << " ";
    tryTranslate(identifier, stream, namespaceSoFar, indent);

    *stream << ";";
    *stream << MACRO_EOL;
}

void TopLevelNamespace::MACRO_TRANS_SIG_IMP {
    //adjust namespaceSoFar
    namespaceSoFar = adjustNamespace(namespaceSoFar, *identifier);
    tryTranslate(contents, stream, namespaceSoFar, indent);
}

void TopLevelDeclaration::MACRO_TRANS_SIG_IMP {
    
    if (type == TopLevelDeclaration::Type::module) {
        
        *stream << "module ";
        tryTranslate(identifier, stream, namespaceSoFar, indent);
        //adjust namespaceSoFar after entering the module to be nothing 
        namespaceSoFar = "";
        *stream << " (";

        MACRO_INDENT;
            // PII
            for (auto& abstractPort: space.lock()->ports) {
                auto actualPort = std::static_pointer_cast<Port>(abstractPort);
                tryTranslate(actualPort->identifier, stream, namespaceSoFar, indent);
                *stream << ", " << MACRO_EOL;
            }
        MACRO_DEDENT;

        *stream << ");" << MACRO_EOL;

        MACRO_INDENT;
            *stream << MACRO_EOL;

            // Get ready for ports
            for (auto& abstractPort: space.lock()->ports) {
                auto actualPort = std::static_pointer_cast<Port>(abstractPort);
                actualPort->translate(stream, namespaceSoFar, indent);
            }
            *stream << MACRO_EOL;

            // Contents
            tryTranslate(preambles, stream, namespaceSoFar, indent);
            *stream << MACRO_EOL;

            // Contents
            tryTranslate(contents, stream, namespaceSoFar, indent);
            *stream << MACRO_EOL;

            // Addenda
            tryTranslate(addenda, stream, namespaceSoFar, indent);

        MACRO_DEDENT;
        *stream << "endmodule" << MACRO_EOL;
        *stream << MACRO_EOL << MACRO_EOL;

    } else if (type == TopLevelDeclaration::Type::interface){
        // Interfaces are elaboration-only
    }
}

void VariableLengthDeclaration::MACRO_TRANS_SIG_IMP {
    tryTranslate(declarationList, stream, namespaceSoFar, indent);
}

void DeclarationListItem::MACRO_TRANS_SIG_IMP {
    using VLD = VariableLengthDeclaration;
    
    auto nsfLocal = adjustNamespace(namespaceSoFar, *identifier);
    for (auto i = 0; i < size; i += 1) {
        auto nsfIndexed = array ? adjustNamespace(nsfLocal, std::to_string(i)) : nsfLocal;

        if (type == VLD::Type::wire && assignedInComb) {
            *stream << "reg ";
        } else if (type != VLD::Type::var) {
            *stream << "wire ";
        } else {
            // Var is immutable and is handled at compile time.
            return;
        }

        if (bus.has_value()) {
            tryTranslate(bus.value().lock(), stream, namespaceSoFar, indent);
        }

        auto finalAccessor = accessor;
        if (array) {
            // Oh dear god. What the hell is this.
            finalAccessor = std::make_shared<LHExpressionEncapsulator>(
                accessor->location,
                std::make_shared<PropertyAccess>(
                    accessor->location,
                    accessor,
                    std::make_shared<IdentifierExpression>(
                        accessor->location,
                        std::make_shared<Identifier>(
                            accessor->location,
                            std::to_string(i)
                        )
                    )
                )
            );
        }

        tryTranslate(finalAccessor, stream, namespaceSoFar, indent);
        // *stream << ("\\" + nsfIndexed + " ") << MACRO_EOL;

        //add wires,regs, always @ block, inside always @ block
        if (type == VLD::Type::reg || type == VLD::Type::latch) {
            *stream << ";" << MACRO_EOL;
            *stream << MACRO_EOL;

            auto verilogPrimitive = type == VLD::Type::reg ? "\\Phi.Common.Register " : "\\Phi.Common.Latch";

            *stream << verilogPrimitive << " ";
            if (bus.has_value()) {
                auto busStrong = bus.value().lock();
                if (busStrong) {
                    *stream << "#(.from(";
                    tryTranslate(busStrong->from, stream, namespaceSoFar, indent);
                    *stream << "), .to(";
                    tryTranslate(busStrong->to, stream, namespaceSoFar, indent);
                    *stream << ")) ";
                }
            }
            *stream <<  ("\\" + nsfIndexed + ".module ");
            
            *stream << "(";
            MACRO_INDENT;
                *stream << ".data(" << ("\\" + nsfIndexed + ".data ") << ")," << MACRO_EOL;
                *stream << ".out(" << ("\\" + nsfIndexed + " ") << ")," << MACRO_EOL;
                if (type == VLD::Type::reg) {
                    *stream << ".resetValue("; tryTranslate(optionalAssignment, stream, nsfIndexed, indent); *stream << "), " << MACRO_EOL;
                    *stream << ".reset(" << ("\\" + nsfIndexed + ".reset ") << ")," << MACRO_EOL;
                    *stream << ".clock(" << ("\\" + nsfIndexed + ".clock ") << ")," << MACRO_EOL;
                } else {
                    *stream << ".condition(" << ("\\" + nsfIndexed + ".condition ") << ")," << MACRO_EOL; 
                }
                *stream << ".enable(" << (hasEnable ? "\\" + nsfIndexed + ".enable " : "1'b1") << ")";
            MACRO_DEDENT;
            *stream << "); " << MACRO_EOL << MACRO_EOL;
            
        } else {
            if (optionalAssignment) {
                *stream << " ";
                *stream << "=";
                *stream << " ";
                *stream << "(";
                tryTranslate(optionalAssignment, stream, namespaceSoFar, indent);
                *stream << ")";
            }

            *stream << ";" << MACRO_EOL;
        }
    }
}

void InstanceDeclaration::MACRO_TRANS_SIG_IMP {

    //example
    // flipflop #(.max(4000), .width(18)) id(.b(b), ...);
    // flipflop:module, .max(4000), .width(18), ..:parameters, 
    // id:identifier, .b(b), ...:ports

    // struct InstanceDeclaration: public Declaration {
    //         Expression* module;
    //         ExpressionIDPair* parameters;

    //         Expression* array; 
    //         ExpressionIDPair* ports;
    // }

    *stream << MACRO_EOL;

    tryTranslate(module, stream, namespaceSoFar, indent);
    if (parameters) {
        *stream << " #(";
        tryTranslate(parameters, stream, namespaceSoFar, indent);
        *stream << ")";
    }

    *stream << " ";
    tryTranslate(identifier, stream, namespaceSoFar, indent);
    *stream << "(";

    MACRO_INDENT;
        tryTranslate(ports, stream, namespaceSoFar, indent);
    MACRO_DEDENT;

    *stream << MACRO_EOL;
    *stream << "); " << MACRO_EOL;
}

void ExpressionIDPair::MACRO_TRANS_SIG_IMP {

    //example 
    //calling another module
    // A (.b(b_in), ... ) 
    // b: Declaration::identifier, b_in: expression

    // struct ExpressionIDPair: public Declaration {
    //         Expression* expression;
    // }
    
    *stream << ".";
    tryTranslate(identifier, stream, namespaceSoFar, indent);
    *stream << "(";
    tryTranslate(expression, stream, namespaceSoFar, indent);
    *stream << ")";

    // PII
    if (next) {
        *stream << "," << MACRO_EOL;
    }
}

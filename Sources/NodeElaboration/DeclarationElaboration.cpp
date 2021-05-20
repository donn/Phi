#include "Node.h"
using namespace Phi::Node;

#include <set>
#include <stack>

std::vector<Phi::SymbolTable::Access> Declaration::immediateAccessList() {
    return {
        Phi::SymbolTable::Access::ID(*identifier)
    };
}

void Port::MACRO_ELAB_SIG_IMP {
    tryElaborate(bus, context);
}

void Port::addInCurrentContext(MACRO_ELAB_PARAMS, bool addOutputCheck) {
    static const char* acceptableAnnotationsInput[] = {
        "@clock",
        "@reset",
        "@enable",
        "@condition",
        0
    };
    static const char* acceptableAnnotationsOutput[] = {
        0
    };
    optional<AccessWidth> from = nullopt;
    optional<AccessWidth> to = nullopt;
    bool msbFirst = true;

    if (bus) {
        if (bus->from->type == Expression::Type::error) {
            return;
        }
        from = bus->from->value.value().getLimitedValue();
        to = bus->to->value.value().getLimitedValue();

        msbFirst = from > to;
    } else {
        from = to = 0;
    }

    auto pointer = std::make_shared<Driven>(*identifier, shared_from_this(), from.value(), to.value(), msbFirst);

    context->table->add(*identifier, pointer); // This will fail and go up in case of redefinition, so its better to do it earlier than possible.

    // Add check if output
    if (addOutputCheck && polarity ==PortObject::Polarity::output) {
        auto check = Context::DriveCheck(pointer, nullopt, nullopt, [=](){
            context->addError(location, "module.outputUndriven");
        });
        context->checks.push_back(check);
    }

            
    // Do annotations
    if (annotation.has_value()) {
        auto annotationUnwrapped = annotation.value();
        // Check if annotation is valid
        auto seeker = (polarity == PortObject::Polarity::input) ? acceptableAnnotationsInput : acceptableAnnotationsOutput;
        while (*seeker && *seeker != annotationUnwrapped) {
            seeker++;
        }
        if (!*seeker) {
            context->addError(location, "port.unknownOrIncompatibleAnnotation");
        } else {
            // Place annotation if possible
            auto module = std::static_pointer_cast<SpaceWithPorts>(context->table->findNearest(Space::Type::module));
            auto target = module->annotations.find(annotationUnwrapped);

            if (target != module->annotations.end()) {
                context->addError(location, "port.repeatedAnnotation");
            } else {
                module->annotations[annotationUnwrapped] = shared_from_this();
            }
        }
    }
}

bool Port::operator==(const Port& rhs) {
    return
        *identifier == *rhs.identifier &&
        polarity == rhs.polarity &&
        (bus ? bus->getValues() == rhs.bus->getValues() : true)
        ;
}
bool Port::operator!=(const Port& rhs) {
    return !(*this == rhs);
}

void TopLevelNamespace::MACRO_ELAB_SIG_IMP {
    if (*identifier == "Phi") {
        throw "namespace.reserved";
    } else {
        context->table->stepIntoAndCreate(*identifier, shared_from_this());
        tryElaborate(contents, context);
        context->table->stepOut();
    }
}

void TopLevelDeclaration::MACRO_ELAB_SIG_IMP {
    auto deducedType = declTypeMap[(int)type];
    space = std::static_pointer_cast<SpaceWithPorts>(context->table->stepIntoAndCreate(*identifier, shared_from_this(), deducedType));

    auto addOutputCheck = type == TopLevelDeclaration::Type::module;

    tryElaborate(ports, context);
    tryElaborate(inheritance, context);
    
    auto space = std::static_pointer_cast<SpaceWithPorts>(context->table->findNearest(deducedType));

    std::set<
        std::shared_ptr<Symbol>
    > symbolSet;

    std::function<
        void(std::shared_ptr<TopLevelDeclaration>, bool)
    > recursivelyProcessPorts = [&](
        std::shared_ptr<TopLevelDeclaration> tld, bool first
    ) {
        if (!first && tld->type != TopLevelDeclaration::Type::interface) {
            throw "compliance.interfacesOnly";
        }

        // PII
        auto port = tld->ports;
        while (port) {
            try {
                port->addInCurrentContext(context, addOutputCheck);
                space->ports.push_back(port);
            } catch (const char* e) {
                if (
                    std::string(e) == "symbol.redefinition"
                ) {
                    if (first) {
                        throw e;
                    }

                    // Duplicate ports *across inheritances* are allowed
                    // if the ports are identical
                    auto accessList = port->immediateAccessList();
                    auto result = context->table->find(&accessList);
                    auto symbolOptional = std::get<0>(result);
                    assert(symbolOptional.has_value());
                    auto symbol = symbolOptional.value();
                    auto portDeclarator = symbol->declarator;
                    auto conflictingPort = std::static_pointer_cast<Port>(portDeclarator);

                    if (*conflictingPort != *port) {
                        throw "compliance.conflictingPortDefinitions";
                    }
                }
            }
            port = std::static_pointer_cast<Port>(port->next);
        }

        auto currentInheritance = tld->inheritance;
        while (currentInheritance) {
            auto lhx = currentInheritance->lhx;
            auto accesses = std::get<0>(lhx->accessList());
            auto inheritedSymbolOptional = std::get<0>(context->table->find(&accesses));
            if (!inheritedSymbolOptional.has_value()) {
                throw "symbol.dne";
            }
            auto inheritedSymbol = inheritedSymbolOptional.value();
            if (symbolSet.find(inheritedSymbol) == symbolSet.end()) {
                // Multiple inheritances of the same symbol are SKIPPED.
                symbolSet.insert(inheritedSymbol);
                auto newTLD = std::static_pointer_cast<TopLevelDeclaration>(inheritedSymbol->declarator);
                recursivelyProcessPorts(newTLD, false);
            }
            currentInheritance = std::static_pointer_cast<InheritanceListItem>(currentInheritance->next);
        }

    };

    recursivelyProcessPorts(std::static_pointer_cast<TopLevelDeclaration>(shared_from_this()), true);

    tryElaborate(contents, context);

    context->table->stepOut();
}


#define MAKE_ID(identifier) std::make_shared<Identifier>(context->noLocation(), identifier.c_str())
#define MAKE_IDE(identifier) std::make_shared<IdentifierExpression>(\
    context->noLocation(),\
    MAKE_ID(identifier)\
)
#define MAKE_PA(id1, id2) std::make_shared<PropertyAccess>(\
    context->noLocation(),\
    id1, id2\
)
std::shared_ptr<LHExpression> TopLevelDeclaration::lhx(Context* context, std::string property) {
    auto accesses = context->table->whereAmI(Space::Type::module);
    // for (auto& e: accesses) {
    //     std::cerr << e << ".";
    // }
    // std::cerr << property << std::endl;

    if (accesses.size() == 0) {
        return MAKE_IDE(property);
    }
    auto rAccesses = accesses;
    std::reverse(std::begin(rAccesses), std::end(rAccesses));

    std::shared_ptr<LHExpression> current = MAKE_PA(MAKE_IDE(rAccesses[0]), MAKE_IDE(property));
    for (auto i = 1; i < rAccesses.size(); i += 1) {
        current = MAKE_PA(MAKE_IDE(rAccesses[i]), current);
    }
    return current;
}

std::shared_ptr<DeclarationListItem> TopLevelDeclaration::propertyDeclaration(Context* context, std::string container, std::string property, std::shared_ptr<Range> bus) {
    auto left = lhx(context, property);

    auto dli = std::make_shared<DeclarationListItem>(context->noLocation(), MAKE_ID(container), nullptr, nullptr);
    dli->accessor = std::make_shared<LHExpressionEncapsulator>(context->noLocation(), left);
    dli->bus = bus;
    dli->type = VariableLengthDeclaration::Type::wire;

    auto placement = &preambles;
    while (*placement != nullptr) {
        placement = (std::shared_ptr<Declaration>*)&(*placement)->next;
    }

    *placement = dli;

    return dli;
}

void TopLevelDeclaration::propertyAssignment(Context* context, std::shared_ptr<DeclarationListItem> dli, std::string rightHandSide) {
    auto left = dli->accessor;
    auto leftEncapsulated = std::make_shared<LHExpressionEncapsulator>(left->location, left);
    auto right = MAKE_IDE(rightHandSide);
    auto nda = std::make_shared<NondeclarativeAssignment>(context->noLocation(), leftEncapsulated, right);

    auto placement = &addenda;
    while (*placement != nullptr) {
        placement = (std::shared_ptr<Statement>*)&(*placement)->next;
    }

    *placement = nda;
}

void VariableLengthDeclaration::MACRO_ELAB_SIG_IMP {
    tryElaborate(bus, context);
    // PII
    declarationList->type = type;
    declarationList->bus = bus;
    tryElaborate(declarationList, context);
}

// This assistant function exists because of the number of exits this function can take.
// This is easily the most disgustingly written function in this whole codebase.
void DeclarationListItem::elaborationAssistant(MACRO_ELAB_PARAMS) {
    using VLD = VariableLengthDeclaration;

    tryElaborate(array, context);

    auto declarativeModule = std::static_pointer_cast<SpaceWithPorts>(context->table->findNearest(Space::Type::module));
    auto tld = std::static_pointer_cast<TopLevelDeclaration>(declarativeModule->declarator);

    accessor = std::make_shared<LHExpressionEncapsulator>(
        identifier->location,
        TopLevelDeclaration::lhx(
            context,
            *identifier
        )
    );

    assert(declarativeModule);

    std::shared_ptr<Symbol> symbol = nullptr;

    if (array) {
        if (optionalAssignment) {
            throw "array.inlineInitialization";
        }
        switch (array->type) {
        case Expression::Type::parameterSensitive:
            throw "phi.parametersUnsupported";
            break;
        case Expression::Type::compileTime:
            if (!Utils::apIntCheck(&array->value.value(), maxAccessWidth)) {
                throw "array.maximumExceeded";
                return; 
            }
            size = array->value.value().getLimitedValue();
            if (size == 0) {
                throw "array.cannotBeZero";
            }
            break;
        case Expression::Type::runTime:
            throw "array.hardwareExpression";
            break;
        case Expression::Type::error:
            return;
        }
    }

    AccessWidth width = 1;
    AccessWidth from = 0, to = 0;
    bool msbFirst = true;

    std::shared_ptr<Range> busShared = nullptr;
    if (bus.has_value()) {
        busShared = bus.value().lock();
        tryElaborate(busShared, context);
        if (busShared) {
            if (busShared->from->type == Expression::Type::error) {
                return;
            }
            from = busShared->from->value.value().getLimitedValue();
            to = busShared->to->value.value().getLimitedValue();

            msbFirst = from > to;

            width = (msbFirst ? (from - to) : (from + to)) + 1;
        }
    }

    tryElaborate(optionalAssignment, context);

    if (array) {
        context->table->stepIntoAndCreate(*identifier, shared_from_this(), Space::Type::array);
    }
    for (AccessWidth it = 0; it < size; it += 1) {
        auto id = array ? std::to_string(it) : *identifier;
        if (type == VLD::Type::reg || type == VLD::Type::latch) {
            auto container = std::make_shared<Container>(id, shared_from_this(), from, to, msbFirst);
            context->table->add(id, std::static_pointer_cast<Space>(container));
            context->table->stepInto(id);

            auto declareProperty = [&](
                std::string name,
                std::string annotationStr = "",
                bool selfAsDLI = false,
                bool fullWidth = false,
                bool mandatory = true,
                optional< std::function<void()> > ifDriven = nullopt
            ) {
                auto dli = selfAsDLI ?
                    std::static_pointer_cast<DeclarationListItem>(shared_from_this()) :
                    tld->propertyDeclaration(context, id, name, fullWidth ? busShared : nullptr)
                    ;
                auto driven = fullWidth ?
                    std::make_shared<Driven>(name, dli, from, to, msbFirst) :
                    std::make_shared<Driven>(name, dli)
                    ;

                auto annotation = declarativeModule->annotations.find(annotationStr);

                container->space[name] = driven;

                context->checks.push_back(Context::DriveCheck(driven, nullopt, nullopt, [=]() {
                    if (annotation != declarativeModule->annotations.end()) {
                        auto port = std::static_pointer_cast<Port>(annotation->second);
                        tld->propertyAssignment(context, dli, *port->identifier);
                        if (ifDriven.has_value()) {
                            ifDriven.value()();
                        }
                    } else {
                        if (mandatory) {
                            context->addError(location, "container." + name + "Undriven");
                        }
                    }
                }, ifDriven));
                return driven;
            };

            if (type == VLD::Type::reg) {
                // Register properties
                declareProperty("clock", "@clock");
                declareProperty("reset", "@reset");

                auto resetValueDriven = declareProperty("resetValue", "", true, true);
                if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                    resetValueDriven->drive(optionalAssignment);
                }
            } else {
                // Latch properties
                declareProperty("condition", "@condition");

                if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                    context->addError(location, "driving.latchNoReset");
                }
            }

            // Common properties
            declareProperty("data", "", false, true);
            declareProperty("enable", "@enable", false, false, false, [=]() {
                this->hasEnable = true;
            });

            context->table->stepOut();
        } else {
            auto driven = std::make_shared<Driven>(id, shared_from_this(), from, to, msbFirst);
            if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                driven->drive(optionalAssignment);
            }
            context->table->add(id, std::static_pointer_cast<Symbol>(driven));
        }
        
        if (optionalAssignment) { // Already reported error if it's an array
            if (optionalAssignment->type != Expression::Type::error) {
                if (width != optionalAssignment->numBits) {
                    context->addError(location, "expr.widthMismatch.driving");
                }
                if (type == VariableLengthDeclaration::Type::var && optionalAssignment->type != Expression::Type::compileTime) {
                    context->addError(location, "expr.widthMismatch.driving");
                }
            }
        } 
    }
    if (array) {
        context->table->stepOut();
    }
    std::shared_ptr<Space> comb;
    if ((comb = context->table->findNearest(Space::Type::comb))) {
        throw "comb.declarationNotAllowed";
    }
}

void DeclarationListItem::MACRO_ELAB_SIG_IMP {
    elaborationAssistant(context);

    // PII
    if (next) {
        auto nextDLI = std::static_pointer_cast<DeclarationListItem>(next);
        nextDLI->type = type;
        nextDLI->bus = bus;
    }
}

void InstanceDeclaration::MACRO_ELAB_SIG_IMP {
    if (array) {
        tryElaborate(array, context);
        if (array->type == Expression::Type::parameterSensitive) {
            // TODO
            // Might be a good idea to use SystemVerilog assert
            throw "phi.parametersUnsupported";
        } else if (array->type == Expression::Type::runTime) {
            throw "elaboration.softwareExpr";
        } else {
            throw "phi.arraysUnsupported";
        }
    }
    tryElaborate(module, context);
    tryElaborate(parameters, context);

    auto accesses = std::get<0>(module->accessList());
    auto symbolOptional = std::get<0>(context->table->find(&accesses));

    if (!symbolOptional.has_value()) {
        throw "symbol.dne";
    }
    auto symbol = symbolOptional.value();
    auto symSpace = std::dynamic_pointer_cast<Space>(symbol);
    if (!symSpace || symSpace->type != Space::Type::module) {
        throw "symbol.notAModule";
    }
    if (context->table->findNearest(Space::Type::module) == symSpace) {
        throw "module.recursion";
    }
    if (context->table->findNearest(Space::Type::comb)) {
        throw "comb.declarationNotAllowed";
    }

    context->table->add(*identifier, std::make_shared<Symbol>(*identifier, shared_from_this()));

    this->symSpace = std::static_pointer_cast<SpaceWithPorts>(symSpace);

    if (ports) {
        elaboratePorts(context);
    }
}

void InstanceDeclaration::elaboratePorts(Context* context) {
    if (!symSpace) { return; }
    tryElaborate(ports, context);
    typedef std::map<std::string, std::pair<std::shared_ptr<PortObject>, bool> > ListType;
    // port checking
    ListType inputs, outputs;
    assert(symSpace.has_value());
    auto symSpaceShared = symSpace.value().lock();
    for (auto& port: symSpaceShared->ports) {
        if (port->getPolarity() == PortObject::Polarity::input) {
            inputs[port->getName()] = std::pair(port, false);
        } else {
            outputs[port->getName()] = std::pair(port, false);
        }
    }

    // PII
    auto seeker = ports;
    while (seeker) {
        auto name = *seeker->identifier;
        auto relevantExpr = seeker->expression;
        auto inputIterator = inputs.find(name);
        auto outputIterator = outputs.find(name);
        if (inputIterator != inputs.end()) {
            // Process as input
            if (inputIterator->second.second) {
                context->addError(location, "module.portAlreadyDriven");
            } else {
                auto relevantPort = inputIterator->second.first;
                auto width = relevantPort->getWidth();

                if (width != relevantExpr->numBits) {
                    context->addError(location, "expr.widthMismatch.driving");
                } else {
                    inputIterator->second.second = true;
                }
            }
        } else if (outputIterator != outputs.end()) {
            if (outputIterator->second.second) {
                context->addError(location, "module.portAlreadyDriven");
            } else {
                // This should be a simple LHExpressionEncapsulator
                // but the fact is an evaluator is used because
                // that's what the expression production yields.
                if (auto lhxev = std::dynamic_pointer_cast<LHExpressionEvaluator>(relevantExpr)) {
                    auto lhxe = lhxev->lhxe;
                    auto relevantPort = outputIterator->second.first;

                    auto width = relevantPort->getWidth();
                    bool trash = false;
                    try {
                        // Fake dA to prevent reassignment.
                        NondeclarativeAssignment::drivingAssignment(context, lhxe, Expression::abstract(Expression::Type::runTime, width), &trash, &trash);
                        outputIterator->second.second = true;
                    } catch (const char* e) {
                        context->addError(location, e);
                    }
                    
                } else { 
                    context->addError(location, "module.outputConnectedToRHE");
                }
            }
        } else {
            context->addError(location, "module.portNotFound");
        }
        seeker = std::static_pointer_cast<ExpressionIDPair>(seeker->next);
    }
}

void ExpressionIDPair::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, context);
}
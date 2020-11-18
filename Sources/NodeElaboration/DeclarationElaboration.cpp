#include "Node.h"
using namespace Phi::Node;

#include <set>
#include <stack>

std::vector<Phi::SymbolTable::Access> Declaration::immediateAccessList() {
    return {
        Phi::SymbolTable::Access::ID(&identifier->idString)
    };
}

void Port::MACRO_ELAB_SIG_IMP {
    tryElaborate(bus, context);
    tryElaborate(right, context);
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
            tryElaborate(right, context);
            return;
        }
        from = bus->from->value.value().getLimitedValue();
        to = bus->to->value.value().getLimitedValue();

        msbFirst = from > to;
    } else {
        from = to = 0;
    }

    auto pointer = std::make_shared<Driven>(identifier->idString, shared_from_this(), from.value(), to.value(), msbFirst);

    context->table->add(identifier->idString, pointer); // This will fail and go up in case of redefinition, so its better to do it earlier than possible.

    // Add check if output
    if (addOutputCheck && polarity ==PortObject::Polarity::output) {
        auto check = Context::DriveCheck(pointer, nullopt, nullopt, [=](){
            context->addError(location, "output.undriven");
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
        identifier->idString == rhs.identifier->idString &&
        polarity == rhs.polarity &&
        (bus ? bus->getValues() == rhs.bus->getValues() : true)
        ;
}
bool Port::operator!=(const Port& rhs) {
    return !(*this == rhs);
}

void TopLevelNamespace::MACRO_ELAB_SIG_IMP {
    if (identifier->idString == "Phi") {
        throw "namespace.reserved";
    } else {
        context->table->stepIntoAndCreate(identifier->idString, shared_from_this());
        tryElaborate(contents, context);
        context->table->stepOut();
    }
    tryElaborate(right, context);
}

void TopLevelDeclaration::MACRO_ELAB_SIG_IMP {
    auto deducedType = declTypeMap[(int)type];
    space = std::static_pointer_cast<SpaceWithPorts>(context->table->stepIntoAndCreate(identifier->idString, shared_from_this(), deducedType));

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
            port = std::static_pointer_cast<Port>(port->right);
        }

        auto currentInheritance = tld->inheritance;
        while (currentInheritance) {
            auto lhExpression = currentInheritance->lhExpression;
            auto accesses = std::get<0>(lhExpression->accessList());
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
            currentInheritance = std::static_pointer_cast<InheritanceListItem>(currentInheritance->right);
        }

    };

    recursivelyProcessPorts(std::static_pointer_cast<TopLevelDeclaration>(shared_from_this()), true);

    tryElaborate(contents, context);

    context->table->stepOut();
    tryElaborate(right, context);
}

std::shared_ptr<DeclarationListItem> TopLevelDeclaration::propertyDeclaration(Context* context, std::string container, std::string property, std::shared_ptr<Range> range) {
    auto containerID = std::make_shared<Identifier>(context->noLocation(), container.c_str());
    auto containerNode = std::make_shared<IdentifierExpression>(context->noLocation(), containerID);
    auto propertyNode = std::make_shared<IdentifierExpression>(context->noLocation(), std::make_shared<Identifier>(context->noLocation(), property.c_str()));
    auto left = std::make_shared<PropertyAccess>(context->noLocation(), containerNode, propertyNode);
    auto dli = std::make_shared<DeclarationListItem>(context->noLocation(), containerID, nullptr, nullptr);
    std::cout << container << "." << property << ":" << range << std::endl;
    dli->trueIdentifier = left;
    dli->bus = range;
    dli->type = VariableLengthDeclaration::Type::wire;

    auto placement = &preambles;
    while (*placement != nullptr) {
        placement = (std::shared_ptr<Declaration>*)&(*placement)->right;
    }

    *placement = dli;

    return dli;
}

void TopLevelDeclaration::propertyAssignment(Context* context, std::string container, std::string property, std::string rightHandSide) {
    auto containerNode = std::make_shared<IdentifierExpression>(context->noLocation(), std::make_shared<Identifier>(context->noLocation(), container.c_str()));
    auto propertyNode = std::make_shared<IdentifierExpression>(context->noLocation(), std::make_shared<Identifier>(context->noLocation(), property.c_str()));
    auto left = std::make_shared<PropertyAccess>(context->noLocation(), containerNode, propertyNode);
    auto right = std::make_shared<IdentifierExpression>(context->noLocation(), std::make_shared<Identifier>(context->noLocation(), rightHandSide.c_str()));
    auto nda = std::make_shared<NondeclarativeAssignment>(context->noLocation(), left, right);

    auto placement = &addenda;
    while (*placement != nullptr) {
        placement = (std::shared_ptr<Statement>*)&(*placement)->right;
    }

    *placement = nda;
}

void VariableLengthDeclaration::MACRO_ELAB_SIG_IMP {
    tryElaborate(bus, context);
    // PII
    declarationList->type = type;
    declarationList->bus = bus;
    tryElaborate(declarationList, context);
    tryElaborate(right, context);
}

// This assistant function exists because of the number of exits this function can take.
// This is easily the most disgustingly written function in this whole codebase.
void DeclarationListItem::elaborationAssistant(MACRO_ELAB_PARAMS) {
    using VLD = VariableLengthDeclaration;

    AccessWidth size = 1;
    AccessWidth width = 1;

    bool msbFirst = true;

    optional<AccessWidth> from = nullopt;
    optional<AccessWidth> to = nullopt;

    tryElaborate(array, context);

    auto declarativeModule = std::static_pointer_cast<SpaceWithPorts>(context->table->findNearest(Space::Type::module));
    auto tld = std::static_pointer_cast<TopLevelDeclaration>(declarativeModule->declarator);

    assert(declarativeModule);

    if (array) {
        if (optionalAssignment) {
            throw "array.inlineInitialization";
        }
        switch (array->type) {
        case Expression::Type::parameterSensitive:
            size = 0;
            context->addError(location, "phi.parametersUnsupported");
            return;
            break;
        case Expression::Type::compileTime:
            if (!Utils::apIntCheck(&array->value.value(), maxAccessWidth)) {
                context->addError(location, "array.maximumExceeded");
                return; 
            }
            size = array->value.value().getLimitedValue();
            if (size == 0) {
                context->addError(location, "array.cannotBeZero");
                return;
            }
            break;
        case Expression::Type::runTime:
            context->addError(location, "array.hardwareExpression");
            [[fallthrough]];
        case Expression::Type::error:
            return;
        }
    }

    from = to = 0;

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

            width = (msbFirst ? (from.value() - to.value()) : (from.value() + to.value())) + 1;
        }
    }

    tryElaborate(optionalAssignment, context);

    auto generatedSymbols = std::vector< std::pair< std::string, std::shared_ptr<Symbol> > >();
    for (AccessWidth it = 0; it < size; it += 1) {
        auto id = size == 1 ? identifier->idString : identifier->idString + "[" + std::to_string(it) + "]";
        if (type == VLD::Type::reg || type == VLD::Type::latch) {
            auto container = std::make_shared<Container>(id, shared_from_this(), from.value(), to.value(), msbFirst);

            auto declProp = [&](
                std::string name,
                std::string annotationStr = "",
                bool selfAsDLI = false,
                bool fullWidth = false,
                bool mandatory = true,
                std::optional< std::function<void()> > ifDriven = nullopt
            ) {
                auto dli = selfAsDLI ?
                    shared_from_this() :
                    tld->propertyDeclaration(context, id, name, fullWidth ? busShared : nullptr)
                    ;
                auto driven = fullWidth ?
                    std::make_shared<Driven>(name, dli, from.value(), to.value(), msbFirst) :
                    std::make_shared<Driven>(name, dli)
                    ;

                auto annotation = declarativeModule->annotations.find(annotationStr);

                container->space[name] = driven;

                context->checks.push_back(Context::DriveCheck(driven, nullopt, nullopt, [=]() {
                    if (annotation != declarativeModule->annotations.end()) {
                        auto port = std::static_pointer_cast<Port>(annotation->second);
                        tld->propertyAssignment(context, id, name, port->identifier->idString);
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
                declProp("clock", "@clock");
                declProp("reset", "@reset");

                auto resetValueDriven = declProp("resetValue", "", true, true);
                if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                    resetValueDriven->drive(optionalAssignment);
                }
            } else {
                // Latch properties
                declProp("condition", "@condition");

                if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                    context->addError(location, "driving.latchNoReset");
                }
            }

            // Common properties
            declProp("data", "", false, true);
            declProp("enable", "@enable", false, false, false, [=]() {
                this->hasEnable = true;
            });

            generatedSymbols.push_back(std::pair(std::to_string(it), std::static_pointer_cast<Symbol>(std::static_pointer_cast<Driven>(container))));
        } else {
            auto driven = std::make_shared<Driven>(id, shared_from_this(), from.value(), to.value(), msbFirst);
            if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                driven->drive(optionalAssignment);
            }
            generatedSymbols.push_back(std::pair(std::to_string(it), std::static_pointer_cast<Symbol>(driven)));
        }
        
        if (optionalAssignment) { // Already reported error if it's an array
            if (optionalAssignment->type != Expression::Type::error) {
                if (width != optionalAssignment->numBits) {
                    context->addError(location, "driving.widthMismatch");
                }
                if (type == VariableLengthDeclaration::Type::var && optionalAssignment->type != Expression::Type::compileTime) {
                    context->addError(location, "driving.hardwareDominance");
                }
            }
        } 
    }
    
    std::shared_ptr<Space> comb;
    if ((comb = context->table->findNearest(Space::Type::comb))) {
        context->addError(location, "comb.declarationNotAllowed");
        return;
    } 
    

    std::shared_ptr<Symbol> symbol = nullptr;
    if (size != 1) {
        auto array = std::make_shared<SymbolArray>(identifier->idString, shared_from_this(), size);
        for (auto& symbol: generatedSymbols) {
            array->space[symbol.first] = symbol.second;
        }
        symbol = array;
    } else {
        symbol = generatedSymbols[0].second;
    }
    context->table->add(identifier->idString, symbol);
}



void DeclarationListItem::MACRO_ELAB_SIG_IMP {
    elaborationAssistant(context);

    //PII
    if (right) {
        auto rightDLI = std::static_pointer_cast<DeclarationListItem>(right);
        rightDLI->type = type;
        rightDLI->bus = bus;
        tryElaborate(right, context);
    }
}

void InstanceDeclaration::MACRO_ELAB_SIG_IMP {

    if (array) {
        tryElaborate(array, context);
        if (array->type == Expression::Type::parameterSensitive) {
            //Translate to assert and generate
        } else if (array->type == Expression::Type::runTime) {
            context->addError(location, "elaboration.softwareExpr");
        } else {
            //unroll
        }
    }
    tryElaborate(module, context);
    tryElaborate(parameters, context);

    auto accesses = std::get<0>(module->accessList());
    auto symbolOptional = std::get<0>(context->table->find(&accesses));

    if (symbolOptional.has_value()) {
        auto symbol = symbolOptional.value();
        if (auto symSpace = std::dynamic_pointer_cast<Space>(symbol)) {
            if (symSpace->type == Space::Type::module) {
                if (context->table->findNearest(Space::Type::module) != symSpace) {
                    if (!context->table->findNearest(Space::Type::comb)) {
                        context->table->add(identifier->idString, std::make_shared<Symbol>(identifier->idString, shared_from_this()));

                        this->symSpace = std::static_pointer_cast<SpaceWithPorts>(symSpace);

                        if (ports) {
                            elaboratePorts(context);
                        }

                    } else {
                        this->symSpace = nullopt;
                        context->addError(location, "comb.declarationNotAllowed");
                    }
                } else {
                    this->symSpace = nullopt;
                    context->addError(location, "module.recursion");
                }
            } else {
                this->symSpace = nullopt;
                context->addError(location, "symbol.notAModule");
            }
        } else {
            this->symSpace = nullopt;
            context->addError(location, "symbol.notAModuleOrSpace");
        }
    } else {
        context->addError(location, "symbol.dne");
    }
    // I profusely apologize for what you just saw.
    
    tryElaborate(right, context);
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

    //PII
    auto seeker = ports;
    while (seeker) {
        auto name = seeker->identifier->idString;
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
                    context->addError(location, "driving.widthMismatch");
                } else {
                    inputIterator->second.second = true;
                }
            }
        } else if (outputIterator != outputs.end()) {
            if (outputIterator->second.second) {
                context->addError(location, "module.portAlreadyDriven");
            } else {
                if (auto lhs = std::dynamic_pointer_cast<LHExpression>(relevantExpr)) {
                    auto relevantPort = outputIterator->second.first;

                    auto width = relevantPort->getWidth();
                    bool trash = false;
                    try {
                        NondeclarativeAssignment::drivingAssignment(context, lhs, Expression::abstract(Expression::Type::runTime, width), &trash, &trash);
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
        seeker = std::static_pointer_cast<ExpressionIDPair>(seeker->right);
    }
}

void ExpressionIDPair::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, context);

    tryElaborate(right, context);
}
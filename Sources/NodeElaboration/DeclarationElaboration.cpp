#include "Node.h"
using namespace Phi::Node;

void Port::MACRO_ELAB_SIG_IMP {
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

    tryElaborate(bus, context);
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

    // Add check if output
    if (polarity ==PortObject::Polarity::output) {
        auto check = Context::DriveCheck(pointer, nullopt, nullopt, [=](){
            context->addError(location, "output.undriven");
        });
        context->checks.push_back(check);
    }
    context->table->add(identifier->idString, pointer);

            
    // Do annotations
    if (annotation.has_value()) {
        auto annotationUnwrapped = annotation.value();
        // Check if annotation is valid
        auto seeker = (polarity ==PortObject::Polarity::input) ? acceptableAnnotationsInput : acceptableAnnotationsOutput;
        while (*seeker && *seeker != annotationUnwrapped) {
            seeker++;
        }
        if (!*seeker) {
            context->addError(location, "port.unknownOrIncompatibleAnnotation");
        } else {
            // Place annotation if possible
            auto module = context->table->findNearest(SymbolSpace::Type::module);
            auto target = module->annotations.find(annotationUnwrapped);

            if (target != module->annotations.end()) {
                context->addError(location, "port.repeatedAnnotation");
            } else {
                module->annotations[annotationUnwrapped] = shared_from_this();
            }
        }
    }
    tryElaborate(right, context);
}

void TopLevelNamespace::MACRO_ELAB_SIG_IMP {
    context->table->stepIntoAndCreate(identifier->idString, shared_from_this());
    tryElaborate(contents, context);
    context->table->stepOut();
    tryElaborate(right, context);
}

void TopLevelDeclaration::MACRO_ELAB_SIG_IMP {
    auto deducedType = declTypeMap[(int)type];
    context->table->stepIntoAndCreate(identifier->idString, shared_from_this(), deducedType);
    tryElaborate(ports, context);
    tryElaborate(contents, context);
    if (deducedType == SymbolSpace::Type::module) {
        auto space = std::static_pointer_cast<Module>(context->table->findNearest(SymbolSpace::Type::module));
        for (auto& element: space->space) {
            if (auto port = std::dynamic_pointer_cast<Port>(element.second->declarator)) {
                space->ports.push_back(port);
            }
        }
    }
    context->table->stepOut();
    tryElaborate(right, context);
}

std::shared_ptr<DeclarationListItem> TopLevelDeclaration::propertyDeclaration(Context* context, std::string container, std::string property, std::shared_ptr<Range> range) {
    auto containerID = std::make_shared<Identifier>(context->noLocation(), container.c_str());
    auto containerNode = std::make_shared<IdentifierExpression>(context->noLocation(), containerID);
    auto propertyNode = std::make_shared<IdentifierExpression>(context->noLocation(), std::make_shared<Identifier>(context->noLocation(), property.c_str()));
    auto left = std::make_shared<PropertyAccess>(context->noLocation(), containerNode, propertyNode);
    auto dli = std::make_shared<DeclarationListItem>(context->noLocation(), containerID, nullptr, nullptr);
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

void DeclarationListItem::MACRO_ELAB_SIG_IMP {
    using VLD = VariableLengthDeclaration;
    // Declaration Block because goto is going to happen
    std::shared_ptr<DeclarationListItem> rightDLI;

    // I adamantly refuse to consider * part of the variable and not the type. To h*ck with society
    std::shared_ptr<DeclarationListItem> clockDLI;
    std::shared_ptr<DeclarationListItem> resetDLI;
    std::shared_ptr<DeclarationListItem> conditionDLI;
    std::shared_ptr<DeclarationListItem> dataDLI;
    std::shared_ptr<DeclarationListItem> enableDLI;
    std::shared_ptr<TopLevelDeclaration> tld;

    std::shared_ptr<Symbol> pointer;
    std::shared_ptr<Driven> pointerAsDriven, clockDriven, resetDriven, resetValueDriven, conditionDriven, dataDriven, enableDriven;
    std::shared_ptr<Container> pointerAsContainer;
    std::shared_ptr<SymbolArray> pointerAsArray;
    std::shared_ptr<SymbolSpace> declarativeModule, comb;
    std::shared_ptr<Range> busShared = nullptr;

    std::map<std::string, std::shared_ptr<Node>>::iterator clockAnnotation, resetAnnotation, conditionAnnotation, enableAnnotation;

    AccessWidth size = 1;
    AccessWidth width = 1;

    bool msbFirst = true;

    optional<AccessWidth> from = nullopt;
    optional<AccessWidth> to = nullopt;

    tryElaborate(array, context);


    declarativeModule = context->table->findNearest(SymbolSpace::Type::module);
    tld = std::static_pointer_cast<TopLevelDeclaration>(declarativeModule->declarator);

    assert(declarativeModule);

    if (array) {
        context->addError(location, "phi.arraysUnsupported"); // UNSUPPORTED

        if (optionalAssignment) {
            throw "array.inlineInitialization";
        }
        switch (array->type) {
        case Expression::Type::parameterSensitive:
            size = 0;
            break;
        case Expression::Type::compileTime:
            if (!Utils::apIntCheck(&array->value.value(), maxAccessWidth)) {
                context->addError(location, "array.maximumExceeded");
                goto exit; 
            }
            size = array->value.value().getLimitedValue();
            if (size == 0) {
                context->addError(location, "array.cannotBeZero");
                goto exit; 
            }
            break;
        case Expression::Type::runTime:
            context->addError(location, "array.hardwareExpression");
            [[fallthrough]];
        case Expression::Type::error:
            goto exit;
            break;
        }
    }

    from = to = 0;

    if (bus.has_value()) {
        busShared = bus.value().lock();
        tryElaborate(busShared, context);
        if (busShared) {
            if (busShared->from->type == Expression::Type::error) {
                goto exit;
            }
            from = busShared->from->value.value().getLimitedValue();
            to = busShared->to->value.value().getLimitedValue();

            msbFirst = from > to;

            width = (msbFirst ? (from.value() - to.value()) : (from.value() + to.value())) + 1;
        }
    }

    tryElaborate(optionalAssignment, context);


    if (size == 1) {
        if (optionalAssignment) {
            if (optionalAssignment->type != Expression::Type::error) {
                if (width != optionalAssignment->numBits) {
                    context->addError(location, "driving.widthMismatch");
                }
                if (type == VariableLengthDeclaration::Type::var && optionalAssignment->type != Expression::Type::compileTime) {
                    context->addError(location, "driving.hardwareDominance");
                }
            }
        } 
        switch (type) {
        case VLD::Type::reg:
        case VLD::Type::latch:     
            pointerAsContainer = std::make_shared<Container>(identifier->idString, shared_from_this(), from.value(), to.value(), msbFirst);   
            if (type == VLD::Type::reg) {                
                // Register properties
                clockDLI = tld->propertyDeclaration(context, identifier->idString, "clock", nullptr);
                clockDriven = std::make_shared<Driven>("clock", clockDLI);
                pointerAsContainer->space["clock"] = clockDriven;
                clockAnnotation = declarativeModule->annotations.find("@clock");
                context->checks.push_back(Context::DriveCheck(clockDriven, nullopt, nullopt, [=]() {
                    if (clockAnnotation != declarativeModule->annotations.end()) {
                        auto port = std::static_pointer_cast<Port>(clockAnnotation->second);
                        tld->propertyAssignment(context, identifier->idString, "clock", port->identifier->idString);
                    } else {
                        context->addError(location, "register.clockUndriven");
                    }
                }));

                resetDLI = tld->propertyDeclaration(context, identifier->idString, "reset", nullptr);
                resetDriven = std::make_shared<Driven>("reset", resetDLI);
                pointerAsContainer->space["reset"] = resetDriven;
                resetAnnotation = declarativeModule->annotations.find("@reset");
                context->checks.push_back(Context::DriveCheck(resetDriven, nullopt, nullopt, [=]() {
                    if (resetAnnotation != declarativeModule->annotations.end()) {
                        auto port = std::static_pointer_cast<Port>(resetAnnotation->second);
                        tld->propertyAssignment(context, identifier->idString, "reset", port->identifier->idString);
                    } else {
                        context->addError(location, "register.resetUndriven");
                    }
                }));

                auto resetValueDriven = std::make_shared<Driven>("_0R", shared_from_this(), from.value(), to.value(), msbFirst);
                if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                    resetValueDriven->drive(optionalAssignment);
                }
                pointerAsContainer->space["_0R"] = resetValueDriven;
                context->checks.push_back(Context::DriveCheck(resetValueDriven, nullopt, nullopt, [=](){
                    context->addError(location, "register.resetValueUndriven");
                }));
            } else {
                // Latch properties
                conditionDLI = tld->propertyDeclaration(context, identifier->idString, "condition", nullptr);
                conditionDriven = std::make_shared<Driven>("condition", conditionDLI);
                pointerAsContainer->space["condition"] = conditionDriven;
                conditionAnnotation = declarativeModule->annotations.find("@condition");
                context->checks.push_back(Context::DriveCheck(conditionDriven, nullopt, nullopt, [=]() {
                    if (conditionAnnotation != declarativeModule->annotations.end()) {
                        auto port = std::static_pointer_cast<Port>(conditionAnnotation->second);
                        tld->propertyAssignment(context, identifier->idString, "condition", port->identifier->idString);
                    } else {
                        context->addError(location, "latch.conditionUndriven");
                    }
                }));

                if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                    context->addError(location, "driving.latchNoReset");
                }
            }

            // Common properties
            dataDLI = tld->propertyDeclaration(context, identifier->idString, "data", busShared);
            dataDriven = std::make_shared<Driven>("data", dataDLI, from.value(), to.value(), msbFirst);
            pointerAsContainer->space["data"] = dataDriven;
            context->checks.push_back(Context::DriveCheck(dataDriven, nullopt, nullopt, [=](){
                context->addError(location, "register.dataUndriven");
            }));

            enableDLI = tld->propertyDeclaration(context, identifier->idString, "enable", nullptr);
            enableDriven = std::make_shared<Driven>("enable", enableDLI);
            pointerAsContainer->space["enable"] = enableDriven;
            enableAnnotation = declarativeModule->annotations.find("@enable");
            context->checks.push_back(Context::DriveCheck(enableDriven, nullopt, nullopt, [=]() {
                if (enableAnnotation != declarativeModule->annotations.end()) {
                    auto port = std::static_pointer_cast<Port>(enableAnnotation->second);
                    tld->propertyAssignment(context, identifier->idString, "enable", port->identifier->idString);
                }
            }, [=]() {
                this->hasEnable = true;
            }));

            pointerAsDriven = pointerAsContainer; // Believe it or not, we need this cast first anyway.
            pointer = pointerAsDriven;
            break;
        default:
            pointerAsDriven = std::make_shared<Driven>(identifier->idString, shared_from_this(), from.value(), to.value(), msbFirst);
            if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                pointerAsDriven->drive(optionalAssignment);
            }
            pointer = pointerAsDriven;
        }
    } else {
        pointerAsArray = std::make_shared<SymbolArray>(identifier->idString, shared_from_this(), size);
        for (AccessWidth i = 0; i < size; i += 1) {
            pointerAsArray->array.push_back(std::make_shared<Driven>(identifier->idString, shared_from_this(), from.value(), to.value(), msbFirst));
        }
        pointer = pointerAsArray;
    }
    if ((comb = context->table->findNearest(SymbolSpace::Type::comb))) {
        context->addError(location, "comb.declarationNotAllowed");
        goto exit;
    } else {
        context->table->add(identifier->idString, pointer);
    }

exit:
    //PII
    if (right) {
        rightDLI = std::static_pointer_cast<DeclarationListItem>(right);
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

    std::optional<AccessWidth> trash;
    auto accesses = module->accessList(&trash, &trash);
    auto symbolOptional = context->table->find(&accesses, &trash, &trash);

    if (symbolOptional.has_value()) {
        auto symbol = symbolOptional.value();
        if (auto symSpace = std::dynamic_pointer_cast<SymbolSpace>(symbol)) {
            if (symSpace->type == SymbolSpace::Type::module) {
                if (context->table->findNearest(SymbolSpace::Type::module) != symSpace) {
                    if (!context->table->findNearest(SymbolSpace::Type::comb)) {
                        context->table->add(identifier->idString, std::make_shared<Symbol>(identifier->idString, shared_from_this()));

                        this->symSpace = std::static_pointer_cast<Module>(symSpace);

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
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

    auto pointer = std::make_shared<Driven>(identifier->idString, this, from.value(), to.value(), msbFirst);

    // Add check if output
    if (polarity == Polarity::output) {
        auto check = Context::DriveCheck(pointer, nullopt, nullopt, [=](){
            context->addError(nullopt, "output.undriven");
        });
        context->checks.push_back(check);
    }
    context->table->add(identifier->idString, pointer);

            
    // Do annotations
    if (annotation.has_value()) {
        auto annotationUnwrapped = annotation.value();
        // Check if annotation is valid
        auto seeker = (polarity == Polarity::input) ? acceptableAnnotationsInput : acceptableAnnotationsOutput;
        while (*seeker && *seeker != annotationUnwrapped) {
            seeker++;
        }
        if (!*seeker) {
            context->addError(nullopt, "port.unknownOrIncompatibleAnnotation");
        } else {
            // Place annotation if possible
            auto module = context->table->findNearest(SymbolSpace::Type::module);
            auto target = module->annotations.find(annotationUnwrapped);

            if (target != module->annotations.end()) {
                context->addError(nullopt, "port.repeatedAnnotation");
            } else {
                module->annotations[annotationUnwrapped] = this;
            }
        }
    }
    tryElaborate(right, context);
}

void TopLevelNamespace::MACRO_ELAB_SIG_IMP {
    context->table->stepIntoAndCreate(identifier->idString, this);
    tryElaborate(contents, context);
    context->table->stepOut();
    tryElaborate(right, context);
}

void TopLevelDeclaration::MACRO_ELAB_SIG_IMP {
    context->table->stepIntoAndCreate(identifier->idString, this, declTypeMap[(int)type]);
    tryElaborate(ports, context);
    tryElaborate(contents, context);
    context->table->stepOut();
    tryElaborate(right, context);
}

DeclarationListItem* TopLevelDeclaration::propertyDeclaration(std::string container, std::string property, Range* range) {
    // Unsafe allocations
    auto containerID = new Identifier(container.c_str());
    IdentifierExpression* containerNode = new IdentifierExpression(containerID);
    IdentifierExpression* propertyNode = new IdentifierExpression(new Identifier(property.c_str()));
    PropertyAccess* left = new PropertyAccess(containerNode, propertyNode);
    DeclarationListItem* dli = new DeclarationListItem(containerID, nullptr, nullptr);
    dli->trueIdentifier = left;
    dli->bus = range;
    dli->type = VariableLengthDeclaration::Type::wire;

    auto placement = &preambles;
    while (*placement != nullptr) {
        placement = (Declaration**)&(*placement)->right;
    }

    *placement = dli;

    return dli;
}

void TopLevelDeclaration::propertyAssignment(std::string container, std::string property, std::string rightHandSide) {
    // Unsafe allocations
    IdentifierExpression* containerNode = new IdentifierExpression(new Identifier(container.c_str()));
    IdentifierExpression* propertyNode = new IdentifierExpression(new Identifier(property.c_str()));
    PropertyAccess* left = new PropertyAccess(containerNode, propertyNode);
    IdentifierExpression* right = new IdentifierExpression(new Identifier(rightHandSide.c_str()));
    NondeclarativeAssignment* nda = new NondeclarativeAssignment(left, right);

    auto placement = &addenda;
    while (*placement != nullptr) {
        placement = (Statement**)&(*placement)->right;
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
    DeclarationListItem* rightDLI;

    // I adamantly refuse to consider * part of the variable and not the type. To h*ck with society
    DeclarationListItem* clockDLI;
    DeclarationListItem* resetDLI;
    DeclarationListItem* conditionDLI;
    DeclarationListItem* dataDLI;
    DeclarationListItem* enableDLI;
    TopLevelDeclaration* tld;

    std::shared_ptr<Symbol> pointer;
    std::shared_ptr<Driven> pointerAsDriven, clockDriven, resetDriven, resetValueDriven, conditionDriven, dataDriven, enableDriven;
    std::shared_ptr<Container> pointerAsContainer;
    std::shared_ptr<SymbolArray> pointerAsArray;
    std::shared_ptr<SymbolSpace> declarativeModule, comb;

    std::map<std::string, Node*>::iterator clockAnnotation, resetAnnotation, conditionAnnotation, enableAnnotation;

    AccessWidth size = 1;
    AccessWidth width = 1;

    bool msbFirst = true;

    optional<AccessWidth> from = nullopt;
    optional<AccessWidth> to = nullopt;

    tryElaborate(array, context);
    LHExpression::lhDrivenProcess(array, context->table);

    declarativeModule = context->table->findNearest(SymbolSpace::Type::module);
    tld = static_cast<TopLevelDeclaration*>(declarativeModule->declarator);

    assert(declarativeModule);

    if (array) {
        context->addError(nullopt, "phi.arraysUnsupported"); // UNSUPPORTED

        if (optionalAssignment) {
            throw "array.inlineInitialization";
        }
        switch (array->type) {
        case Expression::Type::parameterSensitive:
            size = 0;
            break;
        case Expression::Type::compileTime:
            if (!Utils::apIntCheck(&array->value.value(), maxAccessWidth)) {
                context->addError(nullopt, "array.maximumExceeded");
                goto exit; 
            }
            size = array->value.value().getLimitedValue();
            if (size == 0) {
                context->addError(nullopt, "array.cannotBeZero");
                goto exit; 
            }
            break;
        case Expression::Type::runTime:
            context->addError(nullopt, "array.hardwareExpression");
            [[fallthrough]];
        case Expression::Type::error:
            goto exit;
            break;
        }
    }
    tryElaborate(bus, context);
    if (bus) {
        if (bus->from->type == Expression::Type::error) {
            goto exit;
        }
        from = bus->from->value.value().getLimitedValue();
        to = bus->to->value.value().getLimitedValue();

        msbFirst = from > to;

        width = (msbFirst ? (from.value() - to.value()) : (from.value() + to.value())) + 1;
    } else {
        from = to = 0;
    }

    tryElaborate(optionalAssignment, context);
    LHExpression::lhDrivenProcess(optionalAssignment, context->table);

    if (size == 1) {
        if (optionalAssignment) {
            if (optionalAssignment->type != Expression::Type::error) {
                if (width != optionalAssignment->numBits) {
                    context->addError(nullopt, "driving.widthMismatch");
                }
                if (type == VariableLengthDeclaration::Type::var && optionalAssignment->type != Expression::Type::compileTime) {
                    context->addError(nullopt, "driving.hardwareDominance");
                }
            }
        } 
        switch (type) {
        case VLD::Type::reg:
        case VLD::Type::latch:     
            pointerAsContainer = std::make_shared<Container>(identifier->idString, this, from.value(), to.value(), msbFirst);   
            if (type == VLD::Type::reg) {                
                // Register properties
                clockDLI = tld->propertyDeclaration(identifier->idString, "clock", nullptr);
                clockDriven = std::make_shared<Driven>("clock", clockDLI);
                pointerAsContainer->space["clock"] = clockDriven;
                clockAnnotation = declarativeModule->annotations.find("@clock");
                context->checks.push_back(Context::DriveCheck(clockDriven, nullopt, nullopt, [=]() {
                    if (clockAnnotation != declarativeModule->annotations.end()) {
                        auto port = static_cast<Port*>(clockAnnotation->second);
                        tld->propertyAssignment(identifier->idString, "clock", port->identifier->idString);
                    } else {
                        context->addError(nullopt, "register.clockUndriven");
                    }
                }));

                resetDLI = tld->propertyDeclaration(identifier->idString, "reset", nullptr);
                resetDriven = std::make_shared<Driven>("reset", resetDLI);
                pointerAsContainer->space["reset"] = resetDriven;
                resetAnnotation = declarativeModule->annotations.find("@reset");
                context->checks.push_back(Context::DriveCheck(resetDriven, nullopt, nullopt, [=]() {
                    if (resetAnnotation != declarativeModule->annotations.end()) {
                        auto port = static_cast<Port*>(resetAnnotation->second);
                        tld->propertyAssignment(identifier->idString, "reset", port->identifier->idString);
                    } else {
                        context->addError(nullopt, "register.resetUndriven");
                    }
                }));

                auto resetValueDriven = std::make_shared<Driven>("_0R", this, from.value(), to.value(), msbFirst);
                if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                    resetValueDriven->drive(optionalAssignment);
                }
                pointerAsContainer->space["_0R"] = resetValueDriven;
                context->checks.push_back(Context::DriveCheck(resetValueDriven, nullopt, nullopt, [=](){
                    context->addError(nullopt, "register.resetValueUndriven");
                }));
            } else {
                // Latch properties
                conditionDLI = tld->propertyDeclaration(identifier->idString, "condition", nullptr);
                conditionDriven = std::make_shared<Driven>("condition", conditionDLI);
                pointerAsContainer->space["condition"] = conditionDriven;
                conditionAnnotation = declarativeModule->annotations.find("@condition");
                context->checks.push_back(Context::DriveCheck(conditionDriven, nullopt, nullopt, [=]() {
                    if (conditionAnnotation != declarativeModule->annotations.end()) {
                        auto port = static_cast<Port*>(conditionAnnotation->second);
                        tld->propertyAssignment(identifier->idString, "condition", port->identifier->idString);
                    } else {
                        context->addError(nullopt, "latch.conditionUndriven");
                    }
                }));

                if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                    context->addError(nullopt, "driving.latchNoReset");
                }
            }

            // Common properties
            dataDLI = tld->propertyDeclaration(identifier->idString, "data", bus);
            dataDriven = std::make_shared<Driven>("data", dataDLI, from.value(), to.value(), msbFirst);
            pointerAsContainer->space["data"] = dataDriven;
            context->checks.push_back(Context::DriveCheck(dataDriven, nullopt, nullopt, [=](){
                context->addError(nullopt, "register.dataUndriven");
            }));

            enableDLI = tld->propertyDeclaration(identifier->idString, "enable", nullptr);
            enableDriven = std::make_shared<Driven>("enable", enableDLI);
            pointerAsContainer->space["enable"] = enableDriven;
            enableAnnotation = declarativeModule->annotations.find("@enable");
            context->checks.push_back(Context::DriveCheck(enableDriven, nullopt, nullopt, [=]() {
                if (enableAnnotation != declarativeModule->annotations.end()) {
                    auto port = static_cast<Port*>(enableAnnotation->second);
                    tld->propertyAssignment(identifier->idString, "enable", port->identifier->idString);
                    this->hasEnable = true;
                }
            }));

            pointerAsDriven = pointerAsContainer; // Believe it or not, we need this cast first anyway.
            pointer = pointerAsDriven;
            break;
        default:
            pointerAsDriven = std::make_shared<Driven>(identifier->idString, this, from.value(), to.value(), msbFirst);
            if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                pointerAsDriven->drive(optionalAssignment);
            }
            pointer = pointerAsDriven;
        }
    } else {
        pointerAsArray = std::make_shared<SymbolArray>(identifier->idString, this, size);
        for (AccessWidth i = 0; i < size; i += 1) {
            pointerAsArray->array.push_back(std::make_shared<Driven>(identifier->idString, this, from.value(), to.value(), msbFirst));
        }
        pointer = pointerAsArray;
    }
    if ((comb = context->table->findNearest(SymbolSpace::Type::comb))) {
        context->addError(nullopt, "comb.declarationNotAllowed");
        goto exit;
    } else {
        context->table->add(identifier->idString, pointer);
    }

exit:
    //PII
    if (right) {
        rightDLI = (DeclarationListItem*)right;
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
            context->addError(nullopt, "elaboration.softwareExpr");
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
        if ((symSpace = std::dynamic_pointer_cast<SymbolSpace>(symbol))) {
            if (symSpace->type == SymbolSpace::Type::module) {
                if (context->table->findNearest(SymbolSpace::Type::module) != symSpace) {
                    if (!context->table->findNearest(SymbolSpace::Type::comb)) {
                        context->table->add(identifier->idString, std::make_shared<Symbol>(identifier->idString, this));

                        if (ports) {
                            elaboratePorts(context);
                        }

                    } else {
                        symSpace = nullptr;
                        context->addError(nullopt, "comb.declarationNotAllowed");
                    }
                } else {
                    symSpace = nullptr;
                    context->addError(nullopt, "module.recursion");
                }
            } else {
                symSpace = nullptr;
                context->addError(nullopt, "symbol.notAModule");
            }
        } else {
            symSpace = nullptr;
            context->addError(nullopt, "symbol.notAModuleOrSpace");
        }
    } else {
        context->addError(nullopt, "symbol.dne");
    }
    // I profusely apologize for what you just saw.
    
    tryElaborate(right, context);
}

void InstanceDeclaration::elaboratePorts(Context* context) {
    if (!symSpace) { return; }
    tryElaborate(ports, context);
    typedef std::map<std::string, std::pair<Port*, bool> > ListType;
    // port checking
    ListType inputs, outputs;
    for (auto& element: symSpace->space) {
        if (auto port = dynamic_cast<Port*>(element.second->declarator)) {
            if (port->polarity == Port::Polarity::input) {
                inputs[port->identifier->idString] = std::pair(port, false);
            } else {
                outputs[port->identifier->idString] = std::pair(port, false);
            }
        }
    }

    //PII
    auto seeker = ports;
    while (seeker) {
        auto name = seeker->identifier->idString;
        auto relevantExpr = seeker->expression;
        auto inputIterator = inputs.find(name);
        auto outputIterator = outputs.find(name);
        #define MACRO_PORT_WIDTH [&]() {\
            if (relevantPort->bus) {\
                AccessWidth from, to;\
                relevantPort->bus->getValues(&from, &to);\
                return (from < to) ? to - from + 1 : from - to + 1;\
            }\
            return 1;\
        }()
        if (inputIterator != inputs.end()) {
            // Process as input
            if (inputIterator->second.second) {
                context->addError(nullopt, "module.portAlreadyDriven");
            } else {
                auto relevantPort = inputIterator->second.first;
                auto width = MACRO_PORT_WIDTH;
                LHExpression::lhDrivenProcess(relevantExpr, context->table);
                if (width != relevantExpr->numBits) {
                    context->addError(nullopt, "driving.widthMismatch");
                } else {
                    inputIterator->second.second = true;
                }
            }
        } else if (outputIterator != outputs.end()) {
            if (outputIterator->second.second) {
                context->addError(nullopt, "module.portAlreadyDriven");
            } else {
                if (auto lhs = dynamic_cast<LHExpression*>(relevantExpr)) {
                    auto relevantPort = outputIterator->second.first;
                    LHExpression::lhDrivenProcess(relevantExpr, context->table);
                    auto width = MACRO_PORT_WIDTH;
                    bool trash = false;
                    try {
                        NondeclarativeAssignment::drivingAssignment(context, lhs, Expression::abstract(Expression::Type::runTime, width), &trash, &trash);
                        outputIterator->second.second = true;
                    } catch (const char* e) {
                        context->addError(nullopt, e);
                    }
                    
                } else { 
                    context->addError(nullopt, "module.outputConnectedToRHE");
                }
            }
        } else {
            context->addError(nullopt, "module.portNotFound");
        }
        seeker = static_cast<ExpressionIDPair*>(seeker->right);
    }
}

void ExpressionIDPair::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, context);

    tryElaborate(right, context);
}
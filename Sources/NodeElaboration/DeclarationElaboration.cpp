#include "Node.h"
using namespace Phi::Node;

void Port::MACRO_ELAB_SIG_IMP {
    static const char* acceptableAnnotationsInput[] = {
        "@clock",
        "@reset",
        "@enable",
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
    DeclarationListItem* rightDLI;
    std::shared_ptr<Symbol> pointer;
    std::shared_ptr<Driven> pointerAsDriven, clockDriven, resetDriven, enableDriven, resetValueDriven;
    std::shared_ptr<Container> pointerAsContainer;
    std::shared_ptr<SymbolArray> pointerAsArray;
    std::shared_ptr<SymbolSpace> declarativeModule, comb;
    std::map<std::string, Node*>::iterator clockAnnotation, resetAnnotation, enableAnnotation;

    AccessWidth size = 1;
    AccessWidth width = 1;

    bool msbFirst = true;

    optional<AccessWidth> from = nullopt;
    optional<AccessWidth> to = nullopt;

    tryElaborate(array, context);
    LHExpression::lhDrivenProcess(array, context->table);

    declarativeModule = context->table->findNearest(SymbolSpace::Type::module);

    if (array) {
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
            pointerAsContainer->space["data"] = std::make_shared<Driven>("data", this, from.value(), to.value(), msbFirst);
            // TO-DO: Figure out enable
            if (type == VLD::Type::reg) {
                assert(declarativeModule);
                clockDriven = std::make_shared<Driven>("clock", this);
                pointerAsContainer->space["clock"] = clockDriven;
                clockAnnotation = declarativeModule->annotations.find("@clock");
                context->checks.push_back(Context::DriveCheck(clockDriven, nullopt, nullopt, [=]() {
                    if (clockAnnotation != declarativeModule->annotations.end()) {
                        auto port = static_cast<Port*>(clockAnnotation->second);

                        // Unsafe allocations
                        IdentifierExpression* registerName = new IdentifierExpression(identifier);
                        IdentifierExpression* access = new IdentifierExpression(new Identifier("clock"));
                        PropertyAccess* left = new PropertyAccess(registerName, access);
                        IdentifierExpression* right = new IdentifierExpression(port->identifier);
                        NondeclarativeAssignment* nda = new NondeclarativeAssignment(left, right);

                        auto tld = static_cast<TopLevelDeclaration*>(declarativeModule->declarator);
                        auto placement = &tld->addenda;
                        if (*placement != nullptr) {
                            placement = (Statement**)&(*placement)->right;
                        }

                        *placement = nda;
                    } else {
                        context->addError(nullopt, "register.clockUndriven");
                    }
                }));

                resetDriven = std::make_shared<Driven>("reset", this);
                pointerAsContainer->space["reset"] = resetDriven;
                resetAnnotation = declarativeModule->annotations.find("@reset");
                context->checks.push_back(Context::DriveCheck(clockDriven, nullopt, nullopt, [=]() {
                    if (resetAnnotation != declarativeModule->annotations.end()) {
                        auto port = static_cast<Port*>(resetAnnotation->second);

                        // Unsafe allocations
                        IdentifierExpression* registerName = new IdentifierExpression(identifier);
                        IdentifierExpression* access = new IdentifierExpression(new Identifier("reset"));
                        PropertyAccess* left = new PropertyAccess(registerName, access);
                        IdentifierExpression* right = new IdentifierExpression(port->identifier);
                        NondeclarativeAssignment* nda = new NondeclarativeAssignment(left, right);

                        auto tld = static_cast<TopLevelDeclaration*>(declarativeModule->declarator);
                        auto placement = &tld->addenda;
                        if (*placement != nullptr) {
                            placement = (Statement**)&(*placement)->right;
                        }

                        *placement = nda;
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
                pointerAsContainer->space["condition"] = std::make_shared<Driven>("condition", this);
                if (optionalAssignment && optionalAssignment->type != Expression::Type::error) {
                    context->addError(nullopt, "driving.latchNoReset");
                }
            }

            enableDriven = std::make_shared<Driven>("enable", this);
            pointerAsContainer->space["enable"] = enableDriven;
            enableAnnotation = declarativeModule->annotations.find("@enable");
            context->checks.push_back(Context::DriveCheck(enableDriven, nullopt, nullopt, [=]() {
                if (enableAnnotation != declarativeModule->annotations.end()) {
                    auto port = static_cast<Port*>(enableAnnotation->second);

                    // Unsafe allocations
                    IdentifierExpression* registerName = new IdentifierExpression(identifier);
                    IdentifierExpression* access = new IdentifierExpression(new Identifier("enable"));
                    PropertyAccess* left = new PropertyAccess(registerName, access);
                    IdentifierExpression* right = new IdentifierExpression(port->identifier);
                    NondeclarativeAssignment* nda = new NondeclarativeAssignment(left, right);

                    auto tld = static_cast<TopLevelDeclaration*>(declarativeModule->declarator);
                    auto placement = &tld->addenda;
                    if (*placement != nullptr) {
                        placement = (Statement**)&(*placement)->right;
                    }

                    *placement = nda;
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
    tryElaborate(ports, context);

    if (auto comb = context->table->findNearest(SymbolSpace::Type::comb)) {
        context->addError(nullopt, "comb.declarationNotAllowed");
    } else {
        context->table->add(identifier->idString, std::make_shared<Symbol>(identifier->idString, this));
    }

    tryElaborate(right, context);
}

void ExpressionIDPair::MACRO_ELAB_SIG_IMP {
    tryElaborate(expression, context);

    tryElaborate(right, context);
}
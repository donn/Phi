#include "Node.h"

#include <regex>
#include <sstream>

using namespace Phi::Node;

using VLD = Phi::Node::VariableLengthDeclaration;

VLD* VLD::flattenedList(VLD::Type type, Range* bus, DeclarationListItem* list) {
    VLD vld("", VariableLengthDeclaration::Type::wire, nullptr, nullptr, nullptr);

    auto seeker = &vld;
    while (list) {
        auto current = list;

        auto addition = new VLD(current->name, type, bus, current->array, current->optionalAssignment);

        seeker->right = addition;
        seeker = (VLD*)seeker->right;

        list = (DeclarationListItem*)current->right;
        delete current;
    }

    return (VLD*)vld.right;
}
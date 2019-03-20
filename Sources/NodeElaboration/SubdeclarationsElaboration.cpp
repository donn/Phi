#include "Node.h"

#include <regex>
#include <sstream>

using namespace Phi::Node;

using VLD = VariableLengthDeclaration;

VLD* VLD::flattenedList(VLD::Type type, Range* bus, DeclarationListItem* item) {
    VLD vld("", VariableLengthDeclaration::Type::wire, nullptr, nullptr, nullptr);

    auto seeker = &vld;
    while (item) {
        seeker->right = new VLD(item->identifier, type, bus, item->array, item->optionalAssignment);
        seeker = (VLD*)seeker->right;

        auto current = item;
        item = (DeclarationListItem*)current->right;
        delete current;
    }

    return (VLD*)vld.right;
}
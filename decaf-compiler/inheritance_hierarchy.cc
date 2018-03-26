#include "ast_type.h"
#include "inheritance_hierarchy.h"
#include "hashtable.h"
#include "list.h"

bool InheritanceHierarchy::IsSubClassOf(Type *base, Type *derived) {
    Link *l = hierarchy->Lookup(derived->getName());

    while (l) {
        if (l->type->IsEquivalentTo(base)) {
            return true;
        }
        l = l->Parent;
    }
    return false;
}

bool InheritanceHierarchy::IsInterfaceOf(Type *interface, Type *derived) {
    Link *l = hierarchy->Lookup(derived->getName());

    if (l == NULL) return false;

    for (int i = 0; i < l->Interfaces->NumElements(); i++) {
        if (interface->IsEquivalentTo(l->Interfaces->Nth(i))) {
            return true;
        }
    }
    if (l->Parent) return IsInterfaceOf(interface, l->Parent->type);
    return false;
}

void InheritanceHierarchy::AddClassInheritance(Type *base, Type* derived, List<NamedType*> *interfaces) {
    Link *l = new Link();
    l->type = derived;
    l->Interfaces = interfaces;
    l->Parent = base ? hierarchy->Lookup(base->getName()) : NULL;
    hierarchy->Enter(derived->getName(), l);
    
}

InheritanceHierarchy::InheritanceHierarchy() {
    hierarchy = new Hashtable<Link*>();
}
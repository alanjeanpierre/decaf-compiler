#include "ast_type.h"
#include "inheritance_hierarchy.h"
#include "hashtable.h"

bool InheritanceHierarchy::IsSubClassOf(Type *base, Type *derived) {
    List *l = hierarchy->Lookup(derived->getName());

    while (l) {
        if (l->type->IsEquivalentTo(base)) {
            return true;
        }
        l = l->Parent;
    }
    return false;
}

void InheritanceHierarchy::AddClassInheritance(Type *base, Type* derived) {
    List *l = new List();
    l->type = derived;
    l->Parent = hierarchy->Lookup(base->getName());
    if (l->Parent == NULL) {
        l->Parent = new List();
        l->Parent->type = base;
        l->Parent->Parent = NULL;
        hierarchy->Enter(base->getName(), l->Parent);
    }
    hierarchy->Enter(derived->getName(), l);
}

InheritanceHierarchy::InheritanceHierarchy() {
    hierarchy = new Hashtable<List*>();
}
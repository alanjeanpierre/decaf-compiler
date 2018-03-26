#ifndef _INHERITANCE_HIERARCHY_H
#define _INHERITANCE_HIERARCHY_H

#include "hashtable.h"
#include "list.h"

class Type;

class InheritanceHierarchy {
private:
    struct Link {
        Type *type;
        Link *Parent;
        List<NamedType*> *Interfaces;
    };

    Hashtable<Link*> * hierarchy;
public:
    InheritanceHierarchy();
    bool IsSubClassOf(Type *base, Type *derived);
    bool IsInterfaceOf(Type *interface, Type *derived);
    void AddClassInheritance(Type *base, Type* derived, List<NamedType*> *interfaces);
};

#endif
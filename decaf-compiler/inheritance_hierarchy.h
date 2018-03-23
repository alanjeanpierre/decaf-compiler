#ifndef _INHERITANCE_HIERARCHY_H
#define _INHERITANCE_HIERARCHY_H

#include "hashtable.h"

class Type;

class InheritanceHierarchy {
private:
    struct List {
        Type *type;
        List *Parent;
    };

    Hashtable<List*> * hierarchy;
public:
    InheritanceHierarchy();
    bool IsSubClassOf(Type *base, Type *derived);
    void AddClassInheritance(Type *base, Type* derived);
};

#endif
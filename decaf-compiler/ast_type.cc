/* File: ast_type.cc
 * -----------------
 * Implementation of type node classes.
 */
#include "ast_type.h"
#include "ast_decl.h"
#include "errors.h"
#include <string.h>
#include "inheritance_hierarchy.h"

 
/* Class constants
 * ---------------
 * These are public constants for the built-in base types (int, double, etc.)
 * They can be accessed with the syntax Type::intType. This allows you to
 * directly access them and share the built-in types where needed rather that
 * creates lots of copies.
 */

Type *Type::intType    = new Type("int");
Type *Type::doubleType = new Type("double");
Type *Type::voidType   = new Type("void");
Type *Type::boolType   = new Type("bool");
Type *Type::nullType   = new Type("null");
Type *Type::stringType = new Type("string");
Type *Type::errorType  = new Type("error"); 


InheritanceHierarchy *Type::hierarchy = new InheritanceHierarchy();

Type::Type(const char *n) {
    Assert(n);
    typeName = strdup(n);
}

bool Type::IsConvertableTo(Type *other) {
    if (dynamic_cast<ArrayType*>(other) != NULL)
        return false;

    return IsEquivalentTo(other) || (IsEquivalentTo(Type::errorType) || other->IsEquivalentTo(Type::errorType))
    || (IsEquivalentTo(Type::nullType) && dynamic_cast<NamedType*>(other) != NULL);
}
	
NamedType::NamedType(Identifier *i) : Type(*i->GetLocation()) {
    Assert(i != NULL);
    (id=i)->SetParent(this);
} 

bool NamedType::IsConvertableTo(Type *other) {
    // no polymorphism atm

    // add flag for interface OK
    return IsEquivalentTo(other) || Type::hierarchy->IsSubClassOf(other, this) || Type::hierarchy->IsInterfaceOf(other, this);
}

bool NamedType::IsInterfaceableTo(Type *other) {
    return IsEquivalentTo(other) || Type::hierarchy->IsInterfaceOf(other, this);
}

char* NamedType::getName() {
    return id->getName();
}

Identifier* NamedType::getID() {
    return id;
}

bool NamedType::Check() {
    if (!parent->GetEnv()->TypeExists(getID())) {
        ReportError::IdentifierNotDeclared(getID(), LookingForType);
        return false;
    }
    return true;
}

bool NamedType::IsEquivalentTo(Type *other) {
    return strcmp(getName(), other->getName()) == 0;
}

ArrayType::ArrayType(yyltype loc, Type *et) : Type(loc) {
    Assert(et != NULL);
    (elemType=et)->SetParent(this);
}

bool ArrayType::IsConvertableTo(Type *other) {
    ArrayType *o = dynamic_cast<ArrayType*>(other);
    if (o == NULL) {
        return false;
    }

    return elemType->IsConvertableTo(o->elemType);
}


bool ArrayType::IsEquivalentTo(Type *other) {
    return elemType->IsEquivalentTo(other);
}


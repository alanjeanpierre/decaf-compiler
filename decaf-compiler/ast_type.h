/* File: ast_type.h
 * ----------------
 * In our parse tree, Type nodes are used to represent and
 * store type information. The base Type class is used
 * for built-in types, the NamedType for classes and interfaces,
 * and the ArrayType for arrays of other types.  
 *
 * pp3: You will need to extend the Type classes to implement
 * the type system and rules for type equivalency and compatibility.
 */
 
#ifndef _H_ast_type
#define _H_ast_type

#include "ast.h"
#include "list.h"
#include "env_vector.h"
#include <iostream>

class InheritanceHierarchy;

class Type : public Node 
{
  
  protected:
    char *typeName;

  public :
    static Type *intType, *doubleType, *boolType, *voidType,
                *nullType, *stringType, *errorType;

    Type(yyltype loc) : Node(loc) {}
    Type(const char *str);
    
    virtual void PrintToStream(std::ostream& out) { out << typeName; }
    friend std::ostream& operator<<(std::ostream& out, Type *t) { t->PrintToStream(out); return out; }
    bool IsEquivalentTo(Type *other) { return strcmp(getName(), other->getName()) == 0; }
    virtual bool IsConvertableTo(Type *other);
    virtual bool Check() { return true; }
    virtual char* getName() { return typeName; }
    
    static InheritanceHierarchy *hierarchy;
};

class NamedType : public Type 
{
  protected:
    Identifier *id;
    
  public:
    NamedType(Identifier *i);
    
    void PrintToStream(std::ostream& out) { out << id; }
    char* getName();
    Identifier* getID();  
    bool Check();
    bool IsEquivalentTo(Type *other);
    bool IsConvertableTo(Type *other);
    bool IsInterfaceableTo(Type *other);
};

class ArrayType : public Type 
{
  protected:
    Type *elemType;

  public:
    ArrayType(yyltype loc, Type *elemType);
    
    void PrintToStream(std::ostream& out) { out << elemType << "[]"; }
    bool Check() { return elemType->Check(); }
    char* getName() { return elemType->getName(); }
    bool IsEquivalentTo(Type *other);
    bool IsConvertableTo(Type *other);
    Type *GetType() { return elemType; }
};

 
#endif

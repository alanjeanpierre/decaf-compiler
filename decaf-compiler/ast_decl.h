/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 *
 * pp3: You will need to extend the Decl classes to implement 
 * semantic processing including detection of declaration conflicts 
 * and managing scoping issues.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "list.h"
#include "env_vector.h"

class Type;
class NamedType;
class Identifier;
class Stmt;

class Decl : public Node 
{
  protected:
    Identifier *id;
  
  public:
    Decl(Identifier *name);
    friend std::ostream& operator<<(std::ostream& out, Decl *d) { return out << d->id; }
    const char* getName();
    virtual void Check() {;}
    virtual void CheckScope(EnvVector *other) {;}
    virtual void CheckInheritance() {;}
    virtual void CheckImplements() {;}
    virtual void CheckFunctions() {;}
    virtual void CheckTypes() {;}
    bool CheckName(Decl* other) { return strcmp(getName(), other->getName()) == 0;}
};

class VarDecl : public Decl 
{
  protected:
    Type *type;
    
  public:
    VarDecl(Identifier *name, Type *type);
    void Check();
    void CheckScope(EnvVector *env);
    bool MatchesOther(VarDecl *other);

    void CheckInheritance() {;}
    void CheckImplements() {;}
    void CheckFunctions() {;}
    void CheckTypes();
};

class ClassDecl : public Decl 
{
  private:
    bool checked;
    EnvVector *inheritanceVector;

  protected:
    List<Decl*> *members;
    NamedType *extends;
    List<NamedType*> *implements;

  public:
    ClassDecl(Identifier *name, NamedType *extends, 
              List<NamedType*> *implements, List<Decl*> *members);
    void Check() {;}
    void CheckScope(EnvVector *env);

    void CheckInheritance();
    void CheckImplements();
    void CheckFunctions() {;}
    void CheckTypes() {;}
};

class InterfaceDecl : public Decl 
{
  protected:
    List<Decl*> *members;
    
  public:
    InterfaceDecl(Identifier *name, List<Decl*> *members);
    void Check();
    void CheckScope(EnvVector *env);
    bool CheckImplements(EnvVector *sub);
    void AddMethodsToScope(EnvVector *sub);

    void CheckInheritance() {;}
    void CheckImplements() {;}
    void CheckFunctions() {;}
    void CheckTypes() {;}
};

class FnDecl : public Decl 
{
  protected:
    List<VarDecl*> *formals;
    Type *returnType;
    Stmt *body;
    
  public:
    FnDecl(Identifier *name, Type *returnType, List<VarDecl*> *formals);
    void SetFunctionBody(Stmt *b);
    void Check();
    void CheckScope(EnvVector *env);
    bool MatchesOther(FnDecl *other);

    void CheckInheritance() {;}
    void CheckImplements() {;}
    void CheckTypes();
    void CheckFunctions();
};

#endif

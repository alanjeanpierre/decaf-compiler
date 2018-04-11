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
#include "tac.h"

class Type;
class NamedType;
class Identifier;
class Stmt;

class Decl : public Node 
{
  protected:
    Identifier *id;
    Location *memlocation;
  
  public:
    Decl(Identifier *name);
    friend std::ostream& operator<<(std::ostream& out, Decl *d) { return out << d->id; }
    const char* getName();
    Identifier *getID() { return id; }
    virtual void Check() {;}
    virtual void CheckScope(EnvVector *other) {;}
    virtual void CheckInheritance() {;}
    virtual void CheckImplements() {;}
    virtual void CheckFunctions() {;}
    virtual void CheckTypes() {;}
    virtual Type *GetType() { return NULL; }
    bool CheckName(Decl* other) { return strcmp(getName(), other->getName()) == 0;}

    void SetMemLocation(Segment s, int offset);
    Location *GetMemLocation(CodeGenerator *cg) { return memlocation; }
    virtual int EmitClass(CodeGenerator *cg) {;}
    void SetID(char *s) { id->setName(s); }
};

class VarDecl : public Decl 
{
  protected:
    Type *type;
    Type *shadowtype;
   
  public:
    VarDecl(Identifier *name, Type *type);
    void Check();
    void CheckScope(EnvVector *env);
    bool MatchesOther(VarDecl *other);
    
    void AssignType(Type *other) { shadowtype = other; }
    Type *GetCurrentType() { return shadowtype; }

    void CheckInheritance() {;}
    void CheckImplements() {;}
    void CheckFunctions() {;}
    void CheckTypes();
    Type *GetType() { return shadowtype; }

    int Emit(CodeGenerator *cg);
};

class ClassDecl : public Decl 
{
  private:
    bool checked;
    EnvVector *inheritanceVector;
    int ndecls;
    List<const char*> *methodLabels;

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
    void CheckFunctions();
    void CheckTypes() {;}

    void CheckExtends();
    void BuildInterface();
    Type *GetType();

    int Emit(CodeGenerator *cg);
    int GetNumDecls() { return ndecls; }
    int GetVarOffset(char *fieldname);
    int GetFnOffset(char *fieldname);
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

    void CheckInheritance();
    void CheckImplements() {;}
    void CheckFunctions() {;}
    void CheckTypes() {;}
    Type *GetType();


    int Emit(CodeGenerator *cg) {;}
};

class FnDecl : public Decl 
{
  protected:
    List<VarDecl*> *formals;
    Type *returnType;
    Stmt *body;
    const char *vtableid;
    
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
    List<Type*> *GetFormalsTypes();
    Type *GetType() { return returnType; }


    int Emit(CodeGenerator *cg);
    int EmitClass(CodeGenerator *cg);

    void SetVTableID(const char *s) { vtableid = s; }
    const char *GetVTableID() { return vtableid; }
};

#endif

/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "errors.h"
        
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}

const char* Decl::getName() {
    return id->getName();
}


VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
}
  
void VarDecl::Check(EnvVector *env) {
    type->Check(env);
}


ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
}

void ClassDecl::Check(EnvVector *env) {


    if (extends != NULL && !env->TypeExists(extends->getID())) {
        ReportError::IdentifierNotDeclared(extends->getID(), LookingForClass);
    }

    for (int i = 0; i < implements->NumElements(); i++) {
        if(!env->TypeExists(implements->Nth(i)->getID())) {
            ReportError::IdentifierNotDeclared(implements->Nth(i)->getID(), LookingForInterface);
        }
    }

    env->AddType(id);

    env = env->Push();

    // symbols
    for (int i = 0; i < members->NumElements(); i++) {
        env->Insert(members->Nth(i));
    }

    // scope/type check
    for (int i = 0; i < members->NumElements(); i++) {
        members->Nth(i)->Check(env);
    }

}


InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
}

	
void InterfaceDecl::Check(EnvVector *env) {
    ;
}

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}


void FnDecl::Check(EnvVector *env) { 

    env = env->Push();
    for (int i = 0; i < formals->NumElements(); i++) {
        env->InsertIfNotExists(formals->Nth(i));
        formals->Nth(i)->Check(env);
    }
    body->Check(env);

}


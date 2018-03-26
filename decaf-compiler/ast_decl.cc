/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "inheritance_hierarchy.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "errors.h"
        
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
    env = NULL;
}

const char* Decl::getName() {
    return id->getName();
}

bool VarDecl::MatchesOther(VarDecl *other) {
    return this->type->IsEquivalentTo(other->type);
}

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
    shadowtype = t;
}
  
void VarDecl::Check() {
    type->Check();
}

void VarDecl::CheckScope(EnvVector *env) {
    env->InsertIfNotExists(this);
    SetEnv(env);
}

void VarDecl::CheckTypes() {
    if (!type->Check()) {
        AssignType(Type::errorType);
    }
}


ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
    checked = false;
}

void ClassDecl::CheckExtends() {

    if (extends == NULL)
        return;

    if (!env->TypeExists(extends->getID())) {
        ReportError::IdentifierNotDeclared(extends->getID(), LookingForClass);
        env->AddType(new ClassDecl(extends->getID(), NULL, new List<NamedType*>, new List<Decl*>));
    }
    
    ClassDecl *e = dynamic_cast<ClassDecl*>(env->Search(extends->getName()));
    if (e == NULL) {
        return;
    }
        
    e->CheckInheritance();
    EnvVector *parentScope = e->GetEnv();
    env->SetParent(parentScope);

    for (int i = 0; i < members->NumElements(); i++) {
        Decl* n = members->Nth(i);
        Decl* d = parentScope->SearchInScope(n);
        if (d == NULL)
            continue;
        //else
        // matched name
        if (dynamic_cast<VarDecl*>(n)) {
            // no variable redecls
            ReportError::DeclConflict(n, d);
        } else if (FnDecl* dfn = dynamic_cast<FnDecl*>(d)) {
            // is a function too!
            FnDecl* nfn = dynamic_cast<FnDecl*>(n);
            if(!nfn->MatchesOther(dfn)) {
                ReportError::OverrideMismatch(nfn);
            } 
        } else {
            ReportError::DeclConflict(n, d);
        }
    } 
}

void ClassDecl::BuildInterface() {

    for (int i = 0; i < implements->NumElements(); i++) {
        if(!env->TypeExists(implements->Nth(i)->getID())) {
            ReportError::IdentifierNotDeclared(implements->Nth(i)->getID(), LookingForInterface);
            //env->AddType(new InterfaceDecl(implements->Nth(i)->getID(), new List<Decl*>));
        } else {    
            InterfaceDecl *impl = dynamic_cast<InterfaceDecl*>(env->Search(implements->Nth(i)->getName()));
            if (impl) {
                impl->Check();
                impl->AddMethodsToScope(env);
            }
        }
    }
}

void ClassDecl::CheckInheritance() {
    if (checked)
        return;
    checked = true;

    env = parent->GetEnv()->Push();
    env->SetScopeLevel(ClassScope);

    // build class scope
    for (int i = 0; i < members->NumElements(); i++) {
        Decl* n = members->Nth(i);
        env->InsertIfNotExists(n);
        n->SetEnv(env);
        //n->Check();
    }
    
    // build extends symbol table 
    // ONLY NEED DIRECT PARENTS SYMBOL TABLE
    CheckExtends();
    
    // build interface methods
    BuildInterface();

    // add class to type hierarchy
    Type::hierarchy->AddClassInheritance(extends, this->GetType(), implements);
}

void ClassDecl::CheckFunctions() {

    for (int i = 0; i < members->NumElements(); i++) {
        members->Nth(i)->Check();
        //if (FnDecl* dfn = dynamic_cast<FnDecl*>(members->Nth(i))) {
        //    dfn->Check();
        //}
    }
    
}

void ClassDecl::CheckImplements() {
        // build interface methods
    for (int i = 0; i < implements->NumElements(); i++) {
        InterfaceDecl *impl = dynamic_cast<InterfaceDecl*>(env->Search(implements->Nth(i)->getName()));
        if (impl && !impl->CheckImplements(env)) 
            ReportError::InterfaceNotImplemented(this, implements->Nth(i));
    }

}


void ClassDecl::CheckScope(EnvVector *env) {
    env->InsertIfNotExists(this);
    env->AddType(this);
    SetEnv(env);
}

InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
}

	
void InterfaceDecl::Check() {
    EnvVector *next = env->Push();
    for (int i = 0; i < members->NumElements(); i++) {
        members->Nth(i)->CheckScope(next);
    }
}

void InterfaceDecl::CheckScope(EnvVector *env) {
    env->InsertIfNotExists(this);
    env->AddType(this);
    SetEnv(env);
}

bool InterfaceDecl::CheckImplements(EnvVector *sub) {
    bool ok = true;
    for (int i = 0; i < members->NumElements(); i++) {
        FnDecl *match_par = dynamic_cast<FnDecl*>(members->Nth(i));
        FnDecl *match_sub = dynamic_cast<FnDecl*>(sub->SearchInScope(members->Nth(i)));
        if (!match_par || !match_sub) {
            // should be for something like, interface has a method and class 
            // has a field with a name conflict
            // also triggers if interface declares a nonimplemented method
            ok = false;
            continue;
        }
        if(!match_par->MatchesOther(match_sub)) {
            ok = false;
        }

    }
    return ok;
}

void InterfaceDecl::CheckInheritance() {
    env = env->Push();
    for (int i = 0; i < members->NumElements(); i++) {
        env->InsertIfNotExists(members->Nth(i));
    }
}

void InterfaceDecl::AddMethodsToScope(EnvVector *sub) {
    for (int i = 0; i < members->NumElements(); i++) {
        if (FnDecl* d = dynamic_cast<FnDecl*>(sub->SearchInScope(members->Nth(i)))) {
            if (!d->MatchesOther(dynamic_cast<FnDecl*>(members->Nth(i)))) {
                ReportError::OverrideMismatch(d);
            }
        }
        //sub->InsertIfNotExists(members->Nth(i));
    }
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

bool FnDecl::MatchesOther(FnDecl* other) {
    if (!this->returnType->IsEquivalentTo(other->returnType)) 
        return false;

    if (this->formals->NumElements() != other->formals->NumElements())
        return false;

    for (int i = 0; i < formals->NumElements(); i++) {
        if(!formals->Nth(i)->MatchesOther(other->formals->Nth(i)))
            return false;
        
    }
    return true;
}

void FnDecl::CheckTypes() {
    env = env->Push();
    for (int i = 0; i < formals->NumElements(); i++) {
        env->InsertIfNotExists(formals->Nth(i));
        formals->Nth(i)->Check();
    }
}

void FnDecl::Check() { 

    env = env->Push();
    for (int i = 0; i < formals->NumElements(); i++) {
        env->InsertIfNotExists(formals->Nth(i));
        formals->Nth(i)->Check();
    }
    body->SetEnv(env);
    body->Check();

}

void FnDecl::CheckScope(EnvVector *env) {
    env->InsertIfNotExists(this);
    SetEnv(env);
}

void FnDecl::CheckFunctions() {
    body->SetEnv(env);
    body->Check();
}

List<Type*> *FnDecl::GetFormalsTypes() {
    List<Type*> *ts = new List<Type*>;
    for (int i = 0; i < formals->NumElements(); i++) {
        ts->Append(formals->Nth(i)->GetType());
    }
    return ts;
}

Type *ClassDecl::GetType() {
    return new NamedType(id);
}

Type *InterfaceDecl::GetType() {
    return new NamedType(id);
}

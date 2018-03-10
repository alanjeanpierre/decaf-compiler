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
}
  
void VarDecl::Check() {
    type->Check();
}

void VarDecl::CheckScope(EnvVector *env) {
    env->InsertIfNotExists(this);
    SetEnv(env);
}

void VarDecl::CheckInheritance(Decl* other) {
    if (!CheckName(other))
        ReportError::DeclConflict(this, other);
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


void ClassDecl::Check() {
    if (checked)
        return;
    checked = true;

    env = parent->GetEnv()->Push();
    
    // build extends symbol table 
    // ONLY NEED DIRECT PARENTS SYMBOL TABLE
    if (extends != NULL) {
        if (!env->TypeExists(extends->getID())) {
            ReportError::IdentifierNotDeclared(extends->getID(), LookingForClass);
            env->AddType(extends->getID());
        }
        if (ClassDecl *e = dynamic_cast<ClassDecl*>(env->Search(extends->getName()))) {
            for (int i = 0; i < e->members->NumElements(); i++) {
                env->Insert(e->members->Nth(i));
            }
        }
    }

    // build interface methods
    for (int i = 0; i < implements->NumElements(); i++) {
        if(!env->TypeExists(implements->Nth(i)->getID())) {
            ReportError::IdentifierNotDeclared(implements->Nth(i)->getID(), LookingForInterface);
            env->AddType(implements->Nth(i)->getID());
        }
        
        if (Decl *impl = env->Search(implements->Nth(i)->getName()))
            impl->Check();
    }


}
/*
void ClassDecl::Check() {    

    if (checked)
        return;
    checked = true;

    env = parent->GetEnv()->Push();
    // symbols
    for (int i = 0; i < members->NumElements(); i++) {
        members->Nth(i)->CheckScope(env);
    }

    if (extends != NULL ) {
        if (!env->TypeExists(extends->getID())) {
            ReportError::IdentifierNotDeclared(extends->getID(), LookingForClass);
            env->AddType(extends->getID());
        }
        
        // recursively check parents classes
        // and use their environment vectors instead of global scopes
        if (Decl *e = env->Search(extends->getName())) {
            e->Check();
            env->SetParent(e->GetEnv()->Push());
        }
    }

    for (int i = 0; i < implements->NumElements(); i++) {
        if(!env->TypeExists(implements->Nth(i)->getID())) {
            ReportError::IdentifierNotDeclared(implements->Nth(i)->getID(), LookingForInterface);
            env->AddType(implements->Nth(i)->getID());
        }
        
        if (Decl *impl = env->Search(implements->Nth(i)->getName()))
            impl->Check();
    }


    // type check
    for (int i = 0; i < members->NumElements(); i++) {
        members->Nth(i)->Check();
    }

    // check interfacing
    for (int i = 0; i < implements->NumElements(); i++) {
        InterfaceDecl *interface = dynamic_cast<InterfaceDecl*>(parent->GetEnv()->Search(implements->Nth(i)->getName()));
        if(!interface->CheckImplements(env)) {
            ReportError::InterfaceNotImplemented(this, implements->Nth(i));
        }
    }

    // check extends
    if (extends) {
        Decl* p = parent->GetEnv()->Search(extends->getName());
        ClassDecl *e = dynamic_cast<ClassDecl*>(p);
        if (e) {
            EnvVector *pscope = e->GetEnv();
            for (int i = 0; i < members->NumElements(); i++) {
                Decl* cur = members->Nth(i);
                Decl* prev = pscope->SearchInScope(cur);
                if (prev)
                    prev->CheckInheritance(cur);
            }
        }
    }
}
*/

void ClassDecl::CheckScope(EnvVector *env) {
    env->InsertIfNotExists(this);
    env->AddType(this->id);
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
    env->AddType(this->id);
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
            ReportError::OverrideMismatch(match_sub);
            ok = false;
        }

    }
    return ok;
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
    for (int i = 0; i < this->formals->NumElements(); i++) {
        if(!this->formals->Nth(i)->MatchesOther(other->formals->Nth(i)))
            return false;
    }
    return true;
}

void FnDecl::Check() { 

    env = env->Push();
    for (int i = 0; i < formals->NumElements(); i++) {
        env->InsertIfNotExists(formals->Nth(i));
        formals->Nth(i)->Check();
    }
    body->Check();

}

void FnDecl::CheckScope(EnvVector *env) {
    env->InsertIfNotExists(this);
    SetEnv(env);
}

void FnDecl::CheckInheritance(Decl *other) {
    FnDecl *otherF = dynamic_cast<FnDecl*>(other);
    if (!otherF)
        ReportError::DeclConflict(this, other);
    else
        if (!MatchesOther(otherF))
            ReportError::OverrideMismatch(other);
}
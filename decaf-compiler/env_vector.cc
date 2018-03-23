#include "env_vector.h"
#include "errors.h"
#include "ast_expr.h"

Hashtable<Decl*> *EnvVector::types = new Hashtable<Decl*>;

EnvVector::EnvVector() {

    env = new Hashtable<Decl*>;
    parent = NULL;
}

void EnvVector::SetParent(EnvVector *other) {
    parent = other;
}

EnvVector* EnvVector::Push() {
    EnvVector *child = new EnvVector();
    child->parent = this;
    return child;
}

EnvVector* EnvVector::Pop() {
    return parent;
}

Decl* EnvVector::SearchInScope(Decl* id) {
    return env->Lookup(id->getName());
}

Decl* EnvVector::SearchN(Decl* id, int n) {
    EnvVector *h = this;
    Decl* s;
    for (int i = 0; i < n && h; i++) {

        s = h->env->Lookup(id->getName());
        if(s)
            return s;
        h = h->parent;
    }
    return NULL;
}

Decl* EnvVector::Search(Decl* id) {
    EnvVector *h = this;
    Decl* s;
    while(h) {
        s = h->env->Lookup(id->getName());
        if(s)
            return s;
        h = h->parent;
    }
    return NULL;
}


    Decl* EnvVector::Search(const char* id) {
        EnvVector *h = this;
        Decl *s;
        while(h) {
            s = h->env->Lookup(id);
            if(s)
                return s;
            h = h->parent;
        }
        return NULL;
    }

bool EnvVector::InScope(Decl* id) {
    return env->Lookup(id->getName()) != NULL;
}

void EnvVector::Insert(Decl* id) {
    env->Enter(id->getName(), id);
}

bool EnvVector::InsertIfNotExists(Decl* id) {
    if (InScope(id)) {
        ReportError::DeclConflict(id, Search(id));
        return true;
    }
    Insert(id);
    return false;
}

bool EnvVector::TypeExists(Identifier *t) {
    return EnvVector::types->Lookup(t->getName()) != NULL;
}

void EnvVector::AddType(Decl *t) {
    return EnvVector::types->Enter(t->getName(), t);
}

Decl *EnvVector::GetTypeDecl(Identifier *t) {
    return EnvVector::types->Lookup(t->getName());
}

EnvVector *EnvVector::GetProperScope(EnvVector *env, Expr *e) {
    if (e == NULL)
        return env;

    // if this
    if (This* t = dynamic_cast<This*>(e)) {
        if (ClassDecl* c = dynamic_cast<ClassDecl*>(e->GetParent())) {
            return c->GetEnv();
        } else {
            ReportError::ThisOutsideClassScope(t);
            return env;
        }
    }

    // if var.field
    if (FieldAccess *f = dynamic_cast<FieldAccess*>(e)) {
        Decl* d =  env->Search(f->field->getName());
        if (d) {
            if (ClassDecl* c = dynamic_cast<ClassDecl*>(d)) {
                return c->GetEnv();
            } else {
                return NULL;
            }
        }
    } 
    
    return env;
    
}
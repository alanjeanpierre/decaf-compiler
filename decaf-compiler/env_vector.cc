#include "env_vector.h"
#include "errors.h"
#include "ast_expr.h"

Hashtable<Decl*> *EnvVector::types = new Hashtable<Decl*>;

EnvVector::EnvVector() {

    env = new Hashtable<Decl*>;
    scope = GlobalScope;
    parent = NULL;
}

void EnvVector::SetParent(EnvVector *other) {
    parent = other;
}

EnvVector* EnvVector::Push() {
    EnvVector *child = new EnvVector();
    child->parent = this;
    child->scope = scope;
    return child;
}

EnvVector* EnvVector::Pop() {
    return parent;
}

void EnvVector::SetScopeLevel(ScopeLevel s) {
    scope = s;
}

Decl* EnvVector::SearchInScope(Decl* id) {
    return env->Lookup(id->getName());
}

Decl* EnvVector::SearchN(Decl* id, int n) {
    return SearchN(id->getName(), n);
}

Decl *EnvVector::SearchN(const char* id, int n) {
    EnvVector *h = this;
    Decl* s;
    for (int i = 0; i < n && h; i++) {

        s = h->env->Lookup(id);
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

Decl *EnvVector::GetTypeDecl(char *t) {
    return EnvVector::types->Lookup(t);
}

EnvVector *EnvVector::GetProperScope(EnvVector *env, Expr *e) {
    if (e == NULL)
        return env;

    // if this
    if (This* t = dynamic_cast<This*>(e)) {
        return t->GetClass()->GetEnv();
    }

    // if var.field
    if (FieldAccess *f = dynamic_cast<FieldAccess*>(e)) { 
        Decl* d =  env->Search(f->GetFieldName());
        if (d) {
            if (NamedType* t = dynamic_cast<NamedType*>(d->GetType())) {
                Decl *e2 = env->GetTypeDecl(t->getID());
                if (e2 == NULL) return env;
                return e2->GetEnv();
            } else {
                return NULL;
            }
        }
    } //else if (Call *c = dynamic_cast<Call*>(e)) 
    
    return env;
    
}

void EnvVector::PrintScope() { 
    EnvVector *h = this;
    Decl* s;
    while(h) {
        Iterator<Decl*> it = h->env->GetIterator();
        while((s = it.GetNextValue()) != NULL) {
            std::cout << s << std::endl;
        }

        std::cout << std::endl;
        h = h->parent;
    }
}
#include "env_vector.h"
#include "errors.h"

Hashtable<Identifier*> *EnvVector::types = new Hashtable<Identifier*>;

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

void EnvVector::AddType(Identifier *t) {
    return EnvVector::types->Enter(t->getName(), t);
}
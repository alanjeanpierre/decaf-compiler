#ifndef _ENVVECTOR
#define _ENVVECTOR

#include "hashtable.h"
#include "ast_decl.h"
#include "ast_type.h"

class EnvVector {

    private:
        Hashtable<Decl*> *env;
        EnvVector *parent;
        static Hashtable<Identifier*> *types;
    
    public:
        EnvVector();
        EnvVector* Push();
        EnvVector* Pop();
        void SetParent(EnvVector *other);
        Decl* Search(Decl* id);
        Decl* Search(const char* id);
        Decl* SearchInScope(Decl* id);
        bool InScope(Decl* id);
        void Insert(Decl* id);
        bool InsertIfNotExists(Decl* id);

        void AddType(Identifier *t);
        bool TypeExists(Identifier *t);
        
};


#endif
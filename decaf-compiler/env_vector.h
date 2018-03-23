#ifndef _ENVVECTOR
#define _ENVVECTOR

#include "hashtable.h"
#include "ast_decl.h"
#include "ast_type.h"

class Expr;

class EnvVector {

    private:
        Hashtable<Decl*> *env;
        EnvVector *parent;
        static Hashtable<Decl*> *types;
    
    public:
        EnvVector();
        EnvVector* Push();
        EnvVector* Pop();
        void SetParent(EnvVector *other);
        Decl* Search(Decl* id);
        Decl* Search(const char* id);
        Decl* SearchInScope(Decl* id);
        Decl* SearchN(Decl* id, int n);
        bool InScope(Decl* id);
        void Insert(Decl* id);
        bool InsertIfNotExists(Decl* id);

        void AddType(Decl *t);
        bool TypeExists(Identifier *t);
        Decl* GetTypeDecl(Identifier *t);
        
        static EnvVector *GetProperScope(EnvVector *env, Expr *e);
};


#endif
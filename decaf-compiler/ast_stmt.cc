/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "env_vector.h"


Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */
    env = new EnvVector();
    

    // build symbol table for current scope
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->CheckScope(env);
    }

    // check decls/types for current scope and children
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->Check();
    }
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::Check() {
 
    env = env->Push();

    // check decls
    for (int i = 0; i < decls->NumElements(); i++) {
        env->InsertIfNotExists(decls->Nth(i));
        decls->Nth(i)->SetEnv(env);
        decls->Nth(i)->Check();
    }

    for (int i = 0; i < stmts->NumElements(); i++) {
        stmts->Nth(i)->Check();
    }

}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

void ForStmt::Check() {
    env = env->Push();
    body->Check();
}

void WhileStmt::Check() {
    env = env->Push();
    body->Check();
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

void IfStmt::Check() {
    env = env->Push();
    body->Check();
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}
void ReturnStmt::Check() {
    ;
}
  
PrintStmt::PrintStmt(List<Expr*> *a) {    
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}

void PrintStmt::Check() {
    ;
}


void BreakStmt::Check() {
    ;
}
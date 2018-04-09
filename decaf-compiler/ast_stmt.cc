/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "env_vector.h"
#include "codegen.h"


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
    // check types
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->CheckTypes();
    }


    // check inheritance
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->CheckInheritance();
    }
    
    // check implements
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->CheckImplements();
    }

    // check fn children
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->CheckFunctions();
    }
}

void Program::Emit() {
    CodeGenerator *cg = new CodeGenerator();
    bool foundMain = false;
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->SetMemLocation(gpRelative, i*4);
        foundMain |= (strcmp(decls->Nth(i)->getName(), "main") == 0);            
        
    }


    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->Emit(cg);
    }

    if (!foundMain) {
        ReportError::NoMainFound();
    } else {
        cg->DoFinalCodeGen();
    }
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::Check() {
 
    env = env->Push();

    //std::cout << "Printing scope at line: " << parent->GetLocation()->first_line << std::endl;
    //env->PrintScope();
    //std::cout << std::endl;
    // check decls
    for (int i = 0; i < decls->NumElements(); i++) {
        env->InsertIfNotExists(decls->Nth(i));
        decls->Nth(i)->SetEnv(env);
        decls->Nth(i)->Check();
    }

    for (int i = 0; i < stmts->NumElements(); i++) {
        stmts->Nth(i)->SetEnv(env);
        stmts->Nth(i)->Check();
    }

}

int StmtBlock::Emit(CodeGenerator *cg) {
    int space = decls->NumElements();
    for (int i = 0; i < decls->NumElements(); i++) {
        decls->Nth(i)->SetMemLocation(fpRelative, CodeGenerator::OffsetToFirstLocal - i * 4);
        cg->AddStackVar();
    }

    for (int i = 0; i < stmts->NumElements(); i++) {
        space += stmts->Nth(i)->Emit(cg);
    }


    return space;
}

int StmtBlock::GetFrameSize() {
    return decls->NumElements() * 4;
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
    if (init) {
        init->SetEnv(env);
        init->Check();
    }
    test->SetEnv(env);
    if (!test->CheckType(env)->IsConvertableTo(Type::boolType))
        ReportError::TestNotBoolean(test);
    if (step) {
        step->SetEnv(env);
        step->Check();
    }
    body->SetEnv(env);
    body->Check();
}

void WhileStmt::Check() {
    env = env->Push();
    test->SetEnv(env);
    if (!test->CheckType(env)->IsConvertableTo(Type::boolType))
        ReportError::TestNotBoolean(test);
    body->SetEnv(env);
    body->Check();
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

void IfStmt::Check() {
    env = env->Push();
    test->SetEnv(env);
    if (!test->CheckType(env)->IsConvertableTo(Type::boolType))
        ReportError::TestNotBoolean(test);
    
    body->SetEnv(env);
    body->Check();
    if (elseBody) {
        elseBody->SetEnv(env);
        elseBody->Check();
    }
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}
void ReturnStmt::Check() {
    Node *p = parent;
    while (p) {
        FnDecl *f = dynamic_cast<FnDecl*>(p);
        if (f != NULL) {
            // check return type
            Type *rtype = expr->CheckType(env); // empty expr needs to return void?
            Type *ftype = f->GetType();
            if (!rtype->IsConvertableTo(ftype)) {
                ReportError::ReturnMismatch(this, rtype, ftype);
            }
        }
        p = p->GetParent();
    }
   
}

yyltype *ReturnStmt::GetLocation() {
    return expr->GetLocation();
}
  
PrintStmt::PrintStmt(List<Expr*> *a) {    
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}

void PrintStmt::Check() {
    for (int i = 0; i < args->NumElements(); i++) {
        Type *t = args->Nth(i)->CheckType(env);
        if (!(t->IsConvertableTo(Type::intType) || t->IsConvertableTo(Type::stringType) || t->IsConvertableTo(Type::boolType))) {
            ReportError::PrintArgMismatch(args->Nth(i), i+1, t);
        }
    }
}

int PrintStmt::Emit(CodeGenerator *cg) {
    int space = 0;
    for (int i = 0; i < args->NumElements(); i++) {
        Type *t = args->Nth(i)->GetResolvedType();
        Location *tmp = args->Nth(i)->GetMemLocation(cg);
        if (t->IsEquivalentTo(Type::stringType)) {
            cg->GenBuiltInCall(PrintString, tmp);
        } else if (t->IsEquivalentTo(Type::intType)) {
            cg->GenBuiltInCall(PrintInt, tmp);
        } else if (t->IsEquivalentTo(Type::boolType)) {
            cg->GenBuiltInCall(PrintInt, tmp);
        } else { 
                // idk!!
                ;
        }
    }
}


void BreakStmt::Check() {
    Node *p = parent;
    while (p) {
        if (dynamic_cast<LoopStmt*>(p) != NULL) {
            return;
        }
        p = p->GetParent();
    }

    ReportError::BreakOutsideLoop(this);
}

int ReturnStmt::Emit(CodeGenerator *cg) {
    Location *rval = NULL;
    if (expr) 
        rval = expr->GetMemLocation(cg);
    cg->GenReturn(rval);
}

int ForStmt::Emit(CodeGenerator *cg) {
    init->Emit(cg);
    char *start = cg->NewLabel();
    char *end = cg->NewLabel();
    cg->GenLabel(start);
    Location *t = test->GetMemLocation(cg);
    cg->GenIfZ(t, end);
    body->Emit(cg);
    step->Emit(cg);
    cg->GenGoto(start);
    cg->GenLabel(end);
    return 0;
}

int WhileStmt::Emit(CodeGenerator *cg) {
    char *start = cg->NewLabel();
    char *end = cg->NewLabel();
    cg->GenLabel(start);
    Location *t = test->GetMemLocation(cg);
    cg->GenIfZ(t, end);
    body->Emit(cg);
    cg->GenGoto(start);
    cg->GenLabel(end);
}

int IfStmt::Emit(CodeGenerator *cg) {
    char *iftrue = cg->NewLabel();
    char *post = cg->NewLabel();
    Location *t = test->GetMemLocation(cg);
    cg->GenIfZ(t, iftrue);
    
    if (elseBody != NULL) 
        elseBody->Emit(cg);
    cg->GenGoto(post);
    cg->GenLabel(iftrue);
    body->Emit(cg);
    cg->GenLabel(post);
}
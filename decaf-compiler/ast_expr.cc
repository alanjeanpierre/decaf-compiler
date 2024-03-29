/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include <string.h>



IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}
CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}

void ArithmeticExpr::Check() {
    CheckType(env);
}

Type *ArithmeticExpr::CheckType(EnvVector *env) {

    if (left == NULL) {
        Type* r = right->CheckType(env);
        if (r->IsConvertableTo(Type::intType)) return Type::intType;
        if (r->IsConvertableTo(Type::doubleType)) return Type::doubleType;
        ReportError::IncompatibleOperand(op, r);
        return Type::errorType;
    }

    Type* l = left->CheckType(env);
    Type* r = right->CheckType(env);
    if (l->IsConvertableTo(Type::intType) && r->IsConvertableTo(Type::intType)) {
        return Type::intType;
    } else if (l->IsConvertableTo(Type::doubleType) && r->IsConvertableTo(Type::doubleType)) {
        return Type::doubleType;
    } else {
        ReportError::IncompatibleOperands(op, l, r);
        return Type::errorType;
    }
}

void RelationalExpr::Check() {
    CheckType(env);
}

Type *RelationalExpr::CheckType(EnvVector *env) {
    Type *l = left->CheckType(env);
    Type *r = right->CheckType(env);

    if ((l->IsConvertableTo(Type::doubleType) && r->IsConvertableTo(Type::doubleType)) ||
        (l->IsConvertableTo(Type::intType) && r->IsConvertableTo(Type::intType)))
        return Type::boolType;
    else {
        ReportError::IncompatibleOperands(op, l, r);
        return Type::errorType;
    }
}

void EqualityExpr::Check() {
    CheckType(env);
}

Type *EqualityExpr::CheckType(EnvVector *env) {
    Type *l = left->CheckType(env);
    Type *r = right->CheckType(env);

    if (l->IsConvertableTo(r) || r->IsConvertableTo(l)) {
        return Type::boolType;
    } else {
        ReportError::IncompatibleOperands(op, l, r);
        return Type::errorType;
    }
}


void LogicalExpr::Check() {
    CheckType(env);
}

Type *LogicalExpr::CheckType(EnvVector *env) {
    if (left == NULL) {
        Type *r = right->CheckType(env);
        if (r->IsConvertableTo(Type::boolType))
            return Type::boolType;
        ReportError::IncompatibleOperand(op, r);
        return Type::errorType;
    }

    Type *l = left->CheckType(env);
    Type *r = right->CheckType(env);
    if (l->IsConvertableTo(Type::boolType) && r->IsConvertableTo(Type::boolType)) {
        return Type::boolType;
    } else {
        ReportError::IncompatibleOperands(op, l, r);
        return Type::errorType;
    }
}

void AssignExpr::Check() {
    Type *result = CheckType(env);
    FieldAccess *f = dynamic_cast<FieldAccess*>(left);
    if (f != NULL) {
        VarDecl *lval = dynamic_cast<VarDecl*>(env->Search(f->GetFieldName()));
        if (lval)
            lval->AssignType(result);        
    }
}

Type *AssignExpr::CheckType(EnvVector *env) {

    Type *l = left->CheckType(env);
    Type *r = right->CheckType(env);


    if (r->IsConvertableTo(l)) {
        return l;
    }
    else {
        ReportError::IncompatibleOperands(op, l, r);
        // should still return left type and not convert to error?
        return l;
        
    }
}

void ArrayAccess::Check() {
    CheckType(env);
}

Type *ArrayAccess::CheckType(EnvVector *env) {
    ArrayType *b = dynamic_cast<ArrayType*>(base->CheckType(env));
    Type *s = subscript->CheckType(env);

    Type *r_type = Type::errorType;
    if (b == NULL) {
        ReportError::BracketsOnNonArray(base);
    } else {
        r_type = b->GetType();
    }

    if (!s->IsConvertableTo(Type::intType)) {
        ReportError::SubscriptNotInteger(subscript);
    }

    return r_type;

}

void FieldAccess::Check() {
    CheckType(env);
}

Type *FieldAccess::CheckType(EnvVector *env) {

    Type *btype = Type::errorType;
    if (base != NULL) {
        btype = base->CheckType(env);
        if (btype->IsEquivalentTo(Type::errorType)) {
            return Type::errorType;
        }
    }

    if (base == NULL) { // single field access
        Decl* t = env->Search(field->getName());
        if (t){
            if (dynamic_cast<FnDecl*>(t) != NULL) {
                ReportError::IdentifierNotDeclared(field, LookingForVariable);
                return Type::errorType;
            }
            return t->GetType();
        }
        else {
            ReportError::IdentifierNotDeclared(field, LookingForVariable);
            return Type::errorType;
        }
    }


    EnvVector *newEnv = EnvVector::GetProperScope(env, base);

    if (newEnv == NULL) {
        ReportError::FieldNotFoundInBase(field, base->CheckType(env));
        return Type::errorType;
    }


    Decl *f = newEnv->Search(field->getName());
    if ( f == NULL) {
        ReportError::FieldNotFoundInBase(field, btype);
        return Type::errorType;
    } 

    if (!env->IsInClassScope()) {
        ReportError::InaccessibleField(field, base->CheckType(env));
        return Type::errorType;
    }

    return f->GetType();
}
   
void Call::Check() {
    CheckType(env);
}

Type *Call::CheckType(EnvVector *env) {


    Type *btype = Type::errorType;
    if (base != NULL) {
        btype = base->CheckType(env);
        if (btype->IsEquivalentTo(Type::errorType)) {
            return Type::errorType;
        }
    }

    // special case for arr.length()
    if (dynamic_cast<ArrayType*>(btype) != NULL &&
        strcmp(field->getName(), "length") == 0) {
            if (actuals->NumElements() != 0) 
                ReportError::NumArgsMismatch(field, 0, actuals->NumElements());
            return Type::intType;
    }

    EnvVector *newEnv;
    Decl *c = env->GetTypeDecl(btype->getName());
    if (c)
        newEnv = c->GetEnv();
    else {
        newEnv = EnvVector::GetProperScope(env, base);
    }

    if (newEnv == NULL) {
        List<Type*> *actuals_t = new List<Type*>;
        for (int i = 0; i < actuals->NumElements(); i++) {
            actuals_t->Append(actuals->Nth(i)->CheckType(env));
        }
        ReportError::FieldNotFoundInBase(field, base->CheckType(env));
        return Type::errorType;
        
    }


    FnDecl *f = dynamic_cast<FnDecl*>(newEnv->Search(field->getName()));
    if (f == NULL) {
        List<Type*> *actuals_t = new List<Type*>;
        for (int i = 0; i < actuals->NumElements(); i++) {
            actuals_t->Append(actuals->Nth(i)->CheckType(env));
        }
        //std::cerr << "is base null? " << (base == NULL) << std::endl;
        if (base == NULL) {
            ReportError::IdentifierNotDeclared(field, LookingForFunction);
        } else {
            ReportError::FieldNotFoundInBase(field, btype);
        }
        return Type::errorType;
    } 

    List<Type*> *formals = f->GetFormalsTypes();
    List<Type*> *actuals_t = new List<Type*>;
    for (int i = 0; i < actuals->NumElements(); i++) {
        actuals_t->Append(actuals->Nth(i)->CheckType(env));
    }

    int n = actuals_t->NumElements();
    if (actuals_t->NumElements() != formals->NumElements()) {
        ReportError::NumArgsMismatch(field, formals->NumElements(), actuals->NumElements());
        if (actuals_t->NumElements() > formals->NumElements())
            n = formals->NumElements();
    }

    for (int i = 0; i < n; i++) {
        Type *t = actuals_t->Nth(i);
        if (!t->IsConvertableTo(formals->Nth(i))) {
            ReportError::ArgMismatch(actuals->Nth(i), i+1, t, formals->Nth(i));
        }
    }
    return f->GetType();


}

void NewExpr::Check() {
    CheckType(env);
}

Type *NewExpr::CheckType(EnvVector *env) {
    if (env->TypeExists(cType->getID()) && dynamic_cast<ClassDecl*>(env->GetTypeDecl(cType->getID())) != NULL) {
        return cType;
    }

    ReportError::IdentifierNotDeclared(cType->getID(), LookingForClass);
    return Type::errorType;
}

void NewArrayExpr::Check() {
    CheckType(env);
}

void This::Check() {
    CheckType(env);
}

Decl *This::GetClass() {
    Node *t = this;
    while(t) {
        ClassDecl *c = dynamic_cast<ClassDecl*>(t);
        if (c != NULL) {
            return c;
        }
        t = t->GetParent();
    }

    return NULL;
}

Type *This::CheckType(EnvVector *env) {
    if (!env->IsInClassScope()) {
        ReportError::ThisOutsideClassScope(this);
        return Type::errorType;
    }

    
    Decl *class_ = GetClass();
    if (class_) 
        return class_->GetType();

    return Type::errorType;
}

void ReadIntegerExpr::Check() {
    CheckType(env);
}

void ReadLineExpr::Check() {
    CheckType(env);
}

Type *NewArrayExpr::CheckType(EnvVector *env) {
    if (!size->CheckType(env)->IsConvertableTo(Type::intType))
        ReportError::NewArraySizeNotInteger(size);
    
    if (NamedType *t = dynamic_cast<NamedType*>(elemType)) {
        if (!env->TypeExists(t->getID())) {
            ReportError::IdentifierNotDeclared(t->getID(), LookingForType);
            return new ArrayType(*location, Type::errorType);    
        }
    }
    return new ArrayType(*location, elemType);
}

ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}
     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}


Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}
 

NewExpr::NewExpr(yyltype loc, NamedType *c) : Expr(loc) { 
  Assert(c != NULL);
  (cType=c)->SetParent(this);
}


NewArrayExpr::NewArrayExpr(yyltype loc, Expr *sz, Type *et) : Expr(loc) {
    Assert(sz != NULL && et != NULL);
    (size=sz)->SetParent(this); 
    (elemType=et)->SetParent(this);
}

       

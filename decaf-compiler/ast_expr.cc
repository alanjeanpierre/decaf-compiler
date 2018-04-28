/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "codegen.h"
#include "tac.h"
#include <string.h>





IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}

Location *IntConstant::GetMemLocation(CodeGenerator *cg) {
    return cg->GenLoadConstant(GetVal());
}

DoubleConstant::DoubleConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}

Location *BoolConstant::GetMemLocation(CodeGenerator *cg) {
    return cg->GenLoadConstant(GetVal());
}

Location *NullConstant::GetMemLocation(CodeGenerator *cg) {
    return cg->GenLoadConstant(0);
}

StringConstant::StringConstant(yyltype loc, const char *val) : Expr(loc) {
    Assert(val != NULL);
    value = strdup(val);
}

Location *StringConstant::GetMemLocation(CodeGenerator *cg) {
    return cg->GenLoadConstant(GetVal());
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
        resolvedType = Type::errorType;
        return Type::errorType;
    }

    Type* l = left->CheckType(env);
    Type* r = right->CheckType(env);
    if (l->IsConvertableTo(Type::intType) && r->IsConvertableTo(Type::intType)) {
        resolvedType = Type::intType;
        return Type::intType;
    } else if (l->IsConvertableTo(Type::doubleType) && r->IsConvertableTo(Type::doubleType)) {
        resolvedType = Type::doubleType;
        return Type::doubleType;
    } else {
        ReportError::IncompatibleOperands(op, l, r);
        resolvedType = Type::errorType;
        return Type::errorType;
    }
}

Location *ArithmeticExpr::GetMemLocation(CodeGenerator *cg) {
    Location *l;
    if (left == NULL) {
        l = cg->GenLoadConstant(0);
    } else {
        l = left->GetMemLocation(cg);
    }

    Location *r = right->GetMemLocation(cg);
    return cg->GenBinaryOp(op->GetOp(), l, r);
}

void RelationalExpr::Check() {
    resolvedType = CheckType(env);
}

Type *RelationalExpr::CheckType(EnvVector *env) {
    Type *l = left->CheckType(env);
    Type *r = right->CheckType(env);

    if ((l->IsConvertableTo(Type::doubleType) && r->IsConvertableTo(Type::doubleType)) ||
        (l->IsConvertableTo(Type::intType) && r->IsConvertableTo(Type::intType))) {
        resolvedType = Type::boolType;
        return Type::boolType;
    }
    else {
        ReportError::IncompatibleOperands(op, l, r);
        resolvedType = Type::errorType;
        return Type::errorType;
    }
}

Location *RelationalExpr::GetMemLocation(CodeGenerator *cg) {

    if (strcmp(op->GetOp(), ">") == 0) {
        Expr *tmp = left;
        left = right;
        right = tmp;
        op = new Operator(*op->GetLocation(), "<");
    } else if (strcmp(op->GetOp(), ">=") == 0) {
        Expr *tmp = left;
        left = right;
        right = tmp;
        op = new Operator(*op->GetLocation(), "<=");
    }

    Location *l = left->GetMemLocation(cg);
    Location *r = right->GetMemLocation(cg);
    
    if (strcmp(op->GetOp(), "<=") == 0) {
        Location *lt = cg->GenBinaryOp("<", l, r);
        Location *eq = cg->GenBinaryOp("==", l, r);
        return cg->GenBinaryOp("||", lt, eq);
    } else {
        return cg->GenBinaryOp(op->GetOp(), l, r);
    }
}

void EqualityExpr::Check() {
    resolvedType = CheckType(env);
}

Type *EqualityExpr::CheckType(EnvVector *env) {
    Type *l = left->CheckType(env);
    Type *r = right->CheckType(env);

    if (l->IsConvertableTo(r) || r->IsConvertableTo(l)) {
        resolvedType = Type::boolType;
        return Type::boolType;
    } else {
        ReportError::IncompatibleOperands(op, l, r);
        resolvedType = Type::errorType;
        return Type::errorType;
    }
}

Location *EqualityExpr::GetMemLocation(CodeGenerator *cg) {
    Location *l = left->GetMemLocation(cg);
    Location *r = right->GetMemLocation(cg);
    Location *b;
    if (left->GetResolvedType()->IsEquivalentTo(Type::stringType)) {
        b = cg->GenBuiltInCall(StringEqual, l, r);
    } else {
        b = cg->GenBinaryOp("==", l, r);
    }

    if (strcmp(op->GetOp(), "!=") == 0) {
        Location *zero = cg->GenLoadConstant(0);
        b = cg->GenBinaryOp("==", b, zero);
    }

    return b;

}


void LogicalExpr::Check() {
    resolvedType = CheckType(env);
}

Type *LogicalExpr::CheckType(EnvVector *env) {
    if (left == NULL) {
        Type *r = right->CheckType(env);
        if (r->IsConvertableTo(Type::boolType)) {

            resolvedType = Type::boolType;
            return Type::boolType;
        }
        ReportError::IncompatibleOperand(op, r);

        resolvedType = Type::errorType;
        return Type::errorType;
    }

    Type *l = left->CheckType(env);
    Type *r = right->CheckType(env);
    if (l->IsConvertableTo(Type::boolType) && r->IsConvertableTo(Type::boolType)) {
        resolvedType = Type::boolType;
        return Type::boolType;
    } else {
        ReportError::IncompatibleOperands(op, l, r);
        resolvedType = Type::errorType;
        return Type::errorType;
    }
}

Location *LogicalExpr::GetMemLocation(CodeGenerator *cg) {
    Location *r = right->GetMemLocation(cg);
    if (left == NULL) {// negation {
        Location *l = cg->GenLoadConstant(0);
        cg->GenBinaryOp("==", l, r);
    } else {
        Location *l = left->GetMemLocation(cg);
        return cg->GenBinaryOp(op->GetOp(), l, r);
    }
}

void AssignExpr::Check() {
    Type *result = CheckType(env);
    resolvedType = result;
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


    resolvedType = l;
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
    resolvedType = CheckType(env);
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

    resolvedType = r_type;
    return r_type;

}

void FieldAccess::Check() {
    resolvedType = CheckType(env);
}

Type *FieldAccess::CheckType(EnvVector *env) {

    this->env = env;
    Type *btype = Type::errorType;
    resolvedType = Type::errorType;
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
            resolvedType = t->GetType();
            return resolvedType;
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
    resolvedType = f->GetType();
    return resolvedType;
}

ClassDecl *FieldAccess::GetClassFromImplicitThis() {
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

ClassDecl *Call::GetClassFromImplicitThis() {
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

   
void Call::Check() {
    resolvedType = CheckType(env);
}

Type *Call::CheckType(EnvVector *env) {

    this->env = env;
    Type *btype = Type::errorType;
    resolvedType = Type::errorType;
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
            resolvedType = Type::intType;
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
    resolvedType = f->GetType();
    return resolvedType;


}

void NewExpr::Check() {
    resolvedType = CheckType(env);
}

Type *NewExpr::CheckType(EnvVector *env) {
    if (env->TypeExists(cType->getID()) && dynamic_cast<ClassDecl*>(env->GetTypeDecl(cType->getID())) != NULL) {
        resolvedType = cType;
        return cType;
    }

    ReportError::IdentifierNotDeclared(cType->getID(), LookingForClass);
    resolvedType = Type::errorType;
    return Type::errorType;
}

void NewArrayExpr::Check() {
    resolvedType = CheckType(env);
}

void This::Check() {
    resolvedType = CheckType(env);
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
        resolvedType = Type::errorType;
        return Type::errorType;
    }

    
    Decl *class_ = GetClass();
    if (class_) {
        resolvedType = class_->GetType();
        return resolvedType;
    }

    resolvedType = Type::errorType;
    return Type::errorType;
}

void ReadIntegerExpr::Check() {
    resolvedType = CheckType(env);
}

void ReadLineExpr::Check() {
    resolvedType = CheckType(env);
}

Type *NewArrayExpr::CheckType(EnvVector *env) {
    if (!size->CheckType(env)->IsConvertableTo(Type::intType))
        ReportError::NewArraySizeNotInteger(size);
    
    if (NamedType *t = dynamic_cast<NamedType*>(elemType)) {
        if (!env->TypeExists(t->getID())) {
            ReportError::IdentifierNotDeclared(t->getID(), LookingForType);
            resolvedType = new ArrayType(*location, Type::errorType);   
            return resolvedType; 
        }
    }
    resolvedType = new ArrayType(*location, elemType);
    return resolvedType;
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

Location *NewArrayExpr::GetMemLocation(CodeGenerator *cg) {
    Location *sz = size->GetMemLocation(cg);

    // check for < 0 size
    Location *zero = cg->GenLoadConstant(0);
    Location *test = cg->GenBinaryOp("<", sz, zero);
    char *zerojmplabel = cg->NewLabel();
    cg->GenIfZ(test, zerojmplabel);
    Location *errormsg = cg->GenLoadConstant(err_arr_bad_size);
    cg->GenBuiltInCall(PrintString, errormsg);
    cg->GenBuiltInCall(Halt);

    cg->GenLabel(zerojmplabel);
    Location *arraysizeinc = cg->GenLoadConstant(1);
    Location *numbytes = cg->GenBinaryOp("+", sz, arraysizeinc);
    Location *typesizeof = cg->GenLoadConstant(CodeGenerator::VarSize);
    Location *arraysize = cg->GenBinaryOp("*", typesizeof, numbytes);
    Location *arrloc = cg->GenBuiltInCall(Alloc, arraysize);
    // store size of array
    cg->GenStore(arrloc, sz); 
    Location *returnedArrLoc = cg->GenBinaryOp("+", arrloc, typesizeof);
    return returnedArrLoc;
}       

Location *ArrayAccess::GetPtrLocation(CodeGenerator *cg) {

    Location *arraddr = base->GetMemLocation(cg);
    Location *index = subscript->GetMemLocation(cg);
    /*
    static int x = 0;
    char c[10];
    sprintf(c, "access %d\\n", x++);
    Location *out = cg->GenLoadConstant(c);
    cg->GenBuiltInCall(PrintString, out);
    */
    // oob tests
    
    //std::cerr << "l: " << nullcheck(arraddr) << " r: " << nullcheck(index) << std::endl;
    char *notoob = cg->NewLabel();
    Location *zero = cg->GenLoadConstant(0);
    Location *negindex = cg->GenBinaryOp("<", index, zero);
    Location *arrsize = cg->GenLoad(arraddr, -CodeGenerator::VarSize);
    Location *oob1 = cg->GenBinaryOp("<", index, arrsize);
    Location *oob2 = cg->GenBinaryOp("==", oob1, zero);
    Location *test = cg->GenBinaryOp("||", negindex, oob2);
    cg->GenIfZ(test, notoob);
    Location *err = cg->GenLoadConstant(err_arr_out_of_bounds);
    cg->GenBuiltInCall(PrintString, err);
    cg->GenBuiltInCall(Halt);

    cg->GenLabel(notoob);
    Location *varsize = cg->GenLoadConstant(CodeGenerator::VarSize);
    Location *offset = cg->GenBinaryOp("*", varsize, index);
    Location *addr = cg->GenBinaryOp("+", arraddr, offset);
    
    return addr;

}

Location *ArrayAccess::GetMemLocation(CodeGenerator *cg) {
    Location *addr = GetPtrLocation(cg);
    return cg->GenLoad(addr);
}

Location *NewExpr::GetMemLocation(CodeGenerator *cg) {
    ClassDecl *cdecl = dynamic_cast<ClassDecl*>(env->GetTypeDecl(cType->getName()));
    int nvars = cdecl->GetNumDecls() + 1 ; // include vtable ptr
    Location *space = cg->GenLoadConstant(nvars * CodeGenerator::VarSize);
    Location *var = cg->GenBuiltInCall(Alloc, space);
    Location *vtable = cg->GenLoadLabel(cdecl->getName());
    cg->GenStore(var, vtable);


    return var;
} 

bool FieldAccess::IsMemberVariable() {

    ClassDecl* cdecl = GetClassFromImplicitThis();
    
    if (cdecl == NULL) return false;

    int offset = cdecl->GetVarOffset(GetFieldName());

    return offset != -1;
}

int FieldAccess::NScopesToParentFunc() {
    Node *p = this;
    FnDecl *d;
    int i = 0;
    while (p) {
        if (d = dynamic_cast<FnDecl*>(p)) 
            break;
        p = p->GetParent();
    }

    EnvVector *fnenv = d->GetEnv();
    EnvVector *nth = env;
    while (nth) {
        if (nth == fnenv) 
            break;
        i++;
        nth = nth->Pop();
    }
    return i;
}

Location *FieldAccess::GetPtrLocation(CodeGenerator *cg) {
    if (base == NULL) {
        Decl *d = env->Search(field->getName());
        int n = NScopesToParentFunc();
        Decl *d2 = env->SearchN(d, n);
        if (env->IsInClassScope() && d2 == NULL) {
            return cg->ThisPtr;
        } else {
            return d->GetMemLocation(cg);
        }
    } else { 
        return base->GetMemLocation(cg);
    }
}

Location *FieldAccess::GetMemLocation(CodeGenerator *cg) {


    if (base == NULL) {
        Decl *d = env->Search(field->getName());
        int n = NScopesToParentFunc();
        Decl *d2 = env->SearchN(field->getName(), n+1);
        if (env->IsInClassScope() && d2 == NULL) {
                ClassDecl *cdecl = GetClassFromImplicitThis();
                int offset = cdecl->GetVarOffset(GetFieldName());
                if (offset == -1) return d->GetMemLocation(cg);
                Location *memloc = cg->GenLoad(CodeGenerator::ThisPtr, offset * CodeGenerator::VarSize);
                return memloc;
        } else {
            return d->GetMemLocation(cg);
        }
    } else { 
        ClassDecl *c = dynamic_cast<ClassDecl*>(env->GetTypeDecl(base->GetResolvedType()->getName()));
        Location *var = base->GetMemLocation(cg);
        int offset = c->GetVarOffset(GetFieldName()) * CodeGenerator::VarSize;
        Location *memloc = cg->GenLoad(var, offset);
        return memloc;
    }
    

}

void Call::LoadParams(CodeGenerator *cg) {
    params = new List<Location*>();
    for (int i = 0; i < actuals->NumElements(); i++) {
        params->Append(actuals->Nth(i)->GetMemLocation(cg));
    }
} 

void Call::PushParams(CodeGenerator *cg) {
    for (int i = actuals->NumElements()-1; i >= 0; i--) {
        cg->GenPushParam(params->Nth(i));
    }
}

int Call::GetFnOffset() {
    if (base) {
        ClassDecl *c = dynamic_cast<ClassDecl*>(env->GetTypeDecl(base->GetResolvedType()->getName()));
        return c->GetFnOffset(GetFieldName());
    } else {
        ClassDecl *cdecl = GetClassFromImplicitThis();
        return cdecl->GetFnOffset(GetFieldName());
    }
}

bool Call::isMethodCall() {
    if (base == NULL) {
        if (env->IsInClassScope()) {
            ClassDecl *cdecl = GetClassFromImplicitThis();
            int offset = cdecl->GetFnOffset(field->getName());
            return offset != -1;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

bool Call::isArrLength() {
    return base != NULL && 
    dynamic_cast<ArrayType*>(base->GetResolvedType()) != NULL && 
    strcmp(field->getName(), "length") == 0;
}

Location *Call::GetMemLocation(CodeGenerator *cg) {


    // special case for arr.length()
    if (isArrLength()) {
        Location *arraddr = base->GetMemLocation(cg);
        Location *size = cg->GenLoad(arraddr, -CodeGenerator::VarSize);
        return size;
    }

    /*
    Decl *d;
    if (base == NULL) {
        d = env->Search(field->getName());
    } else {
        EnvVector *newEnv = env->GetProperScope(env, base);
        d = newEnv->Search(field->getName());
    }
    */
    Location *rvalue;
    int thisptrbytes = 0;
    LoadParams(cg);
    if (isMethodCall()) {
        Location *maybethis = cg->ThisPtr;
        if (base) maybethis = base->GetMemLocation(cg);
        Location *vtable = cg->GenLoad(maybethis, 0);
        int offset = GetFnOffset();
        Location *fn = cg->GenLoad(vtable, offset * CodeGenerator::VarSize);
        PushParams(cg);
        cg->GenPushParam(maybethis);
        rvalue = cg->GenACall(fn, true);
        thisptrbytes += 1;
    } else {
        PushParams(cg);
        rvalue = cg->GenLCall(field->getName(), true);
    }
/*
    if (base == NULL) {
        if (env->IsInClassScope()) {
            ClassDecl *cdecl = GetClassFromImplicitThis();
            int offset = cdecl->GetFnOffset(field->getName());
            if (offset == -1) {
                // global func
                rvalue = cg->GenLCall(field->getName(), true);    
            } else {

                // else, member function, so push thisptr
                cg->GenPushParam(cg->ThisPtr);
                Location *vtable = cg->GenLoad(cg->ThisPtr, 0);
                ClassDecl *cdecl = GetClassFromImplicitThis();
                int offset = cdecl->GetFnOffset(GetFieldName());
                Location *fn = cg->GenLoad(vtable, offset *CodeGenerator::VarSize);
                rvalue = cg->GenACall(fn, true);
                thisptrbytes = 1;
            }

        } else {
            rvalue = cg->GenLCall(field->getName(), true);
        }
    } else {
        // please don't do ; arr.length(); kkk;;;
        Location *thisptr = base->GetMemLocation(cg);
        Location *vtable = cg->GenLoad(thisptr, 0);
        ClassDecl *cdecl = dynamic_cast<ClassDecl*>(env->GetTypeDecl(base->GetResolvedType()->getName()));
        int offset = cdecl->GetFnOffset(field->getName());

        Location *fn = cg->GenLoad(vtable, offset*CodeGenerator::VarSize);
        cg->GenPushParam(thisptr);
        rvalue = cg->GenACall(fn, true);
        thisptrbytes = 1;
    }
*/
    cg->GenPopParams((thisptrbytes + actuals->NumElements()) * CodeGenerator::VarSize);


    return rvalue;


}

int Call::Emit(CodeGenerator *cg) {

/*
    List<Location*> params;
    for (int i = 0; i < actuals->NumElements(); i++) {
        params.Append(actuals->Nth(i)->GetMemLocation(cg));
    }
    for (int i = actuals->NumElements()-1; i >= 0; i--) {
        cg->GenPushParam(params.Nth(i));
    } 
*/

    Location *rvalue;
    int thisptrbytes = 0;
    LoadParams(cg);
    if (isMethodCall()) {
        Location *maybethis = cg->ThisPtr;
        if (base) maybethis = base->GetMemLocation(cg);
        Location *vtable = cg->GenLoad(maybethis, 0);
        int offset = GetFnOffset();
        Location *fn = cg->GenLoad(vtable, offset * CodeGenerator::VarSize);
        PushParams(cg);
        cg->GenPushParam(maybethis);
        rvalue = cg->GenACall(fn, false);
        thisptrbytes += 1;
    } else {
        PushParams(cg);
        rvalue = cg->GenLCall(field->getName(), false);
    }
/*
    int thisptrbytes = 0;

    if (base == NULL) {
        if (env->IsInClassScope()) {
            ClassDecl *cdecl = GetClassFromImplicitThis();
            int offset = cdecl->GetFnOffset(field->getName());
            if (offset == -1) {
                // global func
                cg->GenLCall(field->getName(), false);    
            } else {

                // else, member function, so push thisptr
                cg->GenPushParam(cg->ThisPtr);
                Location *vtable = cg->GenLoad(cg->ThisPtr, 0);
                ClassDecl *cdecl = GetClassFromImplicitThis();
                int offset = cdecl->GetFnOffset(GetFieldName());
                Location *fn = cg->GenLoad(vtable, offset *CodeGenerator::VarSize);
                cg->GenACall(fn, false);
                thisptrbytes = 1;
                
            }

        } else {
            cg->GenLCall(field->getName(), false);
        }
    } else {
        // please don't do ; arr.length(); kkk;;;
        Location *thisptr = base->GetMemLocation(cg);
        Location *vtable = cg->GenLoad(thisptr, 0);
        ClassDecl *cdecl = dynamic_cast<ClassDecl*>(env->GetTypeDecl(base->GetResolvedType()->getName()));
        int offset = cdecl->GetFnOffset(field->getName());

        Location *fn = cg->GenLoad(vtable, offset*CodeGenerator::VarSize);
        cg->GenPushParam(thisptr);
        cg->GenACall(fn, false);
        thisptrbytes = 1;
    }
*/
    cg->GenPopParams((thisptrbytes + actuals->NumElements()) * CodeGenerator::VarSize);
}

Location *ReadIntegerExpr::GetMemLocation(CodeGenerator *cg) {
    return cg->GenBuiltInCall(ReadInteger);
}

Location *ReadLineExpr::GetMemLocation(CodeGenerator *cg) {
    return cg->GenBuiltInCall(ReadLine);
}

Location *This::GetMemLocation(CodeGenerator *cg) {
     return cg->ThisPtr;
}

int AssignExpr::Emit(CodeGenerator *cg) {

    Location *r = right->GetMemLocation(cg);

    if (dynamic_cast<ArrayAccess*>(left) != NULL) {
        ArrayAccess *arr = dynamic_cast<ArrayAccess*>(left);
        Location *l = arr->GetPtrLocation(cg);
        cg->GenStore(l, r);
    } else if (dynamic_cast<FieldAccess*>(left)->IsMemberVariable()) {
        FieldAccess *member = dynamic_cast<FieldAccess*>(left);
        int offset = member->GetOffset()*4;
        Location *var = member->GetPtrLocation(cg);
        cg->GenStore(var, r, offset);
    }
    else {
        Location *l = left->GetMemLocation(cg);
        cg->GenAssign(l, r);
    }

    return 0;
}

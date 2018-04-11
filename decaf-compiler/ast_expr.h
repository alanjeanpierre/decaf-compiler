/* File: ast_expr.h
 * ----------------
 * The Expr class and its subclasses are used to represent
 * expressions in the parse tree.  For each expression in the
 * language (add, call, New, etc.) there is a corresponding
 * node class for that construct. 
 *
 * pp3: You will need to extend the Expr classes to implement 
 * semantic analysis for rules pertaining to expressions.
 */


#ifndef _H_ast_expr
#define _H_ast_expr

#include "ast.h"
#include "ast_stmt.h"
#include "list.h"
#include "ast_type.h"

class Location;
class NamedType; // for new
//class Type; // for NewArray


class Expr : public Stmt 
{
  protected:
     Type *resolvedType;
  public:
    Expr(yyltype loc) : Stmt(loc) {}
    Expr() : Stmt() {}
    virtual Type *CheckType(EnvVector *env) { return NULL; }
    Type *GetResolvedType() { return resolvedType; }
    virtual Location *GetMemLocation(CodeGenerator *cg) {;}
};

/* This node type is used for those places where an expression is optional.
 * We could use a NULL pointer, but then it adds a lot of checking for
 * NULL. By using a valid, but no-op, node, we save that trouble */
class EmptyExpr : public Expr
{
  public:
    Type *CheckType(EnvVector *env) { return Type::voidType; }
    void Check(EnvVector *env) {;}
    void Check() {;}
    Location *GetMemLocation(CodeGenerator *cg) { return NULL; }
};

class IntConstant : public Expr 
{
  protected:
    int value;
  
  public:
    IntConstant(yyltype loc, int val);
    void Check(EnvVector *env) {;}
    void Check() {;}
    Type *CheckType(EnvVector *env) { resolvedType = Type::intType; return Type::intType; }
    int GetVal() { return value; }
    Location *GetMemLocation(CodeGenerator *cg);
};

class DoubleConstant : public Expr 
{
  protected:
    double value;
    
  public:
    DoubleConstant(yyltype loc, double val);
    void Check(EnvVector *env) {;}
    void Check() {;}
    Type *CheckType(EnvVector *env) { resolvedType = Type::doubleType; return Type::doubleType; }
    double GetVal() { return value; }
};

class BoolConstant : public Expr 
{
  protected:
    bool value;
    
  public:
    BoolConstant(yyltype loc, bool val);
    void Check(EnvVector *env) {;}
    void Check() {;}
    Type *CheckType(EnvVector *env) { resolvedType = Type::boolType; return Type::boolType; }
    int GetVal() { return value ? 1 : 0; }
    Location *GetMemLocation(CodeGenerator *cg);
};

class StringConstant : public Expr 
{ 
  protected:
    char *value;
    
  public:
    StringConstant(yyltype loc, const char *val);
    void Check(EnvVector *env) {;}
    void Check() {;}
    Type *CheckType(EnvVector *env) { resolvedType = Type::stringType; return Type::stringType; }
    char * GetVal() { return value; }
    Location *GetMemLocation(CodeGenerator *cg);
};

class NullConstant: public Expr 
{
  public: 
    NullConstant(yyltype loc) : Expr(loc) {}
    void Check(EnvVector *env) {;}
    void Check() {;}
    Type *CheckType(EnvVector *env) { resolvedType = Type::nullType; return Type::nullType; }
};

class Operator : public Node 
{
  protected:
    char tokenString[4];
    
  public:
    Operator(yyltype loc, const char *tok);
    friend std::ostream& operator<<(std::ostream& out, Operator *o) { return out << o->tokenString; }
    void Check(EnvVector *env) {;}
    char *GetOp() { return tokenString; }
 };
 
class CompoundExpr : public Expr
{
  protected:
    Operator *op;
    Expr *left, *right; // left will be NULL if unary
    
  public:
    CompoundExpr(Expr *lhs, Operator *op, Expr *rhs); // for binary
    CompoundExpr(Operator *op, Expr *rhs);             // for unary
    Type *CheckType(EnvVector *env) { return Type::errorType; }
};

class ArithmeticExpr : public CompoundExpr 
{
  public:
    ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    ArithmeticExpr(Operator *op, Expr *rhs) : CompoundExpr(op,rhs) {}
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env);
    Location *GetMemLocation(CodeGenerator *cg);
};

class RelationalExpr : public CompoundExpr 
{
  public:
    RelationalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    Type *CheckType(EnvVector *env);
    void Check();
    Location *GetMemLocation(CodeGenerator *cg);
};

class EqualityExpr : public CompoundExpr 
{
  public:
    EqualityExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    const char *GetPrintNameForNode() { return "EqualityExpr"; }
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env);
    Location *GetMemLocation(CodeGenerator *cg);
};

class LogicalExpr : public CompoundExpr 
{
  public:
    LogicalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    LogicalExpr(Operator *op, Expr *rhs) : CompoundExpr(op,rhs) {}
    const char *GetPrintNameForNode() { return "LogicalExpr"; }
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env);
    Location *GetMemLocation(CodeGenerator *cg);
};

class AssignExpr : public CompoundExpr 
{
  public:
    AssignExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    const char *GetPrintNameForNode() { return "AssignExpr"; }
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env);
    int Emit(CodeGenerator *cg);
};

class LValue : public Expr 
{
  public:
    LValue(yyltype loc) : Expr(loc) {}
    Type *CheckType(EnvVector *env) { return NULL; }
};

class This : public Expr 
{
  public:
    This(yyltype loc) : Expr(loc) {}
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env);
    Decl *GetClass();
    Location *GetMemLocation(CodeGenerator *cg);
};

class ArrayAccess : public LValue 
{
  protected:
    Expr *base, *subscript;
    
  public:
    ArrayAccess(yyltype loc, Expr *base, Expr *subscript);
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env);
    Location *GetMemLocation(CodeGenerator *cg);
    Location *GetPtrLocation(CodeGenerator *cg);
};

/* Note that field access is used both for qualified names
 * base.field and just field without qualification. We don't
 * know for sure whether there is an implicit "this." in
 * front until later on, so we use one node type for either
 * and sort it out later. */
class FieldAccess : public LValue 
{
  protected:
    Expr *base;	// will be NULL if no explicit base
    Identifier *field;
    ClassDecl *GetClassFromImplicitThis();
    
  public:
    FieldAccess(Expr *base, Identifier *field); //ok to pass NULL base
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env);
    char *GetFieldName() { return field->getName(); }
    Location *GetMemLocation(CodeGenerator *cg);
    int GetOffset() { return GetClassFromImplicitThis()->GetVarOffset(GetFieldName()); }
    bool IsMemberVariable();
};

/* Like field access, call is used both for qualified base.field()
 * and unqualified field().  We won't figure out until later
 * whether we need implicit "this." so we use one node type for either
 * and sort it out later. */
class Call : public Expr 
{
  protected:
    Expr *base;	// will be NULL if no explicit base
    Identifier *field;
    List<Expr*> *actuals;
    ClassDecl * GetClassFromImplicitThis();
    bool isArrLength();
    
  public:
    Call(yyltype loc, Expr *base, Identifier *field, List<Expr*> *args);
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env);
    Expr *GetBase() { return base; }
    char *GetFieldName() { return field->getName(); }
    Location *GetMemLocation(CodeGenerator *cg);
    int Emit(CodeGenerator *cg);
};

class NewExpr : public Expr
{
  protected:
    NamedType *cType;
    
  public:
    NewExpr(yyltype loc, NamedType *clsType);
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env);
    Location *GetMemLocation(CodeGenerator *cg);
};

class NewArrayExpr : public Expr
{
  protected:
    Expr *size;
    Type *elemType;
    
  public:
    NewArrayExpr(yyltype loc, Expr *sizeExpr, Type *elemType);
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env);
    Location *GetMemLocation(CodeGenerator *cg);
};

class ReadIntegerExpr : public Expr
{
  public:
    ReadIntegerExpr(yyltype loc) : Expr(loc) {}
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env) { return Type::intType; }
    Location *GetMemLocation(CodeGenerator *cg);
};

class ReadLineExpr : public Expr
{
  public:
    ReadLineExpr(yyltype loc) : Expr (loc) {}
    void Check(EnvVector *env) {;}
    void Check();
    Type *CheckType(EnvVector *env) { return Type::stringType; }
    Location *GetMemLocation(CodeGenerator *cg);
};

    
#endif

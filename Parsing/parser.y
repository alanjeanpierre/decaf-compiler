/* File: parser.y
 * --------------
 * Yacc input file to generate the parser for the compiler.
 *
 * pp2: your job is to write a parser that will construct the parse tree
 *      and if no parse errors were found, print it.  The parser should 
 *      accept the language as described in specification, and as augmented 
 *      in the pp2 handout.
 */

%{

/* Just like lex, the text within this first region delimited by %{ and %}
 * is assumed to be C/C++ code and will be copied verbatim to the y.tab.c
 * file ahead of the definitions of the yyparse() function. Add other header
 * file inclusions or C++ variable declarations/prototypes that are needed
 * by your code here.
 */
#include "scanner.h" // for yylex
#include "parser.h"
#include "errors.h"

void yyerror(const char *msg); // standard error-handling routine

%}

/* The section before the first %% is the Definitions section of the yacc
 * input file. Here is where you declare tokens and types, add precedence
 * and associativity options, and so on.
 */
 
/* yylval 
 * ------
 * Here we define the type of the yylval global variable that is used by
 * the scanner to store attibute information about the token just scanned
 * and thus communicate that information to the parser. 
 *
 * pp2: You will need to add new fields to this union as you add different 
 *      attributes to your non-terminal symbols.
 */
%union {
    int integerConstant;
    bool boolConstant;
    char *stringConstant;
    double doubleConstant;
    char identifier[MaxIdentLen+1]; // +1 for terminating null
    Decl *decl;
    List<Decl*> *declList;
    VarDecl *var;
    FnDecl  *func;
    Type *T;
    List<VarDecl*> *formallist;
    List<VarDecl*> *formals;
    List<VarDecl*> *vlist;
    List<Stmt*>    *stmtlist;
    StmtBlock *stmtblck;
    Stmt      *stmt;
    VarDecl *v;
    Expr *expr;
    ConditionalStmt *cond;
    IfStmt *ifstmt;
    WhileStmt *whilestmt;
    ForStmt *forstmt;
    LValue *L;
    ArrayAccess *arraccess;
    FieldAccess *field;
    Identifier *id;
    Expr *constant;
    Expr *opt;
    List<Expr*> *exprlist;
    ClassDecl *classdecl;
    List<Decl*> *flist;
    Decl *classfield;   
    NamedType *ext;
    List<NamedType*> *implList;   
    List<NamedType*> *impl;
    Call *call;
    List<Expr*> *actList;
    List<Expr*> *act;
    InterfaceDecl *interfacedecl;
    List<Decl*> *intfield;

}


/* Tokens
 * ------
 * Here we tell yacc about all the token types that we are using.
 * Yacc will assign unique numbers to these and export the #define
 * in the generated y.tab.h header file.
 */
%token   T_Void T_Bool T_Int T_Double T_String T_Class 
%token   T_LessEqual T_GreaterEqual T_Equal T_NotEqual T_Dims
%token   T_And T_Or T_Null T_Extends T_This T_Interface T_Implements
%token   T_While T_For T_If T_Else T_Return T_Break
%token   T_New T_NewArray T_Print T_ReadInteger T_ReadLine

%token   <identifier> T_Identifier
%token   <stringConstant> T_StringConstant 
%token   <integerConstant> T_IntConstant
%token   <doubleConstant> T_DoubleConstant
%token   <boolConstant> T_BoolConstant

%precedence NOELSE
%precedence IFELSE
%left '='
%left T_Or
%left T_And
%left T_Equal T_NotEqual
%nonassoc '<' '>' T_GreaterEqual T_LessEqual
%left '+' '-'
%left '*' '/' '%'
%right '!'
%nonassoc '[' '.'





/* Non-terminal types
 * ------------------
 * In order for yacc to assign/access the correct field of $$, $1, we
 * must to declare which field is appropriate for the non-terminal.
 * As an example, this first type declaration establishes that the DeclList
 * non-terminal uses the field named "declList" in the yylval union. This
 * means that when we are setting $$ for a reduction for DeclList ore reading
 * $n which corresponds to a DeclList nonterminal we are accessing the field
 * of the union named "declList" which is of type List<Decl*>.
 * pp2: You'll need to add many of these of your own.
 */
%type <declList>  DeclList 
%type <decl>      Decl
%type <var>       VarDecl
%type <func>      FnDecl
%type <T>         Type
%type <v>         Variable
%type <vlist>     VarDeclList
%type <formals>   Formals
%type <stmtblck>  StmtBlck
%type <formallist> FormalList
%type <stmt>      Stmt
%type <stmtlist>  StmtList
%type <expr>      Expr
%type <cond>      CondStmt
%type <ifstmt>        IfStmt
%type <forstmt>       ForStmt
%type <whilestmt>     WhileStmt
%type <L>         LValue
%type <arraccess> ArrayAccess
%type <field>     FieldAccess
%type <id>         ID
%type <constant>        Constant
%type <opt>      OptionalExpr
%type <exprlist> ExprList
%type <classdecl> ClassDecl
%type <flist>     FieldList
%type <classfield>     Field
%type <ext>       Ext
%type <impl>      Impl
%type <implList>  ImplList
%type <call>      Call
%type <actList>   ActualList
%type <act>       Actuals
%type <interfacedecl> InterfaceDecl
%type <intfield>  InterfaceField


%%
/* Rules
 * -----
 * All productions and actions should be placed between the start and stop
 * %% markers which delimit the Rules section.
	 
 */
Program   :    DeclList            { 
                                      @1; 
                                      /* pp2: The @1 is needed to convince 
                                       * yacc to set up yylloc. You can remove 
                                       * it once you have other uses of @n*/
                                      Program *program = new Program($1);
                                      // if no errors, advance to next phase
                                      if (ReportError::NumErrors() == 0) 
                                          program->Print(0);
                                    }
          ;

DeclList  :    DeclList Decl        { ($$=$1)->Append($2); }
          |    Decl                 { ($$ = new List<Decl*>)->Append($1); }
          ;

Decl      :    VarDecl               { $$ = $1;}
          |    FnDecl                { $$ = $1; }
          |    ClassDecl             { $$ = $1; }
          |    InterfaceDecl         { $$ = $1; }
	            ;

VarDeclList :  VarDeclList VarDecl { ($$ = $1)->Append($2);}
            |  VarDecl         { ($$ = new List<VarDecl*>)->Append($1);}
          
VarDecl   :    Variable ';' { $$ = $1; }
            ;

Variable  :    Type ID { $$ = new VarDecl( $2, $1);}


Type      :    T_Int {$$ = Type::intType ;}
          |    T_Double {$$ = Type::doubleType ;}
          |    T_Bool {$$ = Type::boolType ;}
          |    T_String {$$ = Type::stringType ;}
          |    Type T_Dims { $$ = new ArrayType(@1, $1);}
          |    ID { $$ = new NamedType($1);}
          ;

FnDecl    :    Type ID '(' Formals ')' StmtBlck { $$ = new FnDecl($2, $1, $4);
                                                            $$->SetFunctionBody($6);
                                                            }
          |    T_Void ID '(' Formals ')' StmtBlck { $$ = new FnDecl($2, Type::voidType, $4);
                                                            $$->SetFunctionBody($6);
                                                            }
          ;

Formals   :    FormalList   { $$ = $1; }
          |    /* empty */ {$$ = new List<VarDecl*>;}
          ;

FormalList   :    FormalList ',' Variable { ($$=$1)->Append($3);}
             |    Variable { ($$ = new List<VarDecl*>)->Append($1);}
             ;

ClassDecl : T_Class ID Ext Impl '{' FieldList '}' {$$ = new ClassDecl($2, $3, $4, $6);}

Ext       : T_Extends ID { $$ = new NamedType($2);}
          | /* empty */ { $$ = NULL; }
          ;

Impl      : T_Implements ImplList { $$ = $2; }
          | /* empty */ { $$ = new List<NamedType*>; }
          ;

ImplList  : ImplList ',' ID { ($$=$1)->Append(new NamedType($3));}
          | ID {($$=new List<NamedType*>)->Append(new NamedType($1));}
          ;

FieldList : FieldList Field { ($$ = $1)->Append($2);}
          | /* empty */ { $$ = new List<Decl*>;}
          ;

Field     : VarDecl { $$ = $1; }
          | FnDecl {$$ = $1;}
          ;

InterfaceDecl : T_Interface ID '{' InterfaceField '}' {$$ = new InterfaceDecl($2, $4);}

InterfaceField : InterfaceField Type ID '(' Formals ')' ';' { ($$=$1)->Append(new FnDecl($3, $2, $5)); }
               | InterfaceField T_Void ID '(' Formals ')' ';' { ($$=$1)->Append(new FnDecl($3, Type::voidType, $5)); }
               | /* empty */ { $$ = new List<Decl*>;}


StmtBlck  :   '{' VarDeclList StmtList '}' { $$ = new StmtBlock($2, $3);}
          |   '{' StmtList '}' {$$ = new StmtBlock(new List<VarDecl*>, $2); }
          |   '{' VarDeclList '}' { $$ = new StmtBlock($2, new List<Stmt*>);}
          |   '{' '}' { $$ = new StmtBlock(new List<VarDecl*>, new List<Stmt*>);}

StmtList  :  StmtList Stmt { ($$ = $1)->Append($2);}
          |  Stmt { ($$ = new List<Stmt*>)->Append($1);}

Stmt      :   OptionalExpr ';' { $$ = $1;}
          |   CondStmt { $$ = $1;}
          |   StmtBlck { $$ = $1;}
          |   T_Break ';' { $$ = new BreakStmt(@1);}
          |   T_Return OptionalExpr ';' {$$ = new ReturnStmt(@1, $2);}
          |   T_Print '(' ExprList ')' ';' { $$ = new PrintStmt($3); }
          ;

CondStmt  :   IfStmt   { $$ = $1;}
          |   WhileStmt {$$ = $1;}
          |   ForStmt  { $$ = $1;}
          ;

IfStmt    :   T_If '(' Expr ')' Stmt T_Else Stmt %prec IFELSE { $$ = new IfStmt($3, $5, $7);}
          |   T_If '(' Expr ')' Stmt %prec NOELSE {$$ = new IfStmt($3, $5, NULL); }
          ;


WhileStmt :   T_While '(' Expr ')' Stmt {$$ = new WhileStmt($3, $5);}
            ;

ForStmt   :   T_For '(' OptionalExpr ';' Expr ';' Expr ')' Stmt {$$ = new ForStmt($3, $5, $7, $9);}
            ;

ExprList  :  ExprList ',' Expr { ($$=$1)->Append($3);}
          |  Expr { ($$ = new List<Expr*>)->Append($1);}

OptionalExpr : /* empty */ { $$ = new EmptyExpr();}
             | Expr { $$ = $1;}
             ;

Expr      :  LValue '=' Expr { $$ = new AssignExpr($1, new Operator(@2, "="), $3);}
          |  Constant { $$ = $1;}   
          |  Call { $$ = $1; }  
          |  LValue { $$ = $1;}
          |  Expr T_Or Expr { $$ = new LogicalExpr($1, new Operator(@2, "||"), $3);}
          |  Expr T_And Expr { $$ = new LogicalExpr($1, new Operator(@2, "&&"), $3);}
          |  Expr T_Equal Expr { $$ = new EqualityExpr($1, new Operator(@2, "=="), $3);}
          |  Expr T_NotEqual Expr { $$ = new EqualityExpr($1, new Operator(@2, "!="), $3);}
          |  Expr '>' Expr { $$ = new RelationalExpr($1, new Operator(@2, ">"), $3);}
          |  Expr '<' Expr { $$ = new RelationalExpr($1, new Operator(@2, "<"), $3);}
          |  Expr T_GreaterEqual Expr { $$ = new RelationalExpr($1, new Operator(@2, ">="), $3);}
          |  Expr T_LessEqual Expr {$$ = new RelationalExpr($1, new Operator(@2, "<="), $3);}
          |  Expr '+' Expr { $$ = new ArithmeticExpr($1, new Operator(@2, "+"), $3);}
          |  Expr '-' Expr { $$ = new ArithmeticExpr($1, new Operator(@2, "-"), $3);}
          |  Expr '*' Expr { $$ = new ArithmeticExpr($1, new Operator(@2, "*"), $3) ;}
          |  Expr '/' Expr { $$ = new ArithmeticExpr($1, new Operator(@2, "/"), $3);}
          |  Expr '%' Expr { $$ = new ArithmeticExpr($1, new Operator(@2, "%"), $3);}
          |  T_New '(' ID ')' { $$ = new NewExpr(@1, new NamedType($3));}
          |  '(' Expr ')' { $$ = $2;}
          |  T_ReadInteger '(' ')' { $$ = new ReadIntegerExpr(@1);}
          |  T_NewArray '(' Expr ',' Type ')' { $$ = new NewArrayExpr(@1, $3, $5);}
          | '!' Expr { $$ = new LogicalExpr(new Operator(@1, "!"), $2);}
          | T_This { $$ = new This(@1);}
          ;


Call      :  Expr '.' ID '(' Actuals ')' { $$ = new Call(@1, $1, $3, $5);}
          |  ID '(' Actuals ')' { $$ = new Call(@1, NULL, $1, $3);}
          ;

Actuals   :  ActualList { $$ = $1; }
          |  /* empty */ { $$ = new List<Expr*>;}
          ;

ActualList : ActualList ',' Expr { ($$ = $1)->Append($3);}
           | Expr { ($$ = new List<Expr*>)->Append($1);}

LValue    :  ArrayAccess { $$ = $1; }
          |  FieldAccess { $$ = $1; }
          ;

ArrayAccess : Expr '[' Expr ']' { $$ = new ArrayAccess(@1, $1, $3);}

FieldAccess : Expr '.' ID { $$ = new FieldAccess($1, $3);}
            | ID { $$ = new FieldAccess(NULL, $1);}
            ;

Constant  : T_IntConstant { $$ = new IntConstant(@1, $1);}
          | T_DoubleConstant { $$ = new DoubleConstant(@1, $1);}
          | T_BoolConstant { $$ = new BoolConstant(@1, $1);}
          | T_StringConstant { $$ = new StringConstant(@1, $1);}
          | T_Null { $$ = new NullConstant(@1);}
          ;


ID        : T_Identifier { $$ = new Identifier(@1, $1);}



%%

/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated y.tab.c file.
 * This section is where you put definitions of helper functions.
 */

/* Function: InitParser
 * --------------------
 * This function will be called before any calls to yyparse().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the parser (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yydebug that controls whether yacc prints debugging information about
 * parser actions (shift/reduce) and contents of state stack during parser.
 * If set to false, no information is printed. Setting it to true will give
 * you a running trail that might be helpful when debugging your parser.
 * Please be sure the variable is set to false when submitting your final
 * version.
 */
void InitParser()
{
   PrintDebug("parser", "Initializing parser");
   yydebug = false;
}

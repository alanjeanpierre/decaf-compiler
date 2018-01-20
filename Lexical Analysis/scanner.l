/* File:  scanner.l
 * ----------------
 * Lex inupt file to generate the scanner for the compiler.
 */

%{

/* The text within this first region delimited by %{ and %} is assumed to
 * be C/C++ code and will be copied verbatim to the lex.yy.c file ahead
 * of the definitions of the yylex() function. Add other header file inclusions
 * or C++ variable declarations/prototypes that are needed by your code here.
 */

#include <string.h>
#include "scanner.h"
#include "utility.h" // for PrintDebug()
#include "errors.h"

/* Global variable: yylval
 * -----------------------
 * This global variable is how we get attribute information about the token
 * just scanned to the client. The scanner sets the global variable
 * appropriately and since it's global the client can just read it.  In the
 * future, this variable will be declared for us in the y.tab.c file
 * produced by Yacc, but for now, we declare it manually.
 */
YYSTYPE yylval;  // manually declared for pp1, later Yacc provides

/* Global variable: yylloc
 * -----------------------
 * This global variable is how we get position information about the token
 * just scanned to the client. (Operates similarly to yylval above)
 */
struct yyltype yylloc; // manually dclared for pp1, later Yacc provides

/* Macro: YY_USER_ACTION 
 * ---------------------
 * This flex built-in macro can be defined to provide an action which is
 * always executed prior to any matched rule's action. Basically, it is
 * a way of having a piece of code common to all actions factored out to
 * this routine.  We already defined it for you and left the empty
 * function DoBeforeEachAction ready for your use as needed. It will
 * be called once for each pattern scanned from the file, before
 * executing its action.
 */
static void DoBeforeEachAction(); 
#define YY_USER_ACTION DoBeforeEachAction();

%}

 /* The section before the first %% is the Definitions section of the lex
  * input file. Here is where you set options for the scanner, define lex
  * states, and can set up definitions to give names to regular expressions
  * as a simple substitution mechanism that allows for more readable
  * entries in the Rules section later. 
  */

DIGIT [0-9]
ALPHA [a-zA-Z]
APLHANUM  [a-zA-Z0-9]


%%             /* BEGIN RULES SECTION */
 /* All patterns and actions should be placed between the start and stop
  * %% markers which delimit the Rules section. 
  */ 
  
  /* Keywords */
  
  /***********************************
  *** Data Types ***
  ***********************************/
void        {
                printf("Found void keyword: %s\n", yytext); 
                return T_Void;
            }
            
int         {
                printf("Found int keyword: %s\n", yytext); 
                return T_Int;
            }
            
double      {
                printf("Found double keyword: %s\n", yytext); 
                return T_Double;
            }
            
bool        {
                printf("Found bool keyword: %s\n", yytext); 
                return T_Bool;
            }
            
null        {   
                printf("Found null keyword: %s\n", yytext); 
                return T_Null;
            }
            
string      {
                printf("Found string keyword: %s\n", yytext); 
                return T_String;
            }

            
            
  /***********************************
  *** Control Flow ***
  ***********************************/
if          {
                printf("Found if keyword: %s\n", yytext); 
                return T_If;
            }
            
else        {
                printf("Found else keyword: %s\n", yytext); 
                return T_Else;
            }
            
while       {
                printf("found while keyword: %s\n", yytext); 
                return T_While;
            }
            
for         {
                printf("Found for keyword: %s\n", yytext); 
                return T_For;
            }
            
return      {   
                printf("Found return keyword: %s\n", yytext); 
                return T_Return;
            }            
            
break       {
                printf("Found break keyword: %s\n", yytext); 
                return T_Break;
            }

            
            
  /***********************************
  *** OO ***
  ***********************************/
              
New         {
                printf("Found new keyword: %s\n", yytext); 
                return T_New;
            }
NewArray    {
                printf("Found NewArray keyword: %s\n", yytext); 
                return T_NewArray;
            }
            
class       {
                printf("Found class keyword: %s\n", yytext);
                return T_Class;
            }
            
extends     {
                printf("Found extends keyword: %s\n", yytext); 
                return T_Extends;
            }
            
this        {
                printf("Found this keyword: %s\n", yytext); 
                return T_This;
            }

            
interface   {
                printf("Found interface keyword: %s\n", yytext); 
                return T_Interface;
            }
            
implements  {
                printf("Found implements keyword: %s\n", yytext); 
                return T_Implements;
            }
            


 /* operators */
"+"         {
                printf("Found + operator: %s\n", yytext);
                return '+';
            }
            
"-"         {
                printf("Found - operator: %s\n", yytext);
                return '-';
            }
            
"*"         {   
                printf("Found * operator: %s\n", yytext);
                return '*';
            }
            
"/"         {
                printf("Found / operator: %s\n", yytext);
                return '/';
            }
            
"%"         {   
                printf("Found %% operator: %s\n", yytext);
                return '%';
            }
            
"<"         {
                printf("Found < operator: %s\n", yytext);
                return '%';
            }
            
"<="        {
                printf("Found <= operator: %s\n", yytext);
                return T_LessEqual;
            }
            
">"         {
                printf("Found > operator: %s\n", yytext);
                return '>';
            }
            
">="        {
                printf("Found >= operator: %s\n", yytext);
                return T_GreaterEqual;
            }
            
"="         {
                printf("Found = operator: %s\n", yytext);
                return '=';
            }
            
"=="        {   
                printf("Found == operator: %s\n", yytext);
                return T_Equal;
            }
            
"!="        {
                printf("Found != operator: %s\n", yytext);
                return T_NotEqual;
            }
            
"&&"        {
                printf("Found && operator: %s\n", yytext);
                return T_And;
            }
            
"||"        {
                printf("Found || operator: %s\n", yytext);
                return T_Or;
            }
            
"!"         {
                printf("Found ! operator: %s\n", yytext);
                return '!';
            }
            
";"         {   
                printf("Found ; operator: %s\n", yytext);
                return ';';
            }
            
","         {
                printf("Found , operator: %s\n", yytext);
                return ',';
            }
            
"."         {
                printf("Found . operator: %s\n", yytext);
                return '.';
            }
            
"["         {
                printf("Found [ operator: %s\n", yytext);
                return '[';
            }
            
"]"         {
                printf("Found ] operator: %s\n", yytext);
                return ']';
            }
            
"("         {
                printf("Found ( operator: %s\n", yytext);
                return '(';
            }
            
")"         {
                printf("Found ) operator: %s\n", yytext);
                return ')';
            }
            
"{"         {
                printf("Found { operator: %s\n", yytext);
                return '{';
            }
            
"}"         {
                printf("Found } operator: %s\n", yytext);
                return '}';
            }
            
 /* Integer */  
{DIGIT}+    {
                printf("Found an int: %s (%d)\n", yytext, atoi(yytext));
                yylval.integerConstant = atoi(yytext);
                return T_IntConstant;
            }
            
    /* Hexadecimal */
0[xX][0-9a-fA-F]+   {
                printf("found some hex: %s\n", yytext);
                yylval.integerConstant = 0;
                return T_IntConstant;
            }
    /* Floataing point */
{DIGIT}+.{DIGIT}*   {
                printf("found a float: %s (%f)\n", yytext, atof(yytext));
                yylval.doubleConstant = atof(yytext);
                return T_DoubleConstant;
            }
    /* Exponent float */
{DIGIT}+.{DIGIT}*E[+\-]{DIGIT}+     {
                printf("found an exponent: %s\n", yytext);
                yylval.doubleConstant = 0;
                return T_DoubleConstant;
            }
    /* Identifier */
{ALPHA}+    {
                printf("Found an identifier: %s\n", yytext);
                strncpy(yylval.identifier, yytext, 31);
                return T_Identifier;
            }

    /* Whitespace */
[ \t]+ /* do nothing */ { ; }

\n          {
                yylloc.first_line += 1;
                yylloc.first_column = 0;
                yylloc.last_column = 0;
            }

.           { 
                printf("Invalid character: %s\n", yytext);
            }
  

%%
/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated lex.yy.c file.
 * This section is where you put definitions of helper functions.
 */


/* Function: InitScanner
 * ---------------------
 * This function will be called before any calls to yylex().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the scanner (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yy_flex_debug that controls whether flex prints debugging information
 * about each token and what rule was matched. If set to false, no information
 * is printed. Setting it to true will give you a running trail that might
 * be helpful when debugging your scanner. Please be sure the variable is
 * set to false when submitting your final version.
 */
void InitScanner()
{
    PrintDebug("lex", "Initializing scanner");
    yy_flex_debug = false;
}


/* Function: DoBeforeEachAction()
 * ------------------------------
 * This function is installed as the YY_USER_ACTION. This is a place
 * to group code common to all actions.
 */
static void DoBeforeEachAction()
{
    yylloc.first_column = yylloc.last_column;
    yylloc.last_column += strlen(yytext);
}

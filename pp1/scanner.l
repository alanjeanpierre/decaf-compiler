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
#include <math.h>
#include <limits.h>

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
int parse_hex(char *str);
int hex2int(char c);
int intpow(int base, int exponent);
double parse_exponent(char *str);
int parse_int(char *str, int base, YYSTYPE *value);
int parse_double(char *str, YYSTYPE *value);

%}

 /* The section before the first %% is the Definitions section of the lex
  * input file. Here is where you set options for the scanner, define lex
  * states, and can set up definitions to give names to regular expressions
  * as a simple substitution mechanism that allows for more readable
  * entries in the Rules section later. 
  */

DIGIT [0-9]
ALPHA [a-zA-Z]
ALPHANUM  [a-zA-Z0-9]


%%             /* BEGIN RULES SECTION */
 /* All patterns and actions should be placed between the start and stop
  * %% markers which delimit the Rules section. 
  */ 
  
  /* Keywords */
  
  /***********************************
  *** Data Types ***
  ***********************************/
void        {
                return T_Void;
            }
            
int         {
                return T_Int;
            }
            
double      {
                return T_Double;
            }
            
bool        {
                return T_Bool;
            }
            
null        {   
                return T_Null;
            }
            
string      {
                return T_String;
            }

            
            
  /***********************************
  *** Control Flow ***
  ***********************************/
if          {
                return T_If;
            }
            
else        {
                return T_Else;
            }
            
while       {
                return T_While;
            }
            
for         {
                return T_For;
            }
            
return      {   
                return T_Return;
            }            
            
break       {
                return T_Break;
            }

            
            
  /***********************************
  *** OO ***
  ***********************************/
              
New         {
                return T_New;
            }
            
NewArray    {
                return T_NewArray;
            }
            
class       {
                return T_Class;
            }
            
extends     {
                return T_Extends;
            }
            
this        {
                return T_This;
            }

            
interface   {
                return T_Interface;
            }
            
implements  {
                return T_Implements;
            }
            


 /* operators */
"+"         {
                return '+';
            }
            
"-"         {
                return '-';
            }
            
"*"         {   
                return '*';
            }
            
"/"         {
                return '/';
            }
            
"%"         {   
                return '%';
            }
            
"<"         {
                return '<';
            }
            
"<="        {
                return T_LessEqual;
            }
            
">"         {
                return '>';
            }
            
">="        {
                return T_GreaterEqual;
            }
            
"="         {
                return '=';
            }
            
"=="        {   
                return T_Equal;
            }
            
"!="        {
                return T_NotEqual;
            }
            
"&&"        {
                return T_And;
            }
            
"||"        {
                return T_Or;
            }
            
"!"         {
                return '!';
            }
            
";"         {   
                return ';';
            }
            
","         {
                return ',';
            }
            
"."         {
                return '.';
            }
            
"["         {
                return '[';
            }
            
"]"         {
                return ']';
            }
            
"("         {
                return '(';
            }
            
")"         {
                return ')';
            }
            
"{"         {
                return '{';
            }
            
"}"         {
                return '}';
            }

 /* Boolean */
 
true        {
                yylval.boolConstant = true;
                return T_BoolConstant;
            }

false       {
                yylval.boolConstant = false;
                return T_BoolConstant;
            }

            
 /* Integer */  
{DIGIT}+    {
                int rt = parse_int(yytext, 10, &yylval);
                if (rt != 0)
                    ReportError::Formatted(&yylloc, "Integer Overflow: %s", yytext);
                //yylval.integerConstant = parse_int(yytext);
                //yylval.integerConstant = atoi(yytext);
                return T_IntConstant;
            }
            
    /* Hexadecimal */
0[xX][0-9a-fA-F]+   {
                //yylval.integerConstant = parse_hex(yytext);
                int rt = parse_int(yytext, 16, &yylval);
                if (rt != 0)
                    ReportError::Formatted(&yylloc, "Integer Constant Overflow: %s", yytext);
                return T_IntConstant;
            }
    /* Floataing point */
{DIGIT}+\.{DIGIT}*   {
                int rt = parse_double(yytext, &yylval);
                if (rt != 0)
                    ReportError::Formatted(&yylloc, "Double Constant \"Overflow\": %s", yytext);
                //yylval.doubleConstant = atof(yytext);
                return T_DoubleConstant;
            }
    /* Exponent float */
[\+\-]?{DIGIT}+\.{DIGIT}*[eE][\+\-]?{DIGIT}+     {
                //yylval.doubleConstant = parse_exponent(yytext);
                
                int rt = parse_double(yytext, &yylval);
                if (rt != 0)
                    ReportError::Formatted(&yylloc, "Double \"overflow\": %s", yytext);
                return T_DoubleConstant;
            }
            
    /* String literal */

\"[^"\n]*\" {
                yylval.stringConstant = yytext;
                return T_StringConstant;
            }
    /* missing terminator string literal */
\"[^"\n]* {
                ReportError::UntermString(&yylloc, yytext);
            }
    
    /* Identifier */
({ALPHA}|_)({ALPHANUM}|_)*   {
                if (strlen(yytext) > 31)
                    ReportError::LongIdentifier(&yylloc, yytext);
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
                //printf("Invalid character: %s\n", yytext);
                ReportError::UnrecogChar(&yylloc, yytext[0]);
            }
  

%%
/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated lex.yy.c file.
 * This section is where you put definitions of helper functions.
 */
 
 /* Function: parse_int
  * -------------------
  * This function parses ints, with error checking
  */
int parse_int(char *str, int base, YYSTYPE *value) {

    errno = 0;
    char *endptr;
    int rt = 0;
    long tmp = strtol(yytext, &endptr, base);
    if (endptr == yytext || *endptr != '\0' 
        || ( errno == ERANGE && (tmp == LONG_MIN || tmp == LONG_MAX)) )
        rt = -1;
    
    value->integerConstant = (int)tmp;
    if ((long)value->integerConstant != tmp)
        rt = -1;
    
    return rt;
    

}

int parse_double(char *str, YYSTYPE *value) {
    errno = 0;
    char *endptr;
    int rt = 0;
    double tmp = strtod(yytext, &endptr);
    if (endptr == yytext || *endptr != '\0'
        || ( errno == ERANGE && (tmp == HUGE_VAL || tmp == -HUGE_VAL)) )
        rt = -1;
    value->doubleConstant = tmp;
    
    return rt;

}

 /* Function: parse_hex
  * -------------------
  * This function reads a hex string and returns its int value
  */
int parse_hex(char *str) {
    int length = strlen(str);
    int num = 0;
    for (int i = 0; i < length-2; i++) {
        int digit = hex2int(str[length-i-1]);
        num += digit * intpow(16, i);
    }
    return num;
}

/* Function: hex2int
 * -----------------
 * Maps hex characters to integers
 */
int hex2int(char c) {
    switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return c-'0';
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        default:
            return 0;    
    }
}

/* Function: intpow
 * ----------------
 * Returns integer exonentiation, rather than floating in math.h
 */
int intpow(int base, int exponent) {
    if (exponent > 0)
        return base * intpow(base, exponent-1);
    return 1;
}

/* Function: parse_exponent
 * ------------------------
 * This function reads an exponent float and returns its FP value
 */
double parse_exponent(char *str) {
    return atof(str);
}

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
    yylloc.first_line = 1;
    //yylloc.first_column = 1;
    //yylloc.last_column = 1;
}


/* Function: DoBeforeEachAction()
 * ------------------------------
 * This function is installed as the YY_USER_ACTION. This is a place
 * to group code common to all actions.
 */
static void DoBeforeEachAction()
{
    //printf("Found something: %s\n", yytext);
    yylloc.first_column = yylloc.last_column + 1;
    yylloc.last_column += strlen(yytext);
}
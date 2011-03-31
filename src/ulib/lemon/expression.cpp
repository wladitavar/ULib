/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#line 1 "expression.y"

#include <ulib/tokenizer.h>
#include <ulib/dynamic/dynamic.h>
#include <ulib/utility/string_ext.h>

typedef long (*lPFv)(void);
typedef long (*lPFll)(long, long);

typedef UVector<UString> Items;

extern void* expressionParserAlloc(void* (*mallocProc)(size_t));
extern void  expressionParserFree(void* p, void (*freeProc)(void*));
extern void  expressionParserTrace(FILE* stream, char* zPrefix);
extern void  expressionParser(void* yyp, int yymajor, UString* yyminor, UString* result);

extern void token_destructor(UString* token);

void token_destructor(UString* token) {
   U_TRACE(0, "::token_destructor(%.*S)", U_STRING_TO_TRACE(*token))

   delete token;
}

#line 35 "expression.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/*
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands.
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    expressionParserTOKENTYPE     is the data type used for minor tokens given
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is expressionParserTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    expressionParserARG_SDECL     A static variable declaration for the %extra_argument
**    expressionParserARG_PDECL     A parameter declaration for the %extra_argument
**    expressionParserARG_STORE     Code to store %extra_argument into yypParser
**    expressionParserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 40
#define YYACTIONTYPE unsigned char
#define expressionParserTOKENTYPE  UString* 
typedef union {
  expressionParserTOKENTYPE yy0;
  UString* yy13;
  int yy46;
  Items* yy65;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define expressionParserARG_SDECL  UString* result ;
#define expressionParserARG_PDECL , UString* result 
#define expressionParserARG_FETCH  UString* result  = yypParser->result 
#define expressionParserARG_STORE yypParser->result  = result 
#define YYNSTATE 50
#define YYNRULE 37
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* Next are that tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
static const YYACTIONTYPE yy_action[] = {
 /*     0 */    88,   12,    9,   11,   17,   15,   42,   43,   20,   14,
 /*    10 */     9,   11,   17,   15,   42,   43,   20,    8,   11,   17,
 /*    20 */    15,   42,   43,   20,   10,   17,   15,   42,   43,   20,
 /*    30 */    16,   15,   42,   43,   20,   28,   29,   13,   42,   43,
 /*    40 */    20,   30,   31,   32,    7,   21,    1,    3,   45,   33,
 /*    50 */    34,   35,   36,   23,   43,   20,   21,    1,    2,   45,
 /*    60 */    50,   27,   26,   39,   40,   41,   27,   26,   24,   20,
 /*    70 */    37,   38,   46,   49,   47,    4,   22,   89,   44,    6,
 /*    80 */    19,    5,   18,   48,   89,   25,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    30,   31,   32,   33,   34,   35,   36,   37,   38,   31,
 /*    10 */    32,   33,   34,   35,   36,   37,   38,   32,   33,   34,
 /*    20 */    35,   36,   37,   38,   33,   34,   35,   36,   37,   38,
 /*    30 */    34,   35,   36,   37,   38,    3,    4,   35,   36,   37,
 /*    40 */    38,    9,   10,   11,   17,   18,   19,   26,   21,    5,
 /*    50 */     6,    7,    8,   36,   37,   38,   18,   19,   25,   21,
 /*    60 */     0,    1,    2,   14,   15,   16,    1,    2,   37,   38,
 /*    70 */    12,   13,   20,   21,   20,   27,   22,   39,   21,   29,
 /*    80 */    24,   28,   19,   21,   39,   20,
};
#define YY_SHIFT_USE_DFLT (-1)
#define YY_SHIFT_MAX 22
static const signed char yy_shift_ofst[] = {
 /*     0 */    27,   27,   27,   27,   27,   27,   27,   38,   32,   32,
 /*    10 */    44,   44,   60,   49,   65,   49,   58,   58,   52,   54,
 /*    20 */    57,   63,   62,
};
#define YY_REDUCE_USE_DFLT (-31)
#define YY_REDUCE_MAX 18
static const signed char yy_reduce_ofst[] = {
 /*     0 */   -30,  -22,  -15,   -9,   -4,    2,   17,   31,   21,   21,
 /*    10 */    48,   48,   33,   50,   33,   50,   53,   53,   56,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */    87,   87,   87,   87,   87,   87,   87,   87,   51,   52,
 /*    10 */    53,   54,   87,   57,   87,   58,   55,   56,   87,   87,
 /*    20 */    64,   87,   87,   59,   61,   63,   67,   68,   69,   70,
 /*    30 */    71,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*    40 */    81,   82,   60,   62,   66,   65,   83,   84,   86,   85,
};
#define YY_SZ_ACTTAB (int)(sizeof(yy_action)/sizeof(yy_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
**
**      %fallback ID X Y Z.
**
** appears in the grammer, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  int stateno;       /* The state-number */
  int major;         /* The major token value.  This is the code
                     ** number for the token at this stack level */
  YYMINORTYPE minor; /* The user-supplied minor token value.  This
                     ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
  int yyerrcnt;                 /* Shifts left before out of the error */
  expressionParserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/*
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void expressionParserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = {
  "$",             "AND",           "OR",            "EQ",          
  "NE",            "GT",            "GE",            "LT",          
  "LE",            "STARTS_WITH",   "ENDS_WITH",     "CONTAINS",    
  "PLUS",          "MINUS",         "MULT",          "DIV",         
  "MOD",           "NOT",           "FN_CALL",       "LPAREN",      
  "RPAREN",        "VALUE",         "COMMA",         "error",       
  "params",        "booleanCond",   "equalityCond",  "relationalCond",
  "additiveCond",  "multiplicativeCond",  "input",         "booleanExpression",
  "equalityExpression",  "relationalExpression",  "additiveExpression",  "multiplicativeExpression",
  "unaryExpression",  "primaryExpression",  "value",       
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "input ::= booleanExpression",
 /*   1 */ "booleanExpression ::= booleanExpression booleanCond equalityExpression",
 /*   2 */ "booleanExpression ::= equalityExpression",
 /*   3 */ "equalityExpression ::= equalityExpression equalityCond relationalExpression",
 /*   4 */ "equalityExpression ::= relationalExpression",
 /*   5 */ "relationalExpression ::= relationalExpression relationalCond additiveExpression",
 /*   6 */ "relationalExpression ::= additiveExpression",
 /*   7 */ "additiveExpression ::= additiveExpression additiveCond multiplicativeExpression",
 /*   8 */ "additiveExpression ::= multiplicativeExpression",
 /*   9 */ "multiplicativeExpression ::= multiplicativeExpression multiplicativeCond unaryExpression",
 /*  10 */ "multiplicativeExpression ::= unaryExpression",
 /*  11 */ "unaryExpression ::= NOT primaryExpression",
 /*  12 */ "unaryExpression ::= primaryExpression",
 /*  13 */ "primaryExpression ::= LPAREN booleanExpression RPAREN",
 /*  14 */ "primaryExpression ::= value",
 /*  15 */ "value ::= VALUE",
 /*  16 */ "value ::= value VALUE",
 /*  17 */ "booleanCond ::= OR",
 /*  18 */ "booleanCond ::= AND",
 /*  19 */ "equalityCond ::= EQ",
 /*  20 */ "equalityCond ::= NE",
 /*  21 */ "equalityCond ::= STARTS_WITH",
 /*  22 */ "equalityCond ::= ENDS_WITH",
 /*  23 */ "equalityCond ::= CONTAINS",
 /*  24 */ "relationalCond ::= GT",
 /*  25 */ "relationalCond ::= GE",
 /*  26 */ "relationalCond ::= LT",
 /*  27 */ "relationalCond ::= LE",
 /*  28 */ "additiveCond ::= PLUS",
 /*  29 */ "additiveCond ::= MINUS",
 /*  30 */ "multiplicativeCond ::= MULT",
 /*  31 */ "multiplicativeCond ::= DIV",
 /*  32 */ "multiplicativeCond ::= MOD",
 /*  33 */ "primaryExpression ::= FN_CALL LPAREN RPAREN",
 /*  34 */ "primaryExpression ::= FN_CALL LPAREN params RPAREN",
 /*  35 */ "params ::= VALUE",
 /*  36 */ "params ::= params COMMA VALUE",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/*
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to expressionParser and expressionParserFree.
*/
void *expressionParserAlloc(void *(*mallocProc)(size_t)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (size_t)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#if YYSTACKDEPTH<=0
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(YYCODETYPE yymajor, YYMINORTYPE *yypminor){
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 1: /* AND */
    case 2: /* OR */
    case 3: /* EQ */
    case 4: /* NE */
    case 5: /* GT */
    case 6: /* GE */
    case 7: /* LT */
    case 8: /* LE */
    case 9: /* STARTS_WITH */
    case 10: /* ENDS_WITH */
    case 11: /* CONTAINS */
    case 12: /* PLUS */
    case 13: /* MINUS */
    case 14: /* MULT */
    case 15: /* DIV */
    case 16: /* MOD */
    case 17: /* NOT */
    case 18: /* FN_CALL */
    case 19: /* LPAREN */
    case 20: /* RPAREN */
    case 21: /* VALUE */
    case 22: /* COMMA */
#line 31 "expression.y"
{ token_destructor((yypminor->yy0)); }
#line 432 "expression.c"
      break;
    case 24: /* params */
#line 33 "expression.y"
{
   U_TRACE(0, "::params_destructor(%p)", (yypminor->yy65))
   delete (yypminor->yy65);
}
#line 440 "expression.c"
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  if( pParser->yyidx<0 ) return 0;
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor( yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/*
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from expressionParserAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void expressionParserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  if( pParser==0 ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;

  if( stateno>YY_SHIFT_MAX || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_SZ_ACTTAB || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      int iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( j>=0 && j<YY_SZ_ACTTAB && yy_lookahead[j]==YYWILDCARD ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  assert( stateno<=YY_REDUCE_MAX );
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  assert( i>=0 && i<YY_SZ_ACTTAB );
  assert( yy_lookahead[i]==iLookAhead );
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   expressionParserARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */
#line 55 "expression.y"

   U_TRACE(0, "::stack_overflow()")
   U_WARNING("Parse stack overflow");
#line 595 "expression.c"
   expressionParserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer ot the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#if YYSTACKDEPTH>0
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = yyNewState;
  yytos->major = yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 30, 1 },
  { 31, 3 },
  { 31, 1 },
  { 32, 3 },
  { 32, 1 },
  { 33, 3 },
  { 33, 1 },
  { 34, 3 },
  { 34, 1 },
  { 35, 3 },
  { 35, 1 },
  { 36, 2 },
  { 36, 1 },
  { 37, 3 },
  { 37, 1 },
  { 38, 1 },
  { 38, 2 },
  { 25, 1 },
  { 25, 1 },
  { 26, 1 },
  { 26, 1 },
  { 26, 1 },
  { 26, 1 },
  { 26, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 27, 1 },
  { 28, 1 },
  { 28, 1 },
  { 29, 1 },
  { 29, 1 },
  { 29, 1 },
  { 37, 3 },
  { 37, 4 },
  { 24, 1 },
  { 24, 3 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  expressionParserARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  memset(&yygotominor, 0, sizeof(yygotominor));


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 0: /* input ::= booleanExpression */
#line 78 "expression.y"
{
   U_TRACE(0, "input ::= booleanExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)
   U_INTERNAL_ASSERT_POINTER(result)

   U_INTERNAL_DUMP("yymsp[0].minor.yy13 = %.*S result = %.*S", U_STRING_TO_TRACE(*yymsp[0].minor.yy13), U_STRING_TO_TRACE(*result))

   *result = *yymsp[0].minor.yy13;

   delete yymsp[0].minor.yy13;
}
#line 751 "expression.c"
        break;
      case 1: /* booleanExpression ::= booleanExpression booleanCond equalityExpression */
#line 91 "expression.y"
{
   U_TRACE(0, "booleanExpression(yygotominor.yy13) ::= booleanExpression(yymsp[-2].minor.yy13) booleanCond(yymsp[-1].minor.yy46) equalityExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy13)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy13 = %.*S yymsp[-1].minor.yy46 = %d yymsp[0].minor.yy13 = %.*S", U_STRING_TO_TRACE(*yymsp[-2].minor.yy13), yymsp[-1].minor.yy46, U_STRING_TO_TRACE(*yymsp[0].minor.yy13))

   bool Bbo = (yymsp[-2].minor.yy13->empty() == false),
        Dbo = (yymsp[0].minor.yy13->empty() == false),
         bo = (yymsp[-1].minor.yy46 == U_TK_AND ? Bbo && Dbo
                             : Bbo || Dbo);

   yygotominor.yy13 = (bo ? U_NEW(UString(U_CONSTANT_TO_PARAM("true"))) : U_NEW(UString));

   delete yymsp[-2].minor.yy13;
   delete yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 775 "expression.c"
        break;
      case 2: /* booleanExpression ::= equalityExpression */
#line 112 "expression.y"
{
   U_TRACE(0, "booleanExpression(yygotominor.yy13) ::= equalityExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[0].minor.yy13 = %p", yygotominor.yy13, yymsp[0].minor.yy13)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   yygotominor.yy13 = yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 790 "expression.c"
        break;
      case 3: /* equalityExpression ::= equalityExpression equalityCond relationalExpression */
#line 124 "expression.y"
{
   U_TRACE(0, "equalityExpression(yygotominor.yy13) ::= equalityExpression(yymsp[-2].minor.yy13) equalityCond(yymsp[-1].minor.yy46) relationalExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy13)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy13 = %.*S yymsp[-1].minor.yy46 = %d yymsp[0].minor.yy13 = %.*S", U_STRING_TO_TRACE(*yymsp[-2].minor.yy13), yymsp[-1].minor.yy46, U_STRING_TO_TRACE(*yymsp[0].minor.yy13))

   bool bo = false,
       Bbo = (yymsp[-2].minor.yy13->empty() == false),
       Dbo = (yymsp[0].minor.yy13->empty() == false);

   int cmp = (Bbo && Dbo ? yymsp[-2].minor.yy13->compare(yymsp[0].minor.yy13->rep) : Bbo - Dbo);

   switch (yymsp[-1].minor.yy46)
      {
      case U_TK_EQ: bo = (cmp == 0); break;
      case U_TK_NE: bo = (cmp != 0); break;

      case U_TK_CONTAINS:     if (Bbo && Dbo) bo = (yymsp[-2].minor.yy13->find(*yymsp[0].minor.yy13) != U_NOT_FOUND);      break;
      case U_TK_ENDS_WITH:    if (Bbo && Dbo) bo = UStringExt::endsWith(  *yymsp[-2].minor.yy13, *yymsp[0].minor.yy13); break;
      case U_TK_STARTS_WITH:  if (Bbo && Dbo) bo = UStringExt::startsWith(*yymsp[-2].minor.yy13, *yymsp[0].minor.yy13); break;
      }

   U_INTERNAL_DUMP("bo = %b cmp = %d", bo, cmp)

   yygotominor.yy13 = (bo ? U_NEW(UString(U_CONSTANT_TO_PARAM("true"))) : U_NEW(UString));

   delete yymsp[-2].minor.yy13;
   delete yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 827 "expression.c"
        break;
      case 4: /* equalityExpression ::= relationalExpression */
#line 158 "expression.y"
{
   U_TRACE(0, "equalityExpression(yygotominor.yy13) ::= relationalExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[0].minor.yy13 = %p", yygotominor.yy13, yymsp[0].minor.yy13)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   yygotominor.yy13 = yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 842 "expression.c"
        break;
      case 5: /* relationalExpression ::= relationalExpression relationalCond additiveExpression */
#line 170 "expression.y"
{
   U_TRACE(0, "relationalExpression(yygotominor.yy13) ::= relationalExpression(yymsp[-2].minor.yy13) relationalCond(yymsp[-1].minor.yy46) additiveExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy13)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy13 = %.*S yymsp[-1].minor.yy46 = %d yymsp[0].minor.yy13 = %.*S", U_STRING_TO_TRACE(*yymsp[-2].minor.yy13), yymsp[-1].minor.yy46, U_STRING_TO_TRACE(*yymsp[0].minor.yy13))

   bool bo = false,
       Bbo = (yymsp[-2].minor.yy13->empty() == false),
       Dbo = (yymsp[0].minor.yy13->empty() == false);

   int cmp = (Bbo && Dbo ? yymsp[-2].minor.yy13->compare(yymsp[0].minor.yy13->rep) : Bbo - Dbo);

   switch (yymsp[-1].minor.yy46)
      {
      case U_TK_LT: bo = (cmp <  0); break;
      case U_TK_LE: bo = (cmp <= 0); break;
      case U_TK_GT: bo = (cmp >  0); break;
      case U_TK_GE: bo = (cmp >= 0); break;
      }

   U_INTERNAL_DUMP("bo = %b cmp = %d", bo, cmp)

   yygotominor.yy13 = (bo ? U_NEW(UString(U_CONSTANT_TO_PARAM("true"))) : U_NEW(UString));

   delete yymsp[-2].minor.yy13;
   delete yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 877 "expression.c"
        break;
      case 6: /* relationalExpression ::= additiveExpression */
#line 202 "expression.y"
{
   U_TRACE(0, "relationalExpression(yygotominor.yy13) ::= additiveExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[0].minor.yy13 = %p", yygotominor.yy13, yymsp[0].minor.yy13)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   yygotominor.yy13 = yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 892 "expression.c"
        break;
      case 7: /* additiveExpression ::= additiveExpression additiveCond multiplicativeExpression */
#line 214 "expression.y"
{
   U_TRACE(0, "additiveExpression(yygotominor.yy13) ::= additiveExpression(yymsp[-2].minor.yy13) additiveCond(yymsp[-1].minor.yy46) multiplicativeExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy13)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy13 = %.*S yymsp[-1].minor.yy46 = %d yymsp[0].minor.yy13 = %.*S", U_STRING_TO_TRACE(*yymsp[-2].minor.yy13), yymsp[-1].minor.yy46, U_STRING_TO_TRACE(*yymsp[0].minor.yy13))

   long Blo = yymsp[-2].minor.yy13->strtol(),
        Dlo = yymsp[0].minor.yy13->strtol(),
         lo = (yymsp[-1].minor.yy46 == U_TK_PLUS ? Blo + Dlo
                              : Blo - Dlo);

   yygotominor.yy13 = U_NEW(UString(UStringExt::numberToString(lo)));

   delete yymsp[-2].minor.yy13;
   delete yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 916 "expression.c"
        break;
      case 8: /* additiveExpression ::= multiplicativeExpression */
#line 235 "expression.y"
{
   U_TRACE(0, "additiveExpression(yygotominor.yy13) ::= multiplicativeExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[0].minor.yy13 = %p", yygotominor.yy13, yymsp[0].minor.yy13)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   yygotominor.yy13 = yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 931 "expression.c"
        break;
      case 9: /* multiplicativeExpression ::= multiplicativeExpression multiplicativeCond unaryExpression */
#line 247 "expression.y"
{
   U_TRACE(0, "multiplicativeExpression(yygotominor.yy13) ::= multiplicativeExpression(yymsp[-2].minor.yy13) multiplicativeCond(yymsp[-1].minor.yy46) unaryExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy13)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy13 = %.*S yymsp[-1].minor.yy46 = %d yymsp[0].minor.yy13 = %.*S", U_STRING_TO_TRACE(*yymsp[-2].minor.yy13), yymsp[-1].minor.yy46, U_STRING_TO_TRACE(*yymsp[0].minor.yy13))

   long Blo = yymsp[-2].minor.yy13->strtol(),
        Dlo = yymsp[0].minor.yy13->strtol(),
         lo = (yymsp[-1].minor.yy46 == U_TK_MULT ? Blo * Dlo :
               yymsp[-1].minor.yy46 == U_TK_DIV  ? Blo / Dlo :
                                Blo % Dlo);

   yygotominor.yy13 = U_NEW(UString(UStringExt::numberToString(lo)));

   delete yymsp[-2].minor.yy13;
   delete yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 956 "expression.c"
        break;
      case 10: /* multiplicativeExpression ::= unaryExpression */
#line 269 "expression.y"
{
   U_TRACE(0, "multiplicativeExpression(yygotominor.yy13) ::= unaryExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[0].minor.yy13 = %p", yygotominor.yy13, yymsp[0].minor.yy13)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   yygotominor.yy13 = yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 971 "expression.c"
        break;
      case 11: /* unaryExpression ::= NOT primaryExpression */
#line 281 "expression.y"
{
   U_TRACE(0, "unaryExpression(yygotominor.yy13) ::= NOT primaryExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[0].minor.yy13 = %p", yygotominor.yy13, yymsp[0].minor.yy13)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   yygotominor.yy13 = (yymsp[0].minor.yy13->empty() ? U_NEW(UString(U_CONSTANT_TO_PARAM("true"))) : U_NEW(UString));

   delete yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
  yy_destructor(17,&yymsp[-1].minor);
}
#line 989 "expression.c"
        break;
      case 12: /* unaryExpression ::= primaryExpression */
#line 295 "expression.y"
{
   U_TRACE(0, "unaryExpression(yygotominor.yy13) ::= primaryExpression(yymsp[0].minor.yy13)")

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[0].minor.yy13 = %p", yygotominor.yy13, yymsp[0].minor.yy13)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   yygotominor.yy13 = yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 1004 "expression.c"
        break;
      case 13: /* primaryExpression ::= LPAREN booleanExpression RPAREN */
#line 307 "expression.y"
{
   U_TRACE(0, "primaryExpression(yygotominor.yy13) ::= LPAREN booleanExpression(yymsp[-1].minor.yy13) RPAREN")

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[-1].minor.yy13 = %p", yygotominor.yy13, yymsp[-1].minor.yy13)

   U_INTERNAL_ASSERT_POINTER(yymsp[-1].minor.yy13)

   yygotominor.yy13 = yymsp[-1].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
  yy_destructor(19,&yymsp[-2].minor);
  yy_destructor(20,&yymsp[0].minor);
}
#line 1021 "expression.c"
        break;
      case 14: /* primaryExpression ::= value */
#line 319 "expression.y"
{
   U_TRACE(0, "primaryExpression(yygotominor.yy13) ::= value(yymsp[0].minor.yy13)")

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[0].minor.yy13 = %p", yygotominor.yy13, yymsp[0].minor.yy13)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy13)

   yygotominor.yy13 = yymsp[0].minor.yy13;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 1036 "expression.c"
        break;
      case 15: /* value ::= VALUE */
#line 331 "expression.y"
{
   U_TRACE(0, "value(yygotominor.yy13) ::= VALUE(yymsp[0].minor.yy0)")

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[0].minor.yy0 = %p", yygotominor.yy13, yymsp[0].minor.yy0)

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy0)

   yygotominor.yy13 = yymsp[0].minor.yy0;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 1051 "expression.c"
        break;
      case 16: /* value ::= value VALUE */
#line 343 "expression.y"
{
   U_TRACE(0, "value(yygotominor.yy13) ::= value(yymsp[-1].minor.yy13) VALUE(yymsp[0].minor.yy0)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-1].minor.yy13)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy0)

   U_INTERNAL_DUMP("yymsp[-1].minor.yy13 = %.*S yymsp[0].minor.yy0 = %.*S", U_STRING_TO_TRACE(*yymsp[-1].minor.yy13), U_STRING_TO_TRACE(*yymsp[0].minor.yy0))

    yygotominor.yy13  =  yymsp[-1].minor.yy13;
   *yygotominor.yy13 += *yymsp[0].minor.yy0;

   delete yymsp[0].minor.yy0;

   U_INTERNAL_DUMP("yygotominor.yy13 = %.*S", U_STRING_TO_TRACE(*yygotominor.yy13))
}
#line 1070 "expression.c"
        break;
      case 17: /* booleanCond ::= OR */
#line 359 "expression.y"
{
   U_TRACE(0, "booleanCond(yygotominor.yy46) ::= OR")
   yygotominor.yy46 = U_TK_OR;
  yy_destructor(2,&yymsp[0].minor);
}
#line 1079 "expression.c"
        break;
      case 18: /* booleanCond ::= AND */
#line 363 "expression.y"
{
   U_TRACE(0, "booleanCond(yygotominor.yy46) ::= AND")
   yygotominor.yy46 = U_TK_AND;
  yy_destructor(1,&yymsp[0].minor);
}
#line 1088 "expression.c"
        break;
      case 19: /* equalityCond ::= EQ */
#line 367 "expression.y"
{
   U_TRACE(0, "equalityCond(yygotominor.yy46) ::= EQ")
   yygotominor.yy46 = U_TK_EQ;
  yy_destructor(3,&yymsp[0].minor);
}
#line 1097 "expression.c"
        break;
      case 20: /* equalityCond ::= NE */
#line 371 "expression.y"
{
   U_TRACE(0, "equalityCond(yygotominor.yy46) ::= NE")
   yygotominor.yy46 = U_TK_NE;
  yy_destructor(4,&yymsp[0].minor);
}
#line 1106 "expression.c"
        break;
      case 21: /* equalityCond ::= STARTS_WITH */
#line 375 "expression.y"
{
   U_TRACE(0, "equalityCond(yygotominor.yy46) ::= STARTS_WITH")
   yygotominor.yy46 = U_TK_STARTS_WITH;
  yy_destructor(9,&yymsp[0].minor);
}
#line 1115 "expression.c"
        break;
      case 22: /* equalityCond ::= ENDS_WITH */
#line 379 "expression.y"
{
   U_TRACE(0, "equalityCond(yygotominor.yy46) ::= ENDS_WITH")
   yygotominor.yy46 = U_TK_ENDS_WITH;
  yy_destructor(10,&yymsp[0].minor);
}
#line 1124 "expression.c"
        break;
      case 23: /* equalityCond ::= CONTAINS */
#line 383 "expression.y"
{
   U_TRACE(0, "equalityCond(yygotominor.yy46) ::= CONTAINS")
   yygotominor.yy46 = U_TK_CONTAINS;
  yy_destructor(11,&yymsp[0].minor);
}
#line 1133 "expression.c"
        break;
      case 24: /* relationalCond ::= GT */
#line 387 "expression.y"
{
   U_TRACE(0, "relationalCond(yygotominor.yy46) ::= GT")
   yygotominor.yy46 = U_TK_GT;
  yy_destructor(5,&yymsp[0].minor);
}
#line 1142 "expression.c"
        break;
      case 25: /* relationalCond ::= GE */
#line 391 "expression.y"
{
   U_TRACE(0, "relationalCond(yygotominor.yy46) ::= GE")
   yygotominor.yy46 = U_TK_GE;
  yy_destructor(6,&yymsp[0].minor);
}
#line 1151 "expression.c"
        break;
      case 26: /* relationalCond ::= LT */
#line 395 "expression.y"
{
   U_TRACE(0, "relationalCond(yygotominor.yy46) ::= LT")
   yygotominor.yy46 = U_TK_LT;
  yy_destructor(7,&yymsp[0].minor);
}
#line 1160 "expression.c"
        break;
      case 27: /* relationalCond ::= LE */
#line 399 "expression.y"
{
   U_TRACE(0, "relationalCond(yygotominor.yy46) ::= LE")
   yygotominor.yy46 = U_TK_LE;
  yy_destructor(8,&yymsp[0].minor);
}
#line 1169 "expression.c"
        break;
      case 28: /* additiveCond ::= PLUS */
#line 403 "expression.y"
{
   U_TRACE(0, "additiveCond(yygotominor.yy46) ::= PLUS")
   yygotominor.yy46 = U_TK_PLUS;
  yy_destructor(12,&yymsp[0].minor);
}
#line 1178 "expression.c"
        break;
      case 29: /* additiveCond ::= MINUS */
#line 407 "expression.y"
{
   U_TRACE(0, "additiveCond(yygotominor.yy46) ::= MINUS")
   yygotominor.yy46 = U_TK_MINUS;
  yy_destructor(13,&yymsp[0].minor);
}
#line 1187 "expression.c"
        break;
      case 30: /* multiplicativeCond ::= MULT */
#line 411 "expression.y"
{
   U_TRACE(0, "multiplicativeCond(yygotominor.yy46) ::= MULT")
   yygotominor.yy46 = U_TK_MULT;
  yy_destructor(14,&yymsp[0].minor);
}
#line 1196 "expression.c"
        break;
      case 31: /* multiplicativeCond ::= DIV */
#line 415 "expression.y"
{
   U_TRACE(0, "multiplicativeCond(yygotominor.yy46) ::= DIV")
   yygotominor.yy46 = U_TK_DIV;
  yy_destructor(15,&yymsp[0].minor);
}
#line 1205 "expression.c"
        break;
      case 32: /* multiplicativeCond ::= MOD */
#line 419 "expression.y"
{
   U_TRACE(0, "multiplicativeCond(yygotominor.yy46) ::= MOD")
   yygotominor.yy46 = U_TK_MOD;
  yy_destructor(16,&yymsp[0].minor);
}
#line 1214 "expression.c"
        break;
      case 33: /* primaryExpression ::= FN_CALL LPAREN RPAREN */
#line 424 "expression.y"
{
   U_TRACE(0, "primaryExpression(yygotominor.yy13) ::= FN_CALL(yymsp[-2].minor.yy0) LPAREN RPAREN")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy0)

   U_INTERNAL_DUMP("yygotominor.yy13 = %p yymsp[-2].minor.yy0 = %.*S", yygotominor.yy13, U_STRING_TO_TRACE(*yymsp[-2].minor.yy0))

   long lo;
#ifdef __MINGW32__
   lPFv addr = (lPFv) ::GetProcAddress((HMODULE)0, yymsp[-2].minor.yy0->c_str());
#else
   lPFv addr = (lPFv) U_SYSCALL(dlsym, "%p,%S", RTLD_DEFAULT, yymsp[-2].minor.yy0->c_str());
#endif

   yygotominor.yy13 = (addr ? (lo = (*addr)(), U_NEW(UString(UStringExt::numberToString(lo)))) : U_NEW(UString));

   delete yymsp[-2].minor.yy0;
  yy_destructor(19,&yymsp[-1].minor);
  yy_destructor(20,&yymsp[0].minor);
}
#line 1238 "expression.c"
        break;
      case 34: /* primaryExpression ::= FN_CALL LPAREN params RPAREN */
#line 443 "expression.y"
{
   U_TRACE(0, "primaryExpression(yygotominor.yy13) ::= FN_CALL(yymsp[-3].minor.yy0) LPAREN params(yymsp[-1].minor.yy65) RPAREN")

   U_INTERNAL_ASSERT_POINTER(yymsp[-3].minor.yy0)
   U_INTERNAL_ASSERT_POINTER(yymsp[-1].minor.yy65)

   U_INTERNAL_DUMP("yymsp[-3].minor.yy0 = %.*S yymsp[-1].minor.yy65 = %p", U_STRING_TO_TRACE(*yymsp[-3].minor.yy0), yymsp[-1].minor.yy65)

   long lo, lo0, lo1;
#ifdef __MINGW32__
   lPFll addr = (lPFll) ::GetProcAddress((HMODULE)0, yymsp[-3].minor.yy0->c_str());
#else
   lPFll addr = (lPFll) U_SYSCALL(dlsym, "%p,%S", RTLD_DEFAULT, yymsp[-3].minor.yy0->c_str());
#endif

   yygotominor.yy13 = (addr ? (lo0 = (*yymsp[-1].minor.yy65)[0].strtol(), lo1 = (*yymsp[-1].minor.yy65)[1].strtol(), lo = (*addr)(lo0, lo1), U_NEW(UString(UStringExt::numberToString(lo)))) : U_NEW(UString));

   delete yymsp[-3].minor.yy0;
  yy_destructor(19,&yymsp[-2].minor);
  yy_destructor(20,&yymsp[0].minor);
}
#line 1263 "expression.c"
        break;
      case 35: /* params ::= VALUE */
#line 463 "expression.y"
{
   U_TRACE(0, "params(yygotominor.yy65) ::= VALUE(yymsp[0].minor.yy0)")

   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy0)

   U_INTERNAL_DUMP("yygotominor.yy65 = %p yymsp[0].minor.yy0 = %.*S", yygotominor.yy65, U_STRING_TO_TRACE(*yymsp[0].minor.yy0))

   yygotominor.yy65 = U_NEW(Items);

   yygotominor.yy65->push_back(*yymsp[0].minor.yy0);

   delete yymsp[0].minor.yy0;
}
#line 1280 "expression.c"
        break;
      case 36: /* params ::= params COMMA VALUE */
#line 477 "expression.y"
{
   U_TRACE(0, "params(yygotominor.yy65) ::= params(yymsp[-2].minor.yy65) COMMA VALUE(yymsp[0].minor.yy0)")

   U_INTERNAL_ASSERT_POINTER(yymsp[-2].minor.yy65)
   U_INTERNAL_ASSERT_POINTER(yymsp[0].minor.yy0)

   U_INTERNAL_DUMP("yymsp[-2].minor.yy65 = %p yymsp[0].minor.yy0 = %.*S", yymsp[-2].minor.yy65, U_STRING_TO_TRACE(*yymsp[0].minor.yy0))

   yymsp[-2].minor.yy65->push_back(*yymsp[0].minor.yy0);

   yygotominor.yy65 = yymsp[-2].minor.yy65;

   delete yymsp[0].minor.yy0;
  yy_destructor(22,&yymsp[-1].minor);
}
#line 1299 "expression.c"
        break;
  };
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = yyact;
      yymsp->major = yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  expressionParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
#line 43 "expression.y"

   U_TRACE(0, "::parse_failure()")
   U_INTERNAL_ASSERT_POINTER(result)
   result->clear();
   U_WARNING("Parse failure!");
#line 1351 "expression.c"
  expressionParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  expressionParserARG_FETCH;
#define TOKEN (yyminor.yy0)
#line 49 "expression.y"

   U_TRACE(0, "::syntax_error()")
   U_INTERNAL_ASSERT_POINTER(result)
   result->clear();
   U_WARNING("Syntax error!");
#line 1372 "expression.c"
  expressionParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  expressionParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
#line 38 "expression.y"

   U_TRACE(0, "::parse_accept()")
   U_INTERNAL_ASSERT_POINTER(result)
   U_INTERNAL_DUMP("result = %.*S", U_STRING_TO_TRACE(*result))
#line 1397 "expression.c"
  expressionParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "expressionParserAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void expressionParser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  expressionParserTOKENTYPE yyminor       /* The value for the token */
  expressionParserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
  int yyendofinput;     /* True if we are at the end of input */
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      memset(&yyminorunion, 0, sizeof(yyminorunion));
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
  yyendofinput = (yymajor==0);
  expressionParserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,yymajor);
    if( yyact<YYNSTATE ){
      assert( !yyendofinput );  /* Impossible to shift the $ token */
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}

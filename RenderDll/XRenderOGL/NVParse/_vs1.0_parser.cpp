#include "RenderPCH.h"
#include "nvparse.h"

/*  A Bison parser, made from vs1.0_grammar.y
    by GNU Bison v1.28 for Win32  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse vs10_parse
#define yylex vs10_lex
#define yyerror vs10_error
#define yylval vs10_lval
#define yychar vs10_char
#define yydebug vs10_debug
#define yynerrs vs10_nerrs
#define	VERTEX_SHADER	257
#define	ADD_INSTR	258
#define	DP3_INSTR	259
#define	DP4_INSTR	260
#define	DST_INSTR	261
#define	EXP_INSTR	262
#define	EXPP_INSTR	263
#define	FRC_INSTR	264
#define	LIT_INSTR	265
#define	LOG_INSTR	266
#define	LOGP_INSTR	267
#define	M3X2_INSTR	268
#define	M3X3_INSTR	269
#define	M3X4_INSTR	270
#define	M4X3_INSTR	271
#define	M4X4_INSTR	272
#define	MAD_INSTR	273
#define	MAX_INSTR	274
#define	MIN_INSTR	275
#define	MOV_INSTR	276
#define	MUL_INSTR	277
#define	NOP_INSTR	278
#define	RCP_INSTR	279
#define	RSQ_INSTR	280
#define	SGE_INSTR	281
#define	SLT_INSTR	282
#define	SUB_INSTR	283
#define	ILLEGAL	284
#define	UNKNOWN_STRING	285
#define	INTVAL	286
#define	REGISTER	287
#define	XYZW_MODIFIER	288
#define	COMMENT	289

#line 2 "vs1.0_grammar.y"

void yyerror(char *s);
int yylex(void);

#include <math.h>
#include <string>

//#include <malloc.h>
#include "vs1.0_inst_list.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"

//extern bool gbTempInsideMacro;
//extern unsigned int &base_linenumber;
void LexError(char *format, ...);
extern int line_incr;

#define do_linenum_incr()		{ line_number+=line_incr; line_incr = 0; }
//int get_linenum()			{ return( gbTempInsideMacro ? base_linenumber : line_number ); }
int get_linenum()			{ return( line_number ); }

#define YYDEBUG 1


#line 26 "vs1.0_grammar.y"
typedef union {
  int ival;
  unsigned int lval;
  float fval;
  char mask[4];
  char *comment;
  VS10Reg reg;
  VS10InstPtr inst;
  VS10InstListPtr instList;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		103
#define	YYFLAG		-32768
#define	YYNTBASE	44

#define YYTRANSLATE(x) ((unsigned)(x) <= 289 ? yytranslate[x] : 61)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    36,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    43,    37,    38,    39,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    41,     2,    42,     2,     2,     2,     2,     2,    40,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     7,     9,    11,    13,    15,    17,    19,
    21,    23,    25,    27,    32,    37,    41,    44,    46,    48,
    50,    55,    62,    71,    76,    83,    89,    97,   105,   115,
   120,   125,   132,   141,   143,   145,   147,   149,   151,   153,
   155,   157,   159,   161,   163,   165,   167,   169,   171,   173,
   175,   177,   179,   181,   183,   185,   187,   189
};

static const short yyrhs[] = {    45,
     0,    45,    46,     0,    46,     0,    47,     0,    36,     0,
    48,     0,    52,     0,    53,     0,    54,     0,    55,     0,
    24,     0,    35,     0,     3,     0,    56,    49,    37,    49,
     0,    38,    50,    39,    34,     0,    50,    39,    34,     0,
    38,    50,     0,    50,     0,    33,     0,    51,     0,    40,
    41,    32,    42,     0,    40,    41,    33,    39,    34,    42,
     0,    40,    41,    33,    39,    34,    43,    32,    42,     0,
    40,    41,    33,    42,     0,    40,    41,    33,    43,    32,
    42,     0,    40,    41,    38,    33,    42,     0,    40,    41,
    38,    33,    43,    32,    42,     0,    40,    41,    38,    33,
    39,    34,    42,     0,    40,    41,    38,    33,    39,    34,
    43,    32,    42,     0,    57,    49,    37,    49,     0,    58,
    49,    37,    49,     0,    59,    49,    37,    49,    37,    49,
     0,    60,    49,    37,    49,    37,    49,    37,    49,     0,
    22,     0,    11,     0,    25,     0,    26,     0,     8,     0,
     9,     0,    12,     0,    13,     0,    10,     0,    23,     0,
     4,     0,     5,     0,     6,     0,     7,     0,    21,     0,
    20,     0,    28,     0,    27,     0,    14,     0,    15,     0,
    16,     0,    17,     0,    18,     0,    29,     0,    19,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    95,   103,   109,   121,   126,   133,   134,   135,   136,   137,
   138,   142,   146,   152,   158,   168,   178,   188,   199,   199,
   203,   210,   245,   281,   286,   291,   296,   301,   306,   313,
   319,   325,   331,   338,   342,   348,   352,   356,   360,   364,
   368,   374,   379,   383,   387,   391,   395,   399,   403,   407,
   411,   415,   419,   423,   427,   431,   435,   441
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","VERTEX_SHADER",
"ADD_INSTR","DP3_INSTR","DP4_INSTR","DST_INSTR","EXP_INSTR","EXPP_INSTR","FRC_INSTR",
"LIT_INSTR","LOG_INSTR","LOGP_INSTR","M3X2_INSTR","M3X3_INSTR","M3X4_INSTR",
"M4X3_INSTR","M4X4_INSTR","MAD_INSTR","MAX_INSTR","MIN_INSTR","MOV_INSTR","MUL_INSTR",
"NOP_INSTR","RCP_INSTR","RSQ_INSTR","SGE_INSTR","SLT_INSTR","SUB_INSTR","ILLEGAL",
"UNKNOWN_STRING","INTVAL","REGISTER","XYZW_MODIFIER","COMMENT","'\\n'","','",
"'-'","'.'","'c'","'['","']'","'+'","VS10Program","InstSequence","InstLine",
"Instruction","VECTORopinstruction","genericReg","genReg","constantReg","SCALARopinstruction",
"UNARYopinstruction","BINopinstruction","TRIopinstruction","VECTORop","SCALARop",
"UNARYop","BINop","TRIop", NULL
};
#endif

static const short yyr1[] = {     0,
    44,    45,    45,    46,    46,    47,    47,    47,    47,    47,
    47,    47,    47,    48,    49,    49,    49,    49,    50,    50,
    51,    51,    51,    51,    51,    51,    51,    51,    51,    52,
    53,    54,    55,    56,    56,    57,    57,    57,    57,    57,
    57,    58,    59,    59,    59,    59,    59,    59,    59,    59,
    59,    59,    59,    59,    59,    59,    59,    60
};

static const short yyr2[] = {     0,
     1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     4,     4,     3,     2,     1,     1,     1,
     4,     6,     8,     4,     6,     5,     7,     7,     9,     4,
     4,     6,     8,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1
};

static const short yydefact[] = {     0,
    13,    44,    45,    46,    47,    38,    39,    42,    35,    40,
    41,    52,    53,    54,    55,    56,    58,    49,    48,    34,
    43,    11,    36,    37,    51,    50,    57,    12,     5,     1,
     3,     4,     6,     7,     8,     9,    10,     0,     0,     0,
     0,     0,     2,    19,     0,     0,     0,    18,    20,     0,
     0,     0,     0,    17,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    14,    16,    30,    31,     0,
     0,    15,    21,     0,    24,     0,     0,     0,     0,     0,
     0,     0,    26,     0,    32,     0,    22,     0,    25,     0,
     0,     0,     0,    28,     0,    27,    33,    23,     0,    29,
     0,     0,     0
};

static const short yydefgoto[] = {   101,
    30,    31,    32,    33,    47,    48,    49,    34,    35,    36,
    37,    38,    39,    40,    41,    42
};

static const short yypact[] = {    51,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    51,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -29,   -29,   -29,
   -29,   -29,-32768,-32768,   -28,   -35,   -23,   -21,-32768,   -13,
    -5,    -4,    -3,    -2,   -25,   -29,   -24,   -29,   -29,   -29,
   -29,     1,    -6,   -16,     5,-32768,-32768,-32768,-32768,     4,
     6,-32768,-32768,     8,-32768,    12,   -14,   -29,   -29,   -27,
     3,    13,-32768,    14,-32768,    11,-32768,    17,-32768,   -12,
     9,   -29,    10,-32768,    18,-32768,-32768,-32768,    39,-32768,
    82,    83,-32768
};

static const short yypgoto[] = {-32768,
-32768,    54,-32768,-32768,   -39,    40,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768
};


#define	YYLAST		87


static const short yytable[] = {    50,
    51,    52,    53,    44,    44,    55,    63,    64,    45,    67,
    46,    46,    65,    56,    87,    88,    66,    57,    68,    69,
    70,    71,    74,    58,    82,    75,    76,    83,    84,    94,
    95,    59,    60,    61,    72,    73,    62,    77,    85,    86,
    78,    80,    79,    81,    89,    91,    90,    92,    93,    99,
    96,    98,    97,     1,     2,     3,     4,     5,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
   100,   102,   103,    43,    54,    28,    29
};

static const short yycheck[] = {    39,
    40,    41,    42,    33,    33,    41,    32,    33,    38,    34,
    40,    40,    38,    37,    42,    43,    56,    39,    58,    59,
    60,    61,    39,    37,    39,    42,    43,    42,    43,    42,
    43,    37,    37,    37,    34,    42,    39,    33,    78,    79,
    37,    34,    37,    32,    42,    32,    34,    37,    32,    32,
    42,    42,    92,     3,     4,     5,     6,     7,     8,     9,
    10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    42,     0,     0,    30,    45,    35,    36
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"
/* This file comes from bison-@bison_version@.  */


#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 96 "vs1.0_grammar.y"
{
	yyvsp[0].instList->Validate();
	yyvsp[0].instList->Translate();
	delete yyvsp[0].instList;
	;
    break;}
case 2:
#line 104 "vs1.0_grammar.y"
{
		*(yyvsp[-1].instList) += yyvsp[0].inst;
		delete yyvsp[0].inst;
		yyval.instList = yyvsp[-1].instList
	;
    break;}
case 3:
#line 110 "vs1.0_grammar.y"
{
 		VS10InstListPtr instList = new VS10InstList;
		if ( yyvsp[0].inst != NULL )
			{
			*instList += yyvsp[0].inst;
			delete yyvsp[0].inst;
			}
		yyval.instList = instList;
	;
    break;}
case 4:
#line 122 "vs1.0_grammar.y"
{
		yyval.inst = yyvsp[0].inst;
		do_linenum_incr();
	;
    break;}
case 5:
#line 127 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum() );
		do_linenum_incr();
	;
    break;}
case 11:
#line 139 "vs1.0_grammar.y"
{
		   yyval.inst = new VS10Inst( get_linenum(), VS10_NOP );
		   ;
    break;}
case 12:
#line 143 "vs1.0_grammar.y"
{
		   yyval.inst = new VS10Inst( get_linenum(), VS10_COMMENT, yyvsp[0].comment );
		   ;
    break;}
case 13:
#line 147 "vs1.0_grammar.y"
{
		   yyval.inst = new VS10Inst( get_linenum(), VS10_HEADER );
		   ;
    break;}
case 14:
#line 153 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-3].ival, yyvsp[-2].reg, yyvsp[0].reg );
	;
    break;}
case 15:
#line 159 "vs1.0_grammar.y"
{
		   VS10Reg reg;
		   reg = yyvsp[-2].reg;
		   reg.sign = -1;
		   reg.type = yyvsp[-2].reg.type;
		   reg.index = yyvsp[-2].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = yyvsp[0].mask[i];
		   yyval.reg = reg;
		   ;
    break;}
case 16:
#line 169 "vs1.0_grammar.y"
{
		   VS10Reg reg;
		   reg = yyvsp[-2].reg;
		   reg.sign = 1;
		   reg.type = yyvsp[-2].reg.type;
		   reg.index = yyvsp[-2].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = yyvsp[0].mask[i];
		   yyval.reg = reg;
		   ;
    break;}
case 17:
#line 179 "vs1.0_grammar.y"
{
		   VS10Reg reg;
		   reg = yyvsp[0].reg;
		   reg.sign = -1;
		   reg.type = yyvsp[0].reg.type;
		   reg.index = yyvsp[0].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = 0;
		   yyval.reg = reg;
		   ;
    break;}
case 18:
#line 189 "vs1.0_grammar.y"
{
		   VS10Reg reg;
		   reg = yyvsp[0].reg;
		   reg.sign = 1;
		   reg.type = yyvsp[0].reg.type;
		   reg.index = yyvsp[0].reg.index;
		   for ( int i = 0; i < 4; i++ ) reg.mask[i] = 0;
		   yyval.reg = reg;
		   ;
    break;}
case 20:
#line 200 "vs1.0_grammar.y"
{
	  ;
    break;}
case 21:
#line 204 "vs1.0_grammar.y"
{
		   VS10Reg reg;
		   reg.type = TYPE_CONSTANT_MEM_REG;
		   reg.index = yyvsp[-1].ival;
		   yyval.reg = reg;
		   ;
    break;}
case 22:
#line 211 "vs1.0_grammar.y"
{
		   // Register is valid only if
		   //	type = TYPE_ADDRESS_REG
		   //	index = 0
		   //	len(mask) = 1
		   //	mask[0] = 'x'
		   VS10Reg reg;
		   yyval.reg.type = TYPE_CONSTANT_A0_REG;
		   if ( yyvsp[-3].reg.type != TYPE_ADDRESS_REG )
		       {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
		       else if ( yyvsp[-3].reg.index != 0 )
			   {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
			   else
			   {
			       int len = 0;
				   while ( len < 2 )
				   {
				       if ( yyvsp[-1].mask[len] == 0 )
				           break;
					   len++;
				   }
				   if ( len != 1 || yyvsp[-1].mask[0] != 'x' )
				   {
			       LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
				   }

				   reg.type = TYPE_CONSTANT_A0_REG;
				   yyval.reg = reg;
			   }
		   ;
    break;}
case 23:
#line 246 "vs1.0_grammar.y"
{
		   // Register is valid only if
		   //	type = TYPE_ADDRESS_REG
		   //	index = 0
		   //	len(mask) = 1
		   //	mask[0] = 'x'
		   VS10Reg reg;
		   yyval.reg.type = TYPE_CONSTANT_A0_OFFSET_REG;
		   if ( yyvsp[-5].reg.type != TYPE_ADDRESS_REG )
		       {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
		       else if ( yyvsp[-5].reg.index != 0 )
			   {
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
			   }
			   else
			   {
			       int len = 0;
				   while ( len < 2 )
				   {
				       if ( yyvsp[-3].mask[len] == 0 )
				           break;
					   len++;
				   }
				   if ( len != 1 || yyvsp[-3].mask[0] != 'x' )
				   {
			       LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
				   }

				   reg.type = TYPE_CONSTANT_A0_OFFSET_REG;
				   reg.index = yyvsp[-1].ival;
				   yyval.reg = reg;
			   }
		   ;
    break;}
case 24:
#line 282 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 25:
#line 287 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 26:
#line 292 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 27:
#line 297 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 28:
#line 302 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 29:
#line 307 "vs1.0_grammar.y"
{
		       yyval.reg.type = TYPE_CONSTANT_A0_REG;
			   LexError( "constant register index must be:\t<int>, a0.x, or a0.x + <int>.\n" );
		   ;
    break;}
case 30:
#line 314 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-3].ival, yyvsp[-2].reg, yyvsp[0].reg );
	;
    break;}
case 31:
#line 320 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-3].ival, yyvsp[-2].reg, yyvsp[0].reg );
	;
    break;}
case 32:
#line 326 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-5].ival, yyvsp[-4].reg, yyvsp[-2].reg, yyvsp[0].reg );
	;
    break;}
case 33:
#line 333 "vs1.0_grammar.y"
{
		yyval.inst = new VS10Inst( get_linenum(), yyvsp[-7].ival, yyvsp[-6].reg, yyvsp[-4].reg, yyvsp[-2].reg, yyvsp[0].reg );
	;
    break;}
case 34:
#line 339 "vs1.0_grammar.y"
{
		yyval.ival = VS10_MOV;
	;
    break;}
case 35:
#line 343 "vs1.0_grammar.y"
{
		yyval.ival = VS10_LIT;
	;
    break;}
case 36:
#line 349 "vs1.0_grammar.y"
{
	yyval.ival = VS10_RCP;
	;
    break;}
case 37:
#line 353 "vs1.0_grammar.y"
{
	yyval.ival = VS10_RSQ;
	;
    break;}
case 38:
#line 357 "vs1.0_grammar.y"
{
	yyval.ival = VS10_EXP;
	;
    break;}
case 39:
#line 361 "vs1.0_grammar.y"
{
	yyval.ival = VS10_EXPP;
	;
    break;}
case 40:
#line 365 "vs1.0_grammar.y"
{
	yyval.ival = VS10_LOG;
	;
    break;}
case 41:
#line 369 "vs1.0_grammar.y"
{
	yyval.ival = VS10_LOGP;
	;
    break;}
case 42:
#line 375 "vs1.0_grammar.y"
{
	yyval.ival = VS10_FRC;
	;
    break;}
case 43:
#line 380 "vs1.0_grammar.y"
{
	yyval.ival = VS10_MUL;
	;
    break;}
case 44:
#line 384 "vs1.0_grammar.y"
{
	yyval.ival = VS10_ADD;
	;
    break;}
case 45:
#line 388 "vs1.0_grammar.y"
{
	yyval.ival = VS10_DP3;
	;
    break;}
case 46:
#line 392 "vs1.0_grammar.y"
{
	yyval.ival = VS10_DP4;
	;
    break;}
case 47:
#line 396 "vs1.0_grammar.y"
{
	yyval.ival = VS10_DST;
	;
    break;}
case 48:
#line 400 "vs1.0_grammar.y"
{
	yyval.ival = VS10_MIN;
	;
    break;}
case 49:
#line 404 "vs1.0_grammar.y"
{
	yyval.ival = VS10_MAX;
	;
    break;}
case 50:
#line 408 "vs1.0_grammar.y"
{
	yyval.ival = VS10_SLT;
	;
    break;}
case 51:
#line 412 "vs1.0_grammar.y"
{
	yyval.ival = VS10_SGE;
	;
    break;}
case 52:
#line 416 "vs1.0_grammar.y"
{
	yyval.ival = VS10_M3X2;
	;
    break;}
case 53:
#line 420 "vs1.0_grammar.y"
{
	yyval.ival = VS10_M3X3;
	;
    break;}
case 54:
#line 424 "vs1.0_grammar.y"
{
	yyval.ival = VS10_M3X4;
	;
    break;}
case 55:
#line 428 "vs1.0_grammar.y"
{
	yyval.ival = VS10_M4X3;
	;
    break;}
case 56:
#line 432 "vs1.0_grammar.y"
{
	yyval.ival = VS10_M4X4;
	;
    break;}
case 57:
#line 436 "vs1.0_grammar.y"
{
	yyval.ival = VS10_SUB;
	;
    break;}
case 58:
#line 442 "vs1.0_grammar.y"
{
	yyval.ival = VS10_MAD;
	;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 448 "vs1.0_grammar.y"

void yyerror(char* s)
{
    LexError( "Syntax Error.\n" );
    //errors.set(s);
    //errors.set("unrecognized token");
}

#include "RenderPCH.h"
#include "nvparse.h"

/*  A Bison parser, made from rc1.0_grammar.y
    by GNU Bison v1.28 for Win32  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse rc10_parse
#define yylex rc10_lex
#define yyerror rc10_error
#define yylval rc10_lval
#define yychar rc10_char
#define yydebug rc10_debug
#define yynerrs rc10_nerrs
#define	regVariable	257
#define	constVariable	258
#define	color_sum	259
#define	final_product	260
#define	expandString	261
#define	halfBiasString	262
#define	unsignedString	263
#define	unsignedInvertString	264
#define	muxString	265
#define	sumString	266
#define	rgb_portion	267
#define	alpha_portion	268
#define	openParen	269
#define	closeParen	270
#define	openBracket	271
#define	closeBracket	272
#define	semicolon	273
#define	comma	274
#define	dot	275
#define	times	276
#define	minus	277
#define	equals	278
#define	plus	279
#define	bias_by_negative_one_half_scale_by_two	280
#define	bias_by_negative_one_half	281
#define	scale_by_one_half	282
#define	scale_by_two	283
#define	scale_by_four	284
#define	clamp_color_sum	285
#define	lerp	286
#define	fragment_rgb	287
#define	fragment_alpha	288
#define	floatValue	289

#line 2 "rc1.0_grammar.y"

void yyerror(char* s);
int yylex ( void );

#ifdef _WIN32
# include <windows.h>
#endif

#include <stdio.h>
//#include <malloc.h>

#include "rc1.0_combiners.h"
#include "nvparse_errors.h"
#include "nvparse_externs.h"



#line 19 "rc1.0_grammar.y"
typedef union {
  int ival;
  float fval;
  RegisterEnum registerEnum;
  BiasScaleEnum biasScaleEnum;
  MappedRegisterStruct mappedRegisterStruct;
  ConstColorStruct constColorStruct;
  GeneralPortionStruct generalPortionStruct;
  GeneralFunctionStruct generalFunctionStruct;
  OpStruct opStruct;
  GeneralCombinerStruct generalCombinerStruct;
  GeneralCombinersStruct generalCombinersStruct;
  FinalProductStruct finalProductStruct;
  FinalRgbFunctionStruct finalRgbFunctionStruct;
  FinalAlphaFunctionStruct finalAlphaFunctionStruct;
  FinalCombinerStruct finalCombinerStruct;
  CombinersStruct combinersStruct;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		220
#define	YYFLAG		-32768
#define	YYNTBASE	36

#define YYTRANSLATE(x) ((unsigned)(x) <= 289 ? yytranslate[x] : 57)

static const char yytranslate[] = {     0,
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
     0,     2,     6,    11,    14,    17,    21,    23,    36,    39,
    41,    46,    52,    59,    63,    68,    74,    80,    85,    87,
    89,    91,    94,    99,   105,   110,   116,   121,   126,   129,
   132,   135,   137,   140,   144,   148,   150,   157,   164,   169,
   176,   183,   188,   193,   198,   203,   208,   210,   215,   220,
   222,   227,   232,   234,   239,   244,   247,   251,   255,   260,
   265,   268,   272,   276,   281,   286,   288,   291,   294,   298,
   302,   304,   307,   310,   314,   318,   323,   330,   344,   358,
   370,   377,   386,   391,   398,   403,   405
};

static const short yyrhs[] = {    37,
     0,    38,    39,    51,     0,    38,    38,    39,    51,     0,
    39,    51,     0,    38,    51,     0,    38,    38,    51,     0,
    51,     0,     4,    24,    15,    35,    20,    35,    20,    35,
    20,    35,    16,    19,     0,    39,    40,     0,    40,     0,
    17,    41,    41,    18,     0,    17,    38,    41,    41,    18,
     0,    17,    38,    38,    41,    41,    18,     0,    17,    41,
    18,     0,    17,    38,    41,    18,     0,    17,    38,    38,
    41,    18,     0,    42,    17,    44,    49,    18,     0,    42,
    17,    44,    18,     0,    13,     0,    14,     0,    56,     0,
    23,    56,     0,     7,    15,    56,    16,     0,    23,     7,
    15,    56,    16,     0,     8,    15,    56,    16,     0,    23,
     8,    15,    56,    16,     0,     9,    15,    56,    16,     0,
    10,    15,    56,    16,     0,    45,    45,     0,    45,    46,
     0,    46,    45,     0,    45,     0,    46,    46,     0,    46,
    46,    47,     0,    46,    46,    48,     0,    46,     0,    56,
    24,    43,    21,    43,    19,     0,    56,    24,    43,    22,
    43,    19,     0,    56,    24,    43,    19,     0,    56,    24,
    11,    15,    16,    19,     0,    56,    24,    12,    15,    16,
    19,     0,    26,    15,    16,    19,     0,    27,    15,    16,
    19,     0,    28,    15,    16,    19,     0,    29,    15,    16,
    19,     0,    30,    15,    16,    19,     0,    56,     0,     9,
    15,    56,    16,     0,    10,    15,    56,    16,     0,     5,
     0,     9,    15,     5,    16,     0,    10,    15,     5,    16,
     0,     6,     0,     9,    15,     6,    16,     0,    10,    15,
     6,    16,     0,    55,    54,     0,    52,    55,    54,     0,
    53,    55,    54,     0,    52,    53,    55,    54,     0,    53,
    52,    55,    54,     0,    54,    55,     0,    52,    54,    55,
     0,    53,    54,    55,     0,    52,    53,    54,    55,     0,
    53,    52,    54,    55,     0,    54,     0,    52,    54,     0,
    53,    54,     0,    52,    53,    54,     0,    53,    52,    54,
     0,    55,     0,    52,    55,     0,    53,    55,     0,    52,
    53,    55,     0,    53,    52,    55,     0,    31,    15,    16,
    19,     0,     6,    24,    50,    22,    50,    19,     0,    33,
    24,    32,    15,    50,    20,    50,    20,    50,    16,    25,
    50,    19,     0,    33,    24,    50,    25,    32,    15,    50,
    20,    50,    20,    50,    16,    19,     0,    33,    24,    32,
    15,    50,    20,    50,    20,    50,    16,    19,     0,    33,
    24,    50,    22,    50,    19,     0,    33,    24,    50,    22,
    50,    25,    50,    19,     0,    33,    24,    50,    19,     0,
    33,    24,    50,    25,    50,    19,     0,    34,    24,    50,
    19,     0,     4,     0,     3,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    72,    79,    85,    91,    97,   105,   113,   123,   132,   137,
   145,   151,   157,   163,   169,   175,   183,   189,   199,   203,
   209,   215,   221,   227,   233,   239,   245,   251,   259,   265,
   271,   277,   283,   289,   295,   301,   309,   317,   323,   335,
   343,   351,   355,   359,   363,   367,   373,   379,   385,   391,
   397,   403,   409,   415,   421,   429,   435,   441,   447,   453,
   460,   466,   472,   478,   484,   491,   499,   507,   515,   523,
   532,   540,   548,   556,   564,   574,   580,   588,   594,   600,
   610,   622,   632,   646,   660,   668,   672
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","regVariable",
"constVariable","color_sum","final_product","expandString","halfBiasString",
"unsignedString","unsignedInvertString","muxString","sumString","rgb_portion",
"alpha_portion","openParen","closeParen","openBracket","closeBracket","semicolon",
"comma","dot","times","minus","equals","plus","bias_by_negative_one_half_scale_by_two",
"bias_by_negative_one_half","scale_by_one_half","scale_by_two","scale_by_four",
"clamp_color_sum","lerp","fragment_rgb","fragment_alpha","floatValue","WholeEnchilada",
"Combiners","ConstColor","GeneralCombiners","GeneralCombiner","GeneralPortion",
"PortionDesignator","GeneralMappedRegister","GeneralFunction","Dot","Mul","Mux",
"Sum","BiasScale","FinalMappedRegister","FinalCombiner","ClampColorSum","FinalProduct",
"FinalRgbFunction","FinalAlphaFunction","Register", NULL
};
#endif

static const short yyr1[] = {     0,
    36,    37,    37,    37,    37,    37,    37,    38,    39,    39,
    40,    40,    40,    40,    40,    40,    41,    41,    42,    42,
    43,    43,    43,    43,    43,    43,    43,    43,    44,    44,
    44,    44,    44,    44,    44,    44,    45,    46,    46,    47,
    48,    49,    49,    49,    49,    49,    50,    50,    50,    50,
    50,    50,    50,    50,    50,    51,    51,    51,    51,    51,
    51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
    51,    51,    51,    51,    51,    52,    53,    54,    54,    54,
    54,    54,    54,    54,    55,    56,    56
};

static const short yyr2[] = {     0,
     1,     3,     4,     2,     2,     3,     1,    12,     2,     1,
     4,     5,     6,     3,     4,     5,     5,     4,     1,     1,
     1,     2,     4,     5,     4,     5,     4,     4,     2,     2,
     2,     1,     2,     3,     3,     1,     6,     6,     4,     6,
     6,     4,     4,     4,     4,     4,     1,     4,     4,     1,
     4,     4,     1,     4,     4,     2,     3,     3,     4,     4,
     2,     3,     3,     4,     4,     1,     2,     2,     3,     3,
     1,     2,     2,     3,     3,     4,     6,    13,    13,    11,
     6,     8,     4,     6,     4,     1,     1
};

static const short yydefact[] = {     0,
     0,     0,     0,     0,     0,     0,     1,     0,     0,    10,
     7,     0,     0,    66,    71,     0,     0,    19,    20,     0,
     0,     0,     0,     0,     0,     0,     0,     5,     9,     4,
     0,    67,    72,     0,    68,    73,    61,    56,     0,    87,
    86,    50,    53,     0,     0,     0,    47,     0,     0,    14,
     0,     0,     0,     0,     0,     0,     0,     6,     2,    69,
    74,    62,    57,    70,    75,    63,    58,     0,     0,     0,
     0,     0,    15,     0,    11,     0,    32,    36,     0,    76,
     0,    83,     0,     0,    85,     3,    64,    59,    65,    60,
     0,     0,     0,     0,     0,     0,     0,     0,    16,     0,
    12,    18,     0,     0,     0,     0,     0,     0,    29,    30,
    31,    33,     0,     0,     0,     0,     0,     0,    51,    54,
    48,    52,    55,    49,    77,    13,     0,     0,     0,     0,
     0,    17,    34,    35,     0,     0,     0,     0,     0,     0,
     0,    21,     0,    81,     0,     0,    84,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    22,    39,     0,     0,     0,     0,     0,     0,    42,    43,
    44,    45,    46,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    82,     0,     0,     0,     0,    23,
    25,    27,    28,     0,     0,    37,    38,     0,     0,     0,
     0,     0,    24,    26,     0,     0,     0,    40,    41,    80,
     0,     0,     8,     0,     0,    78,    79,     0,     0,     0
};

static const short yydefgoto[] = {   218,
     7,     8,     9,    10,    21,    22,   141,    76,    77,    78,
   133,   134,   108,    46,    11,    12,    13,    14,    15,    47
};

static const short yypact[] = {     4,
   -17,   -15,    26,    17,    -6,    19,-32768,     4,    68,-32768,
-32768,     8,   113,    57,    67,    58,   133,-32768,-32768,    26,
    98,    93,   104,    66,   133,    68,    68,-32768,-32768,-32768,
    44,    57,    67,    44,    57,    67,-32768,-32768,   118,-32768,
-32768,-32768,-32768,   144,   146,   153,-32768,    81,   127,-32768,
   150,   100,   157,   162,    31,   159,    68,-32768,-32768,    57,
    67,-32768,-32768,    57,    67,-32768,-32768,   161,   152,   160,
   133,   136,-32768,   165,-32768,    97,   100,   100,   155,-32768,
   133,-32768,   133,    77,-32768,-32768,-32768,-32768,-32768,-32768,
   145,   168,   169,   170,   172,   173,   174,   175,-32768,   177,
-32768,-32768,   176,   178,   181,   182,   183,   184,-32768,-32768,
-32768,   100,   125,   179,    27,   185,   186,   187,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   188,   190,   192,   193,
   194,-32768,-32768,-32768,   189,   196,   197,   199,   200,    85,
   148,-32768,   133,-32768,   133,   133,-32768,   166,   198,   201,
   202,   203,   204,   106,   100,   100,   100,   100,   209,   210,
-32768,-32768,   125,   125,   206,   208,   211,   212,-32768,-32768,
-32768,-32768,-32768,   213,   214,   217,   218,   219,   220,   100,
   100,   221,   222,   133,-32768,   133,   195,   223,   226,-32768,
-32768,-32768,-32768,   227,   228,-32768,-32768,   229,   230,   231,
   232,   233,-32768,-32768,    39,   133,   234,-32768,-32768,-32768,
   133,   238,-32768,   236,   237,-32768,-32768,   216,   246,-32768
};

static const short yypgoto[] = {-32768,
-32768,    76,     3,    -3,    -4,-32768,   -12,-32768,    94,    96,
-32768,-32768,-32768,   -24,    40,   205,   180,     0,    -9,   -50
};


#define	YYLAST		256


static const short yytable[] = {    55,
    56,    79,    33,    36,    37,    29,    16,     1,    17,     2,
    27,    32,    35,     2,    38,    49,    51,    24,    94,    97,
     3,    61,    62,    29,    65,    66,    79,    79,    57,     1,
    60,    23,    63,    64,     4,    67,     5,     6,    18,    19,
     5,     6,    25,    72,    74,   144,    98,    28,    30,    82,
    87,   145,    83,    29,    89,    84,   114,   210,   115,   117,
    88,   135,   142,   211,    90,    58,    59,   100,    40,    41,
    42,    43,    39,     2,    44,    45,     5,     6,    20,    40,
    41,    42,    43,    26,     3,    44,    45,    40,    41,   161,
     6,   159,   160,    18,    19,    48,    86,    54,     4,     5,
     5,     6,    40,    41,   176,   177,   178,   179,   116,    52,
    18,    19,   142,   142,   102,    50,   174,   175,   165,    53,
   166,   167,   103,   104,   105,   106,   107,    40,    41,   194,
   195,   136,   137,   138,   139,    40,    41,    42,    43,    18,
    19,    44,    45,     4,    73,     5,     6,   140,    18,    19,
   182,   183,    68,    99,    40,    41,    92,    93,    69,   198,
    70,   199,    40,    41,    95,    96,   162,    75,   163,   164,
   109,   111,   110,   112,    71,    80,    81,    85,   113,   118,
    91,   212,   101,   119,   120,   121,   214,   122,   123,   124,
   127,    31,   128,   125,   126,   129,   130,   131,   143,   146,
   168,   132,     0,   149,   147,   150,   148,   151,   152,   153,
   155,   156,   154,   157,   158,   219,   169,    34,     0,   170,
   171,   172,   173,   180,   181,   184,   185,   188,   189,   200,
   186,   187,   190,   191,   192,   193,     0,     0,   201,   196,
   197,   202,   203,   204,   205,   220,   207,     0,     0,   206,
   208,   209,   213,   215,   216,   217
};

static const short yycheck[] = {    24,
    25,    52,    12,    13,    14,     9,    24,     4,    24,     6,
     8,    12,    13,     6,    15,    20,    21,    24,    69,    70,
    17,    31,    32,    27,    34,    35,    77,    78,    26,     4,
    31,    15,    33,    34,    31,    36,    33,    34,    13,    14,
    33,    34,    24,    48,    49,    19,    71,     8,     9,    19,
    60,    25,    22,    57,    64,    25,    81,    19,    83,    84,
    61,   112,   113,    25,    65,    26,    27,    72,     3,     4,
     5,     6,    15,     6,     9,    10,    33,    34,     3,     3,
     4,     5,     6,     8,    17,     9,    10,     3,     4,   140,
    34,     7,     8,    13,    14,    20,    57,    32,    31,    33,
    33,    34,     3,     4,   155,   156,   157,   158,    32,    17,
    13,    14,   163,   164,    18,    18,    11,    12,   143,    16,
   145,   146,    26,    27,    28,    29,    30,     3,     4,   180,
   181,     7,     8,     9,    10,     3,     4,     5,     6,    13,
    14,     9,    10,    31,    18,    33,    34,    23,    13,    14,
   163,   164,    35,    18,     3,     4,     5,     6,    15,   184,
    15,   186,     3,     4,     5,     6,    19,    18,    21,    22,
    77,    78,    77,    78,    22,    19,    15,    19,    24,    35,
    20,   206,    18,    16,    16,    16,   211,    16,    16,    16,
    15,    12,    15,    19,    18,    15,    15,    15,    20,    15,
    35,    18,    -1,    16,    19,    16,    20,    16,    16,    16,
    15,    15,    24,    15,    15,     0,    19,    13,    -1,    19,
    19,    19,    19,    15,    15,    20,    19,    15,    15,    35,
    20,    20,    16,    16,    16,    16,    -1,    -1,    16,    19,
    19,    16,    16,    16,    16,     0,    16,    -1,    -1,    20,
    19,    19,    19,    16,    19,    19
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
#line 73 "rc1.0_grammar.y"
{
			yyvsp[0].combinersStruct.Validate();
			yyvsp[0].combinersStruct.Invoke();
		;
    break;}
case 2:
#line 80 "rc1.0_grammar.y"
{
			CombinersStruct combinersStruct;
			combinersStruct.Init(yyvsp[-1].generalCombinersStruct, yyvsp[0].finalCombinerStruct, yyvsp[-2].constColorStruct);
			yyval.combinersStruct = combinersStruct;
		;
    break;}
case 3:
#line 86 "rc1.0_grammar.y"
{
			CombinersStruct combinersStruct;
			combinersStruct.Init(yyvsp[-1].generalCombinersStruct, yyvsp[0].finalCombinerStruct, yyvsp[-3].constColorStruct, yyvsp[-2].constColorStruct);
			yyval.combinersStruct = combinersStruct;
		;
    break;}
case 4:
#line 92 "rc1.0_grammar.y"
{
			CombinersStruct combinersStruct;
			combinersStruct.Init(yyvsp[-1].generalCombinersStruct, yyvsp[0].finalCombinerStruct);
			yyval.combinersStruct = combinersStruct;
		;
    break;}
case 5:
#line 98 "rc1.0_grammar.y"
{
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, yyvsp[0].finalCombinerStruct, yyvsp[-1].constColorStruct);
			yyval.combinersStruct = combinersStruct;
		;
    break;}
case 6:
#line 106 "rc1.0_grammar.y"
{
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, yyvsp[0].finalCombinerStruct, yyvsp[-2].constColorStruct, yyvsp[-1].constColorStruct);
			yyval.combinersStruct = combinersStruct;
		;
    break;}
case 7:
#line 114 "rc1.0_grammar.y"
{
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init();
			CombinersStruct combinersStruct;
			combinersStruct.Init(generalCombinersStruct, yyvsp[0].finalCombinerStruct);
			yyval.combinersStruct = combinersStruct;
		;
    break;}
case 8:
#line 124 "rc1.0_grammar.y"
{
			ConstColorStruct constColorStruct;
			constColorStruct.Init(yyvsp[-11].registerEnum, yyvsp[-8].fval, yyvsp[-6].fval, yyvsp[-4].fval, yyvsp[-2].fval);
			yyval.constColorStruct = constColorStruct;
		;
    break;}
case 9:
#line 133 "rc1.0_grammar.y"
{
			yyvsp[-1].generalCombinersStruct += yyvsp[0].generalCombinerStruct;
			yyval.generalCombinersStruct = yyvsp[-1].generalCombinersStruct;
		;
    break;}
case 10:
#line 138 "rc1.0_grammar.y"
{
			GeneralCombinersStruct generalCombinersStruct;
			generalCombinersStruct.Init(yyvsp[0].generalCombinerStruct);
			yyval.generalCombinersStruct = generalCombinersStruct;
		;
    break;}
case 11:
#line 146 "rc1.0_grammar.y"
{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-2].generalPortionStruct, yyvsp[-1].generalPortionStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;
    break;}
case 12:
#line 152 "rc1.0_grammar.y"
{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-2].generalPortionStruct, yyvsp[-1].generalPortionStruct, yyvsp[-3].constColorStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;
    break;}
case 13:
#line 158 "rc1.0_grammar.y"
{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-2].generalPortionStruct, yyvsp[-1].generalPortionStruct, yyvsp[-4].constColorStruct, yyvsp[-3].constColorStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;
    break;}
case 14:
#line 164 "rc1.0_grammar.y"
{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-1].generalPortionStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;
    break;}
case 15:
#line 170 "rc1.0_grammar.y"
{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-1].generalPortionStruct, yyvsp[-2].constColorStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;
    break;}
case 16:
#line 176 "rc1.0_grammar.y"
{
			GeneralCombinerStruct generalCombinerStruct;
			generalCombinerStruct.Init(yyvsp[-1].generalPortionStruct, yyvsp[-3].constColorStruct, yyvsp[-2].constColorStruct);
			yyval.generalCombinerStruct = generalCombinerStruct;
		;
    break;}
case 17:
#line 184 "rc1.0_grammar.y"
{
			GeneralPortionStruct generalPortionStruct;
			generalPortionStruct.Init(yyvsp[-4].ival, yyvsp[-2].generalFunctionStruct, yyvsp[-1].biasScaleEnum);
			yyval.generalPortionStruct = generalPortionStruct;
		;
    break;}
case 18:
#line 190 "rc1.0_grammar.y"
{
			BiasScaleEnum noScale;
			noScale.word = RCP_SCALE_BY_ONE;
			GeneralPortionStruct generalPortionStruct;
			generalPortionStruct.Init(yyvsp[-3].ival, yyvsp[-1].generalFunctionStruct, noScale);
			yyval.generalPortionStruct = generalPortionStruct;
		;
    break;}
case 19:
#line 200 "rc1.0_grammar.y"
{
			yyval.ival = yyvsp[0].ival;
		;
    break;}
case 20:
#line 204 "rc1.0_grammar.y"
{
			yyval.ival = yyvsp[0].ival;
		;
    break;}
case 21:
#line 210 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[0].registerEnum, GL_SIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 22:
#line 216 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[0].registerEnum, GL_SIGNED_NEGATE_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 23:
#line 222 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_EXPAND_NORMAL_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 24:
#line 228 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_EXPAND_NEGATE_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 25:
#line 234 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_HALF_BIAS_NORMAL_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 26:
#line 240 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_HALF_BIAS_NEGATE_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 27:
#line 246 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 28:
#line 252 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_INVERT_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 29:
#line 260 "rc1.0_grammar.y"
{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;
    break;}
case 30:
#line 266 "rc1.0_grammar.y"
{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;
    break;}
case 31:
#line 272 "rc1.0_grammar.y"
{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;
    break;}
case 32:
#line 278 "rc1.0_grammar.y"
{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;
    break;}
case 33:
#line 284 "rc1.0_grammar.y"
{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;
    break;}
case 34:
#line 290 "rc1.0_grammar.y"
{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-2].opStruct, yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;
    break;}
case 35:
#line 296 "rc1.0_grammar.y"
{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[-2].opStruct, yyvsp[-1].opStruct, yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;
    break;}
case 36:
#line 302 "rc1.0_grammar.y"
{
			GeneralFunctionStruct generalFunction;
			generalFunction.Init(yyvsp[0].opStruct);
			yyval.generalFunctionStruct = generalFunction;
		;
    break;}
case 37:
#line 310 "rc1.0_grammar.y"
{
			OpStruct dotFunction;
			dotFunction.Init(RCP_DOT, yyvsp[-5].registerEnum, yyvsp[-3].mappedRegisterStruct, yyvsp[-1].mappedRegisterStruct);
			yyval.opStruct = dotFunction;
		;
    break;}
case 38:
#line 318 "rc1.0_grammar.y"
{
			OpStruct mulFunction;
			mulFunction.Init(RCP_MUL, yyvsp[-5].registerEnum, yyvsp[-3].mappedRegisterStruct, yyvsp[-1].mappedRegisterStruct);
			yyval.opStruct = mulFunction;
		;
    break;}
case 39:
#line 324 "rc1.0_grammar.y"
{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct one;
			one.Init(zero, GL_UNSIGNED_INVERT_NV);
			OpStruct mulFunction;
			mulFunction.Init(RCP_MUL, yyvsp[-3].registerEnum, yyvsp[-1].mappedRegisterStruct, one);
			yyval.opStruct = mulFunction;
		;
    break;}
case 40:
#line 336 "rc1.0_grammar.y"
{
			OpStruct muxFunction;
			muxFunction.Init(RCP_MUX, yyvsp[-5].registerEnum);
			yyval.opStruct = muxFunction;
		;
    break;}
case 41:
#line 344 "rc1.0_grammar.y"
{
			OpStruct sumFunction;
			sumFunction.Init(RCP_SUM, yyvsp[-5].registerEnum);
			yyval.opStruct = sumFunction;
		;
    break;}
case 42:
#line 352 "rc1.0_grammar.y"
{
			yyval.biasScaleEnum = yyvsp[-3].biasScaleEnum;
		;
    break;}
case 43:
#line 356 "rc1.0_grammar.y"
{
			yyval.biasScaleEnum = yyvsp[-3].biasScaleEnum;
		;
    break;}
case 44:
#line 360 "rc1.0_grammar.y"
{
			yyval.biasScaleEnum = yyvsp[-3].biasScaleEnum;
		;
    break;}
case 45:
#line 364 "rc1.0_grammar.y"
{
			yyval.biasScaleEnum = yyvsp[-3].biasScaleEnum;
		;
    break;}
case 46:
#line 368 "rc1.0_grammar.y"
{
			yyval.biasScaleEnum = yyvsp[-3].biasScaleEnum;
		;
    break;}
case 47:
#line 374 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[0].registerEnum, GL_UNSIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 48:
#line 380 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 49:
#line 386 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_INVERT_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 50:
#line 392 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[0].registerEnum);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 51:
#line 398 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 52:
#line 404 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_INVERT_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 53:
#line 410 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[0].registerEnum);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 54:
#line 416 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_IDENTITY_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 55:
#line 422 "rc1.0_grammar.y"
{
			MappedRegisterStruct reg;
			reg.Init(yyvsp[-1].registerEnum, GL_UNSIGNED_INVERT_NV);
			yyval.mappedRegisterStruct = reg;
		;
    break;}
case 56:
#line 430 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, yyvsp[-1].finalAlphaFunctionStruct, false);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 57:
#line 436 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, yyvsp[-1].finalAlphaFunctionStruct, true);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 58:
#line 442 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, yyvsp[-1].finalAlphaFunctionStruct, false, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 59:
#line 448 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, yyvsp[-1].finalAlphaFunctionStruct, true, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 60:
#line 454 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, yyvsp[-1].finalAlphaFunctionStruct, true, yyvsp[-3].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 61:
#line 461 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[-1].finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, false);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 62:
#line 467 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[-1].finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 63:
#line 473 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[-1].finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, false, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 64:
#line 479 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[-1].finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 65:
#line 485 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			finalCombinerStruct.Init(yyvsp[-1].finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true, yyvsp[-3].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 66:
#line 492 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, finalAlphaFunctionStruct, false);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 67:
#line 500 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, finalAlphaFunctionStruct, true);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 68:
#line 508 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, finalAlphaFunctionStruct, false, yyvsp[-1].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 69:
#line 516 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, finalAlphaFunctionStruct, true, yyvsp[-1].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 70:
#line 524 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(yyvsp[0].finalRgbFunctionStruct, finalAlphaFunctionStruct, true, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 71:
#line 533 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, false);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 72:
#line 541 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 73:
#line 549 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, false, yyvsp[-1].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 74:
#line 557 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true, yyvsp[-1].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 75:
#line 565 "rc1.0_grammar.y"
{
			FinalCombinerStruct finalCombinerStruct;
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.ZeroOut();
			finalCombinerStruct.Init(finalRgbFunctionStruct, yyvsp[0].finalAlphaFunctionStruct, true, yyvsp[-2].finalProductStruct);
			yyval.finalCombinerStruct = finalCombinerStruct;
		;
    break;}
case 76:
#line 575 "rc1.0_grammar.y"
{
			yyval.ival = yyvsp[-3].ival;
		;
    break;}
case 77:
#line 581 "rc1.0_grammar.y"
{
			FinalProductStruct finalProductStruct;
			finalProductStruct.Init(yyvsp[-3].mappedRegisterStruct, yyvsp[-1].mappedRegisterStruct);
			yyval.finalProductStruct = finalProductStruct;
		;
    break;}
case 78:
#line 589 "rc1.0_grammar.y"
{
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-8].mappedRegisterStruct, yyvsp[-6].mappedRegisterStruct, yyvsp[-4].mappedRegisterStruct, yyvsp[-1].mappedRegisterStruct);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;
    break;}
case 79:
#line 595 "rc1.0_grammar.y"
{
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-6].mappedRegisterStruct, yyvsp[-4].mappedRegisterStruct, yyvsp[-2].mappedRegisterStruct, yyvsp[-10].mappedRegisterStruct);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;
    break;}
case 80:
#line 601 "rc1.0_grammar.y"
{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg;
			reg.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-6].mappedRegisterStruct, yyvsp[-4].mappedRegisterStruct, yyvsp[-2].mappedRegisterStruct, reg);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;
    break;}
case 81:
#line 611 "rc1.0_grammar.y"
{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-3].mappedRegisterStruct, yyvsp[-1].mappedRegisterStruct, reg1, reg2);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;
    break;}
case 82:
#line 623 "rc1.0_grammar.y"
{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-5].mappedRegisterStruct, yyvsp[-3].mappedRegisterStruct, reg1, yyvsp[-1].mappedRegisterStruct);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;
    break;}
case 83:
#line 633 "rc1.0_grammar.y"
{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg1;
			reg1.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			MappedRegisterStruct reg3;
			reg3.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(reg1, reg2, reg3, yyvsp[-1].mappedRegisterStruct);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;
    break;}
case 84:
#line 647 "rc1.0_grammar.y"
{
			RegisterEnum zero;
			zero.word = RCP_ZERO;
			MappedRegisterStruct reg2;
			reg2.Init(zero, GL_UNSIGNED_INVERT_NV);
			MappedRegisterStruct reg3;
			reg3.Init(zero, GL_UNSIGNED_IDENTITY_NV);
			FinalRgbFunctionStruct finalRgbFunctionStruct;
			finalRgbFunctionStruct.Init(yyvsp[-3].mappedRegisterStruct, reg2, reg3, yyvsp[-1].mappedRegisterStruct);
			yyval.finalRgbFunctionStruct = finalRgbFunctionStruct;
		;
    break;}
case 85:
#line 661 "rc1.0_grammar.y"
{
			FinalAlphaFunctionStruct finalAlphaFunctionStruct;
			finalAlphaFunctionStruct.Init(yyvsp[-1].mappedRegisterStruct);
			yyval.finalAlphaFunctionStruct = finalAlphaFunctionStruct;
		;
    break;}
case 86:
#line 669 "rc1.0_grammar.y"
{
			yyval.registerEnum = yyvsp[0].registerEnum;
		;
    break;}
case 87:
#line 673 "rc1.0_grammar.y"
{
			yyval.registerEnum = yyvsp[0].registerEnum;
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
#line 678 "rc1.0_grammar.y"

void yyerror(char* s)
{
     errors.set("unrecognized token");
}

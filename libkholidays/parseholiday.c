
/*  A Bison parser, made from parseholiday.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse kcalparse
#define yylex kcallex
#define yyerror kcalerror
#define yylval kcallval
#define yychar kcalchar
#define yydebug kcaldebug
#define yynerrs kcalnerrs
#define	NUMBER	257
#define	MONTH	258
#define	WDAY	259
#define	COLOR	260
#define	STRING	261
#define	IN	262
#define	PLUS	263
#define	MINUS	264
#define	SMALL	265
#define	CYEAR	266
#define	LEAPYEAR	267
#define	SHIFT	268
#define	IF	269
#define	LENGTH	270
#define	EASTER	271
#define	EQ	272
#define	NE	273
#define	LE	274
#define	GE	275
#define	LT	276
#define	GT	277
#define	OR	278
#define	AND	279
#define	UMINUS	280

#line 1 "parseholiday.y"

/*
 * deals with the holiday file. A yacc parser is used to parse the file.
 * All the holidays of the specified year are calculated at once and stored
 * in two arrays that have one entry for each day of the year. The day
 * drawing routines just use the julian date to index into these arrays.
 * There are two arrays because holidays can be printed either on a full
 * line under the day number, or as a small line to the right of the day
 * number. It's convenient to have both.
 *
 *	parse_holidays(year, force)	read the holiday file and evaluate
 *					all the holiday definitions for
 *					<year>. Sets holiday and sm_holiday
 *					arrays. If force is set, re-eval even
 *					if year is the same as last time.
 *
 * Taken from plan by Thomas Driemeyer <thomas@bitrot.de>
 * Adapted for use in KOrganizer by Preston Brown <pbrown@kde.org> and
 * Reinhold Kainhofer <reinhold@kainhofer.com>
 */

#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <limits.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

/*** Macro definitions and constants ***/
/*
 * Before you mail and complain that the following macro is incorrect,
 * please consider that this is one of the main battlegrounds of the
 * Annual Usenet Flame Wars. 2000 is a leap year. Just trust me on this :-)
 */

#define ISLEAPYEAR(y)	!((y)&3)
#define JULIAN(m,d)	(monthbegin[m] + (d)-1+((m)>1 && ISLEAPYEAR(parse_year)))
#define LAST		999
#define ANY		0
#define	BEFORE		-1
#define AFTER		-2
/**** Public forward declarations  ****/
char *parse_holidays(const char *holidays, int year, short force);

/**** Private forward declarations ****/
extern int       kcallex(void);          /* external lexical analyzer */
static void      kcalerror(const char *s);
static time_t    date_to_time(int day, int month, int year, 
			      int *wkday, int *julian, int *weeknum);
static time_t    tm_to_time(struct tm *tm);
static int	 day_from_name(char *str);
static int	 day_from_easter(void);
static int	 day_from_monthday(int month, int day);
static int	 day_from_wday(int day, int wday, int num);
static void	 monthday_from_day(int day, int *m, int *d, int *y);
static int       calc_easter(int year);
static void      setliteraldate(int month, int day, int off, int *ddup);
static void      seteaster(int off, int length);
static void      setdate(int month, int day, int year, int off, int conditionaloff, int length);
static void      setwday(int num, int wday, int month, int off, int length); 
static void      setdoff(int wday, int rel, int month, int day, 
			 int year, int off, int length);
/*** Variables and structures ***/
static int	 m, d, y;
int              kcallineno;	       	/* current line # being parsed */
FILE            *kcalin;                  /* file currently being processed */
int	         yacc_small;		/* small string or on its own line? */
int	         yacc_stringcolor;	/* color of holiday name text, 1..8 */
char	        *yacc_string;		/* holiday name text */
int	         yacc_daycolor;		/* color of day number, 1..8 */
char	        *progname;		/* argv[0] */
int	         parse_year = -1;	/* year being parsed, 0=1970..99=2069*/
static const char *filename;		/* holiday filename */
static char	 errormsg[PATH_MAX+200];/* error message if any, or "" */
static int	 easter_julian;		/* julian date of Easter Sunday */
static char	*holiday_name;		/* strdup'd yacc_string */
short 	         monthlen[12] = { 31, 28, 31, 30, 
				 31, 30, 31, 31,
				 30, 31, 30, 31 };
short	         monthbegin[12] = { 0, 31, 59, 90,
				    120, 151, 181, 
				    212, 243, 273,
				    304, 334 };

/* struct holiday;*/
struct holiday {
  char            *string;        /* name of holiday, 0=not a holiday */
  int             color;
  unsigned short  dup;            /* reference count */
  struct holiday         *next;
};

struct holiday	 holidays[366];		/* info for each day, separate for */
/*struct holiday   sm_holiday[366];*/	/* full-line texts under, and small */
					/* texts next to day number */
static int	initialized=0;

#line 109 "parseholiday.y"
typedef union { int ival; char *sval; } YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		129
#define	YYFLAG		-32768
#define	YYNTBASE	40

#define YYTRANSLATE(x) ((unsigned)(x) <= 280 ? yytranslate[x] : 55)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    31,     2,     2,     2,    30,     2,     2,    38,
    39,    28,    27,     2,    26,    35,    29,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    34,     2,     2,
     2,     2,    33,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    36,     2,    37,     2,     2,     2,     2,     2,     2,     2,
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
    17,    18,    19,    20,    21,    22,    23,    24,    25,    32
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     2,    10,    11,    13,    14,    16,    20,    25,
    29,    34,    41,    47,    48,    51,    54,    55,    60,    61,
    63,    67,    68,    71,    75,    80,    86,    90,    96,    99,
   103,   106,   110,   115,   117,   119,   121,   125,   130,   134,
   137,   140,   144,   149,   151,   153,   155,   159,   163,   167,
   171,   175,   179,   183,   187,   191,   195,   199,   203,   207,
   213,   216,   220,   224,   226,   228,   231,   233
};

static const short yyrhs[] = {    -1,
     0,    40,    42,    43,     7,    43,    41,    44,     0,     0,
    11,     0,     0,     6,     0,    17,    45,    48,     0,    49,
    45,    46,    48,     0,     5,    45,    48,     0,    53,     5,
    45,    48,     0,    53,     5,     8,    51,    45,    48,     0,
     5,    53,    49,    45,    48,     0,     0,     9,    52,     0,
    10,    52,     0,     0,    14,    47,    15,    47,     0,     0,
     5,     0,     5,    24,    47,     0,     0,    16,    52,     0,
    53,    35,    51,     0,    53,    35,    51,    35,     0,    53,
    35,    51,    35,    52,     0,    51,    29,    53,     0,    51,
    29,    53,    29,    53,     0,     4,    53,     0,     4,    53,
    53,     0,    53,     4,     0,    53,     4,    53,     0,    53,
    35,     4,    53,     0,    53,     0,     7,     0,    17,     0,
    53,    35,    51,     0,    53,    35,    51,    35,     0,    51,
    29,    53,     0,    53,     4,     0,     4,    53,     0,     5,
    53,    53,     0,    53,     5,     8,    51,     0,     4,     0,
    53,     0,    53,     0,    52,    24,    52,     0,    52,    25,
    52,     0,    52,    18,    52,     0,    52,    19,    52,     0,
    52,    20,    52,     0,    52,    21,    52,     0,    52,    22,
    52,     0,    52,    23,    52,     0,    52,    27,    52,     0,
    52,    26,    52,     0,    52,    28,    52,     0,    52,    29,
    52,     0,    52,    30,    52,     0,    52,    33,    52,    34,
    52,     0,    31,    52,     0,    36,    50,    37,     0,    38,
    52,    39,     0,    54,     0,     3,     0,    26,     3,     0,
    12,     0,    13,    53,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   128,   129,   132,   135,   136,   139,   140,   143,   144,   145,
   146,   147,   148,   151,   152,   153,   156,   157,   160,   161,
   162,   165,   166,   169,   170,   171,   172,   173,   174,   175,
   176,   177,   178,   179,   183,   184,   185,   187,   189,   191,
   193,   195,   197,   203,   203,   205,   206,   207,   208,   209,
   210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
   220,   221,   224,   225,   228,   229,   230,   231
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","NUMBER",
"MONTH","WDAY","COLOR","STRING","IN","PLUS","MINUS","SMALL","CYEAR","LEAPYEAR",
"SHIFT","IF","LENGTH","EASTER","EQ","NE","LE","GE","LT","GT","OR","AND","'-'",
"'+'","'*'","'/'","'%'","'!'","UMINUS","'?'","':'","'.'","'['","']'","'('","')'",
"list","@1","small","color","entry","offset","conditionaloffset","wdaycondition",
"length","date","reldate","month","expr","pexpr","number", NULL
};
#endif

static const short yyr1[] = {     0,
    40,    41,    40,    42,    42,    43,    43,    44,    44,    44,
    44,    44,    44,    45,    45,    45,    46,    46,    47,    47,
    47,    48,    48,    49,    49,    49,    49,    49,    49,    49,
    49,    49,    49,    49,    50,    50,    50,    50,    50,    50,
    50,    50,    50,    51,    51,    52,    52,    52,    52,    52,
    52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
    52,    52,    53,    53,    54,    54,    54,    54
};

static const short yyr2[] = {     0,
     0,     0,     7,     0,     1,     0,     1,     3,     4,     3,
     4,     6,     5,     0,     2,     2,     0,     4,     0,     1,
     3,     0,     2,     3,     4,     5,     3,     5,     2,     3,
     2,     3,     4,     1,     1,     1,     3,     4,     3,     2,
     2,     3,     4,     1,     1,     1,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
     2,     3,     3,     1,     1,     2,     1,     2
};

static const short yydefact[] = {     1,
     4,     5,     6,     7,     0,     6,     2,     0,    65,    44,
    14,    67,     0,    14,     0,     0,     3,    14,     0,    34,
    64,    29,     0,     0,    22,     0,    68,    22,    66,     0,
     0,     0,    46,    17,     0,    31,    14,     0,    30,    15,
    16,     0,    10,    14,    34,     8,    61,    44,     0,    35,
    36,     0,     0,    45,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    63,    19,
    22,    27,    32,     0,    22,    44,    24,    45,    23,    22,
    41,     0,    62,     0,    40,     0,     0,    49,    50,    51,
    52,    53,    54,    47,    48,    56,    55,    57,    58,    59,
     0,    20,     0,     9,     0,    44,    14,    11,    33,    25,
    13,    42,    39,     0,    37,     0,    19,    19,    28,    22,
    26,    43,    38,    60,    21,    18,    12,     0,     0
};

static const short yydefgoto[] = {     1,
     8,     3,     5,    17,    25,    71,   103,    43,    18,    52,
    19,    32,    33,    21
};

static const short yypact[] = {-32768,
     4,-32768,    -5,-32768,    -1,    -5,-32768,    97,-32768,   148,
   112,-32768,   148,     0,    22,   127,-32768,     0,     5,     3,
-32768,   148,   127,   127,     8,   129,-32768,     8,-32768,   127,
    81,   169,-32768,    15,   148,   148,    12,   133,-32768,   208,
   208,   127,-32768,     0,     7,-32768,    10,   148,   148,-32768,
-32768,    -4,    30,    57,   127,   127,   127,   127,   127,   127,
   127,   127,   127,   127,   127,   127,   127,   127,-32768,    32,
     8,    34,-32768,   144,     8,   148,    45,-32768,   208,     8,
-32768,   148,-32768,   148,-32768,    56,   144,   240,   240,   240,
   240,   240,   240,   224,   240,    42,    42,    10,    10,    10,
   191,    53,    66,-32768,   148,-32768,     0,-32768,-32768,   127,
-32768,-32768,-32768,   144,    47,   127,    32,    32,-32768,     8,
   208,-32768,-32768,-32768,-32768,-32768,-32768,    83,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,    84,-32768,    21,-32768,  -101,    -2,    61,-32768,
    29,   -11,    -8,-32768
};


#define	YYLAST		273


static const short yytable[] = {    20,
     4,    22,    26,   128,    27,     6,    36,    37,    23,    24,
    36,    40,    41,    39,     2,   125,   126,    45,    47,    74,
    23,    24,    54,    42,    29,    46,    72,    73,    70,    78,
    79,   -45,    83,    35,    28,   -45,   102,    38,    34,    81,
    82,    38,    68,    88,    89,    90,    91,    92,    93,    94,
    95,    96,    97,    98,    99,   100,   101,    75,    84,    53,
    85,    86,   105,   114,    80,    78,    77,   109,   104,    65,
    66,    67,   108,   112,    68,   113,   117,   111,    78,   110,
   118,   123,   129,     9,    48,    49,    44,    50,     0,     7,
     0,    87,    12,    13,     0,     0,   119,    51,   121,     9,
    10,    11,   107,     0,   124,    78,    15,     0,    12,    13,
     0,     0,     0,    14,     9,   115,     0,   127,    16,     0,
    23,    24,    15,    12,    13,     0,     0,   120,     0,     9,
     0,     9,    10,     0,    16,     9,    76,    15,    12,    13,
    12,    13,   122,     0,    12,    13,     9,   106,     0,    16,
     9,     0,    15,     0,    15,    12,    13,    30,    15,    12,
    13,     0,    31,     0,    16,     0,    16,     0,     0,    15,
    16,     0,     0,    15,     0,     0,     0,     0,     0,     0,
     0,    16,     0,     0,     0,    16,    55,    56,    57,    58,
    59,    60,    61,    62,    63,    64,    65,    66,    67,     0,
     0,    68,     0,     0,     0,     0,     0,    69,    55,    56,
    57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
    67,     0,     0,    68,   116,    55,    56,    57,    58,    59,
    60,    61,    62,    63,    64,    65,    66,    67,     0,     0,
    68,    55,    56,    57,    58,    59,    60,     0,    62,    63,
    64,    65,    66,    67,     0,     0,    68,    55,    56,    57,
    58,    59,    60,     0,     0,    63,    64,    65,    66,    67,
     0,     0,    68
};

static const short yycheck[] = {     8,
     6,    10,    11,     0,    13,     7,     4,     5,     9,    10,
     4,    23,    24,    22,    11,   117,   118,    26,    30,     8,
     9,    10,    31,    16,     3,    28,    35,    36,    14,    38,
    42,    29,    37,    29,    14,    29,     5,    35,    18,    48,
    49,    35,    33,    55,    56,    57,    58,    59,    60,    61,
    62,    63,    64,    65,    66,    67,    68,    37,    29,    31,
     4,     5,    29,     8,    44,    74,    38,    76,    71,    28,
    29,    30,    75,    82,    33,    84,    24,    80,    87,    35,
    15,    35,     0,     3,     4,     5,    26,     7,    -1,     6,
    -1,    35,    12,    13,    -1,    -1,   105,    17,   110,     3,
     4,     5,    74,    -1,   116,   114,    26,    -1,    12,    13,
    -1,    -1,    -1,    17,     3,    87,    -1,   120,    38,    -1,
     9,    10,    26,    12,    13,    -1,    -1,   107,    -1,     3,
    -1,     3,     4,    -1,    38,     3,     4,    26,    12,    13,
    12,    13,   114,    -1,    12,    13,     3,     4,    -1,    38,
     3,    -1,    26,    -1,    26,    12,    13,    31,    26,    12,
    13,    -1,    36,    -1,    38,    -1,    38,    -1,    -1,    26,
    38,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    38,    -1,    -1,    -1,    38,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    -1,
    -1,    33,    -1,    -1,    -1,    -1,    -1,    39,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    -1,    -1,    33,    34,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29,    30,    -1,    -1,
    33,    18,    19,    20,    21,    22,    23,    -1,    25,    26,
    27,    28,    29,    30,    -1,    -1,    33,    18,    19,    20,
    21,    22,    23,    -1,    -1,    26,    27,    28,    29,    30,
    -1,    -1,    33
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif


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

#line 222 "/usr/share/bison.simple"

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
YYPARSE_RETURN_TYPE
yyparse (void *);
#else
YYPARSE_RETURN_TYPE
yyparse (void);
#endif
#endif

YYPARSE_RETURN_TYPE
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
#ifndef YYSTACK_USE_ALLOCA
  int yyfree_stacks = 0;
#endif

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
#ifndef YYSTACK_USE_ALLOCA
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
#endif	    
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

case 2:
#line 129 "parseholiday.y"
{ yacc_stringcolor = yyvsp[-2].ival;
						  yacc_string	= yyvsp[-1].sval;
						  yacc_daycolor	= yyvsp[0].ival; ;
    break;}
case 3:
#line 132 "parseholiday.y"
{ free(yacc_string); ;
    break;}
case 4:
#line 135 "parseholiday.y"
{ yacc_small = 0; ;
    break;}
case 5:
#line 136 "parseholiday.y"
{ yacc_small = 1; ;
    break;}
case 6:
#line 139 "parseholiday.y"
{ yyval.ival = 0; ;
    break;}
case 7:
#line 140 "parseholiday.y"
{ yyval.ival = yyvsp[0].ival; ;
    break;}
case 8:
#line 143 "parseholiday.y"
{ seteaster(yyvsp[-1].ival, yyvsp[0].ival); ;
    break;}
case 9:
#line 144 "parseholiday.y"
{ setdate( m,  d,  y, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival);;
    break;}
case 10:
#line 145 "parseholiday.y"
{ setwday( 0, yyvsp[-2].ival,  0, yyvsp[-1].ival, yyvsp[0].ival);;
    break;}
case 11:
#line 146 "parseholiday.y"
{ setwday(yyvsp[-3].ival, yyvsp[-2].ival,  0, yyvsp[-1].ival, yyvsp[0].ival);;
    break;}
case 12:
#line 147 "parseholiday.y"
{ setwday(yyvsp[-5].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival);;
    break;}
case 13:
#line 148 "parseholiday.y"
{ setdoff(yyvsp[-4].ival, yyvsp[-3].ival,m,d,y,yyvsp[-1].ival,yyvsp[0].ival);;
    break;}
case 14:
#line 151 "parseholiday.y"
{ yyval.ival =	0; ;
    break;}
case 15:
#line 152 "parseholiday.y"
{ yyval.ival =	yyvsp[0].ival; ;
    break;}
case 16:
#line 153 "parseholiday.y"
{ yyval.ival = -yyvsp[0].ival; ;
    break;}
case 17:
#line 156 "parseholiday.y"
{ yyval.ival = 0; ;
    break;}
case 18:
#line 157 "parseholiday.y"
{ yyval.ival = (yyvsp[-2].ival<<8) | yyvsp[0].ival;printf("Shift to %i if %i\n", yyvsp[-2].ival, yyvsp[0].ival); ;
    break;}
case 19:
#line 160 "parseholiday.y"
{ yyval.ival = 0; ;
    break;}
case 20:
#line 161 "parseholiday.y"
{ yyval.ival = (1<<yyvsp[0].ival); ;
    break;}
case 21:
#line 162 "parseholiday.y"
{ yyval.ival = (1<<yyvsp[-2].ival) | yyvsp[0].ival; ;
    break;}
case 22:
#line 165 "parseholiday.y"
{ yyval.ival =	1; ;
    break;}
case 23:
#line 166 "parseholiday.y"
{ yyval.ival =	yyvsp[0].ival; ;
    break;}
case 24:
#line 169 "parseholiday.y"
{ m = yyvsp[0].ival; d = yyvsp[-2].ival; y = 0;  ;
    break;}
case 25:
#line 170 "parseholiday.y"
{ m = yyvsp[-1].ival; d = yyvsp[-3].ival; y = 0;  ;
    break;}
case 26:
#line 171 "parseholiday.y"
{ m = yyvsp[-2].ival; d = yyvsp[-4].ival; y = yyvsp[0].ival; ;
    break;}
case 27:
#line 172 "parseholiday.y"
{ m = yyvsp[-2].ival; d = yyvsp[0].ival; y = 0;  ;
    break;}
case 28:
#line 173 "parseholiday.y"
{ m = yyvsp[-4].ival; d = yyvsp[-2].ival; y = yyvsp[0].ival; ;
    break;}
case 29:
#line 174 "parseholiday.y"
{ m = yyvsp[-1].ival; d = yyvsp[0].ival; y = 0;  ;
    break;}
case 30:
#line 175 "parseholiday.y"
{ m = yyvsp[-2].ival; d = yyvsp[-1].ival; y = yyvsp[0].ival; ;
    break;}
case 31:
#line 176 "parseholiday.y"
{ m = yyvsp[0].ival; d = yyvsp[-1].ival; y = 0;  ;
    break;}
case 32:
#line 177 "parseholiday.y"
{ m = yyvsp[-1].ival; d = yyvsp[-2].ival; y = yyvsp[0].ival; ;
    break;}
case 33:
#line 178 "parseholiday.y"
{ m = yyvsp[-1].ival; d = yyvsp[-3].ival; y = yyvsp[0].ival; ;
    break;}
case 34:
#line 179 "parseholiday.y"
{ monthday_from_day(yyvsp[0].ival,
								 &m, &d, &y); ;
    break;}
case 35:
#line 183 "parseholiday.y"
{ yyval.ival = day_from_name(yyvsp[0].sval); ;
    break;}
case 36:
#line 184 "parseholiday.y"
{ yyval.ival = day_from_easter(); ;
    break;}
case 37:
#line 185 "parseholiday.y"
{ yyval.ival = day_from_monthday
								 (yyvsp[0].ival, yyvsp[-2].ival); ;
    break;}
case 38:
#line 187 "parseholiday.y"
{ yyval.ival = day_from_monthday
								 (yyvsp[-1].ival, yyvsp[-3].ival); ;
    break;}
case 39:
#line 189 "parseholiday.y"
{ yyval.ival = day_from_monthday
								 (yyvsp[-2].ival, yyvsp[0].ival); ;
    break;}
case 40:
#line 191 "parseholiday.y"
{ yyval.ival = day_from_monthday
								 (yyvsp[0].ival, yyvsp[-1].ival); ;
    break;}
case 41:
#line 193 "parseholiday.y"
{ yyval.ival = day_from_monthday
								 (yyvsp[-1].ival, yyvsp[0].ival); ;
    break;}
case 42:
#line 195 "parseholiday.y"
{ yyval.ival = day_from_wday(yyvsp[0].ival, yyvsp[-2].ival,
							 yyvsp[-1].ival == -1 ? -1 : 0); ;
    break;}
case 43:
#line 197 "parseholiday.y"
{ int day=day_from_monthday(yyvsp[0].ival,1);
						   yyval.ival = yyvsp[-3].ival == 999
						    ? day_from_wday(day+1,yyvsp[-2].ival,-1)
						    : day_from_wday(day,yyvsp[-2].ival,yyvsp[-3].ival-1);;
    break;}
case 46:
#line 205 "parseholiday.y"
{ yyval.ival = yyvsp[0].ival; ;
    break;}
case 47:
#line 206 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival || yyvsp[0].ival; ;
    break;}
case 48:
#line 207 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival && yyvsp[0].ival; ;
    break;}
case 49:
#line 208 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival == yyvsp[0].ival; ;
    break;}
case 50:
#line 209 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival != yyvsp[0].ival; ;
    break;}
case 51:
#line 210 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival <= yyvsp[0].ival; ;
    break;}
case 52:
#line 211 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival >= yyvsp[0].ival; ;
    break;}
case 53:
#line 212 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival <  yyvsp[0].ival; ;
    break;}
case 54:
#line 213 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival >  yyvsp[0].ival; ;
    break;}
case 55:
#line 214 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival +  yyvsp[0].ival; ;
    break;}
case 56:
#line 215 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival -  yyvsp[0].ival; ;
    break;}
case 57:
#line 216 "parseholiday.y"
{ yyval.ival = yyvsp[-2].ival *  yyvsp[0].ival; ;
    break;}
case 58:
#line 217 "parseholiday.y"
{ yyval.ival = yyvsp[0].ival ?  yyvsp[-2].ival / yyvsp[0].ival : 0; ;
    break;}
case 59:
#line 218 "parseholiday.y"
{ yyval.ival = yyvsp[0].ival ?  yyvsp[-2].ival % yyvsp[0].ival : 0; ;
    break;}
case 60:
#line 219 "parseholiday.y"
{ yyval.ival = yyvsp[-4].ival ?  yyvsp[-2].ival : yyvsp[0].ival; ;
    break;}
case 61:
#line 220 "parseholiday.y"
{ yyval.ival = !yyvsp[0].ival; ;
    break;}
case 62:
#line 221 "parseholiday.y"
{ yyval.ival = yyvsp[-1].ival; ;
    break;}
case 63:
#line 224 "parseholiday.y"
{ yyval.ival = yyvsp[-1].ival; ;
    break;}
case 64:
#line 225 "parseholiday.y"
{ yyval.ival = yyvsp[0].ival; ;
    break;}
case 66:
#line 229 "parseholiday.y"
{ yyval.ival = -yyvsp[0].ival; ;
    break;}
case 67:
#line 230 "parseholiday.y"
{ yyval.ival = parse_year; ;
    break;}
case 68:
#line 231 "parseholiday.y"
{ yyval.ival = !((yyvsp[0].ival) & 3); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 554 "/usr/share/bison.simple"

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
#ifndef YYSTACK_USE_ALLOCA
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
#endif
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
#ifndef YYSTACK_USE_ALLOCA
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
#endif    
  return 1;
}
#line 233 "parseholiday.y"

	 
/*** Private Yacc callbacks and helper functions ***/
static void kcalerror(const char *msg)
{
  fprintf(stderr, "%s: %s in line %d of %s\n", progname,
	  msg, kcallineno+1, filename);
  if (!*errormsg)
    snprintf(errormsg,sizeof(errormsg),
	    "Problem with holiday file %s:\n%.80s in line %d",
	    filename, msg, kcallineno+1);
}

static time_t date_to_time(int day, int month, int year, 
			   int *wkday, int *julian, int *weeknum)
{
  struct tm               tm;
  time_t                  ttime;
  
  tm.tm_sec   = 0;
  tm.tm_min   = 0;
  tm.tm_hour  = 0;
  tm.tm_mday  = day;
  tm.tm_mon   = month;
  tm.tm_year  = year;
  ttime = tm_to_time(&tm);
  if (wkday)
    *wkday   = tm.tm_wday;
  if (julian)
    *julian  = tm.tm_yday;
  if (weeknum)
    *weeknum = 0
      ? tm.tm_yday / 7
      : tm.tm_yday ? ((tm.tm_yday - 1) /7) + 1 : 0;
  return(ttime == -1 || day != tm.tm_mday ? 0 : ttime);
} 

static time_t tm_to_time(struct tm *tm)
{
  time_t                  t;              /* return value */
  
  t  = monthbegin[tm->tm_mon]                     /* full months */
    + tm->tm_mday-1                              /* full days */
    + (!(tm->tm_year & 3) && tm->tm_mon > 1);    /* leap day this year*/
  tm->tm_yday = t;
  t += 365 * (tm->tm_year - 70)                   /* full years */
    + (tm->tm_year - 69)/4;                      /* past leap days */
  tm->tm_wday = (t + 4) % 7;
  
  t = t*86400 + tm->tm_hour*3600 + tm->tm_min*60 + tm->tm_sec;
  if (tm->tm_mday > monthlen[tm->tm_mon] +
      (!(tm->tm_year & 3) && tm->tm_mon == 1))
    return((time_t)-1);
  return(t);
} 

/*
 * set holiday by weekday (monday..sunday). The expression is
 * "every <num>-th <wday> of <month> plus <off> days". num and month
 * can be ANY or LAST.
 */

static void setwday(int num, int wday, int month, int off, int length)
{
  int		min_month = 0, max_month = 11;
  int		min_num   = 0, max_num   = 4;
  int		mn, n, dy, l, mlen, wday1;
  int		ddup = 0;
  
  if (month != ANY)
    min_month = max_month = month-1;
  if (month == LAST)
    min_month = max_month = 11;
  if (num != ANY)
    min_num = max_num = num-1;
  
  holiday_name = yacc_string;
  for (mn=min_month; mn <= max_month; mn++) {
    (void)date_to_time(1, mn, parse_year, &wday1, 0, 0);
    dy = (wday-1 - (wday1-1) +7) % 7 + 1;
    mlen = monthlen[mn] + (mn==1 && ISLEAPYEAR(parse_year));
    if (num == LAST)
      for (l=0; l < length; l++)
	setliteraldate(mn, dy+28<=mlen ? dy+28 : dy+21,
		       off+l, &ddup);
    else
      for (dy+=min_num*7, n=min_num; n <= max_num; n++, dy+=7)
	if (dy >= 1 && dy <= mlen)
	  for (l=0; l < length; l++)
	    setliteraldate(mn,dy,off+l,&ddup);
  }
}

/*
 * set holiday by weekday (monday..sunday) date offset. The expression is
 * "every <wday> before/after <date> plus <off> days". 
 * (This routine contributed by Peter Littlefield <plittle@sofkin.ca>)
 */

static void setdoff(int wday, int rel, int month, int day, 
		    int year, int off, int length)
{
  int		min_month = 0, max_month = 11;
  int		min_day   = 1, max_day   = 31;
  int		mn, dy, nd, l, wday1;
  int		ddup = 0;
  
  if (year != ANY) {
    year %= 100;
    if (year < 70) year += 100;
    if (year != parse_year)
      return;
  }
  if (month != ANY)
    min_month = max_month = month-1;
  if (month == LAST)
    min_month = max_month = 11;
  if (day != ANY)
    min_day   = max_day   = day;
  
  holiday_name = yacc_string;
  for (mn=min_month; mn <= max_month; mn++)
    if (day == LAST) {
      (void)date_to_time(monthlen[mn], mn, parse_year,
			 &wday1, 0, 0);
      nd = (((wday - wday1 + 7) % 7) -
	    ((rel == BEFORE) ? 7 : 0)) % 7;
      for (l=0; l < length; l++)
	setliteraldate(mn,monthlen[mn]+nd, off+l, &ddup);
    } else
      for (dy=min_day; dy <= max_day; dy++) {
	(void)date_to_time(dy, mn, parse_year,
			   &wday1, 0, 0);
	nd = (((wday - wday1 + 7) % 7) -
	      ((rel == BEFORE) ? 7 : 0)) % 7;
	for (l=0; l < length; l++)
	  setliteraldate(mn, dy+nd, off+l, &ddup);
      }
}

static int conditionalOffset( int day, int month, int year, int cond ) 
{
  int off = 0;
  int wday = 0;
  (void)date_to_time( day, month, year, &wday, 0, 0);
  if ( wday == 0 ) { wday = 7; } /* sunday is 7, not 0 */
  if ( cond & (1<<wday) ) { 
    /* condition matches -> higher 8 bits contain the possible days to shift to */
    int to = (cond >> 8);
    while ( !(to & (1<<((wday+off)%7))) && (off < 8) ) {
      ++off;
    }
  }
  if ( off >= 8 ) return 0;
  else return off;
}

/*
 * set holiday by date. Ignore holidays in the wrong year. The code is
 * complicated by expressions such as "any/last/any" (every last day of
 * the month).
 */

static void setdate(int month, int day, int year, int off, int conditionaloff, int length)
{
  int		min_month = 0, max_month = 11;
  int		min_day   = 1, max_day   = 31;
  int		mn, dy, l;
  int		ddup = 0;
  
  if (year != ANY) {
    year %= 100;
    if (year < 70) year += 100;
    if (year != parse_year)
      return;
  }
  if (month != ANY)
    min_month = max_month = month-1;
  if (month == LAST)
    min_month = max_month = 11;
  if (day != ANY)
    min_day   = max_day   = day;
  
  holiday_name = yacc_string;
  /** TODO: Include the conditionaloff variable. */
  /** The encoding of the conditional offset is:
        8 lower bits: conditions to shift (bit-register, bit 1=mon, ..., bit 7=sun)
        8 higher bits: weekday to shift to (bit-register, bit 1=mon, ..., bit 7=sun)
  */
  for (mn=min_month; mn <= max_month; mn++) {
    if (day == LAST) {
      int newoff = off + conditionalOffset( monthlen[mn], mn, parse_year, conditionaloff );
      for (l=0; l < length; l++)
	setliteraldate(mn, monthlen[mn], newoff+l, &ddup);
    } else {
      for (dy=min_day; dy <= max_day; dy++) {
        int newoff = off + conditionalOffset( dy, mn, parse_year, conditionaloff );
	for (l=0; l < length; l++)
	  setliteraldate(mn, dy, newoff+l, &ddup);
      }
    }
  }	  
}


/*
 * After the two routines above have removed ambiguities (ANY) and resolved
 * weekday specifications, this routine registers the holiday in the holiday
 * array. There are two of these, for full-line holidays (they take away one
 * appointment line in the month calendar daybox) and "small" holidays, which
 * appear next to the day number. If the day is already some other holiday,
 * add a new item to the singly-linked list and insert the holiday there.
 * <ddup> is information stored for parse_holidays(), it
 * will free() the holiday name only if its dup field is 0 (because many
 * string fields can point to the same string, which was allocated only once
 * by the lexer, and should therefore only be freed once).
 */

static void setliteraldate(int month, int day, int off, int *ddup)
{
  int julian = JULIAN(month, day) + off;
  /*  struct holiday *hp = yacc_small ? &sm_holiday[julian]
      : &holiday[julian]; */
  struct holiday *hp = 0;

  if (julian >= 0 && julian <= 365 ) {
    hp = &holidays[julian];
    if ( hp->string ) {
      while (hp->next) { hp = hp->next; }
      hp->next = malloc( sizeof(struct holiday)*2 );
      hp = hp->next;
      hp->next = 0;
    }
    if (!*ddup)
      holiday_name = strdup(holiday_name);
    hp->string	= holiday_name;
    hp->color   = (yacc_stringcolor == 0) ? yacc_daycolor : yacc_stringcolor;
    hp->dup		= (*ddup)++;
    
  }
}


/*
 * set a holiday relative to Easter
 */

static void seteaster(int off, int length)
{
  int		ddup = 0;	/* flag for later free() */
  int julian = easter_julian + off;
  /*  struct holiday *hp = yacc_small ? &sm_holiday[julian]
      : &holidays[julian];*/
  struct holiday *hp = 0;
  
  holiday_name = yacc_string;
  while (length-- > 0) {
    if (julian >= 0 && julian <= 365 ) {
      hp = &holidays[julian];
      if ( hp->string ) {
        while (hp->next) { hp = hp->next; }
        hp->next = malloc( sizeof(struct holiday)*2 );
        hp = hp->next;
        hp->next = 0;
      }
      if (!ddup)
	holiday_name = strdup(holiday_name);
      hp->string	= holiday_name;
      hp->color     = (yacc_stringcolor == 0) ? yacc_daycolor : yacc_stringcolor;
      hp->dup		= ddup++;
    }
    julian++;
  }
}


/*
 * calculate Easter Sunday as a julian date. I got this from Armin Liebl
 * <liebla@informatik.tu-muenchen.de>, who got it from Knuth. I hope I got
 * all this right...
 */

static int calc_easter(int year)
{
  int golden, cent, grcor, clcor, extra, epact, easter;
  
  golden = (year/19)*(-19);
  golden += year+1;
  cent = year/100+1;
  grcor = (cent*3)/(-4)+12;
  clcor = ((cent-18)/(-25)+cent-16)/3;
  extra = (year*5)/4+grcor-10;
  epact = golden*11+20+clcor+grcor;
  epact += (epact/30)*(-30);
  if (epact<=0)
    epact += 30;
  if (epact==25) {
    if (golden>11)
      epact += 1;
  } else {
    if (epact==24)
      epact += 1;
  }
  easter = epact*(-1)+44;
  if (easter<21)
    easter += 30;
  extra += easter;
  extra += (extra/7)*(-7);
  extra *= -1;
  easter += extra+7;
  easter += 31+28+!(year&3)-1;
  return(easter);
}


/*
 * functions used for [] syntax: (Erwin Hugo Achermann <acherman@inf.ethz.ch>)
 *
 * day_from_name (str)			gets day from symbolic name
 * day_from_easter ()			gets day as easter sunday
 * day_from_monthday (month, day)		gets <day> from <month/day>
 * day_from_wday (day, wday, num)	gets num-th day (wday) after <day> day
 * monthday_from_day (day, *m, *d, *y)	gets month/day/cur_year from <day>
 */

static int day_from_name(char *str)
{
  int	i;
  char	*name;
  
  for (i=0; i < 366; i++) {
    name = holidays[i].string;
    if (name && !strcmp(str, name))
      return(i);
  }
  return(-1);
}


static int day_from_easter(void)
{
  return(easter_julian);
}


static int day_from_monthday(int month, int day)
{
  if (month == 13)
    return(365 + ISLEAPYEAR(parse_year));
  return(JULIAN(month - 1, day));
}


static void monthday_from_day(int day, int *mn, int *dy, int *yr)
{
  int	i, len;
  
  *yr = parse_year;
  *mn = 0;
  *dy = 0;
  if (day < 0)
    return;
  for (i=0; i < 12; i++) {
    len = monthlen[i] + (i == 1 && ISLEAPYEAR(parse_year));
    if (day < len) {
      *mn = i + 1;
      *dy = day + 1;
      break;
    }
    day -= len;
  }
}


static int day_from_wday(int day, int wday, int num)
{
  int	wkday, yday, weeknum;
  
  (void)date_to_time(1, 0, parse_year, &wkday, &yday, &weeknum);
  day += (wday - wkday - day + 1001) % 7;
  day += num * 7;
  return (day);
}

static void initialize() 
{
  register struct holiday *hp;
  register int dy;
  initialized = 1;
  for (hp=holidays, dy=0; dy < 366; dy++, hp++)
  {
      hp->color = 0;
      hp->dup = 0;
      hp->string = 0;
      hp->next = 0;
  }
}

/*** Public Functions ***/
/*
 * parse the holiday text file, and set up the holiday arrays for a year.
 * If year is -1, re-parse the last year parsed (this is used when the
 * holiday file changes). If there is a CPP_PATH, check if the executable
 * really exists, and if so, pipe the holioday files through it.
 * Return an error message if an error occurred, 0 otherwise.
 */

char *parse_holidays(const char *holidayfile, int year, short force)
{
  register struct holiday *hp;
  register int		dy;
  short			piped = 0;
  if (!initialized)
    initialize();

  if (year == parse_year && !force)
      return(0);
  if (year < 0)
      year = parse_year;
  parse_year = year;
  easter_julian = calc_easter(year + 1900);
  
  for (hp=holidays, dy=0; dy < 366; dy++, hp++)
  {
      hp->color = 0;
      if (hp->string) {
        if (!hp->dup )
              free(hp->string);
          hp->string = 0;
      }
      {
      struct holiday *nx = hp->next;
      hp->next = 0;
      while (nx) {
        struct holiday *nxtmp;
        if ( nx->string && !nx->dup ) {
          free( nx->string );
        }
        nxtmp=nx;
        nx = nxtmp->next;
        free( nxtmp );
      }
      }
  }
  /*  for (hp=sm_holiday, d=0; d < 366; d++, hp++)
      if (hp->string) {
      if (!hp->dup)
      free(hp->string);
      hp->string      = 0;
      }*/

  filename = holidayfile;
  if (access(filename, R_OK)) return(0);
  kcalin = fopen(filename, "r");
  if (!kcalin) return(0);
  *errormsg = 0;
  kcallineno = 0;
  kcalparse();
  if (piped) pclose(kcalin);
  else fclose(kcalin);
  if (*errormsg) return(errormsg);

  return(0);
}

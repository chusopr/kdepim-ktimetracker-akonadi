
/*  A Bison parser, made from icalyacc.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse ical_yyparse
#define yylex ical_yylex
#define yyerror ical_yyerror
#define yylval ical_yylval
#define yychar ical_yychar
#define yydebug ical_yydebug
#define yynerrs ical_yynerrs
#define	DIGITS	257
#define	INTNUMBER	258
#define	FLOATNUMBER	259
#define	STRING	260
#define	EOL	261
#define	EQUALS	262
#define	CHARACTER	263
#define	COLON	264
#define	COMMA	265
#define	SEMICOLON	266
#define	MINUS	267
#define	TIMESEPERATOR	268
#define	TRUE	269
#define	FALSE	270
#define	FREQ	271
#define	BYDAY	272
#define	BYHOUR	273
#define	BYMINUTE	274
#define	BYMONTH	275
#define	BYMONTHDAY	276
#define	BYSECOND	277
#define	BYSETPOS	278
#define	BYWEEKNO	279
#define	BYYEARDAY	280
#define	DAILY	281
#define	MINUTELY	282
#define	MONTHLY	283
#define	SECONDLY	284
#define	WEEKLY	285
#define	HOURLY	286
#define	YEARLY	287
#define	INTERVAL	288
#define	COUNT	289
#define	UNTIL	290
#define	WKST	291
#define	MO	292
#define	SA	293
#define	SU	294
#define	TU	295
#define	WE	296
#define	TH	297
#define	FR	298
#define	BIT8	299
#define	ACCEPTED	300
#define	ADD	301
#define	AUDIO	302
#define	BASE64	303
#define	BINARY	304
#define	BOOLEAN	305
#define	BUSY	306
#define	BUSYTENTATIVE	307
#define	BUSYUNAVAILABLE	308
#define	CALADDRESS	309
#define	CANCEL	310
#define	CANCELLED	311
#define	CHAIR	312
#define	CHILD	313
#define	COMPLETED	314
#define	CONFIDENTIAL	315
#define	CONFIRMED	316
#define	COUNTER	317
#define	DATE	318
#define	DATETIME	319
#define	DECLINECOUNTER	320
#define	DECLINED	321
#define	DELEGATED	322
#define	DISPLAY	323
#define	DRAFT	324
#define	DURATION	325
#define	EMAIL	326
#define	END	327
#define	FINAL	328
#define	FLOAT	329
#define	FREE	330
#define	GREGORIAN	331
#define	GROUP	332
#define	INDIVIDUAL	333
#define	INPROCESS	334
#define	INTEGER	335
#define	NEEDSACTION	336
#define	NONPARTICIPANT	337
#define	OPAQUE	338
#define	OPTPARTICIPANT	339
#define	PARENT	340
#define	PERIOD	341
#define	PRIVATE	342
#define	PROCEDURE	343
#define	PUBLIC	344
#define	PUBLISH	345
#define	RECUR	346
#define	REFRESH	347
#define	REPLY	348
#define	REQPARTICIPANT	349
#define	REQUEST	350
#define	RESOURCE	351
#define	ROOM	352
#define	SIBLING	353
#define	START	354
#define	TENTATIVE	355
#define	TEXT	356
#define	THISANDFUTURE	357
#define	THISANDPRIOR	358
#define	TIME	359
#define	TRANSPAENT	360
#define	UNKNOWN	361
#define	UTCOFFSET	362
#define	XNAME	363
#define	ALTREP	364
#define	CN	365
#define	CUTYPE	366
#define	DAYLIGHT	367
#define	DIR	368
#define	ENCODING	369
#define	EVENT	370
#define	FBTYPE	371
#define	FMTTYPE	372
#define	LANGUAGE	373
#define	MEMBER	374
#define	PARTSTAT	375
#define	RANGE	376
#define	RELATED	377
#define	RELTYPE	378
#define	ROLE	379
#define	RSVP	380
#define	SENTBY	381
#define	STANDARD	382
#define	URI	383
#define	TIME_CHAR	384
#define	UTC_CHAR	385

#line 1 "icalyacc.y"

/* -*- Mode: C -*-
  ======================================================================
  FILE: icalitip.y
  CREATOR: eric 10 June 1999
  
  DESCRIPTION:
  
  $Id$
  $Locker:  $

  (C) COPYRIGHT 1999 Eric Busboom 
  http://www.softwarestudio.org

  The contents of this file are subject to the Mozilla Public License
  Version 1.0 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
  the License for the specific language governing rights and
  limitations under the License.

  The original author is Eric Busboom
  The original code is icalitip.y



  =======================================================================*/

#include <stdlib.h>
#include <string.h> /* for strdup() */
#include "icalparser.h"
#include "pvl.h"

icalvalue *icalparser_yy_value; /* Current Value */

void ical_yyerror(char* s);
void icalparser_clear_flex_input();  
int ical_yy_lex(void);

/* Globals for UTCOFFSET values */
int utc; 
int utc_b; 
int utcsign;

/* Globals for DURATION values */
struct icaldurationtype duration;

/* Globals for RECUR values */
struct icalrecurrencetype recur;
short skiplist[367];
short skippos;
int dow_pos;

void copy_list(short* array, size_t size);
void clear_recur();
void add_prop(icalproperty_kind);
void icalparser_fill_date(struct tm* t, char* dstr);
void icalparser_fill_time(struct tm* t, char* tstr);
void set_value_type(icalvalue_kind kind);
void set_parser_value_state();
struct icaltimetype fill_datetime(char* d, char* t);
void ical_yy_error(char *s); /* Don't know why I need this.... */
int yylex(void); /* Or this. */



/* Set the state of the lexer so it will interpret values ( iCAL
   VALUEs, that is, ) correctly. */


#line 75 "icalyacc.y"
typedef union {
	float v_float;
	int   v_int;
	char* v_string;
} YYSTYPE;
#ifndef YYDEBUG
#define YYDEBUG 1
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		143
#define	YYFLAG		-32768
#define	YYNTBASE	142

#define YYTRANSLATE(x) ((unsigned)(x) <= 386 ? yytranslate[x] : 168)

static const short yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,   138,     2,   139,     2,   141,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,   137,     2,     2,
     2,   134,     2,     2,     2,     2,   135,     2,     2,   140,
     2,     2,   136,     2,     2,     2,   133,     2,     2,     2,
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
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
    77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
    87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
    97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
   107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
   117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
   127,   128,   129,   130,   131,   132
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,     8,    10,    12,    14,    16,    18,
    20,    22,    24,    26,    27,    29,    30,    32,    37,    39,
    42,    45,    48,    51,    54,    57,    61,    64,    68,    71,
    74,    75,    77,    79,    83,    87,    91,   101,   108,   112,
   116,   120,   124,   128,   132,   136,   138,   140,   142,   144,
   146,   148,   150,   152,   155,   159,   163,   168,   174,   176,
   180,   184,   188,   192,   196,   200,   204,   208,   212,   216,
   220,   224,   228,   232,   236,   240,   244,   248,   252,   256,
   260,   261,   265,   268,   270,   272,   276
};

static const short yyrhs[] = {   143,
     0,   144,     0,   145,     0,   148,     0,   157,     0,   158,
     0,   165,     0,   167,     0,     1,     0,   132,     0,    15,
     0,    16,     0,     3,     0,     0,   131,     0,     0,   131,
     0,     3,   130,     3,   146,     0,   155,     0,   155,   151,
     0,     3,   133,     0,   130,   152,     0,   130,   153,     0,
   130,   154,     0,     3,   134,     0,     3,   134,   153,     0,
     3,   135,     0,     3,   135,   154,     0,     3,   136,     0,
     3,   137,     0,     0,   138,     0,   139,     0,   156,   140,
   149,     0,   156,   140,   151,     0,   156,   140,   150,     0,
     3,   130,     3,   146,   141,     3,   130,     3,   147,     0,
     3,   130,     3,   146,   141,   157,     0,    17,     8,    30,
     0,    17,     8,    28,     0,    17,     8,    32,     0,    17,
     8,    27,     0,    17,     8,    31,     0,    17,     8,    29,
     0,    17,     8,    33,     0,    40,     0,    38,     0,    41,
     0,    42,     0,    43,     0,    44,     0,    39,     0,   160,
     0,     3,   160,     0,    13,     3,   160,     0,   161,    11,
   160,     0,   161,    11,     3,   160,     0,   161,    11,    13,
     3,   160,     0,     3,     0,   162,    11,     3,     0,    34,
     8,     3,     0,    37,     8,    40,     0,    37,     8,    38,
     0,    37,     8,    41,     0,    37,     8,    42,     0,    37,
     8,    43,     0,    37,     8,    44,     0,    37,     8,    39,
     0,    23,     8,   162,     0,    20,     8,   162,     0,    19,
     8,   162,     0,    18,     8,   161,     0,    21,     8,   162,
     0,    22,     8,   162,     0,    26,     8,   162,     0,    25,
     8,   162,     0,    24,     8,   162,     0,    36,     8,   148,
     0,    36,     8,   145,     0,    35,     8,     3,     0,     0,
   164,    12,   163,     0,   159,   164,     0,   138,     0,   139,
     0,   166,     4,     4,     0,   166,     4,     4,     4,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   181,   183,   184,   185,   186,   187,   188,   189,   190,   196,
   198,   201,   204,   219,   221,   224,   226,   228,   244,   245,
   247,   252,   255,   258,   262,   266,   271,   275,   280,   285,
   290,   294,   298,   303,   308,   313,   322,   343,   375,   377,
   378,   379,   380,   381,   382,   386,   388,   389,   390,   391,
   392,   393,   398,   400,   402,   404,   405,   407,   411,   413,
   416,   418,   419,   420,   421,   422,   423,   424,   425,   426,
   427,   428,   429,   430,   431,   432,   433,   434,   437,   440,
   444,   446,   448,   456,   457,   459,   465
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","DIGITS",
"INTNUMBER","FLOATNUMBER","STRING","EOL","EQUALS","CHARACTER","COLON","COMMA",
"SEMICOLON","MINUS","TIMESEPERATOR","TRUE","FALSE","FREQ","BYDAY","BYHOUR","BYMINUTE",
"BYMONTH","BYMONTHDAY","BYSECOND","BYSETPOS","BYWEEKNO","BYYEARDAY","DAILY",
"MINUTELY","MONTHLY","SECONDLY","WEEKLY","HOURLY","YEARLY","INTERVAL","COUNT",
"UNTIL","WKST","MO","SA","SU","TU","WE","TH","FR","BIT8","ACCEPTED","ADD","AUDIO",
"BASE64","BINARY","BOOLEAN","BUSY","BUSYTENTATIVE","BUSYUNAVAILABLE","CALADDRESS",
"CANCEL","CANCELLED","CHAIR","CHILD","COMPLETED","CONFIDENTIAL","CONFIRMED",
"COUNTER","DATE","DATETIME","DECLINECOUNTER","DECLINED","DELEGATED","DISPLAY",
"DRAFT","DURATION","EMAIL","END","FINAL","FLOAT","FREE","GREGORIAN","GROUP",
"INDIVIDUAL","INPROCESS","INTEGER","NEEDSACTION","NONPARTICIPANT","OPAQUE","OPTPARTICIPANT",
"PARENT","PERIOD","PRIVATE","PROCEDURE","PUBLIC","PUBLISH","RECUR","REFRESH",
"REPLY","REQPARTICIPANT","REQUEST","RESOURCE","ROOM","SIBLING","START","TENTATIVE",
"TEXT","THISANDFUTURE","THISANDPRIOR","TIME","TRANSPAENT","UNKNOWN","UTCOFFSET",
"XNAME","ALTREP","CN","CUTYPE","DAYLIGHT","DIR","ENCODING","EVENT","FBTYPE",
"FMTTYPE","LANGUAGE","MEMBER","PARTSTAT","RANGE","RELATED","RELTYPE","ROLE",
"RSVP","SENTBY","STANDARD","URI","TIME_CHAR","UTC_CHAR","\"unimplemented2\"",
"'W'","'H'","'M'","'S'","'D'","'+'","'-'","'P'","'/'","value","binary_value",
"boolean_value","date_value","utc_char","utc_char_b","datetime_value","dur_date",
"dur_week","dur_time","dur_hour","dur_minute","dur_second","dur_day","dur_prefix",
"duration_value","period_value","recur_start","weekday","weekday_list","recur_list",
"recur_skip","recur_skip_list","recur_value","plusminus","utcoffset_value", NULL
};
#endif

static const short yyr1[] = {     0,
   142,   142,   142,   142,   142,   142,   142,   142,   142,   143,
   144,   144,   145,   146,   146,   147,   147,   148,   149,   149,
   150,   151,   151,   151,   152,   152,   153,   153,   154,   155,
   156,   156,   156,   157,   157,   157,   158,   158,   159,   159,
   159,   159,   159,   159,   159,   160,   160,   160,   160,   160,
   160,   160,   161,   161,   161,   161,   161,   161,   162,   162,
   163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
   163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
   164,   164,   165,   166,   166,   167,   167
};

static const short yyr2[] = {     0,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     0,     1,     0,     1,     4,     1,     2,
     2,     2,     2,     2,     2,     3,     2,     3,     2,     2,
     0,     1,     1,     3,     3,     3,     9,     6,     3,     3,
     3,     3,     3,     3,     3,     1,     1,     1,     1,     1,
     1,     1,     1,     2,     3,     3,     4,     5,     1,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     0,     3,     2,     1,     1,     3,     4
};

static const short yydefact[] = {     0,
     9,    13,    11,    12,     0,    10,    32,    33,     1,     2,
     3,     4,     0,     5,     6,    81,     7,     0,     8,     0,
     0,     0,    83,     0,    14,    42,    40,    44,    39,    43,
    41,    45,     0,     0,    34,    36,    35,    19,     0,    86,
    15,    18,    21,    30,     0,    22,    23,    24,    20,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    82,    87,    31,    25,    27,    29,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    32,    33,    38,     0,    26,     0,    28,     0,
     0,    47,    52,    46,    48,    49,    50,    51,    53,    72,
    59,    71,    70,    73,    74,    69,    77,    76,    75,    61,
    80,    13,    79,    78,    63,    68,    62,    64,    65,    66,
    67,     0,    54,     0,     0,     0,     0,    16,    55,     0,
     0,    56,    60,    14,    17,    37,    57,     0,    18,    58,
     0,     0,     0
};

static const short yydefgoto[] = {   141,
     9,    10,    11,    42,   136,    12,    35,    36,    37,    46,
    47,    48,    38,    13,    14,    15,    16,    99,   100,   102,
    63,    23,    17,    18,    19
};

static const short yypact[] = {    -1,
-32768,  -123,-32768,-32768,     5,-32768,    31,    32,-32768,-32768,
-32768,-32768,  -134,-32768,-32768,-32768,-32768,    35,-32768,    15,
    -7,    -2,    28,    37,   -89,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,  -129,    83,-32768,-32768,-32768,   -43,    41,    84,
-32768,   -52,-32768,-32768,  -124,-32768,-32768,-32768,-32768,    82,
    85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
    95,    96,-32768,-32768,     2,   102,   103,-32768,     6,   104,
   104,   104,   104,   104,   104,   104,   104,   105,   106,   107,
   -10,   -39,-32768,-32768,-32768,   -24,-32768,   -44,-32768,    30,
   109,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   108,
-32768,   110,   110,   110,   110,   110,   110,   110,   110,-32768,
-32768,   -17,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   111,-32768,    30,    14,   112,   113,   -14,-32768,    30,
   115,-32768,-32768,   -89,-32768,-32768,-32768,    30,-32768,-32768,
   120,   122,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,    43,    -9,-32768,    44,-32768,-32768,    97,-32768,
    60,    62,-32768,-32768,    65,-32768,-32768,   -87,-32768,     8,
-32768,-32768,-32768,-32768,-32768
};


#define	YYLAST		141


static const short yytable[] = {     1,
    33,     2,   123,    43,    82,    22,    20,    44,    90,    66,
    67,    68,    21,     3,     4,     5,   130,    25,    91,    26,
    27,    28,    29,    30,    31,    32,   131,   115,   116,   117,
   118,   119,   120,   121,   -84,   -85,   129,   132,    24,    39,
    40,    41,   137,    92,    93,    94,    95,    96,    97,    98,
   140,    92,    93,    94,    95,    96,    97,    98,    50,    51,
    52,    53,    54,    55,    56,    57,    58,    92,    93,    94,
    95,    96,    97,    98,    59,    60,    61,    62,   103,   104,
   105,   106,   107,   108,   109,    45,    34,    64,    65,    69,
   122,    68,    70,    71,    72,    73,    74,    75,    76,    77,
    78,    79,    80,    81,    86,    88,   101,   110,   111,   112,
    67,   124,   127,   128,   133,   134,   135,   138,   125,   142,
   126,   143,   113,   114,   139,    87,     0,    34,    89,    85,
     6,     0,     0,     0,    49,     0,     7,     8,   -31,    83,
    84
};

static const short yycheck[] = {     1,
     3,     3,    90,   133,     3,   140,   130,   137,     3,   134,
   135,   136,     8,    15,    16,    17,     3,     3,    13,    27,
    28,    29,    30,    31,    32,    33,    13,    38,    39,    40,
    41,    42,    43,    44,     4,     4,   124,   125,     4,    12,
     4,   131,   130,    38,    39,    40,    41,    42,    43,    44,
   138,    38,    39,    40,    41,    42,    43,    44,    18,    19,
    20,    21,    22,    23,    24,    25,    26,    38,    39,    40,
    41,    42,    43,    44,    34,    35,    36,    37,    71,    72,
    73,    74,    75,    76,    77,     3,   130,     4,   141,     8,
   130,   136,     8,     8,     8,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     3,     3,     3,     3,     3,     3,
   135,     3,   130,     3,     3,     3,   131,     3,    11,     0,
    11,     0,    80,    80,   134,    66,    -1,   130,    67,    65,
   132,    -1,    -1,    -1,    38,    -1,   138,   139,   140,   138,
   139
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
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

#line 217 "/usr/lib/bison.simple"

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

case 9:
#line 190 "icalyacc.y"
{ 
                  icalparser_yy_value = 0;
		  icalparser_clear_flex_input();
                  yyclearin;
                  ;
    break;}
case 11:
#line 200 "icalyacc.y"
{ icalparser_yy_value = icalvalue_new_boolean(1); ;
    break;}
case 12:
#line 202 "icalyacc.y"
{ icalparser_yy_value = icalvalue_new_boolean(0); ;
    break;}
case 13:
#line 205 "icalyacc.y"
{
	    struct icaltimetype stm;

	    stm = fill_datetime(yyvsp[0].v_string,0);

	    stm.hour = -1;
	    stm.minute = -1;
	    stm.second = -1;
	    stm.is_utc = 0;
	    stm.is_date = 1;

	    icalparser_yy_value = icalvalue_new_date(stm);
	;
    break;}
case 14:
#line 220 "icalyacc.y"
{utc = 0;;
    break;}
case 15:
#line 221 "icalyacc.y"
{utc = 1;;
    break;}
case 16:
#line 225 "icalyacc.y"
{utc_b = 0;;
    break;}
case 17:
#line 226 "icalyacc.y"
{utc_b = 1;;
    break;}
case 18:
#line 230 "icalyacc.y"
{
	    struct  icaltimetype stm;
	    stm = fill_datetime(yyvsp[-3].v_string, yyvsp[-1].v_string);
	    stm.is_utc = utc;
	    stm.is_date = 0;

	    icalparser_yy_value = 
		icalvalue_new_datetime(stm);
	;
    break;}
case 21:
#line 248 "icalyacc.y"
{
	    duration.weeks = atoi(yyvsp[-1].v_string);
	;
    break;}
case 22:
#line 253 "icalyacc.y"
{
	;
    break;}
case 23:
#line 256 "icalyacc.y"
{
	;
    break;}
case 24:
#line 259 "icalyacc.y"
{
	;
    break;}
case 25:
#line 263 "icalyacc.y"
{
	    duration.hours = atoi(yyvsp[-1].v_string);
	;
    break;}
case 26:
#line 267 "icalyacc.y"
{
	    duration.hours = atoi(yyvsp[-2].v_string);
	;
    break;}
case 27:
#line 272 "icalyacc.y"
{
	    duration.minutes = atoi(yyvsp[-1].v_string);
	;
    break;}
case 28:
#line 276 "icalyacc.y"
{
	    duration.minutes = atoi(yyvsp[-2].v_string);
	;
    break;}
case 29:
#line 281 "icalyacc.y"
{
	    duration.seconds = atoi(yyvsp[-1].v_string);
	;
    break;}
case 30:
#line 286 "icalyacc.y"
{
	    duration.days = atoi(yyvsp[-1].v_string);
	;
    break;}
case 31:
#line 291 "icalyacc.y"
{
	    duration.is_neg = 0;
	;
    break;}
case 32:
#line 295 "icalyacc.y"
{
	    duration.is_neg = 0;
	;
    break;}
case 33:
#line 299 "icalyacc.y"
{ 
	    duration.is_neg = 1;
	;
    break;}
case 34:
#line 304 "icalyacc.y"
{ 
	    icalparser_yy_value = icalvalue_new_duration(duration); 
	    memset(&duration,0, sizeof(duration));
	;
    break;}
case 35:
#line 309 "icalyacc.y"
{ 
	    icalparser_yy_value = icalvalue_new_duration(duration); 
	    memset(&duration,0, sizeof(duration));
	;
    break;}
case 36:
#line 314 "icalyacc.y"
{ 
	    icalparser_yy_value = icalvalue_new_duration(duration); 
	    memset(&duration,0, sizeof(duration));
	;
    break;}
case 37:
#line 323 "icalyacc.y"
{
            struct icalperiodtype p;
        
	    p.start = fill_datetime(yyvsp[-8].v_string,yyvsp[-6].v_string);
	    p.start.is_utc = utc;
	    p.start.is_date = 0;


	    p.end = fill_datetime(yyvsp[-3].v_string,yyvsp[-1].v_string);
	    p.end.is_utc = utc_b;
	    p.end.is_date = 0;
		
	    p.duration.days = -1;
	    p.duration.weeks = -1;
	    p.duration.hours = -1;
	    p.duration.minutes = -1;
	    p.duration.seconds = -1;

	    icalparser_yy_value = icalvalue_new_period(p);
	;
    break;}
case 38:
#line 344 "icalyacc.y"
{
            struct icalperiodtype p;
	    
	    p.start = fill_datetime(yyvsp[-5].v_string,yyvsp[-3].v_string);
	    p.start.is_utc = utc;
	    p.start.is_date = 0;

	    p.end.year = -1;
	    p.end.month = -1;
	    p.end.day = -1;
	    p.end.hour = -1;
	    p.end.minute = -1;
	    p.end.second = -1;
		   
	    /* The duration_value rule setes the global 'duration'
               variable, but it also creates a new value in
               icalparser_yy_value. So, free that, then copy
               'duration' into the icalperiodtype struct. */

	    p.duration = icalvalue_get_duration(icalparser_yy_value);
	    icalvalue_free(icalparser_yy_value);
	    icalparser_yy_value = 0;

	    icalparser_yy_value = icalvalue_new_period(p);

	;
    break;}
case 39:
#line 376 "icalyacc.y"
{clear_recur();recur.freq = ICAL_SECONDLY_RECURRENCE;;
    break;}
case 40:
#line 377 "icalyacc.y"
{clear_recur();recur.freq = ICAL_MINUTELY_RECURRENCE;;
    break;}
case 41:
#line 378 "icalyacc.y"
{clear_recur();recur.freq = ICAL_HOURLY_RECURRENCE;;
    break;}
case 42:
#line 379 "icalyacc.y"
{clear_recur();recur.freq = ICAL_DAILY_RECURRENCE;;
    break;}
case 43:
#line 380 "icalyacc.y"
{clear_recur();recur.freq = ICAL_WEEKLY_RECURRENCE;;
    break;}
case 44:
#line 381 "icalyacc.y"
{clear_recur();recur.freq = ICAL_MONTHLY_RECURRENCE;;
    break;}
case 45:
#line 382 "icalyacc.y"
{clear_recur();recur.freq = ICAL_YEARLY_RECURRENCE;;
    break;}
case 46:
#line 387 "icalyacc.y"
{ skiplist[skippos]=ICAL_SUNDAY_WEEKDAY; ;
    break;}
case 47:
#line 388 "icalyacc.y"
{ skiplist[skippos]=ICAL_MONDAY_WEEKDAY; ;
    break;}
case 48:
#line 389 "icalyacc.y"
{ skiplist[skippos]=ICAL_TUESDAY_WEEKDAY; ;
    break;}
case 49:
#line 390 "icalyacc.y"
{ skiplist[skippos]=ICAL_WEDNESDAY_WEEKDAY; ;
    break;}
case 50:
#line 391 "icalyacc.y"
{ skiplist[skippos]=ICAL_THURSDAY_WEEKDAY; ;
    break;}
case 51:
#line 392 "icalyacc.y"
{ skiplist[skippos]=ICAL_FRIDAY_WEEKDAY; ;
    break;}
case 52:
#line 393 "icalyacc.y"
{ skiplist[skippos]=ICAL_SATURDAY_WEEKDAY; ;
    break;}
case 53:
#line 399 "icalyacc.y"
{if( skippos<8) skippos++;;
    break;}
case 54:
#line 400 "icalyacc.y"
{ dow_pos = atoi(yyvsp[-1].v_string);  
	           skiplist[skippos] += 8*dow_pos; if( skippos<8) skippos++; ;
    break;}
case 55:
#line 402 "icalyacc.y"
{ dow_pos = atoi(yyvsp[-1].v_string);  
	           skiplist[skippos] -= 8*dow_pos; if( skippos<8) skippos++; ;
    break;}
case 56:
#line 404 "icalyacc.y"
{if( skippos<8) skippos++;;
    break;}
case 57:
#line 405 "icalyacc.y"
{ dow_pos = atoi(yyvsp[-1].v_string); 
	           skiplist[skippos] += 8*dow_pos;if( skippos<8) skippos++;;
    break;}
case 58:
#line 407 "icalyacc.y"
{ dow_pos = atoi(yyvsp[-1].v_string); 
	           skiplist[skippos] -= 8*dow_pos;if( skippos<8) skippos++;;
    break;}
case 59:
#line 412 "icalyacc.y"
{ skiplist[skippos] = atoi(yyvsp[0].v_string); skippos++;;
    break;}
case 60:
#line 413 "icalyacc.y"
{ skiplist[skippos] = atoi(yyvsp[0].v_string); if (skippos<367) skippos++;;
    break;}
case 61:
#line 417 "icalyacc.y"
{recur.interval = atoi(yyvsp[0].v_string);;
    break;}
case 62:
#line 418 "icalyacc.y"
{recur.week_start = ICAL_SUNDAY_WEEKDAY;;
    break;}
case 63:
#line 419 "icalyacc.y"
{recur.week_start = ICAL_MONDAY_WEEKDAY;;
    break;}
case 64:
#line 420 "icalyacc.y"
{recur.week_start = ICAL_TUESDAY_WEEKDAY;;
    break;}
case 65:
#line 421 "icalyacc.y"
{recur.week_start = ICAL_WEDNESDAY_WEEKDAY;;
    break;}
case 66:
#line 422 "icalyacc.y"
{recur.week_start = ICAL_THURSDAY_WEEKDAY;;
    break;}
case 67:
#line 423 "icalyacc.y"
{recur.week_start = ICAL_FRIDAY_WEEKDAY;;
    break;}
case 68:
#line 424 "icalyacc.y"
{recur.week_start = ICAL_SATURDAY_WEEKDAY;;
    break;}
case 69:
#line 425 "icalyacc.y"
{copy_list(recur.by_second,60);;
    break;}
case 70:
#line 426 "icalyacc.y"
{copy_list(recur.by_minute,60);;
    break;}
case 71:
#line 427 "icalyacc.y"
{copy_list(recur.by_hour,24);;
    break;}
case 72:
#line 428 "icalyacc.y"
{copy_list(recur.by_day,7);;
    break;}
case 73:
#line 429 "icalyacc.y"
{copy_list(recur.by_month,12);;
    break;}
case 74:
#line 430 "icalyacc.y"
{copy_list(recur.by_month_day,31);;
    break;}
case 75:
#line 431 "icalyacc.y"
{copy_list(recur.by_year_day,366);;
    break;}
case 76:
#line 432 "icalyacc.y"
{copy_list(recur.by_week_no,53);;
    break;}
case 77:
#line 433 "icalyacc.y"
{copy_list(recur.by_set_pos,366);;
    break;}
case 78:
#line 435 "icalyacc.y"
{ recur.until = icalvalue_get_datetime(icalparser_yy_value);
	   icalvalue_free(icalparser_yy_value); icalparser_yy_value=0;;
    break;}
case 79:
#line 438 "icalyacc.y"
{ recur.until = icalvalue_get_date(icalparser_yy_value);
	   icalvalue_free(icalparser_yy_value); icalparser_yy_value=0;;
    break;}
case 80:
#line 441 "icalyacc.y"
{ recur.count = atoi(yyvsp[0].v_string); ;
    break;}
case 83:
#line 450 "icalyacc.y"
{ icalparser_yy_value = icalvalue_new_recur(recur); ;
    break;}
case 84:
#line 456 "icalyacc.y"
{ utcsign = 1; ;
    break;}
case 85:
#line 457 "icalyacc.y"
{ utcsign = -1; ;
    break;}
case 86:
#line 461 "icalyacc.y"
{
	    icalparser_yy_value = icalvalue_new_utcoffset( utcsign * (yyvsp[-1].v_int*3600) + (yyvsp[0].v_int*60) );
  	;
    break;}
case 87:
#line 466 "icalyacc.y"
{
	    icalparser_yy_value = icalvalue_new_utcoffset(utcsign * (yyvsp[-2].v_int*3600) + (yyvsp[-1].v_int*60) +(yyvsp[0].v_int));
  	;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

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
#line 472 "icalyacc.y"



void clear_recur()
{   
    memset(&skiplist, ICAL_RECURRENCE_ARRAY_MAX_BYTE, sizeof(skiplist));
    skippos = 0;

    icalrecurrencetype_clear(&recur);
}

void copy_list(short* array, size_t size)
{ 
	memcpy(array, skiplist, size*sizeof(short));
	memset(&skiplist,ICAL_RECURRENCE_ARRAY_MAX_BYTE, sizeof(skiplist));
	skippos = 0;
}

struct icaltimetype fill_datetime(char* datestr, char* timestr)
{
	    struct icaltimetype stm;

	    memset(&stm,0,sizeof(stm));

	    if (datestr != 0){
		sscanf(datestr,"%4d%2d%2d",&(stm.year), &(stm.month), 
		       &(stm.day));
	    }

	    if (timestr != 0){
		sscanf(timestr,"%2d%2d%2d", &(stm.hour), &(stm.minute), 
		       &(stm.second));
	    }

	    return stm;

}

void ical_yyerror(char* s)
{
    /*fprintf(stderr,"Parse error \'%s\'\n", s);*/
}


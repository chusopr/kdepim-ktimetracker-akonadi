#ifndef lint
char yysccsid[] = "@(#)yaccpar	1.4 (Berkeley) 02/25/90";
#endif
#line 2 "vcc.y"

/***************************************************************************
(C) Copyright 1996 Apple Computer, Inc., AT&T Corp., International             
Business Machines Corporation and Siemens Rolm Communications Inc.             
                                                                               
For purposes of this license notice, the term Licensors shall mean,            
collectively, Apple Computer, Inc., AT&T Corp., International                  
Business Machines Corporation and Siemens Rolm Communications Inc.             
The term Licensor shall mean any of the Licensors.                             
                                                                               
Subject to acceptance of the following conditions, permission is hereby        
granted by Licensors without the need for written agreement and without        
license or royalty fees, to use, copy, modify and distribute this              
software for any purpose.                                                      
                                                                               
The above copyright notice and the following four paragraphs must be           
reproduced in all copies of this software and any software including           
this software.                                                                 
                                                                               
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS AND NO LICENSOR SHALL HAVE       
ANY OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS OR       
MODIFICATIONS.                                                                 
                                                                               
IN NO EVENT SHALL ANY LICENSOR BE LIABLE TO ANY PARTY FOR DIRECT,              
INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOST PROFITS ARISING OUT         
OF THE USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH         
DAMAGE.                                                                        
                                                                               
EACH LICENSOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED,       
INCLUDING BUT NOT LIMITED TO ANY WARRANTY OF NONINFRINGEMENT OR THE            
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR             
PURPOSE.                                                                       

The software is provided with RESTRICTED RIGHTS.  Use, duplication, or         
disclosure by the government are subject to restrictions set forth in          
DFARS 252.227-7013 or 48 CFR 52.227-19, as applicable.                         

***************************************************************************/

/*
 * src: vcc.c
 * doc: Parser for vCard and vCalendar. Note that this code is
 * generated by a yacc parser generator. Generally it should not
 * be edited by hand. The real source is vcc.y. The #line directives
 * have been commented out here to make it easier to trace through
 * in a debugger. However, if a bug is found it should 
 */


/* debugging utilities */
#if __DEBUG
#define DBG_(x) printf x
#else
#define DBG_(x)
#endif

/****  External Functions  ****/

/* assign local name to parser variables and functions so that
   we can use more than one yacc based parser.
*/

#define yyparse mime_parse
#define yylex mime_lex
#define yyerror mime_error
#define yychar mime_char
/* #define p_yyval p_mime_val */
#undef yyval
#define yyval mime_yyval
/* #define p_yylval p_mime_lval */
#undef yylval
#define yylval mime_yylval
#define yydebug mime_debug
#define yynerrs mime_nerrs
#define yyerrflag mime_errflag
#define yyss mime_ss
#define yyssp mime_ssp
#define yyvs mime_vs
#define yyvsp mime_vsp
#define yylhs mime_lhs
#define yylen mime_len
#define yydefred mime_defred
#define yydgoto mime_dgoto
#define yysindex mime_sindex
#define yyrindex mime_rindex
#define yygindex mime_gindex
#define yytable mime_table
#define yycheck mime_check
#define yyname mime_name
#define yyrule mime_rule
#define YYPREFIX "mime_"


#ifndef _NO_LINE_FOLDING
#define _SUPPORT_LINE_FOLDING 1
#endif

/* undef below if compile with MFC */
/* #define INCLUDEMFC 1 */

#if defined(WIN32) || defined(_WIN32)
#ifdef INCLUDEMFC
#include <afx.h>
#endif
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "vcc.h"

/****  Types, Constants  ****/

#define YYDEBUG		0	/* 1 to compile in some debugging code */
#define MAXTOKEN	256	/* maximum token (line) length */
#define YYSTACKSIZE 	50	/* ~unref ?*/
#define MAXLEVEL	10	/* max # of nested objects parseable */
				/* (includes outermost) */


/****  Global Variables  ****/
int mime_lineNum, mime_numErrors; /* yyerror() can use these */
static VObject* vObjList;
static VObject *curProp;
static VObject *curObj;
static VObject* ObjStack[MAXLEVEL];
static int ObjStackTop;


/* A helpful utility for the rest of the app. */
#if __CPLUSPLUS__
extern "C" {
#endif

    extern void Parse_Debug(const char *s);
    extern void yyerror(char *s);

#if __CPLUSPLUS__
    };
#endif

int yyparse();

enum LexMode {
	L_NORMAL,
	L_VCARD,
	L_VCAL,
	L_VEVENT,
	L_VTODO,
	L_VALUES,
	L_BASE64,
	L_QUOTED_PRINTABLE
	};

/****  Private Forward Declarations  ****/
static int pushVObject(const char *prop);
static VObject* popVObject();
static char* lexDataFromBase64();
static void lexPopMode(int top);
static int lexWithinMode(enum LexMode mode);
static void lexPushMode(enum LexMode mode);
static void enterProps(const char *s);
static void enterAttr(const char *s1, const char *s2);
static void enterValues(const char *value);
static void mime_error_(char *s);

#line 178 "vcc.y"
typedef union {
    char *str;
    VObject *vobj;
    } YYSTYPE;
#line 181 "y_tab.c"
#define EQ 257
#define COLON 258
#define DOT 259
#define SEMICOLON 260
#define SPACE 261
#define HTAB 262
#define LINESEP 263
#define NEWLINE 264
#define BEGIN_VCARD 265
#define END_VCARD 266
#define BEGIN_VCAL 267
#define END_VCAL 268
#define BEGIN_VEVENT 269
#define END_VEVENT 270
#define BEGIN_VTODO 271
#define END_VTODO 272
#define ID 273
#define STRING 274
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    7,    6,    6,    5,    5,    9,    3,   10,    3,
    8,    8,   14,   11,   16,   12,   12,   15,   15,   17,
   18,   18,    1,   19,   13,   13,    2,    2,   21,    4,
   22,    4,   20,   20,   23,   23,   23,   26,   24,   27,
   24,   28,   25,   29,   25,
};
short yylen[] = {                                         2,
    1,    0,    3,    1,    1,    1,    0,    4,    0,    3,
    2,    1,    0,    5,    0,    3,    1,    2,    1,    2,
    1,    3,    1,    0,    4,    1,    1,    0,    0,    4,
    0,    3,    2,    1,    1,    1,    1,    0,    4,    0,
    3,    0,    4,    0,    3,
};
short yydefred[] = {                                      0,
    0,    0,    0,    5,    6,    0,    1,    0,    0,    0,
    0,    0,   23,    0,    0,    0,    0,   10,    0,    0,
   37,    0,    0,   35,   36,   32,    3,    0,    8,   11,
   13,    0,    0,    0,    0,   30,   33,    0,   16,    0,
    0,    0,   41,    0,   45,    0,   20,   18,   27,    0,
    0,   39,   43,    0,   24,   14,   22,    0,   25,
};
short yydgoto[] = {                                       3,
   14,   50,    4,    5,    6,    7,   12,   21,    8,    9,
   16,   17,   51,   41,   39,   28,   40,   47,   58,   22,
   10,   11,   23,   24,   25,   32,   33,   34,   35,
};
short yysindex[] = {                                   -245,
    0,    0,    0,    0,    0,    0,    0, -244, -230, -246,
 -234, -245,    0,    0, -229, -244, -220,    0,    0,    0,
    0, -227, -246,    0,    0,    0,    0, -221,    0,    0,
    0, -244, -226, -244, -224,    0,    0, -244,    0, -221,
 -231, -223,    0, -222,    0, -217,    0,    0,    0, -218,
 -214,    0,    0, -244,    0,    0,    0, -231,    0,
};
short yyrindex[] = {                                      0,
 -263, -252,    0,    0,    0,    1,    0,    0,    0,    0,
    0,    0,    0, -228,    0, -257,    0,    0, -266, -267,
    0,    0, -216,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -213,
 -232,    0,    0,    0,    0, -225,    0,    0,    0, -212,
    0,    0,    0,    0,    0,    0,    0, -232,    0,
};
short yygindex[] = {                                      0,
  -36,    0,    0,    0,    0,   34,    0,   -8,    0,    0,
    0,    0,   -5,    0,   14,    0,    0,    0,    0,   32,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
#define YYTABLESIZE 268
short yytable[] = {                                      15,
    4,   46,    9,   40,   44,   42,   38,   30,   12,    7,
   12,   12,   12,   12,   12,   31,   29,   57,   29,    1,
   29,    2,   19,   42,   20,   44,   13,   28,   13,   17,
   28,   15,   21,   26,   21,   18,   29,   31,   38,   54,
   36,   55,   49,   43,   19,   27,   52,   45,   56,   53,
   26,   34,   59,   48,   37,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    2,    0,    2,
};
short yycheck[] = {                                       8,
    0,   38,  266,  270,  272,  273,  273,   16,  266,  273,
  268,  269,  270,  271,  272,  268,  269,   54,  271,  265,
  273,  267,  269,   32,  271,   34,  273,  260,  273,  258,
  263,  260,  258,  268,  260,  266,  266,  258,  260,  257,
  268,  260,  274,  270,  258,   12,  270,  272,  263,  272,
  263,  268,   58,   40,   23,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  265,   -1,  267,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 274
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"EQ","COLON","DOT","SEMICOLON",
"SPACE","HTAB","LINESEP","NEWLINE","BEGIN_VCARD","END_VCARD","BEGIN_VCAL",
"END_VCAL","BEGIN_VEVENT","END_VEVENT","BEGIN_VTODO","END_VTODO","ID","STRING",
};
char *yyrule[] = {
"$accept : mime",
"mime : vobjects",
"$$1 :",
"vobjects : vobject $$1 vobjects",
"vobjects : vobject",
"vobject : vcard",
"vobject : vcal",
"$$2 :",
"vcard : BEGIN_VCARD $$2 items END_VCARD",
"$$3 :",
"vcard : BEGIN_VCARD $$3 END_VCARD",
"items : item items",
"items : item",
"$$4 :",
"item : prop COLON $$4 values LINESEP",
"$$5 :",
"prop : name $$5 attr_params",
"prop : name",
"attr_params : attr_param attr_params",
"attr_params : attr_param",
"attr_param : SEMICOLON attr",
"attr : name",
"attr : name EQ name",
"name : ID",
"$$6 :",
"values : value SEMICOLON $$6 values",
"values : value",
"value : STRING",
"value :",
"$$7 :",
"vcal : BEGIN_VCAL $$7 calitems END_VCAL",
"$$8 :",
"vcal : BEGIN_VCAL $$8 END_VCAL",
"calitems : calitem calitems",
"calitems : calitem",
"calitem : eventitem",
"calitem : todoitem",
"calitem : items",
"$$9 :",
"eventitem : BEGIN_VEVENT $$9 items END_VEVENT",
"$$10 :",
"eventitem : BEGIN_VEVENT $$10 END_VEVENT",
"$$11 :",
"todoitem : BEGIN_VTODO $$11 items END_VTODO",
"$$12 :",
"todoitem : BEGIN_VTODO $$12 END_VTODO",
};
#endif
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#ifndef YYSTACKSIZE
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 300
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
#define yystacksize YYSTACKSIZE
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#line 372 "vcc.y"
/*/////////////////////////////////////////////////////////////////////////*/
static int pushVObject(const char *prop)
    {
    VObject *newObj;
    if (ObjStackTop == MAXLEVEL)
	return FALSE;

    ObjStack[++ObjStackTop] = curObj;

    if (curObj) {
        newObj = addProp(curObj,prop);
        curObj = newObj;
	}
    else
	curObj = newVObject(prop);

    return TRUE;
    }


/*/////////////////////////////////////////////////////////////////////////*/
/* This pops the recently built vCard off the stack and returns it. */
static VObject* popVObject()
    {
    VObject *oldObj;
    if (ObjStackTop < 0) {
	yyerror("pop on empty Object Stack\n");
	return 0;
	}
    oldObj = curObj;
    curObj = ObjStack[ObjStackTop--];

    return oldObj;
    }


static void enterValues(const char *value)
    {
    if (fieldedProp && *fieldedProp) {
	if (value) {
	    addPropValue(curProp,*fieldedProp,value);
	    }
	/* else this field is empty, advance to next field */
	fieldedProp++;
	}
    else {
	if (value) {
	    setVObjectUStringZValue_(curProp,fakeUnicode(value,0));
	    }
	}
    deleteStr(value);
    }

static void enterProps(const char *s)
    {
    curProp = addGroup(curObj,s);
    deleteStr(s);
    }

static void enterAttr(const char *s1, const char *s2)
    {
    const char *p1, *p2;
    p1 = lookupProp_(s1);
    if (s2) {
	VObject *a;
	p2 = lookupProp_(s2);
	a = addProp(curProp,p1);
	setVObjectStringZValue(a,p2);
	}
    else
	addProp(curProp,p1);
    if (strcmp(p1,VCBase64Prop) == 0 || (s2 && strcmp(p2,VCBase64Prop)==0))
	lexPushMode(L_BASE64);
    else if (strcmp(p1,VCQuotedPrintableProp) == 0
	    || (s2 && strcmp(p2,VCQuotedPrintableProp)==0))
	lexPushMode(L_QUOTED_PRINTABLE);
    deleteStr(s1); deleteStr(s2);
    }


#define MAX_LEX_LOOKAHEAD_0 32
#define MAX_LEX_LOOKAHEAD 64
#define MAX_LEX_MODE_STACK_SIZE 10
#define LEXMODE() (lexBuf.lexModeStack[lexBuf.lexModeStackTop])

struct LexBuf {
	/* input */
#ifdef INCLUDEMFC
    CFile *inputFile;
#else
    FILE *inputFile;
#endif
    char *inputString;
    unsigned long curPos;
    unsigned long inputLen;
	/* lookahead buffer */
	/*   -- lookahead buffer is short instead of char so that EOF
	 /      can be represented correctly.
	*/
    unsigned long len;
    short buf[MAX_LEX_LOOKAHEAD];
    unsigned long getPtr;
	/* context stack */
    unsigned long lexModeStackTop;
    enum LexMode lexModeStack[MAX_LEX_MODE_STACK_SIZE];
	/* token buffer */
    unsigned long maxToken;
    char *strs;
    unsigned long strsLen;
    } lexBuf;

static void lexPushMode(enum LexMode mode)
    {
    if (lexBuf.lexModeStackTop == (MAX_LEX_MODE_STACK_SIZE-1))
	yyerror("lexical context stack overflow");
    else {
	lexBuf.lexModeStack[++lexBuf.lexModeStackTop] = mode;
	}
    }

static void lexPopMode(int top)
    {
    /* special case of pop for ease of error recovery -- this
	version will never underflow */
    if (top)
	lexBuf.lexModeStackTop = 0;
    else
	if (lexBuf.lexModeStackTop > 0) lexBuf.lexModeStackTop--;
    }

static int lexWithinMode(enum LexMode mode) {
    unsigned long i;
    for (i=0;i<lexBuf.lexModeStackTop;i++)
	if (mode == lexBuf.lexModeStack[i]) return 1;
    return 0;
    }

static char lexGetc_()
    {
    /* get next char from input, no buffering. */
    if (lexBuf.curPos == lexBuf.inputLen)
	return EOF;
    else if (lexBuf.inputString)
	return *(lexBuf.inputString + lexBuf.curPos++);
    else {
#ifdef INCLUDEMFC
	char result;
	return lexBuf.inputFile->Read(&result, 1) == 1 ? result : EOF;
#else
	return fgetc(lexBuf.inputFile);
#endif
	}
    }

static int lexGeta()
    {
    ++lexBuf.len;
    return (lexBuf.buf[lexBuf.getPtr] = lexGetc_());
    }

static int lexGeta_(int i)
    {
    ++lexBuf.len;
    return (lexBuf.buf[(lexBuf.getPtr+i)%MAX_LEX_LOOKAHEAD] = lexGetc_());
    }

static void lexSkipLookahead() {
    if (lexBuf.len > 0 && lexBuf.buf[lexBuf.getPtr]!=EOF) {
	/* don't skip EOF. */
        lexBuf.getPtr = (lexBuf.getPtr + 1) % MAX_LEX_LOOKAHEAD;
	lexBuf.len--;
        }
    }

static int lexLookahead() {
    int c = (lexBuf.len)?
	lexBuf.buf[lexBuf.getPtr]:
	lexGeta();
    /* do the \r\n -> \n or \r -> \n translation here */
    if (c == '\r') {
	int a = (lexBuf.len>1)?
	    lexBuf.buf[(lexBuf.getPtr+1)%MAX_LEX_LOOKAHEAD]:
	    lexGeta_(1);
	if (a == '\n') {
	    lexSkipLookahead();
	    }
	lexBuf.buf[lexBuf.getPtr] = c = '\n';
	}
    else if (c == '\n') {
	int a = (lexBuf.len>1)?
	    lexBuf.buf[lexBuf.getPtr+1]:
	    lexGeta_(1);
	if (a == '\r') {
	    lexSkipLookahead();
	    }
	lexBuf.buf[lexBuf.getPtr] = '\n';
	}
    return c;
    }

static int lexGetc() {
    int c = lexLookahead();
    if (lexBuf.len > 0 && lexBuf.buf[lexBuf.getPtr]!=EOF) {
	/* EOF will remain in lookahead buffer */
        lexBuf.getPtr = (lexBuf.getPtr + 1) % MAX_LEX_LOOKAHEAD;
	lexBuf.len--;
        }
    return c;
    }

static void lexSkipLookaheadWord() {
    if (lexBuf.strsLen <= lexBuf.len) {
	lexBuf.len -= lexBuf.strsLen;
	lexBuf.getPtr = (lexBuf.getPtr + lexBuf.strsLen) % MAX_LEX_LOOKAHEAD;
	}
    }

static void lexClearToken()
    {
    lexBuf.strsLen = 0;
    }

static void lexAppendc(int c)
    {
    lexBuf.strs[lexBuf.strsLen] = c;
    /* append up to zero termination */
    if (c == 0) return;
    lexBuf.strsLen++;
    if (lexBuf.strsLen > lexBuf.maxToken) {
	/* double the token string size */
	lexBuf.maxToken <<= 1;
	lexBuf.strs = (char*) realloc(lexBuf.strs,(size_t)lexBuf.maxToken);
	}
    }

static char* lexStr() {
    return dupStr(lexBuf.strs,(size_t)lexBuf.strsLen+1);
    }

static void lexSkipWhite() {
    int c = lexLookahead();
    while (c == ' ' || c == '\t') {
	lexSkipLookahead();
	c = lexLookahead();
	}
    }

static char* lexGetWord() {
    int c;
    lexSkipWhite();
    lexClearToken();
    c = lexLookahead();
    while (c != EOF && !strchr("\t\n ;:=",c)) {
	lexAppendc(c);
	lexSkipLookahead();
	c = lexLookahead();
	}
    lexAppendc(0);
    return lexStr();
    }

static void lexPushLookahead(char *s, int len) {
    int putptr;
    if (len == 0) len = strlen(s);
    putptr = (int)lexBuf.getPtr - len;
    /* this function assumes that length of word to push back
     /  is not greater than MAX_LEX_LOOKAHEAD.
     */
    if (putptr < 0) putptr += MAX_LEX_LOOKAHEAD;
    lexBuf.getPtr = putptr;
    while (*s) {
	lexBuf.buf[putptr] = *s++;
	putptr = (putptr + 1) % MAX_LEX_LOOKAHEAD;
	}
    lexBuf.len += len;
    }

static void lexPushLookaheadc(int c) {
    int putptr;
    /* can't putback EOF, because it never leaves lookahead buffer */
    if (c == EOF) return;
    putptr = (int)lexBuf.getPtr - 1;
    if (putptr < 0) putptr += MAX_LEX_LOOKAHEAD;
    lexBuf.getPtr = putptr;
    lexBuf.buf[putptr] = c;
    lexBuf.len += 1;
    }

static char* lexLookaheadWord() {
    /* this function can lookahead word with max size of MAX_LEX_LOOKAHEAD_0
     /  and thing bigger than that will stop the lookahead and return 0;
     / leading white spaces are not recoverable.
     */
    int c;
    int len = 0;
    int curgetptr = 0;
    lexSkipWhite();
    lexClearToken();
    curgetptr = (int)lexBuf.getPtr;	// remember!
    while (len < (MAX_LEX_LOOKAHEAD_0)) {
	c = lexGetc();
	len++;
	if (c == EOF || strchr("\t\n ;:=", c)) {
	    lexAppendc(0);
	    /* restore lookahead buf. */
	    lexBuf.len += len;
	    lexBuf.getPtr = curgetptr;
	    return lexStr();
	    }
        else
	    lexAppendc(c);
	}
    lexBuf.len += len;	/* char that has been moved to lookahead buffer */
    lexBuf.getPtr = curgetptr;
    return 0;
    }

#ifdef _SUPPORT_LINE_FOLDING
static char* lexGet1Value() {
    int size = 0;
    int c = lexLookahead();
    lexClearToken();
    while (c != EOF && c != ';') {
	if (c == '\n') {
	    int a;
	    lexSkipLookahead();
	    a  = lexLookahead();
	    if (a == ' ' || a == '\t') {
		lexAppendc(' ');
		lexSkipLookahead();
		}
	    else {
		lexPushLookaheadc('\n');
		break;
		}
	    }
	else {
	    lexAppendc(c);
	    lexSkipLookahead();
	    }
	c = lexLookahead();
	}
    lexAppendc(0);
    return c==EOF?0:lexStr();
    }
#endif

static char* lexGetStrUntil(char *termset) {
    int size = 0;
    int c = lexLookahead();
    lexClearToken();
    while (c != EOF && !strchr(termset,c)) {
	lexAppendc(c);
	lexSkipLookahead();
	c = lexLookahead();
	}
    lexAppendc(0);
    return c==EOF?0:lexStr();
    }

static int match_begin_name(int end) {
    char *n = lexLookaheadWord();
    int token = ID;
    if (n) {
	if (!stricmp(n,"vcard")) token = end?END_VCARD:BEGIN_VCARD;
	else if (!stricmp(n,"vcalendar")) token = end?END_VCAL:BEGIN_VCAL;
	else if (!stricmp(n,"vevent")) token = end?END_VEVENT:BEGIN_VEVENT;
	else if (!stricmp(n,"vtodo")) token = end?END_VTODO:BEGIN_VTODO;
	deleteStr(n);
	return token;
	}
    return 0;
    }


#ifdef INCLUDEMFC
void initLex(const char *inputstring, unsigned long inputlen, CFile *inputfile)
#else
void initLex(const char *inputstring, unsigned long inputlen, FILE *inputfile)
#endif
    {
    // initialize lex mode stack
    lexBuf.lexModeStack[lexBuf.lexModeStackTop=0] = L_NORMAL;

    // iniatialize lex buffer.
    lexBuf.inputString = (char*) inputstring;
    lexBuf.inputLen = inputlen;
    lexBuf.curPos = 0;
    lexBuf.inputFile = inputfile;

    lexBuf.len = 0;
    lexBuf.getPtr = 0;

    lexBuf.maxToken = MAXTOKEN;
    lexBuf.strs = (char*)malloc(MAXTOKEN);
    lexBuf.strsLen = 0;

    }

static void finiLex() {
    free(lexBuf.strs);
    }


/*/////////////////////////////////////////////////////////////////////////*/
/* This parses and converts the base64 format for binary encoding into
 * a decoded buffer (allocated with new).  See RFC 1521.
 */
static char * lexGetDataFromBase64()
    {
    unsigned long bytesLen = 0, bytesMax = 0;
    int quadIx = 0, pad = 0;
    unsigned long trip = 0;
    unsigned char b;
    int c;
    unsigned char *bytes = NULL;

    DBG_(("db: lexGetDataFromBase64\n"));
    while (1) {
	c = lexGetc();
	if (c == '\n') {
	    ++mime_lineNum;
	    if (lexLookahead() == '\n') {
		/* a '\n' character by itself means end of data */
		break;
		}
	    else continue; /* ignore '\n' */
	    }
	else {
	    if ((c >= 'A') && (c <= 'Z'))
		b = (unsigned char)(c - 'A');
	    else if ((c >= 'a') && (c <= 'z'))
		b = (unsigned char)(c - 'a') + 26;
	    else if ((c >= '0') && (c <= '9'))
		b = (unsigned char)(c - '0') + 52;
	    else if (c == '+')
		b = 62;
	    else if (c == '/')
		b = 63;
	    else if (c == '=') {
		b = 0;
		pad++;
	    } else if ((c == ' ') || (c == '\t')) {
		continue;
	    } else { /* error condition */
		if (bytes) free(bytes);
		// error recovery: skip until 2 adjacent newlines.
		DBG_(("db: invalid character 0x%x '%c'\n", c,c));
		if (c != EOF)  {
		    c = lexGetc();
		    while (c != EOF) {
			if (c == '\n' && lexLookahead() == '\n') {
			    ++mime_lineNum;
			    break;
			    }
			c = lexGetc();
			}
		    }
		return NULL;
		}
	    trip = (trip << 6) | b;
	    if (++quadIx == 4) {
		unsigned char outBytes[3];
		int numOut;
		int i;
		for (i = 0; i < 3; i++) {
		    outBytes[2-i] = (unsigned char)(trip & 0xFF);
		    trip >>= 8;
		    }
		numOut = 3 - pad;
		if (bytesLen + numOut > bytesMax) {
		    if (!bytes) {
			bytesMax = 1024;
			bytes = (unsigned char*)malloc((size_t)bytesMax);
			}
		    else {
			bytesMax <<= 2;
			bytes = (unsigned char*)realloc(bytes,(size_t)bytesMax);
			}
		    }
		memcpy(bytes + bytesLen, outBytes, numOut);
		bytesLen += numOut;
		trip = 0;
		quadIx = 0;
		}
	    }
	} /* while */
    DBG_(("db: bytesLen = %d\n",  bytesLen));
    /* kludge: all this won't be necessary if we have tree form
	representation */
    setValueWithSize(curProp,bytes,(unsigned int)bytesLen);
    free(bytes);
    return 0;
    }

static int match_begin_end_name(int end) {
    int token;
    lexSkipWhite();
    if (lexLookahead() != ':') return ID;
    lexSkipLookahead();
    lexSkipWhite();
    token = match_begin_name(end);
    if (token == ID) {
	lexPushLookaheadc(':');
	DBG_(("db: ID '%s'\n", yylval.str));
	return ID;
	}
    else if (token != 0) {
	lexSkipLookaheadWord();
	deleteStr(yylval.str);
	DBG_(("db: begin/end %d\n", token));
	return token;
	}
    return 0;
    }

static char* lexGetQuotedPrintable()
    {
    char cur;
    unsigned long len = 0;

    lexClearToken();
    do {
	cur = lexGetc();
	switch (cur) {
	    case '=': {
		int c = 0;
		int next[2];
		int i;
		for (i = 0; i < 2; i++) {
		    next[i] = lexGetc();
		    if (next[i] >= '0' && next[i] <= '9')
			c = c * 16 + next[i] - '0';
		    else if (next[i] >= 'A' && next[i] <= 'F')
			c = c * 16 + next[i] - 'A' + 10;
		    else
			break;
		    }
		if (i == 0) {
		    /* single '=' follow by LINESEP is continuation sign? */
		    if (next[0] == '\n') {
			++mime_lineNum;
			}
		    else {
			lexPushLookaheadc('=');
			goto EndString;
			}
		    }
		else if (i == 1) {
		    lexPushLookaheadc(next[1]);
		    lexPushLookaheadc(next[0]);
		    lexAppendc('=');
		} else {
		    lexAppendc(c);
		    }
		break;
		} /* '=' */
	    case '\n': {
		lexPushLookaheadc('\n');
		goto EndString;
		}
	    case (char)EOF:
		break;
	    default:
		lexAppendc(cur);
		break;
	    } /* switch */
	} while (cur != (char)EOF);

EndString:
    lexAppendc(0);
    return lexStr();
    } /* LexQuotedPrintable */

static int yylex() {
    int token = 0;

    int lexmode = LEXMODE();
    if (lexmode == L_VALUES) {
	int c = lexGetc();
	if (c == ';') {
	    DBG_(("db: SEMICOLON\n"));
	    return SEMICOLON;
	    }
	else if (strchr("\n",c)) {
	    ++mime_lineNum;
	    /* consume all line separator(s) adjacent to each other */
	    c = lexLookahead();
	    while (strchr("\n",c)) {
		lexSkipLookahead();
		c = lexLookahead();
		++mime_lineNum;
		}
	    DBG_(("db: LINESEP\n"));
	    return LINESEP;
	    }
	else {
	    char *p = 0;
	    lexPushLookaheadc(c);
	    if (lexWithinMode(L_BASE64)) {
		/* get each char and convert to bin on the fly... */
		p = lexGetDataFromBase64();
		yylval.str = p;
		return STRING;
		}
	    else if (lexWithinMode(L_QUOTED_PRINTABLE)) {
		p = lexGetQuotedPrintable();
		}
	    else {
#ifdef _SUPPORT_LINE_FOLDING
		p = lexGet1Value();
#else
		p = lexGetStrUntil(";\n");
#endif
		}
	    if (p) {
		DBG_(("db: STRING: '%s'\n", p));
		yylval.str = p;
		return STRING;
		}
	    else return 0;
	    }
	}
    else {
	/* normal mode */
	while (1) {
	    int c = lexGetc();
	    switch(c) {
		case ':': {
		    /* consume all line separator(s) adjacent to each other */
		    /* ignoring linesep immediately after colon. */
		    c = lexLookahead();
		    while (strchr("\n",c)) {
			lexSkipLookahead();
			c = lexLookahead();
			++mime_lineNum;
			}
		    DBG_(("db: COLON\n"));
		    return COLON;
		    }
		case ';':
		    DBG_(("db: SEMICOLON\n"));
		    return SEMICOLON;
		case '=':
		    DBG_(("db: EQ\n"));
		    return EQ;
		/* ignore whitespace in this mode */
		case '\t':
		case ' ': continue;
		case '\n': {
		    ++mime_lineNum;
		    continue;
		    }
		case EOF: return 0;
		    break;
		default: {
		    lexPushLookaheadc(c);
		    if (isalpha(c)) {
			char *t = lexGetWord();
			yylval.str = t;
			if (!stricmp(t, "begin")) {
			    return match_begin_end_name(0);
			    }
			else if (!stricmp(t,"end")) {
			    return match_begin_end_name(1);
			    }
		        else {
			    DBG_(("db: ID '%s'\n", t));
			    return ID;
			    }
			}
		    else {
			/* unknow token */
			return 0;
			}
		    break;
		    }
		}
	    }
	}
    return 0;
    }


/***************************************************************************/
/***							Public Functions						****/
/***************************************************************************/

static VObject* Parse_MIMEHelper()
    {
    ObjStackTop = -1;
    mime_numErrors = 0;
    mime_lineNum = 1;
    vObjList = 0;
    curObj = 0;

    if (yyparse() != 0)
	return 0;

    finiLex();
    return vObjList;
    }

/*/////////////////////////////////////////////////////////////////////////*/
DLLEXPORT(VObject*) Parse_MIME(const char *input, unsigned long len)
    {
    initLex(input, len, 0);
    return Parse_MIMEHelper();
    }


#if INCLUDEMFC

DLLEXPORT(VObject*) Parse_MIME_FromFile(CFile *file)
    {
    unsigned long startPos;
    VObject *result;	

    initLex(0,-1,file);
    startPos = file->GetPosition();
    if (!(result = Parse_MIMEHelper()))
	file->Seek(startPos, CFile::begin);
    return result;
    }

#else

VObject* Parse_MIME_FromFile(FILE *file)
    {
    VObject *result;	
    long startPos;

    initLex(0,(unsigned long)-1,file);
    startPos = ftell(file);
    if (!(result = Parse_MIMEHelper())) {
	fseek(file,startPos,SEEK_SET);
	}
    return result;
    }

DLLEXPORT(VObject*) Parse_MIME_FromFileName(char *fname)
    {
    FILE *fp = fopen(fname,"r");
    if (fp) {
	VObject* o = Parse_MIME_FromFile(fp);
	fclose(fp);
	return o;
	}
    else {
	char msg[80];
	sprintf(msg, "can't open file '%s' for reading\n", fname);
	mime_error_(msg);
	return 0;
	}
    }

#endif

/*/////////////////////////////////////////////////////////////////////////*/
static void YYDebug(const char *s)
{
	Parse_Debug(s);
}


static MimeErrorHandler mimeErrorHandler;

DLLEXPORT(void) registerMimeErrorHandler(MimeErrorHandler me)
    {
    mimeErrorHandler = me;
    }

static void mime_error(char *s)
    {
    char msg[256];
    if (mimeErrorHandler) {
	sprintf(msg,"%s at line %d", s, mime_lineNum);
	mimeErrorHandler(msg);
	}
    }

static void mime_error_(char *s)
    {
    if (mimeErrorHandler) {
	mimeErrorHandler(s);
	}
    }

#line 1183 "y_tab.c"
#define YYABORT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, reading %d (%s)\n", yystate,
                    yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: state %d, shifting to state %d\n",
                    yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: state %d, error recovery shifting\
 to state %d\n", *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: error recovery discarding state %d\n",
                            *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, error recovery discards token %d (%s)\n",
                    yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("yydebug: state %d, reducing by rule %d (%s)\n",
                yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 2:
#line 210 "vcc.y"
{ addList(&vObjList, yyvsp[0].vobj ); curObj = 0; }
break;
case 4:
#line 213 "vcc.y"
{ addList(&vObjList, yyvsp[0].vobj ); curObj = 0; }
break;
case 7:
#line 222 "vcc.y"
{
	lexPushMode(L_VCARD);
	if (!pushVObject(VCCardProp)) YYERROR;
	}
break;
case 8:
#line 227 "vcc.y"
{
	lexPopMode(0);
	yyval.vobj  = popVObject();
	}
break;
case 9:
#line 232 "vcc.y"
{
	lexPushMode(L_VCARD);
	if (!pushVObject(VCCardProp)) YYERROR;
	}
break;
case 10:
#line 237 "vcc.y"
{
	lexPopMode(0);
	yyval.vobj  = popVObject();
	}
break;
case 13:
#line 248 "vcc.y"
{
	lexPushMode(L_VALUES);
	}
break;
case 14:
#line 252 "vcc.y"
{
	if (lexWithinMode(L_BASE64) || lexWithinMode(L_QUOTED_PRINTABLE))
	   lexPopMode(0);
	lexPopMode(0);
	}
break;
case 15:
#line 260 "vcc.y"
{
	enterProps(yyvsp[0].str );
	}
break;
case 17:
#line 265 "vcc.y"
{
	enterProps(yyvsp[0].str );
	}
break;
case 21:
#line 278 "vcc.y"
{
	enterAttr(yyvsp[0].str ,0);
	}
break;
case 22:
#line 282 "vcc.y"
{
	enterAttr(yyvsp[-2].str ,yyvsp[0].str );

	}
break;
case 24:
#line 291 "vcc.y"
{ enterValues(yyvsp[-1].str ); }
break;
case 26:
#line 293 "vcc.y"
{ enterValues(yyvsp[0].str ); }
break;
case 28:
#line 298 "vcc.y"
{ yyval.str  = 0; }
break;
case 29:
#line 303 "vcc.y"
{ if (!pushVObject(VCCalProp)) YYERROR; }
break;
case 30:
#line 306 "vcc.y"
{ yyval.vobj  = popVObject(); }
break;
case 31:
#line 308 "vcc.y"
{ if (!pushVObject(VCCalProp)) YYERROR; }
break;
case 32:
#line 310 "vcc.y"
{ yyval.vobj  = popVObject(); }
break;
case 38:
#line 325 "vcc.y"
{
	lexPushMode(L_VEVENT);
	if (!pushVObject(VCEventProp)) YYERROR;
	}
break;
case 39:
#line 331 "vcc.y"
{
	lexPopMode(0);
	popVObject();
	}
break;
case 40:
#line 336 "vcc.y"
{
	lexPushMode(L_VEVENT);
	if (!pushVObject(VCEventProp)) YYERROR;
	}
break;
case 41:
#line 341 "vcc.y"
{
	lexPopMode(0);
	popVObject();
	}
break;
case 42:
#line 349 "vcc.y"
{
	lexPushMode(L_VTODO);
	if (!pushVObject(VCTodoProp)) YYERROR;
	}
break;
case 43:
#line 355 "vcc.y"
{
	lexPopMode(0);
	popVObject();
	}
break;
case 44:
#line 360 "vcc.y"
{
	lexPushMode(L_VTODO);
	if (!pushVObject(VCTodoProp)) YYERROR;
	}
break;
case 45:
#line 365 "vcc.y"
{
	lexPopMode(0);
	popVObject();
	}
break;
#line 1482 "y_tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: after reduction, shifting from state 0 to\
 state %d\n", YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("yydebug: state %d, reading %d (%s)\n",
                        YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("yydebug: after reduction, shifting from state %d \
to state %d\n", *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}

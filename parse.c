/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "grammar.y" /* yacc.c:339  */

/*
   SDL_basic written by David Ashley, released 20080621 under the GPL
   http://www.linuxmotors.com/SDL_basic
   dashxdr@gmail.com
*/


#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "misc.h"

#define NAMELEN 16

typedef struct tokeninfo {
	char *at;
	union
	{
		double real;
		char *start;
		int integer;
		char name[NAMELEN];
		char string[256];
		int count;
		step *step;
		void *pointer;
	} value;
} tokeninfo;

typedef struct {
	int stepoff;
	char *at;
} lineref;


#define MAXSTEPS 100000
#define MAXLINEREFS 65536
typedef struct parse_state {
	bc *bc;
	int res;
	char *yypntr;
	char *yystart;
	char *yylast;
	step *nextstep;
	step steps[MAXSTEPS];
	int numlinerefs;
	variable *rankfailure;
	char errormsg[128];
	char *errorpos;
	lineref linerefs[MAXLINEREFS];
} ps;

static void rankcheck(ps *ps, int var, int rank);

static void addlineref(ps *ps, char *at)
{
	ps->linerefs[ps->numlinerefs].at = at;
	ps->linerefs[ps->numlinerefs++].stepoff = ps->nextstep - ps->steps;
}

static void addline(ps *ps, int number, int stepnum, char *at)
{
bc *bc = ps->bc;
	bc->lm[bc->numlines].step = stepnum;
	bc->lm[bc->numlines].src = at;
	bc->lm[bc->numlines++].linenumber = number;
}

static void adddata(ps *ps, double v)
{
	if(ps->bc->datanum < MAXDATA)
		ps->bc->data[ps->bc->datanum++] = v;
}

// returns linemap associated with this line
static linemap *findlinemap(ps *ps, int want)
{
int low, high, mid;
bc *bc=ps->bc;
	low = 0;
	high=bc->numlines;
	if(!high) return 0;
	for(;;)
	{
		mid = (low+high) >> 1;
		if(mid==low) break;
		if(want < bc->lm[mid].linenumber)
			high=mid;
		else
			low=mid;
	}
	if(want == bc->lm[mid].linenumber)
		return bc->lm + mid;
	else
		return 0;
}

// returns program step associated with this line
static int findline(ps *ps, int want)
{
linemap *lm;
	lm = findlinemap(ps, want);
	if(!lm)
		return -1;
	else
		return lm->step;
}

void seterror(ps *ps, char *p, char *msg, ...)
{
va_list ap;
	va_start(ap, msg);
	vsnprintf(ps->errormsg, sizeof(ps->errormsg), msg, ap);
	va_end(ap);
	ps->errorpos = p;
}

#define MAX (sizeof(int)*8/MODIFIER_BITS - 1)
int append_modifier(ps *ps, char *pos, int oldvalue, int toadd)
{
int i;
int t;
char *p;

	for(i=0;i<MAX;++i)
	{
		t=(oldvalue >> (i*MODIFIER_BITS)) & ((1<<MODIFIER_BITS)-1);
		if(t == toadd)
		{
			if(toadd == RENDER_ROUND) p="ROUND";
			else if(toadd == RENDER_ROTATE) p="ROTATE";
			else p="";
			seterror(ps, pos, "Duplicate modifier %s", p);
			break;
		}
	}
	if(i==MAX)
		oldvalue = (oldvalue<<MODIFIER_BITS) | toadd;
	return oldvalue;
}


static int fixuplinerefs(ps *ps)
{
bc *bc=ps->bc;
int i;
int o;
int at;
int line;

	for(i=0;i<ps->numlinerefs;++i)
	{
		o=ps->linerefs[i].stepoff;
		at=findline(ps, ps->steps[o+1].i);
		if(at<0)
		{
			for(line=0;line<bc->numlines;++line)
				if(bc->lm[line].step > o)
					break;
			tprintf(ps->bc,
				"Unresolved line reference on line %d\n",
				bc->lm[line-1].linenumber);
			return -1;
		}
		ps->steps[o+1].i = at - o;
	}
	return 0;
}


static void emitfunc(ps *ps, void (*func)())
{
	ps->nextstep++ -> func = func;
}

static void emitint(ps *ps, int i)
{
	ps->nextstep++ -> i = i;
}

static void emitfuncint(ps *ps, void (*func)(), int v)
{
	emitfunc(ps, func);
	emitint(ps, v);
}

static void emitdouble(ps *ps, double val)
{
	ps->nextstep++ -> d = val;
}

static void emitpushd(ps *ps, double val)
{
	emitfunc(ps, pushd);
	emitdouble(ps, val);
}

static void emitpushs(ps *ps, char *s)
{
int len=strlen(s);
step ts;
int i;
	emitfunc(ps, pushs);
	i=0;
	ts.str[i++]=len;
	do
	{
		if(*s)
			ts.str[i++] = *s++;
		if(i==STEPSIZE || !*s)
		{
			while(i<STEPSIZE)
				ts.str[i++] = 0;
			*ps->nextstep++ = ts;
			i=0;
		}
	} while(*s);
}

#define YYDEBUG 0
#define YYSTYPE tokeninfo

#define PS ((ps *)parm)


int yylex(YYSTYPE *lvalp, ps *parm);
void yyerror(ps *parm, char *s);


#line 297 "parse.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IF = 258,
    THEN = 259,
    ELSE = 260,
    ON = 261,
    GOTO = 262,
    GOSUB = 263,
    RETURN = 264,
    LET = 265,
    INPUT = 266,
    PRINT = 267,
    READ = 268,
    DATA = 269,
    DIM = 270,
    FOR = 271,
    TO = 272,
    NEXT = 273,
    STEP = 274,
    END = 275,
    STOP = 276,
    REM = 277,
    RESTORE = 278,
    INT = 279,
    FIX = 280,
    SGN = 281,
    SIN = 282,
    COS = 283,
    RND = 284,
    POW = 285,
    LOG = 286,
    EXP = 287,
    TAN = 288,
    ATN2 = 289,
    ATN = 290,
    ABS = 291,
    SQR = 292,
    LEN = 293,
    SPRITELOAD = 294,
    LEFTSTR = 295,
    MIDSTR = 296,
    RIGHTSTR = 297,
    CHRSTR = 298,
    STRSTR = 299,
    STRINGSTR = 300,
    VAL = 301,
    ASC = 302,
    TAB = 303,
    MOUSEX = 304,
    MOUSEY = 305,
    MOUSEB = 306,
    XSIZE = 307,
    YSIZE = 308,
    TICKS = 309,
    KEY = 310,
    KEYCODE = 311,
    INKEYSTR = 312,
    MOVE = 313,
    PEN = 314,
    LINE = 315,
    COLOR = 316,
    CLEAR = 317,
    RANDOM = 318,
    CLS = 319,
    FILL = 320,
    HOME = 321,
    CIRCLE = 322,
    DISC = 323,
    TEST = 324,
    BOX = 325,
    RECT = 326,
    SLEEP = 327,
    SPOT = 328,
    UPDATE = 329,
    INTEGER = 330,
    REAL = 331,
    NUMSYMBOL = 332,
    STRINGSYMBOL = 333,
    STRING = 334,
    ARC = 335,
    WEDGE = 336,
    SHINIT = 337,
    SHDONE = 338,
    SHEND = 339,
    SHLINE = 340,
    SHCURVE = 341,
    SHCUBIC = 342,
    ROUND = 343,
    ROTATE = 344,
    LF = 345,
    TONE = 346,
    ADSR = 347,
    WAVE = 348,
    FREQ = 349,
    DUR = 350,
    FMUL = 351,
    VOL = 352,
    WSIN = 353,
    WSQR = 354,
    WTRI = 355,
    WSAW = 356,
    QUIET = 357,
    NOTE = 358,
    OROR = 359,
    ANDAND = 360,
    NE = 361,
    LT = 362,
    GT = 363,
    LE = 364,
    GE = 365,
    OR = 366,
    XOR = 367,
    AND = 368,
    RR = 369,
    LL = 370,
    MOD = 371,
    POWER = 372,
    UNARY = 373,
    NOT = 374
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (ps *parm);



/* Copy the second part of user declarations.  */

#line 464 "parse.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  166
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1645

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  132
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  60
/* YYNRULES -- Number of rules.  */
#define YYNRULES  217
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  410

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   374

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     128,   129,   119,   117,   126,   118,     2,   120,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   125,   130,
       2,   106,     2,   127,   131,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   121,   122,   123,   124
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   263,   263,   265,   266,   270,   271,   275,   281,   285,
     286,   289,   291,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   343,   344,
     348,   349,   354,   355,   359,   360,   364,   367,   368,   372,
     373,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   390,   391,   394,   399,   403,   406,   407,   410,   411,
     415,   419,   420,   423,   424,   428,   432,   435,   438,   442,
     447,   452,   453,   458,   459,   463,   466,   471,   472,   476,
     478,   484,   485,   489,   490,   494,   495,   500,   501,   505,
     506,   509,   510,   516,   517,   523,   524,   527,   528,   529,
     530,   533,   534,   538,   539,   540,   544,   548,   552,   553,
     559,   560,   564,   565,   569,   570,   574,   575,   576,   577,
     578,   579,   580,   581,   582,   583,   584,   585,   586,   587,
     588,   589,   590,   591,   592,   593,   594,   595,   596,   597,
     598,   599,   600,   601,   604,   608,   609,   610,   611,   612,
     613,   614,   615,   616,   617,   618,   619,   620,   621,   622,
     623,   624,   625,   626,   627,   628,   632,   633,   634,   635,
     636,   637,   638,   641,   645,   646,   647,   651,   652,   653,
     654,   655,   659,   660,   661,   663,   664,   665
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IF", "THEN", "ELSE", "ON", "GOTO",
  "GOSUB", "RETURN", "LET", "INPUT", "PRINT", "READ", "DATA", "DIM", "FOR",
  "TO", "NEXT", "STEP", "END", "STOP", "REM", "RESTORE", "INT", "FIX",
  "SGN", "SIN", "COS", "RND", "POW", "LOG", "EXP", "TAN", "ATN2", "ATN",
  "ABS", "SQR", "LEN", "SPRITELOAD", "LEFTSTR", "MIDSTR", "RIGHTSTR",
  "CHRSTR", "STRSTR", "STRINGSTR", "VAL", "ASC", "TAB", "MOUSEX", "MOUSEY",
  "MOUSEB", "XSIZE", "YSIZE", "TICKS", "KEY", "KEYCODE", "INKEYSTR",
  "MOVE", "PEN", "LINE", "COLOR", "CLEAR", "RANDOM", "CLS", "FILL", "HOME",
  "CIRCLE", "DISC", "TEST", "BOX", "RECT", "SLEEP", "SPOT", "UPDATE",
  "INTEGER", "REAL", "NUMSYMBOL", "STRINGSYMBOL", "STRING", "ARC", "WEDGE",
  "SHINIT", "SHDONE", "SHEND", "SHLINE", "SHCURVE", "SHCUBIC", "ROUND",
  "ROTATE", "LF", "TONE", "ADSR", "WAVE", "FREQ", "DUR", "FMUL", "VOL",
  "WSIN", "WSQR", "WTRI", "WSAW", "QUIET", "NOTE", "OROR", "ANDAND", "'='",
  "NE", "LT", "GT", "LE", "GE", "OR", "XOR", "AND", "RR", "LL", "'+'",
  "'-'", "'*'", "'/'", "MOD", "POWER", "UNARY", "NOT", "':'", "','", "'?'",
  "'('", "')'", "';'", "'@'", "$accept", "program", "prog2", "line",
  "linenumber", "statements", "statement", "extrarender", "erlist",
  "eritem", "silist", "tonenumber", "otonelist", "tonelist", "toneitem",
  "print", "fixif", "fixifelse", "mark", "optstep", "optforvar", "forvar",
  "stint", "optthen", "num1", "num2", "num3", "num4", "num5", "num6",
  "num34", "dimarraylist", "dimarrayvar", "dimlist", "linelist",
  "datalist", "dataentry", "real", "readlist", "readvar", "numvar",
  "stringvar", "numlist", "printlist", "printsep", "printitem",
  "singlenumpar", "doublenumpar", "inputlist", "inputlist2", "inputvar",
  "assignexpr", "numexpr", "singlestringpar", "numfunc", "special",
  "specialstr", "stringexpr", "sitem", "stringfunc", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,    61,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,    43,    45,    42,
      47,   371,   372,   373,   374,    58,    44,    63,    40,    41,
      59,    64
};
# endif

#define YYPACT_NINF -201

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-201)))

#define YYTABLE_NINF -86

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     402,   808,   808,   -56,    20,  -201,   -44,   -70,  -201,   -44,
     -62,    -3,   -52,   -52,  -201,  -201,  -201,  -201,   808,   808,
     808,   808,   808,  -201,  -201,  -201,  -201,   808,   808,  -201,
     808,   808,   808,  -201,  -201,   -60,   -28,   808,   808,  -201,
    -201,  -201,   808,   808,   808,   808,   808,  -201,   101,    48,
    -201,     1,  -201,   700,    52,    22,    23,  -201,     5,     5,
       5,     5,     5,     5,     6,     5,     5,     5,     6,     5,
       5,     5,     7,     5,     8,     9,    10,     5,     5,    13,
       7,     7,     5,  -201,  -201,  -201,  -201,  -201,  -201,     5,
    -201,  -201,     5,  -201,  -201,  -201,     5,   808,   808,   808,
    -201,  -201,  -201,     0,  -201,  -201,  -201,   242,  -201,  -201,
     262,  -201,  -201,  -201,    12,  -201,  -201,  -201,    28,  -201,
      30,  -201,  -201,  -201,  -201,    32,  -201,  -201,    27,    31,
      38,  -201,  -201,    24,  -201,  -201,  -201,   595,  -201,  1488,
    -201,  -201,  -201,  -201,  1051,  -201,  -201,  1074,  -201,   -50,
    1097,   -50,  -201,   808,   808,  -201,  1120,  -201,  1143,  -201,
    -201,  -201,    39,  1488,  -201,  1488,  -201,  -201,   596,   808,
    -102,  -201,  1488,   242,  -201,  -201,   808,   723,   808,  -201,
    -201,  -201,  -201,  -201,  -201,   808,  -201,  -201,  -201,  -201,
    -201,  -201,  -201,  -201,   723,  -201,  -201,   723,   723,   723,
    -201,  -201,   808,  -201,  -201,  -201,  -201,  -201,  -201,  -201,
    -201,   895,  -201,   808,   808,   808,   808,   808,   808,   808,
     808,   808,   808,   808,   808,   808,   808,   808,   808,   808,
     808,   808,  -201,   723,   723,   723,  -201,    57,    57,   -44,
     -44,   -44,   -62,    91,    91,    -3,   808,   808,   808,   808,
     808,   808,  -201,   -50,  -201,   808,  -201,   -78,  1488,   -76,
     808,   808,   451,  -201,    41,  -201,  1488,  -201,  -201,   700,
     596,  1488,   849,   921,  1166,    14,    46,   751,   831,  1189,
    -201,  1506,  1523,   418,   418,   418,   418,   418,   418,   574,
     574,    29,   -39,   -39,   -55,   -55,    47,    47,    47,  -201,
     499,   849,   849,  -201,  -201,    44,    44,    28,  -201,  -201,
    -201,  -201,   -53,   -27,  -201,   330,  1488,  1212,  1235,  1488,
    1488,  -201,  1258,   808,  -201,  -201,  1281,  1304,   723,  -201,
     808,   808,   808,   808,  -201,  -201,  -201,  -201,  -201,   451,
    -201,   -84,  -201,   808,  -201,   808,   808,   808,   723,  -201,
       1,   166,    98,    99,  -201,  -201,   808,   808,   808,   808,
    1488,   808,   808,   849,  1488,  1488,  1488,  1488,  -201,  -201,
     947,   973,  1327,   999,   196,  -201,  -201,  -201,  -201,   523,
    1350,  1488,  1350,  1373,  1396,  -201,  -201,   808,  -201,  -201,
     499,   808,  -201,   808,   808,   808,  1025,  -201,  1488,  1488,
    1419,  1442,  -201,  -201,   808,   808,  1488,  1465,   808,  1488
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     0,     0,     0,    45,     0,     0,    81,     0,
       0,     0,     0,    88,    18,    19,    46,    55,     0,     0,
       0,     0,     0,    54,    23,    27,    24,     0,     0,    57,
       0,     0,     0,    40,    41,   121,   123,     0,     0,    34,
      35,    36,     0,     0,     0,     0,    64,    82,     0,     3,
       5,     4,     9,   127,     0,     0,     0,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   196,   197,   198,   199,   200,   201,     0,
     202,   203,     0,   115,   116,   207,     0,     0,     0,     0,
     173,   172,   210,    93,   170,   171,   208,     0,   204,   209,
       0,    13,    44,    14,     0,   142,   143,    49,   138,   140,
      50,   117,   119,   120,   114,    51,   111,   113,     0,     0,
      17,   103,    90,     0,    43,    89,    28,     0,    21,    95,
      29,   101,   102,    22,     0,    56,    25,     0,    26,    58,
       0,    58,    20,     0,     0,    31,     0,    32,     0,    37,
      38,    39,    67,    66,    52,    65,     1,     6,     0,     0,
      16,   128,   134,   135,     8,    85,     0,     0,     0,   175,
     176,   177,   178,   179,   180,     0,   181,   182,   183,   184,
     186,   185,   187,   188,     0,   192,   195,     0,     0,     0,
     215,   216,     0,   193,   194,   211,   190,   189,   191,   146,
     147,     0,    94,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    83,     0,     0,     0,   206,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    30,    59,    60,     0,    33,     0,   125,     0,
       0,     0,     0,    53,    68,    10,   133,   132,   131,   130,
       0,   144,   145,     0,     0,     0,     0,     0,     0,     0,
     148,   167,   166,   157,   158,   159,   160,   161,   162,   164,
     165,   163,   156,   155,   149,   150,   151,   152,   153,   154,
       0,   168,   169,   205,   109,    47,    48,   139,   141,   118,
     112,   107,     0,     0,   104,     0,    96,     0,     0,    62,
      63,    61,     0,     0,   122,   124,     0,     0,     0,    72,
       0,     0,     0,     0,    77,    79,    78,    80,    69,     0,
     129,     0,   136,     0,   174,     0,     0,     0,     0,    92,
      91,    85,     0,     0,   105,   106,     0,     0,     0,     0,
     126,     0,     0,    71,    73,    74,    76,    75,    70,     7,
       0,     0,     0,     0,     0,    84,    11,   110,   108,    86,
      97,    97,     0,     0,     0,   137,   212,     0,   213,   217,
       0,     0,    42,     0,     0,     0,     0,    85,    87,    98,
       0,     0,   214,    12,     0,     0,    99,     0,     0,   100
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -201,  -201,  -201,   129,  -201,     2,    15,    34,  -201,   -67,
    -201,  -201,  -201,  -201,  -152,  -201,  -201,  -201,  -163,  -201,
    -201,   176,  -200,  -201,   -17,   -10,    33,    -8,  -201,   147,
    -201,  -201,   -51,   -49,   -46,  -201,   -43,    -7,  -201,   -48,
      40,    63,    49,  -201,  -201,   -71,   902,   132,  -201,   -35,
     -38,   201,    -1,     4,  -201,  -201,  -201,   -37,   -96,  -201
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    48,    49,    50,   175,   350,    52,   252,   253,   254,
     164,   162,   263,   264,   338,    53,   300,   390,    54,   392,
     134,   133,   351,   232,   138,   136,   141,   142,   155,   157,
     143,   130,   131,   312,   305,   125,   126,   100,   120,   121,
     101,   102,   257,   170,   269,   171,   179,   186,   117,   118,
     119,    57,   137,   195,   104,   105,   106,   107,   108,   109
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     103,   110,    51,   127,   212,   145,   369,    35,    36,   114,
     140,   236,   270,    93,    94,   152,   173,   124,   139,   111,
     144,   139,   149,   151,   267,   132,   147,   147,   268,   150,
     150,   139,   159,    35,    36,   160,   156,   158,   250,   251,
      55,   168,   150,   158,   163,   165,    55,   115,   323,   122,
     323,   324,   172,   325,    74,    75,    76,    77,    78,    79,
     146,   148,    82,    56,   228,   229,   230,   231,   153,    56,
     116,    91,   123,   353,   128,   129,   354,   236,   226,   227,
     228,   229,   230,   231,   203,   204,    74,    75,    76,    77,
      78,    79,    36,    95,    82,   112,   209,   210,   211,   353,
     154,   166,   355,    91,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,   -85,    36,    95,   168,   174,   176,   177,
     246,   235,   304,   178,   185,   194,   197,   198,   199,   303,
     272,   202,   239,   344,   224,   225,   226,   227,   228,   229,
     230,   231,   258,   258,   240,   243,   241,   275,   242,   244,
     276,   277,   278,   235,   245,   262,   311,   339,   266,   231,
     352,   375,   345,   377,   378,   271,   236,   273,   167,   236,
     236,   236,   236,   265,   274,   256,   321,   368,   376,   135,
     397,   161,   306,   309,   314,   313,   301,   302,   340,   310,
     190,   279,   308,   259,   307,   236,   236,   113,    55,     0,
       0,     0,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,    56,   173,     0,   403,   127,    74,    75,    76,    77,
      78,    79,     0,     0,    82,   315,   316,   317,   318,   319,
     320,     0,     0,    91,   322,     0,     0,     0,     0,   326,
     327,     0,     0,     0,     0,     0,     0,   236,   172,   237,
     238,     0,   341,     0,    36,    95,     0,     0,   236,   115,
     115,   122,    74,    75,    76,    77,    78,    79,     0,     0,
      82,   363,     0,     0,     0,     0,     0,     0,     0,    91,
       0,     0,   116,   116,   123,     0,     0,     0,     0,     0,
      55,   374,     0,   235,     0,     0,     0,     0,     0,     0,
      36,    95,   360,     0,     0,   389,     0,     0,     0,   364,
     365,   366,   367,    56,     0,     0,     0,     0,     0,     0,
      55,     0,   370,     0,   371,   372,   373,   356,   233,   234,
       0,     0,     0,     0,     0,   379,   380,   381,   382,   235,
     383,   384,     0,    56,     0,     0,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,     0,   396,     0,     0,     0,
     398,     0,   399,   400,   401,     0,     0,     0,     0,     0,
       0,     0,     0,   406,   407,     1,     0,   409,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,     0,
      13,     0,    14,    15,    16,    17,     0,     0,     0,     0,
      55,     0,     0,     0,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,    56,     0,     0,     0,     0,     0,     0,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,   -85,     0,    35,
      36,     0,    37,    38,    39,    40,    41,    42,    43,    44,
       0,     0,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,     1,     0,    46,     2,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,     0,    13,     0,    14,
      15,    16,    17,     0,     0,     0,     0,     0,     0,    47,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,     0,   391,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,     0,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,   349,     0,    35,    36,     0,    37,
      38,    39,    40,    41,    42,    43,    44,     0,     0,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,     1,
       0,    46,     2,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,     0,    13,     0,    14,    15,    16,    17,
       0,     0,     0,     0,     0,     0,    47,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,     0,     0,     0,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,     0,     0,    35,    36,     0,    37,    38,    39,    40,
      41,    42,    43,    44,     0,     0,     0,    45,   223,   224,
     225,   226,   227,   228,   229,   230,   231,     0,    46,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,     0,     0,
       0,   247,     0,    47,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,     0,    74,    75,    76,    77,    78,    79,     0,
       0,    82,    92,     0,     0,    93,    94,    35,    36,    95,
      91,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    74,    75,    76,    77,    78,    79,     0,     0,    82,
       0,    36,    95,    96,     0,     0,     0,     0,    91,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    97,     0,
       0,     0,     0,     0,    98,     0,     0,     0,    99,    36,
      95,   169,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,     0,     0,   235,     0,
       0,    74,    75,    76,    77,    78,    79,   346,     0,    82,
      92,     0,     0,    93,    94,    35,    36,    95,    91,    74,
      75,    76,    77,    78,    79,     0,     0,    82,     0,     0,
       0,     0,     0,     0,     0,     0,    91,     0,     0,    36,
      95,    96,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    97,    36,    95,     0,
       0,     0,    98,     0,     0,     0,    99,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   235,     0,
       0,     0,     0,     0,     0,     0,     0,   347,     0,     0,
       0,   180,   181,   182,   183,   184,   235,   187,   188,   189,
       0,   191,   192,   193,     0,   196,     0,     0,     0,   200,
     201,     0,     0,     0,   205,     0,     0,     0,     0,     0,
       0,   206,     0,     0,   207,     0,     0,     0,   208,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,     0,     0,
       0,     0,     0,     0,   280,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,     0,     0,     0,     0,     0,     0,
     342,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
       0,     0,     0,     0,     0,     0,   385,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,     0,     0,     0,     0,
       0,     0,   386,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,     0,     0,     0,     0,     0,     0,   388,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,     0,     0,
       0,     0,     0,     0,   402,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,     0,     0,     0,   248,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,     0,     0,     0,
     249,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
       0,     0,     0,   255,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,     0,     0,     0,   260,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,     0,     0,     0,   261,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,     0,
       0,     0,   343,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,     0,     0,     0,   348,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,     0,     0,     0,   357,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,     0,     0,
       0,   358,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,     0,     0,     0,   359,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,     0,     0,     0,   361,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,     0,     0,     0,
     362,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
       0,     0,     0,   387,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,   228,
     229,   230,   231,     0,     0,     0,   393,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,     0,     0,     0,   394,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,     0,
       0,     0,   395,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,     0,     0,     0,   404,   213,   214,   215,   216,
     217,   218,   219,   220,   221,   222,   223,   224,   225,   226,
     227,   228,   229,   230,   231,     0,     0,     0,   405,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,     0,     0,
       0,   408,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231
};

static const yytype_int16 yycheck[] =
{
       1,     2,     0,    10,     4,    22,    90,    77,    78,    79,
      20,   107,   175,    75,    76,    32,    53,    79,    19,    75,
      21,    22,    30,    31,   126,    77,    27,    28,   130,    30,
      31,    32,    42,    77,    78,    43,    37,    38,    88,    89,
       0,   125,    43,    44,    45,    46,     6,     7,   126,     9,
     126,   129,    53,   129,    40,    41,    42,    43,    44,    45,
      27,    28,    48,     0,   119,   120,   121,   122,   128,     6,
       7,    57,     9,   126,    77,    78,   129,   173,   117,   118,
     119,   120,   121,   122,    80,    81,    40,    41,    42,    43,
      44,    45,    78,    79,    48,    75,    97,    98,    99,   126,
     128,     0,   129,    57,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    75,    78,    79,   125,    75,   106,   106,
     106,   117,    75,   128,   128,   128,   128,   128,   128,   235,
     177,   128,   130,   129,   115,   116,   117,   118,   119,   120,
     121,   122,   153,   154,   126,   128,   126,   194,   126,   128,
     197,   198,   199,   117,   126,   126,    75,   126,   169,   122,
     126,     5,   126,    75,    75,   176,   272,   178,    49,   275,
     276,   277,   278,   168,   185,   151,   253,   339,   351,    13,
     390,    44,   238,   241,   245,   244,   233,   234,   269,   242,
      68,   202,   240,   154,   239,   301,   302,     6,   168,    -1,
      -1,    -1,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   168,   269,    -1,   397,   242,    40,    41,    42,    43,
      44,    45,    -1,    -1,    48,   246,   247,   248,   249,   250,
     251,    -1,    -1,    57,   255,    -1,    -1,    -1,    -1,   260,
     261,    -1,    -1,    -1,    -1,    -1,    -1,   363,   269,     7,
       8,    -1,   270,    -1,    78,    79,    -1,    -1,   374,   239,
     240,   241,    40,    41,    42,    43,    44,    45,    -1,    -1,
      48,   328,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,   239,   240,   241,    -1,    -1,    -1,    -1,    -1,
     270,   348,    -1,   117,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    79,   323,    -1,    -1,   129,    -1,    -1,    -1,   330,
     331,   332,   333,   270,    -1,    -1,    -1,    -1,    -1,    -1,
     300,    -1,   343,    -1,   345,   346,   347,    17,   106,   107,
      -1,    -1,    -1,    -1,    -1,   356,   357,   358,   359,   117,
     361,   362,    -1,   300,    -1,    -1,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,   387,    -1,    -1,    -1,
     391,    -1,   393,   394,   395,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   404,   405,     3,    -1,   408,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    -1,
      18,    -1,    20,    21,    22,    23,    -1,    -1,    -1,    -1,
     390,    -1,    -1,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   390,    -1,    -1,    -1,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    -1,    77,
      78,    -1,    80,    81,    82,    83,    84,    85,    86,    87,
      -1,    -1,    -1,    91,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,    -1,   102,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    18,    -1,    20,
      21,    22,    23,    -1,    -1,    -1,    -1,    -1,    -1,   127,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,    19,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,    -1,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    -1,    77,    78,    -1,    80,
      81,    82,    83,    84,    85,    86,    87,    -1,    -1,    -1,
      91,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
      -1,   102,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    -1,    18,    -1,    20,    21,    22,    23,
      -1,    -1,    -1,    -1,    -1,    -1,   127,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    -1,    -1,    77,    78,    -1,    80,    81,    82,    83,
      84,    85,    86,    87,    -1,    -1,    -1,    91,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,   102,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,    -1,   127,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    -1,    -1,
      -1,    -1,    -1,    40,    41,    42,    43,    44,    45,    -1,
      -1,    48,    72,    -1,    -1,    75,    76,    77,    78,    79,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    41,    42,    43,    44,    45,    -1,    -1,    48,
      -1,    78,    79,   103,    -1,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    -1,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,   128,    78,
      79,   131,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    -1,    -1,   117,    -1,
      -1,    40,    41,    42,    43,    44,    45,   126,    -1,    48,
      72,    -1,    -1,    75,    76,    77,    78,    79,    57,    40,
      41,    42,    43,    44,    45,    -1,    -1,    48,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    57,    -1,    -1,    78,
      79,   103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   118,    78,    79,    -1,
      -1,    -1,   124,    -1,    -1,    -1,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   126,    -1,    -1,
      -1,    59,    60,    61,    62,    63,   117,    65,    66,    67,
      -1,    69,    70,    71,    -1,    73,    -1,    -1,    -1,    77,
      78,    -1,    -1,    -1,    82,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    -1,    -1,    92,    -1,    -1,    -1,    96,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,    -1,    -1,    -1,
     129,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,    -1,    -1,    -1,   129,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,    -1,    -1,    -1,    -1,    -1,   129,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,   126,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
     126,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,   126,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,   126,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,   126,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,   126,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,    -1,    -1,   126,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,   126,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,    -1,    -1,    -1,   126,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,   126,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    -1,    -1,    -1,
     126,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
      -1,    -1,    -1,   126,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,    -1,    -1,    -1,   126,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,    -1,    -1,    -1,   126,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
      -1,    -1,   126,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    -1,    -1,    -1,   126,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    -1,    -1,    -1,   126,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,    -1,    -1,
      -1,   126,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    18,    20,    21,    22,    23,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    77,    78,    80,    81,    82,
      83,    84,    85,    86,    87,    91,   102,   127,   133,   134,
     135,   137,   138,   147,   150,   172,   173,   183,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    72,    75,    76,    79,   103,   118,   124,   128,
     169,   172,   173,   184,   186,   187,   188,   189,   190,   191,
     184,    75,    75,   183,    79,   172,   173,   180,   181,   182,
     170,   171,   172,   173,    79,   167,   168,   169,    77,    78,
     163,   164,    77,   153,   152,   153,   157,   184,   156,   184,
     157,   158,   159,   162,   184,   156,   158,   184,   158,   159,
     184,   159,   156,   128,   128,   160,   184,   161,   184,   157,
     159,   161,   143,   184,   142,   184,     0,   135,   125,   131,
     175,   177,   184,   189,    75,   136,   106,   106,   128,   178,
     178,   178,   178,   178,   178,   128,   179,   178,   178,   178,
     179,   178,   178,   178,   128,   185,   178,   128,   128,   128,
     178,   178,   128,   185,   185,   178,   178,   178,   178,   184,
     184,   184,     4,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   155,   106,   107,   117,   190,     7,     8,   130,
     126,   126,   126,   128,   128,   126,   106,   126,   126,   126,
      88,    89,   139,   140,   141,   126,   139,   174,   184,   174,
     126,   126,   126,   144,   145,   138,   184,   126,   130,   176,
     150,   184,   189,   184,   184,   189,   189,   189,   189,   184,
     129,   184,   184,   184,   184,   184,   184,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   184,   184,   184,
     148,   189,   189,   190,    75,   166,   166,   181,   182,   171,
     168,    75,   165,   165,   164,   184,   184,   184,   184,   184,
     184,   141,   184,   126,   129,   129,   184,   184,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   146,   126,
     177,   137,   129,   126,   129,   126,   126,   126,   126,    75,
     137,   154,   126,   126,   129,   129,    17,   126,   126,   126,
     184,   126,   126,   189,   184,   184,   184,   184,   146,    90,
     184,   184,   184,   184,   189,     5,   150,    75,    75,   184,
     184,   184,   184,   184,   184,   129,   129,   126,   129,   129,
     149,    19,   151,   126,   126,   126,   184,   154,   184,   184,
     184,   184,   129,   150,   126,   126,   184,   184,   126,   184
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   132,   133,   133,   133,   134,   134,   135,   136,   137,
     137,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   139,   139,
     140,   140,   141,   141,   142,   142,   143,   144,   144,   145,
     145,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   147,   147,   148,   149,   150,   151,   151,   152,   152,
     153,   154,   154,   155,   155,   156,   157,   158,   159,   160,
     161,   162,   162,   163,   163,   164,   164,   165,   165,   166,
     166,   167,   167,   168,   168,   169,   169,   170,   170,   171,
     171,   172,   172,   173,   173,   174,   174,   175,   175,   175,
     175,   176,   176,   177,   177,   177,   178,   179,   180,   180,
     181,   181,   182,   182,   183,   183,   184,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   184,   184,   184,
     184,   184,   184,   184,   184,   184,   184,   184,   184,   184,
     184,   184,   184,   184,   185,   186,   186,   186,   186,   186,
     186,   186,   186,   186,   186,   186,   186,   186,   186,   186,
     186,   186,   186,   186,   186,   186,   187,   187,   187,   187,
     187,   187,   187,   188,   189,   189,   189,   190,   190,   190,
     190,   190,   191,   191,   191,   191,   191,   191
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     1,     2,     5,     1,     1,
       3,     6,     9,     2,     2,     1,     2,     2,     1,     1,
       2,     2,     2,     1,     1,     2,     2,     1,     2,     2,
       3,     2,     2,     3,     1,     1,     1,     2,     2,     2,
       1,     1,     7,     2,     2,     1,     1,     4,     4,     2,
       2,     2,     2,     3,     1,     1,     2,     1,     0,     1,
       1,     2,     2,     2,     0,     1,     1,     0,     1,     2,
       3,     2,     1,     2,     2,     2,     2,     1,     1,     1,
       1,     1,     1,     0,     0,     0,     0,     2,     0,     1,
       1,     1,     1,     0,     1,     1,     3,     5,     7,     9,
      11,     1,     1,     1,     3,     4,     4,     1,     3,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     4,     1,     4,     1,     3,     0,     1,     3,
       2,     1,     1,     2,     1,     1,     3,     5,     1,     3,
       1,     3,     1,     1,     3,     3,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     1,     1,     3,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     1,
       1,     2,     6,     6,     8,     2,     2,     6
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (parm, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, parm); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, ps *parm)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (parm);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, ps *parm)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, parm);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, ps *parm)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , parm);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, parm); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, ps *parm)
{
  YYUSE (yyvaluep);
  YYUSE (parm);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (ps *parm)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, parm);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 7:
#line 275 "grammar.y" /* yacc.c:1646  */
    {addline(PS, (yyvsp[-3]).value.integer,
					(yyvsp[-4]).value.step - PS->steps,
					(yyvsp[-3]).at);}
#line 2107 "parse.c" /* yacc.c:1646  */
    break;

  case 11:
#line 290 "grammar.y" /* yacc.c:1646  */
    {(yyvsp[-2]).value.step[1].i = (yyvsp[0]).value.step - (yyvsp[-2]).value.step;}
#line 2113 "parse.c" /* yacc.c:1646  */
    break;

  case 12:
#line 292 "grammar.y" /* yacc.c:1646  */
    {(yyvsp[-5]).value.step[1].i = (yyvsp[-2]).value.step - (yyvsp[-5]).value.step;
		(yyvsp[-2]).value.step[-1].i = (yyvsp[0]).value.step - (yyvsp[-2]).value.step+2;}
#line 2120 "parse.c" /* yacc.c:1646  */
    break;

  case 13:
#line 294 "grammar.y" /* yacc.c:1646  */
    {addlineref(PS, (yyvsp[0]).at);emitfuncint(PS, rjmp, (yyvsp[0]).value.integer);}
#line 2126 "parse.c" /* yacc.c:1646  */
    break;

  case 14:
#line 295 "grammar.y" /* yacc.c:1646  */
    {/* implemented */}
#line 2132 "parse.c" /* yacc.c:1646  */
    break;

  case 15:
#line 296 "grammar.y" /* yacc.c:1646  */
    {/* implemented */}
#line 2138 "parse.c" /* yacc.c:1646  */
    break;

  case 16:
#line 297 "grammar.y" /* yacc.c:1646  */
    {if((yyvsp[0]).value.integer) emitfunc(PS, lf);}
#line 2144 "parse.c" /* yacc.c:1646  */
    break;

  case 18:
#line 299 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, performend);}
#line 2150 "parse.c" /* yacc.c:1646  */
    break;

  case 19:
#line 300 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, performstop);}
#line 2156 "parse.c" /* yacc.c:1646  */
    break;

  case 20:
#line 301 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, sleepd);emitfunc(PS, pop);}
#line 2162 "parse.c" /* yacc.c:1646  */
    break;

  case 21:
#line 302 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, performpen);}
#line 2168 "parse.c" /* yacc.c:1646  */
    break;

  case 22:
#line 303 "grammar.y" /* yacc.c:1646  */
    {if((yyvsp[0]).value.count==3) emitfunc(PS, color3);
			else emitfunc(PS, color4);}
#line 2175 "parse.c" /* yacc.c:1646  */
    break;

  case 23:
#line 305 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, cls);}
#line 2181 "parse.c" /* yacc.c:1646  */
    break;

  case 24:
#line 306 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, home);}
#line 2187 "parse.c" /* yacc.c:1646  */
    break;

  case 25:
#line 307 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, performcircle);}
#line 2193 "parse.c" /* yacc.c:1646  */
    break;

  case 26:
#line 308 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, performdisc);}
#line 2199 "parse.c" /* yacc.c:1646  */
    break;

  case 27:
#line 309 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, performfill);}
#line 2205 "parse.c" /* yacc.c:1646  */
    break;

  case 28:
#line 310 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, performmove);}
#line 2211 "parse.c" /* yacc.c:1646  */
    break;

  case 29:
#line 311 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, performline);}
#line 2217 "parse.c" /* yacc.c:1646  */
    break;

  case 30:
#line 312 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, box, (yyvsp[0]).value.integer);}
#line 2223 "parse.c" /* yacc.c:1646  */
    break;

  case 31:
#line 313 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, arc);}
#line 2229 "parse.c" /* yacc.c:1646  */
    break;

  case 32:
#line 314 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, wedge);}
#line 2235 "parse.c" /* yacc.c:1646  */
    break;

  case 33:
#line 315 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, rect, (yyvsp[0]).value.integer);}
#line 2241 "parse.c" /* yacc.c:1646  */
    break;

  case 34:
#line 316 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, shinit);}
#line 2247 "parse.c" /* yacc.c:1646  */
    break;

  case 35:
#line 317 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, shdone);}
#line 2253 "parse.c" /* yacc.c:1646  */
    break;

  case 36:
#line 318 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, shend);}
#line 2259 "parse.c" /* yacc.c:1646  */
    break;

  case 37:
#line 319 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, shline);}
#line 2265 "parse.c" /* yacc.c:1646  */
    break;

  case 38:
#line 320 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, shcurve);}
#line 2271 "parse.c" /* yacc.c:1646  */
    break;

  case 39:
#line 321 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, shcubic);}
#line 2277 "parse.c" /* yacc.c:1646  */
    break;

  case 40:
#line 322 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, spot);}
#line 2283 "parse.c" /* yacc.c:1646  */
    break;

  case 41:
#line 323 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, forceupdate);}
#line 2289 "parse.c" /* yacc.c:1646  */
    break;

  case 42:
#line 325 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, pushav, (yyvsp[-5]).value.integer);emitfunc(PS, performfor);}
#line 2295 "parse.c" /* yacc.c:1646  */
    break;

  case 44:
#line 327 "grammar.y" /* yacc.c:1646  */
    {addlineref(PS, (yyvsp[0]).at);emitfuncint(PS, rcall, (yyvsp[0]).value.integer);}
#line 2301 "parse.c" /* yacc.c:1646  */
    break;

  case 45:
#line 328 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, ret);}
#line 2307 "parse.c" /* yacc.c:1646  */
    break;

  case 46:
#line 329 "grammar.y" /* yacc.c:1646  */
    {/* do nothing */}
#line 2313 "parse.c" /* yacc.c:1646  */
    break;

  case 47:
#line 330 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, ongoto, (yyvsp[0]).value.count);}
#line 2319 "parse.c" /* yacc.c:1646  */
    break;

  case 48:
#line 331 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, ongosub, (yyvsp[0]).value.count);}
#line 2325 "parse.c" /* yacc.c:1646  */
    break;

  case 49:
#line 332 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, input, (yyvsp[0]).value.count);}
#line 2331 "parse.c" /* yacc.c:1646  */
    break;

  case 50:
#line 333 "grammar.y" /* yacc.c:1646  */
    {/* implemented */}
#line 2337 "parse.c" /* yacc.c:1646  */
    break;

  case 51:
#line 334 "grammar.y" /* yacc.c:1646  */
    {/* implemented */}
#line 2343 "parse.c" /* yacc.c:1646  */
    break;

  case 53:
#line 336 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, soundgo);}
#line 2349 "parse.c" /* yacc.c:1646  */
    break;

  case 57:
#line 340 "grammar.y" /* yacc.c:1646  */
    {rendertest(PS->bc);}
#line 2355 "parse.c" /* yacc.c:1646  */
    break;

  case 58:
#line 343 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.integer = 0;}
#line 2361 "parse.c" /* yacc.c:1646  */
    break;

  case 61:
#line 349 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.integer =
				append_modifier(PS, (yyvsp[-1]).at, (yyvsp[-1]).value.integer,
					(yyvsp[0]).value.integer);}
#line 2369 "parse.c" /* yacc.c:1646  */
    break;

  case 62:
#line 354 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.integer = RENDER_ROUND;}
#line 2375 "parse.c" /* yacc.c:1646  */
    break;

  case 63:
#line 355 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.integer = RENDER_ROTATE;}
#line 2381 "parse.c" /* yacc.c:1646  */
    break;

  case 64:
#line 359 "grammar.y" /* yacc.c:1646  */
    {emitpushd(PS, 0.0);emitfunc(PS, quiet);}
#line 2387 "parse.c" /* yacc.c:1646  */
    break;

  case 65:
#line 360 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, quiet);}
#line 2393 "parse.c" /* yacc.c:1646  */
    break;

  case 66:
#line 364 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, setsound);}
#line 2399 "parse.c" /* yacc.c:1646  */
    break;

  case 73:
#line 379 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, freq);}
#line 2405 "parse.c" /* yacc.c:1646  */
    break;

  case 74:
#line 380 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, dur);}
#line 2411 "parse.c" /* yacc.c:1646  */
    break;

  case 75:
#line 381 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, vol);}
#line 2417 "parse.c" /* yacc.c:1646  */
    break;

  case 76:
#line 382 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, fmul);}
#line 2423 "parse.c" /* yacc.c:1646  */
    break;

  case 77:
#line 383 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, wsin);}
#line 2429 "parse.c" /* yacc.c:1646  */
    break;

  case 78:
#line 384 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, wtri);}
#line 2435 "parse.c" /* yacc.c:1646  */
    break;

  case 79:
#line 385 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, wsqr);}
#line 2441 "parse.c" /* yacc.c:1646  */
    break;

  case 80:
#line 386 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, wsaw);}
#line 2447 "parse.c" /* yacc.c:1646  */
    break;

  case 83:
#line 394 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, skip2ne);
		(yyval).value.step = PS->nextstep;
		emitfuncint(PS, rjmp, 0);}
#line 2455 "parse.c" /* yacc.c:1646  */
    break;

  case 84:
#line 399 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, rjmp, 0); // true side to skip over false
		(yyval).value.step = PS->nextstep;}
#line 2462 "parse.c" /* yacc.c:1646  */
    break;

  case 85:
#line 403 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.step = PS->nextstep;}
#line 2468 "parse.c" /* yacc.c:1646  */
    break;

  case 86:
#line 406 "grammar.y" /* yacc.c:1646  */
    { emitpushd(PS, 1.0);}
#line 2474 "parse.c" /* yacc.c:1646  */
    break;

  case 88:
#line 410 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, performnext);}
#line 2480 "parse.c" /* yacc.c:1646  */
    break;

  case 89:
#line 411 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, pushav, (yyvsp[0]).value.integer);emitfunc(PS, performnext1);}
#line 2486 "parse.c" /* yacc.c:1646  */
    break;

  case 92:
#line 420 "grammar.y" /* yacc.c:1646  */
    {addlineref(PS, (yyvsp[0]).at);emitfuncint(PS, rjmp, (yyvsp[0]).value.integer);}
#line 2492 "parse.c" /* yacc.c:1646  */
    break;

  case 95:
#line 428 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count=1;}
#line 2498 "parse.c" /* yacc.c:1646  */
    break;

  case 96:
#line 432 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = 2;}
#line 2504 "parse.c" /* yacc.c:1646  */
    break;

  case 97:
#line 435 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = 3;}
#line 2510 "parse.c" /* yacc.c:1646  */
    break;

  case 98:
#line 438 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = 4;}
#line 2516 "parse.c" /* yacc.c:1646  */
    break;

  case 99:
#line 443 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = 5;}
#line 2522 "parse.c" /* yacc.c:1646  */
    break;

  case 100:
#line 448 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = 6;}
#line 2528 "parse.c" /* yacc.c:1646  */
    break;

  case 105:
#line 463 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, pushav, (yyvsp[-3]).value.integer);
				emitfuncint(PS, dimd, (yyvsp[-1]).value.count);
				rankcheck(PS, (yyvsp[-3]).value.integer, (yyvsp[-1]).value.count);}
#line 2536 "parse.c" /* yacc.c:1646  */
    break;

  case 106:
#line 466 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, pushav, (yyvsp[-3]).value.integer);
				emitfuncint(PS, dims, (yyvsp[-1]).value.count);
				rankcheck(PS, (yyvsp[-3]).value.integer, (yyvsp[-1]).value.count);}
#line 2544 "parse.c" /* yacc.c:1646  */
    break;

  case 107:
#line 471 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = 1;emitfuncint(PS, pushi, (yyvsp[0]).value.integer);}
#line 2550 "parse.c" /* yacc.c:1646  */
    break;

  case 108:
#line 472 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = (yyvsp[-2]).value.count + 1;
				emitfuncint(PS, pushi, (yyvsp[0]).value.integer);}
#line 2557 "parse.c" /* yacc.c:1646  */
    break;

  case 109:
#line 476 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = 1;addlineref(PS, (yyvsp[0]).at);
				emitfuncint(PS, pushea, (yyvsp[0]).value.integer);}
#line 2564 "parse.c" /* yacc.c:1646  */
    break;

  case 110:
#line 478 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = (yyvsp[-2]).value.count + 1;
				addlineref(PS, (yyvsp[0]).at);
				emitfuncint(PS, pushea, (yyvsp[0]).value.integer);}
#line 2572 "parse.c" /* yacc.c:1646  */
    break;

  case 113:
#line 489 "grammar.y" /* yacc.c:1646  */
    {adddata(PS, (yyvsp[0]).value.real);}
#line 2578 "parse.c" /* yacc.c:1646  */
    break;

  case 115:
#line 494 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.real = (double)(yyvsp[0]).value.integer;}
#line 2584 "parse.c" /* yacc.c:1646  */
    break;

  case 119:
#line 505 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, readd);}
#line 2590 "parse.c" /* yacc.c:1646  */
    break;

  case 121:
#line 509 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, pushvd, (yyvsp[0]).value.integer);}
#line 2596 "parse.c" /* yacc.c:1646  */
    break;

  case 122:
#line 511 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, arrayd, (yyvsp[-3]).value.integer);
			rankcheck(PS, (yyvsp[-3]).value.integer, (yyvsp[-1]).value.count);}
#line 2603 "parse.c" /* yacc.c:1646  */
    break;

  case 123:
#line 516 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, pushvs, (yyvsp[0]).value.integer);}
#line 2609 "parse.c" /* yacc.c:1646  */
    break;

  case 124:
#line 518 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, arrays, (yyvsp[-3]).value.integer);
			rankcheck(PS, (yyvsp[-3]).value.integer, (yyvsp[-1]).value.count);}
#line 2616 "parse.c" /* yacc.c:1646  */
    break;

  case 125:
#line 523 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = 1;}
#line 2622 "parse.c" /* yacc.c:1646  */
    break;

  case 126:
#line 524 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = (yyvsp[-2]).value.count + 1;}
#line 2628 "parse.c" /* yacc.c:1646  */
    break;

  case 127:
#line 527 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.integer = 1;}
#line 2634 "parse.c" /* yacc.c:1646  */
    break;

  case 128:
#line 528 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.integer = 1;}
#line 2640 "parse.c" /* yacc.c:1646  */
    break;

  case 129:
#line 529 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.integer = 1;}
#line 2646 "parse.c" /* yacc.c:1646  */
    break;

  case 130:
#line 530 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.integer = 0;}
#line 2652 "parse.c" /* yacc.c:1646  */
    break;

  case 132:
#line 534 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, tab);}
#line 2658 "parse.c" /* yacc.c:1646  */
    break;

  case 133:
#line 538 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, printat);}
#line 2664 "parse.c" /* yacc.c:1646  */
    break;

  case 134:
#line 539 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, printd);}
#line 2670 "parse.c" /* yacc.c:1646  */
    break;

  case 135:
#line 540 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, prints);}
#line 2676 "parse.c" /* yacc.c:1646  */
    break;

  case 139:
#line 553 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = (yyvsp[0]).value.count;
				emitpushs(PS, (yyvsp[-2]).value.string);
				emitfunc(PS, prints);}
#line 2684 "parse.c" /* yacc.c:1646  */
    break;

  case 140:
#line 559 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = 1;}
#line 2690 "parse.c" /* yacc.c:1646  */
    break;

  case 141:
#line 560 "grammar.y" /* yacc.c:1646  */
    {(yyval).value.count = (yyvsp[-2]).value.count + 1;}
#line 2696 "parse.c" /* yacc.c:1646  */
    break;

  case 142:
#line 564 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, pushi, 0);}
#line 2702 "parse.c" /* yacc.c:1646  */
    break;

  case 143:
#line 565 "grammar.y" /* yacc.c:1646  */
    {emitfuncint(PS, pushi, 1);}
#line 2708 "parse.c" /* yacc.c:1646  */
    break;

  case 144:
#line 569 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, assignd);}
#line 2714 "parse.c" /* yacc.c:1646  */
    break;

  case 145:
#line 570 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, assigns);}
#line 2720 "parse.c" /* yacc.c:1646  */
    break;

  case 146:
#line 574 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, chs);}
#line 2726 "parse.c" /* yacc.c:1646  */
    break;

  case 147:
#line 575 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, not);}
#line 2732 "parse.c" /* yacc.c:1646  */
    break;

  case 149:
#line 577 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, addd);}
#line 2738 "parse.c" /* yacc.c:1646  */
    break;

  case 150:
#line 578 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, subd);}
#line 2744 "parse.c" /* yacc.c:1646  */
    break;

  case 151:
#line 579 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, muld);}
#line 2750 "parse.c" /* yacc.c:1646  */
    break;

  case 152:
#line 580 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, divd);}
#line 2756 "parse.c" /* yacc.c:1646  */
    break;

  case 153:
#line 581 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, modd);}
#line 2762 "parse.c" /* yacc.c:1646  */
    break;

  case 154:
#line 582 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, powerd);}
#line 2768 "parse.c" /* yacc.c:1646  */
    break;

  case 157:
#line 585 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, eqd);}
#line 2774 "parse.c" /* yacc.c:1646  */
    break;

  case 158:
#line 586 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, ned);}
#line 2780 "parse.c" /* yacc.c:1646  */
    break;

  case 159:
#line 587 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, ltd);}
#line 2786 "parse.c" /* yacc.c:1646  */
    break;

  case 160:
#line 588 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, gtd);}
#line 2792 "parse.c" /* yacc.c:1646  */
    break;

  case 161:
#line 589 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, led);}
#line 2798 "parse.c" /* yacc.c:1646  */
    break;

  case 162:
#line 590 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, ged);}
#line 2804 "parse.c" /* yacc.c:1646  */
    break;

  case 163:
#line 591 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, andd);}
#line 2810 "parse.c" /* yacc.c:1646  */
    break;

  case 164:
#line 592 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, ord);}
#line 2816 "parse.c" /* yacc.c:1646  */
    break;

  case 165:
#line 593 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, xord);}
#line 2822 "parse.c" /* yacc.c:1646  */
    break;

  case 166:
#line 594 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, andandd);}
#line 2828 "parse.c" /* yacc.c:1646  */
    break;

  case 167:
#line 595 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, orord);}
#line 2834 "parse.c" /* yacc.c:1646  */
    break;

  case 168:
#line 596 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, eqs);}
#line 2840 "parse.c" /* yacc.c:1646  */
    break;

  case 169:
#line 597 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, nes);}
#line 2846 "parse.c" /* yacc.c:1646  */
    break;

  case 172:
#line 600 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, evald);}
#line 2852 "parse.c" /* yacc.c:1646  */
    break;

  case 173:
#line 601 "grammar.y" /* yacc.c:1646  */
    {emitpushd(PS, (yyvsp[0]).value.real);}
#line 2858 "parse.c" /* yacc.c:1646  */
    break;

  case 175:
#line 608 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, intd);}
#line 2864 "parse.c" /* yacc.c:1646  */
    break;

  case 176:
#line 609 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, fixd);}
#line 2870 "parse.c" /* yacc.c:1646  */
    break;

  case 177:
#line 610 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, sgnd);}
#line 2876 "parse.c" /* yacc.c:1646  */
    break;

  case 178:
#line 611 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, sind);}
#line 2882 "parse.c" /* yacc.c:1646  */
    break;

  case 179:
#line 612 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, cosd);}
#line 2888 "parse.c" /* yacc.c:1646  */
    break;

  case 180:
#line 613 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, rndd);}
#line 2894 "parse.c" /* yacc.c:1646  */
    break;

  case 181:
#line 614 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, powd);}
#line 2900 "parse.c" /* yacc.c:1646  */
    break;

  case 182:
#line 615 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, logd);}
#line 2906 "parse.c" /* yacc.c:1646  */
    break;

  case 183:
#line 616 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, expd);}
#line 2912 "parse.c" /* yacc.c:1646  */
    break;

  case 184:
#line 617 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, tand);}
#line 2918 "parse.c" /* yacc.c:1646  */
    break;

  case 185:
#line 618 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, atnd);}
#line 2924 "parse.c" /* yacc.c:1646  */
    break;

  case 186:
#line 619 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, atn2d);}
#line 2930 "parse.c" /* yacc.c:1646  */
    break;

  case 187:
#line 620 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, absd);}
#line 2936 "parse.c" /* yacc.c:1646  */
    break;

  case 188:
#line 621 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, sqrd);}
#line 2942 "parse.c" /* yacc.c:1646  */
    break;

  case 189:
#line 622 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, sleepd);}
#line 2948 "parse.c" /* yacc.c:1646  */
    break;

  case 190:
#line 623 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, keyd);}
#line 2954 "parse.c" /* yacc.c:1646  */
    break;

  case 191:
#line 624 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, note);}
#line 2960 "parse.c" /* yacc.c:1646  */
    break;

  case 192:
#line 625 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, lend);}
#line 2966 "parse.c" /* yacc.c:1646  */
    break;

  case 193:
#line 626 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, vald);}
#line 2972 "parse.c" /* yacc.c:1646  */
    break;

  case 194:
#line 627 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, ascd);}
#line 2978 "parse.c" /* yacc.c:1646  */
    break;

  case 195:
#line 628 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, spriteload);}
#line 2984 "parse.c" /* yacc.c:1646  */
    break;

  case 196:
#line 632 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, mousexd);}
#line 2990 "parse.c" /* yacc.c:1646  */
    break;

  case 197:
#line 633 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, mouseyd);}
#line 2996 "parse.c" /* yacc.c:1646  */
    break;

  case 198:
#line 634 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, mousebd);}
#line 3002 "parse.c" /* yacc.c:1646  */
    break;

  case 199:
#line 635 "grammar.y" /* yacc.c:1646  */
    {emitpushd(PS, PS->bc->xsize);}
#line 3008 "parse.c" /* yacc.c:1646  */
    break;

  case 200:
#line 636 "grammar.y" /* yacc.c:1646  */
    {emitpushd(PS, PS->bc->ysize);}
#line 3014 "parse.c" /* yacc.c:1646  */
    break;

  case 201:
#line 637 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, ticksd);}
#line 3020 "parse.c" /* yacc.c:1646  */
    break;

  case 202:
#line 638 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, keycoded);}
#line 3026 "parse.c" /* yacc.c:1646  */
    break;

  case 203:
#line 641 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, inkey);}
#line 3032 "parse.c" /* yacc.c:1646  */
    break;

  case 205:
#line 646 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, adds);}
#line 3038 "parse.c" /* yacc.c:1646  */
    break;

  case 206:
#line 647 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, adds);}
#line 3044 "parse.c" /* yacc.c:1646  */
    break;

  case 207:
#line 651 "grammar.y" /* yacc.c:1646  */
    {emitpushs(PS, (yyvsp[0]).value.string);}
#line 3050 "parse.c" /* yacc.c:1646  */
    break;

  case 210:
#line 654 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, evals);}
#line 3056 "parse.c" /* yacc.c:1646  */
    break;

  case 211:
#line 655 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, tabstr);}
#line 3062 "parse.c" /* yacc.c:1646  */
    break;

  case 212:
#line 659 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, leftstr);}
#line 3068 "parse.c" /* yacc.c:1646  */
    break;

  case 213:
#line 660 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, rightstr);}
#line 3074 "parse.c" /* yacc.c:1646  */
    break;

  case 214:
#line 662 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, midstr);}
#line 3080 "parse.c" /* yacc.c:1646  */
    break;

  case 215:
#line 663 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, chrstr);}
#line 3086 "parse.c" /* yacc.c:1646  */
    break;

  case 216:
#line 664 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, performstrstr);}
#line 3092 "parse.c" /* yacc.c:1646  */
    break;

  case 217:
#line 665 "grammar.y" /* yacc.c:1646  */
    {emitfunc(PS, stringstr);}
#line 3098 "parse.c" /* yacc.c:1646  */
    break;


#line 3102 "parse.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (parm, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (parm, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, parm);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, parm);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (parm, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, parm);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, parm);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 668 "grammar.y" /* yacc.c:1906  */


void yyerror(ps *parm, char *s)
{
}

static void rankcheck(ps *ps, int var, int rank)
{
variable *v;
	v=ps->bc->vvars + var;
	if(!v->rank)
		v->rank = rank;
	else
		if(v->rank != rank)
			ps->rankfailure = v;
}


int findvar(ps *ps, char *name)
{
int i;
variable *v;
bc *bc = ps->bc;
	v=bc->vvars;
	for(i=0;i<bc->numvars;++i, ++v)
		if(!strcmp(v->name, name))
			return i;
	if(bc->numvars < MAXVARIABLES)
	{
		memset(v, 0, sizeof(*v));
		strcpy(v->name, name);
		return bc->numvars++;
	}
//WARNING
	return -1; // ran out
}

static inline char at(ps *ps)
{
	return *ps->yypntr;
}
static inline char get(ps *ps)
{
	return *ps->yypntr++;
}
static inline char back(ps *ps)
{
	return *--ps->yypntr;
}

static int iskeyword(ps *ps, char *want)
{
char *p1=ps->yypntr;
char *p2=want;

	while(*p2)
	{
		if(tolower(*p1)!=*p2)
			break;
		++p1;
		++p2;
	}
	if(!*p2)
	{
		ps->yypntr += p2-want;
		return 1;
	}
	return 0;
}

int yylex(YYSTYPE *ti, ps *parm)
{
char ch;
ps *ps=parm;

	while(get(ps)==' ');
	back(ps);
	ps->yylast = ps->yypntr;
	ti->at = ps->yypntr;
if(0){char *p=ps->yypntr, csave;
while(*p && *p++ != '\n');
csave = *--p;
*p = 0;
printf("here:%s\n", ps->yypntr);
*p = csave;
}

// alphabetized for readability -- but atn2 must be before atn, keycode < key
	if(iskeyword(ps, "abs")) return ABS;
	if(iskeyword(ps, "adsr")) return ADSR;
	if(iskeyword(ps, "and")) return ANDAND;
	if(iskeyword(ps, "arc")) return ARC;
	if(iskeyword(ps, "asc")) return ASC;
	if(iskeyword(ps, "atn2")) return ATN2; // order critical!
	if(iskeyword(ps, "atn")) return ATN;
	if(iskeyword(ps, "box")) return BOX;
	if(iskeyword(ps, "chr$")) return CHRSTR;
	if(iskeyword(ps, "circle")) return CIRCLE;
	if(iskeyword(ps, "clear")) return CLEAR;
	if(iskeyword(ps, "cls")) return CLS;
	if(iskeyword(ps, "color")) return COLOR;
	if(iskeyword(ps, "cos")) return COS;
	if(iskeyword(ps, "data")) return DATA;
	if(iskeyword(ps, "dim")) return DIM;
	if(iskeyword(ps, "disc")) return DISC;
	if(iskeyword(ps, "dur")) return DUR;
	if(iskeyword(ps, "else")) return ELSE;
	if(iskeyword(ps, "end")) return END;
	if(iskeyword(ps, "exp")) return EXP;
	if(iskeyword(ps, "fill")) return FILL;
	if(iskeyword(ps, "fix")) return FIX;
	if(iskeyword(ps, "fmul")) return FMUL;
	if(iskeyword(ps, "for")) return FOR;
	if(iskeyword(ps, "freq")) return FREQ;
	if(iskeyword(ps, "gosub")) return GOSUB;
	if(iskeyword(ps, "goto")) return GOTO;
	if(iskeyword(ps, "home")) return HOME;
	if(iskeyword(ps, "if")) return IF;
	if(iskeyword(ps, "inkey$")) return INKEYSTR;
	if(iskeyword(ps, "input")) return INPUT;
	if(iskeyword(ps, "int")) return INT;
	if(iskeyword(ps, "keycode")) return KEYCODE;
	if(iskeyword(ps, "key")) return KEY;
	if(iskeyword(ps, "left$")) return LEFTSTR;
	if(iskeyword(ps, "len")) return LEN;
	if(iskeyword(ps, "let")) return LET;
	if(iskeyword(ps, "line")) return LINE;
	if(iskeyword(ps, "log")) return LOG;
	if(iskeyword(ps, "mid$")) return MIDSTR;
	if(iskeyword(ps, "mod")) return MOD;
	if(iskeyword(ps, "mouseb")) return MOUSEB;
	if(iskeyword(ps, "mousex")) return MOUSEX;
	if(iskeyword(ps, "mousey")) return MOUSEY;
	if(iskeyword(ps, "move")) return MOVE;
	if(iskeyword(ps, "next")) return NEXT;
	if(iskeyword(ps, "note")) return NOTE;
	if(iskeyword(ps, "on")) return ON;
	if(iskeyword(ps, "or")) return OROR;
	if(iskeyword(ps, "pen")) return PEN;
	if(iskeyword(ps, "pow")) return POW;
	if(iskeyword(ps, "print")) return PRINT;
	if(iskeyword(ps, "random")) return RANDOM;
	if(iskeyword(ps, "read")) return READ;
	if(iskeyword(ps, "rect")) return RECT;

if(iskeyword(ps, "spriteload")) return SPRITELOAD;

	if(iskeyword(ps, "rem") || at(ps) == '\'')
	{
		while((ch=get(ps)) && ch!='\n');
		back(ps);
		return REM;
	}
	if(iskeyword(ps, "restore")) return RESTORE;
	if(iskeyword(ps, "return")) return RETURN;
	if(iskeyword(ps, "right$")) return RIGHTSTR;
	if(iskeyword(ps, "rnd")) return RND;
	if(iskeyword(ps, "rotate")) return ROTATE;
	if(iskeyword(ps, "round")) return ROUND;
	if(iskeyword(ps, "sgn")) return SGN;
	if(iskeyword(ps, "quiet")) return QUIET;
	if(iskeyword(ps, "shcubic")) return SHCUBIC;
	if(iskeyword(ps, "shcurve")) return SHCURVE;
	if(iskeyword(ps, "shdone")) return SHDONE;
	if(iskeyword(ps, "shend")) return SHEND;
	if(iskeyword(ps, "shinit")) return SHINIT;
	if(iskeyword(ps, "shline")) return SHLINE;
	if(iskeyword(ps, "sin")) return SIN;
	if(iskeyword(ps, "sleep")) return SLEEP;
	if(iskeyword(ps, "spot")) return SPOT;
	if(iskeyword(ps, "sqr")) return SQR;
	if(iskeyword(ps, "step")) return STEP;
	if(iskeyword(ps, "stop")) return STOP;
	if(iskeyword(ps, "str$")) return STRSTR;
	if(iskeyword(ps, "string$")) return STRINGSTR;
	if(iskeyword(ps, "tab")) return TAB;
	if(iskeyword(ps, "tan")) return TAN;
	if(iskeyword(ps, "test")) return TEST;
	if(iskeyword(ps, "then")) return THEN;
	if(iskeyword(ps, "ticks")) return TICKS;
	if(iskeyword(ps, "tone")) return TONE;
	if(iskeyword(ps, "to")) return TO;
	if(iskeyword(ps, "update")) return UPDATE;
	if(iskeyword(ps, "val")) return VAL;
	if(iskeyword(ps, "vol")) return VOL;
	if(iskeyword(ps, "wave")) return WAVE;
	if(iskeyword(ps, "wedge")) return WEDGE;
	if(iskeyword(ps, "wsaw")) return WSAW;
	if(iskeyword(ps, "wsin")) return WSIN;
	if(iskeyword(ps, "wsqr")) return WSQR;
	if(iskeyword(ps, "wtri")) return WTRI;
	if(iskeyword(ps, "xsize")) return XSIZE;
	if(iskeyword(ps, "ysize")) return YSIZE;

	ch=get(ps);
	switch(ch)
	{
	case '%': return MOD;
	case '?':
	case '@':
	case ',':
	case ';':
	case ':':
	case '(':
	case ')':
	case '+':
	case '/':
	case '-': return ch;
	case '*':
		ch=get(ps);
		if(ch=='*') return POWER;
		back(ps);
		return '*';
	case '&': return AND;
	case '|': return OR;
	case '^': return XOR;
	case '\n': return LF;
	case '=': return ch;
	case '~': return NOT;
	case 0: back(ps);
		return 0;
	case '>':
		ch=get(ps);
		if(ch=='>') return RR;
		if(ch=='=') return GE;
		back(ps);
		return GT;
	case '<':
		ch=get(ps);
		if(ch=='>') return NE;
		if(ch=='=') return LE;
		if(ch=='<') return LL;
		back(ps);
		return LT;
	}
	ch=back(ps);

	if(isdigit(ch) || ch=='.')
	{
		double intpart;
		double fracpart=0.0;
		int isreal = (ch=='.');

		intpart = 0.0;
		while(isdigit(ch=get(ps))) {intpart=intpart*10 + ch - '0';}
		if(ch=='.')
		{
			double power = 1;
			isreal = 1;
			while(isdigit(ch=get(ps)))
			{
				fracpart = fracpart*10 + ch - '0';
				power *= 10;
			}
			fracpart /= power;
		}
		back(ps);
		if(isreal)
		{
			ti->value.real = intpart + fracpart;
			return REAL;
		}
		ti->value.integer = intpart;
		return INTEGER;
	}
	if(isalpha(ch))
	{
		int t=0;
		char name[NAMELEN];
		for(;;)
		{
			ch=get(ps);
			if(!isalpha(ch) && !isdigit(ch))
				break;
			if(t<sizeof(name)-2)
				name[t++] = tolower(ch);
		}
		if(ch=='$')
		{
			name[t++] = ch;
			name[t] = 0;
			ti->value.integer = findvar(ps, name);
			return STRINGSYMBOL;
		}
		back(ps);
		name[t] = 0;
		ti->value.integer = findvar(ps, name);
		return NUMSYMBOL;
	}
	if(ch=='"')
	{
		get(ps);
		int t=0;
		for(;;)
		{
			ch=get(ps);
			if(!ch) {back(ps);return -1;} // nonterminated string
			if(ch=='\\')
			{
				ch=get(ps);
				if(!ch || ch=='\n')
					{back(ps);return -1;} // nonterminated
			} else if(ch=='"')
				break;
			if(t<sizeof(ti->value.string)-1)
				ti->value.string[t++] = ch;
		}
		ti->value.string[t] = 0;
		return STRING;
	}
	return -1;
}

char *myindex(char *p, char want)
{
	while(*p && *p!=want) ++p;
	if(*p) return p;
	else return 0;
}

void freeold(bc *bc)
{
int i;
int j,size;
bstring **s;
variable *v;

	for(i=0,v=bc->vvars;i<bc->numvars;++i,++v)
	{
		if(!v->pointer)
			continue;
		if(myindex(v->name, '$'))
		{
			if(v->rank)
			{
				s=v->pointer;
				size=v->dimensions[v->rank];
				for(j=0;j<size;++j)
					if(s[j])
						free_bstring(bc, s[j]);
			} else
				if(v->value.s)
					free_bstring(bc, v->value.s);
			v->value.s = 0;
		}
		free(v->pointer);
		v->pointer = 0;
	}

	bc->numvars = 0;
}

void pruninit(bc *bc)
{
	bc->time = 0.0;
	bc->soundtime = 0.0; // NEEDS A MUTEX!!!!
	memset(bc->sounds, 0, sizeof(bc->sounds));
	memset(bc->isounds, 0, sizeof(bc->isounds));
	bc->numvars = 0;
	bc->datanum=0;
	bc->datapull=0;
	bc->flags = 0;
	bc->gosubsp = 0;
	bc->numfors = 0;
	bc->gx=0;
	bc->gy=0;
	bc->gred=255;
	bc->ggreen=255;
	bc->gblue=255;
	bc->galpha=255;
	bc->pen = 1.0;
	bc->shape.numpoints = 0;
	bc->shape.numcontours = 0;
	freeold(bc);
	bc->starttime = SDL_GetTicks();
	memset(bc->vvars, 0, sizeof(bc->vvars));
	reset_waitbase(bc);
}

void dump_data_init(ps *ps)
{
	emitfuncint(ps, rcall, 0); // call to load up all the data...
}

void dump_data_finish(ps *ps)
{
bc *bc=ps->bc;
int i;
	bc->base[1].i = ps->nextstep - bc->base; // fixup initial rcall
	for(i=0;i<bc->datanum;++i)
	{
		emitfunc(ps, datad);
		emitdouble(ps, bc->data[i]);
	}

	emitfunc(ps, ret);
}

char *recover_line(ps *ps, char *put, int len, char *p)
{
int n;
	while(p>ps->yystart && p[-1] != '\n') --p;
	n=0;
	for(n=0;p[n] && p[n]!='\n' && n<len-1;++n)
		put[n] = p[n];
	put[n] = 0;
	return p;
}

ps *newps(bc *bc, char *take)
{
struct parse_state *ps;
int i;
char linecopy[1024];
	ps = malloc(sizeof(struct parse_state));
	if(!ps)
	{
		tprintf(bc, "Out of memory.\n");
		return 0;
	}
	memset(ps, 0, sizeof(*ps));
	ps->bc = bc;
	ps->nextstep = ps->steps;
	bc->base = ps->steps;
	ps->yypntr = take;
	ps->yystart = take;
	dump_data_init(ps);

	bc->numlines=0;


	ps->res=yyparse(ps);


	if(ps->res)
	{
		int n;
		char *p;
		tprintf(bc, "Parse error\n");

		p=recover_line(ps, linecopy, sizeof(linecopy), ps->yylast);
		tprintf(bc, "%s\n", linecopy);

		n=ps->yylast - p;
		if(n>sizeof(linecopy)-1)
			n=sizeof(linecopy)-1;
		memset(linecopy, ' ', n);
		linecopy[n]=0;
		tprintf(bc, "%s^\n", linecopy);
	} else if(ps->errormsg[0])
	{
		ps->res = -1;
		recover_line(ps, linecopy, sizeof(linecopy), ps->errorpos);
		tprintf(bc, "%s\n", linecopy);
		tprintf(bc, "%s\n", ps->errormsg);
	} else
	{
		emitfunc(ps, performend);
		dump_data_finish(ps);
		ps->res = fixuplinerefs(ps);
		if(!ps->res && ps->rankfailure)
		{
			tprintf(bc, "Array variable '%s' changes "
					"rank within program\n",
					ps->rankfailure->name);
			ps->res = -1;
		}
		for(i=0;i<bc->numvars;++i)	// clear out all the ranks
			bc->vvars[i].rank = 0;
	}
	return ps;
}

void parseline(bc *bc, char *line)
{
ps *ps;
	ps = newps(bc, line);
	if(ps)
	{
		if(!ps->res)
		{
			bc->flags &= ~BF_RUNNING;
//			disassemble(ps->steps, ps->nextstep - ps->steps);
			reset_waitbase(bc);
			vmachine(bc, ps->steps, bc->vstack);
		}
		free(ps);
	}
}

// I think it's an insertion sort, works great if they're already sorted
void sortlinerefs(ps *ps)
{
lineref *p1, *p2, *s, *e, t;
	s=ps->linerefs;
	e = s + ps->numlinerefs;
	for(p1 = s+1;p1<e;++p1)
	{
		t=*p1;
		p2=p1;
		while(p2>s && p2[-1].at > t.at)
		{
			*p2 = p2[-1];
			--p2;
		}
		*p2=t;
	}
}

void renumber(bc *bc, int delta, int start)
{
ps *ps;
int len;
char *put;
char *take, *end;
lineref *lr, *s;
int n;
int v;
char temp[32];
linemap *lm;

	ps = newps(bc, bc->program);
	if(!ps || ps->res)
		return;
// we know all the line references are good, the parser has to fix them
	sortlinerefs(ps); // just to be safe...we work 'em from back to front

// pass 1, replace all the references with the new values
	put = bc->program + sizeof(bc->program);
	*--put = 0;
	end = bc->program + strlen(bc->program);
	s = ps->linerefs;
	lr = ps->linerefs + ps->numlinerefs;
	while(lr > s)
	{
		--lr;
		take = lr->at;
		v=0;
		while(isdigit(*take))
			v=v*10 + *take++ - '0';
		len = end - take;
		put -= len;
		memmove(put, take, len);
		end = lr->at;
		lm=findlinemap(ps, v); // we know we find it...
		if(!lm)
		{
			tprintf(bc, "The world is insane...giving up.\n");
			free(ps);
			return;
		}
		n = lm - bc->lm;
		len = sprintf(temp, "%d", start+n*delta);
		put -= len;
		memcpy(put, temp, len);
	}
	len = end - bc->program;
	put -= len;
	memmove(put, bc->program, len);

// pass 2, fix up the line numbers themselves
	take = put;
	n = start;
	put = bc->program;
	while(*take)
	{
		while(isdigit(*take)) ++take;
		put += sprintf(put, "%d", n);
		n += delta;
		while((*put++ = *take) && *take++ != '\n');
	}
	*put = 0;
	free(ps);
}

void parse(bc *bc, int runit)
{
ps *ps;

	pruninit(bc);
	ps = newps(bc, bc->program);

	if(ps)
	{
		if(!ps->res)
		{
			if(!runit)
			{
				tprintf(bc, "Program parsed correctly\n");
				disassemble(bc, ps->steps,
					ps->nextstep - ps->steps);
			} else
			{
				bc->flags |= BF_RUNNING;
				vmachine(bc, ps->steps, bc->vstack);
			}
		}
		free(ps);
	}
}



#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20091027

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
#ifdef YYPARSE_PARAM_TYPE
#define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
#else
#define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
#endif
#else
#define YYPARSE_DECL() yyparse(void)
#endif /* YYPARSE_PARAM */

extern int YYPARSE_DECL();

static int yygrowstack(void);
#define YYPREFIX "yy"
#line 2 "yacc.yacc"
#include "stdio.h"
#include "eight.h"
#include "eight.c"
#include "print.c"
#include "basic_commands.c"
#include "symbols.c"
#include "strings.c"
#include "memory.c"

machine *m;

#line 17 "yacc.yacc"
typedef union {closure * clos;
  char * strin;
  int fixi;
} YYSTYPE;
#line 51 "y.tab.c"
#define END 257
#define OPEN 258
#define CLOSE 259
#define STRIN 260
#define NUMBE 261
#define SYMBO 262
#define QUOT 263
#define ASTERI 264
#define ATPEN 265
#define ELIPSI 266
#define COMM 267
#define YYERRCODE 256
static const short yylhs[] = {                           -1,
    0,    0,    0,    0,    1,    1,    1,    1,    1,    1,
    2,    2,    2,    2,    3,    3,    3,    3,    3,    3,
    3,    3,
};
static const short yylen[] = {                            2,
    0,    1,    1,    1,    3,    2,    2,    2,    2,    2,
    2,    2,    1,    1,    2,    2,    2,    2,    1,    1,
    1,    1,
};
static const short yydefred[] = {                         0,
    4,    0,   22,   19,   20,    0,    0,    0,   21,    0,
    0,    2,    3,    6,    0,    0,    0,    7,   15,    8,
   16,    9,   17,   10,   18,   12,    5,   11,
};
static const short yydgoto[] = {                         11,
   15,   16,   17,
};
static const short yysindex[] = {                      -257,
    0, -231,    0,    0,    0, -221, -221, -221,    0, -221,
    0,    0,    0,    0, -221, -246, -221,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};
static const short yyrindex[] = {                         2,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0, -244,    0, -239,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
};
static const short yygindex[] = {                         0,
   11,   -3,   16,
};
#define YYTABLESIZE 46
static const short yytable[] = {                          1,
    2,    1,    3,    4,    5,    6,    7,    8,    9,   10,
   12,   26,   27,   28,   13,   13,   18,   20,   22,   14,
   24,   19,   21,   23,    0,   25,    2,   14,    3,    4,
    5,    6,    7,    8,    9,   10,    2,    0,    3,    4,
    5,    6,    7,    8,    9,   10,
};
static const short yycheck[] = {                        257,
  258,    0,  260,  261,  262,  263,  264,  265,  266,  267,
    0,   15,  259,   17,  259,    0,    6,    7,    8,  259,
   10,    6,    7,    8,   -1,   10,  258,  259,  260,  261,
  262,  263,  264,  265,  266,  267,  258,   -1,  260,  261,
  262,  263,  264,  265,  266,  267,
};
#define YYFINAL 11
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 267
#if YYDEBUG
static const char *yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"END","OPEN","CLOSE","STRIN",
"NUMBE","SYMBO","QUOT","ASTERI","ATPEN","ELIPSI","COMM",
};
static const char *yyrule[] = {
"$accept : root",
"root :",
"root : list",
"root : atom",
"root : END",
"list : OPEN cons CLOSE",
"list : OPEN CLOSE",
"list : QUOT list",
"list : ASTERI list",
"list : ATPEN list",
"list : COMM list",
"cons : atom cons",
"cons : list cons",
"cons : list",
"cons : atom",
"atom : QUOT atom",
"atom : ASTERI atom",
"atom : ATPEN atom",
"atom : COMM atom",
"atom : NUMBE",
"atom : SYMBO",
"atom : ELIPSI",
"atom : STRIN",

};
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

int      yydebug;
int      yynerrs;
int      yyerrflag;
int      yychar;
short   *yyssp;
YYSTYPE *yyvsp;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* variables for the parser stack */
static short   *yyss;
static short   *yysslim;
static YYSTYPE *yyvs;
static unsigned yystacksize;
#line 81 "yacc.yacc"
void set_input_source(FILE *s);

int main( int   argc,
          char *argv[] )
{
     initialize_symbol_table();
     m = init_8VM();
     set_input_source(fopen( "test.8", "r" ));
      int rep = 0;
      while (rep != -1){
           rep = yyparse();
      }
      fclose(yyin);
      int i;
      int interact = 0;

      if(argc == 1){
	yyrestart(stdin);
	  interact = 1;
	  printf("-> ");
	  rep = 0;
	  while (rep != -1){
	    rep = yyparse();
	    if ((interact!=0) && (m->accum != NULL)){
	      print_closure(m->accum);
	      printf("\n");
	      printf("-> ");
	    }
	  }
      } 



      for (i=1;i<argc; i++){
	if (!strcmp(argv[i], "-i")){
	  yyrestart(stdin);
	  interact = 1;
	  printf("-> ");
	} else {
	  yyrestart(fopen(argv[i], "r"));
	  interact = 0;
	}
	  rep = 0;
	  while (rep != -1){
	    rep = yyparse();
	    if ((interact!=0) && (m->accum != NULL)){
	      print_closure(m->accum);
	      printf("\n");
	      printf("-> ");
	    }
	  }
	  fclose(yyin); 
	}
      return 0;
}

yyerror(s) char *s; {
     fprintf( stderr, "%s\n", s );
     return 1;
}
#line 248 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(void)
{
    int i;
    unsigned newsize;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = yyssp - yyss;
    newss = (yyss != 0)
          ? (short *)realloc(yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    yyss  = newss;
    yyssp = newss + i;
    newvs = (yyvs != 0)
          ? (YYSTYPE *)realloc(yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    yystate = 0;
    *yyssp = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yyssp = yytable[yyn];
        *++yyvsp = yylval;
        yychar = YYEMPTY;
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

    yyerror("syntax error");

    goto yyerrlab;

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
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yyssp = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
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
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yyvsp[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 2:
#line 40 "yacc.yacc"
	{ print_closure((closure*)yyvsp[0].clos);	
                           printf("\n");
			   m = eval(yyvsp[0].clos, m); return 1;}
break;
case 3:
#line 43 "yacc.yacc"
	{ /*print_closure((closure*)$1);*/
  m = eval(yyvsp[0].clos, m); return 1;}
break;
case 4:
#line 45 "yacc.yacc"
	{ return -1; }
break;
case 5:
#line 48 "yacc.yacc"
	{ yyval.clos = yyvsp[-1].clos; }
break;
case 6:
#line 49 "yacc.yacc"
	{ yyval.clos = nil();}
break;
case 7:
#line 50 "yacc.yacc"
	{ yyval.clos = cons(symbol(QUOTE), 
					 cons(yyvsp[0].clos, nil())); }
break;
case 8:
#line 52 "yacc.yacc"
	{ yyval.clos = cons(symbol(ASTERIX), 
					 cons(yyvsp[0].clos, nil())); }
break;
case 9:
#line 54 "yacc.yacc"
	{ yyval.clos = cons(symbol(ATPEND), 
					 cons(yyvsp[0].clos, nil())); }
break;
case 10:
#line 56 "yacc.yacc"
	{ yyval.clos = cons(symbol(COMMA), 
					 cons(yyvsp[0].clos, nil())); }
break;
case 11:
#line 60 "yacc.yacc"
	{ yyval.clos = cons(yyvsp[-1].clos, yyvsp[0].clos);}
break;
case 12:
#line 61 "yacc.yacc"
	{ yyval.clos = cons(yyvsp[-1].clos, yyvsp[0].clos);}
break;
case 13:
#line 62 "yacc.yacc"
	{ yyval.clos = cons(yyvsp[0].clos, nil()); }
break;
case 14:
#line 63 "yacc.yacc"
	{ yyval.clos = cons(yyvsp[0].clos, nil()); }
break;
case 15:
#line 67 "yacc.yacc"
	{ yyval.clos =  quote(yyvsp[0].clos); }
break;
case 16:
#line 68 "yacc.yacc"
	{ yyval.clos =  cons(symbol(ASTERIX), 
					  cons(yyvsp[0].clos, nil())); }
break;
case 17:
#line 70 "yacc.yacc"
	{ yyval.clos =  cons(symbol(ATPEND), 
					  cons(yyvsp[0].clos, nil())); }
break;
case 18:
#line 72 "yacc.yacc"
	{ yyval.clos =  cons(symbol(COMMA), 
					  cons(yyvsp[0].clos, nil())); }
break;
case 19:
#line 74 "yacc.yacc"
	{ yyval.clos = number(yyvsp[0].fixi); }
break;
case 20:
#line 75 "yacc.yacc"
	{ yyval.clos = symbol(string_to_symbol_id(yyvsp[0].strin)); }
break;
case 21:
#line 76 "yacc.yacc"
	{ yyval.clos = symbol(ELIPSIS); }
break;
case 22:
#line 77 "yacc.yacc"
	{ yyval.clos = string(yyvsp[0].strin); }
break;
#line 527 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
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
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
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
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = (short) yystate;
    *++yyvsp = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    return (1);

yyaccept:
    return (0);
}

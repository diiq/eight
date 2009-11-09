%{
#include "stdio.h"
#include "y.tab.c"
extern YYSTYPE yylval;


%}

%x str

%%
[\"]   {BEGIN(str);}
<str>[\"]  { if (yytext[yyleng-1] == '\\'){
                 yymore();
             } else {yytext[yyleng-1] = 0;
                     BEGIN(INITIAL); 
                     return STRIN;}}
<str>([^\"](\\\")?)*  {yylval = (int)yytext;}


\;.*$         {}
\,           { return COMM; }
\*           { return ASTERI; }
\'           { return QUOT; }
\(           { return OPEN;}
\)           { return CLOSE;}
\.\.\.       { return ELIPSI;}
[0-9]+       {yylval = atoi(yytext); return NUMBE;}
[^\"\*\(\)\t\r\n\;\.\," '"]+   { yylval = (int)yytext; return SYMBO;}
<<EOF>>      { return END; }
[\n\t ]*  {}   

%%

 void set_input_source(FILE *s)
 {
   yyin = s;
   BEGIN(INITIAL);
 }

int yywrap() { return 1; }

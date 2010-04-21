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
typedef union {closure * clos;
  char * strin;
  int fixi;
} YYSTYPE;
extern YYSTYPE yylval;

typedef union {
	char *str;
	ConfigDefaults	*ConfLine;
	HtConfiguration	*ConfLines;
} YYSTYPE;
#define	NUM	258
#define	T_DELIMITER	259
#define	T_NEWLINE	260
#define	T_RIGHT_BR	261
#define	T_LEFT_BR	262
#define	T_SLASH	263
#define	T_STRING	264
#define	T_KEYWORD	265
#define	T_NUMBER	266


extern YYSTYPE yylval;

typedef union {
	char *str;
	ConfigDefaults	*ConfLine;
	Configuration	*ConfLines;
} YYSTYPE;
#define	NUM	257
#define	T_DELIMITER	258
#define	T_NEWLINE	259
#define	T_RIGHT_BR	260
#define	T_LEFT_BR	261
#define	T_SLASH	262
#define	T_STRING	263
#define	T_KEYWORD	264
#define	T_NUMBER	265


extern YYSTYPE yylval;

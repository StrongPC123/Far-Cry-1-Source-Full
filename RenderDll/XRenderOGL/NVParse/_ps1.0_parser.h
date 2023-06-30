typedef union 
{
	int ival;
	float fval;
	
  string * sval;
	constdef * cdef;
  std::vector<constdef> * consts;
  std::vector<string> * line;
  std::list<std::vector<string> > * lines;
} YYSTYPE;
#define	HEADER	257
#define	NEWLINE	258
#define	NUMBER	259
#define	REG	260
#define	DEF	261
#define	ADDROP	262
#define	BLENDOP	263


extern YYSTYPE ps10_lval;

typedef union {
  int ival;
  unsigned int lval;
  float fval;
  char mask[4];
  char *comment;
  VS10Reg reg;
  VS10InstPtr inst;
  VS10InstListPtr instList;
} YYSTYPE;
#define	VERTEX_SHADER	257
#define	ADD_INSTR	258
#define	DP3_INSTR	259
#define	DP4_INSTR	260
#define	DST_INSTR	261
#define	EXP_INSTR	262
#define	EXPP_INSTR	263
#define	FRC_INSTR	264
#define	LIT_INSTR	265
#define	LOG_INSTR	266
#define	LOGP_INSTR	267
#define	M3X2_INSTR	268
#define	M3X3_INSTR	269
#define	M3X4_INSTR	270
#define	M4X3_INSTR	271
#define	M4X4_INSTR	272
#define	MAD_INSTR	273
#define	MAX_INSTR	274
#define	MIN_INSTR	275
#define	MOV_INSTR	276
#define	MUL_INSTR	277
#define	NOP_INSTR	278
#define	RCP_INSTR	279
#define	RSQ_INSTR	280
#define	SGE_INSTR	281
#define	SLT_INSTR	282
#define	SUB_INSTR	283
#define	ILLEGAL	284
#define	UNKNOWN_STRING	285
#define	INTVAL	286
#define	REGISTER	287
#define	XYZW_MODIFIER	288
#define	COMMENT	289


extern YYSTYPE vs10_lval;

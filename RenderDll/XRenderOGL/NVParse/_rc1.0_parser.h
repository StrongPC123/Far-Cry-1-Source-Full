typedef union {
  int ival;
  float fval;
  RegisterEnum registerEnum;
  BiasScaleEnum biasScaleEnum;
  MappedRegisterStruct mappedRegisterStruct;
  ConstColorStruct constColorStruct;
  GeneralPortionStruct generalPortionStruct;
  GeneralFunctionStruct generalFunctionStruct;
  OpStruct opStruct;
  GeneralCombinerStruct generalCombinerStruct;
  GeneralCombinersStruct generalCombinersStruct;
  FinalProductStruct finalProductStruct;
  FinalRgbFunctionStruct finalRgbFunctionStruct;
  FinalAlphaFunctionStruct finalAlphaFunctionStruct;
  FinalCombinerStruct finalCombinerStruct;
  CombinersStruct combinersStruct;
} YYSTYPE;
#define	regVariable	257
#define	constVariable	258
#define	color_sum	259
#define	final_product	260
#define	expandString	261
#define	halfBiasString	262
#define	unsignedString	263
#define	unsignedInvertString	264
#define	muxString	265
#define	sumString	266
#define	rgb_portion	267
#define	alpha_portion	268
#define	openParen	269
#define	closeParen	270
#define	openBracket	271
#define	closeBracket	272
#define	semicolon	273
#define	comma	274
#define	dot	275
#define	times	276
#define	minus	277
#define	equals	278
#define	plus	279
#define	bias_by_negative_one_half_scale_by_two	280
#define	bias_by_negative_one_half	281
#define	scale_by_one_half	282
#define	scale_by_two	283
#define	scale_by_four	284
#define	clamp_color_sum	285
#define	lerp	286
#define	fragment_rgb	287
#define	fragment_alpha	288
#define	floatValue	289


extern YYSTYPE rc10_lval;

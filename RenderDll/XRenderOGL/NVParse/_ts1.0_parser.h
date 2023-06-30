typedef union {
  float fval;
  InstPtr inst;
  InstListPtr instList;
  MappedVariablePtr variable;
} YYSTYPE;
#define	floatValue	257
#define	gequal	258
#define	less	259
#define	texVariable	260
#define	expandString	261
#define	openParen	262
#define	closeParen	263
#define	semicolon	264
#define	comma	265
#define	nop	266
#define	texture_1d	267
#define	texture_2d	268
#define	texture_rectangle	269
#define	texture_3d	270
#define	texture_cube_map	271
#define	cull_fragment	272
#define	pass_through	273
#define	offset_2d_scale	274
#define	offset_2d	275
#define	offset_rectangle_scale	276
#define	offset_rectangle	277
#define	offset_projective_2d_scale	278
#define	offset_projective_2d	279
#define	offset_projective_rectangle_scale	280
#define	offset_projective_rectangle	281
#define	dependent_ar	282
#define	dependent_gb	283
#define	dot_product_2d_1of2	284
#define	dot_product_2d_2of2	285
#define	dot_product_rectangle_1of2	286
#define	dot_product_rectangle_2of2	287
#define	dot_product_depth_replace_1of2	288
#define	dot_product_depth_replace_2of2	289
#define	dot_product_3d_1of3	290
#define	dot_product_3d_2of3	291
#define	dot_product_3d_3of3	292
#define	dot_product_cube_map_1of3	293
#define	dot_product_cube_map_2of3	294
#define	dot_product_cube_map_3of3	295
#define	dot_product_reflect_cube_map_eye_from_qs_1of3	296
#define	dot_product_reflect_cube_map_eye_from_qs_2of3	297
#define	dot_product_reflect_cube_map_eye_from_qs_3of3	298
#define	dot_product_reflect_cube_map_const_eye_1of3	299
#define	dot_product_reflect_cube_map_const_eye_2of3	300
#define	dot_product_reflect_cube_map_const_eye_3of3	301
#define	dot_product_cube_map_and_reflect_cube_map_eye_from_qs_1of3	302
#define	dot_product_cube_map_and_reflect_cube_map_eye_from_qs_2of3	303
#define	dot_product_cube_map_and_reflect_cube_map_eye_from_qs_3of3	304
#define	dot_product_cube_map_and_reflect_cube_map_const_eye_1of3	305
#define	dot_product_cube_map_and_reflect_cube_map_const_eye_2of3	306
#define	dot_product_cube_map_and_reflect_cube_map_const_eye_3of3	307


extern YYSTYPE ts10_lval;

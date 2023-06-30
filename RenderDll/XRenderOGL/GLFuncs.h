/*=============================================================================
	GlFuncs.h: OpenGL function-declaration macros, to enable dynamic linking
	to OpenGL32.dll or a minidriver such as 3dfxvgl.dll.

  Copyright (c) 2001-2002 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honich Andrey
=============================================================================*/

/*-----------------------------------------------------------------------------
	Standard OpenGL functions.
-----------------------------------------------------------------------------*/

GL_EXT(_GL)

// OpenGL.
GL_PROC(_GL,void,glAccum,(GLenum,GLfloat))
GL_PROC(_GL,void,glAlphaFunc,(GLenum,GLclampf))
GL_PROC(_GL,GLboolean,glAreTexturesResident,(GLsizei,const GLuint*,GLboolean*))
GL_PROC(_GL,void,glArrayElement,(GLint))
GL_PROC(_GL,void,glBegin,(GLenum))
GL_PROC(_GL,void,glBindTexture,(GLenum,GLuint))
GL_PROC(_GL,void,glBitmap,(GLsizei,GLsizei,GLfloat,GLfloat,GLfloat,GLfloat,const GLubyte*))
GL_PROC(_GL,void,glBlendFunc,(GLenum,GLenum))
GL_PROC(_GL,void,glCallList,(GLuint))
GL_PROC(_GL,void,glCallLists,(GLsizei,GLenum,const GLvoid*))
GL_PROC(_GL,void,glClear,(GLbitfield))
GL_PROC(_GL,void,glClearAccum,(GLfloat,GLfloat,GLfloat,GLfloat))
GL_PROC(_GL,void,glClearColor,(GLclampf,GLclampf,GLclampf,GLclampf))
GL_PROC(_GL,void,glClearDepth,(GLclampd))
GL_PROC(_GL,void,glClearIndex,(GLfloat))
GL_PROC(_GL,void,glClearStencil,(GLint))
GL_PROC(_GL,void,glClipPlane,(GLenum,const GLdouble*))
GL_PROC(_GL,void,glColor3b,(GLbyte,GLbyte,GLbyte))
GL_PROC(_GL,void,glColor3bv,(const GLbyte*))
GL_PROC(_GL,void,glColor3d,(GLdouble,GLdouble,GLdouble))
GL_PROC(_GL,void,glColor3dv,(const GLdouble*))
GL_PROC(_GL,void,glColor3f,(GLfloat,GLfloat,GLfloat))
GL_PROC(_GL,void,glColor3fv,(const GLfloat*))
GL_PROC(_GL,void,glColor3i,(GLint,GLint,GLint))
GL_PROC(_GL,void,glColor3iv,(const GLint*))
GL_PROC(_GL,void,glColor3s,(GLshort,GLshort,GLshort))
GL_PROC(_GL,void,glColor3sv,(const GLshort*))
GL_PROC(_GL,void,glColor3ub,(GLubyte,GLubyte,GLubyte))
GL_PROC(_GL,void,glColor3ubv,(const GLubyte*))
GL_PROC(_GL,void,glColor3ui,(GLuint,GLuint,GLuint))
GL_PROC(_GL,void,glColor3uiv,(const GLuint*))
GL_PROC(_GL,void,glColor3us,(GLushort,GLushort,GLushort))
GL_PROC(_GL,void,glColor3usv,(const GLushort*))
GL_PROC(_GL,void,glColor4b,(GLbyte,GLbyte,GLbyte,GLbyte))
GL_PROC(_GL,void,glColor4bv,(const GLbyte*))
GL_PROC(_GL,void,glColor4d,(GLdouble,GLdouble,GLdouble,GLdouble))
GL_PROC(_GL,void,glColor4dv,(const GLdouble*))
GL_PROC(_GL,void,glColor4f,(GLfloat,GLfloat,GLfloat,GLfloat))
GL_PROC(_GL,void,glColor4fv,(const GLfloat*))
GL_PROC(_GL,void,glColor4i,(GLint,GLint,GLint,GLint))
GL_PROC(_GL,void,glColor4iv,(const GLint*))
GL_PROC(_GL,void,glColor4s,(GLshort,GLshort,GLshort,GLshort))
GL_PROC(_GL,void,glColor4sv,(const GLshort*))
GL_PROC(_GL,void,glColor4ub,(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha))
GL_PROC(_GL,void,glColor4ubv,(const GLubyte *v))
GL_PROC(_GL,void,glColor4ui,(GLuint red, GLuint green, GLuint blue, GLuint alpha))
GL_PROC(_GL,void,glColor4uiv,(const GLuint *v))
GL_PROC(_GL,void,glColor4us,(GLushort red, GLushort green, GLushort blue, GLushort alpha))
GL_PROC(_GL,void,glColor4usv,(const GLushort *v))
GL_PROC(_GL,void,glColorMask,(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha))
GL_PROC(_GL,void,glColorMaterial,(GLenum face, GLenum mode))
GL_PROC(_GL,void,glColorPointer,(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer))
GL_PROC(_GL,void,glCopyPixels,(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type))
GL_PROC(_GL,void,glCopyTexImage1D,(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border))
GL_PROC(_GL,void,glCopyTexImage2D,(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border))
GL_PROC(_GL,void,glCopyTexSubImage1D,(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width))
GL_PROC(_GL,void,glCopyTexSubImage2D,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height))
GL_PROC(_GL,void,glCullFace,(GLenum mode))
GL_PROC(_GL,void,glDeleteLists,(GLuint l, GLsizei range))
GL_PROC(_GL,void,glDeleteTextures,(GLsizei n, const GLuint *textures))
GL_PROC(_GL,void,glDepthFunc,(GLenum func))
GL_PROC(_GL,void,glDepthMask,(GLboolean flag))
GL_PROC(_GL,void,glDepthRange,(GLclampd zNear, GLclampd zFar))
GL_PROC(_GL,void,glDisable,(GLenum cap))
GL_PROC(_GL,void,glDisableClientState,(GLenum array))
GL_PROC(_GL,void,glDrawArrays,(GLenum mode, GLint first, GLsizei count))
GL_PROC(_GL,void,glDrawBuffer,(GLenum mode))
GL_PROC(_GL,void,glDrawElements,(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices))
GL_PROC(_GL,void,glDrawPixels,(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels))
GL_PROC(_GL,void,glEdgeFlag,(GLboolean flag))
GL_PROC(_GL,void,glEdgeFlagPointer,(GLsizei stride, const GLvoid *pointer))
GL_PROC(_GL,void,glEdgeFlagv,(const GLboolean *flag))
GL_PROC(_GL,void,glEnable,(GLenum cap))
GL_PROC(_GL,void,glEnableClientState,(GLenum array))
GL_PROC(_GL,void,glEnd,(void))
GL_PROC(_GL,void,glEndList,(void))
GL_PROC(_GL,void,glEvalCoord1d,(GLdouble u))
GL_PROC(_GL,void,glEvalCoord1dv,(const GLdouble *u))
GL_PROC(_GL,void,glEvalCoord1f,(GLfloat u))
GL_PROC(_GL,void,glEvalCoord1fv,(const GLfloat *u))
GL_PROC(_GL,void,glEvalCoord2d,(GLdouble u, GLdouble v))
GL_PROC(_GL,void,glEvalCoord2dv,(const GLdouble *u))
GL_PROC(_GL,void,glEvalCoord2f,(GLfloat u, GLfloat v))
GL_PROC(_GL,void,glEvalCoord2fv,(const GLfloat *u))
GL_PROC(_GL,void,glEvalMesh1,(GLenum mode, GLint i1, GLint i2))
GL_PROC(_GL,void,glEvalMesh2,(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2))
GL_PROC(_GL,void,glEvalPoint1,(GLint i))
GL_PROC(_GL,void,glEvalPoint2,(GLint i, GLint j))
GL_PROC(_GL,void,glFeedbackBuffer,(GLsizei size, GLenum type, GLfloat *buffer))
GL_PROC(_GL,void,glFinish,(void))
GL_PROC(_GL,void,glFlush,(void))
GL_PROC(_GL,void,glFogf,(GLenum pname, GLfloat param))
GL_PROC(_GL,void,glFogfv,(GLenum pname, const GLfloat *params))
GL_PROC(_GL,void,glFogi,(GLenum pname, GLint param))
GL_PROC(_GL,void,glFogiv,(GLenum pname, const GLint *params))
GL_PROC(_GL,void,glFrontFace,(GLenum mode))
GL_PROC(_GL,void,glFrustum,(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar))
GL_PROC(_GL,GLuint,glGenLists,(GLsizei range))
GL_PROC(_GL,void,glGenTextures,(GLsizei n, GLuint *textures))
GL_PROC(_GL,void,glGetBooleanv,(GLenum pname, GLboolean *params))
GL_PROC(_GL,void,glGetClipPlane,(GLenum plane, GLdouble *equation))
GL_PROC(_GL,void,glGetDoublev,(GLenum pname, GLdouble *params))
GL_PROC(_GL,GLenum,glGetError,(void))
GL_PROC(_GL,void,glGetFloatv,(GLenum pname, GLfloat *params))
GL_PROC(_GL,void,glGetIntegerv,(GLenum pname, GLint *params))
GL_PROC(_GL,void,glGetLightfv,(GLenum light, GLenum pname, GLfloat *params))
GL_PROC(_GL,void,glGetLightiv,(GLenum light, GLenum pname, GLint *params))
GL_PROC(_GL,void,glGetMapdv,(GLenum target, GLenum query, GLdouble *v))
GL_PROC(_GL,void,glGetMapfv,(GLenum target, GLenum query, GLfloat *v))
GL_PROC(_GL,void,glGetMapiv,(GLenum target, GLenum query, GLint *v))
GL_PROC(_GL,void,glGetMaterialfv,(GLenum face, GLenum pname, GLfloat *params))
GL_PROC(_GL,void,glGetMaterialiv,(GLenum face, GLenum pname, GLint *params))
GL_PROC(_GL,void,glGetPixelMapfv,(GLenum map, GLfloat *values))
GL_PROC(_GL,void,glGetPixelMapuiv,(GLenum map, GLuint *values))
GL_PROC(_GL,void,glGetPixelMapusv,(GLenum map, GLushort *values))
GL_PROC(_GL,void,glGetPointerv,(GLenum pname, GLvoid* *params))
GL_PROC(_GL,void,glGetPolygonStipple,(GLubyte *mask))
GL_PROC(_GL,const GLubyte *,glGetString,(GLenum name))
GL_PROC(_GL,void,glGetTexEnvfv,(GLenum target, GLenum pname, GLfloat *params))
GL_PROC(_GL,void,glGetTexEnviv,(GLenum target, GLenum pname, GLint *params))
GL_PROC(_GL,void,glGetTexGendv,(GLenum coord, GLenum pname, GLdouble *params))
GL_PROC(_GL,void,glGetTexGenfv,(GLenum coord, GLenum pname, GLfloat *params))
GL_PROC(_GL,void,glGetTexGeniv,(GLenum coord, GLenum pname, GLint *params))
GL_PROC(_GL,void,glGetTexImage,(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels))
GL_PROC(_GL,void,glGetTexLevelParameterfv,(GLenum target, GLint level, GLenum pname, GLfloat *params))
GL_PROC(_GL,void,glGetTexLevelParameteriv,(GLenum target, GLint level, GLenum pname, GLint *params))
GL_PROC(_GL,void,glGetTexParameterfv,(GLenum target, GLenum pname, GLfloat *params))
GL_PROC(_GL,void,glGetTexParameteriv,(GLenum target, GLenum pname, GLint *params))
GL_PROC(_GL,void,glHint,(GLenum target, GLenum mode))
GL_PROC(_GL,void,glIndexMask,(GLuint mask))
GL_PROC(_GL,void,glIndexPointer,(GLenum type, GLsizei stride, const GLvoid *pointer))
GL_PROC(_GL,void,glIndexd,(GLdouble c))
GL_PROC(_GL,void,glIndexdv,(const GLdouble *c))
GL_PROC(_GL,void,glIndexf,(GLfloat c))
GL_PROC(_GL,void,glIndexfv,(const GLfloat *c))
GL_PROC(_GL,void,glIndexi,(GLint c))
GL_PROC(_GL,void,glIndexiv,(const GLint *c))
GL_PROC(_GL,void,glIndexs,(GLshort c))
GL_PROC(_GL,void,glIndexsv,(const GLshort *c))
GL_PROC(_GL,void,glIndexub,(GLubyte c))
GL_PROC(_GL,void,glIndexubv,(const GLubyte *c))
GL_PROC(_GL,void,glInitNames,(void))
GL_PROC(_GL,void,glInterleavedArrays,(GLenum format, GLsizei stride, const GLvoid *pointer))
GL_PROC(_GL,GLboolean,glIsEnabled,(GLenum cap))
GL_PROC(_GL,GLboolean,glIsList,(GLuint l))
GL_PROC(_GL,GLboolean,glIsTexture,(GLuint texture))
GL_PROC(_GL,void,glLightModelf,(GLenum pname, GLfloat param))
GL_PROC(_GL,void,glLightModelfv,(GLenum pname, const GLfloat *params))
GL_PROC(_GL,void,glLightModeli,(GLenum pname, GLint param))
GL_PROC(_GL,void,glLightModeliv,(GLenum pname, const GLint *params))
GL_PROC(_GL,void,glLightf,(GLenum light, GLenum pname, GLfloat param))
GL_PROC(_GL,void,glLightfv,(GLenum light, GLenum pname, const GLfloat *params))
GL_PROC(_GL,void,glLighti,(GLenum light, GLenum pname, GLint param))
GL_PROC(_GL,void,glLightiv,(GLenum light, GLenum pname, const GLint *params))
GL_PROC(_GL,void,glLineStipple,(GLint factor, GLushort pattern))
GL_PROC(_GL,void,glLineWidth,(GLfloat width))
GL_PROC(_GL,void,glListBase,(GLuint base))
GL_PROC(_GL,void,glLoadIdentity,(void))
GL_PROC(_GL,void,glLoadMatrixd,(const GLdouble *m))
GL_PROC(_GL,void,glLoadMatrixf,(const GLfloat *m))
GL_PROC(_GL,void,glLoadName,(GLuint name))
GL_PROC(_GL,void,glLogicOp,(GLenum opcode))
GL_PROC(_GL,void,glMap1d,(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points))
GL_PROC(_GL,void,glMap1f,(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points))
GL_PROC(_GL,void,glMap2d,(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points))
GL_PROC(_GL,void,glMap2f,(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points))
GL_PROC(_GL,void,glMapGrid1d,(GLint un, GLdouble u1, GLdouble u2))
GL_PROC(_GL,void,glMapGrid1f,(GLint un, GLfloat u1, GLfloat u2))
GL_PROC(_GL,void,glMapGrid2d,(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2))
GL_PROC(_GL,void,glMapGrid2f,(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2))
GL_PROC(_GL,void,glMaterialf,(GLenum face, GLenum pname, GLfloat param))
GL_PROC(_GL,void,glMaterialfv,(GLenum face, GLenum pname, const GLfloat *params))
GL_PROC(_GL,void,glMateriali,(GLenum face, GLenum pname, GLint param))
GL_PROC(_GL,void,glMaterialiv,(GLenum face, GLenum pname, const GLint *params))
GL_PROC(_GL,void,glMatrixMode,(GLenum mode))
GL_PROC(_GL,void,glMultMatrixd,(const GLdouble *m))
GL_PROC(_GL,void,glMultMatrixf,(const GLfloat *m))
GL_PROC(_GL,void,glNewList,(GLuint l, GLenum mode))
GL_PROC(_GL,void,glNormal3b,(GLbyte nx, GLbyte ny, GLbyte nz))
GL_PROC(_GL,void,glNormal3bv,(const GLbyte *v))
GL_PROC(_GL,void,glNormal3d,(GLdouble nx, GLdouble ny, GLdouble nz))
GL_PROC(_GL,void,glNormal3dv,(const GLdouble *v))
GL_PROC(_GL,void,glNormal3f,(GLfloat nx, GLfloat ny, GLfloat nz))
GL_PROC(_GL,void,glNormal3fv,(const GLfloat *v))
GL_PROC(_GL,void,glNormal3i,(GLint nx, GLint ny, GLint nz))
GL_PROC(_GL,void,glNormal3iv,(const GLint *v))
GL_PROC(_GL,void,glNormal3s,(GLshort nx, GLshort ny, GLshort nz))
GL_PROC(_GL,void,glNormal3sv,(const GLshort *v))
GL_PROC(_GL,void,glNormalPointer,(GLenum type, GLsizei stride, const GLvoid *pointer))
GL_PROC(_GL,void,glOrtho,(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar))
GL_PROC(_GL,void,glPassThrough,(GLfloat token))
GL_PROC(_GL,void,glPixelMapfv,(GLenum map, GLsizei mapsize, const GLfloat *values))
GL_PROC(_GL,void,glPixelMapuiv,(GLenum map, GLsizei mapsize, const GLuint *values))
GL_PROC(_GL,void,glPixelMapusv,(GLenum map, GLsizei mapsize, const GLushort *values))
GL_PROC(_GL,void,glPixelStoref,(GLenum pname, GLfloat param))
GL_PROC(_GL,void,glPixelStorei,(GLenum pname, GLint param))
GL_PROC(_GL,void,glPixelTransferf,(GLenum pname, GLfloat param))
GL_PROC(_GL,void,glPixelTransferi,(GLenum pname, GLint param))
GL_PROC(_GL,void,glPixelZoom,(GLfloat xfactor, GLfloat yfactor))
GL_PROC(_GL,void,glPointSize,(GLfloat size))
GL_PROC(_GL,void,glPolygonMode,(GLenum face, GLenum mode))
GL_PROC(_GL,void,glPolygonOffset,(GLfloat factor, GLfloat units))
GL_PROC(_GL,void,glPolygonStipple,(const GLubyte *mask))
GL_PROC(_GL,void,glPopAttrib,(void))
GL_PROC(_GL,void,glPopClientAttrib,(void))
GL_PROC(_GL,void,glPopMatrix,(void))
GL_PROC(_GL,void,glPopName,(void))
GL_PROC(_GL,void,glPrioritizeTextures,(GLsizei n, const GLuint *textures, const GLclampf *priorities))
GL_PROC(_GL,void,glPushAttrib,(GLbitfield mask))
GL_PROC(_GL,void,glPushClientAttrib,(GLbitfield mask))
GL_PROC(_GL,void,glPushMatrix,(void))
GL_PROC(_GL,void,glPushName,(GLuint name))
GL_PROC(_GL,void,glRasterPos2d,(GLdouble x, GLdouble y))
GL_PROC(_GL,void,glRasterPos2dv,(const GLdouble *v))
GL_PROC(_GL,void,glRasterPos2f,(GLfloat x, GLfloat y))
GL_PROC(_GL,void,glRasterPos2fv,(const GLfloat *v))
GL_PROC(_GL,void,glRasterPos2i,(GLint x, GLint y))
GL_PROC(_GL,void,glRasterPos2iv,(const GLint *v))
GL_PROC(_GL,void,glRasterPos2s,(GLshort x, GLshort y))
GL_PROC(_GL,void,glRasterPos2sv,(const GLshort *v))
GL_PROC(_GL,void,glRasterPos3d,(GLdouble x, GLdouble y, GLdouble z))
GL_PROC(_GL,void,glRasterPos3dv,(const GLdouble *v))
GL_PROC(_GL,void,glRasterPos3f,(GLfloat x, GLfloat y, GLfloat z))
GL_PROC(_GL,void,glRasterPos3fv,(const GLfloat *v))
GL_PROC(_GL,void,glRasterPos3i,(GLint x, GLint y, GLint z))
GL_PROC(_GL,void,glRasterPos3iv,(const GLint *v))
GL_PROC(_GL,void,glRasterPos3s,(GLshort x, GLshort y, GLshort z))
GL_PROC(_GL,void,glRasterPos3sv,(const GLshort *v))
GL_PROC(_GL,void,glRasterPos4d,(GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_PROC(_GL,void,glRasterPos4dv,(const GLdouble *v))
GL_PROC(_GL,void,glRasterPos4f,(GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_PROC(_GL,void,glRasterPos4fv,(const GLfloat *v))
GL_PROC(_GL,void,glRasterPos4i,(GLint x, GLint y, GLint z, GLint w))
GL_PROC(_GL,void,glRasterPos4iv,(const GLint *v))
GL_PROC(_GL,void,glRasterPos4s,(GLshort x, GLshort y, GLshort z, GLshort w))
GL_PROC(_GL,void,glRasterPos4sv,(const GLshort *v))
GL_PROC(_GL,void,glReadBuffer,(GLenum mode))
GL_PROC(_GL,void,glReadPixels,(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels))
GL_PROC(_GL,void,glRectd,(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2))
GL_PROC(_GL,void,glRectdv,(const GLdouble *v1, const GLdouble *v2))
GL_PROC(_GL,void,glRectf,(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2))
GL_PROC(_GL,void,glRectfv,(const GLfloat *v1, const GLfloat *v2))
GL_PROC(_GL,void,glRecti,(GLint x1, GLint y1, GLint x2, GLint y2))
GL_PROC(_GL,void,glRectiv,(const GLint *v1, const GLint *v2))
GL_PROC(_GL,void,glRects,(GLshort x1, GLshort y1, GLshort x2, GLshort y2))
GL_PROC(_GL,void,glRectsv,(const GLshort *v1, const GLshort *v2))
GL_PROC(_GL,GLint,glRenderMode,(GLenum mode))
GL_PROC(_GL,void,glRotated,(GLdouble angle, GLdouble x, GLdouble y, GLdouble z))
GL_PROC(_GL,void,glRotatef,(GLfloat angle, GLfloat x, GLfloat y, GLfloat z))
GL_PROC(_GL,void,glScaled,(GLdouble x, GLdouble y, GLdouble z))
GL_PROC(_GL,void,glScalef,(GLfloat x, GLfloat y, GLfloat z))
GL_PROC(_GL,void,glScissor,(GLint x, GLint y, GLsizei width, GLsizei height))
GL_PROC(_GL,void,glSelectBuffer,(GLsizei size, GLuint *buffer))
GL_PROC(_GL,void,glShadeModel,(GLenum mode))
GL_PROC(_GL,void,glStencilFunc,(GLenum func, GLint ref, GLuint mask))
GL_PROC(_GL,void,glStencilMask,(GLuint mask))
GL_PROC(_GL,void,glStencilOp,(GLenum fail, GLenum zfail, GLenum zpass))
GL_PROC(_GL,void,glTexCoord1d,(GLdouble s))
GL_PROC(_GL,void,glTexCoord1dv,(const GLdouble *v))
GL_PROC(_GL,void,glTexCoord1f,(GLfloat s))
GL_PROC(_GL,void,glTexCoord1fv,(const GLfloat *v))
GL_PROC(_GL,void,glTexCoord1i,(GLint s))
GL_PROC(_GL,void,glTexCoord1iv,(const GLint *v))
GL_PROC(_GL,void,glTexCoord1s,(GLshort s))
GL_PROC(_GL,void,glTexCoord1sv,(const GLshort *v))
GL_PROC(_GL,void,glTexCoord2d,(GLdouble s, GLdouble t))
GL_PROC(_GL,void,glTexCoord2dv,(const GLdouble *v))
GL_PROC(_GL,void,glTexCoord2f,(GLfloat s, GLfloat t))
GL_PROC(_GL,void,glTexCoord2fv,(const GLfloat *v))
GL_PROC(_GL,void,glTexCoord2i,(GLint s, GLint t))
GL_PROC(_GL,void,glTexCoord2iv,(const GLint *v))
GL_PROC(_GL,void,glTexCoord2s,(GLshort s, GLshort t))
GL_PROC(_GL,void,glTexCoord2sv,(const GLshort *v))
GL_PROC(_GL,void,glTexCoord3d,(GLdouble s, GLdouble t, GLdouble r))
GL_PROC(_GL,void,glTexCoord3dv,(const GLdouble *v))
GL_PROC(_GL,void,glTexCoord3f,(GLfloat s, GLfloat t, GLfloat r))
GL_PROC(_GL,void,glTexCoord3fv,(const GLfloat *v))
GL_PROC(_GL,void,glTexCoord3i,(GLint s, GLint t, GLint r))
GL_PROC(_GL,void,glTexCoord3iv,(const GLint *v))
GL_PROC(_GL,void,glTexCoord3s,(GLshort s, GLshort t, GLshort r))
GL_PROC(_GL,void,glTexCoord3sv,(const GLshort *v))
GL_PROC(_GL,void,glTexCoord4d,(GLdouble s, GLdouble t, GLdouble r, GLdouble q))
GL_PROC(_GL,void,glTexCoord4dv,(const GLdouble *v))
GL_PROC(_GL,void,glTexCoord4f,(GLfloat s, GLfloat t, GLfloat r, GLfloat q))
GL_PROC(_GL,void,glTexCoord4fv,(const GLfloat *v))
GL_PROC(_GL,void,glTexCoord4i,(GLint s, GLint t, GLint r, GLint q))
GL_PROC(_GL,void,glTexCoord4iv,(const GLint *v))
GL_PROC(_GL,void,glTexCoord4s,(GLshort s, GLshort t, GLshort r, GLshort q))
GL_PROC(_GL,void,glTexCoord4sv,(const GLshort *v))
GL_PROC(_GL,void,glTexCoordPointer,(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer))
GL_PROC(_GL,void,glTexEnvf,(GLenum target, GLenum pname, GLfloat param))
GL_PROC(_GL,void,glTexEnvfv,(GLenum target, GLenum pname, const GLfloat *params))
GL_PROC(_GL,void,glTexEnvi,(GLenum target, GLenum pname, GLint param))
GL_PROC(_GL,void,glTexEnviv,(GLenum target, GLenum pname, const GLint *params))
GL_PROC(_GL,void,glTexGend,(GLenum coord, GLenum pname, GLdouble param))
GL_PROC(_GL,void,glTexGendv,(GLenum coord, GLenum pname, const GLdouble *params))
GL_PROC(_GL,void,glTexGenf,(GLenum coord, GLenum pname, GLfloat param))
GL_PROC(_GL,void,glTexGenfv,(GLenum coord, GLenum pname, const GLfloat *params))
GL_PROC(_GL,void,glTexGeni,(GLenum coord, GLenum pname, GLint param))
GL_PROC(_GL,void,glTexGeniv,(GLenum coord, GLenum pname, const GLint *params))
GL_PROC(_GL,void,glTexImage1D,(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels))
GL_PROC(_GL,void,glTexImage2D,(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels))
GL_PROC(_GL,void,glTexParameterf,(GLenum target, GLenum pname, GLfloat param))
GL_PROC(_GL,void,glTexParameterfv,(GLenum target, GLenum pname, const GLfloat *params))
GL_PROC(_GL,void,glTexParameteri,(GLenum target, GLenum pname, GLint param))
GL_PROC(_GL,void,glTexParameteriv,(GLenum target, GLenum pname, const GLint *params))
GL_PROC(_GL,void,glTexSubImage1D,(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels))
GL_PROC(_GL,void,glTexSubImage2D,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels))
GL_PROC(_GL,void,glTranslated,(GLdouble x, GLdouble y, GLdouble z))
GL_PROC(_GL,void,glTranslatef,(GLfloat x, GLfloat y, GLfloat z))
GL_PROC(_GL,void,glVertex2d,(GLdouble x, GLdouble y))
GL_PROC(_GL,void,glVertex2dv,(const GLdouble *v))
GL_PROC(_GL,void,glVertex2f,(GLfloat x, GLfloat y))
GL_PROC(_GL,void,glVertex2fv,(const GLfloat *v))
GL_PROC(_GL,void,glVertex2i,(GLint x, GLint y))
GL_PROC(_GL,void,glVertex2iv,(const GLint *v))
GL_PROC(_GL,void,glVertex2s,(GLshort x, GLshort y))
GL_PROC(_GL,void,glVertex2sv,(const GLshort *v))
GL_PROC(_GL,void,glVertex3d,(GLdouble x, GLdouble y, GLdouble z))
GL_PROC(_GL,void,glVertex3dv,(const GLdouble *v))
GL_PROC(_GL,void,glVertex3f,(GLfloat x, GLfloat y, GLfloat z))
GL_PROC(_GL,void,glVertex3fv,(const GLfloat *v))
GL_PROC(_GL,void,glVertex3i,(GLint x, GLint y, GLint z))
GL_PROC(_GL,void,glVertex3iv,(const GLint *v))
GL_PROC(_GL,void,glVertex3s,(GLshort x, GLshort y, GLshort z))
GL_PROC(_GL,void,glVertex3sv,(const GLshort *v))
GL_PROC(_GL,void,glVertex4d,(GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_PROC(_GL,void,glVertex4dv,(const GLdouble *v))
GL_PROC(_GL,void,glVertex4f,(GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_PROC(_GL,void,glVertex4fv,(const GLfloat *v))
GL_PROC(_GL,void,glVertex4i,(GLint x, GLint y, GLint z, GLint w))
GL_PROC(_GL,void,glVertex4iv,(const GLint *v))
GL_PROC(_GL,void,glVertex4s,(GLshort x, GLshort y, GLshort z, GLshort w))
GL_PROC(_GL,void,glVertex4sv,(const GLshort *v))
GL_PROC(_GL,void,glVertexPointer,(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer))
GL_PROC(_GL,void,glViewport,(GLint x, GLint y, GLsizei width, GLsizei height))

// WGL functions.
GL_PROC(_GL,BOOL,pwglCopyContext,(HGLRC,HGLRC,UINT))
GL_PROC(_GL,HGLRC,pwglCreateContext,(HDC))
GL_PROC(_GL,HGLRC,pwglCreateLayerContext,(HGLRC))
GL_PROC(_GL,BOOL,pwglDeleteContext,(HGLRC))
GL_PROC(_GL,HGLRC,pwglGetCurrentContext,(VOID))
GL_PROC(_GL,HDC,pwglGetCurrentDC,(VOID))
GL_PROC(_GL,PROC,pwglGetProcAddress,(LPCSTR))
GL_PROC(_GL,BOOL,pwglMakeCurrent,(HDC, HGLRC))
GL_PROC(_GL,BOOL,pwglShareLists,(HGLRC,HGLRC))
GL_PROC(_GL,INT,pwglChoosePixelFormat,(HDC hDC,CONST PIXELFORMATDESCRIPTOR*))
GL_PROC(_GL,INT,pwglDescribePixelFormat,(HDC,INT,UINT,PIXELFORMATDESCRIPTOR*))
GL_PROC(_GL,BOOL,pwglSetPixelFormat,(HDC,INT,CONST PIXELFORMATDESCRIPTOR*))
GL_PROC(_GL,BOOL,pwglSwapBuffers,(HDC hDC))

// GDI functions.
GL_PROC(_GL,INT,pChoosePixelFormat,(HDC hDC,CONST PIXELFORMATDESCRIPTOR*))
GL_PROC(_GL,INT,pDescribePixelFormat,(HDC,INT,UINT,PIXELFORMATDESCRIPTOR*))
GL_PROC(_GL,BOOL,pGetPixelFormat,(HDC))
GL_PROC(_GL,BOOL,pSetPixelFormat,(HDC,INT,CONST PIXELFORMATDESCRIPTOR*))
GL_PROC(_GL,BOOL,pSwapBuffers,(HDC hDC))

#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE_MIN_LOD                0x813A
#define GL_TEXTURE_MAX_LOD                0x813B
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D

/*-----------------------------------------------------------------------------
	OpenGL extensions.
-----------------------------------------------------------------------------*/

#if defined(_WIN64)
typedef int64 GLintptrARB;
typedef int64 GLsizeiptrARB;
#elif defined(__ia64__) || defined(__x86_64__)
typedef long int GLintptrARB;
typedef long int GLsizeiptrARB;
#else
typedef int GLintptrARB;
typedef int GLsizeiptrARB;
#endif 


// BGRA textures.
GL_EXT(_GL_EXT_bgra)
#define GL_BGR_EXT                          0x80E0
#define GL_BGRA_EXT                         0x80E1

// Paletted textures.
GL_EXT(_GL_EXT_paletted_texture)
GL_PROC(_GL_EXT_paletted_texture,void,glColorTableEXT,(GLenum target,GLenum internalFormat,GLsizei width,GLenum format,GLenum type,const void *data))
GL_PROC(_GL_EXT_paletted_texture,void,glColorSubTableEXT,(GLenum target,GLsizei start,GLsizei count,GLenum format,GLenum type,const void *data))
GL_PROC(_GL_EXT_paletted_texture,void,glGetColorTableEXT,(GLenum target,GLenum format,GLenum type,void *data))
GL_PROC(_GL_EXT_paletted_texture,void,glGetColorTableParameterivEXT,(GLenum target,GLenum pname,int *params))
GL_PROC(_GL_EXT_paletted_texture,void,glGetColorTableParameterfvEXT,(GLenum target,GLenum pname,float *params))

// ARB_texture_compression.
GL_EXT(_GL_ARB_texture_compression)
GL_PROC(_GL_ARB_texture_compression,void,glCompressedTexImage3DARB,(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *))
GL_PROC(_GL_ARB_texture_compression,void,glCompressedTexImage2DARB,(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *))
GL_PROC(_GL_ARB_texture_compression,void,glCompressedTexImage1DARB,(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *))
GL_PROC(_GL_ARB_texture_compression,void,glCompressedTexSubImage3DARB,(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *))
GL_PROC(_GL_ARB_texture_compression,void,glCompressedTexSubImage2DARB,(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *))
GL_PROC(_GL_ARB_texture_compression,void,glCompressedTexSubImage1DARB,(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *))
GL_PROC(_GL_ARB_texture_compression,void,glGetCompressedTexImageARB,(GLenum, GLint, void *))
#define GL_COMPRESSED_ALPHA_ARB           0x84E9
#define GL_COMPRESSED_LUMINANCE_ARB       0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB 0x84EB
#define GL_COMPRESSED_INTENSITY_ARB       0x84EC
#define GL_COMPRESSED_RGB_ARB             0x84ED
#define GL_COMPRESSED_RGBA_ARB            0x84EE
#define GL_TEXTURE_COMPRESSION_HINT_ARB   0x84EF
#define GL_TEXTURE_IMAGE_SIZE_ARB         0x86A0
#define GL_TEXTURE_COMPRESSED_ARB         0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A3
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

GL_EXT(_GL_EXT_depth_bounds_test)
GL_PROC(_GL_EXT_depth_bounds_test,void,glDepthBoundsEXT,(GLclampd, GLclampd))
#define GL_DEPTH_BOUNDS_TEST_EXT          0x8890
#define GL_DEPTH_BOUNDS_EXT               0x8890

// ARB_texture_cube_map.
GL_EXT(_GL_ARB_texture_cube_map)
#define GL_NORMAL_MAP_ARB                 0x8511
#define GL_REFLECTION_MAP_ARB             0x8512
#define GL_TEXTURE_CUBE_MAP_ARB           0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB   0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB     0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB  0x851C

// Compiled vertex arrays.
GL_EXT(_GL_EXT_compiled_vertex_array)
GL_PROC(_GL_EXT_compiled_vertex_array,void,glLockArraysEXT,(GLint first, GLsizei count))
GL_PROC(_GL_EXT_compiled_vertex_array,void,glUnlockArraysEXT,(void))

// Swap interval control.
GL_EXT(_WGL_EXT_swap_control)
GL_PROC(_WGL_EXT_swap_control,BOOL,wglSwapIntervalEXT,(int))

GL_EXT(_WGL_ARB_pixel_format)
GL_PROC(_WGL_ARB_pixel_format,BOOL,wglGetPixelFormatAttribivARB,(HDC, int, int, UINT, const int *, int *))
GL_PROC(_WGL_ARB_pixel_format,BOOL,wglGetPixelFormatAttribfvARB,(HDC, int, int, UINT, const int *, FLOAT *))
GL_PROC(_WGL_ARB_pixel_format,BOOL,wglChoosePixelFormatARB,(HDC, const int *, const FLOAT *, UINT, int *, UINT *))
#define WGL_NUMBER_PIXEL_FORMATS_ARB   0x2000
#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_DRAW_TO_BITMAP_ARB         0x2002
#define WGL_ACCELERATION_ARB           0x2003
#define WGL_NEED_PALETTE_ARB           0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB    0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB     0x2006
#define WGL_SWAP_METHOD_ARB            0x2007
#define WGL_NUMBER_OVERLAYS_ARB        0x2008
#define WGL_NUMBER_UNDERLAYS_ARB       0x2009
#define WGL_TRANSPARENT_ARB            0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB  0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB            0x200C
#define WGL_SHARE_STENCIL_ARB          0x200D
#define WGL_SHARE_ACCUM_ARB            0x200E
#define WGL_SUPPORT_GDI_ARB            0x200F
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_STEREO_ARB                 0x2012
#define WGL_PIXEL_TYPE_ARB             0x2013
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_RED_BITS_ARB               0x2015
#define WGL_RED_SHIFT_ARB              0x2016
#define WGL_GREEN_BITS_ARB             0x2017
#define WGL_GREEN_SHIFT_ARB            0x2018
#define WGL_BLUE_BITS_ARB              0x2019
#define WGL_BLUE_SHIFT_ARB             0x201A
#define WGL_ALPHA_BITS_ARB             0x201B
#define WGL_ALPHA_SHIFT_ARB            0x201C
#define WGL_ACCUM_BITS_ARB             0x201D
#define WGL_ACCUM_RED_BITS_ARB         0x201E
#define WGL_ACCUM_GREEN_BITS_ARB       0x201F
#define WGL_ACCUM_BLUE_BITS_ARB        0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB       0x2021
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#define WGL_AUX_BUFFERS_ARB            0x2024
#define WGL_NO_ACCELERATION_ARB        0x2025
#define WGL_GENERIC_ACCELERATION_ARB   0x2026
#define WGL_FULL_ACCELERATION_ARB      0x2027
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_SWAP_COPY_ARB              0x2029
#define WGL_SWAP_UNDEFINED_ARB         0x202A
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_TYPE_COLORINDEX_ARB        0x202C

GL_EXT(_WGL_ARB_render_texture)
GL_PROC(_WGL_ARB_render_texture,BOOL,wglBindTexImageARB,( HPBUFFERARB hPbuffer, int iBuffer ))
GL_PROC(_WGL_ARB_render_texture,BOOL,wglReleaseTexImageARB,( HPBUFFERARB hPbuffer, int iBuffer ))
#define WGL_BIND_TO_TEXTURE_RGB_ARB        0x2070
#define WGL_BIND_TO_TEXTURE_RGBA_ARB       0x2071
#define WGL_TEXTURE_FORMAT_ARB             0x2072
#define WGL_TEXTURE_TARGET_ARB             0x2073
#define WGL_MIPMAP_TEXTURE_ARB             0x2074
#define WGL_TEXTURE_RGB_ARB                0x2075
#define WGL_TEXTURE_RGBA_ARB               0x2076
#define WGL_NO_TEXTURE_ARB                 0x2077
#define WGL_TEXTURE_CUBE_MAP_ARB           0x2078
#define WGL_TEXTURE_1D_ARB                 0x2079
#define WGL_TEXTURE_2D_ARB                 0x207A
#define WGL_MIPMAP_LEVEL_ARB               0x207B
#define WGL_CUBE_MAP_FACE_ARB              0x207C
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x207D
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x207E
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x207F
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x2080
#define WGL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x2081
#define WGL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x2082
#define WGL_FRONT_LEFT_ARB                 0x2083
#define WGL_FRONT_RIGHT_ARB                0x2084
#define WGL_BACK_LEFT_ARB                  0x2085
#define WGL_BACK_RIGHT_ARB                 0x2086
#define WGL_AUX0_ARB                       0x2087
#define WGL_AUX1_ARB                       0x2088
#define WGL_AUX2_ARB                       0x2089
#define WGL_AUX3_ARB                       0x208A
#define WGL_AUX4_ARB                       0x208B
#define WGL_AUX5_ARB                       0x208C
#define WGL_AUX6_ARB                       0x208D
#define WGL_AUX7_ARB                       0x208E
#define WGL_AUX8_ARB                       0x208F
#define WGL_AUX9_ARB                       0x2090


GL_EXT(_WGL_ARB_make_current_read)
GL_PROC(_WGL_ARB_make_current_read,BOOL,wglMakeContextCurrentARB,(HDC, HDC, HGLRC))
GL_PROC(_WGL_ARB_make_current_read,HDC,wglGetCurrentReadDCARB,(void))
#define ERROR_INVALID_PIXEL_TYPE_ARB   0x2043
#define ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB 0x2054

GL_EXT(_GL_ARB_multisample)
GL_PROC(_GL_ARB_multisample,void,glSampleCoverageARB,(GLclampf, GLboolean))
//GL_PROC(_GL_ARB_multisample,void,glSamplePassARB,(GLenum))
#define GL_MULTISAMPLE_ARB                0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB   0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_ARB        0x809F
#define GL_SAMPLE_COVERAGE_ARB            0x80A0
#define GL_SAMPLE_BUFFERS_ARB             0x80A8
#define GL_SAMPLES_ARB                    0x80A9
#define GL_SAMPLE_COVERAGE_VALUE_ARB      0x80AA
#define GL_SAMPLE_COVERAGE_INVERT_ARB     0x80AB
#define GL_MULTISAMPLE_BIT_ARB            0x20000000

#define WGL_SAMPLE_BUFFERS_ARB            0x2041
#define WGL_SAMPLES_ARB                   0x2042

GL_EXT(_GL_NV_multisample_filter_hint)
#define GL_MULTISAMPLE_FILTER_HINT_NV        0x8534

GL_EXT(_WGL_ARB_pbuffer)
GL_PROC(_WGL_ARB_pbuffer,HPBUFFERARB,wglCreatePbufferARB,(HDC, int, int, int, const int *))
GL_PROC(_WGL_ARB_pbuffer,HDC,wglGetPbufferDCARB,(HPBUFFERARB))
GL_PROC(_WGL_ARB_pbuffer,int,wglReleasePbufferDCARB,(HPBUFFERARB, HDC))
GL_PROC(_WGL_ARB_pbuffer,BOOL,wglDestroyPbufferARB,(HPBUFFERARB))
GL_PROC(_WGL_ARB_pbuffer,BOOL,wglQueryPbufferARB,(HPBUFFERARB, int, int *))
#define WGL_DRAW_TO_PBUFFER_ARB        0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB     0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB      0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB     0x2030
#define WGL_PBUFFER_LARGEST_ARB        0x2033
#define WGL_PBUFFER_WIDTH_ARB          0x2034
#define WGL_PBUFFER_HEIGHT_ARB         0x2035
#define WGL_PBUFFER_LOST_ARB           0x2036

GL_EXT(_WGL_ARB_buffer_region)
GL_PROC(_WGL_ARB_buffer_region,HANDLE,wglCreateBufferRegionARB,(HDC, int, UINT))
GL_PROC(_WGL_ARB_buffer_region,void,wglDeleteBufferRegionARB,(HANDLE))
GL_PROC(_WGL_ARB_buffer_region,BOOL,wglSaveBufferRegionARB,(HANDLE, int, int, int, int))
GL_PROC(_WGL_ARB_buffer_region,BOOL,wglRestoreBufferRegionARB,(HANDLE, int, int, int, int, int, int))
#define WGL_FRONT_COLOR_BUFFER_BIT_ARB 0x00000001
#define WGL_BACK_COLOR_BUFFER_BIT_ARB  0x00000002
#define WGL_DEPTH_BUFFER_BIT_ARB       0x00000004
#define WGL_STENCIL_BUFFER_BIT_ARB     0x00000008

GL_EXT(_GL_EXT_texture_env_add)

// 3DFX gamma control.
GL_EXT(_WGL_3DFX_gamma_control)
GL_PROC(_WGL_3DFX_gamma_control,BOOL,wglGetDeviceGammaRamp3DFX,(HDC hDC, LPVOID lpRamp))
GL_PROC(_WGL_3DFX_gamma_control,BOOL,wglSetDeviceGammaRamp3DFX,(HDC hDC, LPVOID lpRamp))

// Windows swap control.
GL_EXT(_GL_WIN_swap_hint)

GL_EXT(_GL_SGIX_depth_texture)
#define GL_DEPTH_COMPONENT16_SGIX         0x81A5
#define GL_DEPTH_COMPONENT24_SGIX         0x81A6
#define GL_DEPTH_COMPONENT32_SGIX         0x81A7

GL_EXT(_GL_SGIX_shadow)
#define GL_TEXTURE_COMPARE_SGIX           0x819A
#define GL_TEXTURE_COMPARE_OPERATOR_SGIX  0x819B
#define GL_TEXTURE_LEQUAL_R_SGIX          0x819C
#define GL_TEXTURE_GEQUAL_R_SGIX          0x819D

// ARB multitexture.
GL_EXT(_GL_ARB_multitexture)
GL_PROC(_GL_ARB_multitexture,void,glMultiTexCoord1fARB,(GLenum target,GLfloat))
GL_PROC(_GL_ARB_multitexture,void,glMultiTexCoord2fARB,(GLenum target,GLfloat,GLfloat))
GL_PROC(_GL_ARB_multitexture,void,glMultiTexCoord3fARB,(GLenum target,GLfloat,GLfloat,GLfloat))
GL_PROC(_GL_ARB_multitexture,void,glMultiTexCoord4fARB,(GLenum target,GLfloat,GLfloat,GLfloat,GLfloat))
GL_PROC(_GL_ARB_multitexture,void,glMultiTexCoord1fvARB,(GLenum target,GLfloat))
GL_PROC(_GL_ARB_multitexture,void,glMultiTexCoord2fvARB,(GLenum target,const GLfloat*v))
GL_PROC(_GL_ARB_multitexture,void,glMultiTexCoord3fvARB,(GLenum target,const GLfloat*v))
GL_PROC(_GL_ARB_multitexture,void,glMultiTexCoord4fvARB,(GLenum target,const GLfloat*v))
GL_PROC(_GL_ARB_multitexture,void,glActiveTextureARB,(GLenum target))
GL_PROC(_GL_ARB_multitexture,void,glClientActiveTextureARB,(GLenum target))
#define GL_ACTIVE_TEXTURE_ARB               0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB        0x84E1
#define GL_MAX_TEXTURES_UNITS_ARB           0x84E2
#define GL_TEXTURE0_ARB                     0x84C0
#define GL_TEXTURE1_ARB                     0x84C1
#define GL_TEXTURE2_ARB                     0x84C2
#define GL_TEXTURE3_ARB                     0x84C3
#define GL_TEXTURE4_ARB                     0x84C4
#define GL_TEXTURE5_ARB                     0x84C5
#define GL_TEXTURE6_ARB                     0x84C6
#define GL_TEXTURE7_ARB                     0x84C7


// SGI multitexture. outdated!!

//GL_EXT(_GL_SGIS_multitexture)
//GL_PROC(_GL_SGIS_multitexture,void,glMultiTexCoord1fSGIS,(GLenum target,GLfloat))
//GL_PROC(_GL_SGIS_multitexture,void,glMultiTexCoord2fSGIS,(GLenum target,GLfloat,GLfloat))
//GL_PROC(_GL_SGIS_multitexture,void,glMTexCoord2fSGIS,(GLenum target,GLfloat,GLfloat))
//GL_PROC(_GL_SGIS_multitexture,void,glMultiTexCoord3fSGIS,(GLenum target,GLfloat,GLfloat,GLfloat))
//GL_PROC(_GL_SGIS_multitexture,void,glMultiTexCoord4fSGIS,(GLenum target,GLfloat,GLfloat,GLfloat,GLfloat))
//GL_PROC(_GL_SGIS_multitexture,void,glMultiTexCoord1fvSGIS,(GLenum target,GLfloat*))
//GL_PROC(_GL_SGIS_multitexture,void,glMultiTexCoord2fvSGIS,(GLenum target,GLfloat*))
//GL_PROC(_GL_SGIS_multitexture,void,glMultiTexCoord3fvSGIS,(GLenum target,GLfloat*))
//GL_PROC(_GL_SGIS_multitexture,void,glMultiTexCoord4fvSGIS,(GLenum target,GLfloat*))
//GL_PROC(_GL_SGIS_multitexture,void,glSelectTextureSGIS,(GLenum target))
//GL_PROC(_GL_SGIS_multitexture,void,glSelectTextureCoordSetSGIS,(GLenum target))
#define GL_SELECTED_TEXTURE_SGIS            0x835C
#define GL_MAX_TEXTURES_SGIS                0x835D
#define TEXTURE0_SGIS                       0x835E
#define TEXTURE1_SGIS                       0x835F
#define TEXTURE2_SGIS                       0x8360
#define TEXTURE3_SGIS                       0x8361

GL_EXT(_GL_EXT_texture_filter_anisotropic)
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

//WGL_EXT_gamma_control
// SetDeviceGammaRampExt
// GetDeviceGammaRampExt

//GL_EXT(_GL_EXT_clip_volume_hint)
// glClipVolumeEXT 0x80F0

GL_EXT(_GL_EXT_point_parameters)
GL_PROC(_GL_EXT_point_parameters,void,glPointParameterfEXT,(GLenum pname, GLfloat param))
GL_PROC(_GL_EXT_point_parameters,void,glPointParameterfvEXT,(GLenum pname, GLfloat *params))

GL_EXT(_GL_SGI_cull_vertex)
GL_PROC(_GL_SGI_cull_vertex,void,glCullParameterdvSGI,(GLenum pname, GLdouble* params))
GL_PROC(_GL_SGI_cull_vertex,void,glCullParameterfvSGI,(GLenum pname, GLfloat* params))

GL_EXT(_GL_EXT_texture_env)
//modulate 2x, etc

GL_EXT(_GL_EXT_texture_lod_bias)
#define GL_MAX_TEXTURE_LOD_BIAS_EXT       0x84FD
#define GL_TEXTURE_FILTER_CONTROL_EXT     0x8500
#define GL_TEXTURE_LOD_BIAS_EXT           0x8501

/* EXT_texture_env_combine */
GL_EXT(_GL_EXT_texture_env_combine)
#define GL_COMBINE_EXT                      0x8570
#define GL_COMBINE_RGB_EXT                  0x8571
#define GL_COMBINE_ALPHA_EXT                0x8572
#define GL_RGB_SCALE_EXT                    0x8573
#define GL_ADD_SIGNED_EXT                   0x8574
#define GL_INTERPOLATE_EXT                  0x8575
#define GL_CONSTANT_EXT                     0x8576
#define GL_PRIMARY_COLOR_EXT                0x8577
#define GL_PREVIOUS_EXT                     0x8578
#define GL_SOURCE0_RGB_EXT                  0x8580
#define GL_SOURCE1_RGB_EXT                  0x8581
#define GL_SOURCE2_RGB_EXT                  0x8582
#define GL_SOURCE0_ALPHA_EXT                0x8588
#define GL_SOURCE1_ALPHA_EXT                0x8589
#define GL_SOURCE2_ALPHA_EXT                0x858A
#define GL_OPERAND0_RGB_EXT                 0x8590
#define GL_OPERAND1_RGB_EXT                 0x8591
#define GL_OPERAND2_RGB_EXT                 0x8592
#define GL_OPERAND0_ALPHA_EXT               0x8598
#define GL_OPERAND1_ALPHA_EXT               0x8599
#define GL_OPERAND2_ALPHA_EXT               0x859A

/* NV_texture_env_combine4 */
GL_EXT(_GL_NV_texture_env_combine4)
#define GL_COMBINE4_NV                      0x8503
#define GL_SOURCE3_RGB_NV                   0x8583
#define GL_SOURCE3_ALPHA_NV                 0x858B
#define GL_OPERAND3_RGB_NV                  0x8593
#define GL_OPERAND3_ALPHA_NV                0x859B

/* EXT_separate_specular_color */
GL_EXT(_GL_EXT_separate_specular_color)
#define GL_SINGLE_COLOR_EXT                 0x81F9
#define GL_SEPARATE_SPECULAR_COLOR_EXT      0x81FA
#define GL_LIGHT_MODEL_COLOR_CONTROL_EXT    0x81F8

/* NV_vertex_array_range */
GL_EXT(_GL_NV_vertex_array_range)
GL_PROC(_GL_NV_vertex_array_range,void,glVertexArrayRangeNV,(int length, void *pointer))
GL_PROC(_GL_NV_vertex_array_range,void,glFlushVertexArrayRangeNV,(void))
GL_PROC(_GL_NV_vertex_array_range,void*,wglAllocateMemoryNV,(int size,float readFrequency,float writeFrequency,float priority))
GL_PROC(_GL_NV_vertex_array_range,void,wglFreeMemoryNV,(void *pointer))
#define GL_VERTEX_ARRAY_RANGE_NV            0x851D
#define GL_VERTEX_ARRAY_RANGE_LENGTH_NV     0x851E
#define GL_VERTEX_ARRAY_RANGE_VALID_NV      0x851F
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV 0x8520
#define GL_VERTEX_ARRAY_RANGE_POINTER_NV    0x8521

GL_EXT(_GL_NV_fence)
GL_PROC(_GL_NV_fence,void,glDeleteFencesNV,(GLsizei n, const GLuint *fences))
GL_PROC(_GL_NV_fence,void,glGenFencesNV,(GLsizei n, GLuint *fences))
GL_PROC(_GL_NV_fence,GLboolean,glIsFenceNV,(GLuint fence))
GL_PROC(_GL_NV_fence,GLboolean,glTestFenceNV,(GLuint fence))
GL_PROC(_GL_NV_fence,void,glGetFenceivNV,(GLuint fence, GLenum pname, GLint *params))
GL_PROC(_GL_NV_fence,void,glFinishFenceNV,(GLuint fence))
GL_PROC(_GL_NV_fence,void,glSetFenceNV,(GLuint fence, GLenum condition))
#define GL_ALL_COMPLETED_NV               0x84F2
#define GL_FENCE_STATUS_NV                0x84F3
#define GL_FENCE_CONDITION_NV             0x84F4

/* EXT_texture_cube_map */
GL_EXT(_GL_EXT_texture_cube_map)
#define GL_NORMAL_MAP_EXT                   0x8511
#define GL_REFLECTION_MAP_EXT               0x8512
#define GL_TEXTURE_CUBE_MAP_EXT             0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT     0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT  0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT  0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT  0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT  0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT  0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT  0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT       0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT    0x851C

/* EXT_secondary_color */
GL_EXT(_GL_EXT_secondary_color)
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3bEXT,(GLbyte, GLbyte, GLbyte))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3bvEXT,(const GLbyte *))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3dEXT,(GLdouble, GLdouble, GLdouble))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3dvEXT,(const GLdouble *))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3fEXT,(GLfloat, GLfloat, GLfloat))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3fvEXT,(const GLfloat *))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3iEXT,(GLint, GLint, GLint))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3ivEXT,(const GLint *))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3sEXT,(GLshort, GLshort, GLshort))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3svEXT,(const GLshort *))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3ubEXT,(GLubyte, GLubyte, GLubyte))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3ubvEXT,(const GLubyte *))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3uiEXT,(GLuint, GLuint, GLuint))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3uivEXT,(const GLuint *))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3usEXT,(GLushort, GLushort, GLushort))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColor3usvEXT,(const GLushort *))
GL_PROC(_GL_EXT_secondary_color,void,glSecondaryColorPointerEXT,(GLint, GLenum, GLsizei, GLvoid *))
#define GL_COLOR_SUM_EXT                  0x8458
#define GL_CURRENT_SECONDARY_COLOR_EXT    0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE_EXT 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE_EXT 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER_EXT 0x845D
#define GL_SECONDARY_COLOR_ARRAY_EXT      0x845E

GL_EXT(_GL_EXT_multi_draw_arrays)
GL_PROC(_GL_EXT_multi_draw_arrays,void,glMultiDrawArraysEXT,(GLenum, GLint *, GLsizei *, GLsizei))
GL_PROC(_GL_EXT_multi_draw_arrays,void,glMultiDrawElementsEXT,(GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei))

/* NV_fog_distance */
GL_EXT(_GL_NV_fog_distance)
#define GL_FOG_DISTANCE_MODE_NV           0x855A
#define GL_EYE_RADIAL_NV                  0x855B
#define GL_EYE_PLANE_ABSOLUTE_NV          0x855C

/* NV_point_sprite */
GL_EXT(_GL_NV_point_sprite)
GL_PROC(_GL_NV_point_sprite,void,glPointParameteriNV,(GLenum pname, int param))
GL_PROC(_GL_NV_point_sprite,void,glPointParameterivNV,(GLenum pname, const int *params))
#define GL_POINT_SPRITE_NV                0x8861
#define GL_COORD_REPLACE_NV               0x8862
#define GL_POINT_SPRITE_R_MODE_NV         0x8863

GL_EXT(_GL_NV_texgen_reflection)
#define GL_NORMAL_MAP_NV                  0x8511
#define GL_REFLECTION_MAP_NV              0x8512

/* NV_texgen_emboss */
GL_EXT(_GL_NV_texgen_emboss)
#define GL_EMBOSS_LIGHT_NV                0x855D
#define GL_EMBOSS_CONSTANT_NV             0x855E
#define GL_EMBOSS_MAP_NV                  0x855F

/* NV_texture_rectangle */
GL_EXT(_GL_NV_texture_rectangle)
#define GL_TEXTURE_RECTANGLE_NV           0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE_NV   0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE_NV     0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_NV  0x84F8

GL_EXT(_GL_EXT_texture_rectangle)

/* NV_register_combiners */
GL_EXT(_GL_NV_register_combiners)
GL_PROC(_GL_NV_register_combiners,void,glCombinerParameterfvNV,(GLenum pname, const GLfloat *params))
GL_PROC(_GL_NV_register_combiners,void,glCombinerParameterfNV,(GLenum pname, GLfloat param))
GL_PROC(_GL_NV_register_combiners,void,glCombinerParameterivNV,(GLenum pname, const GLint *params))
GL_PROC(_GL_NV_register_combiners,void,glCombinerParameteriNV,(GLenum pname, GLint param))
GL_PROC(_GL_NV_register_combiners,void,glCombinerInputNV,(GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage))
GL_PROC(_GL_NV_register_combiners,void,glCombinerOutputNV,(GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum))
GL_PROC(_GL_NV_register_combiners,void,glFinalCombinerInputNV,(GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage))
GL_PROC(_GL_NV_register_combiners,void,glGetCombinerInputParameterfvNV,(GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params))
GL_PROC(_GL_NV_register_combiners,void,glGetCombinerInputParameterivNV,(GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params))
GL_PROC(_GL_NV_register_combiners,void,glGetCombinerOutputParameterfvNV,(GLenum stage, GLenum portion, GLenum pname, GLfloat *params))
GL_PROC(_GL_NV_register_combiners,void,glGetCombinerOutputParameterivNV,(GLenum stage, GLenum portion, GLenum pname, GLint *params))
GL_PROC(_GL_NV_register_combiners,void,glGetFinalCombinerInputParameterfvNV,(GLenum variable, GLenum pname, GLfloat *params))
GL_PROC(_GL_NV_register_combiners,void,glGetFinalCombinerInputParameterivNV,(GLenum variable, GLenum pname, GLint *params))
#define GL_REGISTER_COMBINERS_NV            0x8522
#define GL_COMBINER0_NV                     0x8550
#define GL_COMBINER1_NV                     0x8551
#define GL_COMBINER2_NV                     0x8552
#define GL_COMBINER3_NV                     0x8553
#define GL_COMBINER4_NV                     0x8554
#define GL_COMBINER5_NV                     0x8555
#define GL_COMBINER6_NV                     0x8556
#define GL_COMBINER7_NV                     0x8557
#define GL_VARIABLE_A_NV                    0x8523
#define GL_VARIABLE_B_NV                    0x8524
#define GL_VARIABLE_C_NV                    0x8525
#define GL_VARIABLE_D_NV                    0x8526
#define GL_VARIABLE_E_NV                    0x8527
#define GL_VARIABLE_F_NV                    0x8528
#define GL_VARIABLE_G_NV                    0x8529
/*      GL_ZERO */
#define GL_CONSTANT_COLOR0_NV               0x852A
#define GL_CONSTANT_COLOR1_NV               0x852B
/*      GL_FOG */
#define GL_PRIMARY_COLOR_NV                 0x852C
#define GL_SECONDARY_COLOR_NV               0x852D
#define GL_SPARE0_NV                        0x852E
#define GL_SPARE1_NV                        0x852F
/*      GL_TEXTURE0_ARB */
/*      GL_TEXTURE1_ARB */
#define GL_UNSIGNED_IDENTITY_NV             0x8536
#define GL_UNSIGNED_INVERT_NV               0x8537
#define GL_EXPAND_NORMAL_NV                 0x8538
#define GL_EXPAND_NEGATE_NV                 0x8539
#define GL_HALF_BIAS_NORMAL_NV              0x853A
#define GL_HALF_BIAS_NEGATE_NV              0x853B
#define GL_SIGNED_IDENTITY_NV               0x853C
#define GL_SIGNED_NEGATE_NV                 0x853D
#define GL_E_TIMES_F_NV                     0x8531
#define GL_SPARE0_PLUS_SECONDARY_COLOR_NV   0x8532
/*      GL_NONE */
#define GL_SCALE_BY_TWO_NV                  0x853E
#define GL_SCALE_BY_FOUR_NV                 0x853F
#define GL_SCALE_BY_ONE_HALF_NV             0x8540
#define GL_BIAS_BY_NEGATIVE_ONE_HALF_NV     0x8541
#define GL_DISCARD_NV                       0x8530
#define GL_COMBINER_INPUT_NV                0x8542
#define GL_COMBINER_MAPPING_NV              0x8543
#define GL_COMBINER_COMPONENT_USAGE_NV      0x8544
#define GL_COMBINER_AB_DOT_PRODUCT_NV       0x8545
#define GL_COMBINER_CD_DOT_PRODUCT_NV       0x8546
#define GL_COMBINER_MUX_SUM_NV              0x8547
#define GL_COMBINER_SCALE_NV                0x8548
#define GL_COMBINER_BIAS_NV                 0x8549
#define GL_COMBINER_AB_OUTPUT_NV            0x854A
#define GL_COMBINER_CD_OUTPUT_NV            0x854B
#define GL_COMBINER_SUM_OUTPUT_NV           0x854C
#define GL_MAX_GENERAL_COMBINERS_NV         0x854D
#define GL_NUM_GENERAL_COMBINERS_NV         0x854E
#define GL_COLOR_SUM_CLAMP_NV               0x854F

/* NV_register_combiners2 */
GL_EXT(_GL_NV_register_combiners2)
GL_PROC(_GL_NV_register_combiners2,void,glCombinerStageParameterfvNV,(GLenum stage, GLenum pname, const GLfloat *params))
GL_PROC(_GL_NV_register_combiners2,void,glGetCombinerStageParameterfvNV,(GLenum stage, GLenum pname, GLfloat *params))
#define GL_PER_STAGE_CONSTANTS_NV         0x8535

/* NV_vertex_program3 */
GL_EXT(_GL_NV_vertex_program3)

/* NV_vertex_program2 */
GL_EXT(_GL_NV_vertex_program2)

/* NV_vertex_program */
GL_EXT(_GL_NV_vertex_program)
GL_PROC(_GL_NV_vertex_program,GLboolean,glAreProgramsResidentNV,(GLsizei n, const GLuint *programs, GLboolean *residences))
GL_PROC(_GL_NV_vertex_program,void,glBindProgramNV,(GLenum target, GLuint id))
GL_PROC(_GL_NV_vertex_program,void,glDeleteProgramsNV, (GLsizei n, const GLuint *programs))
GL_PROC(_GL_NV_vertex_program,void,glExecuteProgramNV, (GLenum target, GLuint id, const GLfloat *params))
GL_PROC(_GL_NV_vertex_program,void,glGenProgramsNV, (GLsizei n, GLuint *programs))
GL_PROC(_GL_NV_vertex_program,void,glGetProgramParameterdvNV, (GLenum target, GLuint index, GLenum pname, GLdouble *params))
GL_PROC(_GL_NV_vertex_program,void,glGetProgramParameterfvNV, (GLenum target, GLuint index, GLenum pname, GLfloat *params))
GL_PROC(_GL_NV_vertex_program,void,glGetProgramivNV, (GLuint id, GLenum pname, GLint *params))
GL_PROC(_GL_NV_vertex_program,void,glGetProgramStringNV, (GLuint id, GLenum pname, GLubyte *program))
GL_PROC(_GL_NV_vertex_program,void,glGetTrackMatrixivNV, (GLenum target, GLuint address, GLenum pname, GLint *params))
GL_PROC(_GL_NV_vertex_program,void,glGetVertexAttribdvNV, (GLuint index, GLenum pname, GLdouble *params))
GL_PROC(_GL_NV_vertex_program,void,glGetVertexAttribfvNV, (GLuint index, GLenum pname, GLfloat *params))
GL_PROC(_GL_NV_vertex_program,void,glGetVertexAttribivNV, (GLuint index, GLenum pname, GLint *params))
//GL_PROC(_GL_NV_vertex_program,void,glGetVertexAttribPointervNV, (GLuint index, GLenum pname, GLvoid* *pointer))
GL_PROC(_GL_NV_vertex_program,GLboolean,glIsProgramNV, (GLuint id))
GL_PROC(_GL_NV_vertex_program,void,glLoadProgramNV, (GLenum target, GLuint id, GLsizei len, const GLubyte *program))
GL_PROC(_GL_NV_vertex_program,void,glProgramParameter4dNV, (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_PROC(_GL_NV_vertex_program,void,glProgramParameter4dvNV, (GLenum target, GLuint index, const GLdouble *v))
GL_PROC(_GL_NV_vertex_program,void,glProgramParameter4fNV, (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_PROC(_GL_NV_vertex_program,void,glProgramParameter4fvNV, (GLenum target, GLuint index, const GLfloat *v))
GL_PROC(_GL_NV_vertex_program,void,glProgramParameters4dvNV, (GLenum target, GLuint index, GLsizei count, const GLdouble *v))
GL_PROC(_GL_NV_vertex_program,void,glProgramParameters4fvNV, (GLenum target, GLuint index, GLsizei count, const GLfloat *v))
GL_PROC(_GL_NV_vertex_program,void,glRequestResidentProgramsNV, (GLsizei n, const GLuint *programs))
GL_PROC(_GL_NV_vertex_program,void,glTrackMatrixNV, (GLenum target, GLuint address, GLenum matrix, GLenum transform))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribPointerNV, (GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib1dNV, (GLuint index, GLdouble x))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib1dvNV, (GLuint index, const GLdouble *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib1fNV, (GLuint index, GLfloat x))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib1fvNV, (GLuint index, const GLfloat *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib1sNV, (GLuint index, GLshort x))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib1svNV, (GLuint index, const GLshort *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib2dNV, (GLuint index, GLdouble x, GLdouble y))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib2dvNV, (GLuint index, const GLdouble *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib2fNV, (GLuint index, GLfloat x, GLfloat y))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib2fvNV, (GLuint index, const GLfloat *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib2sNV, (GLuint index, GLshort x, GLshort y))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib2svNV, (GLuint index, const GLshort *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib3dNV, (GLuint index, GLdouble x, GLdouble y, GLdouble z))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib3dvNV, (GLuint index, const GLdouble *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib3fNV, (GLuint index, GLfloat x, GLfloat y, GLfloat z))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib3fvNV, (GLuint index, const GLfloat *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib3sNV, (GLuint index, GLshort x, GLshort y, GLshort z))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib3svNV, (GLuint index, const GLshort *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib4dNV, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib4dvNV, (GLuint index, const GLdouble *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib4fNV, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib4fvNV, (GLuint index, const GLfloat *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib4sNV, (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib4svNV, (GLuint index, const GLshort *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttrib4ubvNV, (GLuint index, const GLubyte *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs1dvNV, (GLuint index, GLsizei count, const GLdouble *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs1fvNV, (GLuint index, GLsizei count, const GLfloat *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs1svNV, (GLuint index, GLsizei count, const GLshort *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs2dvNV, (GLuint index, GLsizei count, const GLdouble *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs2fvNV, (GLuint index, GLsizei count, const GLfloat *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs2svNV, (GLuint index, GLsizei count, const GLshort *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs3dvNV, (GLuint index, GLsizei count, const GLdouble *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs3fvNV, (GLuint index, GLsizei count, const GLfloat *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs3svNV, (GLuint index, GLsizei count, const GLshort *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs4dvNV, (GLuint index, GLsizei count, const GLdouble *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs4fvNV, (GLuint index, GLsizei count, const GLfloat *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs4svNV, (GLuint index, GLsizei count, const GLshort *v))
GL_PROC(_GL_NV_vertex_program,void,glVertexAttribs4ubvNV, (GLuint index, GLsizei count, const GLubyte *v))
#define GL_VERTEX_PROGRAM_NV              0x8620
#define GL_VERTEX_STATE_PROGRAM_NV        0x8621
#define GL_ATTRIB_ARRAY_SIZE_NV           0x8623
#define GL_ATTRIB_ARRAY_STRIDE_NV         0x8624
#define GL_ATTRIB_ARRAY_TYPE_NV           0x8625
#define GL_CURRENT_ATTRIB_NV              0x8626
#define GL_PROGRAM_LENGTH_NV              0x8627
#define GL_PROGRAM_STRING_NV              0x8628
#define GL_MODELVIEW_PROJECTION_NV        0x8629
#define GL_IDENTITY_NV                    0x862A
#define GL_INVERSE_NV                     0x862B
#define GL_TRANSPOSE_NV                   0x862C
#define GL_INVERSE_TRANSPOSE_NV           0x862D
#define GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV 0x862E
#define GL_MAX_TRACK_MATRICES_NV          0x862F
#define GL_MATRIX0_NV                     0x8630
#define GL_MATRIX1_NV                     0x8631
#define GL_MATRIX2_NV                     0x8632
#define GL_MATRIX3_NV                     0x8633
#define GL_MATRIX4_NV                     0x8634
#define GL_MATRIX5_NV                     0x8635
#define GL_MATRIX6_NV                     0x8636
#define GL_MATRIX7_NV                     0x8637
#define GL_CURRENT_MATRIX_STACK_DEPTH_NV  0x8640
#define GL_CURRENT_MATRIX_NV              0x8641
#define GL_VERTEX_PROGRAM_POINT_SIZE_NV   0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_NV     0x8643
#define GL_PROGRAM_PARAMETER_NV           0x8644
#define GL_ATTRIB_ARRAY_POINTER_NV        0x8645
#define GL_PROGRAM_TARGET_NV              0x8646
#define GL_PROGRAM_RESIDENT_NV            0x8647
#define GL_TRACK_MATRIX_NV                0x8648
#define GL_TRACK_MATRIX_TRANSFORM_NV      0x8649
#define GL_VERTEX_PROGRAM_BINDING_NV      0x864A
#define GL_PROGRAM_ERROR_POSITION_NV      0x864B
#define GL_VERTEX_ATTRIB_ARRAY0_NV        0x8650
#define GL_VERTEX_ATTRIB_ARRAY1_NV        0x8651
#define GL_VERTEX_ATTRIB_ARRAY2_NV        0x8652
#define GL_VERTEX_ATTRIB_ARRAY3_NV        0x8653
#define GL_VERTEX_ATTRIB_ARRAY4_NV        0x8654
#define GL_VERTEX_ATTRIB_ARRAY5_NV        0x8655
#define GL_VERTEX_ATTRIB_ARRAY6_NV        0x8656
#define GL_VERTEX_ATTRIB_ARRAY7_NV        0x8657
#define GL_VERTEX_ATTRIB_ARRAY8_NV        0x8658
#define GL_VERTEX_ATTRIB_ARRAY9_NV        0x8659
#define GL_VERTEX_ATTRIB_ARRAY10_NV       0x865A
#define GL_VERTEX_ATTRIB_ARRAY11_NV       0x865B
#define GL_VERTEX_ATTRIB_ARRAY12_NV       0x865C
#define GL_VERTEX_ATTRIB_ARRAY13_NV       0x865D
#define GL_VERTEX_ATTRIB_ARRAY14_NV       0x865E
#define GL_VERTEX_ATTRIB_ARRAY15_NV       0x865F
#define GL_MAP1_VERTEX_ATTRIB0_4_NV       0x8660
#define GL_MAP1_VERTEX_ATTRIB1_4_NV       0x8661
#define GL_MAP1_VERTEX_ATTRIB2_4_NV       0x8662
#define GL_MAP1_VERTEX_ATTRIB3_4_NV       0x8663
#define GL_MAP1_VERTEX_ATTRIB4_4_NV       0x8664
#define GL_MAP1_VERTEX_ATTRIB5_4_NV       0x8665
#define GL_MAP1_VERTEX_ATTRIB6_4_NV       0x8666
#define GL_MAP1_VERTEX_ATTRIB7_4_NV       0x8667
#define GL_MAP1_VERTEX_ATTRIB8_4_NV       0x8668
#define GL_MAP1_VERTEX_ATTRIB9_4_NV       0x8669
#define GL_MAP1_VERTEX_ATTRIB10_4_NV      0x866A
#define GL_MAP1_VERTEX_ATTRIB11_4_NV      0x866B
#define GL_MAP1_VERTEX_ATTRIB12_4_NV      0x866C
#define GL_MAP1_VERTEX_ATTRIB13_4_NV      0x866D
#define GL_MAP1_VERTEX_ATTRIB14_4_NV      0x866E
#define GL_MAP1_VERTEX_ATTRIB15_4_NV      0x866F
#define GL_MAP2_VERTEX_ATTRIB0_4_NV       0x8670
#define GL_MAP2_VERTEX_ATTRIB1_4_NV       0x8671
#define GL_MAP2_VERTEX_ATTRIB2_4_NV       0x8672
#define GL_MAP2_VERTEX_ATTRIB3_4_NV       0x8673
#define GL_MAP2_VERTEX_ATTRIB4_4_NV       0x8674
#define GL_MAP2_VERTEX_ATTRIB5_4_NV       0x8675
#define GL_MAP2_VERTEX_ATTRIB6_4_NV       0x8676
#define GL_MAP2_VERTEX_ATTRIB7_4_NV       0x8677
#define GL_MAP2_VERTEX_ATTRIB8_4_NV       0x8678
#define GL_MAP2_VERTEX_ATTRIB9_4_NV       0x8679
#define GL_MAP2_VERTEX_ATTRIB10_4_NV      0x867A
#define GL_MAP2_VERTEX_ATTRIB11_4_NV      0x867B
#define GL_MAP2_VERTEX_ATTRIB12_4_NV      0x867C
#define GL_MAP2_VERTEX_ATTRIB13_4_NV      0x867D
#define GL_MAP2_VERTEX_ATTRIB14_4_NV      0x867E
#define GL_MAP2_VERTEX_ATTRIB15_4_NV      0x867F

GL_EXT(_GL_NV_fragment_program)
GL_PROC(_GL_NV_fragment_program,void,glProgramNamedParameter4fNV,(GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_PROC(_GL_NV_fragment_program,void,glProgramNamedParameter4dNV,(GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_PROC(_GL_NV_fragment_program,void,glProgramNamedParameter4fvNV,(GLuint id, GLsizei len, const GLubyte *name, const GLfloat v[]))
GL_PROC(_GL_NV_fragment_program,void,glProgramNamedParameter4dvNV,(GLuint id, GLsizei len, const GLubyte *name, const GLdouble v[]))
GL_PROC(_GL_NV_fragment_program,void,glGetProgramNamedParameterfvNV,(GLuint id, GLsizei len, const GLubyte *name, GLfloat *params))
GL_PROC(_GL_NV_fragment_program,void,glGetProgramNamedParameterdvNV,(GLuint id, GLsizei len, const GLubyte *name, GLdouble *params))
#define GL_FRAGMENT_PROGRAM_NV            0x8870
#define GL_MAX_TEXTURE_COORDS_NV          0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_NV     0x8872
#define GL_FRAGMENT_PROGRAM_BINDING_NV    0x8873
#define GL_PROGRAM_ERROR_STRING_NV        0x8874
#define GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV 0x8868
 
GL_EXT(_GL_ARB_vertex_program)
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib1sARB,(GLuint index, GLshort x))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib1fARB,(GLuint index, GLfloat x))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib1dARB,(GLuint index, GLdouble x))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib2sARB,(GLuint index, GLshort x, GLshort y))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib2fARB,(GLuint index, GLfloat x, GLfloat y))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib2dARB,(GLuint index, GLdouble x, GLdouble y))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib3sARB,(GLuint index, GLshort x, GLshort y, GLshort z))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib3fARB,(GLuint index, GLfloat x, GLfloat y, GLfloat z))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib3dARB,(GLuint index, GLdouble x, GLdouble y, GLdouble z))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4sARB,(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4fARB,(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4dARB,(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4NubARB,(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib1svARB,(GLuint index, const GLshort *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib1fvARB,(GLuint index, const GLfloat *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib1dvARB,(GLuint index, const GLdouble *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib2svARB,(GLuint index, const GLshort *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib2fvARB,(GLuint index, const GLfloat *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib2dvARB,(GLuint index, const GLdouble *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib3svARB,(GLuint index, const GLshort *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib3fvARB,(GLuint index, const GLfloat *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib3dvARB,(GLuint index, const GLdouble *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4bvARB,(GLuint index, const GLbyte *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4svARB,(GLuint index, const GLshort *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4ivARB,(GLuint index, const GLint *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4ubvARB,(GLuint index, const GLubyte *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4usvARB,(GLuint index, const GLushort *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4uivARB,(GLuint index, const GLuint *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4fvARB,(GLuint index, const GLfloat *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4dvARB,(GLuint index, const GLdouble *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4NbvARB,(GLuint index, const GLbyte *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4NsvARB,(GLuint index, const GLshort *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4NivARB,(GLuint index, const GLint *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4NubvARB,(GLuint index, const GLubyte *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4NusvARB,(GLuint index, const GLushort *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttrib4NuivARB,(GLuint index, const GLuint *v))
GL_PROC(_GL_ARB_vertex_program,void,glVertexAttribPointerARB,(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer))
GL_PROC(_GL_ARB_vertex_program,void,glEnableVertexAttribArrayARB,(GLuint index))
GL_PROC(_GL_ARB_vertex_program,void,glDisableVertexAttribArrayARB,(GLuint index))
GL_PROC(_GL_ARB_vertex_program,void,glProgramStringARB,(GLenum target, GLenum format, GLsizei len, const void *string))
GL_PROC(_GL_ARB_vertex_program,void,glBindProgramARB,(GLenum target, GLuint program))
GL_PROC(_GL_ARB_vertex_program,void,glDeleteProgramsARB,(GLsizei n, const GLuint *programs))
GL_PROC(_GL_ARB_vertex_program,void,glGenProgramsARB,(GLsizei n, GLuint *programs))
GL_PROC(_GL_ARB_vertex_program,void,glProgramEnvParameter4dARB,(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_PROC(_GL_ARB_vertex_program,void,glProgramEnvParameter4dvARB,(GLenum target, GLuint index, const GLdouble *params))
GL_PROC(_GL_ARB_vertex_program,void,glProgramEnvParameter4fARB,(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_PROC(_GL_ARB_vertex_program,void,glProgramEnvParameter4fvARB,(GLenum target, GLuint index, const GLfloat *params))
GL_PROC(_GL_ARB_vertex_program,void,glProgramLocalParameter4dARB,(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
GL_PROC(_GL_ARB_vertex_program,void,glProgramLocalParameter4dvARB,(GLenum target, GLuint index, const GLdouble *params))
GL_PROC(_GL_ARB_vertex_program,void,glProgramLocalParameter4fARB,(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
GL_PROC(_GL_ARB_vertex_program,void,glProgramLocalParameter4fvARB,(GLenum target, GLuint index, const GLfloat *params))
GL_PROC(_GL_ARB_vertex_program,void,glGetProgramEnvParameterdvARB,(GLenum target, GLuint index, GLdouble *params))
GL_PROC(_GL_ARB_vertex_program,void,glGetProgramEnvParameterfvARB,(GLenum target, GLuint index, GLfloat *params))
GL_PROC(_GL_ARB_vertex_program,void,glGetProgramLocalParameterdvARB,(GLenum target, GLuint index, GLdouble *params))
GL_PROC(_GL_ARB_vertex_program,void,glGetProgramLocalParameterfvARB,(GLenum target, GLuint index, GLfloat *params))
GL_PROC(_GL_ARB_vertex_program,void,glGetProgramivARB,(GLenum target, GLenum pname, GLint *params))
GL_PROC(_GL_ARB_vertex_program,void,glGetProgramStringARB,(GLenum target, GLenum pname, void *string))
GL_PROC(_GL_ARB_vertex_program,void,glGetVertexAttribdvARB,(GLuint index, GLenum pname, GLdouble *params))
GL_PROC(_GL_ARB_vertex_program,void,glGetVertexAttribfvARB,(GLuint index, GLenum pname, GLfloat *params))
GL_PROC(_GL_ARB_vertex_program,void,glGetVertexAttribivARB,(GLuint index, GLenum pname, GLint *params))
GL_PROC(_GL_ARB_vertex_program,void,glGetVertexAttribPointervARB,(GLuint index, GLenum pname, void **pointer))
GL_PROC(_GL_ARB_vertex_program,GLboolean,glIsProgramARB,(GLuint program))
#define GL_VERTEX_PROGRAM_ARB                       0x8620
#define GL_VERTEX_PROGRAM_POINT_SIZE_ARB            0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE_ARB              0x8643
#define GL_COLOR_SUM_ARB                            0x8458
#define GL_PROGRAM_FORMAT_ASCII_ARB                 0x8875
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED_ARB          0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE_ARB             0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE_ARB           0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE_ARB             0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED_ARB       0x886A
#define GL_CURRENT_VERTEX_ATTRIB_ARB                0x8626
#define GL_VERTEX_ATTRIB_ARRAY_POINTER_ARB          0x8645
#define GL_PROGRAM_LENGTH_ARB                       0x8627
#define GL_PROGRAM_FORMAT_ARB                       0x8876
#define GL_PROGRAM_BINDING_ARB                      0x8677
#define GL_PROGRAM_INSTRUCTIONS_ARB                 0x88A0
#define GL_MAX_PROGRAM_INSTRUCTIONS_ARB             0x88A1
#define GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB          0x88A2
#define GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB      0x88A3
#define GL_PROGRAM_TEMPORARIES_ARB                  0x88A4
#define GL_MAX_PROGRAM_TEMPORARIES_ARB              0x88A5
#define GL_PROGRAM_NATIVE_TEMPORARIES_ARB           0x88A6
#define GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB       0x88A7
#define GL_PROGRAM_PARAMETERS_ARB                   0x88A8
#define GL_MAX_PROGRAM_PARAMETERS_ARB               0x88A9
#define GL_PROGRAM_NATIVE_PARAMETERS_ARB            0x88AA
#define GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB        0x88AB
#define GL_PROGRAM_ATTRIBS_ARB                      0x88AC
#define GL_MAX_PROGRAM_ATTRIBS_ARB                  0x88AD
#define GL_PROGRAM_NATIVE_ATTRIBS_ARB               0x88AE
#define GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB           0x88AF
#define GL_PROGRAM_ADDRESS_REGISTERS_ARB            0x88B0
#define GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB        0x88B1
#define GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB     0x88B2
#define GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB 0x88B3
#define GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB         0x88B4
#define GL_MAX_PROGRAM_ENV_PARAMETERS_ARB           0x88B5
#define GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB          0x88B6
#define GL_PROGRAM_STRING_ARB                       0x8628
#define GL_PROGRAM_ERROR_POSITION_ARB               0x864B
#define GL_CURRENT_MATRIX_ARB                       0x8641
#define GL_TRANSPOSE_CURRENT_MATRIX_ARB             0x88B7
#define GL_CURRENT_MATRIX_STACK_DEPTH_ARB           0x8640
#define GL_MAX_VERTEX_ATTRIBS_ARB                   0x8869
#define GL_MAX_PROGRAM_MATRICES_ARB                 0x862F
#define GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB       0x862E
#define GL_PROGRAM_ERROR_STRING_ARB                 0x8874
#define GL_MATRIX0_ARB                              0x88C0
#define GL_MATRIX1_ARB                              0x88C1
#define GL_MATRIX2_ARB                              0x88C2
#define GL_MATRIX3_ARB                              0x88C3
#define GL_MATRIX4_ARB                              0x88C4
#define GL_MATRIX5_ARB                              0x88C5
#define GL_MATRIX6_ARB                              0x88C6
#define GL_MATRIX7_ARB                              0x88C7
#define GL_MATRIX8_ARB                              0x88C8
#define GL_MATRIX9_ARB                              0x88C9
#define GL_MATRIX10_ARB                             0x88CA
#define GL_MATRIX11_ARB                             0x88CB
#define GL_MATRIX12_ARB                             0x88CC
#define GL_MATRIX13_ARB                             0x88CD
#define GL_MATRIX14_ARB                             0x88CE
#define GL_MATRIX15_ARB                             0x88CF
#define GL_MATRIX16_ARB                             0x88D0
#define GL_MATRIX17_ARB                             0x88D1
#define GL_MATRIX18_ARB                             0x88D2
#define GL_MATRIX19_ARB                             0x88D3
#define GL_MATRIX20_ARB                             0x88D4
#define GL_MATRIX21_ARB                             0x88D5
#define GL_MATRIX22_ARB                             0x88D6
#define GL_MATRIX23_ARB                             0x88D7
#define GL_MATRIX24_ARB                             0x88D8
#define GL_MATRIX25_ARB                             0x88D9
#define GL_MATRIX26_ARB                             0x88DA
#define GL_MATRIX27_ARB                             0x88DB
#define GL_MATRIX28_ARB                             0x88DC
#define GL_MATRIX29_ARB                             0x88DD
#define GL_MATRIX30_ARB                             0x88DE
#define GL_MATRIX31_ARB                             0x88DF


GL_EXT(_GL_ARB_fragment_program)
#define GL_FRAGMENT_PROGRAM_ARB                     0x8804
#define GL_PROGRAM_ALU_INSTRUCTIONS_ARB             0x8805
#define GL_PROGRAM_TEX_INSTRUCTIONS_ARB             0x8806
#define GL_PROGRAM_TEX_INDIRECTIONS_ARB             0x8807
#define GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB      0x8808
#define GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB      0x8809
#define GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB      0x880A
#define GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB         0x880B
#define GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB         0x880C
#define GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB         0x880D
#define GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB  0x880E
#define GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB  0x880F
#define GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB  0x8810
#define GL_MAX_TEXTURE_COORDS_ARB                   0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB              0x8872 


#define BUFFER_OFFSET(i) ((char *) NULL + (i))

GL_EXT(_GL_ARB_vertex_buffer_object)
GL_PROC(_GL_ARB_vertex_buffer_object,void,glBindBufferARB,(GLenum target, GLuint buffer))
GL_PROC(_GL_ARB_vertex_buffer_object,void,glDeleteBuffersARB,(GLsizei n, const GLuint *buffers))
GL_PROC(_GL_ARB_vertex_buffer_object,void,glGenBuffersARB,(GLsizei n, GLuint *buffers))
GL_PROC(_GL_ARB_vertex_buffer_object,GLboolean,glIsBufferARB,(GLuint buffer))
GL_PROC(_GL_ARB_vertex_buffer_object,void,glBufferDataARB,(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage))
GL_PROC(_GL_ARB_vertex_buffer_object,void,glBufferSubDataARB,(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data))
GL_PROC(_GL_ARB_vertex_buffer_object,void,glGetBufferSubDataARB,(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data))
GL_PROC(_GL_ARB_vertex_buffer_object,void*,glMapBufferARB,(GLenum target, GLenum access))
GL_PROC(_GL_ARB_vertex_buffer_object,GLboolean,glUnmapBufferARB,(GLenum target))
GL_PROC(_GL_ARB_vertex_buffer_object,void,glGetBufferParameterivARB,(GLenum target, GLenum pname, GLint *params))
GL_PROC(_GL_ARB_vertex_buffer_object,void,glGetBufferPointervARB,(GLenum target, GLenum pname, GLvoid **params))
#define GL_ARRAY_BUFFER_ARB                             0x8892
#define GL_ELEMENT_ARRAY_BUFFER_ARB                     0x8893
#define GL_ARRAY_BUFFER_BINDING_ARB                     0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB             0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING_ARB              0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING_ARB              0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING_ARB               0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING_ARB               0x8899
#define TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB          0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB           0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB     0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB      0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB              0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB       0x889F
#define GL_STREAM_DRAW_ARB                              0x88E0
#define GL_STREAM_READ_ARB                              0x88E1
#define GL_STREAM_COPY_ARB                              0x88E2
#define GL_STATIC_DRAW_ARB                              0x88E4
#define GL_STATIC_READ_ARB                              0x88E5
#define GL_STATIC_COPY_ARB                              0x88E6
#define GL_DYNAMIC_DRAW_ARB                             0x88E8
#define GL_DYNAMIC_READ_ARB                             0x88E9
#define GL_DYNAMIC_COPY_ARB                             0x88EA
#define GL_READ_ONLY_ARB                                0x88B8
#define GL_WRITE_ONLY_ARB                               0x88B9
#define GL_READ_WRITE_ARB                               0x88BA
#define GL_BUFFER_SIZE_ARB                              0x8764
#define GL_BUFFER_USAGE_ARB                             0x8765
#define GL_BUFFER_ACCESS_ARB                            0x88BB
#define GL_BUFFER_MAPPED_ARB                            0x88BC
#define GL_BUFFER_MAP_POINTER_ARB                       0x88BD 



/* NV_texture_shader */
GL_EXT(_GL_NV_texture_shader)
#define GL_OFFSET_TEXTURE_RECTANGLE_NV    0x864C
#define GL_OFFSET_TEXTURE_RECTANGLE_SCALE_NV 0x864D
#define GL_DOT_PRODUCT_TEXTURE_RECTANGLE_NV 0x864E
#define GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV 0x86D9
#define GL_UNSIGNED_INT_S8_S8_8_8_NV      0x86DA
#define GL_UNSIGNED_INT_8_8_S8_S8_REV_NV  0x86DB
#define GL_DSDT_MAG_INTENSITY_NV          0x86DC
#define GL_SHADER_CONSISTENT_NV           0x86DD
#define GL_TEXTURE_SHADER_NV              0x86DE
#define GL_SHADER_OPERATION_NV            0x86DF
#define GL_CULL_MODES_NV                  0x86E0
#define GL_OFFSET_TEXTURE_MATRIX_NV       0x86E1
#define GL_OFFSET_TEXTURE_SCALE_NV        0x86E2
#define GL_OFFSET_TEXTURE_BIAS_NV         0x86E3
#define GL_OFFSET_TEXTURE_2D_MATRIX_NV    GL_OFFSET_TEXTURE_MATRIX_NV
#define GL_OFFSET_TEXTURE_2D_SCALE_NV     GL_OFFSET_TEXTURE_SCALE_NV
#define GL_OFFSET_TEXTURE_2D_BIAS_NV      GL_OFFSET_TEXTURE_BIAS_NV
#define GL_PREVIOUS_TEXTURE_INPUT_NV      0x86E4
#define GL_CONST_EYE_NV                   0x86E5
#define GL_PASS_THROUGH_NV                0x86E6
#define GL_CULL_FRAGMENT_NV               0x86E7
#define GL_OFFSET_TEXTURE_2D_NV           0x86E8
#define GL_DEPENDENT_AR_TEXTURE_2D_NV     0x86E9
#define GL_DEPENDENT_GB_TEXTURE_2D_NV     0x86EA
#define GL_DOT_PRODUCT_NV                 0x86EC
#define GL_DOT_PRODUCT_DEPTH_REPLACE_NV   0x86ED
#define GL_DOT_PRODUCT_TEXTURE_2D_NV      0x86EE
#define GL_DOT_PRODUCT_TEXTURE_3D_NV      0x86EF
#define GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV 0x86F0
#define GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV 0x86F1
#define GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV 0x86F2
#define GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV 0x86F3
#define GL_HILO_NV                        0x86F4
#define GL_DSDT_NV                        0x86F5
#define GL_DSDT_MAG_NV                    0x86F6
#define GL_DSDT_MAG_VIB_NV                0x86F7
#define GL_HILO16_NV                      0x86F8
#define GL_SIGNED_HILO_NV                 0x86F9
#define GL_SIGNED_HILO16_NV               0x86FA
#define GL_SIGNED_RGBA_NV                 0x86FB
#define GL_SIGNED_RGBA8_NV                0x86FC
#define GL_SIGNED_RGB_NV                  0x86FE
#define GL_SIGNED_RGB8_NV                 0x86FF
#define GL_SIGNED_LUMINANCE_NV            0x8701
#define GL_SIGNED_LUMINANCE8_NV           0x8702
#define GL_SIGNED_LUMINANCE_ALPHA_NV      0x8703
#define GL_SIGNED_LUMINANCE8_ALPHA8_NV    0x8704
#define GL_SIGNED_ALPHA_NV                0x8705
#define GL_SIGNED_ALPHA8_NV               0x8706
#define GL_SIGNED_INTENSITY_NV            0x8707
#define GL_SIGNED_INTENSITY8_NV           0x8708
#define GL_DSDT8_NV                       0x8709
#define GL_DSDT8_MAG8_NV                  0x870A
#define GL_DSDT8_MAG8_INTENSITY8_NV       0x870B
#define GL_SIGNED_RGB_UNSIGNED_ALPHA_NV   0x870C
#define GL_SIGNED_RGB8_UNSIGNED_ALPHA8_NV 0x870D
#define GL_HI_SCALE_NV                    0x870E
#define GL_LO_SCALE_NV                    0x870F
#define GL_DS_SCALE_NV                    0x8710
#define GL_DT_SCALE_NV                    0x8711
#define GL_MAGNITUDE_SCALE_NV             0x8712
#define GL_VIBRANCE_SCALE_NV              0x8713
#define GL_HI_BIAS_NV                     0x8714
#define GL_LO_BIAS_NV                     0x8715
#define GL_DS_BIAS_NV                     0x8716
#define GL_DT_BIAS_NV                     0x8717
#define GL_MAGNITUDE_BIAS_NV              0x8718
#define GL_VIBRANCE_BIAS_NV               0x8719
#define GL_TEXTURE_BORDER_VALUES_NV       0x871A
#define GL_TEXTURE_HI_SIZE_NV             0x871B
#define GL_TEXTURE_LO_SIZE_NV             0x871C
#define GL_TEXTURE_DS_SIZE_NV             0x871D
#define GL_TEXTURE_DT_SIZE_NV             0x871E
#define GL_TEXTURE_MAG_SIZE_NV            0x871F

/* NV_texture_shader */
GL_EXT(_GL_NV_texture_shader2)
#define GL_DOT_PRODUCT_TEXTURE_3D_NV      0x86EF

GL_EXT(_GL_NV_texture_shader3)
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_NV         0x8850
#define GL_OFFSET_PROJECTIVE_TEXTURE_2D_SCALE_NV   0x8851
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_NV  0x8852
#define GL_OFFSET_PROJECTIVE_TEXTURE_RECTANGLE_SCALE_NV 0x8853
#define GL_OFFSET_HILO_TEXTURE_2D_NV               0x8854
#define GL_OFFSET_HILO_TEXTURE_RECTANGLE_NV        0x8855
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_2D_NV    0x8856
#define GL_OFFSET_HILO_PROJECTIVE_TEXTURE_RECTANGLE_NV 0x8857
#define GL_DEPENDENT_HILO_TEXTURE_2D_NV            0x8858
#define GL_DEPENDENT_RGB_TEXTURE_3D_NV             0x8859
#define GL_DEPENDENT_RGB_TEXTURE_CUBE_MAP_NV       0x885A
#define GL_DOT_PRODUCT_PASS_THROUGH_NV             0x885B
#define GL_DOT_PRODUCT_TEXTURE_1D_NV               0x885C
#define GL_DOT_PRODUCT_AFFINE_DEPTH_REPLACE_NV     0x885D

/* GL_EXT_texture3D */
GL_EXT(_GL_EXT_texture3D)
GL_PROC(_GL_EXT_texture3D,void,glTexImage3DEXT, (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *))
GL_PROC(_GL_EXT_texture3D,void,glTexSubImage3DEXT, (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *))
#define GL_PACK_SKIP_IMAGES               0x806B
#define GL_PACK_SKIP_IMAGES_EXT           0x806B
#define GL_PACK_IMAGE_HEIGHT              0x806C
#define GL_PACK_IMAGE_HEIGHT_EXT          0x806C
#define GL_UNPACK_SKIP_IMAGES             0x806D
#define GL_UNPACK_SKIP_IMAGES_EXT         0x806D
#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_UNPACK_IMAGE_HEIGHT_EXT        0x806E
#define GL_TEXTURE_3D                     0x806F
#define GL_TEXTURE_3D_EXT                 0x806F
#define GL_PROXY_TEXTURE_3D               0x8070
#define GL_PROXY_TEXTURE_3D_EXT           0x8070
#define GL_TEXTURE_DEPTH                  0x8071
#define GL_TEXTURE_DEPTH_EXT              0x8071
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_TEXTURE_WRAP_R_EXT             0x8072
#define GL_MAX_3D_TEXTURE_SIZE            0x8073
#define GL_MAX_3D_TEXTURE_SIZE_EXT        0x8073

GL_EXT(_GL_EXT_fog_coord)
GL_PROC(_GL_EXT_fog_coord,void,glFogCoordPointerEXT,(GLenum, GLsizei, const GLvoid *))
GL_PROC(_GL_EXT_fog_coord,void,glFogCoordfEXT,(GLfloat))
#define GL_FOG_COORDINATE_SOURCE_EXT      0x8450
#define GL_FOG_COORDINATE_EXT             0x8451
#define GL_FRAGMENT_DEPTH_EXT             0x8452
#define GL_CURRENT_FOG_COORDINATE_EXT     0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE_EXT  0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE_EXT 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER_EXT 0x8456
#define GL_FOG_COORDINATE_ARRAY_EXT       0x8457

GL_EXT(_GL_EXT_draw_range_elements)
GL_PROC(_GL_EXT_draw_range_elements,void,glDrawRangeElementsEXT,(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices))
#define GL_MAX_ELEMENTS_VERTICES_EXT 0x80E8
#define GL_MAX_ELEMENTS_INDICES_EXT  0x80E9

/* GL_SGIS_generate_mipmap */
GL_EXT(_GL_SGIS_generate_mipmap)
#define GL_GENERATE_MIPMAP_SGIS           0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS      0x8192

/* GL_SGIS_texture_lod */
GL_EXT(_GL_SGIS_texture_lod)
#define GL_TEXTURE_MIN_LOD_SGIS           0x813A
#define GL_TEXTURE_MAX_LOD_SGIS           0x813B
#define GL_TEXTURE_BASE_LEVEL_SGIS        0x813C
#define GL_TEXTURE_MAX_LEVEL_SGIS         0x813D

GL_EXT(_GL_HP_occlusion_test)
#define GL_OCCLUSION_TEST_HP              0x8165
#define GL_OCCLUSION_TEST_RESULT_HP       0x8166

GL_EXT(_GL_NV_occlusion_query)
GL_PROC(_GL_NV_occlusion_query,void,glGenOcclusionQueriesNV,(GLsizei n, GLuint *ids))
GL_PROC(_GL_NV_occlusion_query,void,glDeleteOcclusionQueriesNV,(GLsizei n, GLuint *ids))
GL_PROC(_GL_NV_occlusion_query,void,glIsOcclusionQueryNV,(GLuint id))
GL_PROC(_GL_NV_occlusion_query,void,glBeginOcclusionQueryNV,(GLuint id))
GL_PROC(_GL_NV_occlusion_query,void,glEndOcclusionQueryNV,(GLvoid))
GL_PROC(_GL_NV_occlusion_query,void,glGetOcclusionQueryivNV,(GLuint id, GLenum pname, GLint *params))
GL_PROC(_GL_NV_occlusion_query,void,glGetOcclusionQueryuivNV,(GLuint id, GLenum pname, GLint *params))
#define GL_PIXEL_COUNTER_BITS_NV          0x8864
#define GL_CURRENT_OCCLUSION_QUERY_ID_NV  0x8865
#define GL_PIXEL_COUNT_NV                 0x8866
#define GL_PIXEL_COUNT_AVAILABLE_NV       0x8867

GL_EXT(_GL_ARB_texture_env_combine)
#define GL_COMBINE_ARB						0x8570
#define GL_COMBINE_RGB_ARB					0x8571
#define GL_COMBINE_ALPHA_ARB				0x8572
#define GL_SOURCE0_RGB_ARB					0x8580
#define GL_SOURCE1_RGB_ARB					0x8581
#define GL_SOURCE2_RGB_ARB					0x8582
#define GL_SOURCE0_ALPHA_ARB				0x8588
#define GL_SOURCE1_ALPHA_ARB				0x8589
#define GL_SOURCE2_ALPHA_ARB				0x858A
#define GL_OPERAND0_RGB_ARB					0x8590
#define GL_OPERAND1_RGB_ARB					0x8591
#define GL_OPERAND2_RGB_ARB					0x8592
#define GL_OPERAND0_ALPHA_ARB				0x8598
#define GL_OPERAND1_ALPHA_ARB				0x8599
#define GL_OPERAND2_ALPHA_ARB				0x859A
#define GL_RGB_SCALE_ARB					0x8573
#define GL_ADD_SIGNED_ARB					0x8574
#define GL_INTERPOLATE_ARB					0x8575
#define GL_CONSTANT_ARB						0x8576
#define GL_PRIMARY_COLOR_ARB				0x8577
#define GL_PREVIOUS_ARB						0x8578
#define GL_SUBTRACT_ARB						0x84E7


GL_EXT(_GL_ATI_separate_stencil)
GL_PROC(_GL_ATI_separate_stencil,GLvoid,glStencilFuncSeparateATI,(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask))
GL_PROC(_GL_ATI_separate_stencil,GLvoid,glStencilOpSeparateATI,(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass))
#define GL_STENCIL_BACK_FUNC_ATI             0x8800
#define GL_STENCIL_BACK_FAIL_ATI             0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI  0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI  0x8803

GL_EXT(_GL_EXT_stencil_two_side)
GL_PROC(_GL_EXT_stencil_two_side,GLvoid,glActiveStencilFaceEXT,(GLenum face))
#define GL_STENCIL_TEST_TWO_SIDE_EXT         0x8910
#define GL_ACTIVE_STENCIL_FACE_EXT           0x8911

GL_EXT(_GL_EXT_stencil_wrap)
#define GL_INCR_WRAP_EXT                  0x8507
#define GL_DECR_WRAP_EXT                  0x8508
 
GL_EXT(_GL_ATI_fragment_shader)
GL_PROC(_GL_ATI_fragment_shader,GLuint,glGenFragmentShadersATI,(GLuint range))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glBindFragmentShaderATI,(GLuint id))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glDeleteFragmentShaderATI,(GLuint id))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glBeginFragmentShaderATI,(GLvoid))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glEndFragmentShaderATI,(GLvoid))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glPassTexCoordATI,(GLuint dst, GLuint coord, GLenum swizzle))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glSampleMapATI,(GLuint dst, GLuint interp, GLenum swizzle))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glColorFragmentOp1ATI,(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glColorFragmentOp2ATI,(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glColorFragmentOp3ATI,(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glAlphaFragmentOp1ATI,(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glAlphaFragmentOp2ATI,(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glAlphaFragmentOp3ATI,(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod))
GL_PROC(_GL_ATI_fragment_shader,GLvoid,glSetFragmentShaderConstantATI,(GLuint dst, const GLfloat *value))
#define GL_FRAGMENT_SHADER_ATI			0x8920
#define GL_REG_0_ATI								0x8921
#define GL_REG_1_ATI								0x8922
#define GL_REG_2_ATI								0x8923
#define GL_REG_3_ATI								0x8924
#define GL_REG_4_ATI								0x8925
#define GL_REG_5_ATI								0x8926
#define GL_REG_6_ATI								0x8927
#define GL_REG_7_ATI								0x8928
#define GL_REG_8_ATI								0x8929
#define GL_REG_9_ATI								0x892A
#define GL_REG_10_ATI								0x892B
#define GL_REG_11_ATI								0x892C
#define GL_REG_12_ATI								0x892D
#define GL_REG_13_ATI								0x892E
#define GL_REG_14_ATI								0x892F
#define GL_REG_15_ATI								0x8930
#define GL_REG_16_ATI								0x8931
#define GL_REG_17_ATI								0x8932
#define GL_REG_18_ATI								0x8933
#define GL_REG_19_ATI								0x8934
#define GL_REG_20_ATI								0x8935
#define GL_REG_21_ATI								0x8936
#define GL_REG_22_ATI								0x8937
#define GL_REG_23_ATI								0x8938
#define GL_REG_24_ATI								0x8939
#define GL_REG_25_ATI								0x893A
#define GL_REG_26_ATI								0x893B
#define GL_REG_27_ATI								0x893C
#define GL_REG_28_ATI								0x893D
#define GL_REG_29_ATI								0x893E
#define GL_REG_30_ATI								0x893F
#define GL_REG_31_ATI								0x8940
#define GL_CON_0_ATI								0x8941
#define GL_CON_1_ATI								0x8942
#define GL_CON_2_ATI								0x8943
#define GL_CON_3_ATI								0x8944
#define GL_CON_4_ATI								0x8945
#define GL_CON_5_ATI								0x8946
#define GL_CON_6_ATI								0x8947
#define GL_CON_7_ATI								0x8948
#define GL_CON_8_ATI								0x8949
#define GL_CON_9_ATI								0x894A
#define GL_CON_10_ATI								0x894B
#define GL_CON_11_ATI								0x894C
#define GL_CON_12_ATI								0x894D
#define GL_CON_13_ATI								0x894E
#define GL_CON_14_ATI								0x894F
#define GL_CON_15_ATI								0x8950
#define GL_CON_16_ATI								0x8951
#define GL_CON_17_ATI								0x8952
#define GL_CON_18_ATI								0x8953
#define GL_CON_19_ATI								0x8954
#define GL_CON_20_ATI								0x8955
#define GL_CON_21_ATI								0x8956
#define GL_CON_22_ATI								0x8957
#define GL_CON_23_ATI								0x8958
#define GL_CON_24_ATI								0x8959
#define GL_CON_25_ATI								0x895A
#define GL_CON_26_ATI								0x895B
#define GL_CON_27_ATI								0x895C
#define GL_CON_28_ATI								0x895D
#define GL_CON_29_ATI								0x895E
#define GL_CON_30_ATI								0x895F
#define GL_CON_31_ATI								0x8960
#define GL_MOV_ATI									0x8961
#define GL_ADD_ATI									0x8963
#define GL_MUL_ATI									0x8964
#define GL_SUB_ATI									0x8965
#define GL_DOT3_ATI									0x8966
#define GL_DOT4_ATI									0x8967
#define GL_MAD_ATI									0x8968
#define GL_LERP_ATI									0x8969
#define GL_CND_ATI									0x896A
#define GL_CND0_ATI									0x896B
#define GL_DOT2_ADD_ATI							0x896C
#define GL_SECONDARY_INTERPOLATOR_ATI				0x896D
#define GL_NUM_FRAGMENT_REGISTERS_ATI				0x896E
#define GL_NUM_FRAGMENT_CONSTANTS_ATI				0x896F
#define GL_NUM_PASSES_ATI					      		0x8970
#define GL_NUM_INSTRUCTIONS_PER_PASS_ATI		0x8971
#define GL_NUM_INSTRUCTIONS_TOTAL_ATI				0x8972
#define GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI	0x8973
#define GL_NUM_LOOPBACK_COMPONENTS_ATI			0x8974
#define GL_COLOR_ALPHA_PAIRING_ATI					0x8975
#define GL_SWIZZLE_STR_ATI							    0x8976
#define GL_SWIZZLE_STQ_ATI						  	  0x8977
#define GL_SWIZZLE_STR_DR_ATI						    0x8978
#define GL_SWIZZLE_STQ_DQ_ATI						    0x8979
#define GL_SWIZZLE_STRQ_ATI							    0x897A
#define GL_SWIZZLE_STRQ_DQ_ATI						  0x897B
#define GL_RED_BIT_ATI								      0x00000001
#define GL_GREEN_BIT_ATI							      0x00000002
#define GL_BLUE_BIT_ATI								      0x00000004
#define GL_2X_BIT_ATI								        0x00000001
#define GL_4X_BIT_ATI								        0x00000002
#define GL_8X_BIT_ATI								        0x00000004
#define GL_HALF_BIT_ATI								      0x00000008
#define GL_QUARTER_BIT_ATI							    0x00000010
#define GL_EIGHTH_BIT_ATI							      0x00000020
#define GL_SATURATE_BIT_ATI							    0x00000040
#define GL_COMP_BIT_ATI								      0x00000002
#define GL_NEGATE_BIT_ATI							      0x00000004
#define GL_BIAS_BIT_ATI								      0x00000008


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

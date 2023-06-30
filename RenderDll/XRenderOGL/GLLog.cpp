/*=============================================================================
  GLLog.cpp : implementation of the Logging OpenGL functions.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/


#include "RenderPCH.h"
#include "GL_Renderer.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

//======================================================================

// GL functions implement.
#define GL_EXT(name) 
#define GL_PROC(ext,ret,func,parms) ret (__stdcall *t##func)parms;
#include "GLFuncs.h"
#undef GL_EXT
#undef GL_PROC

/*#define GL_EXT(name) 
#define GL_PROC(ext,ret,func,parms) ret l##func parms;
{\
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", #func);\
  return t##func parms; \
}\

#include "GLFuncs.h"
#undef GL_EXT
#undef GL_PROC*/

#define ecase(e) case e: return #e

static char sStr[1024];
#define edefault(e) default: sprintf(sStr, "0x%x", e); return sStr;

static char *senFace(GLenum e)
{
  switch (e)
  {
    ecase (GL_FRONT);
    ecase (GL_BACK);
    ecase (GL_FRONT_AND_BACK);
    edefault (e);
  }
}

static char *senProgramARB(GLenum e)
{
  switch (e)
  {
    ecase (GL_VERTEX_PROGRAM_ARB);
    ecase (GL_FRAGMENT_PROGRAM_ARB);
    edefault (e);
  }
}

static char *senArrayBufferARB(GLenum e)
{
  switch (e)
  {
    ecase (GL_ELEMENT_ARRAY_BUFFER_ARB);
    ecase (GL_ARRAY_BUFFER_ARB);
    edefault (e);
  }
}

static char *senMaterial(GLenum e)
{
  switch (e)
  {
    ecase (GL_SHININESS);
    edefault (e);
  }
}

static char *senFog(GLenum e)
{
  switch (e)
  {
    ecase (GL_FOG_COLOR);
    ecase (GL_FOG_END);
    ecase (GL_FOG_START);
    edefault (e);
  }
}

static char *senMaterialv(GLenum e)
{
  switch (e)
  {
    ecase (GL_AMBIENT);
    ecase (GL_DIFFUSE);
    ecase (GL_SPECULAR);
    ecase (GL_EMISSION);
    edefault (e);
  }
}

static char *senLightv(GLenum e)
{
  switch (e)
  {
    ecase (GL_POSITION);
    ecase (GL_AMBIENT);
    ecase (GL_DIFFUSE);
    ecase (GL_SPECULAR);
    edefault (e);
  }
}

static char *senLight(GLenum e)
{
  switch (e)
  {
    ecase (GL_CONSTANT_ATTENUATION);
    ecase (GL_LINEAR_ATTENUATION);
    ecase (GL_QUADRATIC_ATTENUATION);
    edefault (e);
  }
}

static char *senStencilFunc(GLenum e)
{
  switch (e)
  {
    ecase (GL_NEVER);
    ecase (GL_LESS);
    ecase (GL_EQUAL);
    ecase (GL_LEQUAL);
    ecase (GL_GREATER);
    ecase (GL_NOTEQUAL);
    ecase (GL_GEQUAL);
    ecase (GL_ALWAYS);
    edefault (e);
  }
}

static char *senStencilOp(GLenum e)
{
  switch (e)
  {
    ecase (GL_ZERO);
    ecase (GL_KEEP);
    ecase (GL_REPLACE);
    ecase (GL_INCR);
    ecase (GL_DECR);
    ecase (GL_INCR_WRAP_EXT);
    ecase (GL_DECR_WRAP_EXT);
    edefault (e);
  }
}

static char *senNewList(GLenum e)
{
  switch (e)
  {
    ecase (GL_COMPILE);
    ecase (GL_COMPILE_AND_EXECUTE);
    edefault (e);
  }
}

static char *senMatrixMode(GLenum e)
{
  switch (e)
  {
    ecase (GL_MODELVIEW);
    ecase (GL_PROJECTION);
    ecase (GL_TEXTURE);
    edefault (e);
  }
}

static char *senPolyType(GLenum e)
{
  switch (e)
  {
    ecase (GL_POINTS);
    ecase (GL_LINES);
    ecase (GL_LINE_LOOP);
    ecase (GL_LINE_STRIP);
    ecase (GL_TRIANGLES);
    ecase (GL_TRIANGLE_STRIP);
    ecase (GL_TRIANGLE_FAN);
    ecase (GL_QUADS);
    ecase (GL_QUAD_STRIP);
    ecase (GL_POLYGON);
    edefault (e);
  }
}

static char *senDataType(GLenum e)
{
  switch (e)
  {
    ecase (GL_BYTE);
    ecase (GL_UNSIGNED_BYTE);
    ecase (GL_SHORT);
    ecase (GL_UNSIGNED_SHORT);
    ecase (GL_INT);
    ecase (GL_UNSIGNED_INT);
    ecase (GL_FLOAT);
    ecase (GL_DOUBLE);
    edefault (e);
  }
}

static char *senAlphaFunction(GLenum e)
{
  switch (e)
  {
    ecase (GL_NEVER);
    ecase (GL_LESS);
    ecase (GL_EQUAL);
    ecase (GL_LEQUAL);
    ecase (GL_GREATER);
    ecase (GL_NOTEQUAL);
    ecase (GL_GEQUAL);
    ecase (GL_ALWAYS);
    edefault (e);
  }
}

static char *senBlendingFactor(GLenum e)
{
  switch (e)
  {
    ecase (GL_ZERO);
    ecase (GL_ONE);
    ecase (GL_SRC_COLOR);
    ecase (GL_ONE_MINUS_SRC_COLOR);
    ecase (GL_SRC_ALPHA);
    ecase (GL_ONE_MINUS_SRC_ALPHA);
    ecase (GL_DST_ALPHA);
    ecase (GL_ONE_MINUS_DST_ALPHA);
    ecase (GL_DST_COLOR);
    ecase (GL_ONE_MINUS_DST_COLOR);
    ecase (GL_SRC_ALPHA_SATURATE);
    edefault (e);
  }
}

static char *senAccumOp(GLenum e)
{
  switch (e)
  {
    ecase (GL_ACCUM);
    ecase (GL_LOAD);
    ecase (GL_RETURN);
    ecase (GL_MULT);
    ecase (GL_ADD);
    edefault (e);
  }
}

static char *senClientState(GLenum e)
{
  switch (e)
  {
    ecase (GL_VERTEX_ARRAY);
    ecase (GL_NORMAL_ARRAY);
    ecase (GL_COLOR_ARRAY);
    ecase (GL_TEXTURE_COORD_ARRAY);
    ecase (GL_VERTEX_ARRAY_RANGE_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY0_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY1_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY2_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY3_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY4_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY5_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY6_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY7_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY8_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY9_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY10_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY11_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY12_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY13_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY14_NV);
    ecase (GL_VERTEX_ATTRIB_ARRAY15_NV);
    edefault (e);
  }
}

static char *senCullFace(GLenum e)
{
  switch (e)
  {
    ecase (GL_FRONT);
    ecase (GL_BACK);
    ecase (GL_FRONT_AND_BACK);
    edefault (e);
  }
}

static char *senTextureEnvTarget(GLenum e)
{
  switch (e)
  {
    ecase (GL_TEXTURE_ENV);
    ecase (GL_TEXTURE_SHADER_NV);
    edefault (e);
  }
}

static char *senTextureEnvPName(GLenum e)
{
  switch (e)
  {
    ecase (GL_TEXTURE_ENV_MODE);
    ecase (GL_COMBINE_RGB_EXT);
    ecase (GL_COMBINE_ALPHA_EXT);
    ecase (GL_SOURCE0_RGB_EXT);
    ecase (GL_SOURCE1_RGB_EXT);
    ecase (GL_SOURCE2_RGB_EXT);
    ecase (GL_SOURCE3_RGB_NV);
    ecase (GL_SOURCE0_ALPHA_EXT);
    ecase (GL_SOURCE1_ALPHA_EXT);
    ecase (GL_SOURCE2_ALPHA_EXT);
    ecase (GL_SOURCE3_ALPHA_NV);
    ecase (GL_OPERAND0_RGB_EXT);
    ecase (GL_OPERAND1_RGB_EXT);
    ecase (GL_OPERAND2_RGB_EXT);
    ecase (GL_OPERAND3_RGB_NV);
    ecase (GL_OPERAND0_ALPHA_EXT);
    ecase (GL_OPERAND1_ALPHA_EXT);
    ecase (GL_OPERAND2_ALPHA_EXT);
    ecase (GL_OPERAND3_ALPHA_NV);
    ecase (GL_RGB_SCALE_EXT);
    ecase (GL_ALPHA_SCALE);
    ecase (GL_SHADER_OPERATION_NV);
    edefault (e);
  }
}

static char *senTextureEnvfPName(GLenum e)
{
  switch (e)
  {
    ecase (GL_TEXTURE_ENV_COLOR);
    edefault (e);
  }
}

static char *senTextureEnvParam(GLenum e)
{
  switch (e)
  {
    ecase (GL_MODULATE);
    ecase (GL_NONE);
    ecase (GL_COMBINE_ALPHA_EXT);
    ecase (GL_REPLACE);
    ecase (GL_DECAL);
    ecase (GL_ADD);
    ecase (GL_COMBINE_EXT);
    ecase (GL_ADD_SIGNED_EXT);
    ecase (GL_COMBINE4_NV);
    ecase (GL_TEXTURE);
    ecase (GL_SRC_COLOR);
    ecase (GL_PRIMARY_COLOR_EXT);
    ecase (GL_SRC_ALPHA);
    ecase (GL_ONE_MINUS_SRC_ALPHA);
    ecase (GL_TEXTURE0_ARB);
    ecase (GL_TEXTURE1_ARB);
    ecase (GL_TEXTURE2_ARB);
    ecase (GL_TEXTURE3_ARB);
    ecase (GL_TEXTURE4_ARB);
    ecase (GL_TEXTURE5_ARB);
    ecase (GL_TEXTURE6_ARB);
    ecase (GL_TEXTURE7_ARB);
    ecase (GL_ONE_MINUS_SRC_COLOR);
    ecase (GL_PREVIOUS_EXT);
    ecase (GL_CONSTANT_EXT);
    ecase (GL_ONE);
    ecase (GL_CULL_FRAGMENT_NV);
    edefault (e);
  }
}

static char *senFenceContition(GLenum e)
{
  switch (e)
  {
    ecase (GL_ALL_COMPLETED_NV);
    edefault (e);
  }
}


static char *senTexFormat(GLenum e)
{
  switch (e)
  {
    ecase (GL_BGRA_EXT);
    ecase (GL_BGR_EXT);
    ecase (GL_RGBA);
    ecase (GL_RGB);
    ecase (GL_RGB8);
    ecase (GL_ALPHA);
    ecase (GL_RGBA4);
    ecase (GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
    ecase (GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
    ecase (GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
    edefault (e);
  }
}

static char *senTexDataType(GLenum e)
{
  switch (e)
  {
    ecase (GL_UNSIGNED_BYTE);
    ecase (GL_BYTE);
    ecase (GL_SHORT);
    ecase (GL_UNSIGNED_SHORT);
    ecase (GL_INT);
    ecase (GL_UNSIGNED_INT);
    ecase (GL_FLOAT);
    edefault (e);
  }
}

static char *senTexTarg(GLenum e)
{
  switch (e)
  {
    ecase (GL_TEXTURE_1D);
    ecase (GL_TEXTURE_2D);
    ecase (GL_TEXTURE_3D);
    ecase (GL_TEXTURE_CUBE_MAP_EXT);
    ecase (GL_TEXTURE_RECTANGLE_NV);
    ecase (GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT);
    ecase (GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT);
    ecase (GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT);
    ecase (GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT);
    ecase (GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT);
    ecase (GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT);
    edefault (e);
  }
}

static char *senTexParmName(GLenum e)
{
  switch (e)
  {
    ecase (GL_TEXTURE_MIN_FILTER);
    ecase (GL_TEXTURE_MAG_FILTER);
    ecase (GL_TEXTURE_MAX_ANISOTROPY_EXT);
    ecase (GL_TEXTURE_WRAP_S);
    ecase (GL_TEXTURE_WRAP_T);
    ecase (GL_TEXTURE_WRAP_R);
    ecase (GL_GENERATE_MIPMAP_SGIS);
    edefault (e);
  }
}

static char *senTexParm(GLenum e)
{
  switch (e)
  {
    ecase (GL_CLAMP_TO_EDGE);
    ecase (GL_REPEAT);
    ecase (GL_LINEAR);
    ecase (GL_LINEAR_MIPMAP_NEAREST);
    ecase (GL_LINEAR_MIPMAP_LINEAR);
    ecase (GL_NEAREST_MIPMAP_NEAREST);
    ecase (GL_NEAREST);
    ecase (GL_TEXTURE_MAX_ANISOTROPY_EXT);
    ecase (GL_TEXTURE_WRAP_S);
    ecase (GL_TEXTURE_WRAP_T);
    ecase (GL_TEXTURE_WRAP_R);
    edefault (e);
  }
}

static char *senDisEn(GLenum e)
{
  switch (e)
  {
    ecase (GL_FRAGMENT_PROGRAM_ARB);
    ecase (GL_FOG);
    ecase (GL_TEXTURE_GEN_S);
    ecase (GL_TEXTURE_GEN_T);
    ecase (GL_TEXTURE_GEN_R);
    ecase (GL_TEXTURE_GEN_Q);
    ecase (GL_LIGHTING);
    ecase (GL_TEXTURE_1D);
    ecase (GL_TEXTURE_2D);
    ecase (GL_LINE_STIPPLE);
    ecase (GL_POLYGON_STIPPLE);
    ecase (GL_CULL_FACE);
    ecase (GL_ALPHA_TEST);
    ecase (GL_BLEND);
    ecase (GL_INDEX_LOGIC_OP);
    ecase (GL_COLOR_LOGIC_OP);
    ecase (GL_DITHER);
    ecase (GL_STENCIL_TEST);
    ecase (GL_DEPTH_TEST);
    ecase (GL_CLIP_PLANE0);
    ecase (GL_CLIP_PLANE1);
    ecase (GL_CLIP_PLANE2);
    ecase (GL_CLIP_PLANE3);
    ecase (GL_CLIP_PLANE4);
    ecase (GL_CLIP_PLANE5);
    ecase (GL_LIGHT0);
    ecase (GL_LIGHT1);
    ecase (GL_LIGHT2);
    ecase (GL_LIGHT3);
    ecase (GL_LIGHT4);
    ecase (GL_LIGHT5);
    ecase (GL_LIGHT6);
    ecase (GL_LIGHT7);
    ecase (GL_POINT_SMOOTH);
    ecase (GL_LINE_SMOOTH);
    ecase (GL_SCISSOR_TEST);
    ecase (GL_COLOR_MATERIAL);
    ecase (GL_VERTEX_ARRAY);
    ecase (GL_NORMAL_ARRAY);
    ecase (GL_NORMALIZE);
    ecase (GL_COLOR_ARRAY);
    ecase (GL_TEXTURE_COORD_ARRAY);
    ecase (GL_POLYGON_OFFSET_FILL);
    ecase (GL_POLYGON_OFFSET_LINE);
    ecase (GL_REGISTER_COMBINERS_NV);
    ecase (GL_VERTEX_PROGRAM_NV);
    ecase (GL_VERTEX_STATE_PROGRAM_NV);
    ecase (GL_PER_STAGE_CONSTANTS_NV);
    ecase (GL_TEXTURE_SHADER_NV);
    ecase (GL_TEXTURE_CUBE_MAP_EXT);
    ecase (GL_FRAGMENT_SHADER_ATI);
    ecase (GL_STENCIL_TEST_TWO_SIDE_EXT);
    edefault (e);
  }
}

static char *senActiveTextureARB(GLenum e)
{
  switch (e)
  {
    ecase (GL_TEXTURE0_ARB);
    ecase (GL_TEXTURE1_ARB);
    ecase (GL_TEXTURE2_ARB);
    ecase (GL_TEXTURE3_ARB);
    ecase (GL_TEXTURE4_ARB);
    ecase (GL_TEXTURE5_ARB);
    ecase (GL_TEXTURE6_ARB);
    ecase (GL_TEXTURE7_ARB);
    edefault (e);
  }
}

void __stdcall lglAccum (GLenum Parm0, GLfloat Parm1)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %f)\n", "glAccum", senAccumOp(Parm0), Parm1);
  tglAccum(Parm0, Parm1);
}
void __stdcall lglAlphaFunc (GLenum Parm0, GLclampf Parm1)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %.3f)\n", "glAlphaFunc", senAlphaFunction(Parm0), (float)Parm1);
  tglAlphaFunc(Parm0, Parm1);
}
GLboolean __stdcall lglAreTexturesResident (GLsizei Parm0, const GLuint* Parm1, GLboolean* Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glAreTexturesResident");
  return tglAreTexturesResident(Parm0, Parm1, Parm2);
}
void __stdcall lglArrayElement (GLint Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (0x%x)\n", "glArrayElement", Parm0);
  tglArrayElement(Parm0);
}
void __stdcall lglBegin (GLenum Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "glBegin", senPolyType(Parm0));
  tglBegin(Parm0);
}
void __stdcall lglBindTexture (GLenum Parm0, GLuint Parm1)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, 0x%x)\n", "glBindTexture", senTexTarg(Parm0), Parm1);
  tglBindTexture(Parm0, Parm1);
}
void __stdcall lglBitmap (GLsizei Parm0, GLsizei Parm1, GLfloat Parm2, GLfloat Parm3, GLfloat Parm4, GLfloat Parm5, const GLubyte* Parm6)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glBitmap");
  tglBitmap(Parm0, Parm1, Parm2, Parm3, Parm4, Parm5, Parm6);
}
void __stdcall lglBlendFunc (GLenum Parm0, GLenum Parm1)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s)\n", "glBlendFunc", senBlendingFactor(Parm0), senBlendingFactor(Parm1));
  tglBlendFunc(Parm0, Parm1);
}
void __stdcall lglCallList (GLuint Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (0x%x)\n", "glCallList", Parm0);
  tglCallList(Parm0);
}
void __stdcall lglCallLists (GLsizei Parm0, GLenum Parm1, const GLvoid* Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCallLists");
  tglCallLists(Parm0, Parm1, Parm2);
}
void __stdcall lglClear (GLbitfield Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (0x%x)\n", "glClear", Parm0);
  tglClear(Parm0);
}
void __stdcall lglClearAccum (GLfloat Parm0, GLfloat Parm1, GLfloat Parm2, GLfloat Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f, %f, %f)\n", "glClearAccum", Parm0, Parm1, Parm2, Parm3);
  tglClearAccum(Parm0, Parm1, Parm2, Parm3);
}
void __stdcall lglClearColor (GLclampf Parm0, GLclampf Parm1, GLclampf Parm2, GLclampf Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f, %f, %f)\n", "glClearColor", Parm0, Parm1, Parm2, Parm3);
  tglClearColor(Parm0, Parm1, Parm2, Parm3);
}
void __stdcall lglClearDepth (GLclampd Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%.3f)\n", "glClearDepth", Parm0);
  tglClearDepth(Parm0);
}
void __stdcall lglClearIndex (GLfloat Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glClearIndex");
  tglClearIndex(Parm0);
}
void __stdcall lglClearStencil (GLint Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glClearStencil");
  tglClearStencil(Parm0);
}
void __stdcall lglClipPlane (GLenum Parm0, const GLdouble* Parm1)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glClipPlane");
  tglClipPlane(Parm0, Parm1);
}
void __stdcall lglColor3b (GLbyte Parm0, GLbyte Parm1, GLbyte Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3b");
  tglColor3b(Parm0, Parm1, Parm2);
}
void __stdcall lglColor3bv (const GLbyte* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3bv");
  tglColor3bv(Parm0);
}
void __stdcall lglColor3d (GLdouble Parm0, GLdouble Parm1, GLdouble Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3d");
  tglColor3d(Parm0, Parm1, Parm2);
}
void __stdcall lglColor3dv (const GLdouble* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3dv");
  tglColor3dv(Parm0);
}
void __stdcall lglColor3f (GLfloat Parm0, GLfloat Parm1, GLfloat Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f, %f)\n", "glColor3f", Parm0, Parm1, Parm2);
  tglColor3f(Parm0, Parm1, Parm2);
}
void __stdcall lglColor3fv (const GLfloat* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3fv");
  tglColor3fv(Parm0);
}
void __stdcall lglColor3i (GLint Parm0, GLint Parm1, GLint Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3i");
  tglColor3i(Parm0, Parm1, Parm2);
}
void __stdcall lglColor3iv (const GLint* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3iv");
  tglColor3iv(Parm0);
}
void __stdcall lglColor3s (GLshort Parm0, GLshort Parm1, GLshort Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3s");
  tglColor3s(Parm0, Parm1, Parm2);
}
void __stdcall lglColor3sv (const GLshort* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3sv");
  tglColor3sv(Parm0);
}
void __stdcall lglColor3ub (GLubyte Parm0, GLubyte Parm1, GLubyte Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3ub");
  tglColor3ub(Parm0, Parm1, Parm2);
}
void __stdcall lglColor3ubv (const GLubyte* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3ubv");
  tglColor3ubv(Parm0);
}
void __stdcall lglColor3ui (GLuint Parm0, GLuint Parm1, GLuint Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3ui");
  tglColor3ui(Parm0, Parm1, Parm2);
}
void __stdcall lglColor3uiv (const GLuint* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3uiv");
  tglColor3uiv(Parm0);
}
void __stdcall lglColor3us (GLushort Parm0, GLushort Parm1, GLushort Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3us");
  tglColor3us(Parm0, Parm1, Parm2);
}
void __stdcall lglColor3usv (const GLushort* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor3usv");
  tglColor3usv(Parm0);
}
void __stdcall lglColor4b (GLbyte Parm0, GLbyte Parm1, GLbyte Parm2, GLbyte Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4b");
  tglColor4b(Parm0, Parm1, Parm2, Parm3);
}
void __stdcall lglColor4bv (const GLbyte* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4bv");
  tglColor4bv(Parm0);
}
void __stdcall lglColor4d (GLdouble Parm0, GLdouble Parm1, GLdouble Parm2, GLdouble Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4d");
  tglColor4d(Parm0, Parm1, Parm2, Parm3);
}
void __stdcall lglColor4dv (const GLdouble* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4dv");
  tglColor4dv(Parm0);
}
void __stdcall lglColor4f (GLfloat Parm0, GLfloat Parm1, GLfloat Parm2, GLfloat Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f, %f, %f)\n", "glColor4f", Parm0, Parm1, Parm2, Parm3);
  tglColor4f(Parm0, Parm1, Parm2, Parm3);
}
void __stdcall lglColor4fv (const GLfloat* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4fv");
  tglColor4fv(Parm0);
}
void __stdcall lglColor4i (GLint Parm0, GLint Parm1, GLint Parm2, GLint Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4i");
  tglColor4i(Parm0, Parm1, Parm2, Parm3);
}
void __stdcall lglColor4iv (const GLint* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4iv");
  tglColor4iv(Parm0);
}
void __stdcall lglColor4s (GLshort Parm0, GLshort Parm1, GLshort Parm2, GLshort Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4s");
  tglColor4s(Parm0, Parm1, Parm2, Parm3);
}
void __stdcall lglColor4sv (const GLshort* Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4sv");
  tglColor4sv(Parm0);
}
void __stdcall lglColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4ub");
  tglColor4ub(red, green, blue, alpha);
}
void __stdcall lglColor4ubv (const GLubyte *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4ubv");
  tglColor4ubv(v);
}
void __stdcall lglColor4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4ui");
  tglColor4ui(red, green, blue, alpha);
}
void __stdcall lglColor4uiv (const GLuint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4uiv");
  tglColor4uiv(v);
}
void __stdcall lglColor4us (GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4us");
  tglColor4us(red, green, blue, alpha);
}
void __stdcall lglColor4usv (const GLushort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColor4usv");
  tglColor4usv(v);
}
void __stdcall lglColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %d, %d, %d)\n", "glColorMask", red, green, blue, alpha);
  tglColorMask(red, green, blue, alpha);
}
void __stdcall lglColorMaterial (GLenum face, GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColorMaterial");
  tglColorMaterial(face, mode);
}
void __stdcall lglColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %s, %d, 0x%x)\n", "glColorPointer", size, senDataType(type), stride, pointer);
  tglColorPointer(size, type, stride, pointer);
}
void __stdcall lglCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCopyPixels");
  tglCopyPixels(x, y, width, height, type);
}
void __stdcall lglCopyTexImage1D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCopyTexImage1D");
  tglCopyTexImage1D(target, level, internalFormat, x, y, width, border);
}
void __stdcall lglCopyTexImage2D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCopyTexImage2D");
  tglCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);
}
void __stdcall lglCopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCopyTexSubImage1D");
  tglCopyTexSubImage1D(target, level, xoffset, x, y, width);
}
void __stdcall lglCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCopyTexSubImage2D");
  tglCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}
void __stdcall lglCullFace (GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "glCullFace", senCullFace(mode));
  tglCullFace(mode);
}
void __stdcall lglDeleteLists (GLuint list, GLsizei range)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDeleteLists");
  tglDeleteLists(list, range);
}
void __stdcall lglDeleteTextures (GLsizei n, const GLuint *textures)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, MEM)\n", "glDeleteTextures", n);
  tglDeleteTextures(n, textures);
}
void __stdcall lglDepthFunc (GLenum func)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "glDepthFunc", senAlphaFunction(func));
  tglDepthFunc(func);
}
void __stdcall lglDepthMask (GLboolean flag)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d)\n", "glDepthMask", flag);
  tglDepthMask(flag);
}
void __stdcall lglDepthRange (GLclampd zNear, GLclampd zFar)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f)\n", "glDepthRange", zNear, zFar);
  tglDepthRange(zNear, zFar);
}
void __stdcall lglDisable (GLenum cap)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "glDisable", senDisEn(cap));
  tglDisable(cap);
}
void __stdcall lglDisableClientState (GLenum array)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "glDisableClientState", senClientState(array));
  tglDisableClientState(array);
}
void __stdcall lglDrawArrays (GLenum mode, GLint first, GLsizei count)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDrawArrays");
  tglDrawArrays(mode, first, count);
}
void __stdcall lglDrawBuffer (GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDrawBuffer");
  tglDrawBuffer(mode);
}
void __stdcall lglDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d, %s, 0x%x)\n", "glDrawElements", senPolyType(mode), count, senDataType(type), indices);
  tglDrawElements(mode, count, type, indices);
}
void __stdcall lglDrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDrawPixels");
  tglDrawPixels(width, height, format, type, pixels);
}
void __stdcall lglEdgeFlag (GLboolean flag)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEdgeFlag");
  tglEdgeFlag(flag);
}
void __stdcall lglEdgeFlagPointer (GLsizei stride, const GLvoid *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEdgeFlagPointer");
  tglEdgeFlagPointer(stride, pointer);
}
void __stdcall lglEdgeFlagv (const GLboolean *flag)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEdgeFlagv");
  tglEdgeFlagv(flag);
}
void __stdcall lglEnable (GLenum cap)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "glEnable", senDisEn(cap));
  tglEnable(cap);
}
void __stdcall lglEnableClientState (GLenum array)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "glEnableClientState", senClientState(array));
  tglEnableClientState(array);
}
void __stdcall lglEnd (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEnd");
  tglEnd();
}
void __stdcall lglEndList (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEndList");
  tglEndList();
}
void __stdcall lglEvalCoord1d (GLdouble u)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalCoord1d");
  tglEvalCoord1d(u);
}
void __stdcall lglEvalCoord1dv (const GLdouble *u)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalCoord1dv");
  tglEvalCoord1dv(u);
}
void __stdcall lglEvalCoord1f (GLfloat u)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalCoord1f");
  tglEvalCoord1f(u);
}
void __stdcall lglEvalCoord1fv (const GLfloat *u)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalCoord1fv");
  tglEvalCoord1fv(u);
}
void __stdcall lglEvalCoord2d (GLdouble u, GLdouble v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalCoord2d");
  tglEvalCoord2d(u, v);
}
void __stdcall lglEvalCoord2dv (const GLdouble *u)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalCoord2dv");
  tglEvalCoord2dv(u);
}
void __stdcall lglEvalCoord2f (GLfloat u, GLfloat v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalCoord2f");
  tglEvalCoord2f(u, v);
}
void __stdcall lglEvalCoord2fv (const GLfloat *u)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalCoord2fv");
  tglEvalCoord2fv(u);
}
void __stdcall lglEvalMesh1 (GLenum mode, GLint i1, GLint i2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalMesh1");
  tglEvalMesh1(mode, i1, i2);
}
void __stdcall lglEvalMesh2 (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalMesh2");
  tglEvalMesh2(mode, i1, i2, j1, j2);
}
void __stdcall lglEvalPoint1 (GLint i)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalPoint1");
  tglEvalPoint1(i);
}
void __stdcall lglEvalPoint2 (GLint i, GLint j)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEvalPoint2");
  tglEvalPoint2(i, j);
}
void __stdcall lglFeedbackBuffer (GLsizei size, GLenum type, GLfloat *buffer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glFeedbackBuffer");
  tglFeedbackBuffer(size, type, buffer);
}
void __stdcall lglFinish (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glFinish");
  tglFinish();
}
void __stdcall lglFlush (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glFlush");
  tglFlush();
}
void __stdcall lglFogf (GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %.3f)\n", "glFogf", senFog(pname), param);
  tglFogf(pname, param);
}
void __stdcall lglFogfv (GLenum pname, const GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, [%.3f, %.3f, %.3f, %.3f])\n", "glFogfv", senFog(pname), params[0], params[1], params[2], params[3]);
  tglFogfv(pname, params);
}
void __stdcall lglFogi (GLenum pname, GLint param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d)\n", "glFogi", senFog(pname), param);
  tglFogi(pname, param);
}
void __stdcall lglFogiv (GLenum pname, const GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glFogiv");
  tglFogiv(pname, params);
}
void __stdcall lglFrontFace (GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glFrontFace");
  tglFrontFace(mode);
}
void __stdcall lglFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glFrustum");
  tglFrustum(left, right, bottom, top, zNear, zFar);
}
GLuint __stdcall lglGenLists (GLsizei range)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGenLists");
  return tglGenLists(range);
}
void __stdcall lglGenTextures (GLsizei n, GLuint *textures)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGenTextures");
  tglGenTextures(n, textures);
  assert(textures[n-1]<14000);
}
void __stdcall lglGetBooleanv (GLenum pname, GLboolean *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetBooleanv");
  tglGetBooleanv(pname, params);
}
void __stdcall lglGetClipPlane (GLenum plane, GLdouble *equation)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetClipPlane");
  tglGetClipPlane(plane, equation);
}
void __stdcall lglGetDoublev (GLenum pname, GLdouble *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetDoublev");
  tglGetDoublev(pname, params);
}
GLenum __stdcall lglGetError (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetError");
  return tglGetError();
}
void __stdcall lglGetFloatv (GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetFloatv");
  tglGetFloatv(pname, params);
}
void __stdcall lglGetIntegerv (GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetIntegerv");
  tglGetIntegerv(pname, params);
}
void __stdcall lglGetLightfv (GLenum light, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetLightfv");
  tglGetLightfv(light, pname, params);
}
void __stdcall lglGetLightiv (GLenum light, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetLightiv");
  tglGetLightiv(light, pname, params);
}
void __stdcall lglGetMapdv (GLenum target, GLenum query, GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetMapdv");
  tglGetMapdv(target, query, v);
}
void __stdcall lglGetMapfv (GLenum target, GLenum query, GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetMapfv");
  tglGetMapfv(target, query, v);
}
void __stdcall lglGetMapiv (GLenum target, GLenum query, GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetMapiv");
  tglGetMapiv(target, query, v);
}
void __stdcall lglGetMaterialfv (GLenum face, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetMaterialfv");
  tglGetMaterialfv(face, pname, params);
}
void __stdcall lglGetMaterialiv (GLenum face, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetMaterialiv");
  tglGetMaterialiv(face, pname, params);
}
void __stdcall lglGetPixelMapfv (GLenum map, GLfloat *values)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetPixelMapfv");
  tglGetPixelMapfv(map, values);
}
void __stdcall lglGetPixelMapuiv (GLenum map, GLuint *values)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetPixelMapuiv");
  tglGetPixelMapuiv(map, values);
}
void __stdcall lglGetPixelMapusv (GLenum map, GLushort *values)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetPixelMapusv");
  tglGetPixelMapusv(map, values);
}
void __stdcall lglGetPointerv (GLenum pname, GLvoid* *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetPointerv");
  tglGetPointerv(pname, params);
}
void __stdcall lglGetPolygonStipple (GLubyte *mask)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetPolygonStipple");
  tglGetPolygonStipple(mask);
}
const GLubyte * __stdcall lglGetString (GLenum name)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetString");
  return tglGetString(name);
}
void __stdcall lglGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTexEnvfv");
  tglGetTexEnvfv(target, pname, params);
}
void __stdcall lglGetTexEnviv (GLenum target, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTexEnviv");
  tglGetTexEnviv(target, pname, params);
}
void __stdcall lglGetTexGendv (GLenum coord, GLenum pname, GLdouble *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTexGendv");
  tglGetTexGendv(coord, pname, params);
}
void __stdcall lglGetTexGenfv (GLenum coord, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTexGenfv");
  tglGetTexGenfv(coord, pname, params);
}
void __stdcall lglGetTexGeniv (GLenum coord, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTexGeniv");
  tglGetTexGeniv(coord, pname, params);
}
void __stdcall lglGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTexImage");
  tglGetTexImage(target, level, format, type, pixels);
}
void __stdcall lglGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTexLevelParameterfv");
  tglGetTexLevelParameterfv(target, level, pname, params);
}
void __stdcall lglGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTexLevelParameteriv");
  tglGetTexLevelParameteriv(target, level, pname, params);
}
void __stdcall lglGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTexParameterfv");
  tglGetTexParameterfv(target, pname, params);
}
void __stdcall lglGetTexParameteriv (GLenum target, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTexParameteriv");
  tglGetTexParameteriv(target, pname, params);
}
void __stdcall lglHint (GLenum target, GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glHint");
  tglHint(target, mode);
}
void __stdcall lglIndexMask (GLuint mask)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexMask");
  tglIndexMask(mask);
}
void __stdcall lglIndexPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexPointer");
  tglIndexPointer(type, stride, pointer);
}
void __stdcall lglIndexd (GLdouble c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexd");
  tglIndexd(c);
}
void __stdcall lglIndexdv (const GLdouble *c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexdv");
  tglIndexdv(c);
}
void __stdcall lglIndexf (GLfloat c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexf");
  tglIndexf(c);
}
void __stdcall lglIndexfv (const GLfloat *c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexfv");
  tglIndexfv(c);
}
void __stdcall lglIndexi (GLint c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexi");
  tglIndexi(c);
}
void __stdcall lglIndexiv (const GLint *c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexiv");
  tglIndexiv(c);
}
void __stdcall lglIndexs (GLshort c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexs");
  tglIndexs(c);
}
void __stdcall lglIndexsv (const GLshort *c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexsv");
  tglIndexsv(c);
}
void __stdcall lglIndexub (GLubyte c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexub");
  tglIndexub(c);
}
void __stdcall lglIndexubv (const GLubyte *c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIndexubv");
  tglIndexubv(c);
}
void __stdcall lglInitNames (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glInitNames");
  tglInitNames();
}
void __stdcall lglInterleavedArrays (GLenum format, GLsizei stride, const GLvoid *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glInterleavedArrays");
  tglInterleavedArrays(format, stride, pointer);
}
GLboolean __stdcall lglIsEnabled (GLenum cap)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIsEnabled");
  return tglIsEnabled(cap);
}
GLboolean __stdcall lglIsList (GLuint list)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIsList");
  return tglIsList(list);
}
GLboolean __stdcall lglIsTexture (GLuint texture)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIsTexture");
  return tglIsTexture(texture);
}
void __stdcall lglLightModelf (GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLightModelf");
  tglLightModelf(pname, param);
}
void __stdcall lglLightModelfv (GLenum pname, const GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLightModelfv");
  tglLightModelfv(pname, params);
}
void __stdcall lglLightModeli (GLenum pname, GLint param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLightModeli");
  tglLightModeli(pname, param);
}
void __stdcall lglLightModeliv (GLenum pname, const GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLightModeliv");
  tglLightModeliv(pname, params);
}
void __stdcall lglLightf (GLenum light, GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %s, %.3f)\n", "glLightf", light-GL_LIGHT0, senLight(pname), param);
  tglLightf(light, pname, param);
}
void __stdcall lglLightfv (GLenum light, GLenum pname, const GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %s, [%.3f %.3f %.3f %.3f])\n", "glLightfv", light-GL_LIGHT0, senLightv(pname), params[0], params[1], params[2], params[3]);
  tglLightfv(light, pname, params);
}
void __stdcall lglLighti (GLenum light, GLenum pname, GLint param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLighti");
  tglLighti(light, pname, param);
}
void __stdcall lglLightiv (GLenum light, GLenum pname, const GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLightiv");
  tglLightiv(light, pname, params);
}
void __stdcall lglLineStipple (GLint factor, GLushort pattern)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLineStipple");
  tglLineStipple(factor, pattern);
}
void __stdcall lglLineWidth (GLfloat width)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLineWidth");
  tglLineWidth(width);
}
void __stdcall lglListBase (GLuint base)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glListBase");
  tglListBase(base);
}
void __stdcall lglLoadIdentity (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLoadIdentity");
  tglLoadIdentity();
}
void __stdcall lglLoadMatrixd (const GLdouble *m)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLoadMatrixd");
  tglLoadMatrixd(m);
}
void __stdcall lglLoadMatrixf (const GLfloat *m)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLoadMatrixf");
  tglLoadMatrixf(m);
}
void __stdcall lglLoadName (GLuint name)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLoadName");
  tglLoadName(name);
}
void __stdcall lglLogicOp (GLenum opcode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLogicOp");
  tglLogicOp(opcode);
}
void __stdcall lglMap1d (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMap1d");
  tglMap1d(target, u1, u2, stride, order, points);
}
void __stdcall lglMap1f (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMap1f");
  tglMap1f(target, u1, u2, stride, order, points);
}
void __stdcall lglMap2d (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMap2d");
  tglMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}
void __stdcall lglMap2f (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMap2f");
  tglMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}
void __stdcall lglMapGrid1d (GLint un, GLdouble u1, GLdouble u2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMapGrid1d");
  tglMapGrid1d(un, u1, u2);
}
void __stdcall lglMapGrid1f (GLint un, GLfloat u1, GLfloat u2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMapGrid1f");
  tglMapGrid1f(un, u1, u2);
}
void __stdcall lglMapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMapGrid2d");
  tglMapGrid2d(un, u1, u2, vn, v1, v2);
}
void __stdcall lglMapGrid2f (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMapGrid2f");
  tglMapGrid2f(un, u1, u2, vn, v1, v2);
}
void __stdcall lglMaterialf (GLenum face, GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s, %.3f)\n", "glMaterialf", senFace(face), senMaterial(pname), param);
  tglMaterialf(face, pname, param);
}
void __stdcall lglMaterialfv (GLenum face, GLenum pname, const GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s, [%.3f %.3f %.3f %.3f])\n", "glMaterialfv", senFace(face), senMaterialv(pname), params[0], params[1], params[2], params[3]);
  tglMaterialfv(face, pname, params);
}
void __stdcall lglMateriali (GLenum face, GLenum pname, GLint param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMateriali");
  tglMateriali(face, pname, param);
}
void __stdcall lglMaterialiv (GLenum face, GLenum pname, const GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMaterialiv");
  tglMaterialiv(face, pname, params);
}
void __stdcall lglMatrixMode (GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "glMatrixMode", senMatrixMode(mode));
  tglMatrixMode(mode);
}
void __stdcall lglMultMatrixd (const GLdouble *m)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultMatrixd");
  tglMultMatrixd(m);
}
void __stdcall lglMultMatrixf (const GLfloat *m)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultMatrixf");
  tglMultMatrixf(m);
}
void __stdcall lglNewList (GLuint list, GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %s)\n", "glNewList", list, senNewList(mode));
  tglNewList(list, mode);
}
void __stdcall lglNormal3b (GLbyte nx, GLbyte ny, GLbyte nz)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glNormal3b");
  tglNormal3b(nx, ny, nz);
}
void __stdcall lglNormal3bv (const GLbyte *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glNormal3bv");
  tglNormal3bv(v);
}
void __stdcall lglNormal3d (GLdouble nx, GLdouble ny, GLdouble nz)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f, %f)\n", "glNormal3d", nx, ny, nz);
  tglNormal3d(nx, ny, nz);
}
void __stdcall lglNormal3dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glNormal3dv");
  tglNormal3dv(v);
}
void __stdcall lglNormal3f (GLfloat nx, GLfloat ny, GLfloat nz)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f, %f)\n", "glNormal3f", nx, ny, nz);
  tglNormal3f(nx, ny, nz);
}
void __stdcall lglNormal3fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glNormal3fv");
  tglNormal3fv(v);
}
void __stdcall lglNormal3i (GLint nx, GLint ny, GLint nz)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glNormal3i");
  tglNormal3i(nx, ny, nz);
}
void __stdcall lglNormal3iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glNormal3iv");
  tglNormal3iv(v);
}
void __stdcall lglNormal3s (GLshort nx, GLshort ny, GLshort nz)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glNormal3s");
  tglNormal3s(nx, ny, nz);
}
void __stdcall lglNormal3sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glNormal3sv");
  tglNormal3sv(v);
}
void __stdcall lglNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d, 0x%x)\n", "glNormalPointer", senDataType(type), stride, pointer);
  tglNormalPointer(type, stride, pointer);
}
void __stdcall lglOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glOrtho");
  tglOrtho(left, right, bottom, top, zNear, zFar);
}
void __stdcall lglPassThrough (GLfloat token)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPassThrough");
  tglPassThrough(token);
}
void __stdcall lglPixelMapfv (GLenum map, GLsizei mapsize, const GLfloat *values)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPixelMapfv");
  tglPixelMapfv(map, mapsize, values);
}
void __stdcall lglPixelMapuiv (GLenum map, GLsizei mapsize, const GLuint *values)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPixelMapuiv");
  tglPixelMapuiv(map, mapsize, values);
}
void __stdcall lglPixelMapusv (GLenum map, GLsizei mapsize, const GLushort *values)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPixelMapusv");
  tglPixelMapusv(map, mapsize, values);
}
void __stdcall lglPixelStoref (GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPixelStoref");
  tglPixelStoref(pname, param);
}
void __stdcall lglPixelStorei (GLenum pname, GLint param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPixelStorei");
  tglPixelStorei(pname, param);
}
void __stdcall lglPixelTransferf (GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPixelTransferf");
  tglPixelTransferf(pname, param);
}
void __stdcall lglPixelTransferi (GLenum pname, GLint param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPixelTransferi");
  tglPixelTransferi(pname, param);
}
void __stdcall lglPixelZoom (GLfloat xfactor, GLfloat yfactor)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPixelZoom");
  tglPixelZoom(xfactor, yfactor);
}
void __stdcall lglPointSize (GLfloat size)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPointSize");
  tglPointSize(size);
}
void __stdcall lglPolygonMode (GLenum face, GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPolygonMode");
  tglPolygonMode(face, mode);
}
void __stdcall lglPolygonOffset (GLfloat factor, GLfloat units)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f)\n", "glPolygonOffset", factor, units);
  tglPolygonOffset(factor, units);
}
void __stdcall lglPolygonStipple (const GLubyte *mask)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPolygonStipple");
  tglPolygonStipple(mask);
}
void __stdcall lglPopAttrib (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPopAttrib");
  tglPopAttrib();
}
void __stdcall lglPopClientAttrib (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPopClientAttrib");
  tglPopClientAttrib();
}
void __stdcall lglPopMatrix (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPopMatrix");
  tglPopMatrix();
}
void __stdcall lglPopName (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPopName");
  tglPopName();
}
void __stdcall lglPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPrioritizeTextures");
  tglPrioritizeTextures(n, textures, priorities);
}
void __stdcall lglPushAttrib (GLbitfield mask)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPushAttrib");
  tglPushAttrib(mask);
}
void __stdcall lglPushClientAttrib (GLbitfield mask)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPushClientAttrib");
  tglPushClientAttrib(mask);
}
void __stdcall lglPushMatrix (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPushMatrix");
  tglPushMatrix();
}
void __stdcall lglPushName (GLuint name)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPushName");
  tglPushName(name);
}
void __stdcall lglRasterPos2d (GLdouble x, GLdouble y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos2d");
  tglRasterPos2d(x, y);
}
void __stdcall lglRasterPos2dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos2dv");
  tglRasterPos2dv(v);
}
void __stdcall lglRasterPos2f (GLfloat x, GLfloat y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos2f");
  tglRasterPos2f(x, y);
}
void __stdcall lglRasterPos2fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos2fv");
  tglRasterPos2fv(v);
}
void __stdcall lglRasterPos2i (GLint x, GLint y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos2i");
  tglRasterPos2i(x, y);
}
void __stdcall lglRasterPos2iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos2iv");
  tglRasterPos2iv(v);
}
void __stdcall lglRasterPos2s (GLshort x, GLshort y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos2s");
  tglRasterPos2s(x, y);
}
void __stdcall lglRasterPos2sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos2sv");
  tglRasterPos2sv(v);
}
void __stdcall lglRasterPos3d (GLdouble x, GLdouble y, GLdouble z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos3d");
  tglRasterPos3d(x, y, z);
}
void __stdcall lglRasterPos3dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos3dv");
  tglRasterPos3dv(v);
}
void __stdcall lglRasterPos3f (GLfloat x, GLfloat y, GLfloat z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos3f");
  tglRasterPos3f(x, y, z);
}
void __stdcall lglRasterPos3fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos3fv");
  tglRasterPos3fv(v);
}
void __stdcall lglRasterPos3i (GLint x, GLint y, GLint z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos3i");
  tglRasterPos3i(x, y, z);
}
void __stdcall lglRasterPos3iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos3iv");
  tglRasterPos3iv(v);
}
void __stdcall lglRasterPos3s (GLshort x, GLshort y, GLshort z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos3s");
  tglRasterPos3s(x, y, z);
}
void __stdcall lglRasterPos3sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos3sv");
  tglRasterPos3sv(v);
}
void __stdcall lglRasterPos4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos4d");
  tglRasterPos4d(x, y, z, w);
}
void __stdcall lglRasterPos4dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos4dv");
  tglRasterPos4dv(v);
}
void __stdcall lglRasterPos4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos4f");
  tglRasterPos4f(x, y, z, w);
}
void __stdcall lglRasterPos4fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos4fv");
  tglRasterPos4fv(v);
}
void __stdcall lglRasterPos4i (GLint x, GLint y, GLint z, GLint w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos4i");
  tglRasterPos4i(x, y, z, w);
}
void __stdcall lglRasterPos4iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos4iv");
  tglRasterPos4iv(v);
}
void __stdcall lglRasterPos4s (GLshort x, GLshort y, GLshort z, GLshort w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos4s");
  tglRasterPos4s(x, y, z, w);
}
void __stdcall lglRasterPos4sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRasterPos4sv");
  tglRasterPos4sv(v);
}
void __stdcall lglReadBuffer (GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glReadBuffer");
  tglReadBuffer(mode);
}
void __stdcall lglReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glReadPixels");
  tglReadPixels(x, y, width, height, format, type, pixels);
}
void __stdcall lglRectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRectd");
  tglRectd(x1, y1, x2, y2);
}
void __stdcall lglRectdv (const GLdouble *v1, const GLdouble *v2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRectdv");
  tglRectdv(v1, v2);
}
void __stdcall lglRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRectf");
  tglRectf(x1, y1, x2, y2);
}
void __stdcall lglRectfv (const GLfloat *v1, const GLfloat *v2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRectfv");
  tglRectfv(v1, v2);
}
void __stdcall lglRecti (GLint x1, GLint y1, GLint x2, GLint y2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRecti");
  tglRecti(x1, y1, x2, y2);
}
void __stdcall lglRectiv (const GLint *v1, const GLint *v2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRectiv");
  tglRectiv(v1, v2);
}
void __stdcall lglRects (GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRects");
  tglRects(x1, y1, x2, y2);
}
void __stdcall lglRectsv (const GLshort *v1, const GLshort *v2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRectsv");
  tglRectsv(v1, v2);
}
GLint __stdcall lglRenderMode (GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRenderMode");
  return tglRenderMode(mode);
}
void __stdcall lglRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRotated");
  tglRotated(angle, x, y, z);
}
void __stdcall lglRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRotatef");
  tglRotatef(angle, x, y, z);
}
void __stdcall lglScaled (GLdouble x, GLdouble y, GLdouble z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f, %f)\n", "glScaled", x, y, z);
  tglScaled(x, y, z);
}
void __stdcall lglScalef (GLfloat x, GLfloat y, GLfloat z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f, %f)\n", "glScalef", x, y, z);
  tglScalef(x, y, z);
}
void __stdcall lglScissor (GLint x, GLint y, GLsizei width, GLsizei height)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %d, %d, %d)\n", "glScissor", x, y, width, height);
  tglScissor(x, y, width, height);
}
void __stdcall lglSelectBuffer (GLsizei size, GLuint *buffer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSelectBuffer");
  tglSelectBuffer(size, buffer);
}
void __stdcall lglShadeModel (GLenum mode)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glShadeModel");
  tglShadeModel(mode);
}
void __stdcall lglStencilFunc (GLenum func, GLint ref, GLuint mask)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d, 0x%x)\n", "glStencilFunc", senStencilFunc(func), ref, mask);
  tglStencilFunc(func, ref, mask);
}
void __stdcall lglStencilMask (GLuint mask)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (0x%x)\n", "glStencilMask", mask);
  tglStencilMask(mask);
}
void __stdcall lglStencilOp (GLenum fail, GLenum zfail, GLenum zpass)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s, %s)\n", "glStencilOp", senStencilOp(fail), senStencilOp(zfail), senStencilOp(zpass));
  tglStencilOp(fail, zfail, zpass);
}
void __stdcall lglTexCoord1d (GLdouble s)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord1d");
  tglTexCoord1d(s);
}
void __stdcall lglTexCoord1dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord1dv");
  tglTexCoord1dv(v);
}
void __stdcall lglTexCoord1f (GLfloat s)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord1f");
  tglTexCoord1f(s);
}
void __stdcall lglTexCoord1fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord1fv");
  tglTexCoord1fv(v);
}
void __stdcall lglTexCoord1i (GLint s)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord1i");
  tglTexCoord1i(s);
}
void __stdcall lglTexCoord1iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord1iv");
  tglTexCoord1iv(v);
}
void __stdcall lglTexCoord1s (GLshort s)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord1s");
  tglTexCoord1s(s);
}
void __stdcall lglTexCoord1sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord1sv");
  tglTexCoord1sv(v);
}
void __stdcall lglTexCoord2d (GLdouble s, GLdouble t)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord2d");
  tglTexCoord2d(s, t);
}
void __stdcall lglTexCoord2dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord2dv");
  tglTexCoord2dv(v);
}
void __stdcall lglTexCoord2f (GLfloat s, GLfloat t)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%f, %f)\n", "glTexCoord2f", s, t);
  tglTexCoord2f(s, t);
}
void __stdcall lglTexCoord2fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord2fv");
  tglTexCoord2fv(v);
}
void __stdcall lglTexCoord2i (GLint s, GLint t)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord2i");
  tglTexCoord2i(s, t);
}
void __stdcall lglTexCoord2iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord2iv");
  tglTexCoord2iv(v);
}
void __stdcall lglTexCoord2s (GLshort s, GLshort t)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord2s");
  tglTexCoord2s(s, t);
}
void __stdcall lglTexCoord2sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord2sv");
  tglTexCoord2sv(v);
}
void __stdcall lglTexCoord3d (GLdouble s, GLdouble t, GLdouble r)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord3d");
  tglTexCoord3d(s, t, r);
}
void __stdcall lglTexCoord3dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord3dv");
  tglTexCoord3dv(v);
}
void __stdcall lglTexCoord3f (GLfloat s, GLfloat t, GLfloat r)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord3f");
  tglTexCoord3f(s, t, r);
}
void __stdcall lglTexCoord3fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord3fv");
  tglTexCoord3fv(v);
}
void __stdcall lglTexCoord3i (GLint s, GLint t, GLint r)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord3i");
  tglTexCoord3i(s, t, r);
}
void __stdcall lglTexCoord3iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord3iv");
  tglTexCoord3iv(v);
}
void __stdcall lglTexCoord3s (GLshort s, GLshort t, GLshort r)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord3s");
  tglTexCoord3s(s, t, r);
}
void __stdcall lglTexCoord3sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord3sv");
  tglTexCoord3sv(v);
}
void __stdcall lglTexCoord4d (GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord4d");
  tglTexCoord4d(s, t, r, q);
}
void __stdcall lglTexCoord4dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord4dv");
  tglTexCoord4dv(v);
}
void __stdcall lglTexCoord4f (GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord4f");
  tglTexCoord4f(s, t, r, q);
}
void __stdcall lglTexCoord4fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord4fv");
  tglTexCoord4fv(v);
}
void __stdcall lglTexCoord4i (GLint s, GLint t, GLint r, GLint q)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord4i");
  tglTexCoord4i(s, t, r, q);
}
void __stdcall lglTexCoord4iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord4iv");
  tglTexCoord4iv(v);
}
void __stdcall lglTexCoord4s (GLshort s, GLshort t, GLshort r, GLshort q)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord4s");
  tglTexCoord4s(s, t, r, q);
}
void __stdcall lglTexCoord4sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexCoord4sv");
  tglTexCoord4sv(v);
}
void __stdcall lglTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %s, %d, 0x%x)\n", "glTexCoordPointer", size, senDataType(type), stride, pointer);
  tglTexCoordPointer(size, type, stride, pointer);
}
void __stdcall lglTexEnvf (GLenum target, GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s, %s)\n", "glTexEnvf", senTextureEnvTarget(target), senTextureEnvPName(pname), senTextureEnvParam((int)param));
  tglTexEnvf(target, pname, param);
}
void __stdcall lglTexEnvfv (GLenum target, GLenum pname, const GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s, (%.3f, %.3f, %.3f, %.3f))\n", "glTexEnvfv", senTextureEnvTarget(target), senTextureEnvfPName(pname), params[0], params[1], params[2], params[3]);
  tglTexEnvfv(target, pname, params);
}
void __stdcall lglTexEnvi (GLenum target, GLenum pname, GLint param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s, %s)\n", "glTexEnvi", senTextureEnvTarget(target), senTextureEnvPName(pname), senTextureEnvParam(param));
  tglTexEnvi(target, pname, param);
}
void __stdcall lglTexEnviv (GLenum target, GLenum pname, const GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s, (%i, %i, %i, %i))\n", "glTexEnviv", senTextureEnvTarget(target), senTextureEnvfPName(pname), params[0], params[1], params[2], params[3]);
  tglTexEnviv(target, pname, params);
}
void __stdcall lglTexGend (GLenum coord, GLenum pname, GLdouble param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexGend");
  tglTexGend(coord, pname, param);
}
void __stdcall lglTexGendv (GLenum coord, GLenum pname, const GLdouble *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexGendv");
  tglTexGendv(coord, pname, params);
}
void __stdcall lglTexGenf (GLenum coord, GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexGenf");
  tglTexGenf(coord, pname, param);
}
void __stdcall lglTexGenfv (GLenum coord, GLenum pname, const GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexGenfv");
  tglTexGenfv(coord, pname, params);
}
void __stdcall lglTexGeni (GLenum coord, GLenum pname, GLint param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexGeni");
  tglTexGeni(coord, pname, param);
}
void __stdcall lglTexGeniv (GLenum coord, GLenum pname, const GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexGeniv");
  tglTexGeniv(coord, pname, params);
}
void __stdcall lglTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexImage1D");
  tglTexImage1D(target, level, internalformat, width, border, format, type, pixels);
}
void __stdcall lglTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d, %s, %d, %d, %d, %s, %s, 0x%x)\n", "glTexImage2D", senTexTarg(target), level, senTexFormat(internalformat), width, height, border, senTexFormat(format), senTexDataType(type), pixels);
  tglTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}
void __stdcall lglTexParameterf (GLenum target, GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s, %.3f)\n", "glTexParameterf", senTexTarg(target), senTexParmName(pname), param);
  tglTexParameterf(target, pname, param);
}
void __stdcall lglTexParameterfv (GLenum target, GLenum pname, const GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexParameterfv");
  tglTexParameterfv(target, pname, params);
}
void __stdcall lglTexParameteri (GLenum target, GLenum pname, GLint param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %s, %s)\n", "glTexParameteri", senTexTarg(target), senTexParmName(pname), senTexParm(param));
  tglTexParameteri(target, pname, param);
}
void __stdcall lglTexParameteriv (GLenum target, GLenum pname, const GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexParameteriv");
  tglTexParameteriv(target, pname, params);
}
void __stdcall lglTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexSubImage1D");
  tglTexSubImage1D(target, level, xoffset, width, format, type, pixels);
}
void __stdcall lglTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexSubImage2D");
  tglTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}
void __stdcall lglTranslated (GLdouble x, GLdouble y, GLdouble z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTranslated");
  tglTranslated(x, y, z);
}
void __stdcall lglTranslatef (GLfloat x, GLfloat y, GLfloat z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTranslatef");
  tglTranslatef(x, y, z);
}
void __stdcall lglVertex2d (GLdouble x, GLdouble y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex2d");
  tglVertex2d(x, y);
}
void __stdcall lglVertex2dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex2dv");
  tglVertex2dv(v);
}
void __stdcall lglVertex2f (GLfloat x, GLfloat y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex2f");
  tglVertex2f(x, y);
}
void __stdcall lglVertex2fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex2fv");
  tglVertex2fv(v);
}
void __stdcall lglVertex2i (GLint x, GLint y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex2i");
  tglVertex2i(x, y);
}
void __stdcall lglVertex2iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex2iv");
  tglVertex2iv(v);
}
void __stdcall lglVertex2s (GLshort x, GLshort y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex2s");
  tglVertex2s(x, y);
}
void __stdcall lglVertex2sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex2sv");
  tglVertex2sv(v);
}
void __stdcall lglVertex3d (GLdouble x, GLdouble y, GLdouble z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex3d");
  tglVertex3d(x, y, z);
}
void __stdcall lglVertex3dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex3dv");
  tglVertex3dv(v);
}
void __stdcall lglVertex3f (GLfloat x, GLfloat y, GLfloat z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex3f");
  tglVertex3f(x, y, z);
}
void __stdcall lglVertex3fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex3fv");
  tglVertex3fv(v);
}
void __stdcall lglVertex3i (GLint x, GLint y, GLint z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex3i");
  tglVertex3i(x, y, z);
}
void __stdcall lglVertex3iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex3iv");
  tglVertex3iv(v);
}
void __stdcall lglVertex3s (GLshort x, GLshort y, GLshort z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex3s");
  tglVertex3s(x, y, z);
}
void __stdcall lglVertex3sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex3sv");
  tglVertex3sv(v);
}
void __stdcall lglVertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex4d");
  tglVertex4d(x, y, z, w);
}
void __stdcall lglVertex4dv (const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex4dv");
  tglVertex4dv(v);
}
void __stdcall lglVertex4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex4f");
  tglVertex4f(x, y, z, w);
}
void __stdcall lglVertex4fv (const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex4fv");
  tglVertex4fv(v);
}
void __stdcall lglVertex4i (GLint x, GLint y, GLint z, GLint w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex4i");
  tglVertex4i(x, y, z, w);
}
void __stdcall lglVertex4iv (const GLint *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex4iv");
  tglVertex4iv(v);
}
void __stdcall lglVertex4s (GLshort x, GLshort y, GLshort z, GLshort w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex4s");
  tglVertex4s(x, y, z, w);
}
void __stdcall lglVertex4sv (const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertex4sv");
  tglVertex4sv(v);
}
void __stdcall lglVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %s, %d, 0x%x)\n", "glVertexPointer", size, senDataType(type), stride, pointer);
  tglVertexPointer(size, type, stride, pointer);
}
void __stdcall lglViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %d, %d, %d)\n", "glViewport", x, y, width, height);
  tglViewport(x, y, width, height);
}
BOOL __stdcall lpwglCopyContext (HGLRC Parm0, HGLRC Parm1, UINT Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglCopyContext");
  return tpwglCopyContext(Parm0, Parm1, Parm2);
}
HGLRC __stdcall lpwglCreateContext (HDC Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglCreateContext");
  return tpwglCreateContext(Parm0);
}
HGLRC __stdcall lpwglCreateLayerContext (HGLRC Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglCreateLayerContext");
  return tpwglCreateLayerContext(Parm0);
}
BOOL __stdcall lpwglDeleteContext (HGLRC Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglDeleteContext");
  return tpwglDeleteContext(Parm0);
}
HGLRC __stdcall lpwglGetCurrentContext (VOID)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglGetCurrentContext");
  return tpwglGetCurrentContext();
}
HDC __stdcall lpwglGetCurrentDC (VOID)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglGetCurrentDC");
  return tpwglGetCurrentDC();
}
PROC __stdcall lpwglGetProcAddress (LPCSTR Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglGetProcAddress");
  return tpwglGetProcAddress(Parm0);
}
BOOL __stdcall lpwglMakeCurrent (HDC Parm0, HGLRC Parm1)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglMakeCurrent");
  return tpwglMakeCurrent(Parm0, Parm1);
}
BOOL __stdcall lpwglShareLists (HGLRC Parm0, HGLRC Parm1)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglShareLists");
  return tpwglShareLists(Parm0, Parm1);
}
INT __stdcall lpwglChoosePixelFormat (HDC hDC, CONST PIXELFORMATDESCRIPTOR* pfd)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglChoosePixelFormat");
  return tpwglChoosePixelFormat(hDC, pfd);
}
INT __stdcall lpwglDescribePixelFormat (HDC Parm0, INT Parm1, UINT Parm2, PIXELFORMATDESCRIPTOR* Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglDescribePixelFormat");
  return tpwglDescribePixelFormat(Parm0, Parm1, Parm2, Parm3);
}
BOOL __stdcall lpwglSetPixelFormat (HDC Parm0, INT Parm1, CONST PIXELFORMATDESCRIPTOR* Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglSetPixelFormat");
  return tpwglSetPixelFormat(Parm0, Parm1, Parm2);
}
BOOL __stdcall lpwglSwapBuffers (HDC hDC)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglSwapBuffers");
  return tpwglSwapBuffers(hDC);
}
BOOL __stdcall lwglBindTexImageARB (HPBUFFERARB hPbuffer, int iBuffer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglBindTexImageARB");
  return twglBindTexImageARB(hPbuffer, iBuffer);
}
BOOL __stdcall lwglReleaseTexImageARB (HPBUFFERARB hPbuffer, int iBuffer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglReleaseTexImageARB");
  return twglReleaseTexImageARB(hPbuffer, iBuffer);
}
void __stdcall lglSampleCoverageARB(GLclampf p0, GLboolean p1)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSampleCoverageARB");
  tglSampleCoverageARB(p0, p1);
}
/*void __stdcall lglSamplePassARB(GLenum p0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSamplePassARB");
  tglSamplePassARB(p0);
}*/
HANDLE __stdcall lwglCreateBufferRegionARB (HDC hDC, int a, UINT b)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglCreateBufferRegionARB");
  return twglCreateBufferRegionARB(hDC, a, b);
}
void __stdcall lwglDeleteBufferRegionARB (HANDLE a)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglDeleteBufferRegionARB");
  twglDeleteBufferRegionARB(a);
}
BOOL __stdcall lwglSaveBufferRegionARB (HANDLE a, int b, int c, int d, int e)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglSaveBufferRegionARB");
  return twglSaveBufferRegionARB(a,b,c,d,e);
}
BOOL __stdcall lwglRestoreBufferRegionARB (HANDLE a, int b, int c, int d, int e, int f, int g)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglRestoreBufferRegionARB");
  return twglRestoreBufferRegionARB(a,b,c,d,e,f,g);
}
void __stdcall lglTexImage3DEXT (GLenum a, GLint b, GLenum c, GLsizei d, GLsizei e, GLsizei f, GLint g, GLenum h, GLenum i, const GLvoid *j)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexImage3DEXT");
  tglTexImage3DEXT(a,b,c,d,e,f,g,h,i,j);
}
void __stdcall lglTexSubImage3DEXT (GLenum a, GLint b, GLint c, GLint d, GLint e, GLsizei f, GLsizei g, GLsizei h, GLenum i, GLenum j, const GLvoid *k)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTexSubImage3DEXT");
  tglTexSubImage3DEXT(a,b,c,d,e,f,g,h,i,j,k);
}
void __stdcall lglFogCoordPointerEXT (GLenum a, GLsizei b, const GLvoid *c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glFogCoordPointerEXT");
  tglFogCoordPointerEXT(a,b,c);
}
void __stdcall lglFogCoordfEXT (GLfloat a)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glFogCoordfEXT");
  tglFogCoordfEXT(a);
}
void __stdcall lglDrawRangeElementsEXT (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDrawRangeElementsEXT");
  tglDrawRangeElementsEXT(mode,start,end,count,type,indices);
}
INT __stdcall lpChoosePixelFormat (HDC hDC, CONST PIXELFORMATDESCRIPTOR* pfd)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "ChoosePixelFormat");
  return tpChoosePixelFormat(hDC, pfd);
}
INT __stdcall lpDescribePixelFormat (HDC Parm0, INT Parm1, UINT Parm2, PIXELFORMATDESCRIPTOR* Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "DescribePixelFormat");
  return tpDescribePixelFormat(Parm0, Parm1, Parm2, Parm3);
}
BOOL __stdcall lpGetPixelFormat (HDC Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "GetPixelFormat");
  return tpGetPixelFormat(Parm0);
}
BOOL __stdcall lpSetPixelFormat (HDC Parm0, INT Parm1, CONST PIXELFORMATDESCRIPTOR* pfd)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "SetPixelFormat");
  return tpSetPixelFormat(Parm0, Parm1, pfd);
}
BOOL __stdcall lpSwapBuffers (HDC hDC)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "SwapBuffers");
  return tpSwapBuffers(hDC);
}
void __stdcall lglColorTableEXT (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const void *data)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColorTableEXT");
  tglColorTableEXT(target, internalFormat, width, format, type, data);
}
void __stdcall lglColorSubTableEXT (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void *data)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColorSubTableEXT");
  tglColorSubTableEXT(target, start, count, format, type, data);
}
void __stdcall lglGetColorTableEXT (GLenum target, GLenum format, GLenum type, void *data)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetColorTableEXT");
  tglGetColorTableEXT(target, format, type, data);
}
void __stdcall lglGetColorTableParameterivEXT (GLenum target, GLenum pname, int *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetColorTableParameterivEXT");
  tglGetColorTableParameterivEXT(target, pname, params);
}
void __stdcall lglGetColorTableParameterfvEXT (GLenum target, GLenum pname, float *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetColorTableParameterfvEXT");
  tglGetColorTableParameterfvEXT(target, pname, params);
}
void __stdcall lglCompressedTexImage3DARB (GLenum Parm0, GLint Parm1, GLenum Parm2, GLsizei Parm3, GLsizei Parm4, GLsizei Parm5, GLint Parm6, GLsizei Parm7, const GLvoid * Parm8)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCompressedTexImage3DARB");
  tglCompressedTexImage3DARB(Parm0, Parm1, Parm2, Parm3, Parm4, Parm5, Parm6, Parm7, Parm8);
}
void __stdcall lglCompressedTexImage2DARB (GLenum Parm0, GLint Parm1, GLenum Parm2, GLsizei Parm3, GLsizei Parm4, GLint Parm5, GLsizei Parm6, const GLvoid *Parm7)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d, %s, %d, %d, %d, %d, 0x%x)\n", "glCompressedTexImage2DARB", senTexTarg(Parm0), Parm1, senTexFormat(Parm2), Parm3, Parm4, Parm5, Parm6, Parm7);
  tglCompressedTexImage2DARB(Parm0, Parm1, Parm2, Parm3, Parm4, Parm5, Parm6, Parm7);
}
void __stdcall lglCompressedTexImage1DARB (GLenum Parm0, GLint Parm1, GLenum Parm2, GLsizei Parm3, GLint Parm4, GLsizei Parm5, const GLvoid *Parm6)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCompressedTexImage1DARB");
  tglCompressedTexImage1DARB(Parm0, Parm1, Parm2, Parm3, Parm4, Parm5, Parm6);
}
void __stdcall lglCompressedTexSubImage3DARB (GLenum Parm0, GLint Parm1, GLint Parm2, GLint Parm3, GLint Parm4, GLsizei Parm5, GLsizei Parm6, GLsizei Parm7, GLenum Parm8, GLsizei Parm9, const GLvoid *Parm10)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCompressedTexSubImage3DARB");
  tglCompressedTexSubImage3DARB(Parm0, Parm1, Parm2, Parm3, Parm4, Parm5, Parm6, Parm7, Parm8, Parm9, Parm10);
}
void __stdcall lglCompressedTexSubImage2DARB (GLenum Parm0, GLint Parm1, GLint Parm2, GLint Parm3, GLsizei Parm4, GLsizei Parm5, GLenum Parm6, GLsizei Parm7, const GLvoid *Parm8)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCompressedTexSubImage2DARB");
  tglCompressedTexSubImage2DARB(Parm0, Parm1, Parm2, Parm3, Parm4, Parm5, Parm6, Parm7, Parm8);
}
void __stdcall lglCompressedTexSubImage1DARB (GLenum Parm0, GLint Parm1, GLint Parm2, GLsizei Parm3, GLenum Parm4, GLsizei Parm5, const GLvoid *Parm6)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCompressedTexSubImage1DARB");
  tglCompressedTexSubImage1DARB(Parm0, Parm1, Parm2, Parm3, Parm4, Parm5, Parm6);
}
void __stdcall lglGetCompressedTexImageARB (GLenum Parm0, GLint Parm1, void *Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetCompressedTexImageARB");
  tglGetCompressedTexImageARB(Parm0, Parm1, Parm2);
}
void __stdcall lglLockArraysEXT (GLint first, GLsizei count)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %d)\n", "glLockArraysEXT", first, count);
  tglLockArraysEXT(first, count);
}
void __stdcall lglUnlockArraysEXT (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glUnlockArraysEXT");
  tglUnlockArraysEXT();
}
BOOL __stdcall lwglSwapIntervalEXT (int Parm0)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d)\n", "wglSwapIntervalEXT", Parm0);
  return twglSwapIntervalEXT(Parm0);
}
void __stdcall lglMultiTexCoord1fARB (GLenum target, GLfloat Parm1)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultiTexCoord1fARB");
  tglMultiTexCoord1fARB(target, Parm1);
}
void __stdcall lglMultiTexCoord2fARB (GLenum target, GLfloat Parm1, GLfloat Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultiTexCoord2fARB");
  tglMultiTexCoord2fARB(target, Parm1, Parm2);
}
void __stdcall lglMultiTexCoord3fARB (GLenum target, GLfloat Parm1, GLfloat Parm2, GLfloat Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultiTexCoord3fARB");
  tglMultiTexCoord3fARB(target, Parm1, Parm2, Parm3);
}
void __stdcall lglMultiTexCoord4fARB (GLenum target, GLfloat Parm1, GLfloat Parm2, GLfloat Parm3, GLfloat Parm4)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultiTexCoord4fARB");
  tglMultiTexCoord4fARB(target, Parm1, Parm2, Parm3, Parm4);
}
void __stdcall lglMultiTexCoord1fvARB (GLenum target, GLfloat Parm1)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultiTexCoord1fvARB");
  tglMultiTexCoord1fvARB(target, Parm1);
}
void __stdcall lglMultiTexCoord2fvARB (GLenum target, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultiTexCoord2fvARB");
  tglMultiTexCoord2fvARB(target, v);
}
void __stdcall lglMultiTexCoord3fvARB (GLenum target, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultiTexCoord3fvARB");
  tglMultiTexCoord3fvARB(target, v);
}
void __stdcall lglMultiTexCoord4fvARB (GLenum target, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultiTexCoord4fvARB");
  tglMultiTexCoord4fvARB(target, v);
}
void __stdcall lglActiveTextureARB (GLenum target)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "glActiveTextureARB", senActiveTextureARB(target));
  tglActiveTextureARB(target);
}
void __stdcall lglClientActiveTextureARB (GLenum target)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s)\n", "glClientActiveTextureARB", senActiveTextureARB(target));
  tglClientActiveTextureARB(target);
}
void __stdcall lglPointParameterfEXT (GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPointParameterfEXT");
  tglPointParameterfEXT(pname, param);
}
void __stdcall lglPointParameterfvEXT (GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPointParameterfvEXT");
  tglPointParameterfvEXT(pname, params);
}
BOOL __stdcall lwglGetDeviceGammaRamp3DFX(HDC hDC, LPVOID lpRamp)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglGetDeviceGammaRamp3DFX");
  return twglGetDeviceGammaRamp3DFX(hDC, lpRamp);
}
BOOL __stdcall lwglSetDeviceGammaRamp3DFX(HDC hDC, LPVOID lpRamp)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglSetDeviceGammaRamp3DFX");
  return twglSetDeviceGammaRamp3DFX(hDC, lpRamp);
}
void __stdcall lglCullParameterdvSGI (GLenum pname, GLdouble* params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCullParameterdvSGI");
  tglCullParameterdvSGI(pname, params);
}
void __stdcall lglCullParameterfvSGI (GLenum pname, GLfloat* params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCullParameterfvSGI");
  tglCullParameterfvSGI(pname, params);
}
void __stdcall lglVertexArrayRangeNV (int length, void *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexArrayRangeNV");
  tglVertexArrayRangeNV(length, pointer);
}
void __stdcall lglFlushVertexArrayRangeNV (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glFlushVertexArrayRangeNV");
  tglFlushVertexArrayRangeNV();
}
void* __stdcall lwglAllocateMemoryNV (int size, float readFrequency, float writeFrequency, float priority)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglAllocateMemoryNV");
  return twglAllocateMemoryNV(size, readFrequency, writeFrequency, priority);
}
void __stdcall lwglFreeMemoryNV (void *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglFreeMemoryNV");
  twglFreeMemoryNV(pointer);
}
void __stdcall lglDeleteFencesNV (GLsizei n, const GLuint *fences)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDeleteFencesNV");
  tglDeleteFencesNV(n, fences);
}
void __stdcall lglGenFencesNV (GLsizei n, GLuint *fences)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGenFencesNV");
  tglGenFencesNV(n, fences);
}
GLboolean __stdcall lglIsFenceNV (GLuint fence)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIsFenceNV");
  return tglIsFenceNV(fence);
}
GLboolean __stdcall lglTestFenceNV (GLuint fence)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTestFenceNV");
  return tglTestFenceNV(fence);
}
void __stdcall lglGetFenceivNV (GLuint fence, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetFenceivNV");
  tglGetFenceivNV(fence, pname, params);
}
void __stdcall lglFinishFenceNV (GLuint fence)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d)\n", "glFinishFenceNV", fence);
  tglFinishFenceNV(fence);
}
void __stdcall lglSetFenceNV (GLuint fence, GLenum condition)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %s)\n", "glSetFenceNV", fence, senFenceContition(condition));
  tglSetFenceNV(fence, condition);
}
void __stdcall lglSecondaryColor3bEXT (GLbyte Parm0, GLbyte Parm1, GLbyte Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3bEXT");
  tglSecondaryColor3bEXT(Parm0, Parm1, Parm2);
}
void __stdcall lglSecondaryColor3bvEXT (const GLbyte *p)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3bvEXT");
  tglSecondaryColor3bvEXT(p);
}
void __stdcall lglSecondaryColor3dEXT (GLdouble Parm0, GLdouble Parm1, GLdouble Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3dEXT");
  tglSecondaryColor3dEXT(Parm0, Parm1, Parm2);
}
void __stdcall lglSecondaryColor3dvEXT (const GLdouble *p)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3dvEXT");
  tglSecondaryColor3dvEXT(p);
}
void __stdcall lglSecondaryColor3fEXT (GLfloat Parm0, GLfloat Parm1, GLfloat Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3fEXT");
  tglSecondaryColor3fEXT(Parm0, Parm1, Parm2);
}
void __stdcall lglSecondaryColor3fvEXT (const GLfloat *p)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3fvEXT");
  tglSecondaryColor3fvEXT(p);
}
void __stdcall lglSecondaryColor3iEXT (GLint Parm0, GLint Parm1, GLint Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3iEXT");
  tglSecondaryColor3iEXT(Parm0, Parm1, Parm2);
}
void __stdcall lglSecondaryColor3ivEXT (const GLint *p)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3ivEXT");
  tglSecondaryColor3ivEXT(p);
}
void __stdcall lglSecondaryColor3sEXT (GLshort Parm0, GLshort Parm1, GLshort Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3sEXT");
  tglSecondaryColor3sEXT(Parm0, Parm1, Parm2);
}
void __stdcall lglSecondaryColor3svEXT (const GLshort *p)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3svEXT");
  tglSecondaryColor3svEXT(p);
}
void __stdcall lglSecondaryColor3ubEXT (GLubyte Parm0, GLubyte Parm1, GLubyte Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3ubEXT");
  tglSecondaryColor3ubEXT(Parm0, Parm1, Parm2);
}
void __stdcall lglSecondaryColor3ubvEXT (const GLubyte *p)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3ubvEXT");
  tglSecondaryColor3ubvEXT(p);
}
void __stdcall lglSecondaryColor3uiEXT (GLuint Parm0, GLuint Parm1, GLuint Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3uiEXT");
  tglSecondaryColor3uiEXT(Parm0, Parm1, Parm2);
}
void __stdcall lglSecondaryColor3uivEXT (const GLuint *p)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3uivEXT");
  tglSecondaryColor3uivEXT(p);
}
void __stdcall lglSecondaryColor3usEXT (GLushort Parm0, GLushort Parm1, GLushort Parm2)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3usEXT");
  tglSecondaryColor3usEXT(Parm0, Parm1, Parm2);
}
void __stdcall lglSecondaryColor3usvEXT (const GLushort *p)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColor3usvEXT");
  tglSecondaryColor3usvEXT(p);
}
void __stdcall lglSecondaryColorPointerEXT (GLint Parm0, GLenum Parm1, GLsizei Parm2, GLvoid *Parm3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSecondaryColorPointerEXT");
  tglSecondaryColorPointerEXT(Parm0, Parm1, Parm2, Parm3);
}
void __stdcall lglCombinerParameterfvNV (GLenum pname, const GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, (%.3f, %.3f, %.3f, %.3f))\n", "glCombinerParameterfvNV", pname-GL_CONSTANT_COLOR0_NV, params[0], params[1], params[2], params[3]);
  tglCombinerParameterfvNV(pname, params);
}
void __stdcall lglCombinerStageParameterfvNV (GLenum stage, GLenum pname, const GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %d, (%.3f, %.3f, %.3f, %.3f))\n", "glCombinerStageParameterfvNV", stage-GL_COMBINER0_NV, pname-GL_CONSTANT_COLOR0_NV, params[0], params[1], params[2], params[3]);
  tglCombinerStageParameterfvNV(stage, pname, params);
}
void __stdcall lglCombinerParameterfNV (GLenum pname, GLfloat param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCombinerParameterfNV");
  tglCombinerParameterfNV(pname, param);
}
void __stdcall lglCombinerParameterivNV (GLenum pname, const GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCombinerParameterivNV");
  tglCombinerParameterivNV(pname, params);
}
void __stdcall lglCombinerParameteriNV (GLenum pname, GLint param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCombinerParameteriNV");
  tglCombinerParameteriNV(pname, param);
}
void __stdcall lglCombinerInputNV (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCombinerInputNV");
  tglCombinerInputNV(stage, portion, variable, input, mapping, componentUsage);
}
void __stdcall lglCombinerOutputNV (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glCombinerOutputNV");
  tglCombinerOutputNV(stage, portion, abOutput, cdOutput, sumOutput, scale, bias, abDotProduct, cdDotProduct, muxSum);
}
void __stdcall lglFinalCombinerInputNV (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glFinalCombinerInputNV");
  tglFinalCombinerInputNV(variable, input, mapping, componentUsage);
}
void __stdcall lglGetCombinerInputParameterfvNV (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetCombinerInputParameterfvNV");
  tglGetCombinerInputParameterfvNV(stage, portion, variable, pname, params);
}
void __stdcall lglGetCombinerStageParameterfvNV (GLenum stage, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetCombinerStageParameterfvNV");
  tglGetCombinerStageParameterfvNV(stage, pname, params);
}
void __stdcall lglGetCombinerInputParameterivNV (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetCombinerInputParameterivNV");
  tglGetCombinerInputParameterivNV(stage, portion, variable, pname, params);
}
void __stdcall lglGetCombinerOutputParameterfvNV (GLenum stage, GLenum portion, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetCombinerOutputParameterfvNV");
  tglGetCombinerOutputParameterfvNV(stage, portion, pname, params);
}
void __stdcall lglGetCombinerOutputParameterivNV (GLenum stage, GLenum portion, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetCombinerOutputParameterivNV");
  tglGetCombinerOutputParameterivNV(stage, portion, pname, params);
}
void __stdcall lglGetFinalCombinerInputParameterfvNV (GLenum variable, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetFinalCombinerInputParameterfvNV");
  tglGetFinalCombinerInputParameterfvNV(variable, pname, params);
}
void __stdcall lglGetFinalCombinerInputParameterivNV (GLenum variable, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetFinalCombinerInputParameterivNV");
  tglGetFinalCombinerInputParameterivNV(variable, pname, params);
}

GLboolean _stdcall lglAreProgramsResidentNV (GLsizei n, const GLuint *programs, GLboolean *residences)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glAreProgramsResidentNV");
  return tglAreProgramsResidentNV (n, programs, residences);
}
void _stdcall lglBindProgramNV (GLenum target, GLuint id)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d)\n", "glBindProgramNV", id);
  tglBindProgramNV (target, id);
}
void _stdcall lglDeleteProgramsNV  (GLsizei n, const GLuint *programs)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDeleteProgramsNV");
  tglDeleteProgramsNV (n, programs);
}
void _stdcall lglExecuteProgramNV  (GLenum target, GLuint id, const GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glExecuteProgramNV");
  tglExecuteProgramNV  (target, id, params);
}
void _stdcall lglGenProgramsNV  (GLsizei n, GLuint *programs)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGenProgramsNV");
  tglGenProgramsNV  (n, programs);
}
void _stdcall lglGetProgramParameterdvNV  (GLenum target, GLuint index, GLenum pname, GLdouble *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramParameterdvNV");
  tglGetProgramParameterdvNV  (target, index, pname, params);
}
void _stdcall lglGetProgramParameterfvNV  (GLenum target, GLuint index, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramParameterfvNV");
  tglGetProgramParameterfvNV  (target, index, pname, params);
}
void _stdcall lglGetProgramivNV  (GLuint id, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramivNV");
  tglGetProgramivNV  (id, pname, params);
}
void _stdcall lglGetProgramStringNV  (GLuint id, GLenum pname, GLubyte *program)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramStringNV");
  tglGetProgramStringNV  (id, pname, program);
}
void _stdcall lglGetTrackMatrixivNV  (GLenum target, GLuint address, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetTrackMatrixivNV");
  tglGetTrackMatrixivNV  (target, address, pname, params);
}
void _stdcall lglGetVertexAttribdvNV  (GLuint index, GLenum pname, GLdouble *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetVertexAttribdvNV");
  tglGetVertexAttribdvNV  (index, pname, params);
}
void _stdcall lglGetVertexAttribfvNV  (GLuint index, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetVertexAttribfvNV");
  tglGetVertexAttribfvNV  (index, pname, params);
}
void _stdcall lglGetVertexAttribivNV  (GLuint index, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetVertexAttribivNV");
  tglGetVertexAttribivNV  (index, pname, params);
}
/*void _stdcall lglGetVertexAttribPointervNV  (GLuint index, GLenum pname, GLvoid* *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetVertexAttribPointervNV");
  tglGetVertexAttribPointervNV  (index, pname, pointer);
}*/
GLboolean _stdcall lglIsProgramNV  (GLuint id)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIsProgramNV");
  return tglIsProgramNV  (id);
}
void _stdcall lglLoadProgramNV  (GLenum target, GLuint id, GLsizei len, const GLubyte *program)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glLoadProgramNV");
  tglLoadProgramNV  (target, id, len, program);
}
void _stdcall lglProgramParameter4dNV  (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramParameter4dNV");
  tglProgramParameter4dNV  (target, index, x, y, z, w);
}
void _stdcall lglProgramParameter4dvNV  (GLenum target, GLuint index, const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramParameter4dvNV");
  tglProgramParameter4dvNV  (target, index, v);
}
void _stdcall lglProgramParameter4fNV  (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %f, %f, %f, %f)\n", "glProgramParameter4fNV", index, x, y, z, w);
  tglProgramParameter4fNV  (target, index, x, y, z, w);
}
void _stdcall lglProgramParameter4fvNV  (GLenum target, GLuint index, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %f, %f, %f, %f)\n", "glProgramParameter4fvNV", index, v[0], v[1], v[2], v[3]);
  tglProgramParameter4fvNV  (target, index, v);
}
void _stdcall lglProgramParameters4dvNV  (GLenum target, GLuint index, GLsizei count, const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramParameters4dvNV");
  tglProgramParameters4dvNV  (target, index, count, v);
}
void _stdcall lglProgramParameters4fvNV  (GLenum target, GLuint index, GLsizei count, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d", "glProgramParameters4fvNV", index);
  const float *vv = v;
  for (int i=0; i<(int)count; i++)
  {
    gRenDev->Logv(0, ", [%.3f, %.3f, %.3f, %.3f]", vv[0], vv[1], vv[2], vv[3]);
    vv += 4;
  }
  gRenDev->Logv(0, ", %d)\n", count);
  tglProgramParameters4fvNV  (target, index, count, v);
}
void _stdcall lglRequestResidentProgramsNV  (GLsizei n, const GLuint *programs)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glRequestResidentProgramsNV");
  tglRequestResidentProgramsNV  (n, programs);
}
void _stdcall lglTrackMatrixNV  (GLenum target, GLuint address, GLenum matrix, GLenum transform)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glTrackMatrixNV");
  tglTrackMatrixNV  (target, address, matrix, transform);
}
void _stdcall lglVertexAttribPointerNV  (GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d, %d, %s, %d, 0x%x)\n", "glVertexAttribPointerNV", index, fsize, senDataType(type), stride, pointer);
  tglVertexAttribPointerNV  (index, fsize, type, stride, pointer);
}
void _stdcall lglVertexAttrib1dNV  (GLuint index, GLdouble x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1dNV");
  tglVertexAttrib1dNV  (index, x);
}
void _stdcall lglVertexAttrib1dvNV  (GLuint index, const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1dvNV");
  tglVertexAttrib1dvNV  (index, v);
}
void _stdcall lglVertexAttrib1fNV  (GLuint index, GLfloat x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1fNV");
  tglVertexAttrib1fNV  (index, x);
}
void _stdcall lglVertexAttrib1fvNV  (GLuint index, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1fvNV");
  tglVertexAttrib1fvNV  (index, v);
}
void _stdcall lglVertexAttrib1sNV  (GLuint index, GLshort x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1sNV");
  tglVertexAttrib1sNV  (index, x);
}
void _stdcall lglVertexAttrib1svNV  (GLuint index, const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1svNV");
  tglVertexAttrib1svNV  (index, v);
}
void _stdcall lglVertexAttrib2dNV  (GLuint index, GLdouble x, GLdouble y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2dNV");
  tglVertexAttrib2dNV  (index, x, y);
}
void _stdcall lglVertexAttrib2dvNV  (GLuint index, const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2dvNV");
  tglVertexAttrib2dvNV  (index, v);
}
void _stdcall lglVertexAttrib2fNV  (GLuint index, GLfloat x, GLfloat y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2fNV");
  tglVertexAttrib2fNV  (index, x, y);
}
void _stdcall lglVertexAttrib2fvNV  (GLuint index, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2fvNV");
  tglVertexAttrib2fvNV  (index, v);
}
void _stdcall lglVertexAttrib2sNV  (GLuint index, GLshort x, GLshort y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2sNV");
  tglVertexAttrib2sNV  (index, x, y);
}
void _stdcall lglVertexAttrib2svNV  (GLuint index, const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2svNV");
  tglVertexAttrib2svNV  (index, v);
}
void _stdcall lglVertexAttrib3dNV  (GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3dNV");
  tglVertexAttrib3dNV  (index, x, y, z);
}
void _stdcall lglVertexAttrib3dvNV  (GLuint index, const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3dvNV");
  tglVertexAttrib3dvNV  (index, v);
}
void _stdcall lglVertexAttrib3fNV  (GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3fNV");
  tglVertexAttrib3fNV  (index, x, y, z);
}
void _stdcall lglVertexAttrib3fvNV  (GLuint index, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3fvNV");
  tglVertexAttrib3fvNV  (index, v);
}
void _stdcall lglVertexAttrib3sNV  (GLuint index, GLshort x, GLshort y, GLshort z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3sNV");
  tglVertexAttrib3sNV  (index, x, y, z);
}
void _stdcall lglVertexAttrib3svNV  (GLuint index, const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3svNV");
  tglVertexAttrib3svNV  (index, v);
}
void _stdcall lglVertexAttrib4dNV  (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4dNV");
  tglVertexAttrib4dNV  (index, x, y, z, w);
}
void _stdcall lglVertexAttrib4dvNV  (GLuint index, const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4dvNV");
  tglVertexAttrib4dvNV  (index, v);
}
void _stdcall lglVertexAttrib4fNV  (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4fNV");
  tglVertexAttrib4fNV  (index, x, y, z, w);
}
void _stdcall lglVertexAttrib4fvNV  (GLuint index, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4fvNV");
  tglVertexAttrib4fvNV  (index, v);
}
void _stdcall lglVertexAttrib4sNV  (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4sNV");
  tglVertexAttrib4sNV  (index, x, y, z, w);
}
void _stdcall lglVertexAttrib4svNV  (GLuint index, const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4svNV");
  tglVertexAttrib4svNV  (index, v);
}
void _stdcall lglVertexAttrib4ubvNV  (GLuint index, const GLubyte *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4ubvNV");
  tglVertexAttrib4ubvNV  (index, v);
}
void _stdcall lglVertexAttribs1dvNV  (GLuint index, GLsizei count, const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs1dvNV");
  tglVertexAttribs1dvNV  (index, count, v);
}
void _stdcall lglVertexAttribs1fvNV  (GLuint index, GLsizei count, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs1fvNV");
  tglVertexAttribs1fvNV  (index, count, v);
}
void _stdcall lglVertexAttribs1svNV  (GLuint index, GLsizei count, const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs1svNV");
  tglVertexAttribs1svNV  (index, count, v);
}
void _stdcall lglVertexAttribs2dvNV  (GLuint index, GLsizei count, const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs2dvNV");
  tglVertexAttribs2dvNV  (index, count, v);
}
void _stdcall lglVertexAttribs2fvNV  (GLuint index, GLsizei count, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs2fvNV");
  tglVertexAttribs2fvNV  (index, count, v);
}
void _stdcall lglVertexAttribs2svNV  (GLuint index, GLsizei count, const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs2svNV");
  tglVertexAttribs2svNV  (index, count, v);
}
void _stdcall lglVertexAttribs3dvNV  (GLuint index, GLsizei count, const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs3dvNV");
  tglVertexAttribs3dvNV  (index, count, v);
}
void _stdcall lglVertexAttribs3fvNV  (GLuint index, GLsizei count, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs3fvNV");
  tglVertexAttribs3fvNV  (index, count, v);
}
void _stdcall lglVertexAttribs3svNV  (GLuint index, GLsizei count, const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs3svNV");
  tglVertexAttribs3svNV  (index, count, v);
}
void _stdcall lglVertexAttribs4dvNV  (GLuint index, GLsizei count, const GLdouble *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs4dvNV");
  tglVertexAttribs4dvNV  (index, count, v);
}
void _stdcall lglVertexAttribs4fvNV  (GLuint index, GLsizei count, const GLfloat *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs4fvNV");
  tglVertexAttribs4fvNV  (index, count, v);
}
void _stdcall lglVertexAttribs4svNV  (GLuint index, GLsizei count, const GLshort *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs4svNV");
  tglVertexAttribs4svNV  (index, count, v);
}
void _stdcall lglVertexAttribs4ubvNV  (GLuint index, GLsizei count, const GLubyte *v)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribs4ubvNV");
  tglVertexAttribs4ubvNV  (index, count, v);
}

void __stdcall lglMultiDrawArraysEXT(GLenum p0, GLint *p1, GLsizei *p2, GLsizei p3)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultiDrawArraysEXT");
  tglMultiDrawArraysEXT(p0, p1, p2, p3);
}
void __stdcall lglMultiDrawElementsEXT(GLenum p0, const GLsizei *p1, GLenum p2, const GLvoid* *p3, GLsizei p4)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMultiDrawElementsEXT");
  tglMultiDrawElementsEXT(p0, p1, p2, p3, p4);
}

void __stdcall lglPointParameteriNV (GLenum pname, int param)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPointParameteriNV");
  tglPointParameteriNV(pname, param);
}
void __stdcall lglPointParameterivNV(GLenum pname, const int *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPointParameterivNV");
  tglPointParameterivNV(pname, params);
}

BOOL _stdcall lwglGetPixelFormatAttribivARB (HDC hdc, int a, int b, UINT c, const int *d, int *e)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglGetPixelFormatAttribivARB");
  return twglGetPixelFormatAttribivARB (hdc, a,b,c,d,e);
}
BOOL _stdcall lwglGetPixelFormatAttribfvARB (HDC hdc, int a, int b, UINT c, const int *d, FLOAT *e)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglGetPixelFormatAttribfvARB");
  return twglGetPixelFormatAttribfvARB (hdc, a,b,c,d,e);
}
BOOL _stdcall lwglChoosePixelFormatARB (HDC hdc, const int *a, const FLOAT *b, UINT c, int *d, UINT *e)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglChoosePixelFormatARB");
  return twglChoosePixelFormatARB (hdc, a,b,c,d,e);
}

BOOL _stdcall lwglMakeContextCurrentARB (HDC a, HDC b, HGLRC c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglMakeContextCurrentARB");
  return twglMakeContextCurrentARB (a,b,c);
}
HDC _stdcall lwglGetCurrentReadDCARB (void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglGetCurrentReadDCARB");
  return twglGetCurrentReadDCARB ();
}

HPBUFFERARB _stdcall lwglCreatePbufferARB (HDC a, int b, int c, int d, const int * e)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglCreatePbufferARB");
  return twglCreatePbufferARB (a,b,c,d,e);
}
HDC _stdcall lwglGetPbufferDCARB (HPBUFFERARB a)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglGetPbufferDCARB");
  return twglGetPbufferDCARB (a);
}
int _stdcall lwglReleasePbufferDCARB (HPBUFFERARB a, HDC b)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglReleasePbufferDCARB");
  return twglReleasePbufferDCARB (a,b);
}
BOOL _stdcall lwglDestroyPbufferARB (HPBUFFERARB a)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglDestroyPbufferARB");
  return twglDestroyPbufferARB (a);
}
BOOL _stdcall lwglQueryPbufferARB (HPBUFFERARB a, int b, int *c)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "wglQueryPbufferARB");
  return twglQueryPbufferARB (a,b,c);
}

void _stdcall lglGenOcclusionQueriesNV(GLsizei n, GLuint *ids)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGenOcclusionQueriesNV");
  return tglGenOcclusionQueriesNV(n, ids);
}
void _stdcall lglDeleteOcclusionQueriesNV(GLsizei n, GLuint *ids)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDeleteOcclusionQueriesNV");
  return tglDeleteOcclusionQueriesNV(n, ids);
}
void _stdcall lglIsOcclusionQueryNV(GLuint id)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIsOcclusionQueryNV");
  return tglIsOcclusionQueryNV(id);
}
void _stdcall lglBeginOcclusionQueryNV(GLuint id)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glBeginOcclusionQueryNV");
  return tglBeginOcclusionQueryNV(id);
}
void _stdcall lglEndOcclusionQueryNV(void)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEndOcclusionQueryNV");
  return tglEndOcclusionQueryNV();
}
void _stdcall lglGetOcclusionQueryivNV(GLuint id, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetOcclusionQueryivNV");
  return tglGetOcclusionQueryivNV(id, pname, params);
}
void _stdcall lglGetOcclusionQueryuivNV(GLuint id, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetOcclusionQueryuivNV");
  return tglGetOcclusionQueryuivNV(id, pname, params);
}

GLuint _stdcall lglGenFragmentShadersATI(GLuint range)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d)\n", "glGenFragmentShadersATI", range);
  return tglGenFragmentShadersATI(range);
}
void _stdcall lglBindFragmentShaderATI(GLuint id)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d)\n", "glBindFragmentShaderATI", id);
  tglBindFragmentShaderATI(id);
}
void _stdcall lglDeleteFragmentShaderATI(GLuint id)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d)\n", "glDeleteFragmentShaderATI", id);
  tglDeleteFragmentShaderATI(id);
}
void _stdcall lglBeginFragmentShaderATI(GLvoid)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glBeginFragmentShaderATI");
  tglBeginFragmentShaderATI();
}
void _stdcall lglEndFragmentShaderATI(GLvoid)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEndFragmentShaderATI");
  tglEndFragmentShaderATI();
}
void _stdcall lglPassTexCoordATI(GLuint dst, GLuint coord, GLenum swizzle)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glPassTexCoordATI");
  tglPassTexCoordATI(dst, coord, swizzle);
}
void _stdcall lglSampleMapATI(GLuint dst, GLuint interp, GLenum swizzle)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glSampleMapATI");
  tglSampleMapATI(dst, interp, swizzle);
}
void _stdcall lglColorFragmentOp1ATI(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColorFragmentOp1ATI");
  tglColorFragmentOp1ATI(op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod);
}
void _stdcall lglColorFragmentOp2ATI(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColorFragmentOp2ATI");
  tglColorFragmentOp2ATI(op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod);
}
void _stdcall lglColorFragmentOp3ATI(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glColorFragmentOp3ATI");
  tglColorFragmentOp3ATI(op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod);
}
void _stdcall lglAlphaFragmentOp1ATI(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glAlphaFragmentOp1ATI");
  tglAlphaFragmentOp1ATI(op, dst, dstMod, arg1, arg1Rep, arg1Mod);
}
void _stdcall lglAlphaFragmentOp2ATI(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glAlphaFragmentOp2ATI");
  tglAlphaFragmentOp2ATI(op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod);
}
void _stdcall lglAlphaFragmentOp3ATI(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glAlphaFragmentOp3ATI");
  tglAlphaFragmentOp3ATI(op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod);
}
void _stdcall lglSetFragmentShaderConstantATI(GLuint dst, const GLfloat *value)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (0x%x, (%.3f, %.3f, %.3f, %.3f))\n", "glSetFragmentShaderConstantATI", dst, value[0], value[1], value[2], value[3]);
  tglSetFragmentShaderConstantATI(dst, value);
}

void _stdcall lglVertexAttrib1sARB(GLuint index, GLshort x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1sARB");
  tglVertexAttrib1sARB(index, x);
}
void _stdcall lglVertexAttrib1fARB(GLuint index, GLfloat x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1fARB");
  tglVertexAttrib1fARB(index, x);
}
void _stdcall lglVertexAttrib1dARB(GLuint index, GLdouble x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1dARB");
  tglVertexAttrib1dARB(index, x);
}
void _stdcall lglVertexAttrib2sARB(GLuint index, GLshort x, GLshort y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2sARB");
  tglVertexAttrib2sARB(index, x, y);
}
void _stdcall lglVertexAttrib2fARB(GLuint index, GLfloat x, GLfloat y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2fARB");
  tglVertexAttrib2fARB(index, x, y);
}
void _stdcall lglVertexAttrib2dARB(GLuint index, GLdouble x, GLdouble y)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2dARB");
  tglVertexAttrib2dARB(index, x, y);
}
void _stdcall lglVertexAttrib3sARB(GLuint index, GLshort x, GLshort y, GLshort z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3sARB");
  tglVertexAttrib3sARB(index, x, y, z);
}
void _stdcall lglVertexAttrib3fARB(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3fARB");
  tglVertexAttrib3fARB(index, x, y, z);
}
void _stdcall lglVertexAttrib3dARB(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3dARB");
  tglVertexAttrib3dARB(index, x, y, z);
}
void _stdcall lglVertexAttrib4sARB(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4sARB");
  tglVertexAttrib4sARB(index, x, y, z, w);
}
void _stdcall lglVertexAttrib4fARB(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4fARB");
  tglVertexAttrib4fARB(index, x, y, z, w);
}
void _stdcall lglVertexAttrib4dARB(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4dARB");
  tglVertexAttrib4dARB(index, x, y, z, w);
}
void _stdcall lglVertexAttrib4NubARB(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4NubARB");
  tglVertexAttrib4NubARB(index, x, y, z, w);
}
void _stdcall lglVertexAttrib1svARB(GLuint index, const GLshort *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1svARB");
  tglVertexAttrib1svARB(index, x);
}
void _stdcall lglVertexAttrib1fvARB(GLuint index, const GLfloat *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1fvARB");
  tglVertexAttrib1fvARB(index, x);
}
void _stdcall lglVertexAttrib1dvARB(GLuint index, const GLdouble *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib1dvARB");
  tglVertexAttrib1dvARB(index, x);
}
void _stdcall lglVertexAttrib2svARB(GLuint index, const GLshort *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2svARB");
  tglVertexAttrib2svARB(index, x);
}
void _stdcall lglVertexAttrib2fvARB(GLuint index, const GLfloat *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2fvARB");
  tglVertexAttrib2fvARB(index, x);
}
void _stdcall lglVertexAttrib2dvARB(GLuint index, const GLdouble *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib2dvARB");
  tglVertexAttrib2dvARB(index, x);
}
void _stdcall lglVertexAttrib3svARB(GLuint index, const GLshort *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3svARB");
  tglVertexAttrib3svARB(index, x);
}
void _stdcall lglVertexAttrib3fvARB(GLuint index, const GLfloat *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3fvARB");
  tglVertexAttrib3fvARB(index, x);
}
void _stdcall lglVertexAttrib3dvARB(GLuint index, const GLdouble *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib3dvARB");
  tglVertexAttrib3dvARB(index, x);
}
void _stdcall lglVertexAttrib4bvARB(GLuint index, const GLbyte *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4bvARB");
  tglVertexAttrib4bvARB(index, x);
}
void _stdcall lglVertexAttrib4ubvARB(GLuint index, const GLubyte *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4ubvARB");
  tglVertexAttrib4ubvARB(index, x);
}
void _stdcall lglVertexAttrib4svARB(GLuint index, const GLshort *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4svARB");
  tglVertexAttrib4svARB(index, x);
}
void _stdcall lglVertexAttrib4usvARB(GLuint index, const GLushort *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4usvARB");
  tglVertexAttrib4usvARB(index, x);
}
void _stdcall lglVertexAttrib4ivARB(GLuint index, const GLint *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4ivARB");
  tglVertexAttrib4ivARB(index, x);
}
void _stdcall lglVertexAttrib4uivARB(GLuint index, const GLuint *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4uivARB");
  tglVertexAttrib4uivARB(index, x);
}
void _stdcall lglVertexAttrib4fvARB(GLuint index, const GLfloat *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4fvARB");
  tglVertexAttrib4fvARB(index, x);
}
void _stdcall lglVertexAttrib4dvARB(GLuint index, const GLdouble *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4dvARB");
  tglVertexAttrib4dvARB(index, x);
}
void _stdcall lglVertexAttrib4NbvARB(GLuint index, const GLbyte *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4NbvARB");
  tglVertexAttrib4NbvARB(index, x);
}
void _stdcall lglVertexAttrib4NsvARB(GLuint index, const GLshort *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4NsvARB");
  tglVertexAttrib4NsvARB(index, x);
}
void _stdcall lglVertexAttrib4NivARB(GLuint index, const GLint *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4NivARB");
  tglVertexAttrib4NivARB(index, x);
}
void _stdcall lglVertexAttrib4NubvARB(GLuint index, const GLubyte *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4NubARB");
  tglVertexAttrib4NubvARB(index, x);
}
void _stdcall lglVertexAttrib4NusvARB(GLuint index, const GLushort *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4NusARB");
  tglVertexAttrib4NusvARB(index, x);
}
void _stdcall lglVertexAttrib4NuivARB(GLuint index, const GLuint *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttrib4NuiARB");
  tglVertexAttrib4NuivARB(index, x);
}
void _stdcall lglVertexAttribPointerARB(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glVertexAttribPointerARB");
  tglVertexAttribPointerARB(index, size, type, normalized, stride, pointer);
}
void _stdcall lglEnableVertexAttribArrayARB(GLuint index)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glEnableVertexAttribArrayARB");
  tglEnableVertexAttribArrayARB(index);
}
void _stdcall lglDisableVertexAttribArrayARB(GLuint index)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDisableVertexAttribArrayARB");
  tglDisableVertexAttribArrayARB(index);
}
void _stdcall lglProgramStringARB(GLenum target, GLenum format, GLsizei len, const void *string)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramStringARB");
  tglProgramStringARB(target, format, len, string);
}
void _stdcall lglBindProgramARB(GLenum target, GLuint program)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, 0x%x)\n", "glBindProgramARB", senProgramARB(target), program);
  tglBindProgramARB(target, program);
}
void _stdcall lglDeleteProgramsARB(GLsizei n, const GLuint *programs)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDeleteProgramsARB");
  tglDeleteProgramsARB(n, programs);
}
void _stdcall lglGenProgramsARB(GLsizei n, GLuint *programs)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGenProgramsARB");
  tglGenProgramsARB(n, programs);
}
void _stdcall lglProgramEnvParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramEnvParameter4dARB");
  tglProgramEnvParameter4dARB(target, index, x, y, z, w);
}
void _stdcall lglProgramEnvParameter4dvARB(GLenum target, GLuint index, const GLdouble *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramEnvParameter4dvARB");
  tglProgramEnvParameter4dvARB(target, index, x);
}
void _stdcall lglProgramEnvParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d, [%.3f, %.3f, %.3f, %.3f])\n", "glProgramEnvParameter4fARB", senProgramARB(target), index, x, y, z, w);
  tglProgramEnvParameter4fARB(target, index, x, y, z, w);
}
void _stdcall lglProgramEnvParameter4fvARB(GLenum target, GLuint index, const GLfloat *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d, [%.3f, %.3f, %.3f, %.3f])\n", "glProgramEnvParameter4fvARB", senProgramARB(target), index, x[0], x[1], x[2], x[3]);
  tglProgramEnvParameter4fvARB(target, index, x);
}
void _stdcall lglProgramLocalParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramLocalParameter4dARB");
  tglProgramLocalParameter4dARB(target, index, x, y, z, w);
}
void _stdcall lglProgramLocalParameter4dvARB(GLenum target, GLuint index, const GLdouble *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramLocalParameter4dvARB");
  tglProgramLocalParameter4dvARB(target, index, x);
}
void _stdcall lglProgramLocalParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramLocalParameter4fARB");
  tglProgramLocalParameter4fARB(target, index, x, y, z, w);
}
void _stdcall lglProgramLocalParameter4fvARB(GLenum target, GLuint index, const GLfloat *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s(0x%x, %d, (%.3f, %.3f, %.3f, %.3f))\n", "glProgramLocalParameter4fvARB", target, index, x[0], x[1], x[2], x[3]);
  tglProgramLocalParameter4fvARB(target, index, x);
}
void _stdcall lglGetProgramEnvParameterfvARB(GLenum target, GLuint index, GLfloat *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramEnvParameterfvARB");
  tglGetProgramEnvParameterfvARB(target, index, x);
}
void _stdcall lglGetProgramEnvParameterdvARB(GLenum target, GLuint index, GLdouble *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramEnvParameterdvARB");
  tglGetProgramEnvParameterdvARB(target, index, x);
}
void _stdcall lglGetProgramLocalParameterfvARB(GLenum target, GLuint index, GLfloat *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramLocalParameterfvARB");
  tglGetProgramLocalParameterfvARB(target, index, x);
}
void _stdcall lglGetProgramLocalParameterdvARB(GLenum target, GLuint index, GLdouble *x)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramLocalParameterdvARB");
  tglGetProgramLocalParameterdvARB(target, index, x);
}
void _stdcall lglGetProgramivARB(GLenum target, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramivARB");
  tglGetProgramivARB(target, pname, params);
}
void _stdcall lglGetProgramStringARB(GLenum target, GLenum pname, void *string)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramStringARB");
  tglGetProgramStringARB(target, pname, string);
}
void _stdcall lglGetVertexAttribdvARB(GLuint index, GLenum pname, GLdouble *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetVertexAttribdvARB");
  tglGetVertexAttribdvARB(index, pname, params);
}
void _stdcall lglGetVertexAttribfvARB(GLuint index, GLenum pname, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetVertexAttribfvARB");
  tglGetVertexAttribfvARB(index, pname, params);
}
void _stdcall lglGetVertexAttribivARB(GLuint index, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetVertexAttribivARB");
  tglGetVertexAttribivARB(index, pname, params);
}
void _stdcall lglGetVertexAttribPointervARB(GLuint index, GLenum pname, void **pointer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetVertexAttribPointervARB");
  tglGetVertexAttribPointervARB(index, pname, pointer);
}
GLboolean _stdcall lglIsProgramARB(GLuint program)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glIsProgramARB");
  return tglIsProgramARB(program);
}

void _stdcall lglBindBufferARB(GLenum target, GLuint buffer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%s, %d)\n", "glBindBufferARB", senArrayBufferARB(target), buffer);
  tglBindBufferARB(target, buffer);
}
void _stdcall lglDeleteBuffersARB(GLsizei n, const GLuint *buffers)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDeleteBuffersARB");
  tglDeleteBuffersARB(n, buffers);
}
void _stdcall lglGenBuffersARB(GLsizei n, GLuint *buffers)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGenBuffersARB");
  tglGenBuffersARB(n, buffers);
}
GLboolean _stdcall lglIsBufferARB(GLuint buffer)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s (%d)\n", "glIsBuffersARB", buffer);
  return tglIsBufferARB(buffer);
}
void _stdcall lglBufferDataARB(GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glBufferDataARB");
  tglBufferDataARB(target, size, data, usage);
}
void _stdcall lglBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glBufferSubDataARB");
  tglBufferSubDataARB(target, offset, size, data);
}
void _stdcall lglGetBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetBufferSubDataARB");
  tglGetBufferSubDataARB(target, offset, size, data);
}
void * _stdcall lglMapBufferARB(GLenum target, GLenum access)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glMapBufferARB");
  return tglMapBufferARB(target, access);
}
GLboolean _stdcall lglUnmapBufferARB(GLenum target)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glUnmapBufferARB");
  return tglUnmapBufferARB(target);
}
void _stdcall lglGetBufferParameterivARB(GLenum target, GLenum pname, GLint *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetBufferParameterivARB");
  tglGetBufferParameterivARB(target, pname, params);
}
void _stdcall lglGetBufferPointervARB(GLenum target, GLenum pname, GLvoid **params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetBufferPointervARB");
  tglGetBufferPointervARB(target, pname, params);
}

void _stdcall lglDepthBoundsEXT(GLclampd minx, GLclampd maxx)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glDepthBoundsEXT");
  tglDepthBoundsEXT(minx, maxx);
}

void _stdcall lglStencilFuncSeparateATI(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glStencilFuncSeparateATI (%s, %s, 0x%x, 0x%x)", senStencilFunc(frontfunc), senStencilFunc(backfunc), ref, mask);
  tglStencilFuncSeparateATI(frontfunc, backfunc, ref, mask);
}
void _stdcall lglStencilOpSeparateATI(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glStencilOpSeparateATI (%s, %s, %s)", senFace(face), senStencilOp(sfail), senStencilOp(dpfail), senStencilOp(dppass));
  tglStencilOpSeparateATI(face, sfail, dpfail, dppass);
}

void _stdcall lglActiveStencilFaceEXT(GLenum face)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glActiveStencilFaceEXT (%s)", senFace(face));
  tglActiveStencilFaceEXT(face);
}

void _stdcall lglProgramNamedParameter4fNV(GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramNamedParameter4fNV (%d, %d, %s, [%.3f, %.3f, %.3f, %.3f])", id, len, name, x,y,z,w);
  tglProgramNamedParameter4fNV(id, len, name, x, y, z, w);
}
void _stdcall lglProgramNamedParameter4dNV(GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramNamedParameter4dNV (%d, %d, %s, [%.3f, %.3f, %.3f, %.3f])", id, len, name, x,y,z,w);
  tglProgramNamedParameter4dNV(id, len, name, x, y, z, w);
}
void _stdcall lglProgramNamedParameter4fvNV(GLuint id, GLsizei len, const GLubyte *name, const GLfloat v[])
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramNamedParameter4fvNV (%d, %d, %s, [%.3f, %.3f, %.3f, %.3f])", id, len, name, v[0],v[1],v[2],v[3]);
  tglProgramNamedParameter4fvNV(id, len, name, v);
}
void _stdcall lglProgramNamedParameter4dvNV(GLuint id, GLsizei len, const GLubyte *name, const GLdouble v[])
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glProgramNamedParameter4dvNV (%d, %d, %s, [%.3f, %.3f, %.3f, %.3f])", id, len, name, v[0],v[1],v[2],v[3]);
  tglProgramNamedParameter4dvNV(id, len, name, v);
}
void _stdcall lglGetProgramNamedParameterfvNV(GLuint id, GLsizei len, const GLubyte *name, GLfloat *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramNamedParameterfvNV");
  tglGetProgramNamedParameterfvNV(id, len, name, params);
}
void _stdcall lglGetProgramNamedParameterdvNV(GLuint id, GLsizei len, const GLubyte *name, GLdouble *params)
{
  gRenDev->Logv(SRendItem::m_RecurseLevel, "%s\n", "glGetProgramNamedParameterdvNV");
  tglGetProgramNamedParameterdvNV(id, len, name, params);
}

//==============================================================================================

void CGLRenderer::SetLogFuncs(bool set)
{
  static bool sSet = 0;

  if (set == sSet)
    return;

  sSet = set;

  if (set)
  {
#define GL_EXT(name) 
#define GL_PROC(ext,ret,func,parms) if (func) t##func = func; func = l##func;
#include "GLFuncs.h"
#undef GL_EXT
#undef GL_PROC
  }
  else
  {
#define GL_EXT(name) 
#define GL_PROC(ext,ret,func,parms) if (t##func) func = t##func;
#include "GLFuncs.h"
#undef GL_EXT
#undef GL_PROC
  }
}

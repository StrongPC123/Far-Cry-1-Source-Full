/*
 *
 * Copyright (c) 2002, NVIDIA Corporation.
 * 
 *  
 * 
 * NVIDIA Corporation("NVIDIA") supplies this software to you in consideration 
 * of your agreement to the following terms, and your use, installation, 
 * modification or redistribution of this NVIDIA software constitutes 
 * acceptance of these terms.  If you do not agree with these terms, please do 
 * not use, install, modify or redistribute this NVIDIA software.
 * 
 *  
 * 
 * In consideration of your agreement to abide by the following terms, and 
 * subject to these terms, NVIDIA grants you a personal, non-exclusive license,
 * under NVIDIA’s copyrights in this original NVIDIA software (the "NVIDIA 
 * Software"), to use, reproduce, modify and redistribute the NVIDIA 
 * Software, with or without modifications, in source and/or binary forms; 
 * provided that if you redistribute the NVIDIA Software, you must retain the 
 * copyright notice of NVIDIA, this notice and the following text and 
 * disclaimers in all such redistributions of the NVIDIA Software. Neither the 
 * name, trademarks, service marks nor logos of NVIDIA Corporation may be used 
 * to endorse or promote products derived from the NVIDIA Software without 
 * specific prior written permission from NVIDIA.  Except as expressly stated 
 * in this notice, no other rights or licenses express or implied, are granted 
 * by NVIDIA herein, including but not limited to any patent rights that may be 
 * infringed by your derivative works or by other works in which the NVIDIA 
 * Software may be incorporated. No hardware is licensed hereunder. 
 * 
 *  
 * 
 * THE NVIDIA SOFTWARE IS BEING PROVIDED ON AN "AS IS" BASIS, WITHOUT 
 * WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING 
 * WITHOUT LIMITATION, WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR ITS USE AND OPERATION 
 * EITHER ALONE OR IN COMBINATION WITH OTHER PRODUCTS.
 * 
 *  
 * 
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL, 
 * EXEMPLARY, CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, LOST 
 * PROFITS; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) OR ARISING IN ANY WAY OUT OF THE USE, 
 * REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE NVIDIA SOFTWARE, 
 * HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING 
 * NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF NVIDIA HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */ 

#ifndef _cggl_h
#define _cggl_h



#include "Cg/cg.h"

/* Set up for either Win32 import/export/lib. */
#ifndef CGGLDLL_API
#if WIN32
    # include <windows.h>
    #ifdef CGGLDLL_EXPORTS
    #define CGGLDLL_API __declspec(dllexport)
    #elif defined (CG_LIB)
    #define CGGLDLL_API
    #else
    #define CGGLDLL_API __declspec(dllimport)
    #endif
#else
    #define CGGLDLL_API
#endif
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/*** cgGL Type Definitions                                                 ***/
/*****************************************************************************/

typedef enum
 {
  CG_GL_MATRIX_IDENTITY = 0,
  CG_GL_MATRIX_TRANSPOSE = 1,
  CG_GL_MATRIX_INVERSE = 2,
  CG_GL_MATRIX_INVERSE_TRANSPOSE = 3,

  CG_GL_MODELVIEW_MATRIX,
  CG_GL_PROJECTION_MATRIX,
  CG_GL_TEXTURE_MATRIX,
  CG_GL_MODELVIEW_PROJECTION_MATRIX,

  CG_GL_VERTEX,
  CG_GL_FRAGMENT,

 } CGGLenum;


#ifndef CGGL_EXPLICIT

/******************************************************************************
 *** Profile Functions                                 
 *****************************************************************************/

CGGLDLL_API CGbool cgGLIsProfileSupported(CGprofile profile);

CGGLDLL_API void cgGLEnableProfile(CGprofile profile);
CGGLDLL_API void cgGLDisableProfile(CGprofile profile);

CGGLDLL_API CGprofile cgGLGetLatestProfile(CGGLenum profile_type);
CGGLDLL_API void cgGLSetOptimalOptions(CGprofile profile);

/******************************************************************************
 *** Program Managment Functions                                 
 *****************************************************************************/

CGGLDLL_API void cgGLLoadProgram(CGprogram program);
CGGLDLL_API CGbool cgGLIsProgramLoaded(CGprogram program);
CGGLDLL_API void cgGLBindProgram(CGprogram program);
CGGLDLL_API void cgGLUnbindProgram(CGprofile profile);
CGGLDLL_API GLuint cgGLGetProgramID(CGprogram program);

/******************************************************************************
 *** Parameter Managment Functions                                 
 *****************************************************************************/

CGGLDLL_API void cgGLSetParameter1f(CGparameter param,
                                    float x);

CGGLDLL_API void cgGLSetParameter2f(CGparameter param,
                                    float x,
                                    float y);

CGGLDLL_API void cgGLSetParameter3f(CGparameter param,
                                    float x,
                                    float y,
                                    float z);

CGGLDLL_API void cgGLSetParameter4f(CGparameter param,
                                    float x,
                                    float y,
                                    float z,
                                    float w);

CGGLDLL_API void cgGLSetParameter1fv(CGparameter param, const float *v);

CGGLDLL_API void cgGLSetParameter2fv(CGparameter param, const float *v);

CGGLDLL_API void cgGLSetParameter3fv(CGparameter param, const float *v);

CGGLDLL_API void cgGLSetParameter4fv(CGparameter param, const float *v);

CGGLDLL_API void cgGLSetParameter1d(CGparameter param,
                                    double x);

CGGLDLL_API void cgGLSetParameter2d(CGparameter param,
                                    double x,
                                    double y);

CGGLDLL_API void cgGLSetParameter3d(CGparameter param,
                                    double x,
                                    double y,
                                    double z);

CGGLDLL_API void cgGLSetParameter4d(CGparameter param,
                                    double x,
                                    double y,
                                    double z,
                                    double w);

CGGLDLL_API void cgGLSetParameter1dv(CGparameter param, const double *v);

CGGLDLL_API void cgGLSetParameter2dv(CGparameter param, const double *v);

CGGLDLL_API void cgGLSetParameter3dv(CGparameter param, const double *v);

CGGLDLL_API void cgGLSetParameter4dv(CGparameter param, const double *v);

CGGLDLL_API void cgGLSetParameter4dv(CGparameter param, const double *v);

CGGLDLL_API void cgGLGetParameter1f(CGparameter param, float *v);

CGGLDLL_API void cgGLGetParameter2f(CGparameter param, float *v);

CGGLDLL_API void cgGLGetParameter3f(CGparameter param, float *v);

CGGLDLL_API void cgGLGetParameter4f(CGparameter param, float *v);

CGGLDLL_API void cgGLGetParameter1d(CGparameter param, double *v);

CGGLDLL_API void cgGLGetParameter2d(CGparameter param, double *v);

CGGLDLL_API void cgGLGetParameter3d(CGparameter param, double *v);

CGGLDLL_API void cgGLGetParameter4d(CGparameter param, double *v);

CGGLDLL_API void cgGLSetParameterArray1f(CGparameter param,
                                         long offset,
                                         long nelements,
                                         const float *v);

CGGLDLL_API void cgGLSetParameterArray2f(CGparameter param,
                                         long offset,
                                         long nelements,
                                         const float *v);

CGGLDLL_API void cgGLSetParameterArray3f(CGparameter param,
                                         long offset,
                                         long nelements,
                                         const float *v);

CGGLDLL_API void cgGLSetParameterArray4f(CGparameter param,
                                         long offset,
                                         long nelements,
                                         const float *v);

CGGLDLL_API void cgGLSetParameterArray1d(CGparameter param,
                                         long offset,
                                         long nelements,
                                         const double *v);

CGGLDLL_API void cgGLSetParameterArray2d(CGparameter param,
                                         long offset,
                                         long nelements,
                                         const double *v);

CGGLDLL_API void cgGLSetParameterArray3d(CGparameter param,
                                         long offset,
                                         long nelements,
                                         const double *v);

CGGLDLL_API void cgGLSetParameterArray4d(CGparameter param,
                                         long offset,
                                         long nelements,
                                         const double *v);

CGGLDLL_API void cgGLGetParameterArray1f(CGparameter param,
                                         long offset,
                                         long nelements,
                                         float *v);

CGGLDLL_API void cgGLGetParameterArray2f(CGparameter param,
                                         long offset,
                                         long nelements,
                                         float *v);

CGGLDLL_API void cgGLGetParameterArray3f(CGparameter param,
                                         long offset,
                                         long nelements,
                                         float *v);

CGGLDLL_API void cgGLGetParameterArray4f(CGparameter param,
                                         long offset,
                                         long nelements,
                                         float *v);

CGGLDLL_API void cgGLGetParameterArray1d(CGparameter param,
                                         long offset,
                                         long nelements,
                                         double *v);

CGGLDLL_API void cgGLGetParameterArray2d(CGparameter param,
                                         long offset,
                                         long nelements,
                                         double *v);

CGGLDLL_API void cgGLGetParameterArray3d(CGparameter param,
                                         long offset,
                                         long nelements,
                                         double *v);

CGGLDLL_API void cgGLGetParameterArray4d(CGparameter param,
                                         long offset,
                                         long nelements,
                                         double *v);

CGGLDLL_API void cgGLSetParameterPointer(CGparameter param,
                                         GLint fsize,
                                         GLenum type,
                                         GLsizei stride,
                                         const GLvoid *pointer);

CGGLDLL_API void cgGLEnableClientState(CGparameter param);
CGGLDLL_API void cgGLDisableClientState(CGparameter param);

/******************************************************************************
 *** Matrix Parameter Managment Functions                                 
 *****************************************************************************/

CGGLDLL_API void cgGLSetMatrixParameterdr(CGparameter param, 
                                          const double *matrix);
CGGLDLL_API void cgGLSetMatrixParameterfr(CGparameter param, 
                                          const float *matrix);
CGGLDLL_API void cgGLSetMatrixParameterdc(CGparameter param, 
                                          const double *matrix);
CGGLDLL_API void cgGLSetMatrixParameterfc(CGparameter param, 
                                          const float *matrix);

CGGLDLL_API void cgGLGetMatrixParameterdr(CGparameter param, double *matrix);
CGGLDLL_API void cgGLGetMatrixParameterfr(CGparameter param, float *matrix);
CGGLDLL_API void cgGLGetMatrixParameterdc(CGparameter param, double *matrix);
CGGLDLL_API void cgGLGetMatrixParameterfc(CGparameter param, float *matrix);

CGGLDLL_API void cgGLSetStateMatrixParameter(CGparameter param, 
                                             CGGLenum matrix,
                                             CGGLenum transform);

CGGLDLL_API void cgGLSetMatrixParameterArrayfc(CGparameter param, 
                                               long offset,
                                               long nelements,
                                               const float *matrices);

CGGLDLL_API void cgGLSetMatrixParameterArrayfr(CGparameter param, 
                                               long offset,
                                               long nelements,
                                               const float *matrices);

CGGLDLL_API void cgGLSetMatrixParameterArraydc(CGparameter param, 
                                               long offset,
                                               long nelements,
                                               const double *matrices);

CGGLDLL_API void cgGLSetMatrixParameterArraydr(CGparameter param, 
                                               long offset,
                                               long nelements,
                                               const double *matrices);

CGGLDLL_API void cgGLGetMatrixParameterArrayfc(CGparameter param, 
                                               long offset,
                                               long nelements,
                                               float *matrices);

CGGLDLL_API void cgGLGetMatrixParameterArrayfr(CGparameter param, 
                                               long offset,
                                               long nelements,
                                               float *matrices);

CGGLDLL_API void cgGLGetMatrixParameterArraydc(CGparameter param, 
                                               long offset,
                                               long nelements,
                                               double *matrices);

CGGLDLL_API void cgGLGetMatrixParameterArraydr(CGparameter param, 
                                               long offset,
                                               long nelements,
                                               double *matrices);

/******************************************************************************
 *** Texture Parameter Managment Functions
 *****************************************************************************/

CGGLDLL_API void cgGLSetTextureParameter(CGparameter param, GLuint texobj);
CGGLDLL_API GLuint cgGLGetTextureParameter(CGparameter param);
CGGLDLL_API void cgGLEnableTextureParameter(CGparameter param);
CGGLDLL_API void cgGLDisableTextureParameter(CGparameter param);
CGGLDLL_API GLenum cgGLGetTextureEnum(CGparameter param);
CGGLDLL_API void cgGLSetManageTextureParameters(CGcontext ctx, CGbool flag);
CGGLDLL_API CGbool cgGLGetManageTextureParameters(CGcontext ctx);

#endif

#ifdef __cplusplus
}
#endif


#endif

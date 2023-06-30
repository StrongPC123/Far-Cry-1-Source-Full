
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

#ifndef _cgGL_deprecated_api_h
#define _cgGL_deprecated_api_h

#include <Cg/cg.h>

// Set up for either Win32 import/export/lib.
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
  cgGLMatrixIdentity = 0,
  cgGLMatrixTranspose = 1,
  cgGLMatrixInverse = 2,
 } cgGLMatrixFormat;

typedef enum 
 {
  cgGLModelViewMatrix,
  cgGLProjectionMatrix,
  cgGLTextureMatrix,
  cgGLModelViewProjectionMatrix,
 } cgGLMatrixType;

/*****************************************************************************/
/*** Resource Aliases                                                      ***/
/*****************************************************************************/

#define cgBindPosition cgBindPosition0 
#define cgBindNormal cgBindNormal0 
#define cgBindCol cgBindCol0 
#define cgBindColor cgBindColor0 
#define cgBindBlendWeight cgBindBlendWeight0 

/*****************************************************************************/
/*** Program Loading and Binding Functions                                 ***/
/*****************************************************************************/

CGGLDLL_API cgError cgGLLoadProgram(cgProgramIter *program, GLuint prog_id);
CGGLDLL_API cgError cgGLBindProgram(cgProgramIter *program);

CGGLDLL_API cgError cgGLEnableProgramType(cgProfileType profile);
CGGLDLL_API cgError cgGLDisableProgramType(cgProfileType profile);

/*****************************************************************************/
/*** Texture Unit Identifier Functions                                     ***/
/*****************************************************************************/

CGGLDLL_API GLenum cgGLTextureUnit(const cgBindIter *param);
CGGLDLL_API cgError cgGLActiveTexture(const cgBindIter *param);
CGGLDLL_API cgError cgGLClientActiveTexture(const cgBindIter *param);

/*****************************************************************************/
/*** Uniform Parameter Binding Functions                                   ***/
/*****************************************************************************/

CGGLDLL_API cgError cgGLBindUniform4f(cgProgramIter *program,
                                   cgBindIter *param, 
		                   float x,
		                   float y,
		                   float z,
		                   float w);

CGGLDLL_API cgError cgGLBindUniform4d(cgProgramIter *program,
                                   cgBindIter *param, 
		                   double x,
		                   double y,
		                   double z,
		                   double w);

CGGLDLL_API cgError cgGLBindUniform4fv(cgProgramIter *program,
                                    cgBindIter *param, 
                                    const float v[]); 

CGGLDLL_API cgError cgGLBindUniform4dv(cgProgramIter *program,
                                    cgBindIter *param, 
                                    const double v[]); 

CGGLDLL_API cgError cgGLBindUniformMatrixrf(cgProgramIter *program,
                                       cgBindIter *param, 
                                       const float *matrix);

CGGLDLL_API cgError cgGLBindUniformMatrixrd(cgProgramIter *program,
                                       cgBindIter *param, 
                                       const double *matrix);

CGGLDLL_API cgError cgGLBindUniformMatrixcf(cgProgramIter *program,
                                       cgBindIter *param, 
                                       const float *matrix);

CGGLDLL_API cgError cgGLBindUniformMatrixcd(cgProgramIter *program,
                                       cgBindIter *param, 
                                       const double *matrix);

CGGLDLL_API cgError cgGLBindUniformStateMatrix(cgProgramIter *program,
                                          cgBindIter *param,
                                          cgGLMatrixType matrix_type,
                                          int format);

/*****************************************************************************/
/*** Varying Parameter Binding Functions                                   ***/
/*****************************************************************************/

CGGLDLL_API cgError cgGLEnableClientState(cgProgramIter *program,
                                     cgBindIter *param);

CGGLDLL_API cgError cgGLDisableClientState(cgProgramIter *program,
                                      cgBindIter *param);

CGGLDLL_API cgError cgGLBindVaryingPointer(cgProgramIter *program,
                                      cgBindIter *param,
                                      GLint fsize,
                                      GLenum type,
                                      GLsizei stride,
                                      GLvoid *pointer);

CGGLDLL_API cgError cgGLBindVarying1s(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLshort x);

CGGLDLL_API cgError cgGLBindVarying1f(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLfloat x);

CGGLDLL_API cgError cgGLBindVarying1d(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLdouble x);

CGGLDLL_API cgError cgGLBindVarying2s(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLshort x,
                                 GLshort y);

CGGLDLL_API cgError cgGLBindVarying2f(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLfloat x,
                                 GLfloat y);

CGGLDLL_API cgError cgGLBindVarying2d(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLdouble x,
                                 GLdouble y);

CGGLDLL_API cgError cgGLBindVarying3s(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLshort x,
                                 GLshort y,
                                 GLshort z);

CGGLDLL_API cgError cgGLBindVarying3f(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLfloat x,
                                 GLfloat y,
                                 GLfloat z);

CGGLDLL_API cgError cgGLBindVarying3d(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLdouble x,
                                 GLdouble y,
                                 GLdouble z);

CGGLDLL_API cgError cgGLBindVarying4s(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLshort x,
                                 GLshort y,
                                 GLshort z,
                                 GLshort w);

CGGLDLL_API cgError cgGLBindVarying4f(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLfloat x,
                                 GLfloat y,
                                 GLfloat z,
                                 GLfloat w);

CGGLDLL_API cgError cgGLBindVarying4d(cgProgramIter *program,
                                 cgBindIter *param,
                                 GLdouble x,
                                 GLdouble y,
                                 GLdouble z,
                                 GLdouble w);

CGGLDLL_API cgError cgGLBindVarying4ub(cgProgramIter *program,
                                  cgBindIter *param,
                                  GLubyte x,
                                  GLubyte y,
                                  GLubyte z,
                                  GLubyte w);

CGGLDLL_API cgError cgGLBindVarying1sv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLshort *v);

CGGLDLL_API cgError cgGLBindVarying1fv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLfloat *v);

CGGLDLL_API cgError cgGLBindVarying1dv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLdouble *v);

CGGLDLL_API cgError cgGLBindVarying2sv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLshort *v);

CGGLDLL_API cgError cgGLBindVarying2fv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLfloat *v);

CGGLDLL_API cgError cgGLBindVarying2dv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLdouble *v);

CGGLDLL_API cgError cgGLBindVarying3sv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLshort *v);

CGGLDLL_API cgError cgGLBindVarying3fv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLfloat *v);

CGGLDLL_API cgError cgGLBindVarying3dv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLdouble *v);

CGGLDLL_API cgError cgGLBindVarying4sv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLshort *v);

CGGLDLL_API cgError cgGLBindVarying4fv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLfloat *v);

CGGLDLL_API cgError cgGLBindVarying4dv(cgProgramIter *program,
                                  cgBindIter *param,
                                  const GLdouble *v);

CGGLDLL_API cgError cgGLBindVarying4ubv(cgProgramIter *program,
                                   cgBindIter *param,
                                   const GLubyte *v);


#ifdef __cplusplus
}
#endif

#endif

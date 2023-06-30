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

#ifndef _cg_h
#define _cg_h

#ifndef CG_DEPRECATED_API

// Set up for either Win32 import/export/lib.
#ifndef CGDLL_API
#ifdef WIN32
    #ifdef CGDLL_EXPORTS
    #define CGDLL_API __declspec(dllexport)
    #elif defined (CG_LIB)
    #define CGDLL_API
    #else
    #define CGDLL_API __declspec(dllimport)
    #endif
#else
    #define CGDLL_API
#endif
#endif

/*************************************************************************/
/*** CG Run-Time Library API                                          ***/
/*************************************************************************/

/*************************************************************************/
/*** Data types and enumerants                                         ***/
/*************************************************************************/

typedef int CGbool;

#define CG_FALSE ((CGbool)0)
#define CG_TRUE ((CGbool)1)

typedef struct _CGcontext *CGcontext;
typedef struct _CGprogram *CGprogram;
typedef struct _CGparameter *CGparameter;


typedef enum
 {
  CG_UNKNOWN_TYPE,
  CG_STRUCT,
  CG_ARRAY,

  CG_TYPE_START_ENUM = 1024,
# define CG_DATATYPE_MACRO(name, compiler_name, enum_name, ncols, nrows) \
  enum_name ,

#include "cg_datatypes.h"

 } CGtype;

typedef enum
 {
# define CG_BINDLOCATION_MACRO(name,enum_name,compiler_name,\
                               enum_int,addressable,param_type) \
  enum_name = enum_int,

#include "cg_bindlocations.h"

  CG_UNDEFINED,

 } CGresource;

typedef enum
 {
  CG_PROFILE_START = 6144,
  CG_PROFILE_UNKNOWN,

# define CG_PROFILE_MACRO(name, compiler_id, compiler_id_caps, compiler_opt,int_id,vertex_profile) \
   CG_PROFILE_##compiler_id_caps = int_id,
  
#include "cg_profiles.h"

  CG_PROFILE_MAX,
 } CGprofile;

typedef enum
 {
# define CG_ERROR_MACRO(code, enum_name, new_enum_name, message) \
   new_enum_name = code,
# include "cg_errors.h"
 } CGerror;

typedef enum
 {
  CG_UNKNOWN = 4096,
  CG_IN,
  CG_OUT,
  CG_INOUT,
  CG_MIXED,
  CG_VARYING,
  CG_UNIFORM,
  CG_CONSTANT,
  CG_PROGRAM_SOURCE,
  CG_PROGRAM_ENTRY,
  CG_COMPILED_PROGRAM,
  CG_PROGRAM_PROFILE,

  CG_GLOBAL,
  CG_PROGRAM,

  CG_DEFAULT,
  CG_ERROR,

  CG_SOURCE,
  CG_OBJECT,

 } CGenum;

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*CGerrorCallbackFunc)(void);

/*************************************************************************/
/*** Functions                                                         ***/
/*************************************************************************/

/*** Context functions ***/

CGDLL_API CGcontext cgCreateContext(void); 
CGDLL_API void cgDestroyContext(CGcontext ctx); 
CGDLL_API CGbool cgIsContext(CGcontext ctx);
CGDLL_API const char *cgGetLastListing(CGcontext ctx);

/*** Program functions ***/
CGDLL_API CGprogram cgCreateProgram(CGcontext ctx, 
		                    CGenum program_type,
				    const char *program,
				    CGprofile profile,
				    const char *entry,
				    const char **args);
CGDLL_API CGprogram cgCreateProgramFromFile(CGcontext ctx, 
                                            CGenum program_type,
                                            const char *program_file,
                                            CGprofile profile,
                                            const char *entry,
                                            const char **args);
CGDLL_API CGprogram cgCopyProgram(CGprogram program); 
CGDLL_API void cgDestroyProgram(CGprogram program); 

CGDLL_API CGprogram cgGetFirstProgram(CGcontext ctx);
CGDLL_API CGprogram cgGetNextProgram(CGprogram current);
CGDLL_API CGcontext cgGetProgramContext(CGprogram prog);
CGDLL_API CGbool cgIsProgram(CGprogram program); 

CGDLL_API void cgCompileProgram(CGprogram program); 
CGDLL_API CGbool cgIsProgramCompiled(CGprogram program); 
CGDLL_API const char *cgGetProgramString(CGprogram prog, CGenum pname); 
CGDLL_API CGprofile cgGetProgramProfile(CGprogram prog); 

/*** Parameter functions ***/
CGDLL_API CGparameter cgGetNamedParameter(CGprogram prog, const char *name);
CGDLL_API CGparameter cgGetFirstParameter(CGprogram prog, CGenum name_space);
CGDLL_API CGparameter cgGetNextParameter(CGparameter current);
CGDLL_API CGparameter cgGetFirstLeafParameter(CGprogram prog, CGenum name_space);
CGDLL_API CGparameter cgGetNextLeafParameter(CGparameter current);


CGDLL_API CGparameter cgGetFirstStructParameter(CGparameter param);
CGDLL_API CGparameter cgGetFirstDependentParameter(CGparameter param);

CGDLL_API CGparameter cgGetArrayParameter(CGparameter aparam, int index);
CGDLL_API int cgGetArrayDimension(CGparameter param);
CGDLL_API int cgGetArraySize(CGparameter param, int dimension);

CGDLL_API CGprogram cgGetParameterProgram(CGparameter prog);
CGDLL_API CGbool cgIsParameter(CGparameter param);
CGDLL_API const char *cgGetParameterName(CGparameter param);
CGDLL_API CGtype cgGetParameterType(CGparameter param);
CGDLL_API const char *cgGetParameterSemantic(CGparameter param);
CGDLL_API CGresource cgGetParameterResource(CGparameter param);
CGDLL_API CGresource cgGetParameterBaseResource(CGparameter param);
CGDLL_API unsigned long cgGetParameterResourceIndex(CGparameter param);
CGDLL_API CGenum cgGetParameterVariability(CGparameter param);
CGDLL_API CGenum cgGetParameterDirection(CGparameter param);
CGDLL_API CGbool cgIsParameterReferenced(CGparameter param);
CGDLL_API const double *cgGetParameterValues(CGparameter param, 
                                             CGenum value_type,
                                             int *nvalues);
CGDLL_API int cgGetParameterOrdinalNumber(CGparameter param);

/*** Type Functions ***/

CGDLL_API const char *cgGetTypeString(CGtype type);
CGDLL_API CGtype cgGetType(const char *type_string);

/*** Resource Functions ***/

CGDLL_API const char *cgGetResourceString(CGresource resource);
CGDLL_API CGresource cgGetResource(const char *resource_string);

/*** Profile Functions ***/

CGDLL_API const char *cgGetProfileString(CGprofile profile);
CGDLL_API CGprofile cgGetProfile(const char *profile_string);

/*** Error Functions ***/
CGDLL_API CGerror cgGetError(void);
CGDLL_API const char *cgGetErrorString(CGerror error);
CGDLL_API void cgSetErrorCallback(CGerrorCallbackFunc func);
CGDLL_API CGerrorCallbackFunc cgGetErrorCallback(void);

#ifdef __cplusplus
}
#endif

#else

#define cgCreateContext cgGL_DEPRECATEDAPI_CreateContext
#define cgGetNextProgram cgGL_DEPRECATEDAPI_GetNextProgram
#define cgGetLastListing cgGL_DEPRECATEDAPI_GetLastListing
#define cgGetProgramProfile cgGL_DEPRECATEDAPI_ProgramProfile

# include "cg_deprecated_api.h"

#endif

#endif


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

#ifndef _cg_deprecated_api_h
#define _cg_deprecated_api_h


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
/*** Data types and structures                                         ***/
/*************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
 {
# define CG_ERROR_MACRO(code, enum_name, new_enum_name, message) \
   cg##enum_name = code,
# include <Cg/cg_errors.h>
 } cgError;

typedef struct
 {
  /* Opaque structure */
  void *Data;
 } cgContext;

typedef struct
 {
  /* Opaque structure */
  void *Data;
 } cgProgramIter;

typedef struct
 {
  /* Opaque structure */
  void *Data;
 } cgBindIter;

typedef enum
 {
  cgUnknownValueType,

  cgTypeStartEnum = 1024,
# define CG_DATATYPE_MACRO(name, compiler_name, enum_name, ncols, nrows) \
  cg##name##ValueType,

#include <Cg/cg_datatypes.h>
 } cgValueType;

typedef enum
 {
  cgConstParam,
  cgUniformParam,
  cgConnectorMemberParam,
  cgVaryingParam = cgConnectorMemberParam,
  cgTexObjParam,
  cgAnyParam,
  cgUndefinedParam,
 } cgParamType;

typedef enum
 {

# define CG_BINDLOCATION_MACRO(name,enum_name,compiler_name,\
                               enum_int,addressable,param_type) \
  cgBind##name = enum_int,
#include <Cg/cg_bindlocations.h>

  cgBindUndefined
 } cgBindLocation;


typedef enum
 {
  cgProfileStart = 6144,

  CG_PROFILE_UNKNOWN,
  cgUnknownProfile = CG_PROFILE_UNKNOWN,

# define CG_PROFILE_MACRO(name, compiler_id, compiler_id_caps, compiler_opt,int_id,vertex_profile) \
   cg##name##Profile = int_id,
# include "Cg/cg_profiles.h"

  cgNProfiles,
 } cgProfileType;



/*************************************************************************/
/*** Function Prototypes                                               ***/
/*************************************************************************/


/*******************************************************************/
/*** cg Misc Functions                                         ****/
/*******************************************************************/

CGDLL_API void cgCleanup(void);

/* This will probably be deprecated in future releases */
CGDLL_API void cgSetCompilerExe(const char *exe);

CGDLL_API const char *cgErrorMsg(cgError error);

/*******************************************************************/
/*** cgContext Functions                                       ****/
/*******************************************************************/

CGDLL_API cgContext *cgCreateContext(void);

CGDLL_API void cgFreeContext(cgContext *context);

CGDLL_API cgError cgAddProgram(cgContext *context,
                            const char *text,
                            cgProfileType profile,
                            const char *entry);

CGDLL_API cgError cgAddProgramFromFile(cgContext *context,
                                    const char *filename,
                                    cgProfileType profile,
                                    const char *entry);

CGDLL_API cgError cgAddProgramArgs(cgContext *context,
                                const char *text,
                                cgProfileType profile,
                                const char *entry,
                                int nargs,
                                const char **args);

CGDLL_API cgError cgAddProgramFromFileArgs(cgContext *context,
                                        const char *filename,
                                        cgProfileType profile,
                                        const char *entry,
                                        int nargs,
                                        const char **args);

CGDLL_API cgError cgAddPrecompiledProgram(cgContext *context,
                                       const char *text,
                                       cgProfileType profile,
                                       const char *entry);

CGDLL_API cgError cgAddPrecompiledProgramFromFile(cgContext *context,
                                               const char *filename,
                                               cgProfileType profile,
                                               const char *entry);

CGDLL_API const char *cgGetLastListing(cgContext *context);

/*******************************************************************/
/*** cgProgram Functions                                        ***/
/*******************************************************************/

CGDLL_API cgProgramIter *cgGetNextProgram(cgContext *context,
                                       cgProgramIter *iter);

CGDLL_API cgProgramIter *cgProgramByName(cgContext *context, 
                                      const char *name);


CGDLL_API cgProgramIter *cgGetLastProgram(cgContext *context);

CGDLL_API void cgFreeProgramIter(cgProgramIter *program);

CGDLL_API const char *cgGetProgramName(const cgProgramIter *program);

CGDLL_API cgProfileType cgGetProgramProfile(const cgProgramIter *program);

CGDLL_API const char *cgGetProgramObjectCode(const cgProgramIter *program);



/*******************************************************************/
/*** Parameter and Binding Functions                            ****/
/*******************************************************************/

CGDLL_API cgBindIter *cgGetNextBind(cgProgramIter *program,
                                 cgBindIter *iter);

CGDLL_API cgBindIter *cgGetBindByName(cgProgramIter *program, 
                                   const char *parameter_name);

CGDLL_API cgBindIter *cgDuplicateBindIter(cgBindIter *bind);

CGDLL_API void cgFreeBindIter(cgBindIter *iter);

CGDLL_API const char *cgGetBindParamName(const cgBindIter *bind);

CGDLL_API cgParamType cgGetBindParamType(const cgBindIter *bind);

CGDLL_API cgBindLocation cgGetBindLocation(const cgBindIter *bind);

CGDLL_API unsigned long cgGetBindAddress(const cgBindIter *bind);

CGDLL_API cgValueType cgGetBindValueType(const cgBindIter *bind,
                                      int *array_size);

#ifdef __cplusplus
}
#endif



#endif

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

/* 
 * The following macro invocations define error codes returned by various cg 
 * API functions.
 *
 * The macros have the form :
 *
 *   CG_ERROR_MACRO(code, enum_name, message)
 *
 *     code      : The integer error code associated with the error.
 *     enum_name : The name of enumerant of the error code.  
 *                  (Note : the prefix "cg" will be added to the name).
 *     new_enum_name : The name of enumerant of the error code in the new API.  
 *     message   : A description string associated with the error.
 *   
 */


CG_ERROR_MACRO(0, 
               NoError,
               CG_NO_ERROR,
               "No error has occurred.")

CG_ERROR_MACRO(1, 
               CompileError, 
               CG_COMPILER_ERROR, 
               "The compile returned an error.")

CG_ERROR_MACRO(2, 
               InvalidParameterError,
               CG_INVALID_PARAMETER_ERROR,
               "The parameter used is invalid.")

CG_ERROR_MACRO(3, 
               InvalidProfileError, 
               CG_INVALID_PROFILE_ERROR,
               "The profile is not supported.")

CG_ERROR_MACRO(4, 
               ProgramLoadError,
               CG_PROGRAM_LOAD_ERROR,
               "The program could not load.")

CG_ERROR_MACRO(5, 
               ProgramBindError, 
               CG_PROGRAM_BIND_ERROR, 
               "The program could not bind.")

CG_ERROR_MACRO(6, 
               ProgramNotLoadedError,
               CG_PROGRAM_NOT_LOADED_ERROR, 
               "The program must be loaded before this operation may be used.")

CG_ERROR_MACRO(7, 
               UnsupportedGLExtensionError,
               CG_UNSUPPORTED_GL_EXTENSION_ERROR, 
               "An unsupported GL extension was required to perform this operation.")

CG_ERROR_MACRO(8, 
               InvalidValueTypeError,
               CG_INVALID_VALUE_TYPE_ERROR, 
               "An unknown value type was assigned to a parameter.")

CG_ERROR_MACRO(9, 
               NotMatrixParamError,
               CG_NOT_MATRIX_PARAM_ERROR, 
               "The parameter is not of matrix type.")

CG_ERROR_MACRO(10, 
               InvalidEnumerantError,
               CG_INVALID_ENUMERANT_ERROR, 
               "The enumerant parameter has an invalid value.")

CG_ERROR_MACRO(11, 
               Not4x4MatrixError,
               CG_NOT_4x4_MATRIX_ERROR, 
               "The parameter must be a 4x4 matrix type.")

CG_ERROR_MACRO(12, 
               FileReadError,
               CG_FILE_READ_ERROR, 
               "The file could not be read.")

CG_ERROR_MACRO(13, 
               FileWriteError,
               CG_FILE_WRITE_ERROR, 
               "The file could not be written.")

CG_ERROR_MACRO(14, 
               NVParseError,
               CG_NVPARSE_ERROR, 
               "nvparse could not successfully parse the output from the Cg "
               "compiler backend.")

CG_ERROR_MACRO(15, 
               MemoryAllocError,
               CG_MEMORY_ALLOC_ERROR, 
               "Memory allocation failed.")

CG_ERROR_MACRO(16, 
               InvalidContextHandleError,
               CG_INVALID_CONTEXT_HANDLE_ERROR, 
               "Invalid context handle.")

CG_ERROR_MACRO(17, 
               InvalidProgramHandleError,
               CG_INVALID_PROGRAM_HANDLE_ERROR, 
               "Invalid program handle.")

CG_ERROR_MACRO(18, 
               InvalidParamHandleError,
               CG_INVALID_PARAM_HANDLE_ERROR, 
               "Invalid parameter handle.")

CG_ERROR_MACRO(19, 
               UnknownProfileError,
               CG_UNKNOWN_PROFILE_ERROR, 
               "The specified profile is unknown.")

CG_ERROR_MACRO(20, 
               VarArgError,
               CG_VAR_ARG_ERROR, 
               "The variable arguments were specified incorrectly.")

CG_ERROR_MACRO(21, 
               InvalidDimensionError,
               CG_INVALID_DIMENSION_ERROR, 
               "The dimension value is invalid.")

CG_ERROR_MACRO(22, 
               ArrayParamError,
               CG_ARRAY_PARAM_ERROR, 
               "The parameter must be an array.")

CG_ERROR_MACRO(23, 
               OutOfArrayBoundsError,
               CG_OUT_OF_ARRAY_BOUNDS_ERROR, 
               "Index into the array is out of bounds.")



#undef CG_ERROR_MACRO


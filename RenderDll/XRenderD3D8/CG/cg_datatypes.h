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
 * The following macro invocations define the supported CG basic data types.
 *
 * The macros have the form :
 *
 *   CG_DATATYPE_MACRO(name, compiler_name, nrows, ncols)
 *
 *     name          : The name of the data type.
 *     compiler_name : The name of the data type within the compiler syntax.
 *     enum_name     : The C enumerant.
 *     nrows         : Number of rows for matrix types.  Should be 0 other-wise.
 *     ncols         : Number of columns for matrix types.  Should be 0 
 *                     other-wise.
 *   
 */



CG_DATATYPE_MACRO(Half,half,CG_HALF,0,0)
CG_DATATYPE_MACRO(Half2,half2,CG_HALF2,0,0)
CG_DATATYPE_MACRO(Half3,half3,CG_HALF3,0,0)
CG_DATATYPE_MACRO(Half4,half4,CG_HALF4,0,0)
CG_DATATYPE_MACRO(Half1x1,half1x1,CG_HALF1x1,1,1)
CG_DATATYPE_MACRO(Half1x2,half1x2,CG_HALF1x2,1,2)
CG_DATATYPE_MACRO(Half1x3,half1x3,CG_HALF1x3,1,3)
CG_DATATYPE_MACRO(Half1x4,half1x4,CG_HALF1x4,1,4)
CG_DATATYPE_MACRO(Half2x1,half2x1,CG_HALF2x1,2,1)
CG_DATATYPE_MACRO(Half2x2,half2x2,CG_HALF2x2,2,2)
CG_DATATYPE_MACRO(Half2x3,half2x3,CG_HALF2x3,2,3)
CG_DATATYPE_MACRO(Half2x4,half2x4,CG_HALF2x4,2,4)
CG_DATATYPE_MACRO(Half3x1,half3x1,CG_HALF3x1,3,1)
CG_DATATYPE_MACRO(Half3x2,half3x2,CG_HALF3x2,3,2)
CG_DATATYPE_MACRO(Half3x3,half3x3,CG_HALF3x3,3,3)
CG_DATATYPE_MACRO(Half3x4,half3x4,CG_HALF3x4,3,4)
CG_DATATYPE_MACRO(Half4x1,half4x1,CG_HALF4x1,4,1)
CG_DATATYPE_MACRO(Half4x2,half4x2,CG_HALF4x2,4,2)
CG_DATATYPE_MACRO(Half4x3,half4x3,CG_HALF4x3,4,3)
CG_DATATYPE_MACRO(Half4x4,half4x4,CG_HALF4x4,4,4)
CG_DATATYPE_MACRO(Float,float,CG_FLOAT,0,0)
CG_DATATYPE_MACRO(Float2,float2,CG_FLOAT2,0,0)
CG_DATATYPE_MACRO(Float3,float3,CG_FLOAT3,0,0)
CG_DATATYPE_MACRO(Float4,float4,CG_FLOAT4,0,0)
CG_DATATYPE_MACRO(Float1x1,float1x1,CG_FLOAT1x1,1,1)
CG_DATATYPE_MACRO(Float1x2,float1x2,CG_FLOAT1x2,1,2)
CG_DATATYPE_MACRO(Float1x3,float1x3,CG_FLOAT1x3,1,3)
CG_DATATYPE_MACRO(Float1x4,float1x4,CG_FLOAT1x4,1,4)
CG_DATATYPE_MACRO(Float2x1,float2x1,CG_FLOAT2x1,2,1)
CG_DATATYPE_MACRO(Float2x2,float2x2,CG_FLOAT2x2,2,2)
CG_DATATYPE_MACRO(Float2x3,float2x3,CG_FLOAT2x3,2,3)
CG_DATATYPE_MACRO(Float2x4,float2x4,CG_FLOAT2x4,2,4)
CG_DATATYPE_MACRO(Float3x1,float3x1,CG_FLOAT3x1,3,1)
CG_DATATYPE_MACRO(Float3x2,float3x2,CG_FLOAT3x2,3,2)
CG_DATATYPE_MACRO(Float3x3,float3x3,CG_FLOAT3x3,3,3)
CG_DATATYPE_MACRO(Float3x4,float3x4,CG_FLOAT3x4,3,4)
CG_DATATYPE_MACRO(Float4x1,float4x1,CG_FLOAT4x1,4,1)
CG_DATATYPE_MACRO(Float4x2,float4x2,CG_FLOAT4x2,4,2)
CG_DATATYPE_MACRO(Float4x3,float4x3,CG_FLOAT4x3,4,3)
CG_DATATYPE_MACRO(Float4x4,float4x4,CG_FLOAT4x4,4,4)
CG_DATATYPE_MACRO(Sampler1D,sampler1D,CG_SAMPLER1D,0,0)
CG_DATATYPE_MACRO(Sampler2D,sampler2D,CG_SAMPLER2D,0,0)
CG_DATATYPE_MACRO(Sampler3D,sampler3D,CG_SAMPLER3D,0,0)
CG_DATATYPE_MACRO(SamplerRECT,samplerRECT,CG_SAMPLERRECT,0,0)
CG_DATATYPE_MACRO(SamplerCUBE,samplerCUBE,CG_SAMPLERCUBE,0,0)
CG_DATATYPE_MACRO(Fixed,fixed,CG_FIXED,0,0)
CG_DATATYPE_MACRO(Fixed2,fixed2,CG_FIXED2,0,0)
CG_DATATYPE_MACRO(Fixed3,fixed3,CG_FIXED3,0,0)
CG_DATATYPE_MACRO(Fixed4,fixed4,CG_FIXED4,0,0)
CG_DATATYPE_MACRO(Fixed1x1,fixed1x1,CG_FIXED1x1,1,1)
CG_DATATYPE_MACRO(Fixed1x2,fixed1x2,CG_FIXED1x2,1,2)
CG_DATATYPE_MACRO(Fixed1x3,fixed1x3,CG_FIXED1x3,1,3)
CG_DATATYPE_MACRO(Fixed1x4,fixed1x4,CG_FIXED1x4,1,4)
CG_DATATYPE_MACRO(Fixed2x1,fixed2x1,CG_FIXED2x1,2,1)
CG_DATATYPE_MACRO(Fixed2x2,fixed2x2,CG_FIXED2x2,2,2)
CG_DATATYPE_MACRO(Fixed2x3,fixed2x3,CG_FIXED2x3,2,3)
CG_DATATYPE_MACRO(Fixed2x4,fixed2x4,CG_FIXED2x4,2,4)
CG_DATATYPE_MACRO(Fixed3x1,fixed3x1,CG_FIXED3x1,3,1)
CG_DATATYPE_MACRO(Fixed3x2,fixed3x2,CG_FIXED3x2,3,2)
CG_DATATYPE_MACRO(Fixed3x3,fixed3x3,CG_FIXED3x3,3,3)
CG_DATATYPE_MACRO(Fixed3x4,fixed3x4,CG_FIXED3x4,3,4)
CG_DATATYPE_MACRO(Fixed4x1,fixed4x1,CG_FIXED4x1,4,1)
CG_DATATYPE_MACRO(Fixed4x2,fixed4x2,CG_FIXED4x2,4,2)
CG_DATATYPE_MACRO(Fixed4x3,fixed4x3,CG_FIXED4x3,4,3)
CG_DATATYPE_MACRO(Fixed4x4,fixed4x4,CG_FIXED4x4,4,4)
CG_DATATYPE_MACRO(Half1,half1,CG_HALF1,0,0)
CG_DATATYPE_MACRO(Float1,float1,CG_FLOAT1,0,0)
CG_DATATYPE_MACRO(Fixed1,fixed1,CG_FIXED1,0,0)

#undef CG_DATATYPE_MACRO



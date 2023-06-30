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

/*********************************************************************NVMH2****
File:  cgContextManager.h

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:

cgContextManager - a class for loading various cg programs,


******************************************************************************/



#ifndef __NV_CG_MANAGER_H
#define	__NV_CG_MANAGER_H

#if WIN32

#pragma warning (disable:4786) // identifier was truncated to '255' characters in the browser information

#include <windows.h>
#include <d3d8.h>
#include <d3dx8.h>

#include <Cg/cgTemplates.h>
#include <Cg/cg.h>
#include <Cg/cgProgramManager.h>


 



class cgContextManager;


class cgContextContainer
{
public:


    // retrieve the listing file for the last compile
    const char *GetLastListing();


    // loads and compiles the cg program
    cgProgramContainer * LoadCGProgramFromFile(
        const char * filename,           // filename of the Cg program
        const char * title,               // title of the program                               
        cgProfileType type,       // profile of the program                            
        cgVertexDefinition * va = 0, // optionally, set streams or allow different vetex definitions
        DWORD * outIndex = 0,
        const char * entry = 0);

    cgProgramContainer *  LoadCGProgramFromMemory(
        const char * memory,          // memory location that contain the cg program 
        const char * title,            // name of the program 
        cgProfileType type,    // profile of the program
        cgVertexDefinition * va = 0,   // optionally, set the streams for each vertex entry
        DWORD * outIndex = 0,
        const char * entry = 0);


    // use outIndex returned from LoadCGProgramFrom to retriece the program iterator
    cgProgramContainer * GetProgramIterator(DWORD index);


    // program operators (for multiple programs(vertex, pixel) in one cg
    // pass NULL to get first program
    cgProgramIter *GetNextProgram(cgProgramIter *iter);

    // get a specific program
    cgProgramIter *ProgramByName(const char *name);


    // get the vertexshader or pixel shader handle that is passed to 
    // SetVertexShader and SetPixelShader
    DWORD GetShaderHandle(cgProfileType type)
    {
        return pm.GetShaderHandle( type);
    }

    ~cgContextContainer( );

    friend class cgContextManager;
protected:

    cgContextContainer( LPDIRECT3DDEVICE8 pd3dDevice, cgContext * context);


private:


    cgProgramContainer * CreateProgramContainer(DWORD * outIndex = 0);

    cgProgramContainer * CreateProgramContainer( 
        cgProgramIter *ProgramIter, 
        LPDIRECT3DDEVICE8 pd3dDevice, 
        DWORD * outIndex = 0);       // index of this 

    // loads and compiles the cg program
    HRESULT AddProgramFromFile(const char * filename, const char * name, cgProfileType type, const char * entry = 0);
    HRESULT AddProgramFromMemory(const char * memory, const char * name, cgProfileType type, const char * entry = 0);


    void Free();
    cgProgramManager pm; // manage the vertex or pixel shader that is derived from this
    cgContext * m_Context;
    cg_string		m_ContextDefinition;
    LPDIRECT3DDEVICE8 m_pd3dDevice;

};



class cgContextManager
{
private:

	cg_vector < cgContextContainer *>	m_ContextContainers;		
	cg_vector < DWORD >		        m_ContextIndices;	// indirection to m_Shaders
											// Programs hold an index to this array for
											// their shaders.  The array int at that index
											// holds the index into the m_Shaders array.

    // each context has a list of programs (up to one for each profile)


public:
	cgContextManager();
	~cgContextManager();

	cgError Free();

    cgContextContainer  *CreateContextContainer(LPDIRECT3DDEVICE8 pd3dDevice, DWORD * outIndex);



};


#endif // WIN32

#endif

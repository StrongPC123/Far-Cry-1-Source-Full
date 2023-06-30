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
File:  cgProgramManager.h

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:

cgProgramManager - a class for loading various pixel & vertex shaders


******************************************************************************/


#ifndef __NV_PROGRAM_MANAGER_H
#define	__NV_PROGRAM_MANAGER_H

#if WIN32

#pragma warning (disable:4786) // identifier was truncated to '255' characters in the browser information

#include <windows.h>
#include <d3d8.h>
#include <d3dx8.h>

#include <Cg/cgTemplates.h>
#include <Cg/cg.h>



// to allow multiple streams and user specifed vertex type
// D3DVSDT_FLOAT1      0x00    // 1D float expanded to (value, 0., 0., 1.)
// D3DVSDT_FLOAT2      0x01    // 2D float expanded to (value, value, 0., 1.)
// D3DVSDT_FLOAT3      0x02    // 3D float expanded to (value, value, value, 1.)
// D3DVSDT_FLOAT4      0x03    // 4D float
// D3DVSDT_D3DCOLOR    0x04    // 4D packed unsigned bytes mapped to 0. to 1. range
//                             // Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
// D3DVSDT_UBYTE4      0x05    // 4D unsigned byte
// D3DVSDT_SHORT2      0x06    // 2D signed short expanded to (value, value, 0., 1.)
// D3DVSDT_SHORT4      0x07    // 4D signed short


// was cgVertexAttribute
typedef struct cgVertexDefinition
{
    int D3DVSDT_type;       // one of D3DVSDT_*
    char * name;    // name of the entry
    int stream;     // stream to put the vertex in
} cgVertexDefinition;

       
#define CGVERTEXDEFINITIONEND {-1, 0, 0}

// internal cgDirect3D data structure
typedef struct cgiVertexAttribute
{
    char name[64];
    int D3DVSDT_type;
    int bind_location;
    cgBindIter * BindIter;
} cgiVertexAttribute;



typedef cg_vector<cgiVertexAttribute> cgiVertexAttributeList;
typedef cg_list<cgBindIter *> cgBindIterList;


class cgProgramManager;
class cgContextContainer;

class cgProgramContainer
{

public:

    // get a text definition of what the runtime expects for the vertex
    const char * GetVertexDeclaration();
    /*
        Example: returns string containing
        "struct stream0
        {
            D3DXVECTOR3 position;
            D3DXVECTOR2 tex0;
        };"

    */



    
    // convert D3DXMATRIX class to type specified in cg program
    // even if your cg program uses 3x3, you can use this to pass a matrix to it
    // upper left hand part of D3DXMATRIX is used to fill the cg matrix
    HRESULT SetShaderConstantD3DXMATRIX(cgBindIter * iter, const D3DXMATRIX *, int nArrayElements = 0, int ArrayOffset = 0);



    // All other data formats use this interface
    // The D3D runtime records the size of your data and puts that in the constant fields
    HRESULT SetShaderConstant(cgBindIter * iter, void * data, int nArrayElements = 0, int ArrayOffset = 0);
    // for arrays:
    //   if nArrayElements is specified, write this number of elements rather than the whole array
    //   if ArrayOffset is specified, start writing at this array element
    // otherwise nArrayElements and ArrayOffset are not specified


    // returns texture position 0-8 for the iterator
    // -1 is failure
    int GetTexturePosition(cgBindIter * BindIter);
    int GetTexturePosition(const char * name);

    // set the texture via name (needs to search bindings by name, so this is the slowest
    // not recommended
    HRESULT SetTexture(const char * name, LPDIRECT3DBASETEXTURE8 pTexture);

    // does a GetTexturePosition, then settexture(n, nTexture)
    HRESULT SetTexture(cgBindIter * BindIter, LPDIRECT3DBASETEXTURE8 pTexture);

    // does a GetTexturePosition, then SetTextureStageState(n, nTexture, Value)
    HRESULT SetTextureStageState(cgBindIter * BindIter, D3DTEXTURESTAGESTATETYPE Type,DWORD Value);

    // Direct3D hardcodes wrap state, D3DRS_WRAP1, D3DWRAPCOORD_0 | D3DWRAPCOORD_1
    //   WrapCoords : 0, D3DWRAPCOORD_0, D3DWRAPCOORD_1 or'd together
    HRESULT SetTextureWrapMode(cgBindIter * BindIter, DWORD WrapCoords);


    // activate this shader (SetVertexShader or SetPixelShader)
    HRESULT SetShaderActive();


    // locate a binding within a program by parameter name
    // it is recommended to do this once and use the cgBindIter to reference data
    cgBindIter *GetVertexBindByName(const char *vertex_attr_name);
    cgBindIter *GetTextureBindByName(const char *texture_name);
    cgBindIter *GetParameterBindByName(const char *parameter_name);
    
    void FreeBindIter(cgBindIter *);




    // Get the type for this bind iterator
    cgValueType GetBindValueType(const cgBindIter *BindIter,  int *array_size)
    {
        return cgGetBindValueType(BindIter,  array_size);
    }


    // locate and bindinf iterator by full name 
    //  e.g. appdata.texcoord0
    cgBindIter *GetBindByFullName(const char *parameter_name);

    // from the iterator, get the name of the program
    const char *GetProgramName();
    
    const char *GetProgramObjectCode();

    cgProfileType GetProgramProfile() { return m_ProfileType;}

    // get the shader handle passed to Direct3D
    DWORD GetShaderHandle() {return m_dwShaderHandle;}  

private:

    cgBindIter *AllocBind();
    void SaveBind(cgBindIter * BindIter);  // for deleting

    cgBindIterList m_BindIterList;

    void GetParameterConstantPosition(cgBindIter * iter, int * pos, int *width, int * num4vectors, int *nArrayElements);
    void GetParameterConstantPosition(const char * name, int * pos, int *width, int * num4vectors, int *nArrayElements);


    HRESULT CreateProgramShader();
    HRESULT CreateVertexShader();
    HRESULT CreatePixelShader();
    
    // take asm code, assembles it and create a handle
	HRESULT	CreateShaderHandleFromFile( 
        const cg_string & strFilePath, 
        const cg_string & strSourceFile,
		const DWORD * pDeclaration,	
        DWORD Usage);
    
    HRESULT CreateShaderHandleFromMemory(
        const char * pShaderText, 
        const DWORD * pDeclaration,	
        DWORD Usage);


    HRESULT Free();

    
    // cgDX8VertexProfile or cgDX8PixelProfile (cg_profiles.h)
	// These types are defined with macros:
	//	CG_PROFILE_MACRO(DX8Vertex,dx8vs,"dx8vs")
	//	CG_PROFILE_MACRO(DX8Pixel,dx8ps,"dx8ps")


    friend  cgProgramManager;
    friend  cgContextContainer;
    DWORD		    m_dwShaderHandle;		// DWORD handle returned by DX8 runtime
	cgProfileType	m_ProfileType;          // profile for this program
    HRESULT SetVertexDefinition(cgVertexDefinition * streams);


    cgProgramContainer(LPDIRECT3DDEVICE8 pDev, cgProgramIter *m_ProgramIter,
						cgProfileType ProfileType);
    ~cgProgramContainer();


    HRESULT         LoadShaderConstants();
    cg_string		    m_ProgramSourceFile;	// text file name 
    cg_string		    m_ProgramObjectFile;		// compiled shader object file (.vso, .pso)

    // for declaration of vertex positions
    cgiVertexAttributeList m_VertexAttributes[MAX_STREAMS];

    cgProgramIter *m_ProgramIter;
	
 
	LPDIRECT3DDEVICE8 m_pd3dDevice;

};




class cgProgramManager
{
private:
	cg_vector < cgProgramContainer * >	m_ProgramContainers;			// m_Shaders[0] is always = 0
	cg_vector < DWORD >		        m_ProgramIndices;	// indirection to m_Shaders
											// Programs hold an index to this array for
											// their shaders.  The array int at that index
											// holds the index into the m_Shaders array.

public:


	cgProgramManager(LPDIRECT3DDEVICE8 pDev);
	~cgProgramManager();

	HRESULT Free();


    cgProgramContainer * CreateProgramContainer(LPDIRECT3DDEVICE8 pDev,
        cgProgramIter * ProgramIter,
        DWORD Usage,
        cgProfileType ProfileType,
        DWORD * outIndex );



	DWORD	GetShaderHandle( DWORD Index );
	DWORD	GetShaderHandle( cgProfileType ProfileType );

    // 
    cgProgramContainer * GetProgramIterator(DWORD index);



private:
    // for creating programs
    LPDIRECT3DDEVICE8 m_pd3dDevice;

};

#endif // WIN32

#endif			

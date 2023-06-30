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

#ifndef _cgD3D_h
#define _cgD3D_h

#if WIN32

#include <Cg/cg.h>
#include <assert.h>


#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <stdio.h>
#include <d3d8.h>
#include <d3dx8.h>


#define MAX_STREAMS 16

#include <Cg/cgProgramManager.h>
#include <Cg/cgContextManager.h>
           



////////////////////////////////////////////////
//
//	cgDX8Vertex  and cgDX8Pixel are defined with
//  macros:
//
//	CG_PROFILE_MACRO(DX8Vertex,dx8vs,"dx8vs")
//	CG_PROFILE_MACRO(DX8Pixel,dx8ps,"dx8ps")
//
//	cgDX8VertexProfile




class cgDirect3D
{
public:

    cgDirect3D();
    ~cgDirect3D();

    // add a path to locate the cg programs
    // do not add the trailing '\\'
    void AddFilePath(const char * dirpath);
    void AddFilePath(cg_string & path);
     
    
    void NotePad(const char * title, const char * listing )
    {
        
        cg_string text;
        text = title;

        if(listing != NULL)
          text += listing;
         else
          text += "*** NO LISTING ***\n";
        
        char * TmpLstFile = tmpnam(0);
        
        FILE * fpLst = fopen(TmpLstFile, "w");
        fputs(text.c_str(), fpLst);
        fclose(fpLst);
        
        char Cmd[1000];
        sprintf(Cmd, "notepad %s", TmpLstFile);
        int Ret =  system(Cmd);
        
        unlink(TmpLstFile);
        
    }

    cgContextContainer * CreateContextContainer(LPDIRECT3DDEVICE8 pd3dDevice, DWORD * outIndex = 0)
    {
        return cm.CreateContextContainer(pd3dDevice,  outIndex);
    }
    

private:
    cgContextManager cm;
    LPDIRECT3DDEVICE8 m_pd3dDevice;
    
};
                            








#define CGRTERR_ILLEGAL_PROFILE -1
#define CGRTERR_VERTEX_ATTRIBUTES_NOT_SET -2

#endif // WIN32

#endif


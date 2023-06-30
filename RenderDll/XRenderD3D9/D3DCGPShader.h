/*=============================================================================
  D3DCGPShader.h : Direct3D9 CG pixel shaders interface declaration.
  Copyright 1999 K&M. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#ifndef __D3DCGPSHADER_H__
#define __D3DCGPSAHDER_H__

#include "cg\cgD3D9.h"
#include <direct.h>

#define CG_FP_CACHE_VER    3.4

#define GL_OFFSET_TEXTURE_2D_MATRIX_NV      0x86E1

class CCGPShader_D3D : public CPShader
{
  SCGScript *m_DeclarationsScript;
  SCGScript *m_CoreScript;
  SCGScript *m_InputParmsScript;
  SCGScript *m_SubroutinesScript;

  bool ActivateCacheItem(SShaderCacheHeaderItem *pItem);
  bool CreateCacheItem(int nMask, byte *pData, int nLen);

  int m_CurInst;
  int m_dwFrame;
  struct SCacheInstance
  {
    int m_Mask;
    SShaderCache *m_pCache;
    SCacheInstance()
    {
      m_Mask = 0;
      m_pCache = NULL;
    }
    int Size()
    {
      int nSize = sizeof(*this);
      return nSize;
    }
  };
  struct SCGInstance
  {
    void *m_pHandle;

    int m_Mask;
    int m_LightMask;
    CName m_PosScriptName;
    TArray<SCGParam4f> *m_ParamsNoObj;
    TArray<SCGParam4f> *m_ParamsObj;
    TArray<SCGMatrix> *m_MatrixObj;
    TArray<SCGBindConst> *m_BindConstants;
    TArray<SCGBind> *m_BindVars;
    int m_nCacheID;
    SCGInstance()
    {
      m_Mask = 0;
      m_LightMask = 0;
      m_ParamsNoObj = NULL;
      m_ParamsObj = NULL;
      m_MatrixObj = NULL;
      m_BindConstants = NULL;
      m_BindVars = NULL;
      m_pHandle = NULL;
      m_nCacheID = -1;
    }

    int Size()
    {
      int nSize = sizeof(*this);
      if (m_ParamsNoObj)
        nSize += m_ParamsNoObj->GetMemoryUsage() + 12;
      if (m_ParamsObj)
        nSize += m_ParamsObj->GetMemoryUsage() + 12;
      if (m_MatrixObj)
        nSize += m_MatrixObj->GetMemoryUsage() + 12;
      if (m_BindConstants)
        nSize += m_BindConstants->GetMemoryUsage() + 12;
      if (m_BindVars)
        nSize += m_BindVars->GetMemoryUsage() + 12;

      return nSize;
    }
  };
  TArray<SCGInstance> m_Insts;
  TArray<SCacheInstance> m_InstCache;
  TArray<SCGParam4f> m_ParamsNoObj;
  TArray<SCGParam4f> m_ParamsObj;

public:
  // FX support
  std::vector<SFXStruct> m_Functions;
  CName m_EntryFunc;

  virtual int Size()
  {
    int nSize = sizeof(*this);
    if (m_DeclarationsScript)
      nSize += m_DeclarationsScript->Size(false);
    if (m_CoreScript)
      nSize += m_CoreScript->Size(false);
    if (m_InputParmsScript)
      nSize += m_InputParmsScript->Size(false);
    if (m_SubroutinesScript)
      nSize += m_SubroutinesScript->Size(false);
    nSize += m_ParamsNoObj.GetMemoryUsage();
    nSize += m_ParamsObj.GetMemoryUsage();
    for (int i=0; i<m_Insts.GetSize(); i++)
    {
      if (i < m_Insts.Num())
      {
        SCGInstance *cgi = &m_Insts[i];
        nSize += cgi->Size();
      }
      else
        nSize += sizeof(SCGInstance);
    }
    return nSize;
  }
  int mfGetCacheInstanceID(int Mask, const char *name=NULL)
  {
    int i;

    for (i=0; i<m_InstCache.Num(); i++)
    {
      if (m_InstCache[i].m_Mask == Mask)
        return i;
    }
    if (!name)
      return -1;
    int nNum = m_InstCache.Num();
    SCacheInstance ci;
    ci.m_pCache = gRenDev->m_cEF.OpenCacheFile(name, (float)CG_FP_CACHE_VER);
    ci.m_Mask = Mask;
    m_InstCache.AddElem(ci);

    return nNum;
  }
  int mfGetCGInstanceID(int Type, int LightMask)
  {
    SCGInstance *cgi;
    if (m_CurInst >= 0 && m_Insts.Num() > m_CurInst)
    {
      cgi = &m_Insts[m_CurInst];
      if (cgi->m_Mask == Type && cgi->m_LightMask == LightMask)
        return m_CurInst;
    }
    m_dwFrame++;
    int i;
    for (i=0; i<m_Insts.Num(); i++)
    {
      cgi = &m_Insts[i];
      if (cgi->m_Mask == Type && cgi->m_LightMask == LightMask)
      {
        m_CurInst = i;
        return i;
      }
    }
    SCGInstance cg;
    cg.m_Mask = Type;
    cg.m_LightMask = LightMask;
    cg.m_pHandle = NULL;
    cg.m_ParamsNoObj = NULL;
    cg.m_ParamsObj = NULL;
    cg.m_BindConstants = NULL;
    cg.m_BindVars = NULL;
    cg.m_nCacheID = -1;

    m_CurInst = m_Insts.Num();
    m_Insts.AddElem(cg);
    return m_CurInst;
  }

  TArray<SCGParam4f> *mfGetParams(int Type)
  {
    if (!Type)
      return &m_ParamsNoObj;
    else
      return &m_ParamsObj;
  }

  CCGPShader_D3D(CGprofile ProfType)
  {
    m_CGProfileType = ProfType;
    mfInit();
  }
  CCGPShader_D3D()
  {
    mfInit();
    m_CGProfileType = CG_PROFILE_PS_1_1;
  }
  void mfSaveCGFile(const char *scr)
  {
    if (CRenderer::CV_r_shaderssave < 1)
      return;
    char name[1024];
    if (m_nMaskGen)
      sprintf(name, "%s$%x(%x).cg", m_Name.c_str(), m_Insts[m_CurInst].m_LightMask, m_nMaskGen);
    else
      sprintf(name, "%s.cg", m_Name.c_str());
    FILE *fp = fopen(name, "w");
    if (fp)
    {
      fprintf(fp, scr);
      fclose (fp);
    }
  }
  void mfInit()
  {
#ifndef WIN64
		// NOTE: AMD64 port: find the 64-bit CG runtime
    if (!gcpRendD3D->m_CGContext)
    {
      cgD3D9SetDevice(gcpRendD3D->mfGetD3DDevice());
      gcpRendD3D->m_CGContext = cgCreateContext();
      assert(gcpRendD3D->m_CGContext);
#ifdef _DEBUG
      cgD3D9EnableDebugTracing(true);
#endif
    }
#endif
    m_dwFrame = 1;
    m_CurInst = -1;
    m_CoreScript = NULL;
    m_InputParmsScript = NULL;
    m_SubroutinesScript = NULL;
    m_DeclarationsScript = NULL;
    m_bCGType = true;
  }

  void mfBind()
  {
    HRESULT hr;
    if (m_Insts[m_CurInst].m_pHandle)
    {
      hr = gcpRendD3D->mfGetD3DDevice()->SetPixelShader((IDirect3DPixelShader9 *)m_Insts[m_CurInst].m_pHandle);
      if (FAILED(hr))
        return;
    }
    if (m_Insts[m_CurInst].m_BindConstants)
    {
      int i;
      for (i=0; i<m_Insts[m_CurInst].m_BindConstants->Num(); i++)
      {
        SCGBindConst *p = &m_Insts[m_CurInst].m_BindConstants->Get(i);
        gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstantF(p->m_dwBind, &p->m_Val[0], 1);
      }
    }
  }

  void mfUnbind()
  {
    gcpRendD3D->mfGetD3DDevice()->SetPixelShader(NULL);
  }

  char *mfLoadCG_Int(char *prog_text)
  {
#ifndef WIN64
		// NOTE: AMD64 port: find the 64-bit CG runtime
    CGprofile pr = (CGprofile)m_CGProfileType;
    //if (pr == CG_PROFILE_PS_1_1)
    //  pr = CG_PROFILE_PS_1_3;
    // Use HLSL compiler for PS_2_X profile since it generates more efficient code (and bugs free)
    if (pr == CG_PROFILE_PS_2_0 || pr == CG_PROFILE_PS_2_X || pr == CG_PROFILE_PS_3_0)
    {
      remove("$$out.cg");

      // make command for execution
      FILE *fp = fopen("$$in.cg", "w");
      if (!fp)
        return NULL;
      fputs(prog_text, fp);
      fclose (fp);

      char szCmdLine[512];
      char *sz2AProfile;
      if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
      {
        if (CRenderer::CV_r_sm2xpath == 2)
          sz2AProfile = "ps_2_a";
        else
          sz2AProfile = "ps_2_b";
      }
      else
        sz2AProfile = "ps_2_b";
      sprintf(szCmdLine, "fxc.exe /T %s /DCGC=0 /Fc $$out.cg $$in.cg", pr == CG_PROFILE_PS_2_0 ? "ps_2_0" : pr == CG_PROFILE_PS_3_0 ? "ps_3_0" : sz2AProfile);
      if (m_EntryFunc.GetIndex())
      {
        strcat(szCmdLine, " /E ");
        strcat(szCmdLine, m_EntryFunc.c_str());
      }

      STARTUPINFO si;
      ZeroMemory( &si, sizeof(si) );
      si.cb = sizeof(si);
      si.dwX = 100;
      si.dwY = 100;
      si.dwFlags = STARTF_USEPOSITION;

      PROCESS_INFORMATION pi;
      ZeroMemory( &pi, sizeof(pi) );
      if( !CreateProcess( NULL, // No module name (use command line). 
        szCmdLine,				// Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        FALSE,            // Set handle inheritance to FALSE. 
        CREATE_NO_WINDOW, // No creation flags. 
        NULL,             // Use parent's environment block. 
        NULL/*szFolderName*/,     // Set starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
        ) 
      {
        iLog->LogError("CreateProcess failed: %s", szCmdLine);
        return NULL;
      }

      while (WAIT_OBJECT_0 != WaitForSingleObject (pi.hProcess, 10000))
        iLog->LogWarning ("CG runtime takes forever to compile.. waiting.. (last error %d)", GetLastError());

      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);

      fp = fopen("$$out.cg", "rb");
      if (!fp)
      {
        remove("$$in.cg");
        return NULL;
      }
      fseek(fp, 0, SEEK_END);
      int size = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      if (size < 20)
      {
        remove("$$in.cg");
        remove("$$out.cg");
        return NULL;
      }
      char *pBuf = new char[size+1];
      fread(pBuf, sizeof(char), size, fp);
      pBuf[size] = '\0';
      fclose(fp);

      if (CRenderer::CV_r_shaderssave == 2)
      {
        _chdir("c:\\MasterCD\\TestCG");
        mfSaveCGFile(prog_text);
        _chdir("c:\\MasterCD");
      }

      remove("$$in.cg");
      remove("$$out.cg");

      return pBuf;
    }
    else
    {
      char szParams[256];
      sprintf(szParams, "-DCGC=1");
      const char *profileOpts[] =
      {
        szParams,
        cgD3D9GetOptimalOptions(pr),
        NULL,
      };
      CGprogram cgPr;
      if (m_EntryFunc.GetIndex())
        cgPr = cgCreateProgram(gcpRendD3D->m_CGContext, CG_SOURCE, prog_text, pr, m_EntryFunc.c_str(), profileOpts);    
      else
        cgPr = cgCreateProgram(gcpRendD3D->m_CGContext, CG_SOURCE, prog_text, pr, "main", profileOpts);    
      CGerror err = cgGetError();
      if (err != CG_NO_ERROR)
        return NULL;
      if (!cgPr)
      {
        Warning( 0,0,"Couldn't find function '%s' in CG pixel program '%s'", "main", m_Name.c_str());
        return NULL;
      }
      if (CRenderer::CV_r_shaderssave == 2)
      {
        _chdir("c:\\MasterCD\\TestCG");
        mfSaveCGFile(prog_text);
        _chdir("c:\\MasterCD");
      }
      char *code = mfGetObjectCode(cgPr);     
      cgDestroyProgram(cgPr);
      return code;
    }
#else
    CGprofile pr = (CGprofile)m_CGProfileType;
    //if (pr == CG_PROFILE_PS_1_1)
    //  pr = CG_PROFILE_PS_1_3;
    // Use HLSL compiler for PS_2_X profile since it generates more efficient code (and bugs free)
    if (pr == CG_PROFILE_PS_2_0 || pr == CG_PROFILE_PS_2_X || pr == CG_PROFILE_PS_3_0)
    {
      // make command for execution
      FILE *fp = fopen("$$in.cg", "w");
      if (!fp)
        return NULL;
      fputs(prog_text, fp);
      fclose (fp);

      char szCmdLine[512];
      char *sz2AProfile = (gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX ? "ps_2_a" : "ps_2_b";
      sprintf(szCmdLine, "fxc.exe /T %s /DCGC=0 /Fc $$out.cg $$in.cg", pr == CG_PROFILE_PS_2_0 ? "ps_2_0" : pr == CG_PROFILE_PS_3_0 ? "ps_3_0" : sz2AProfile);

      STARTUPINFO si;
      ZeroMemory( &si, sizeof(si) );
      si.cb = sizeof(si);
      si.dwX = 100;
      si.dwY = 100;
      si.dwFlags = STARTF_USEPOSITION;

      PROCESS_INFORMATION pi;
      ZeroMemory( &pi, sizeof(pi) );
      if( !CreateProcess( NULL, // No module name (use command line). 
        szCmdLine,				// Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        FALSE,            // Set handle inheritance to FALSE. 
        CREATE_NO_WINDOW, // No creation flags. 
        NULL,             // Use parent's environment block. 
        NULL/*szFolderName*/,     // Set starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
        ) 
      {
        iLog->LogError("CreateProcess failed: %s", szCmdLine);
        return NULL;
      }

      while (WAIT_OBJECT_0 != WaitForSingleObject (pi.hProcess, 10000))
        iLog->LogWarning ("CG runtime takes forever to compile.. waiting.. (last error %d)", GetLastError());

      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);

      fp = fopen("$$out.cg", "rb");
      if (!fp)
      {
        return NULL;
      }
      fseek(fp, 0, SEEK_END);
      int size = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      if (size < 20)
      {
        remove("$$in.cg");
        remove("$$out.cg");
        return NULL;
      }
      char *pBuf = new char[size+1];
      fread(pBuf, sizeof(char), size, fp);
      pBuf[size] = '\0';
      fclose(fp);

      remove("$$in.cg");
      remove("$$out.cg");

      return pBuf;
    }
    else
    {
      // make command for execution
      FILE *fp = fopen("$$in.cg", "w");
      if (!fp)
        return NULL;
      assert (*prog_text);
      fputs(prog_text, fp);
      fclose (fp);

      char szCmdLine[512];
      sprintf(szCmdLine, "cgc.exe -DCGC=1 -profile ps_1_1 -o $$out.cg $$in.cg");

      STARTUPINFO si;
      ZeroMemory( &si, sizeof(si) );
      si.cb = sizeof(si);
      si.dwX = 100;
      si.dwY = 100;
      si.dwFlags = STARTF_USEPOSITION;

      PROCESS_INFORMATION pi;
      ZeroMemory( &pi, sizeof(pi) );
      if( !CreateProcess( NULL, // No module name (use command line). 
        szCmdLine,				// Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        FALSE,            // Set handle inheritance to FALSE. 
        CREATE_NO_WINDOW, // No creation flags. 
        NULL,             // Use parent's environment block. 
        NULL/*szFolderName*/,     // Set starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
        ) 
      {
        iLog->LogError("CreateProcess failed: %s", szCmdLine);
        return NULL;
      }

      while (WAIT_OBJECT_0 != WaitForSingleObject (pi.hProcess, 10000))
        iLog->LogWarning ("CG runtime takes forever to compile.. waiting.. (last error %d)", GetLastError());

      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);

      fp = fopen("$$out.cg", "rb");
      if (!fp)
      {
        remove("$$in.cg");
        remove("$$out.cg");
        return NULL;
      }
      fseek(fp, 0, SEEK_END);
      int size = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      if (size < 20)
      {
        remove("$$in.cg");
        remove("$$out.cg");
        return NULL;
      }
      char *pBuf = new char[size+1];
      fread(pBuf, sizeof(char), size, fp);
      pBuf[size] = '\0';
      fclose(fp);
      remove("$$in.cg");
      remove("$$out.cg");

      return pBuf;
    }
#endif
  }

  char *mfLoadCG(char *prog_text)
  {
    // Test adding source text to context
    char *pOut = mfLoadCG_Int(prog_text);
    if (!pOut)
    {
      if ((m_Flags & PSFI_AUTOENUMTC) && m_CGProfileType != CG_PROFILE_PS_3_0 && m_CGProfileType != CG_PROFILE_PS_2_X)
      {
        if (gRenDev->GetFeatures() & RFT_HW_PS20)
        {
          if ((gRenDev->GetFeatures() & RFT_HW_MASK) == RFT_HW_GFFX)
            m_CGProfileType = CG_PROFILE_PS_2_X;
          else
            m_CGProfileType = CG_PROFILE_PS_2_0;
          pOut = mfLoadCG_Int(prog_text);
          if (!pOut)
          {
            if (gRenDev->GetFeatures() & RFT_HW_PS30)
            {
              m_CGProfileType = CG_PROFILE_PS_3_0;
              pOut = mfLoadCG_Int(prog_text);
            }
          }
        }
      }
    }
    if (!pOut)
    {
      m_nFailed++;
      Warning( 0,0,"Couldn't create CG pixel program '%s'", m_Name.c_str());
      mfSaveCGFile(prog_text);
    }
    return pOut;
  }

  const char *mfSkipLine(const char *prog)
  {
    while (*prog != 0xa) {prog++;}
    prog++;
    return prog;
  }
  const char *mfGetLine(const char *prog, char *line)
  {
    while (*prog == 0x20 || *prog == 0x9) {prog++;}
    while (*prog != 0xa) {*line++ = *prog; prog++;}
    *line = 0;
    prog++;
    return prog;
  }

  const char *mfGetTexInstruction(const char *prog, int& Op0, int& Op1)
  {
    Op0 = -1;
    Op1 = -1;
    char line[128];
    const char *str = prog;
    while (str = mfGetLine(str, line))
    {
      if (!line[0])
        continue;
      if (!strncmp(line, "def", 3))
        continue;
      if (strncmp(line, "tex", 3) != 0)
        return NULL;
      int n = 0;
      while (line[n] != 0x20 && line[n] != 0x9 && line[n] != 0) {n++;}
      if (line[n] == 0)
        return str;
      while (line[n] == 0x20 || line[n] == 0x9) {n++;}
      if (line[n] == 0)
        return str;
      assert (line[n] == 't');
      Op0 = atoi(&line[n+1]);
      n += 2;
      while (line[n] != 0x20 && line[n] != 0x9 && line[n] != 0) {n++;}
      if (line[n] == 0)
        return str;
      while (line[n] == 0x20 || line[n] == 0x9) {n++;}
      if (line[n] == 0)
        return str;
      assert (line[n] == 't');
      Op1 = atoi(&line[n+1]);
	  return str;
    }
    return NULL;
  }
  LPD3DXBUFFER mfLoad(const char *prog_text)
  {
		// Assemble and create pixel shader
    HRESULT hr;
    LPD3DXBUFFER pCode;
    LPD3DXBUFFER pBuffer = NULL;
    char *pNewText = NULL;
    if (m_Insts[m_CurInst].m_Mask & VPVST_CLIPPLANES3)
    {
      const char *str;
      if (str=strstr(prog_text, "ps.1."))
      {
        int Op0, Op1;
        str = mfSkipLine(str);
        while (!strncmp(str, "def", 3))
        {
          str = mfSkipLine(str);
        }
        const char *sLastStr = str;
        while (str=mfGetTexInstruction(str, Op0, Op1))
        {
          if (Op0 == 3 || Op1 == 3)
            break;
          sLastStr = str;
        }
        if (!str)
        {
          size_t size = strlen(prog_text)+strlen("texkill t3\n")+1;
          pNewText = new char[size];
          memcpy(pNewText, prog_text, sLastStr-prog_text);
          strcpy(&pNewText[sLastStr-prog_text], "texkill t3\n");
          strcpy(&pNewText[sLastStr-prog_text+strlen("texkill t3\n")], &prog_text[sLastStr-prog_text]);
        }
      }
    }
    if (pNewText)
      hr = D3DXAssembleShader(pNewText, strlen(pNewText), NULL, NULL, 0, &pCode, &pBuffer);
    else
      hr = D3DXAssembleShader(prog_text, strlen(prog_text), NULL, NULL, 0, &pCode, &pBuffer);
    if (FAILED(hr))
    {
      Warning( 0,0,"WARNING: CCGPShader_D3D::mfLoad: Could not assemble pixel shader '%s'(0x%x) (%s)\n", m_Name.c_str(), m_nMaskGen, gcpRendD3D->D3DError(hr));
      if( pBuffer != NULL)
      {
        TCHAR* pstr;
        TCHAR strOut[4096];
        TCHAR* pstrOut;
        // Need to replace \n with \r\n so edit box shows newlines properly
        pstr = (TCHAR*)pBuffer->GetBufferPointer();
        strOut[0] = '\0';
        pstrOut = strOut;
        for( int i = 0; i < 4096; i++ )
        {
          if( *pstr == '\n' )
            *pstrOut++ = '\r';
          *pstrOut = *pstr;
          if( *pstr == '\0' )
            break;
          if( i == 4095 )
            *pstrOut  = '\0';
          pstrOut++;
          pstr++;
        }
        // remove any blank lines at the end
        while( strOut[lstrlen(strOut) - 1] == '\n' || strOut[lstrlen(strOut) - 1] == '\r' )
        {
          strOut[lstrlen(strOut) - 1] = '\0';
        }
        Warning( 0,0,"WARNING: CCGPShader_D3D::mfLoad: Shader script error (%s)\n", strOut);
        SAFE_RELEASE(pBuffer);
        SAFE_DELETE_ARRAY(pNewText);
      }
      return NULL;
    }
    if (pCode && !(m_Flags & PSFI_PRECACHEPHASE))
      hr = gcpRendD3D->mfGetD3DDevice()->CreatePixelShader((DWORD*)pCode->GetBufferPointer(), (IDirect3DPixelShader9 **)&m_Insts[m_CurInst].m_pHandle);
    SAFE_RELEASE(pBuffer);
    SAFE_DELETE_ARRAY(pNewText);
    if (FAILED(hr))
    {
      Warning( 0,0,"CCGPShader_D3D::mfLoad: Could not create pixel shader '%s'(0x%x) (%s)\n", m_Name.c_str(), m_nMaskGen, gcpRendD3D->D3DError(hr));
      return NULL;
    }
    return pCode;
  }

  void mfParameteri(SCGBind *ParamBind, const float *v)
  {
    int i;
    if(!ParamBind)
      return;

    if (!ParamBind->m_dwBind || ParamBind->m_dwFrameCreated != m_dwFrame)
    {
      ParamBind->m_dwFrameCreated = m_dwFrame;
      if (m_Insts[m_CurInst].m_BindVars)
      {
        for (i=0; i<m_Insts[m_CurInst].m_BindVars->Num(); i++)
        {
          SCGBind *p = &m_Insts[m_CurInst].m_BindVars->Get(i);
          if (p->m_Name == ParamBind->m_Name)
          {
            ParamBind->m_dwBind = p->m_dwBind;
            ParamBind->m_nBindComponents = p->m_nComponents;
            if (!ParamBind->m_dwBind)
              ParamBind->m_dwBind = 65536;
            break;
          }
        }
        if (i == m_Insts[m_CurInst].m_BindVars->Num())
          ParamBind->m_dwBind = -1;
      }
      else
        ParamBind->m_dwBind = -1;
      if (ParamBind->m_dwBind == -1 && CRenderer::CV_r_shaderssave >= 2)
        iLog->Log("Warning: couldn't find parameter %s for pixel shader %s (%x)", ParamBind->m_Name.c_str(), m_Name.c_str(), m_nMaskGen);
    }
    if ((int)ParamBind->m_dwBind == -1)
      return;
    int n;
    int iparms[4];
    if (ParamBind->m_dwBind == 65536)
      n = 0;
    else
      n = ParamBind->m_dwBind;
    if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
    {
      m_CurParams[n][0] = v[0];
      m_CurParams[n][1] = v[1];
      m_CurParams[n][2] = v[2];
      m_CurParams[n][3] = v[3];

      iparms[0] = (int)v[0];
      iparms[1] = (int)v[1];
      iparms[2] = (int)v[2];
      iparms[3] = (int)v[3];
      gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstantI(n, iparms, 1);
    }
    v += 4;
  }

  void mfParameter(SCGBind *ParamBind, const float *v, int nComps)
  {
    int i;
    if(!ParamBind)
      return;

    if (!ParamBind->m_dwBind || ParamBind->m_dwFrameCreated != m_dwFrame)
    {
      ParamBind->m_dwFrameCreated = m_dwFrame;
      if (m_Insts[m_CurInst].m_BindVars)
      {
        for (i=0; i<m_Insts[m_CurInst].m_BindVars->Num(); i++)
        {
          SCGBind *p = &m_Insts[m_CurInst].m_BindVars->Get(i);
          if (p->m_Name == ParamBind->m_Name)
          {
            assert(p->m_nComponents <= nComps);
            ParamBind->m_dwBind = p->m_dwBind;
            ParamBind->m_nBindComponents = p->m_nComponents;
            if (!ParamBind->m_dwBind)
              ParamBind->m_dwBind = 65536;
            break;
          }
        }
        if (i == m_Insts[m_CurInst].m_BindVars->Num())
          ParamBind->m_dwBind = -1;
      }
      else
        ParamBind->m_dwBind = -1;
      if (ParamBind->m_dwBind == -1 && CRenderer::CV_r_shaderssave >= 2)
        iLog->Log("Warning: couldn't find parameter %s for pixel shader %s (%x)", ParamBind->m_Name.c_str(), m_Name.c_str(), m_nMaskGen);
    }
    if ((int)ParamBind->m_dwBind == -1)
      return;
    if ((ParamBind->m_dwBind & 0xffff) == GL_OFFSET_TEXTURE_2D_MATRIX_NV)
    {
      int tmu = (ParamBind->m_dwBind >> 28) - 1;
      float parm[4];
      float fScaleX = CRenderer::CV_r_embm;
      float fScaleY = CRenderer::CV_r_embm;
      if (gcpRendD3D->m_bHackEMBM && gcpRendD3D->m_RP.m_TexStages[tmu].Texture && gcpRendD3D->m_RP.m_TexStages[tmu].Texture->m_eTT == eTT_Rectangle)
      {
        fScaleX *= (float)gcpRendD3D->GetWidth();
        fScaleY *= (float)gcpRendD3D->GetHeight();
      }
      parm[0] = v[0] * fScaleX;
      parm[1] = v[1] * fScaleX;
      parm[2] = v[2] * fScaleY;
      parm[3] = v[3] * fScaleY;
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVMAT00, FLOATtoDWORD(parm[0]));
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVMAT01, FLOATtoDWORD(parm[1]));
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVMAT10, FLOATtoDWORD(parm[2]));
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVMAT11, FLOATtoDWORD(parm[3]));
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVLSCALE, FLOATtoDWORD(4.0f));
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(tmu, D3DTSS_BUMPENVLOFFSET, FLOATtoDWORD(0.0f));
    }
    else
    {
      int nR;
      if (ParamBind->m_dwBind == 65536)
        nR = 0;
      else
        nR = ParamBind->m_dwBind;
      for (int i=0; i<ParamBind->m_nBindComponents; i++)
      {
        int n = nR+i;
        if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
        {
          m_CurParams[n][0] = v[0];
          m_CurParams[n][1] = v[1];
          m_CurParams[n][2] = v[2];
          m_CurParams[n][3] = v[3];
          gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstantF(n, v, 1);
        }
        v += 4;
      }
    }
  }
  void mfParameter4i(SCGBind *ParamBind, const float *v)
  {
    mfParameteri(ParamBind, v);
  }
  void mfParameter4f(SCGBind *ParamBind, const float *v)
  {
    mfParameter(ParamBind, v, 1);
  }
  SCGBind *mfGetParameterBind(const char *Name)
  {
    CName nm = CName(Name, eFN_Add);
    if (!m_Insts[m_CurInst].m_BindVars)
      m_Insts[m_CurInst].m_BindVars = new TArray<SCGBind>;

    int i;
    for (i=0; i<m_Insts[m_CurInst].m_BindVars->Num(); i++)
    {
      if (nm == m_Insts[m_CurInst].m_BindVars->Get(i).m_Name)
        return &m_Insts[m_CurInst].m_BindVars->Get(i);
    }
    return NULL;
  }
  void mfParameter4f(const char *Name, const float *v)
  {
    SCGBind *pBind = mfGetParameterBind(Name);
    if (pBind)
      mfParameter4f(pBind, v);
  }

  virtual void mfEnable()
  {
  }
  virtual void mfDisable()
  {
  }

  void mfFree();

  void mfDelInst()
  {
    if(m_Insts[m_CurInst].m_pHandle && (INT_PTR)m_Insts[m_CurInst].m_pHandle != -1)
    {
      if (m_Insts[m_CurInst].m_BindConstants)
        delete m_Insts[m_CurInst].m_BindConstants;
      if (m_Insts[m_CurInst].m_BindVars)
        delete m_Insts[m_CurInst].m_BindVars;
      if (m_Insts[m_CurInst].m_ParamsNoObj)
        delete m_Insts[m_CurInst].m_ParamsNoObj;
      IDirect3DPixelShader9 *pVS = (IDirect3DPixelShader9 *)m_Insts[m_CurInst].m_pHandle;
      SAFE_RELEASE (pVS);
    }
    m_Insts[m_CurInst].m_pHandle = NULL;
  }

  void mfSetVariables(TArray<SCGParam4f>* Parms);

  bool mfIsValid(int Num) const { return (m_Insts[m_CurInst].m_pHandle != NULL); }
  void mfGetSrcFileName(char *srcname, int nSize);
  void mfGetDstFileName(char *dstname, int nSize, bool bUseASCIICache);
  void mfPrecacheLights(int nMask);

public:
  char *mfGenerateScriptPS();
  char *mfCreateAdditionalPS();
  void mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params);
  bool mfActivate();
  void mfConstructFX(std::vector<SFXStruct>& Structs, std::vector<SPair>& Macros, char *entryFunc);
  void mfAddFXParameter(SFXParam *pr, const char *ParamName, SShader *ef);
  bool mfGetFXParamNameByID(int nParam, char *ParamName);

public:
  virtual ~CCGPShader_D3D();
  virtual void Release();
  virtual bool mfCompile(char *scr);
  virtual bool mfSet(bool bStat, SShaderPassHW *slw=NULL, int nFlags=0);
  virtual void mfSetVariables(bool bObj, TArray<SCGParam4f>* Vars);
  virtual void mfReset();
  virtual void mfPrecache();
  virtual bool mfIsCombiner() { return false; }
  virtual void mfGatherFXParameters(const char *buf, SShaderPassHW *pSHPass, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, SShader *ef);
  virtual void mfPostLoad();
  char *mfGetObjectCode(CGprogram cgPr)
  {
    const char *code = cgGetProgramString(cgPr, CG_COMPILED_PROGRAM);
    size_t size = strlen(code)+1;
    char *str = new char[size];
    cryMemcpy(str, code, size);
    return str;
  }

  static vec4_t m_CurParams[32];
};


#endif  // __GLCGPSHADER_H__

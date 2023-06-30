/*=============================================================================
  GLCGPShader.h : OpenGL CG programs interface declaration.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#ifndef __GLCGPSHADER_H__
#define __GLCGPSHADER_H__

#include "cg\cgGL.h"
#include "nvparse/nvparse.h"

#define CG_FP_CACHE_VER    3.4

extern char *PS20toARBfp(const char *inName);
extern char *PS20toNV30fp(const char *inName);

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

class CCGPShader_GL : public CPShader
{
  SCGScript *m_DeclarationsScript;
  SCGScript *m_CoreScript;
  SCGScript *m_InputParmsScript;
  SCGScript *m_SubroutinesScript;

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
    int m_Mask;
    int m_LightMask;
    CName m_PosScriptName;
    TArray<SCGParam4f> *m_ParamsNoObj;
    TArray<SCGParam4f> *m_ParamsObj;
    TArray<SCGMatrix> *m_MatrixObj;
    uint m_dwHandle;
    uint m_dwHandleExt;
    TArray<SCGBindConst> *m_BindConstants;
    TArray<SCGBind> *m_BindVars;
    int m_nCacheID;
    SCGInstance()
    {
      m_Mask = 0;
      m_LightMask = 0;
      m_dwHandle = m_dwHandleExt = 0;
      m_ParamsNoObj = NULL;
      m_ParamsObj = NULL;
      m_MatrixObj = NULL;
      m_BindConstants = NULL;
      m_BindVars = NULL;
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
    cg.m_dwHandle = NULL;
    cg.m_dwHandleExt = NULL;
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


  CCGPShader_GL(CGprofile ProfType)
  {
    m_CGProfileType = ProfType;
    mfInit();
  }
  CCGPShader_GL();
  void mfSaveCGFile(const char *scr)
  {
    char name[1024];
    if (m_nMaskGen)
      sprintf(name, "%s(%x).cg", m_Name.c_str(), m_nMaskGen);
    else
      sprintf(name, "%s.cg", m_Name.c_str());
    FILE *fp = fopen(name, "w");
    fprintf(fp, scr);
    fclose (fp);
  }
  void mfInit()
  {
#ifndef WIN64
		// NOTE: AMD64 port: find the 64-bit CG runtime
		if (!gcpOGL->m_CGContext)
    {
      gcpOGL->m_CGContext = cgCreateContext();
      assert(gcpOGL->m_CGContext != NULL);
    }
#endif
    m_dwFrame = 1;
    m_CurInst = -1;
    m_CoreScript = NULL;
    m_InputParmsScript = NULL;
    m_SubroutinesScript = NULL;
    m_DeclarationsScript = NULL;
  }

  void mfBind()
  {
    if (m_CGProfileType == CG_PROFILE_FP20)
    {
      if (m_Insts[m_CurInst].m_dwHandleExt > 0)
      {
        glCallList(m_Insts[m_CurInst].m_dwHandleExt);
        gRenDev->m_RP.m_PersFlags |= RBPF_TSNEEDSET;
      }
      else
        gRenDev->m_RP.m_PersFlags &= ~RBPF_TSNEEDSET;
      if (m_Insts[m_CurInst].m_dwHandle > 0)
      {
        glCallList(m_Insts[m_CurInst].m_dwHandle);
        if (m_Insts[m_CurInst].m_BindConstants)
        {
          int i;
          for (i=0; i<m_Insts[m_CurInst].m_BindConstants->Num(); i++)
          {
            SCGBindConst *p = &m_Insts[m_CurInst].m_BindConstants->Get(i);
            int Reg = p->m_dwBind;
            int reg = (Reg & 0xffffff);
            int nComb = (Reg >> 28) - 1;
            if (reg >= 65536)
              reg -= 65536;
            assert (reg >= 0 && reg < 2);
            if (reg < 0 || reg >= 2)
              continue;
            int n = nComb*2+reg;
            if (m_CurParams[n][0] != p->m_Val[0] || m_CurParams[n][1] != p->m_Val[1] || m_CurParams[n][2] != p->m_Val[2] || m_CurParams[n][3] != p->m_Val[3])
            {
              m_CurParams[n][0] = p->m_Val[0];
              m_CurParams[n][1] = p->m_Val[1];
              m_CurParams[n][2] = p->m_Val[2];
              m_CurParams[n][3] = p->m_Val[3];
              if (Reg & 0xf0000000)
                glCombinerStageParameterfvNV(GL_COMBINER0_NV+nComb, reg+GL_CONSTANT_COLOR0_NV, p->m_Val);
              else
                glCombinerParameterfvNV(reg+GL_CONSTANT_COLOR0_NV, p->m_Val);
            }
          }
        }
        gRenDev->m_RP.m_PersFlags |=  RBPF_PS1NEEDSET;
        gRenDev->m_RP.m_PersFlags &= ~RBPF_PS2NEEDSET;
      }
    }
    else
    if (m_CGProfileType == CG_PROFILE_ARBFP1)
    {
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_Insts[m_CurInst].m_dwHandle);
      if (m_Insts[m_CurInst].m_BindConstants)
      {
        int i;
        for (i=0; i<m_Insts[m_CurInst].m_BindConstants->Num(); i++)
        {
          SCGBindConst *p = &m_Insts[m_CurInst].m_BindConstants->Get(i);
          int n = p->m_dwBind;
          if (m_CurParamsARB[n][0] != p->m_Val[0] || m_CurParamsARB[n][1] != p->m_Val[1] || m_CurParamsARB[n][2] != p->m_Val[2] || m_CurParamsARB[n][3] != p->m_Val[3])
          {
            m_CurParamsARB[n][0] = p->m_Val[0];
            m_CurParamsARB[n][1] = p->m_Val[1];
            m_CurParamsARB[n][2] = p->m_Val[2];
            m_CurParamsARB[n][3] = p->m_Val[3];
            glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, p->m_dwBind, &p->m_Val[0]);
          }
        }
      }
      gRenDev->m_RP.m_PersFlags |=  RBPF_PS2NEEDSET;
      gRenDev->m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET;
    }
  }

  void mfUnbind()
  {
  }

  char *mfLoadCG(char *prog_text)
  {
#ifndef WIN64
    CGprofile pr = (CGprofile)m_CGProfileType;

#if 0
    // Use HLSL compiler for PS_2_X profile since it generates more efficient code (and allmost bugs free)
    if (pr == CG_PROFILE_FP30 || pr == CG_PROFILE_ARBFP1)
    {
      char *Buf = sReplaceInText(prog_text, "pixout OUT", "float4 Color = (float4)0");
      char *Buf1 = sReplaceInText(Buf, "pixout main", "float4 main");
      remove("$$out.cg");
      if (Buf != Buf1)
      {
        if (Buf != prog_text)
          delete [] Buf;
        Buf = Buf1;
      }
      Buf1 = sReplaceInText(Buf, "return OUT", "return Color");
      if (Buf != Buf1)
      {
        if (Buf != prog_text)
          delete [] Buf;
        Buf = Buf1;
      }
      Buf1 = sReplaceInText(Buf, "OUT.Color", "Color");
      if (Buf != Buf1)
      {
        if (Buf != prog_text)
          delete [] Buf;
        Buf = Buf1;
      }

      // make command for execution
      FILE *fp = fopen("$$in.cg", "w");
      if (!fp)
        return NULL;
      fputs(Buf, fp);
      fclose (fp);

      char szCmdLine[512];
      sprintf(szCmdLine, "fxc.exe /T %s /Fc $$out.cg $$in.cg", pr == CG_PROFILE_FP30 ? "ps_2_a" : "ps_2_0");

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
      char *pBuf;
      if (pr == CG_PROFILE_FP30)
        pBuf = PS20toNV30fp("$$out.cg");
      else
        pBuf = PS20toARBfp("$$out.cg");

      if (Buf != prog_text)
        delete [] Buf;
      remove("$$in.cg");
      remove("$$out.cg");

      return pBuf;
    }
    else
#endif
    {
      // Test adding source text to context
      const char *profileOpts[] =
      {
        "-DCGC=1",
        NULL,
      };
      cgGLSetOptimalOptions((CGprofile)m_CGProfileType);
      CGprogram cgPr = cgCreateProgram(gcpOGL->m_CGContext, CG_SOURCE, prog_text, (CGprofile)m_CGProfileType, "main", profileOpts);    
      CGerror err = cgGetError();
      if (err != CG_NO_ERROR)
      {
        if ((m_Flags & PSFI_AUTOENUMTC) && m_CGProfileType == CG_PROFILE_FP20 && (gRenDev->GetFeatures() & RFT_HW_PS20))
        {
#ifndef WIN64
          // NOTE: AMD64 port: find the 64-bit CG runtime
          m_CGProfileType = cgGLGetLatestProfile(CG_GL_FRAGMENT);
#endif
          if (SUPPORTS_GL_ARB_vertex_program)
            m_CGProfileType = CG_PROFILE_ARBFP1;
          return mfLoadCG(prog_text);
        }
        Warning(0,0,"Couldn't create CG program '%s' (%s)", m_Name.c_str(), cgGetErrorString(err));
        mfSaveCGFile(prog_text);
        return NULL;
      }
      if (cgPr)
      {
        char *code = mfGetObjectCode(cgPr);     
        cgDestroyProgram(cgPr);
        return code;
      }
    }
    return NULL;
#else
    // make command for execution
    FILE *fp = fopen("$$in.cg", "w");
    if (!fp)
      return NULL;
    assert (*prog_text);
    fputs(prog_text, fp);
    fclose (fp);

    char szCmdLine[512];
    sprintf(szCmdLine, 
      "cgc.exe -profile %s -o $$out.cg $$in.cg",
      m_CGProfileType == CG_PROFILE_FP20 ? "fp20" : "arbfp1");

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
      Warning( 0,0,"CG compiler (cgc.exe) wasn't able to compile pixel shader '%s'", m_Name.c_str());
      mfSaveCGFile(prog_text);
      return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (size < 20)
    {
      Warning( 0,0,"CG compiler (cgc.exe) wasn't able to compile pixel shader '%s'", m_Name.c_str());
      mfSaveCGFile(prog_text);
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
#endif
  }

  void mfLoad(const char *prog_text)
  {
    if (m_CGProfileType == CG_PROFILE_ARBFP1)
    {
      glGenProgramsARB(1, &m_Insts[m_CurInst].m_dwHandle);
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_Insts[m_CurInst].m_dwHandle);
      int size = strlen(prog_text);
      glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, size, (const GLubyte *)prog_text);
      GLint errpos;
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errpos);
      if(errpos != -1)
      {
        const GLubyte *pError = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        iLog->Log("Warning: Fragment Program '%s' error (%s):\n", m_Name.c_str(), pError);
        int bgn = errpos - 10;
        bgn < 0 ? 0 : bgn;
        const char * c = (const char *)(prog_text + bgn);
        for(int i = 0; i < 30; i++)
        {
          if(bgn+i >= int(size-1))
            break;
          iLog->Log("%c", *c++);
        }
        iLog->Log("\n");
      }
    }
    else
    if (m_CGProfileType == CG_PROFILE_FP30)
    {
      glGenProgramsNV(1, &m_Insts[m_CurInst].m_dwHandle);
      glBindProgramNV(GL_FRAGMENT_PROGRAM_NV, m_Insts[m_CurInst].m_dwHandle);
      int size = strlen(prog_text);
      glLoadProgramNV(GL_FRAGMENT_PROGRAM_NV, m_Insts[m_CurInst].m_dwHandle, size, (const GLubyte *)prog_text);
      const GLubyte *pError = glGetString(GL_PROGRAM_ERROR_STRING_NV);
      if(pError && pError[0])
        iLog->Log("Error: %s", pError);
    }
    else
    if (m_CGProfileType == CG_PROFILE_FP20)
    {
      const char *sStr = strstr(prog_text, "!!TS1.0");
      if (sStr)
      {
        char sLine[4][128];
        int n = 8;
        for (int i=0; i<4; i++)
        {
          int m = 0;
          while (sStr[n] != 0xa)
          {
            if (sStr[n] == '/' && sStr[n+1] == '/')
              break;
            sLine[i][m++] = sStr[n++];
          }
          if (sStr[n] == '/' && sStr[n+1] == '/')
            break;
          n++;
          sLine[i][m++] = 0;
        }
        int nS = i;
        bool bUseTS = false;
        for (i=0; i<nS; i++)
        {
          if (strcmp(sLine[i], "texture_2d();") && strcmp(sLine[i], "nop();") && strcmp(sLine[i], "texture_3d();") && strcmp(sLine[i], "texture_cube_map();") && strcmp(sLine[i], "texture_rectangle();"))
          {
            bUseTS = true;
            // Use projected env. bump mapping if it's supported
            if (gRenDev->GetFeatures() & RFT_HW_ENVBUMPPROJECTED)
            {
              if (!strncmp(sLine[i], "offset_rectangle", 16) || !strncmp(sLine[i], "offset_2d", 9))
              {
                // replace texture shader offset operation by projective one
                char sTemp[128];
                char *s = strchr(sLine[i], '(');
                if (s)
                {
                  strcpy(sTemp, s);
                  if (!strncmp(sLine[i], "offset_rectangle", 16))
                    strcpy(sLine[i], "offset_projective_rectangle");
                  else
                    strcpy(sLine[i], "offset_projective_2d");
                  strcat(sLine[i], sTemp);
                }
              }
            }
          }
        }
        if (bUseTS)
        {
          char code[256];
          switch(nS)
          {
            case 1:
              sprintf(code, "!!TS1.0\n %s\n", sLine[0]);
            	break;
            case 2:
              sprintf(code, "!!TS1.0\n %s\n %s\n", sLine[0], sLine[1]);
              break;
            case 3:
              sprintf(code, "!!TS1.0\n %s\n %s\n %s\n", sLine[0], sLine[1], sLine[2]);
              break;
            case 4:
              sprintf(code, "!!TS1.0\n %s\n %s\n %s\n %s\n", sLine[0], sLine[1], sLine[2], sLine[3]);
              break;
            default:
              assert(0);
          }
          uint listTS = glGenLists(1);
          glNewList(listTS, GL_COMPILE);
          nvparse(false, code);
          int n = 0;
          for (char * const * errors = nvparse_get_errors(); *errors; errors++)
          {
            n++;
            iLog->Log(*errors);
          }
          glEndList();
          if (!n)
            m_Insts[m_CurInst].m_dwHandleExt = listTS;
        }
      }

      sStr = strstr(prog_text, "!!RC1.0");
      if (sStr)
      {
        uint listRC = glGenLists(1);
        glNewList(listRC, GL_COMPILE);
        nvparse(false, sStr);
        int n = 0;
        for (char * const * errors = nvparse_get_errors(); *errors; errors++)
        {
          n++;
          iLog->Log(*errors);
        }
        glEndList();
        if (!n)
        {
          m_Insts[m_CurInst].m_dwHandle = listRC;
          if (gParseConsts.Num())
          {
            m_Insts[m_CurInst].m_BindConstants = new TArray<SCGBindConst>;
            m_Insts[m_CurInst].m_BindConstants->Copy(gParseConsts);
            gParseConsts.Free();
          }
        }
      }
    }
  }

  void mfParameter(SCGBind *ParamBind, const float *vData, int nComps)
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
            ParamBind->m_dwBind = (ParamBind->m_dwBind & 0x80000) | p->m_dwBind;
            if (!(ParamBind->m_dwBind & 0xffff))
              ParamBind->m_dwBind |= 0x10000;
            ParamBind->m_nBindComponents = p->m_nComponents;
            ParamBind->m_pNext = p->m_pNext;
            break;
          }
        }
        if (i == m_Insts[m_CurInst].m_BindVars->Num())
          ParamBind->m_dwBind = -1;
      }
      else
        ParamBind->m_dwBind = -1;
      if (ParamBind->m_dwBind == -1 && CRenderer::CV_r_shaderssave >= 2)
        iLog->Log("Warning: couldn't find parameter %s for vertex shader %s (%x)", ParamBind->m_Name.c_str(), m_Name.c_str(), m_nMaskGen);
    }
    if ((int)ParamBind->m_dwBind == -1)
      return;
    if (m_CGProfileType == CG_PROFILE_FP20)
    {
      SCGBind *pBind = ParamBind;
      while (pBind)
      {
        const float *v = vData;
        for (i=0; i<pBind->m_nBindComponents; i++)
        {
          int Reg = pBind->m_dwBind+i;
          int reg = (Reg & 0xf7ffff);
          int nComb = (Reg >> 28) - 1;
          if (reg == GL_OFFSET_TEXTURE_2D_MATRIX_NV)
          {
            float pr[4];
            pr[0] = v[0] * CRenderer::CV_r_embm;
            pr[1] = v[1] * CRenderer::CV_r_embm;
            pr[2] = v[2] * CRenderer::CV_r_embm;
            pr[3] = v[3] * CRenderer::CV_r_embm;
            if (CGLTexMan::m_TUState[nComb].m_Target == GL_TEXTURE_RECTANGLE_NV)
            {
              float fScrWidth  = (float)gRenDev->GetWidth();
              float fScrHeight = (float)gRenDev->GetHeight();
              pr[0] *= fScrWidth;
              pr[1] *= fScrWidth;
              pr[2] *= fScrHeight;
              pr[3] *= fScrHeight;
            }
            glActiveTextureARB(GL_TEXTURE0_ARB+nComb);
            glTexEnvfv(GL_TEXTURE_SHADER_NV, reg, pr);
            glActiveTextureARB(GL_TEXTURE0_ARB+CTexMan::m_CurStage);
          }
          else
          {
            if (reg & 0x10000)
              reg = 0;
            assert (reg >= 0 && reg < 2);
            int n = nComb*2+reg;
            if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
            {
              m_CurParams[n][0] = v[0];
              m_CurParams[n][1] = v[1];
              m_CurParams[n][2] = v[2];
              m_CurParams[n][3] = v[3];
              if (Reg & 0xf0000000)
                glCombinerStageParameterfvNV(GL_COMBINER0_NV+nComb, reg+GL_CONSTANT_COLOR0_NV, v);
              else
                glCombinerParameterfvNV(reg+GL_CONSTANT_COLOR0_NV, v);
            }
          }
          v += 4;
        }
        pBind = pBind->m_pNext;
      }
    }
    else
    {
      for (i=0; i<ParamBind->m_nBindComponents; i++)
      {
        int n = (ParamBind->m_dwBind+i) & ~0x10000;
        if (n & 0x80000)
        {
          n &= ~0x80000;
          float fScaleX = CRenderer::CV_r_embm * (float)gcpOGL->GetWidth();
          float fScaleY = CRenderer::CV_r_embm * (float)gcpOGL->GetHeight();
          float v[4];
          v[0] = vData[0] * fScaleX;
          v[1] = vData[1] * fScaleX;
          v[2] = vData[2] * fScaleY;
          v[3] = vData[3] * fScaleY;
          vData = v;
        }
        if (m_CurParamsARB[n][0] != vData[0] || m_CurParamsARB[n][1] != vData[1] || m_CurParamsARB[n][2] != vData[2] || m_CurParamsARB[n][3] != vData[3])
        {
          m_CurParamsARB[n][0] = vData[0];
          m_CurParamsARB[n][1] = vData[1];
          m_CurParamsARB[n][2] = vData[2];
          m_CurParamsARB[n][3] = vData[3];
          glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, n, vData);
        }
        vData += 4;
      }
    }
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
    if (m_CGProfileType == CG_PROFILE_FP20)
    {
      if (m_Insts[m_CurInst].m_dwHandleExt)
      {
        glEnable(GL_TEXTURE_SHADER_NV);
        gRenDev->m_RP.m_PersFlags |= RBPF_TSWASSET;
      }
      if (m_Insts[m_CurInst].m_dwHandle)
      {
        glEnable(GL_REGISTER_COMBINERS_NV);
        gRenDev->m_RP.m_PersFlags |= RBPF_PS1WASSET;
      }
    }
    else
    if (m_CGProfileType == CG_PROFILE_ARBFP1)
    {
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      gRenDev->m_RP.m_PersFlags |= RBPF_PS2WASSET;
    }
  }
  virtual void mfDisable()
  {
    if (m_CGProfileType == CG_PROFILE_FP20)
    {
      if (m_Insts[m_CurInst].m_dwHandleExt)
      {
        glDisable(GL_TEXTURE_SHADER_NV);
        gRenDev->m_RP.m_PersFlags &= ~RBPF_TSWASSET;
      }
      if (m_Insts[m_CurInst].m_dwHandle)
      {
        glDisable(GL_REGISTER_COMBINERS_NV);
        gRenDev->m_RP.m_PersFlags &= ~RBPF_PS1WASSET;
      }
    }
    else
    if (m_CGProfileType == CG_PROFILE_ARBFP1)
    {
      glDisable(GL_FRAGMENT_PROGRAM_ARB);
      gRenDev->m_RP.m_PersFlags &= ~RBPF_PS2WASSET;
    }
  }

  void mfFree();

  void mfDel()
  {
    if(m_Insts[m_CurInst].m_dwHandle && m_Insts[m_CurInst].m_dwHandle != -1)
    {
      if (m_Insts[m_CurInst].m_BindConstants)
        delete m_Insts[m_CurInst].m_BindConstants;
      if (m_Insts[m_CurInst].m_BindVars)
        delete m_Insts[m_CurInst].m_BindVars;
      if (m_Insts[m_CurInst].m_ParamsNoObj)
        delete m_Insts[m_CurInst].m_ParamsNoObj;
      if (m_CGProfileType == CG_PROFILE_FP20)
        glDeleteLists(m_Insts[m_CurInst].m_dwHandle, 1);
      else
      if (m_CGProfileType == CG_PROFILE_ARBFP1)
        glDeleteProgramsARB(1, &m_Insts[m_CurInst].m_dwHandle);
      m_Insts[m_CurInst].m_dwHandle = 0;
    }
    if(m_Insts[m_CurInst].m_dwHandleExt && m_Insts[m_CurInst].m_dwHandleExt != -1)
    {
      if (m_CGProfileType == CG_PROFILE_FP20)
        glDeleteLists(m_Insts[m_CurInst].m_dwHandleExt, 1);
      m_Insts[m_CurInst].m_dwHandleExt = 0;
    }
  }

  void mfSetVariables(TArray<SCGParam4f>* Parms);

  bool mfIsValid(int Num) const { return (m_Insts[m_CurInst].m_dwHandle != 0); }
  void mfGetSrcFileName(char *srcname, int nSize);
  void mfGetDstFileName(char *dstname, int nSize);

public:
  char *mfGenerateScriptPS();
  char *mfCreateAdditionalPS();
  void mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params);
  bool mfActivate();

public:
  virtual ~CCGPShader_GL();
  virtual void Release();
  virtual bool mfCompile(char *scr);
  virtual bool mfSet(bool bStat, SShaderPassHW *slw=NULL, int nFLags=0);
  virtual void mfSetVariables(bool bObj, TArray<SCGParam4f>* Vars);
  virtual void mfReset();
  virtual void mfPrecache();
  virtual bool mfIsCombiner() { return false; }
  virtual void mfGatherFXParameters(const char *buf, SShaderPassHW *pSHPass, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, SShader *ef) {};
  virtual void mfPostLoad() {};
  char *mfGetObjectCode(CGprogram cgPr)
  {
    const char *code = cgGetProgramString(cgPr, CG_COMPILED_PROGRAM);
    int size = strlen(code)+1;
    char *str = new char[size];
    cryMemcpy(str, code, size);

    return str;
  }
  static vec4_t m_CurParams[32];
  static vec4_t m_CurParamsARB[32];
};

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

#endif  // __GLCGPSHADER_H__

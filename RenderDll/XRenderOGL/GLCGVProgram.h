/*=============================================================================
  GLCGVProgram.h : OpenGL CG programs interface declaration.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#ifndef __GLCGVPROGRAM_H__
#define __GLCGVPROGRAM_H__

#include "cg\cgGL.h"

#define CG_VP_CACHE_VER    3.4

#define VSCONST_0_025_05_1 28
#define VSCONST_FOG 29

#define PSCONST_HDR_FOGCOLOR 31

class CCGVProgram_GL : public CVProgram
{
  SCGScript *m_Script;
  SCGScript *m_PosScript;
  SCGScript *m_SubroutinesScript;
  SCGScript *m_DeclarationsScript;
  SCGScript *m_CoreScript;
  SCGScript *m_InputParmsScript;

  struct SCGInstance
  {
    int m_Mask;
    CName m_PosScriptName;
    TArray<SCGParam4f> *m_ParamsNoObj;
    TArray<SCGParam4f> *m_ParamsObj;
    TArray<SCGMatrix> *m_MatrixObj;
    uint m_dwHandle;
    TArray<SCGBindConst> *m_BindConstants;
    TArray<SCGBind> *m_BindVars;
    SCGInstance()
    {
      m_dwHandle = 0;
      m_ParamsNoObj = NULL;
      m_ParamsObj = NULL;
      m_MatrixObj = NULL;
      m_BindConstants = NULL;
      m_BindVars = NULL;
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

  int m_CurInst;
  TArray<SCGInstance> m_Insts;
  TArray<SCGParam4f> m_ParamsNoObj;
  TArray<SCGParam4f> m_Params_Inst;
  TArray<SCGParam4f> m_ParamsObj;
  TArray<SCGMatrix> m_MatrixObj;
  TArray<SArrayPointer *> m_Pointers;

  int m_dwFrame;

  void mfSetPointers(int nType);

public:

  virtual int Size()
  {
    int nSize = sizeof(*this);
    if (m_Script)
      nSize += m_Script->Size(false);
    if (m_PosScript)
      nSize += m_PosScript->Size(false);
    if (m_DeclarationsScript)
      nSize += m_DeclarationsScript->Size(false);
    if (m_CoreScript)
      nSize += m_CoreScript->Size(false);
    if (m_SubroutinesScript)
      nSize += m_SubroutinesScript->Size(false);
    if (m_InputParmsScript)
      nSize += m_InputParmsScript->Size(false);
    nSize += m_ParamsNoObj.GetMemoryUsage();
    nSize += m_ParamsObj.GetMemoryUsage();
    nSize += m_MatrixObj.GetMemoryUsage();
    nSize += m_Pointers.GetMemoryUsage();
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

  TArray<SCGParam4f> *mfGetParams(int Type)
  {
    if (!Type)
      return &m_ParamsNoObj;
    else
      return &m_ParamsObj;
  }


  int mfGetCGInstanceID(int Type, CVProgram *pPosVP)
  {
    CCGVProgram_GL *pVP = (CCGVProgram_GL *)pPosVP;
    SCGInstance *cgi;
    if (m_CurInst >= 0 && m_Insts.Num() > m_CurInst)
    {
      cgi = &m_Insts[m_CurInst];
      if (cgi->m_Mask == Type)
      {
        if (!pVP || !pVP->m_PosScript || pVP->m_PosScript->m_Name == cgi->m_PosScriptName)
          return m_CurInst;
      }
    }
    int i;
    for (i=0; i<m_Insts.Num(); i++)
    {
      cgi = &m_Insts[i];
      if (cgi->m_Mask == Type && (!pVP || !pVP->m_PosScript || pVP->m_PosScript->m_Name == cgi->m_PosScriptName))
      {
        m_CurInst = i;
        return i;
      }
    }
    SCGInstance cg;
    cg.m_Mask = Type;
    cg.m_ParamsNoObj = NULL;
    cg.m_ParamsObj = NULL;
    cg.m_MatrixObj = NULL;
    cg.m_BindConstants = NULL;
    cg.m_BindVars = NULL;
    if (Type & VPVST_CLIPPLANES3)
    {
      if (!cg.m_ParamsNoObj)
        cg.m_ParamsNoObj = new TArray<SCGParam4f>;
      SCGParam4f pr;
      pr.m_nComponents = 1;
      pr.m_dwBind = 0;
      pr.m_Name = "ClipPlane";
      SParamComp_ClipPlane p;
      pr.m_Flags = PF_CANMERGED;
      pr.m_Comps[0] = SParamComp::mfAdd(&p);
      cg.m_ParamsNoObj->AddElem(pr);
    }
    if (Type & VPVST_TCMASK)
    {
      if (!cg.m_ParamsNoObj)
        cg.m_ParamsNoObj = new TArray<SCGParam4f>;
      int nm = VPVST_TCM0;
      int ngol = VPVST_TCGOL0;
      int ngrm = VPVST_TCGRM0;
      int ngsm = VPVST_TCGSM0;
      int ngnm = VPVST_TCGNM0;
      for (int i=0; i<4; i++)
      {
        char str[128];
        if (Type & (ngol<<i))
        {
          SCGParam4f pr;
          pr.m_nComponents = 4;
          pr.m_dwBind = 0;
          sprintf(str, "MatrixTCG%d", i);
          pr.m_Name = str;
          SParamComp_MatrixTCG p;
          pr.m_Flags = PF_CANMERGED;
          p.m_Stage = i;
          p.m_Row = 0;
          pr.m_Comps[0] = SParamComp::mfAdd(&p);
          cg.m_ParamsNoObj->AddElem(pr);

          p.m_Row = 1;
          pr.m_Comps[0] = SParamComp::mfAdd(&p);
          cg.m_ParamsNoObj->AddElem(pr);

          p.m_Row = 2;
          pr.m_Comps[0] = SParamComp::mfAdd(&p);
          cg.m_ParamsNoObj->AddElem(pr);

          p.m_Row = 3;
          pr.m_Comps[0] = SParamComp::mfAdd(&p);
          cg.m_ParamsNoObj->AddElem(pr);
        }
        if (Type & ((ngrm|ngsm)<<i))
        {
          SCGParam4f pr;
          pr.m_nComponents = 1;
          pr.m_dwBind = 0;
          pr.m_Name = "CameraPos";
          SParamComp_OSCameraPos p;
          pr.m_Flags = PF_CANMERGED;
          pr.m_Comps[0] = SParamComp::mfAdd(&p);
          cg.m_ParamsNoObj->AddElem(pr);

          if (!cg.m_MatrixObj)
            cg.m_MatrixObj = new TArray<SCGMatrix>;
          SCGMatrix m;
          m.m_eCGParamType = ECGP_Matr_View;
          m.m_Name = "ModelView";
          cg.m_MatrixObj->AddElem(m);
          m.m_eCGParamType = ECGP_Matr_View_IT;
          m.m_Name = "ModelViewIT";
          cg.m_MatrixObj->AddElem(m);
        }
        if (Type & (ngnm<<i))
        {
          if (!cg.m_MatrixObj)
            cg.m_MatrixObj = new TArray<SCGMatrix>;
          SCGMatrix m;
          m.m_eCGParamType = ECGP_Matr_View_IT;
          m.m_Name = "ModelViewIT";
          cg.m_MatrixObj->AddElem(m);
        }
        if (Type & (nm<<i))
        {
          SCGParam4f pr;
          pr.m_nComponents = 4;
          pr.m_dwBind = 0;
          sprintf(str, "MatrixTCM%d", i);
          pr.m_Name = str;
          SParamComp_MatrixTCM p;
          pr.m_Flags = PF_CANMERGED;
          p.m_Stage = i;

          p.m_Row = 0;
          pr.m_Comps[0] = SParamComp::mfAdd(&p);
          cg.m_ParamsNoObj->AddElem(pr);

          p.m_Row = 1;
          pr.m_Comps[0] = SParamComp::mfAdd(&p);
          cg.m_ParamsNoObj->AddElem(pr);

          p.m_Row = 2;
          pr.m_Comps[0] = SParamComp::mfAdd(&p);
          cg.m_ParamsNoObj->AddElem(pr);

          p.m_Row = 3;
          pr.m_Comps[0] = SParamComp::mfAdd(&p);
          cg.m_ParamsNoObj->AddElem(pr);
        }
      }
    }
    if (pVP && pVP->m_PosScript)
    {
      cg.m_PosScriptName = pVP->m_PosScript->m_Name;
      if (cg.m_PosScriptName != m_PosScript->m_Name && pVP->m_ParamsNoObj.Num())
      {
        if (!cg.m_ParamsNoObj)
          cg.m_ParamsNoObj = new TArray<SCGParam4f>;
        for (int i=0; i<pVP->m_ParamsNoObj.Num(); i++)
        {
          cg.m_ParamsNoObj->AddElem(pVP->m_ParamsNoObj[i]);
          cg.m_ParamsNoObj->Get(cg.m_ParamsNoObj->Num()-1).m_dwBind = 0;
        }
        cg.m_ParamsNoObj->Shrink();
      }
      if (cg.m_PosScriptName != m_PosScript->m_Name && pVP->m_ParamsObj.Num())
      {
        if (!cg.m_ParamsObj)
          cg.m_ParamsObj = new TArray<SCGParam4f>;
        for (int i=0; i<pVP->m_ParamsObj.Num(); i++)
        {
          cg.m_ParamsObj->AddElem(pVP->m_ParamsObj[i]);
          cg.m_ParamsObj->Get(cg.m_ParamsObj->Num()-1).m_dwBind = 0;
        }
        cg.m_ParamsObj->Shrink();
      }
    }
    if (cg.m_ParamsNoObj)
    {
      TArray<SCGParam4f> ObjP;
      gRenDev->m_cEF.mfCheckObjectDependParams(cg.m_ParamsNoObj, &ObjP);
      if (ObjP.Num())
      {
        cg.m_ParamsObj = new TArray<SCGParam4f>;
        cg.m_ParamsObj->Copy(ObjP);
      }
    }
    m_CurInst = m_Insts.Num();
    m_Insts.AddElem(cg);
    return m_CurInst;
  }

  CCGVProgram_GL(CGprofile ProfType)
  {
    m_CGProfileType = ProfType;
    mfInit();
  }
  CCGVProgram_GL()
  {
#ifndef WIN64
		// NOTE: AMD64 port: find the 64-bit CG runtime
    m_CGProfileType = cgGLGetLatestProfile(CG_GL_VERTEX);
    if (m_CGProfileType != CG_PROFILE_VP20 && SUPPORTS_GL_ARB_vertex_program)
#endif
			m_CGProfileType = CG_PROFILE_ARBVP1;
    if (CGLRenderer::CV_gl_vsforce11 && SUPPORTS_GL_NV_vertex_program)
      m_CGProfileType = CG_PROFILE_VP20;
    mfInit();
  }
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
      //CGSetCompilerExe("cgc.exe");

      //cgSetErrorCallback(mfErrorCallback);
    }
#endif
    m_dwFrame = 1;
    m_CurInst = -1;
    m_Flags = 0;
    m_Script = NULL;
    m_CoreScript = NULL;
    m_InputParmsScript = NULL;
    m_PosScript = NULL;
    m_SubroutinesScript = NULL;
    m_DeclarationsScript = NULL;
  }

  void mfBind()
  {
    if (m_CGProfileType == CG_PROFILE_VP20)
    {
      glBindProgramNV(GL_VERTEX_PROGRAM_NV, m_Insts[m_CurInst].m_dwHandle);
      if (m_Insts[m_CurInst].m_BindConstants)
      {
        int i;
        for (i=0; i<m_Insts[m_CurInst].m_BindConstants->Num(); i++)
        {
          SCGBindConst *p = &m_Insts[m_CurInst].m_BindConstants->Get(i);
          int n = p->m_dwBind;
          if (m_CurParams[n][0] != p->m_Val[0] || m_CurParams[n][1] != p->m_Val[1] || m_CurParams[n][2] != p->m_Val[2] || m_CurParams[n][3] != p->m_Val[3])
          {
            m_CurParams[n][0] = p->m_Val[0];
            m_CurParams[n][1] = p->m_Val[1];
            m_CurParams[n][2] = p->m_Val[2];
            m_CurParams[n][3] = p->m_Val[3];
            glProgramParameter4fvNV(GL_VERTEX_PROGRAM_NV, p->m_dwBind, &p->m_Val[0]);
          }
        }
      }
    }
    else
    if (m_CGProfileType == CG_PROFILE_ARBVP1)
    {
      glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_Insts[m_CurInst].m_dwHandle);
      if (m_Insts[m_CurInst].m_BindConstants)
      {
        int i;
        for (i=0; i<m_Insts[m_CurInst].m_BindConstants->Num(); i++)
        {
          SCGBindConst *p = &m_Insts[m_CurInst].m_BindConstants->Get(i);
          int n = p->m_dwBind;
          if (m_CurParams[n][0] != p->m_Val[0] || m_CurParams[n][1] != p->m_Val[1] || m_CurParams[n][2] != p->m_Val[2] || m_CurParams[n][3] != p->m_Val[3])
          {
            m_CurParams[n][0] = p->m_Val[0];
            m_CurParams[n][1] = p->m_Val[1];
            m_CurParams[n][2] = p->m_Val[2];
            m_CurParams[n][3] = p->m_Val[3];
            glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, p->m_dwBind, &p->m_Val[0]);
          }
        }
      }
    }
  }
  
  void mfUnbind()
  {
    //cgError Ret = cgGLBindProgram(NULL);
  }

  char *mfLoadCG(const char *prog_text)
  {
#ifndef WIN64
		// TODO: AMD64 port: find 64-bit CG
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
      if ((m_Flags & PSFI_AUTOENUMTC) && m_CGProfileType == CG_PROFILE_VP20 && (gRenDev->GetFeatures() & RFT_HW_PS20))
      {
#ifndef WIN64
        // NOTE: AMD64 port: find the 64-bit CG runtime
        m_CGProfileType = cgGLGetLatestProfile(CG_GL_VERTEX);
#endif
        if (SUPPORTS_GL_ARB_vertex_program)
          m_CGProfileType = CG_PROFILE_ARBVP1;
        return mfLoadCG(prog_text);
      }
      Warning(0,0,"Couldn't create CG program '%s' (%s)", m_Name.c_str(), cgGetErrorString(err));
      mfSaveCGFile(prog_text);
      return NULL;
    }
    if (!cgPr)
    {
      iLog->Log("Couldn't find function '%s' in CG pixel program '%s'", "main", m_Name.c_str());
      return NULL;
    }
    char *code = mfGetObjectCode(cgPr);     
    cgDestroyProgram(cgPr);
    return code;
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
      m_CGProfileType == CG_PROFILE_VP20 ? "vp20" : "arbvp1");

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
      Warning( 0,0,"CG compiler (cgc.exe) wasn't able to compile vertex shader '%s'", m_Name.c_str());
      mfSaveCGFile(prog_text);
      return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (size < 20)
    {
      Warning( 0,0,"CG compiler (cgc.exe) wasn't able to compile vertex shader '%s'", m_Name.c_str());
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

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

  void mfLoad(const char *prog_text)
  {
    if (m_CGProfileType == CG_PROFILE_VP20)
    {
      glGenProgramsNV(1, &m_Insts[m_CurInst].m_dwHandle);
      int size = strlen(prog_text);
      glLoadProgramNV(GL_VERTEX_PROGRAM_NV, m_Insts[m_CurInst].m_dwHandle, size, (const GLubyte *)prog_text);
      GLint errpos = -1;
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_NV, &errpos);
      if(errpos != -1)
      {
        iLog->Log("Warning: VProgram error:\n");
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
    if (m_CGProfileType == CG_PROFILE_ARBVP1)
    {
      glGenProgramsARB(1, &m_Insts[m_CurInst].m_dwHandle);
      glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_Insts[m_CurInst].m_dwHandle);
      int size = strlen(prog_text);
      glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, size, (const GLubyte *)prog_text);
      GLint errpos;
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errpos);
      if(errpos != -1)
      {
        iLog->Log("Warning: VProgram error:\n");
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
  }

  void mfParameter(SCGBind *ParamBind, const float *v, int nComps)
  {
    int i;
    if (!ParamBind)
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
    if (m_CGProfileType == CG_PROFILE_VP20)
    {
      for (i=0; i<ParamBind->m_nBindComponents; i++)
      {
        int n = ParamBind->m_dwBind+i;
        if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
        {
          m_CurParams[n][0] = v[0];
          m_CurParams[n][1] = v[1];
          m_CurParams[n][2] = v[2];
          m_CurParams[n][3] = v[3];
          glProgramParameter4fvNV(GL_VERTEX_PROGRAM_NV, n, v);
        }
        v += 4;
      }
    }
    else
    {
      for (i=0; i<ParamBind->m_nBindComponents; i++)
      {
        int n = ParamBind->m_dwBind+i;
        if (m_CurParams[n][0] != v[0] || m_CurParams[n][1] != v[1] || m_CurParams[n][2] != v[2] || m_CurParams[n][3] != v[3])
        {
          m_CurParams[n][0] = v[0];
          m_CurParams[n][1] = v[1];
          m_CurParams[n][2] = v[2];
          m_CurParams[n][3] = v[3];
          glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, n, v);
        }
        v += 4;
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

  void mfParameterStateMatrix(SCGMatrix *tm)
  {
    int i;

    if (!tm->m_dwBind || tm->m_dwFrameCreated != m_dwFrame)
    {
      tm->m_dwFrameCreated = m_dwFrame;
      if (m_Insts[m_CurInst].m_BindVars)
      {
        for (i=0; i<m_Insts[m_CurInst].m_BindVars->Num(); i++)
        {
          SCGBind *p = &m_Insts[m_CurInst].m_BindVars->Get(i);
          if (p->m_Name == tm->m_Name)
          {
            assert(p->m_nComponents == 4);
            tm->m_dwBind = p->m_dwBind;
            if (tm->m_dwBind == 0)
              tm->m_dwBind = 65536;
            break;
          }
        }
        if (i == m_Insts[m_CurInst].m_BindVars->Num())
          tm->m_dwBind = -1;
      }
      else
        tm->m_dwBind = -1;
    }
    if (tm->m_dwBind == -1)
      return;

    float *v = NULL;
    Matrix44 m;
    CGLRenderer *r = gcpOGL;
    switch(tm->m_eCGParamType)
    {
      case ECGP_Matr_ViewProj:
        if (!(r->m_RP.m_ObjFlags & FOB_TRANS_MASK))
        {
          if (r->m_RP.m_PersFlags & RBPF_WASWORLDSPACE)
            break;
          r->m_RP.m_PersFlags |= RBPF_WASWORLDSPACE;
        }
        else
          r->m_RP.m_PersFlags &= ~RBPF_WASWORLDSPACE;
        if (m_FrameObj == r->m_RP.m_FrameObject)
          return;
        m_FrameObj = r->m_RP.m_FrameObject;
        mathMatrixTranspose(m.GetData(), r->m_RP.m_pCurObject->GetVPMatrix().GetData(), g_CpuFlags);
        v = m.GetData();
        break;
      case ECGP_Matr_View_IT:
        mathMatrixInverse(m.GetData(), r->m_ViewMatrix.GetData(), g_CpuFlags);
        v = m.GetData();
        break;
      case ECGP_Matr_View:
        m = r->m_ViewMatrix;
        mathMatrixTranspose(m.GetData(), m.GetData(), g_CpuFlags);
        v = m.GetData();
        break;
      case ECGP_Matr_View_I:
        mathMatrixInverse(m.GetData(), r->m_ViewMatrix.GetData(), g_CpuFlags);
        mathMatrixTranspose(m.GetData(), m.GetData(), g_CpuFlags);
        v = m.GetData();
        break;
      case ECGP_Matr_View_T:
        v = r->m_ViewMatrix.GetData();
        break;
      default:
        iLog->Log("Unknown matrix state type %d int CG program '%s'", m_Name.c_str());
        assert(0);
    }
    if (v)
    {
      int n = (tm->m_dwBind == 65536) ? 0 : tm->m_dwBind;
      if (m_CGProfileType == CG_PROFILE_VP20)
      {
        glProgramParameters4fvNV(GL_VERTEX_PROGRAM_NV, n, 4, v);
        if (n)
          memcpy(&m_CurParams[n][0], v, 4*4*sizeof(float));
      }
      else
      {
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, n, v);
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, n+1, v+4);
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, n+2, v+8);
        glProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, n+3, v+12);
        if (n)
          memcpy(&m_CurParams[n][0], v, 4*4*sizeof(float));
      }
    }
  }

  void mfEnable()
  {
    if (m_CGProfileType == CG_PROFILE_VP20)
      glEnable(GL_VERTEX_PROGRAM_NV);
    else
    if (m_CGProfileType == CG_PROFILE_ARBVP1)
      glEnable(GL_VERTEX_PROGRAM_ARB);
  }
  void mfDisable()
  {
    if (m_CGProfileType == CG_PROFILE_VP20)
      glDisable(GL_VERTEX_PROGRAM_NV);
    else
    if (m_CGProfileType == CG_PROFILE_ARBVP1)
      glDisable(GL_VERTEX_PROGRAM_ARB);
    gRenDev->m_RP.m_PersFlags &= ~RBPF_VSWASSET;
  }

  void mfFree();

  void mfDel()
  {
    if(m_Insts[m_CurInst].m_dwHandle >= 0)
    {
      if (m_Insts[m_CurInst].m_BindConstants)
        delete m_Insts[m_CurInst].m_BindConstants;
      if (m_Insts[m_CurInst].m_BindVars)
        delete m_Insts[m_CurInst].m_BindVars;
      if (m_Insts[m_CurInst].m_ParamsNoObj)
        delete m_Insts[m_CurInst].m_ParamsNoObj;
      if (m_Insts[m_CurInst].m_MatrixObj)
        delete m_Insts[m_CurInst].m_MatrixObj;
      if (m_CGProfileType == CG_PROFILE_VP20)
        glDeleteProgramsNV(1, &m_Insts[m_CurInst].m_dwHandle);
      else
      if (m_CGProfileType == CG_PROFILE_ARBVP1)
        glDeleteProgramsARB(1, &m_Insts[m_CurInst].m_dwHandle);
    }
    m_Insts[m_CurInst].m_dwHandle = 0;
  }
  
  void mfSetVariables(TArray<SCGParam4f>* Parms);

  bool mfIsValid(int Num) const { return (m_Insts[m_CurInst].m_dwHandle != 0); }
  void mfGetSrcFileName(char *srcname, int nSize);
  void mfGetDstFileName(char *dstname, int nSize);

public:
  SCGScript *mfGenerateScriptVP(CVProgram *pVP);
  char *mfGenerateTCScript(char *Script, int nt);
  char *mfCreateAdditionalVP(CVProgram *pPosVP);
  void mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params);
  void mfCompileParamStateMatrix(char *scr, SShader *ef, TArray<SCGMatrix> *Params);
  void mfCompileVertAttributes(char *scr, SShader *ef);
  bool mfActivate(CVProgram *pPosVP);

public:
  virtual ~CCGVProgram_GL();
  virtual void Release();
  virtual bool mfCompile(char *scr);
  virtual bool mfSet(bool bStat, SShaderPassHW *slw, int nSetPointers=1);
  virtual void mfSetVariables(bool bObj, TArray<SCGParam4f>* Vars);
  virtual void mfReset();
  virtual void mfPrecache();
  virtual bool mfHasPointer(ESrcPointer ePtr);
  virtual void mfGatherFXParameters(const char *buf, SShaderPassHW *pSHPass, std::vector<SFXParam>& Params, std::vector<SFXSampler>& Samplers, std::vector<SFXTexture>& Textures, SShader *ef) {};
  virtual void mfPostLoad() {};
  virtual int  mfVertexFormat(bool &bUseTangents, bool &bUseLM) { return 0; }
  virtual void mfSetStateMatrices()
  {
    //PROFILE_FRAME(Shader_VShadersMatr);
    for (int i=0; i<m_MatrixObj.Num(); i++)
    {
      SCGMatrix *tm = &m_MatrixObj[i];
      mfParameterStateMatrix(tm);
    }
    if (m_Insts[m_CurInst].m_MatrixObj)
    {
      for (int i=0; i<m_Insts[m_CurInst].m_MatrixObj->Num(); i++)
      {
        SCGMatrix *tm = &m_Insts[m_CurInst].m_MatrixObj->Get(i);
        mfParameterStateMatrix(tm);
      }
    }
  }
  static void mfSetGlobalParams();
  static bool m_bCreateNoise;

  char *mfGetObjectCode(CGprogram cgPr)
  {
    const char *code = cgGetProgramString(cgPr, CG_COMPILED_PROGRAM);
    int size = strlen(code)+1;
    char *str = new char[size];
    cryMemcpy(str, code, size);
    return str;
  }
  static SCGScript *mfAddNewScript(const char *Name, const char *Script)
  {
    int i;
    if (Name)
    {
      CName nm = CName(Name);
      if (nm.GetIndex())
      {
        for (i=0; i<m_CGScripts.Num(); i++)
        {
          SCGScript *scr = m_CGScripts[i];
          if (!scr || !scr->m_Name.GetIndex())
            continue;
          if (nm == scr->m_Name)
          {
            if (!Script || stricmp(Script, scr->m_Script) == 0)
              return scr;
            delete [] scr->m_Script;
            break;
          }
        }
      }
    }
    if (!Script)
    {
      if (Name)
        iLog->Log("Error: CCGVProgram_GL::mfAddNewScript: Couldn't find CG script for name '%s'", Name);
      return NULL;
    }

    SCGScript *scr = new SCGScript;
    if (Name)
      scr->m_Name = CName(Name, eFN_Add);

    int len = strlen(Script)+1;
    scr->m_Script = new char[len];
    strcpy(scr->m_Script, Script);

    if (Name)
      m_CGScripts.AddElem(scr);

    return scr;
  }
  static void mfDeleteSharedScripts()
  {
    int i;

    for (i=0; i<m_CGScripts.Num(); i++)
    {
      SCGScript *scr = m_CGScripts[i];
      if (!scr)
        continue;
      SAFE_DELETE_ARRAY(scr->m_Script);
      delete scr;
      m_CGScripts[i] = NULL;
    }
  }

  static TArray<SCGScript *> m_CGScripts;
  static vec4_t m_CurParams[96];
  static vec4_t m_CurParamsARB[96];
};

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

#endif  // __GLCGVPROGRAMS_H__

/*=============================================================================
  D3DCGVProgram.h : Direct3D CG programs interface declaration.
  Copyright 1999 K&M. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#ifndef __D3DCGVPROGRAM_H__
#define __D3DCGVPROGRAM_H__

#ifndef _XBOX
#include "cg\cgD3D8.h"
#endif

#define CG_CACHE_VER    1.0

#define VPFI_NOFOG      1
#define VPFI_UNIFIEDPOS 2
#define VPFI_SUPPORTMULTVLIGHTS 4
#define VPFI_NOISE      8


class CCGVProgram_D3D : public CVProgram
{
  uint m_Flags;

  SCGScript *m_Script;
  SCGScript *m_PosScript;
  SCGScript *m_DeclarationsScript;
  SCGScript *m_CoreScript;
  SCGScript *m_InputParmsScript;

  TArray<SCGParam4f> m_ParamsNoObj;
  TArray<SCGMatrix> m_MatrixNoObj;
  TArray<SCGParam4f> m_ParamsObj;
  TArray<SCGMatrix> m_MatrixObj;
  TArray<SArrayPointer *> m_Pointers;

  int m_CurInst;
#ifndef _XBOX
  CGprofile m_CGProfileType;
#endif
  struct SCGInstance
  {
    int Streams;
    int VertFormat;
    int m_Mask;
    CName m_PosScriptName;
    TArray<SCGParam4f> *m_ParamsNoObj;
    TArray<SCGMatrix> *m_MatrixNoObj;
    TArray<SCGParam4f> *m_ParamsObj;
    TArray<SCGMatrix> *m_MatrixObj;
    TArray<SCGBindConst> *m_BindConstants;
    TArray<SCGBind> *m_BindVars;
    union
    {
#ifndef _XBOX
      CGprogram m_CGProgram;
#endif
      DWORD m_dwHandle;
    };
  };
  TArray<SCGInstance> m_Insts;

public:
  virtual int Size()
  {
    return 0;
  }

#ifndef _XBOX
  void mfSaveCGFile(const char *scr)
  {
    char name[128];
    sprintf(name, "%s.cg", m_Name.c_str());
    FILE *fp = fopen(name, "w");
    fprintf(fp, scr);
    fclose (fp);
  }
  static void mfCgErrorCallback()
  {
    CGerror error = cgGetError(); // Recall that cgGetError() removes the error from the stack error, so you won't be able to check it later on
    const char* str = cgD3D8TranslateCGerror(error);
    CCGVProgram_D3D *pVP = (CCGVProgram_D3D *)gRenDev->m_RP.m_CurVP;
    iLog->Log("WARNING: CCGVProgram_D3D::mfError: (CGError: '%s' in CG program '%s')\n", str, pVP->m_Name.c_str());
    if (error == cgD3D8Failed)
    {
      HRESULT hres = cgD3D8GetLastError();
      const char* errStr = cgD3D8TranslateHRESULT(hres);
      iLog->Log("WARNING: CCGVProgram_D3D::mfError: (D3DError: '%s' in CG program '%s')\n", errStr, pVP->m_Name.c_str());
    }
    else
    if (error == cgD3D8DebugTrace)
      bool debugTrace = true;
    else
      bool otherError = true;
    if (gRenDev->m_RP.m_CurVP && gRenDev->m_RP.m_CurVP->mfGetCurScript())
    {
      pVP->mfSaveCGFile(gRenDev->m_RP.m_CurVP->mfGetCurScript());
    }
  }
#endif

  CCGVProgram_D3D()
  {
    mfInit();
#ifndef _XBOX
    m_CGProfileType = cgD3D8GetLatestVertexProfile();
#endif
  }
  void mfInit()
  {
#ifndef _XBOX
    if (!gcpRendD3D->m_CGContext)
    {
      cgD3D8SetDevice(gcpRendD3D->mfGetD3DDevice());
      gcpRendD3D->m_CGContext = cgCreateContext();
      assert(gcpRendD3D->m_CGContext);
      cgSetErrorCallback(mfCgErrorCallback);
#ifdef _DEBUG
      cgD3D8EnableDebugTracing(true);
#endif
    }
#endif
    m_CurScript = NULL;
    m_CurInst = -1;
    m_Flags = 0;
    m_Script = NULL;
    m_CoreScript = NULL;
    m_InputParmsScript = NULL;
    m_PosScript = NULL;
    m_DeclarationsScript = NULL;
    m_bCGType = true;
  }
  void mfFree();

  void mfGetSrcFileName(char *srcname);
  void mfGetDstFileName(char *dstname, SCGScript *pPosScr);

  void mfUnbind()
  {
  }

  void mfBind()
  {
    HRESULT hr;
    if (m_Insts[m_CurInst].m_dwHandle)
    {
      hr = gcpRendD3D->mfGetD3DDevice()->SetVertexShader(m_Insts[m_CurInst].m_dwHandle);
      if (FAILED(hr))
        return;
    }
    if (m_Insts[m_CurInst].m_BindConstants)
    {
      int i;
      for (i=0; i<m_Insts[m_CurInst].m_BindConstants->Num(); i++)
      {
        SCGBindConst *p = &m_Insts[m_CurInst].m_BindConstants->Get(i);
        gcpRendD3D->mfGetD3DDevice()->SetVertexShaderConstant(p->m_dwBind, &p->m_Val[0], 1);
      }
    }
  }
  int mfGetCGInstanceID(int Streams, int VertFormat, int Num, CCGVProgram_D3D *pPosVP)
  {
    SCGInstance *cgc;
    if (m_CurInst >= 0 && m_Insts.Num() > m_CurInst)
    {
      cgc = &m_Insts[m_CurInst];
      if (cgc->Streams == Streams && cgc->VertFormat == VertFormat && cgc->m_Mask == Num)
      {
        if (!pPosVP || pPosVP->m_PosScript->m_Name == cgc->m_PosScriptName)
          return m_CurInst;
      }
    }
    int i;
    for (i=0; i<m_Insts.Num(); i++)
    {
      cgc = &m_Insts[i];
      if (cgc->Streams == Streams && cgc->VertFormat == VertFormat && cgc->m_Mask == Num && (!pPosVP || pPosVP->m_PosScript->m_Name == cgc->m_PosScriptName))
      {
        m_CurInst = i;
        return i;
      }
    }
    SCGInstance cg;
    cg.Streams = Streams;
    cg.VertFormat = VertFormat;
    cg.m_Mask = Num;
    cg.m_BindConstants = NULL;
    cg.m_BindVars = NULL;
    cg.m_dwHandle = 0;
#ifndef _XBOX
    cg.m_CGProgram = NULL;
#endif
    cg.m_ParamsNoObj = NULL;
    cg.m_ParamsObj = NULL;
    cg.m_MatrixNoObj = NULL;
    cg.m_MatrixObj = NULL;
    if (Num & VPVST_FOGGLOBAL)
    {
      if (!cg.m_ParamsNoObj)
        cg.m_ParamsNoObj = new TArray<SCGParam4f>;
      SCGParam4f pr;
      pr.m_Name = "Fog";
      pr.m_nComponents = 1;
      pr.m_pBind = NULL;
      SParamComp_Fog p;
      p.m_Type = 2;
      pr.m_Comps[0] = SParamComp::mfAdd(&p);
      p.m_Type = 3;
      pr.m_Comps[1] = SParamComp::mfAdd(&p);
      cg.m_ParamsNoObj->AddElem(pr);
    }
    if (Num & VPVST_CLIPPLANES)
    {
      if (!cg.m_ParamsNoObj)
        cg.m_ParamsNoObj = new TArray<SCGParam4f>;
      SCGParam4f pr;
      pr.m_nComponents = 1;
      pr.m_pBind = NULL;
      pr.m_Name = "ClipPlane";
      SParamComp_ClipPlane p;
      pr.m_Flags = PF_CANMERGED;
      pr.m_Comps[0] = SParamComp::mfAdd(&p);
      cg.m_ParamsNoObj->AddElem(pr);
    }

    if (pPosVP && pPosVP->m_PosScript)
    {
      cg.m_PosScriptName = pPosVP->m_PosScript->m_Name;
      if (cg.m_PosScriptName != m_PosScript->m_Name && pPosVP->m_ParamsNoObj.Num())
      {
        if (!cg.m_ParamsNoObj)
          cg.m_ParamsNoObj = new TArray<SCGParam4f>;
        for (int i=0; i<pPosVP->m_ParamsNoObj.Num(); i++)
        {
          cg.m_ParamsNoObj->AddElem(pPosVP->m_ParamsNoObj[i]);
          cg.m_ParamsNoObj->Get(cg.m_ParamsNoObj->Num()-1).m_pBind = NULL;
          cg.m_ParamsNoObj->Get(cg.m_ParamsNoObj->Num()-1).m_dwBind = 0;
        }
        cg.m_ParamsNoObj->Shrink();
        for (int i=0; i<pPosVP->m_ParamsObj.Num(); i++)
        {
          cg.m_ParamsObj->AddElem(pPosVP->m_ParamsObj[i]);
          cg.m_ParamsObj->Get(cg.m_ParamsObj->Num()-1).m_pBind = NULL;
          cg.m_ParamsObj->Get(cg.m_ParamsObj->Num()-1).m_dwBind = 0;
        }
        cg.m_ParamsObj->Shrink();
      }
    }
    m_Insts.AddElem(cg);
    m_CurInst = m_Insts.Num()-1;
    return m_CurInst;
  }

#ifndef _XBOX
  const char *mfGetObjectCode(CGprogram cgPr)
  {
    return cgGetProgramString(cgPr, CG_COMPILED_PROGRAM);
  }

  const char *mfLoadCG(const char *prog_text)
  {
    // Test adding source text to context
    m_CurScript = prog_text;
    const char *profileOpts[] =
    {
      cgD3D8GetOptimalOptions(m_CGProfileType),
      NULL,
    };
    m_Insts[m_CurInst].m_CGProgram = cgCreateProgram(gcpRendD3D->m_CGContext, CG_SOURCE, prog_text, m_CGProfileType, "main", profileOpts);    
    CGerror err = cgGetError();
    if (err != CG_NO_ERROR)
    {
      iLog->Log("Couldn't create CG program '%s' (%s)", m_Name.c_str(), cgGetErrorString(err));
      mfSaveCGFile(prog_text);
      m_CurScript = NULL;
      return NULL;
    }
    if (!m_Insts[m_CurInst].m_CGProgram)
    {
      iLog->Log("Couldn't load function '%s' in CG program '%s'", "main", m_Name.c_str());
      m_CurScript = NULL;
      return NULL;
    }
    // Assemble the shader
    if (FAILED(cgD3D8LoadProgram(m_Insts[m_CurInst].m_CGProgram, false, 0, 0, &gRenDev->m_RP.m_D3DFixedPipeline[m_Insts[m_CurInst].Streams][m_Insts[m_CurInst].VertFormat].m_Declaration[0])))
    {
      iLog->Log("Couldn't load CG vertex program '%s'", m_Name.c_str());
      m_CurScript = NULL;
      return NULL;
    }
    m_CurScript = NULL;
    if (m_Insts[m_CurInst].m_CGProgram)
    {
      const char *code = mfGetObjectCode(-1);     
      return code;
    }
    return NULL;
  }
#endif

  void mfLoad(const char *prog_text)
  {
    // Load and create vertex shader
    HRESULT hr;
#ifdef _XBOX
    LPXGBUFFER pCode;
    LPXGBUFFER pBuffer = NULL;
    hr = XGAssembleShader(m_Name.c_str(), prog_text, strlen(prog_text), 0, NULL, &pCode, &pBuffer, NULL, NULL, NULL, 0);
#else
    LPD3DXBUFFER pCode;
    LPD3DXBUFFER pBuffer = NULL;
    hr = D3DXAssembleShader( prog_text, strlen(prog_text), 0, NULL, &pCode, &pBuffer );
#endif
    if (FAILED(hr))
    {
      iLog->Log("WARNING: CCGVProgram_D3D::mfLoad: Could not assemble vertex shader '%s' (%s)\n", m_Name.c_str(), gcpRendD3D->D3DError(hr));
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
            *pstrOut = '\0';
          pstrOut++;
          pstr++;
        }
        // remove any blank lines at the end
        while( strOut[lstrlen(strOut) - 1] == '\n' || strOut[lstrlen(strOut) - 1] == '\r' )
        {
          strOut[lstrlen(strOut) - 1] = '\0';
        }
        iLog->Log("WARNING: CCGVProgram_D3D::mfLoad: Shader script error (%s)\n", strOut);
        SAFE_RELEASE( pBuffer );
      }
    }
    hr = gcpRendD3D->mfGetD3DDevice()->CreateVertexShader( &gRenDev->m_RP.m_D3DFixedPipeline[m_Insts[m_CurInst].Streams][m_Insts[m_CurInst].VertFormat].m_Declaration[0], (DWORD*)pCode->GetBufferPointer(), (DWORD *)&m_Insts[m_CurInst].m_dwHandle, 0);
    if (FAILED(hr))
    {
      iLog->Log("CCGVProgram_D3D::mfLoad: Could not create vertex shader '%s'\n", m_Name.c_str());
      return;
    }
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

  void mfParameter(SCGBind *ParamBind, const float *v, int nComps)
  {
    if (!ParamBind->m_dwBind)
    {
      if (m_Insts[m_CurInst].m_BindVars)
      {
        int i;
        for (i=0; i<m_Insts[m_CurInst].m_BindVars->Num(); i++)
        {
          SCGBind *p = &m_Insts[m_CurInst].m_BindVars->Get(i);
          if (p->m_Name == ParamBind->m_Name)
          {
            assert(p->m_nComponents <= nComps);
            ParamBind->m_dwBind = p->m_dwBind;
            if (ParamBind->m_dwBind == 0)
              ParamBind->m_dwBind = 65536;
            ParamBind->m_nBindComponents = p->m_nComponents;
            break;
          }
        }
        if (i == m_Insts[m_CurInst].m_BindVars->Num())
          ParamBind->m_dwBind = -1;
      }
      else
        ParamBind->m_dwBind = -1;
    }
    if (ParamBind->m_dwBind == -1)
      return;
    if (ParamBind->m_dwBind == 65536)
      gcpRendD3D->mfGetD3DDevice()->SetVertexShaderConstant(0, v, ParamBind->m_nBindComponents);
    else
      gcpRendD3D->mfGetD3DDevice()->SetVertexShaderConstant(ParamBind->m_dwBind, v, ParamBind->m_nBindComponents);
  }

  void mfParameter4f(SCGBind *ParamBind, const vec4_t v)
  {
    mfParameter(ParamBind, v, 1);
  }


  int m_ObjFrame;
  void mfParameterStateMatrix(SCGMatrix *ParamBind)
  {
    static int sFrame;
    static int sFlags;
    static D3DXMATRIX matWorldViewProj;
    CD3D8Renderer *r = gcpRendD3D;
    LPDIRECT3DDEVICE8 dv = r->mfGetD3DDevice();
    D3DXMATRIX *matView = r->m_matView->GetTop();
    switch(ParamBind->m_eCGParamType)
    {
      case ECGP_Matr_ViewProj:
        {
          if (sFrame != r->m_RP.m_FrameObject)
          {
            sFrame = r->m_RP.m_FrameObject;
            D3DXMATRIX *matProj = r->m_matProj->GetTop();
    	      D3DXMatrixMultiply(&matWorldViewProj, matView, matProj);
            D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);
          }
          if (m_ObjFrame != r->m_RP.m_FrameObject)
          {
            m_ObjFrame = r->m_RP.m_FrameObject;
            mfParameter(ParamBind, &matWorldViewProj(0, 0), 4);
          }
        }
    	  break;
      case ECGP_Matr_View_IT:
        D3DXMatrixInverse(&matWorldViewProj, NULL, matView);
        mfParameter(ParamBind, &matWorldViewProj(0, 0), 4);
    	  break;
      case ECGP_Matr_View:
        D3DXMatrixTranspose(&matWorldViewProj, matView);
        mfParameter(ParamBind, &matWorldViewProj(0, 0), 4);
        break;
      case ECGP_Matr_View_I:
        D3DXMatrixInverse(&matWorldViewProj, NULL, matView);
        D3DXMatrixTranspose(&matWorldViewProj, &matWorldViewProj);
        mfParameter(ParamBind, &matWorldViewProj(0, 0), 4);
        break;
      case ECGP_Matr_View_T:
        mfParameter(ParamBind, (float *)matView, 4);
    	  break;
      default:
        iLog->Log("Unknown matrix state type %d int CG program '%s'", m_Name.c_str());
        assert(0);
    }
  }

  void mfSetStateMatrices(bool bObj)
  {
    if (!bObj)
    {
      for (int i=0; i<m_MatrixNoObj.Num(); i++)
      {
        SCGMatrix *tm = &m_MatrixNoObj[i];
        mfParameterStateMatrix(tm);
      }
      if (m_Insts[m_CurInst].m_MatrixNoObj)
      {
        for (int i=0; i<m_Insts[m_CurInst].m_MatrixNoObj->Num(); i++)
        {
          SCGMatrix *tm = &m_Insts[m_CurInst].m_MatrixNoObj->Get(i);
          mfParameterStateMatrix(tm);
        }
      }
    }
    else
    {
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
  }


  void mfDel()
  {
    if(m_Insts[m_CurInst].m_dwHandle && (int)m_Insts[m_CurInst].m_dwHandle != -1)
    {
      if (m_Insts[m_CurInst].m_BindConstants)
        delete m_Insts[m_CurInst].m_BindConstants;
      if (m_Insts[m_CurInst].m_BindVars)
        delete m_Insts[m_CurInst].m_BindVars;
      if (m_Insts[m_CurInst].m_ParamsNoObj)
        delete m_Insts[m_CurInst].m_ParamsNoObj;
      if (m_Insts[m_CurInst].m_ParamsObj)
        delete m_Insts[m_CurInst].m_ParamsObj;
      if (m_Insts[m_CurInst].m_MatrixNoObj)
        delete m_Insts[m_CurInst].m_MatrixNoObj;
      if (m_Insts[m_CurInst].m_MatrixObj)
        delete m_Insts[m_CurInst].m_MatrixObj;
      if (m_Insts[m_CurInst].m_dwHandle)
      {
        gcpRendD3D->mfGetD3DDevice()->DeleteVertexShader(m_Insts[m_CurInst].m_dwHandle);
        m_Insts[m_CurInst].m_dwHandle = 0;
      }
    }
    m_Insts[m_CurInst].m_dwHandle = 0;
  }
  
  bool mfIsValid(int Num) const { return (m_Insts[Num].m_dwHandle != 0); }
  SCGScript *mfGenerateScriptVP(SCGScript *pPosScript);
  void mfCompileVertAttributes(char *scr, SShader *ef);
  void mfSetVariables(TArray<SCGParam4f>* Vars);

public:
  char *mfCreateAdditionalVP(SCGScript *pPosScript);
  void mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params, int nLights);
  void mfCompileParamStateMatrix(char *scr, SShader *ef, TArray<SCGMatrix> *Params);
  bool mfActivate(CCGVProgram_D3D *pPosVP);

public:
  virtual ~CCGVProgram_D3D();
  virtual bool mfCompile(char *scr);
  virtual bool mfSet(bool bStat, SShaderPassHW *slw, int nSetPointers=1);
  virtual void mfSetVariables(bool bObj, TArray<SCGParam4f>* Vars);
  virtual void mfReset();
  virtual void mfPrecache();
  virtual bool mfHasPointer(ESrcPointer ePtr);
  virtual const char *mfGetCurScript() { return m_CurScript; }

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
};


#endif  // __D3DCGVPROGRAMS_H__

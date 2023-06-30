/*=============================================================================
  D3DCGPShader.h : Direct3D8 CG pixel shaders interface declaration.
  Copyright 1999 K&M. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#ifndef __D3DCGPSHADER_H__
#define __D3DCGPSAHDER_H__

#ifndef _XBOX
#include "cg\cgD3D8.h"
#endif
#include "D3DPShaders.h"

class CCGPShader_D3D : public CPShader
{
  uint m_Flags;

  SCGScript *m_Script;
  SCGScript *m_DeclarationsScript;
  SCGScript *m_CoreScript;
  SCGScript *m_InputParmsScript;

  TArray<SCGParam4f> m_Params;

  int m_CurInst;
#ifndef _XBOX
  CGprofile m_CGProfileType;
#endif
  struct SCGInstance
  {
    int m_Mask;
    TArray<SCGParam4f> *m_Params;
    TArray<SCGBindConst> *m_BindConstants;
    TArray<SCGBind> *m_BindVars;
    union
    {
#ifndef _XBOX
      CGprogram m_CGProgram;
#endif
      int m_dwHandle;
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
    CCGPShader_D3D *pPS = (CCGPShader_D3D *)gRenDev->m_RP.m_CurPS;
    iLog->Log("WARNING: CCGPShader_D3D::mfError: (CGError: '%s' in CG program '%s')\n", str, pPS->m_Name.c_str());
    if (error == cgD3D8Failed)
    {
      HRESULT hres = cgD3D8GetLastError();
      const char* errStr = cgD3D8TranslateHRESULT(hres);
      iLog->Log("WARNING: CCGPShader_D3D::mfError: (D3DError: '%s' in CG program '%s')\n", errStr, pPS->m_Name.c_str());
    }
    else
    if (error == cgD3D8DebugTrace)
      bool debugTrace = true;
    else
      bool otherError = true;
    if (gRenDev->m_RP.m_CurPS && gRenDev->m_RP.m_CurPS->mfGetCurScript())
    {
      pPS->mfSaveCGFile(gRenDev->m_RP.m_CurPS->mfGetCurScript());
    }
  }
#endif

  CCGPShader_D3D()
  {
    mfInit();
#ifndef _XBOX
    m_CGProfileType = cgD3D8GetLatestPixelProfile();
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
    m_DeclarationsScript = NULL;
    m_bCGType = true;
  }
  void mfFree();

  void mfGetSrcFileName(char *srcname);
  void mfGetDstFileName(char *dstname, SCGScript *pPosScr);

  void mfUnbind()
  {
    gcpRendD3D->mfGetD3DDevice()->SetPixelShader(NULL);
  }

  void mfBind()
  {
    HRESULT hr;
    if (m_Insts[m_CurInst].m_dwHandle)
    {
      hr = gcpRendD3D->mfGetD3DDevice()->SetPixelShader(m_Insts[m_CurInst].m_dwHandle);
      if (FAILED(hr))
        return;
    }
    if (m_Insts[m_CurInst].m_BindConstants)
    {
      int i;
      for (i=0; i<m_Insts[m_CurInst].m_BindConstants->Num(); i++)
      {
        SCGBindConst *p = &m_Insts[m_CurInst].m_BindConstants->Get(i);
        gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstant(p->m_dwBind, &p->m_Val[0], 1);
      }
    }
  }
  int mfGetCGInstanceID(int Num)
  {
    SCGInstance *cgc;
    if (m_CurInst >= 0 && m_Insts.Num() > m_CurInst)
    {
      cgc = &m_Insts[m_CurInst];
      if (cgc->m_Mask == Num)
      {
        return m_CurInst;
      }
    }
    int i;
    for (i=0; i<m_Insts.Num(); i++)
    {
      cgc = &m_Insts[i];
      if (cgc->m_Mask == Num)
      {
        m_CurInst = i;
        return i;
      }
    }
    SCGInstance cg;
    cg.m_Mask = Num;
    cg.m_BindConstants = NULL;
    cg.m_BindVars = NULL;
    cg.m_dwHandle = 0;
#ifndef _XBOX
    cg.m_CGProgram = NULL;
#endif
    cg.m_Params = NULL;

    m_Insts.AddElem(cg);
    m_CurInst = m_Insts.Num()-1;
    return m_CurInst;
  }

#ifndef _XBOX
  const char *mfGetObjectCode(int Num)
  {
    CGprogram pi = NULL;
    if (m_Insts[m_CurInst].m_CGProgram && (int)m_Insts[m_CurInst].m_CGProgram != -1)
      pi = m_Insts[m_CurInst].m_CGProgram;
    else
    {
      for (int i=0; i<m_Insts.Num(); i++)
      {
        if (m_Insts[i].m_CGProgram && (int)m_Insts[i].m_CGProgram != -1)
        {
          pi = m_Insts[i].m_CGProgram;
          break;
        }
      }
    }
    if (pi)
    	return cgGetProgramString(pi, CG_COMPILED_PROGRAM);
    return NULL;
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
      iLog->Log("Couldn't create CG pixel program '%s' (%s)", m_Name.c_str(), cgGetErrorString(err));
      mfSaveCGFile(prog_text);
      return NULL;
    }
    if (!m_Insts[m_CurInst].m_CGProgram)
    {
      iLog->Log("Couldn't find function '%s' in CG program '%s'", "main", m_Name.c_str());
      return NULL;
    }
    // Assemble the shader
    if (FAILED(cgD3D8LoadProgram(m_Insts[m_CurInst].m_CGProgram, false, 0, 0, NULL)))
    {
      iLog->Log("Couldn't load CG vertex program '%s'", m_Name.c_str());
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
    hr = D3DXAssembleShader(prog_text, strlen(prog_text), 0, NULL, &pCode, &pBuffer);
#endif
    if (FAILED(hr))
    {
      iLog->Log("WARNING: CCGPShader_D3D::mfLoad: Could not assemble vertex shader '%s' (%s)\n", m_Name.c_str(), gcpRendD3D->D3DError(hr));
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
        iLog->Log("WARNING: CCGPShader_D3D::mfLoad: Shader script error (%s)\n", strOut);
        SAFE_RELEASE( pBuffer );
      }
    }
#ifndef _XBOX
    hr = gcpRendD3D->mfGetD3DDevice()->CreatePixelShader((DWORD*)pCode->GetBufferPointer(), (DWORD*)&m_Insts[m_CurInst].m_dwHandle);
#else
    hr = gcpRendD3D->mfGetD3DDevice()->CreatePixelShader((D3DPIXELSHADERDEF*)pCode->GetBufferPointer(), (DWORD*)&m_Insts[m_CurInst].m_dwHandle);
#endif
    if (FAILED(hr))
    {
      iLog->Log("CCGPShader_D3D::mfLoad: Could not create vertex shader '%s' (%s)\n", m_Name.c_str(), gcpRendD3D->D3DError(hr));
      return;
    }
  }

  void mfParameter(SCGParam *ParamBind, const float *v, int nComps)
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
      if (ParamBind->m_dwBind == -1)
        return;
    }
    if (ParamBind->m_dwBind == 65536)
      gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstant(0, v, nComps);
    else
      gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstant(ParamBind->m_dwBind, v, nComps);
  }

  void mfParameter4f(SCGParam *ParamBind, const vec4_t v)
  {
    mfParameter(ParamBind, v, 1);
  }

  void mfDel()
  {
    if(m_Insts[m_CurInst].m_dwHandle && (int)m_Insts[m_CurInst].m_dwHandle != -1)
    {
      if (m_Insts[m_CurInst].m_BindConstants)
        delete m_Insts[m_CurInst].m_BindConstants;
      if (m_Insts[m_CurInst].m_BindVars)
        delete m_Insts[m_CurInst].m_BindVars;
      if (m_Insts[m_CurInst].m_Params)
        delete m_Insts[m_CurInst].m_Params;
      if (m_Insts[m_CurInst].m_dwHandle)
      {
        gcpRendD3D->mfGetD3DDevice()->DeletePixelShader(m_Insts[m_CurInst].m_dwHandle);
        m_Insts[m_CurInst].m_dwHandle = 0;
      }
    }
    m_Insts[m_CurInst].m_dwHandle = 0;
  }
  
  bool mfIsValid(int Num) const { return (m_Insts[Num].m_dwHandle != 0); }
  SCGScript *mfGenerateScriptVP();
  void mfSetVariables(TArray<SCGParam4f>* Vars);

public:
  char *mfCreateAdditionalVP();
  void mfCompileParam4f(char *scr, SShader *ef, TArray<SCGParam4f> *Params, int nLights);
  bool mfActivate() { return false; }

public:
  virtual ~CCGPShader_D3D() {};
  virtual bool mfCompile(char *scr) { return false; }
  virtual bool mfSet(bool bStat, int nSetPointers=1) { return false; }
  virtual void mfSetVariables(TArray<SParam>* Vars) {}
  virtual void mfReset() {}
  virtual const char *mfGetCurScript() { return m_CurScript; }
  virtual bool mfIsCombiner() { return false; }
  virtual void mfEnable() {}
  virtual void mfDisable() {}

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
        iLog->Log("Error: CCGPShader_GL::mfAddNewScript: Couldn't find CG script for name '%s'", Name);
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

  const char *m_CurScript;
  static TArray<SCGScript *> m_CGScripts;
};


#endif  // __D3DCGPSHADER_H__

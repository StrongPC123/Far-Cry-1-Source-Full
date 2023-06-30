/*=============================================================================
  D3DPShaders.cpp : pixel shaders support for D3D.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#include "stdafx.h"
#include "DriverD3D8.h"
#include "D3DCGPShader.h"

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

// init memory pool usage
#ifndef PS2
#ifndef _XBOX
 _ACCESS_POOL;
#endif
#endif

TArray<CPShader *> CPShader::m_PShaders;
CPShader *CPShader::m_CurRC;

//=======================================================================

CPShader *CPShader::mfForName(char *name, bool bCGType)
{
  int i;

  for (i=0; i<m_PShaders.Num(); i++)
  {
    if (!m_PShaders[i])
      continue;
    if (!stricmp(name, m_PShaders[i]->m_Name.c_str()))
      return m_PShaders[i];
  }
  char scrname[128];
  char dir[128];
  if (!bCGType)
  {
    sprintf(dir, "%sDeclarations/PShaders/", gcEf.m_HWPath);
    sprintf(scrname, "%s%s.cryps", dir, name);
  }
  else
  {
    sprintf(dir, "%sDeclarations/CGPShaders/", gcEf.m_HWPath);
    sprintf(scrname, "%s%s.crycg", dir, name);
  }
  FILE *fp = iSystem->GetIPak()->FOpen(scrname, "r");
  if (!fp)
  {
    iLog->Log("WARNING: Couldn't find pixel shader '%s'", name);
    iLog->Log("WARNING: Full file name: '%s'", scrname);
    return NULL;
  }
  iSystem->GetIPak()->FSeek(fp, 0, SEEK_END);
  int len = iSystem->GetIPak()->FTell(fp);
  char *buf = new char [len+1];
  iSystem->GetIPak()->FSeek(fp, 0, SEEK_SET);
  len = iSystem->GetIPak()->FRead(buf, 1, len, fp);
  iSystem->GetIPak()->FClose(fp);
  buf[len] = 0;
  buf = gcEf.mfScriptPreprocessor(buf, dir, scrname);

  CPShader *p = NULL;
  if (!bCGType)
  {
    CPShader_D3D *pr = new CPShader_D3D;
    pr->m_Name = name;
    pr->m_Id = i;
    m_PShaders.AddElem(pr);
    pr->m_RefCounter = 1;
    p = pr;
  }
  else
  {
    CPShader *pr;
    pr = new CCGPShader_D3D;
    pr->m_Name = name;
    pr->m_Id = i;
    m_PShaders.AddElem(pr);
    pr->m_RefCounter = 1;
    p = pr;
  }
  HANDLE statussrc = CreateFile(scrname,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (statussrc != INVALID_HANDLE_VALUE)
  {
    GetFileTime(statussrc,NULL,NULL,&p->m_WriteTime);
    CloseHandle(statussrc);
  }

  p->mfCompile(buf);
  delete [] buf;

  return p;
}

//=============================================================================

void CPShader::mfClearAll(void)
{
  for (int i=0; i<m_PShaders.Num(); i++)
  {
    CPShader *vp = m_PShaders[i];
    if (vp)
      delete vp;
  }
  m_PShaders.Free();
}

bool CPShader_D3D::mfCompile(char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;

  guard(CPShader_D3D::mfCompile);

  enum {eScript=1, eParam};
  static tokenDesc commands[] =
  {
    {eScript, "Script"},
    {eParam, "Param"},

    {0,0}
  };

  SShader *ef = gcEf.m_CurShader;

  while ((cmd = shGetObject (&scr, commands, &name, &params)) > 0)
  {
    data = NULL;
    if (name)
      data = name;
    else
    if (params)
      data = params;

    switch (cmd)
    {
      case eScript:
        {
          if (!(gRenDev->GetFeatures() & RFT_HW_TS))
          {
            iLog->Log("Render device doesn't support texture shaders (Shader '%s')\n", ef->m_Name.c_str());
            break;
          }
          int n = strlen(params);
          m_pScript = new char[n+1];
          memcpy(m_pScript, params, n);
          m_pScript[n] = 0;
        }
        break;

      case eParam:
        gcEf.mfCompileParam(params, ef, &m_Params);
        break;
    }
  }

  unguard;

  return 1;
}

void CPShader_D3D::mfDeleteParams(TArray<SParam> &Vars)
{
  int i, j;

  for (i=0; i<Vars.Num(); i++)
  {
    SParam *p = &Vars[i];
    for (j=0; j<4; j++)
    {
      if (p->m_Comps[j])
      {
        delete p->m_Comps[j];
        p->m_Comps[j] = NULL;
      }
    }
  }
  Vars.Free();
}

void CPShader_D3D::mfReset()
{
  if (m_dwShader)
  {
    gcpRendD3D->mfGetD3DDevice()->DeletePixelShader(m_dwShader);
    m_dwShader = 0;
  }
  m_bActive = false;
}

CPShader_D3D::~CPShader_D3D()
{
  mfDeleteParams(m_Params);
  SAFE_DELETE (m_pScript);
  mfReset();
}

void CPShader_D3D::mfSetFloat4f(int reg, float *v)
{
  HRESULT hr = gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstant(reg, v, 1);
}

#define GL_OFFSET_TEXTURE_2D_MATRIX_NV      0x86E1

void CPShader_D3D::mfSetParams(TArray<SParam>& Vars, TArray<SParam>* AddParams)
{
  int i;
  HRESULT hr = 0;

  for (i=0; i<Vars.Num(); i++)
  {
    SParam *p = &Vars[i];
    float *v = p->mfGet();
    if (p->m_Reg != GL_OFFSET_TEXTURE_2D_MATRIX_NV)
      hr = gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstant(p->m_Reg, v, 1);
    else
    {
      //v[0] = v[3] = 0;
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVMAT00, FLOATtoDWORD(v[0]));
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVMAT01, FLOATtoDWORD(v[1]));
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVMAT10, FLOATtoDWORD(v[2]));
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVMAT11, FLOATtoDWORD(v[3]));
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVLSCALE, FLOATtoDWORD(4.0f));
      gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVLOFFSET, FLOATtoDWORD(0.0f));
    }
    if (FAILED(hr))
      return;
  }
  if (AddParams)
  {
    for (i=0; i<AddParams->Num(); i++)
    {
      SParam *p = &AddParams->Get(i);
      float *v = p->mfGet();
      if (p->m_Reg != GL_OFFSET_TEXTURE_2D_MATRIX_NV)
        hr = gcpRendD3D->mfGetD3DDevice()->SetPixelShaderConstant(p->m_Reg, v, 1);
      else
      {
        //v[0] = v[3] = 0;
        v[0] *= CRenderer::CV_r_embm;
        v[3] *= CRenderer::CV_r_embm;
        gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVMAT00, FLOATtoDWORD(v[0]));
        gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVMAT01, FLOATtoDWORD(v[1]));
        gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVMAT10, FLOATtoDWORD(v[2]));
        gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVMAT11, FLOATtoDWORD(v[3]));
        gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVLSCALE, FLOATtoDWORD(4.0f));
        gcpRendD3D->mfGetD3DDevice()->SetTextureStageState(1, D3DTSS_BUMPENVLOFFSET, FLOATtoDWORD(0.0f));
      }
      if (FAILED(hr))
        return;
    }
  }
}

bool CPShader_D3D::mfBind()
{
  HRESULT hr;
  if (!m_bActive)
    mfActivate();
  hr = gcpRendD3D->mfGetD3DDevice()->SetPixelShader( m_dwShader );
  if (FAILED(hr))
    return false;
  return true;
}

bool CPShader_D3D::mfUnbind()
{
  HRESULT hr;
  hr = gcpRendD3D->mfGetD3DDevice()->SetPixelShader( NULL );
  if (FAILED(hr))
    return false;
  return true;
}

#ifdef _XBOX
char * PSToXboxPS(const char * pInp)
{
  size_t len = strlen(pInp);
  char * p, *tmp;
  char * pRet = p = new char[len+256];

  const char * pFind = "_sat";
  int lenFind = 4;

  int nChar = 0;
  int state = 0;

  char cReg[16];
  int iReg=0;

  int AfterCom = 0;
  bool bIsAlphaString = false;
  int ChanelReg = 0;

  while(*p++ = *pInp++)
  {
    if(*(pInp) != 0 && *(pInp) == 'p' && *(pInp+1) == 's')
      *p++ = 'x';
  }

  for(p = pRet; *p; p++)
  {
    if(*p == '\n')
    {
      AfterCom = 0;
      bIsAlphaString = false;
    }
    if(!AfterCom && *p == ',')
      AfterCom = 1;

    if(!AfterCom && *p == '.' && (*(p+1) == 'a') || (*(p+1) == 'A'))
      bIsAlphaString = true;

    if(state != 3 && state != 4)
    {
      if(state==0 && !AfterCom && pFind[nChar] == *p)
      {
        nChar++;
        if(nChar >= lenFind)
        {
          for(int i = 1; *(p+i-1); i++)
            *(p-lenFind+i) = *(p+i);
          p -= lenFind;
          nChar = 0;
          state = 1;
        }
      }
      else
      {
        nChar = 0;
        if(state == 1)
        {
          if (*p != ' ' && *p != '\t')
            state = 2;
        }

        if(state == 2)
        {
          if (*p != ' ' && *p != '\t' && *p != ',')
            cReg[iReg++] = *p;
          else
          {
            cReg[iReg] = 0;
            char * ch = strchr(cReg, '.');
            ChanelReg = 0;
            if(ch)
            {
              if( *(ch+1) == 'a' || *(ch+1) == 'A')
                ChanelReg = 2;
              *ch = 0;
              iReg = int(ch - cReg);

            }
            state = 3;
            tmp = p;
            AfterCom = 2;
          }
        }
      }
    }
    else // if(state == 3 || state == 4)
    {
      if(AfterCom == 1 && cReg[nChar] == *p)
      {
        nChar++;
        if(nChar >= iReg)
        {
          bool bSet = false;
          if(!bIsAlphaString)
          {
            if(ChanelReg ==2)
            {
            }
            else
            if(*(p+1)=='.' && *(p+2)=='a')
            {
            }
            else
              bSet=true;
          }
          else // if(bIsAlphaString)
          {
            if(ChanelReg == 0)
            {
              if(*(p+1)=='.' && *(p+2)!='a')
              {
                bSet = true;
              }
            }
            if(ChanelReg ==2)
            {
              bSet = true;
              if(*(p+1)=='.' && *(p+2)!='a')
              {
                bSet = false;
              }
            }
          }

          if(bSet)
          {
            char * vp = strchr(p,0);
            while (vp != p)
              *(vp + lenFind) = *vp--;
            p++;
            for(int i=0; i<=lenFind-1; i++)
              *p++ = pFind[i];
            nChar = 0;
            //iReg = 0;
            state = 4;
            //p = tmp-1;
          }
        }
      }
      else
        nChar = 0;
      if(*(p+1)==0 || (state==4 &&  (*(p+1)=='\n') ))
      {
        nChar = 0;
        iReg = 0;
        state = 0;
        p = tmp-1;
      }
    }
  }
  
  strcpy(p-1, "\nxfc fog.a,r0,fog.rgb,zero,zero,zero,r0.a\n");  

  return pRet;
}
#endif //_XBOX



bool CPShader_D3D::mfActivate()
{
  HRESULT hr;

  if (m_bActive)
    return true;

#ifdef _XBOX  
  LPXGBUFFER pCode;
  LPXGBUFFER pBuffer = NULL;

  if (!m_pScript)
    return false;
  m_bActive = 1;

#ifndef _XBOX
  hr = XGAssembleShader(m_Name.c_str(), m_pScript, strlen(m_pScript), 0, NULL, &pCode, &pBuffer, NULL, NULL, NULL, 0);
#else //_XBOX
  char * pScriptTmp = PSToXboxPS(m_pScript);
  hr = XGAssembleShader(m_Name.c_str(), pScriptTmp, strlen(pScriptTmp), 0, NULL, &pCode, &pBuffer, NULL, NULL, NULL, 0);
  delete [] pScriptTmp;
#endif ///_XBOX


  if (FAILED(hr))
  {
    iLog->Log("CPShader_D3D::mfActivate: Couldn't assemble pixel shader '%s' (%s)\n", m_Name.c_str(), gcpRendD3D->D3DError(hr));
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
      iLog->Log("ERROR: CPShader_D3D::mfActivate: Shader '%s' script error (%s)\n", strOut, m_Name.c_str());
      SAFE_RELEASE( pBuffer );
    }
    return false;
  }
  
  // Create the pixel shader
  if( FAILED( hr = gcpRendD3D->mfGetD3DDevice()->CreatePixelShader((D3DPIXELSHADERDEF *)pCode->GetBufferPointer(), &m_dwShader)))
  {
    iLog->Log("ERROR: CPShader_D3D::mfActivate: Couldn't create pixel shader '%s'\n", m_Name.c_str());
    return false;
  }
#else
  LPD3DXBUFFER pCode;
  LPD3DXBUFFER pBuffer = NULL;
  
  if (!m_pScript)
    return false;
  m_bActive = 1;

 // Assemble script (.nvp) and create shader
  hr = D3DXAssembleShader( m_pScript, strlen(m_pScript), 0, NULL, &pCode, &pBuffer );
  if (FAILED(hr))
  {
    iLog->Log("ERROR: CPShader_D3D::mfActivate: Couldn't assemble pixel shader '%s'\n", m_Name.c_str());
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
      iLog->Log("ERROR: CPShader_D3D::mfActivate: Pixel shader '%s' script error (%s)\n", m_Name.c_str(), strOut);
      SAFE_RELEASE( pBuffer );
    }
    return false;
  }
  // Create the pixel shader
  hr = gcpRendD3D->mfGetD3DDevice()->CreatePixelShader( (DWORD*)pCode->GetBufferPointer(), &m_dwShader );
  if (FAILED(hr))
  {
    iLog->Log("ERROR: CPShader_D3D::mfActivate: Couldn't create pixel shader '%s'\n", m_Name.c_str());
    return false;
  }
#endif

  return true;
}

bool CPShader_D3D::mfSet(bool bStat, SShaderPassHW *slw)
{
  if (!m_bActive)
  {
    if (!mfActivate())
      return false;
  }

  if (!bStat)
  {
    if (CRenderer::CV_r_log >= 3)
      gRenDev->Logv(SRendItem::m_RecurseLevel, "--- Reset PShader \"%s\"\n", m_Name.c_str());
    mfUnbind();
  }
  else
  {
    if (CRenderer::CV_r_log >= 3)
      gRenDev->Logv(SRendItem::m_RecurseLevel, "--- Set PShader \"%s\"\n", m_Name.c_str());

    mfBind();
    if (slw)
      mfSetParams(m_Params, slw->m_RCParamsNoObj);
    else
      mfSetParams(m_Params, NULL);
  }
  return true;
}

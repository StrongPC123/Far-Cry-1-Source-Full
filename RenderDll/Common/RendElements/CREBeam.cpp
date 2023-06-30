#include "RenderPCH.h"
#include "RendElement.h"
#include "I3dengine.h"

void CREBeam::mfPrepare(void)
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  int Features = gRenDev->GetFeatures();

  if (!m_pBuffer)
  {
    I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
    IStatObj *pObj = eng->MakeObject(m_ModelName.c_str(), NULL, evs_ShareAndSortForCache, false, false);	
    m_pBuffer = pObj->GetLeafBuffer();
    if (!m_pBuffer)
    {
      gRenDev->m_RP.m_pRE = NULL;
      gRenDev->m_RP.m_RendNumIndices = 0;
      gRenDev->m_RP.m_RendNumVerts = 0;
      return;
    }
    Vec3d Mins = m_pBuffer->m_vBoxMin;
    Vec3d Maxs = m_pBuffer->m_vBoxMax;

    m_fLengthScale = Maxs.x;
    m_fWidthScale  = Maxs.z;
  }

  CLeafBuffer *lb = m_pBuffer;
  CMatInfo *mi = &(*lb->m_pMats)[0];

  gRenDev->m_RP.m_pShader = (SShader *)mi->shaderItem.m_pShader->GetTemplate(-1);
  gRenDev->m_RP.m_pShaderResources = mi->shaderItem.m_pShaderResources;
  gRenDev->m_RP.m_pRE = mi->pRE;

  // Choose appropriate shader technique depend on some input parameters
  if (gRenDev->m_RP.m_pShader->m_HWTechniques.Num())
  {
    int nHW = gRenDev->EF_SelectHWTechnique(gRenDev->m_RP.m_pShader);
    if (nHW >= 0)
      gRenDev->m_RP.m_pCurTechnique = gRenDev->m_RP.m_pShader->m_HWTechniques[nHW];
    else
      gRenDev->m_RP.m_pCurTechnique = NULL;
  }
  else
    gRenDev->m_RP.m_pCurTechnique = NULL;

  CCObject *obj = gRenDev->m_RP.m_pCurObject;
  obj->m_RE = this;
  UParamVal pv;
  pv.m_Float = m_fLengthScale;
  SShaderParam::SetParam("origlength", &gRenDev->m_RP.m_pShader->m_PublicParams, pv, -1);

  pv.m_Float = m_fWidthScale;
  SShaderParam::SetParam("origwidth", &gRenDev->m_RP.m_pShader->m_PublicParams, pv, -1);

  pv.m_Float = m_fLength;
  SShaderParam::SetParam("length", &gRenDev->m_RP.m_pShader->m_PublicParams, pv, -1);

  pv.m_Float = m_fStartRadius;
  SShaderParam::SetParam("startradius", &gRenDev->m_RP.m_pShader->m_PublicParams, pv, -1);

  pv.m_Float = m_fEndRadius;
  SShaderParam::SetParam("endradius", &gRenDev->m_RP.m_pShader->m_PublicParams, pv, -1);

  pv.m_Color[0] = m_StartColor[0];
  pv.m_Color[1] = m_StartColor[1];
  pv.m_Color[2] = m_StartColor[2];
  pv.m_Color[3] = m_StartColor[3];
  if (m_LightStyle == 0)
  {
    pv.m_Color[0] = obj->m_Color[0];
    pv.m_Color[1] = obj->m_Color[1];
    pv.m_Color[2] = obj->m_Color[2];
  }
  SShaderParam::SetParam("startcolor", &gRenDev->m_RP.m_pShader->m_PublicParams, pv, -1);

  pv.m_Color[0] = m_EndColor[0];
  pv.m_Color[1] = m_EndColor[1];
  pv.m_Color[2] = m_EndColor[2];
  pv.m_Color[3] = m_EndColor[3];
  if (m_LightStyle == 0)
  {
    pv.m_Color[0] = obj->m_Color[0];
    pv.m_Color[1] = obj->m_Color[1];
    pv.m_Color[2] = obj->m_Color[2];
  }
  SShaderParam::SetParam("endcolor", &gRenDev->m_RP.m_pShader->m_PublicParams, pv, -1);

  obj->m_ShaderParams = &gRenDev->m_RP.m_pShader->m_PublicParams;

  gRenDev->m_RP.m_FirstVertex = mi->nFirstVertId;
  gRenDev->m_RP.m_RendNumIndices = mi->nNumIndices;
  gRenDev->m_RP.m_RendNumVerts = mi->nNumVerts;
  gRenDev->m_RP.m_FirstIndex = mi->nFirstIndexId;
}

bool CREBeam::mfCompile(SShader *ef, char *scr)
{
  char* name;
  long cmd;
  char *params;
  char *data;
  
  enum {eModel = 1, eLength, eStartRadius, eEndRadius, eStartColor, eEndColor, eLightStyle};
  static tokenDesc commands[] =
  {
    {eModel, "Model"},
    {eStartRadius, "StartRadius"},
    {eEndRadius, "EndRadius"},
    {eStartColor, "StartColor"},
    {eEndColor, "EndColor"},
    {eLightStyle, "LightStyle"},
    {eLength, "Length"},
    {0, 0},
  };

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

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
      case eModel: 
        {
          m_ModelName = data;
          //IStatObj *pObj = eng->MakeObject(data, NULL, 0, false, false);	
          //m_pBuffer = pObj->GetLeafBuffer();
        }
        break;

      case eStartRadius:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing StartRadius argument for Beam Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        m_fStartRadius = shGetFloat(data);
        break;

      case eEndRadius:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing EndRadius argument for Beam Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        m_fEndRadius = shGetFloat(data);
        break;

      case eLightStyle:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing LightStyle argument for Beam Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        m_LightStyle = shGetInt(data);
        break;

      case eLength:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing Length argument for Beam Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        m_fLength = shGetFloat(data);
        break;

      case eStartColor:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing StartColor argument for Beam Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        shGetColor(data, m_StartColor);
        break;

      case eEndColor:
        if (!data || !data[0])
        {
          Warning( 0,0,"missing EndColor argument for Beam Effect in Shader '%s'\n", ef->m_Name.c_str());
          break;
        }
        shGetColor(data, m_EndColor);
        break;
    }
  }

  if (!m_ModelName.c_str()[0])
    return false;
  SShaderParam *sp;

  sp = new SShaderParam;
  strcpy(sp->m_Name, "origlength");
  sp->m_Type = eType_FLOAT;
  sp->m_Value.m_Float = 10.0f;
  m_ShaderParams.AddElem(sp);

  sp = new SShaderParam;
  strcpy(sp->m_Name, "origwidth");
  sp->m_Type = eType_FLOAT;
  sp->m_Value.m_Float = 1.0f;
  m_ShaderParams.AddElem(sp);

  sp = new SShaderParam;
  strcpy(sp->m_Name, "startradius");
  sp->m_Type = eType_FLOAT;
  sp->m_Value.m_Float = 0.1f;
  m_ShaderParams.AddElem(sp);

  sp = new SShaderParam;
  strcpy(sp->m_Name, "endradius");
  sp->m_Type = eType_FLOAT;
  sp->m_Value.m_Float = 1.0f;
  m_ShaderParams.AddElem(sp);

  sp = new SShaderParam;
  strcpy(sp->m_Name, "startcolor");
  sp->m_Type = eType_FCOLOR;
  sp->m_Value.m_Color[0] = 1.0f;
  sp->m_Value.m_Color[1] = 1.0f;
  sp->m_Value.m_Color[2] = 1.0f;
  sp->m_Value.m_Color[3] = 1.0f;
  m_ShaderParams.AddElem(sp);

  sp = new SShaderParam;
  strcpy(sp->m_Name, "endcolor");
  sp->m_Type = eType_FCOLOR;
  sp->m_Value.m_Color[0] = 1.0f;
  sp->m_Value.m_Color[1] = 1.0f;
  sp->m_Value.m_Color[2] = 1.0f;
  sp->m_Value.m_Color[3] = 0.1f;
  m_ShaderParams.AddElem(sp);

  return true;
}

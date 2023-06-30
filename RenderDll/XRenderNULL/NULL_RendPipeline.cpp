/*=============================================================================
  PS2_RendPipeline.cpp : PS2 specific rendering using shaders pipeline.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

    Revision history:
  		* Created by Honich Andrey
    
=============================================================================*/

#include "RenderPCH.h"
#include "NULL_Renderer.h"
#include "I3dengine.h"

#include "Platform.h"

//============================================================================================
// Shaders rendering
//============================================================================================

//============================================================================================
// Init Shaders rendering

void CNULLRenderer::EF_InitRandTables()
{
}

void CNULLRenderer::EF_InitWaveTables()
{  
}

void CNULLRenderer::EF_InitEvalFuncs(int num)
{
}

int CNULLRenderer::EF_RegisterFogVolume(float fMaxFogDist, float fFogLayerZ, CFColor color, int nIndex, bool bCaustics)
{
  if (nIndex < 0)
  {
    SMFog Fog;
    memset(&Fog,0,sizeof(Fog));

    Fog.m_fMaxDist = fMaxFogDist;
    Fog.m_FogInfo.m_FogColor = color;
    Fog.m_Dist = fFogLayerZ;
    Fog.m_Color = color;
    Fog.m_Color.a = 1.0f;
    Fog.bCaustics = bCaustics;
    //Fog.m_FogInfo.m_FogColor = m_FogColor;
    Fog.m_Normal = Vec3d(0,0,1);

    m_RP.m_FogVolumes.AddElem(Fog);
    return m_RP.m_FogVolumes.Num()-1;
  }
  else
  {
    assert (nIndex < m_RP.m_FogVolumes.Num());
    SMFog *pFog = &m_RP.m_FogVolumes[nIndex];
    pFog->m_fMaxDist = fMaxFogDist;
    pFog->m_FogInfo.m_FogColor = color;
    pFog->m_Dist = fFogLayerZ;
    pFog->m_Color = color;
    pFog->m_Color.a = 1.0f;
    pFog->bCaustics = bCaustics;
    return nIndex;
  }
}

void CNULLRenderer::EF_PipelineInit()
{
  bool nv = 0;

  m_RP.m_MaxVerts = 600;
  m_RP.m_MaxTris = 300;

  EF_InitWaveTables();
  EF_InitRandTables();
  EF_InitEvalFuncs(0);
  EF_InitFogVolumes();

//==================================================

  SAFE_DELETE_ARRAY(m_RP.m_VisObjects);

  CCObject::m_Waves.Create(32);
  m_RP.m_VisObjects = new CCObject *[MAX_REND_OBJECTS];

  if (!m_RP.m_TempObjects.Num())
    m_RP.m_TempObjects.Reserve(MAX_REND_OBJECTS);
  if (!m_RP.m_Objects.Num())
  {
    m_RP.m_Objects.Reserve(MAX_REND_OBJECTS);
    m_RP.m_Objects.SetUse(1);
    SAFE_DELETE_ARRAY(m_RP.m_ObjectsPool);
    m_RP.m_nNumObjectsInPool = 384;
    m_RP.m_ObjectsPool = new CCObject[m_RP.m_nNumObjectsInPool];
    for (int i=0; i<m_RP.m_nNumObjectsInPool; i++)
    {
      m_RP.m_TempObjects[i] = &m_RP.m_ObjectsPool[i];
      m_RP.m_TempObjects[i]->Init();
      m_RP.m_TempObjects[i]->m_Color = Col_White;
      m_RP.m_TempObjects[i]->m_ObjFlags = 0;
      m_RP.m_TempObjects[i]->m_Matrix.SetIdentity();
      m_RP.m_TempObjects[i]->m_RenderState = 0;
    }
    m_RP.m_VisObjects[0] = &m_RP.m_ObjectsPool[0];
  }

  //m_RP.m_DLights.Create(64);
  //m_RP.m_DLights.SetUse(0);

  m_RP.m_pREGlare = (CREGlare *)EF_CreateRE(eDATA_Glare);

  for (int i=0; i<VERTEX_FORMAT_NUMS; i++)
  {
    for (int j=0; j<VERTEX_FORMAT_NUMS; j++)
    {
      SVertBufComps Cps[2];
      GetVertBufComps(&Cps[0], i);
      GetVertBufComps(&Cps[1], j);

      bool bNeedTC = Cps[1].m_bHasTC | Cps[0].m_bHasTC;
      bool bNeedCol = Cps[1].m_bHasColors | Cps[0].m_bHasColors;
      bool bNeedSecCol = Cps[1].m_bHasSecColors | Cps[0].m_bHasSecColors;
      bool bNeedNormals = Cps[1].m_bHasNormals | Cps[0].m_bHasNormals;
      m_RP.m_VFormatsMerge[i][j] = VertFormatForComponents(bNeedCol, bNeedSecCol, bNeedNormals, bNeedTC);
    }
  }
}

void CNULLRenderer::EF_ClearBuffers(bool bForce, float *Colors)
{
}

void CNULLRenderer::EF_SetClipPlane (bool bEnable, float *pPlane, bool bRefract)
{
}

void CNULLRenderer::EF_PipelineShutdown()
{
  int i, j;

  CCObject::m_Waves.Free();
  SAFE_DELETE_ARRAY(m_RP.m_VisObjects);

  //m_RP.m_DLights.Free();
  m_RP.m_FogVolumes.Free();
  SAFE_RELEASE(m_RP.m_pREGlare);
  
  for (i=0; i<CREClientPoly2D::mPolysStorage.GetSize(); i++)
  {
    SAFE_RELEASE(CREClientPoly2D::mPolysStorage[i]);
  }
  CREClientPoly2D::mPolysStorage.Free();

  for (j=0; j<4; j++)
  {
    for (i=0; i<CREClientPoly::mPolysStorage[j].GetSize(); i++)
    {
      SAFE_RELEASE(CREClientPoly::mPolysStorage[j][i]);
    }
    CREClientPoly::mPolysStorage[j].Free();
  }
}

void CNULLRenderer::EF_Release(int nFlags)
{
}

//==========================================================================

void CNULLRenderer::EF_SetCameraInfo()
{
}

void CNULLRenderer::EF_CalcObjectMatrix(CCObject *obj)
{
}

void CNULLRenderer::EF_SetObjectTransform(CCObject *obj)
{
}

void CNULLRenderer::EF_PreRender(int Stage)
{
}

//=======================================================================


void CNULLRenderer::EF_Eval_DeformVerts(TArray<SDeform>* Defs)
{
}

void CNULLRenderer::EF_Eval_TexGen(SShaderPass *sfm)
{
}

void CNULLRenderer::EF_Eval_RGBAGen(SShaderPass *sfm)
{
}

void CNULLRenderer::EF_EvalNormalsRB(SShader *ef)
{
}

//=================================================================================

void CNULLRenderer::PS2SetCull(ECull eCull)
{ 
  m_RP.m_eCull = eCull;
}


void CRenderer::EF_SetState(int st)
{
  m_CurState = st;
}

void CNULLRenderer::EF_SetColorOp(byte co)
{
}

//=================================================================================

DEFINE_ALIGNED_DATA_STATIC( Matrix44, sIdentityMatrix( 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 ), 16 ); 

// Get inverted matrix of the object matrix
// All matrices are 16 bytes alligned to speedup matrix calculations using SSE instructions
Matrix44 &CCObject::GetInvMatrix()
{
  return sIdentityMatrix;
}


bool CNULLRenderer::EF_ObjectChange(SShader *Shader, int nObject, CRendElement *pRE)
{
  return true;
}


// Initialize of the new shader pipeline (only 2d)
void CNULLRenderer::EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, CRendElement *re) 
{
  m_RP.m_Frame++;
}

// Initialize of the new shader pipeline
void CNULLRenderer::EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, int numFog, CRendElement *re)
{
  m_RP.m_Frame++;
}

void CNULLRenderer::EF_CheckOverflow(int nVerts, int nInds, CRendElement *re)
{
}

//========================================================================================

void CNULLRenderer::EF_LightMaterial(SLightMaterial *lm, int Flags)
{
}

//===================================================================================================

// Used for HW effectors for rendering of tri mesh (vertex array)
void CNULLRenderer::EF_DrawIndexedMesh (int nPrimType)
{
}


void CNULLRenderer::EF_FlushShader()
{
}

void CNULLRenderer::EF_Flush()
{
}

void CNULLRenderer::EF_EndEf3D(int nFlags)
{
  m_RP.m_RealTime = iTimer->GetCurrTime();
  EF_RemovePolysFromScene();
  SRendItem::m_RecurseLevel--;
}

void CNULLRenderer::EF_RenderPipeLine(void (*RenderFunc)())
{
}

void CNULLRenderer::EF_PipeLine(int nums, int nume, int nList, int nSortType, void (*RenderFunc)())
{
}

void CNULLRenderer::EF_DrawWire()
{
}

void CNULLRenderer::EF_DrawNormals()
{
}

void CNULLRenderer::EF_DrawTangents()
{
}

void CNULLRenderer::EF_DrawDebugLights()
{
}

void CNULLRenderer::EF_DrawDebugTools()
{
}


void CNULLRenderer::EF_PrintProfileInfo()
{
}

int CNULLRenderer::EF_Preprocess(SRendItemPre *ri, int nums, int nume)
{
  return 0;
}

//double timeFtoI, timeFtoL, timeQRound;
//int sSome;
void CNULLRenderer::EF_EndEf2D(bool bSort)
{
}

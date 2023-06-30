/*=============================================================================
  GLRendPipeline.cpp : OpenGL specific rendering using shaders pipeline.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

    Revision history:
      * Created by Honich Andrey
    
=============================================================================*/

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include <ICryAnimation.h>
#include "..\common\shadow_renderer.h"
#include "GLCGPShader.h"
#include "GLCGVProgram.h"
#include "I3dengine.h"
#include "CryHeaders.h"

//==================================================================

//============================================================================================
// Shaders rendering
//============================================================================================

//============================================================================================
// Init Shaders rendering

void CGLRenderer::EF_InitRandTables()
{
  int i;
  float f;

  for (i=0; i<256; i++)
  {
    f = (float)rand() / 32767.0f;
    m_RP.m_tRandFloats[i] = f + f - 1.0f;

    m_RP.m_tRandBytes[i] = (byte)((float)rand() / 32767.0f * 255.0f);
  }
}

void CGLRenderer::EF_InitWaveTables()
{  
  int i;
  
  //Init wave Tables
  for (i=0; i<1024; i++)
  {
    float f = (float)i;
    
    m_RP.m_tSinTable[i] = cry_sinf(f * (360.0f/1023.0f) * M_PI / 180.0f);
    m_RP.m_tHalfSinTable[i] = cry_sinf(f * (360.0f/1023.0f) * M_PI / 180.0f);
    if (m_RP.m_tHalfSinTable[i] < 0)
      m_RP.m_tHalfSinTable[i] = 0;
    m_RP.m_tCosTable[i] = cry_cosf(f * (360.0f/1023.0f) * M_PI / 180.0f);
    m_RP.m_tHillTable[i] = cry_sinf(f * (180.0f/1023.0f) * M_PI / 180.0f);
    
    if (i < 512)
      m_RP.m_tSquareTable[i] = 1.0f;
    else
      m_RP.m_tSquareTable[i] = -1.0f;
    
    m_RP.m_tSawtoothTable[i] = f / 1024.0f;
    m_RP.m_tInvSawtoothTable[i] = 1.0f - m_RP.m_tSawtoothTable[i];
    
    if (i < 512)
    {
      if (i < 256)
        m_RP.m_tTriTable[i] = f / 256.0f;
      else
        m_RP.m_tTriTable[i] = 1.0f - m_RP.m_tTriTable[i-256];
    }
    else
      m_RP.m_tTriTable[i] = 1.0f - m_RP.m_tTriTable[i-512];
  }
}

void CGLRenderer::EF_InitEvalFuncs(int num)
{
  switch(num)
  {
    case 0:
      m_RP.m_pCurFuncs = &m_RP.m_EvalFuncs_C;
      break;
    default:
    case 1:
      m_RP.m_pCurFuncs = &m_RP.m_EvalFuncs_RE;
      break;
  }
}

// Register new fog volume
int CGLRenderer::EF_RegisterFogVolume(float fMaxFogDist, float fFogLayerZ, CFColor color, int nIndex, bool bCaustics)
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

_inline static void *sAlign0x20(byte *vrts)
{
  INT_PTR b = (INT_PTR)vrts;              //AMD Port

  if (!(b & 0x1f))
    return vrts;
                            // Was b = (b+0x20)&0xffffffe0;
  b = (b+0x20) & (~0x1f);               //AMD Port

  return (void *)b;
}

// Allocate render buffers in video/agp memory
bool CGLRenderer::EF_AllocateBuffersVid()
{
  iLog->Log("\nAllocate vertex buffers in Video/AGP memory (%d verts, %d tris)...\n", m_RP.m_MaxVerts, m_RP.m_MaxTris);
  
  int i;
  int n = 0;
  int nv = 0;
  byte *buf = NULL;
  
  for (i=0; i<m_RP.m_NumFences; i++)
  {
    nv += sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F) * m_RP.m_MaxVerts + 32;
  }
  
  n += sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F) * m_RP.m_MaxVerts + 32;
  
  //m_RP.mFogVertValues
  n += sizeof(float)*m_RP.m_MaxVerts+32;
  //m_RP.mBaseTexCoordPointer
  n += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  //m_RP.mLMTexCoordPointer
  n += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  //m_RP.mClientColors
  n += sizeof(bvec4)*m_RP.m_MaxVerts+32;
  //m_RP.mRendIndices;
  n += sizeof(ushort)*3*m_RP.m_MaxTris+32;
  if (GetFeatures() & RFT_BUMP)
  {
    //m_RP.mBinormals
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mTangents
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mTNormals
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    for (i=0; i<4; i++)
    {
      //m_RP.mHalfAngleVectors
      n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
      //m_RP.mLightVectors
      n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    }
    //m_RP.mAttenuation
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mLAttenSpec0
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mLAttenSpec1
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
  }
  m_SizeBigArray = nv;
  int singleSize = nv / m_RP.m_NumFences;
  while (m_RP.m_NumFences > 2)
  {
    float kbytes = ((float)nv/1024.0f);
    iLog->Log("  Allocating %.2f kbytes of AGP/Video memory for (%d buffers)...", kbytes, m_RP.m_NumFences);
    m_BigArray = (float *)AllocateVarShunk(nv, "Shaders pipeline");
    if(!m_BigArray)
    {
      iLog->Log("Failed\n");
      m_SizeBigArray -= 2*singleSize;
      nv = m_SizeBigArray;
      m_RP.m_NumFences -= 2;
    }
    else
      break;
  }
  if(!m_BigArray)
  {
    iLog->Log("Failed\n");
    m_RP.m_NumFences = 0;
    return false;
  }
  iLog->LogPlus("Ok\n");
  
  buf = (byte *)m_BigArray;
  memset(buf, 0, nv);
  
  int size = sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F) * m_RP.m_MaxVerts+32;
  for (i=0; i<m_RP.m_NumFences; i++)
  {
    m_RP.m_VidBufs[i].Ptr.Ptr = sAlign0x20(buf);
    buf += size;
    m_RP.m_VidBufs[i].m_nCount = size;
    
    glGenFencesNV(1, &m_RP.m_VidBufs[i].m_Fence);
  }

  SAFE_DELETE_ARRAY(m_SysArray);
  
  m_SysArray = new byte [n];
  m_SizeSysArray = n;
  buf = m_SysArray;
  if (!buf)
    iConsole->Exit("Can't allocate system vertex buffers");
  
  memset(buf, 0, n);
  
  m_RP.m_Ptr.Ptr = sAlign0x20(buf);
  buf += sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F) * m_RP.m_MaxVerts + 32;

  m_RP.m_pFogVertValues = (float *)sAlign0x20(buf);
  buf += sizeof(float)*m_RP.m_MaxVerts+32;
  m_RP.m_pBaseTexCoordPointer = (SMRendTexVert *)sAlign0x20(buf);
  buf += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  m_RP.m_pLMTexCoordPointer = (SMRendTexVert *)sAlign0x20(buf);
  buf += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  m_RP.m_pClientColors = (bvec4 *)sAlign0x20(buf);
  buf += sizeof(bvec4)*m_RP.m_MaxVerts+32;
  m_RP.m_RendIndices = (ushort *)sAlign0x20(buf);
  m_RP.m_SysRendIndices = m_RP.m_RendIndices;
  buf += sizeof(ushort)*3*m_RP.m_MaxTris+32;
  if (GetFeatures() & RFT_BUMP)
  {
    m_RP.m_pBinormals = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pTangents = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pTNormals = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    for (i=0; i<4; i++)
    {
      m_RP.m_pLightVectors[i] = (Vec3d *)sAlign0x20(buf);
      buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
      m_RP.m_pHalfAngleVectors[i] = (Vec3d *)sAlign0x20(buf);
      buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    }
    m_RP.m_pAttenuation = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pLAttenSpec0 = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pLAttenSpec1 = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
  }

  return true;
}

// Allocate render buffers/tables
void CGLRenderer::EF_AllocateBuffers()
{
  iLog->Log("\nAllocate vertex buffers in system memory (%d verts, %d tris)...\n", m_RP.m_MaxVerts, m_RP.m_MaxTris);
  
  int n = 0;
  
  n += sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F) * m_RP.m_MaxVerts + 32;
  
  //m_RP.mFogVertValues
  n += sizeof(float)*m_RP.m_MaxVerts+32;
  //m_RP.mBaseTexCoordPointer
  n += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  //m_RP.mLMTexCoordPointer
  n += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  //m_RP.mClientColors
  n += sizeof(bvec4)*m_RP.m_MaxVerts+32;
  
  //m_RP.mRendIndices;
  n += sizeof(ushort)*3*m_RP.m_MaxTris+32;
  
  int i;
  if (GetFeatures() & RFT_BUMP)
  {
    //m_RP.mBinormals
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mTangents
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mTNormals
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    for (i=0; i<4; i++)
    {
      //m_RP.mHalfAngleVectors
      n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
      //m_RP.mLightVectors
      n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    }
    //m_RP.mAttenuation
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mLAttenSpec0
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    //m_RP.mLAttenSpec1
    n += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
  }

  SAFE_DELETE_ARRAY(m_SysArray);

  byte *buf = new byte [n];
  m_SysArray = buf;
  if (!buf)
    iConsole->Exit("Can't allocate vertex buffers in system memory");
  
  memset(buf, 0, n);
  
  m_RP.m_Ptr.Ptr = sAlign0x20(buf);
  buf += sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F) * m_RP.m_MaxVerts + 32;
  m_RP.m_pClientColors = (bvec4 *)sAlign0x20(buf);
  buf += sizeof(bvec4)*m_RP.m_MaxVerts+32;
  m_RP.m_pBaseTexCoordPointer = (SMRendTexVert *)sAlign0x20(buf);
  buf += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  m_RP.m_pLMTexCoordPointer = (SMRendTexVert *)sAlign0x20(buf);
  buf += sizeof(SMRendTexVert)*m_RP.m_MaxVerts+32;
  m_RP.m_pFogVertValues = (float *)sAlign0x20(buf);
  buf += sizeof(float)*m_RP.m_MaxVerts+32;
  m_RP.m_RendIndices = (ushort *)sAlign0x20(buf);
  m_RP.m_SysRendIndices = m_RP.m_RendIndices;
  buf += sizeof(ushort)*3*m_RP.m_MaxTris+32;
  if (GetFeatures() & RFT_BUMP)
  {
    m_RP.m_pBinormals = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pTangents = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pTNormals = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    for (i=0; i<4; i++)
    {
      m_RP.m_pLightVectors[i] = (Vec3d *)sAlign0x20(buf);
      buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
      m_RP.m_pHalfAngleVectors[i] = (Vec3d *)sAlign0x20(buf);
      buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    }
    m_RP.m_pAttenuation = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pLAttenSpec0 = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
    m_RP.m_pLAttenSpec1 = (Vec3d *)sAlign0x20(buf);
    buf += sizeof(Vec3d)*m_RP.m_MaxVerts+32;
  }
}

// Initialize of shaders pipeline
void CGLRenderer::EF_PipelineInit()
{
  bool nv = 0;

  m_RP.m_pCurFuncs = NULL;
  m_RP.m_MaxVerts = CV_gl_rb_verts;
  m_RP.m_MaxTris = CV_gl_rb_tris;
  m_RP.m_CurFence = 0;
  m_RP.m_FenceCount = 0;

  if (m_RP.m_NumFences)
    nv = EF_AllocateBuffersVid();
  if (!nv)
    EF_AllocateBuffers();

  if (SUPPORTS_GL_ARB_vertex_buffer_object)
  {
    m_RP.m_IBDynSize = m_RP.m_MaxTris*3*sizeof(ushort);

    int nVerts = m_RP.m_MaxVerts;
    for (int i=0; i<MAX_DYNVBS; i++)
    {
      if (m_RP.m_VidBufs[i].m_pVBDyn)
        ReleaseBuffer(m_RP.m_VidBufs[i].m_pVBDyn);

      CVertexBuffer *pBuf = new CVertexBuffer;
      pBuf->m_bDynamic = true;
      int size = m_VertexSize[VERTEX_FORMAT_P3F_N_COL4UB_TEX2F]*nVerts*4;
      glGenBuffersARB(1, &pBuf->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, pBuf->m_VS[VSF_GENERAL].m_VertBuf.m_nID);
      glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, NULL, GL_STREAM_DRAW_ARB);
      m_CurVertBufferSize += size;
      pBuf->m_vertexformat = VERTEX_FORMAT_P3F_N_COL4UB_TEX2F;
      pBuf->m_NumVerts = nVerts;
      m_RP.m_VidBufs[i].m_pVBDyn = pBuf;
      m_RP.m_VidBufs[i].m_nCount = size;
    }

    int nInds = m_RP.m_MaxTris*3;
    ReleaseIndexBuffer(&m_RP.m_IBDyn);
    glGenBuffersARB(1, &m_RP.m_IBDyn.m_VertBuf.m_nID);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_RP.m_IBDyn.m_VertBuf.m_nID);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, nInds*sizeof(ushort), NULL, GL_STREAM_DRAW_ARB);
    m_RP.m_IBDyn.m_nItems = nInds;
  }

  glEnableClientState(GL_VERTEX_ARRAY);

  m_Features |= RFT_ZLOCKABLE;

  EF_InitWaveTables();
  EF_InitRandTables();
  EF_InitEvalFuncs(0);
  EF_InitFogVolumes();

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
//==================================================

  SAFE_DELETE_ARRAY(m_RP.m_VisObjects);

  CCObject::m_Waves.Create(32);
  CCObject::m_ObjMatrices.reinit(32);

  m_RP.m_VisObjects = new CCObject *[MAX_REND_OBJECTS];

  if (!m_RP.m_TempObjects.Num())
    m_RP.m_TempObjects.Reserve(MAX_REND_OBJECTS);
  if (!m_RP.m_Objects.Num())
  {
    m_RP.m_Objects.Reserve(MAX_REND_OBJECTS);
    m_RP.m_Objects.SetUse(1);
    SAFE_DELETE_ARRAY(m_RP.m_ObjectsPool);
    m_RP.m_nNumObjectsInPool = 512;
    m_RP.m_ObjectsPool = new CCObject[m_RP.m_nNumObjectsInPool];
    for (int i=0; i<m_RP.m_nNumObjectsInPool; i++)
    {
      m_RP.m_TempObjects[i] = &m_RP.m_ObjectsPool[i];
      m_RP.m_TempObjects[i]->Init();
      m_RP.m_TempObjects[i]->m_Id = i;
      m_RP.m_TempObjects[i]->m_VisId = 0;
      m_RP.m_TempObjects[i]->m_Color = Col_White;
      m_RP.m_TempObjects[i]->m_ObjFlags = 0;
      m_RP.m_TempObjects[i]->m_Matrix.SetIdentity();
      m_RP.m_TempObjects[i]->m_RenderState = 0;
    }
    m_RP.m_VisObjects[0] = &m_RP.m_ObjectsPool[0];
  }

  for (i=0; i<eSHP_MAX; i++)
  {
    if (i == eSHP_DiffuseLight || i == eSHP_SpecularLight)
      m_SHPTable[i] = eSHP_Light;
    else
      m_SHPTable[i] = (EShaderPassType)i;
  }

  //m_RP.m_DLights.Create(64);
  //m_RP.m_DLights.SetUse(0);

  m_RP.m_pREGlare = (CREGlare *)EF_CreateRE(eDATA_Glare);
  m_RP.m_pREHDR = (CREHDRProcess *)EF_CreateRE(eDATA_HDRProcess);

  m_RP.m_CurGlobalColor.dcolor = -1;

  m_ArraysEnum[0] = GL_VERTEX_ARRAY;
  m_ArraysEnum[1] = GL_NORMAL_ARRAY;
  m_ArraysEnum[2] = GL_COLOR_ARRAY;
  m_ArraysEnum[3] = GL_SECONDARY_COLOR_ARRAY_EXT;
  m_ArraysEnum[4] = GL_FOG_COORDINATE_ARRAY_EXT;
}

// Clear buffers (color, depth/stencil)
void CGLRenderer::EF_ClearBuffers(bool bForce, bool bOnlyDepth, float *Colors)
{
  if (m_bWasCleared && !bForce)
    return;

  //m_vClearColor[0] = 0;
  //m_vClearColor[1] = 0;
  //m_vClearColor[2] = 0;
  if (m_bHeatVision)
  {
    CFColor col = Col_Black;
    Colors = &col[0];
  }

  EF_SetState(GS_DEPTHWRITE);

  m_bWasCleared = true;
  if (Colors)
    glClearColor(Colors[0],Colors[1], Colors[2], Colors[3]);
  else
  if(m_polygon_mode==R_WIREFRAME_MODE)
    glClearColor(0.25f,0.5f,1.0f,0); // blue to see something in whireframe mode
  else
    glClearColor(m_vClearColor.x,m_vClearColor.y,m_vClearColor.z,0);

  if (m_sbpp && m_cbpp > 16)
  {
    glClearStencil(0);
    if (bOnlyDepth)
      glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);   
    else
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);   

    if (CV_r_measureoverdraw)
    {
      if (m_sbpp <= 4)
      {
        iLog->Log("Not enough stencil bits to measure overdraw: %d\n");
        CV_r_measureoverdraw = 0;
        m_RP.m_PersFlags &= ~RBPF_MEASUREOVERDRAW;
      }
      else
      {
        m_RP.m_PersFlags &= ~RBPF_MEASUREOVERDRAW;
        EF_SetState(GS_STENCIL | GS_DEPTHWRITE);
        m_RP.m_PersFlags |= RBPF_MEASUREOVERDRAW;
        EF_SetStencilState(STENC_FUNC(FSS_STENCFUNC_ALWAYS) |
                          STENCOP_FAIL(FSS_STENCOP_KEEP) |
                          STENCOP_ZFAIL(FSS_STENCOP_INCR) |
                          STENCOP_PASS(FSS_STENCOP_INCR),
                          0, 0xffffffff);
      }
    }
    else
      m_RP.m_PersFlags &= ~RBPF_MEASUREOVERDRAW;
  }
  else
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);         
  }
}

// Set clip plane for the current scene
// on NVidia GPUs we use fake clip planes using texkill PS instruction : m_RP.m_ClipPlaneEnabled = 1
// on ATI hardware we use native ClipPlanes : m_RP.m_ClipPlaneEnabled = 2
void CGLRenderer::EF_SetClipPlane (bool bEnable, float *pPlane, bool bRefract)
{
  if (!CV_gl_clipplanes)
    return;

  if (bEnable)
  {
    if (m_LogFile)
      Logv(SRendItem::m_RecurseLevel, "Set clip-plane\n");
    if (m_RP.m_ClipPlaneEnabled)
      return;
    m_RP.m_ClipPlaneWasOverrided = 0;
    m_RP.m_bClipPlaneRefract = bRefract;
    m_RP.m_CurClipPlane.m_Normal.x = pPlane[0];
    m_RP.m_CurClipPlane.m_Normal.y = pPlane[1];
    m_RP.m_CurClipPlane.m_Normal.z = pPlane[2];
    m_RP.m_CurClipPlane.m_Dist = pPlane[3];
    m_RP.m_CurClipPlane.Init();

    m_RP.m_CurClipPlaneCull = m_RP.m_CurClipPlane;
    m_RP.m_CurClipPlaneCull.m_Dist = -m_RP.m_CurClipPlaneCull.m_Dist;
    if (m_MaxClipPlanes)
    {
      m_RP.m_ClipPlaneEnabled = 2;
      double p[4];
      p[0] = pPlane[0];
      p[1] = pPlane[1];
      p[2] = pPlane[2];
      p[3] = pPlane[3];
      glPushMatrix();
      glLoadMatrixf(m_CameraMatrix.GetData());
      glClipPlane(GL_CLIP_PLANE0, p);
      glEnable(GL_CLIP_PLANE0);
      glPopMatrix();
    }
    else
    {
      m_RP.m_ClipPlaneEnabled = 1;
      if (m_MaxActiveTexturesARB_VP > 6)
        m_RP.m_nClipPlaneTMU = 3;
      else
        m_RP.m_nClipPlaneTMU = 3;


      // Set the tex-gen clipping plane for fixed pipeline.
      // Special set for terrain detail textures (fixed pipe)
      glLoadMatrixf(m_CameraMatrix.GetData());
      EF_SelectTMU(3);
      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
      glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
      glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
      if (bRefract)
      {
        float plane[4];
        plane[0] = pPlane[0];
        plane[1] = pPlane[1];
        plane[2] = pPlane[2];
        plane[3] = pPlane[3];
        glTexGenfv(GL_S, GL_EYE_PLANE, plane);
        glTexGenfv(GL_T, GL_EYE_PLANE, plane);
        glTexGenfv(GL_R, GL_EYE_PLANE, plane);
        glTexGenfv(GL_Q, GL_EYE_PLANE, plane);
      }
      else
      {
        glTexGenfv(GL_S, GL_EYE_PLANE, pPlane);
        glTexGenfv(GL_T, GL_EYE_PLANE, pPlane);
        glTexGenfv(GL_R, GL_EYE_PLANE, pPlane);
        glTexGenfv(GL_Q, GL_EYE_PLANE, pPlane);
      }
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
      glEnable(GL_TEXTURE_GEN_R);
      glEnable(GL_TEXTURE_GEN_Q);
      if (SUPPORTS_GL_NV_texture_shader)
      {
        for (int i=0; i<m_MaxActiveTexturesARBFixed; i++)
        {
          EF_SelectTMU(i);
          if (i == 3)
          {
            glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_CULL_FRAGMENT_NV);
            float args[4];
            args[0] = args[1] = args[2] = args[3] = GL_GEQUAL;
            glTexEnvfv(GL_TEXTURE_SHADER_NV, GL_CULL_MODES_NV, &args[0]);
          }
          else
            glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target);
        }
      }
      EF_SelectTMU(0);
      if (SUPPORTS_GL_NV_texture_shader)
        glEnable(GL_TEXTURE_SHADER_NV);
    }
  }
  else
  {
    if (m_LogFile)
      Logv(SRendItem::m_RecurseLevel, "Reset clip-plane\n");

    if (!m_RP.m_ClipPlaneEnabled)
      return;

    if (m_RP.m_ClipPlaneEnabled == 2)
    {
      glDisable(GL_CLIP_PLANE0);
    }
    else
    {
      if (SUPPORTS_GL_NV_texture_shader)
        glDisable(GL_TEXTURE_SHADER_NV);
      EF_SelectTMU(3);
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
      glDisable(GL_TEXTURE_GEN_R);
      glDisable(GL_TEXTURE_GEN_Q);
      EF_SelectTMU(0);
    }
    m_RP.m_ClipPlaneEnabled = 0;
    m_RP.m_ClipPlaneWasOverrided = 0;
  }
}

// Shutdown shaders pipeline
void CGLRenderer::EF_PipelineShutdown()
{
  int i, j;

  CCObject::m_Waves.Free();
  SAFE_DELETE_ARRAY(m_RP.m_VisObjects);
  SAFE_DELETE_ARRAY(m_SysArray);  
  if (m_BigArray)
  {
    ReleaseVarShunk(m_BigArray);
    m_BigArray = NULL;
  }

  //m_RP.m_DLights.Free();
  m_RP.m_FogVolumes.Free();
  
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

  uint tID;
  tID = TO_LIGHT_CUBE_MAP;
  glDeleteTextures(1, &tID);

  tID = TO_NORMALIZE_CUBE_MAP;
  glDeleteTextures(1, &tID);

  EF_Release(EFRF_VSHADERS);
  CCGVProgram_GL::mfDeleteSharedScripts();
}

// Release all vertex and pixel shaders
void CGLRenderer::EF_Release(int nFlags)
{
  int i;

  if (nFlags & EFRF_VSHADERS)
  {
    for (i=0; i<CVProgram::m_VPrograms.Num(); i++)
    {
      CVProgram::m_VPrograms[i]->mfReset();
    }
  }
  if (nFlags & EFRF_PSHADERS)
  {
    for (i=0; i<CPShader::m_PShaders.Num(); i++)
    {
      CPShader::m_PShaders[i]->mfReset();
    }
  }
}

//==========================================================================

// Init states before rendering of the scene
void CGLRenderer::EF_PreRender(int Stage)
{
  if (Stage & 1)
  { // Before preprocess
    m_RP.m_RenderFrame++;
    m_RP.m_Flags = 0;
    m_RP.m_pPrevObject = NULL;
    m_RP.m_FrameObject++;
    
    EF_SetCameraInfo();
    if (Stage == 1)
      CCGVProgram_GL::mfSetGlobalParams();

    for (int i=0; i<m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); i++)
    {
      CDLight *dl = m_RP.m_DLights[SRendItem::m_RecurseLevel][i];
      if (dl->m_Flags & DLF_FAKE)
        continue;

      if (dl->m_Flags & DLF_POINT)
      {
      }
      if (dl->m_Flags & DLF_SUN)
      {
        Vec3d pos = GetCamera().GetPos();
        Vec3d delta = dl->m_Origin - pos;
        delta.Normalize();
        m_RP.m_SunDir = delta;
      }
    }
  //  if (i == m_RP.m_DLights.Num() && m_RP.m_SunDir == Vec3d(0,0,0))
    if (i == m_RP.m_DLights[SRendItem::m_RecurseLevel].Num() && IsEquivalent(m_RP.m_SunDir,Vec3d(0,0,0)))
      m_RP.m_SunDir = Vec3d(0,0,1);
  }
  if (Stage & 2)
  {  // After preprocess
    if (!m_RP.m_bStartPipeline && !m_bWasCleared && !(m_RP.m_PersFlags & RBPF_NOCLEARBUF))
    {
      m_RP.m_bStartPipeline = true;
      glDepthRange(0, 1);
      EF_ClearBuffers(false, false);
    }

    if (!m_RP.m_WasPortals)
    {
      m_RP.m_fMinDepthRange = 0;
      m_RP.m_fMaxDepthRange = 1.0f;
    }
    else
    {
      if (!m_RP.m_CurPortal)
      {
        m_RP.m_fMinDepthRange = 0;
        m_RP.m_fMaxDepthRange = 0.5f;
      }
      else
      {
        float strt = 0.5f;
        float end  = 1;

        float delta = (end - strt) / (float)m_RP.m_WasPortals;
        strt += delta * (float)(m_RP.m_CurPortal-1);
        m_RP.m_fMinDepthRange = strt+0;
        m_RP.m_fMaxDepthRange = strt+delta;
      }
    }
    glDepthRange(m_RP.m_fMinDepthRange, m_RP.m_fMaxDepthRange);
  }
}

//==============================================================================
// Shader Pipeline
//=======================================================================

// Software vertex deformations stages handling for the current shader
void CGLRenderer::EF_Eval_DeformVerts(TArray<SDeform>* Defs)
{
  int i;

  if (!Defs)
    return;

  for (i=0; i<Defs->Num(); i++)
  {
    SDeform *df = &Defs->Get(i);
    switch (df->m_eType)
    {
      case eDT_Wave:
        m_RP.m_pCurFuncs->WaveDeform(df);
        break;

      case eDT_Flare:
        m_RP.m_pCurFuncs->FlareDeform(df);
        break;

      case eDT_Beam:
        m_RP.m_pCurFuncs->BeamDeform(df);
        break;

      case eDT_VerticalWave:
        m_RP.m_pCurFuncs->VerticalWaveDeform(df);
        break;

      case eDT_Bulge:
        m_RP.m_pCurFuncs->BulgeDeform(df);
        break;

      case eDT_FromCenter:
        m_RP.m_pCurFuncs->FromCenterDeform(df);
        break;

      case eDT_Squeeze:
        m_RP.m_pCurFuncs->SqueezeDeform(df);
        break;

      default:
        iLog->Log("Unknown deform type %d in Shader '%s'\n", df->m_eType, m_RP.m_pShader->m_Name);
        break;
    }
  }
}

// Software/hardware texture coordinates generating modes handling for the current shader
void CGLRenderer::EF_Eval_TexGen(SShaderPass *sfm)
{
  int j, n;
  SShaderTexUnit *shl;
  SShader *ef = m_RP.m_pShader;
  SMRendTexVert *src;
  int m;

  for (j=0; j<sfm->m_TUnits.Num(); j++)
  {
    shl = &sfm->m_TUnits[j];
    
    if (!shl->m_TexPic || shl->m_GTC)
      continue;

    switch (shl->m_eGenTC)
    {
      case eGTC_NoFill:
        break;

      case eGTC_None:
        break;

      case eGTC_Base:
        if (!m_RP.m_pRE)
        {
          m = m_RP.m_RendNumVerts;
          src = m_RP.m_pBaseTexCoordPointer;
          byte *ptr = m_RP.m_Ptr.PtrB + m_RP.m_OffsT + j*8;
          for (n=0; n<m; n++, ptr+=m_RP.m_Stride)
          {
            *(float *)(ptr) = src[n].vert[0];
            *(float *)(ptr+4) = src[n].vert[1];
          }
        }
        break;

      case eGTC_Quad:
        if (!m_RP.m_pRE)
        {
          m = m_RP.m_RendNumVerts;
          byte *ptr = m_RP.m_Ptr.PtrB + m_RP.m_OffsT + j*16;
          float tverts[4][2] = 
          {
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1}
          };
          for (n=0; n<m; n++, ptr+=m_RP.m_Stride)
          {
            int nm = n & 3;
            *(float *)(ptr) = tverts[nm][0];
            *(float *)(ptr+4) = tverts[nm][1];
          }
        }
        break;

      case eGTC_LightMap:
        if (!m_RP.m_pRE)
        {
          m = m_RP.m_RendNumVerts;
          src = m_RP.m_pLMTexCoordPointer;
          byte *ptr = m_RP.m_Ptr.PtrB + m_RP.m_OffsT + j*16;
          for (n=0; n<m; n++, ptr+=m_RP.m_Stride)
          {
            *(float *)(ptr) = src[n].vert[0];
            *(float *)(ptr+4) = src[n].vert[1];
          }
        }
        break;

      case eGTC_Environment:
        m_RP.m_pCurFuncs->ETC_Environment(j);
        break;

      case eGTC_Projection:
        if (shl->m_TexPic->m_Matrix)
          m_RP.m_pCurFuncs->ETC_Projection(j, shl->m_TexPic->m_Matrix, 1, 1);
        break;

      case eGTC_SphereMap:
        m_RP.m_pCurFuncs->ETC_SphereMap(j);
        break;

      case eGTC_SphereMapEnvironment:
        m_RP.m_pCurFuncs->ETC_SphereMapEnvironment(j);
        break;

      case eGTC_ShadowMap:
        m_RP.m_pCurFuncs->ETC_ShadowMap(j);
        break;
        
      default:
        iLog->Log("ERROR: invalid eGenTC '%d' specified for Shader\n", shl->m_eGenTC);
        break;
    }
  }
}

// Software/hardware RGBA generating/modificating modes handling for the current shader
void CGLRenderer::EF_Eval_RGBAGen(SShaderPass *sfm)
{
  SShader *ef = m_RP.m_pShader;
  int n;
  UCol color;
  bool bSetCol = false;
  color.dcolor = -1;

  switch(sfm->m_eEvalRGB)
  {
    case eERGB_NoFill:
      break;

    case eERGB_Identity:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
        {
          color.dcolor = -1;
          bSetCol = true;
          m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
        }
      }
      else
      {
        byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
        for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
        {
          *(uint *)ptr = -1;
        }
      }
      break;

    case eERGB_FromClient:
      if (!m_RP.m_pRE)
      {
        if (!gbRgb)
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          byte *src = (byte *)(m_RP.m_pClientColors[0]);
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride, src+=4)
          {
            *(uint *)ptr = *(uint *)(src);
          }
        }
        else
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          byte *src = (byte *)(&m_RP.m_pClientColors[0]);
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride, src+=4)
          {
            ptr[2] = src[0];
            ptr[1] = src[1];
            ptr[0] = src[2];
            ptr[3] = src[3];
          }
        }
      }
      break;

    case eERGB_Fixed:
      color = sfm->m_FixedColor;
      bSetCol = true;
      m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      break;

    case eERGB_StyleIntens:
      {
        CLightStyle *ls = CLightStyle::mfGetStyle(sfm->m_Style, m_RP.m_RealTime);
        color = sfm->m_FixedColor;
        color.bcolor[0] = (byte)((float)color.bcolor[0] * ls->m_fIntensity);
        color.bcolor[1] = (byte)((float)color.bcolor[1] * ls->m_fIntensity);
        color.bcolor[2] = (byte)((float)color.bcolor[2] * ls->m_fIntensity);
        bSetCol = true;
        m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      }
      break;

    case eERGB_StyleColor:
      {
        CLightStyle *ls = CLightStyle::mfGetStyle(sfm->m_Style, m_RP.m_RealTime);
        color.dcolor = ls->m_Color.GetTrue();
        bSetCol = true;
        m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      }
      break;

    case eERGB_Comps:
      {
        if (sfm->m_RGBComps)
        {
          float *vals = sfm->m_RGBComps->mfGet();
          color.bcolor[0] = (byte)(vals[0] * 255.0f);
          color.bcolor[1] = (byte)(vals[1] * 255.0f);
          color.bcolor[2] = (byte)(vals[2] * 255.0f);
          color.bcolor[3] = (byte)(vals[3] * 255.0f);
          bSetCol = true;
          m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
        }
      }
      break;

    case eERGB_OneMinusFromClient:
      if (!gbRgb)
      {
        byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
        for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
        {
          ptr[0] = 255 - m_RP.m_pClientColors[n][0];
          ptr[1] = 255 - m_RP.m_pClientColors[n][1];
          ptr[2] = 255 - m_RP.m_pClientColors[n][2];
        }
      }
      else
      {
        byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
        for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
        {
          ptr[0] = 255 - m_RP.m_pClientColors[n][2];
          ptr[1] = 255 - m_RP.m_pClientColors[n][1];
          ptr[2] = 255 - m_RP.m_pClientColors[n][0];
        }
      }
      break;

    case eERGB_Wave:
      if (sfm->m_WaveEvalRGB)
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
          {
            float val = SEvalFuncs::EvalWaveForm(sfm->m_WaveEvalRGB);
            if (val < 0)
              val = 0;
            if (val > 1)
              val = 1;
            
            color.bcolor[0] = color.bcolor[1] = color.bcolor[2] = (int)(val * 255.0f);
            COLCONV(color.dcolor);
            bSetCol = true;
            m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
          }
          else
            m_RP.m_pCurFuncs->ERGB_Wave(sfm->m_WaveEvalRGB, color);
        }
      }
      break;

    case eERGB_Noise:
      if (sfm->m_RGBNoise)
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
          {
            float v = RandomNum();
            byte r = (byte)(CLAMP(v * sfm->m_RGBNoise->m_RangeR + sfm->m_RGBNoise->m_ConstR, 0.0f, 1.0f) * 255.0f);
            v = RandomNum();
            byte g = (byte)(CLAMP(v * sfm->m_RGBNoise->m_RangeG + sfm->m_RGBNoise->m_ConstG, 0.0f, 1.0f) * 255.0f);
            v = RandomNum();
            byte b = (byte)(CLAMP(v * sfm->m_RGBNoise->m_RangeB + sfm->m_RGBNoise->m_ConstB, 0.0f, 1.0f) * 255.0f);
            
            color.bcolor[0] = r;
            color.bcolor[1] = g;
            color.bcolor[2] = b;
            COLCONV(color.dcolor);
            bSetCol = true;
            m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
          }
          else
            m_RP.m_pCurFuncs->ERGB_Noise(sfm->m_RGBNoise, color);
        }
      }
      break;

    case eERGB_Object:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
        {
          bSetCol = true;
          color.bcolor[0] = (byte)(m_RP.m_pCurObject->m_Color[0] * 255.0f);
          color.bcolor[1] = (byte)(m_RP.m_pCurObject->m_Color[1] * 255.0f);
          color.bcolor[2] = (byte)(m_RP.m_pCurObject->m_Color[2] * 255.0f);
          m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
        }
        else
          m_RP.m_pCurFuncs->ERGB_Object();
      }
      break;

    case eERGB_OneMinusObject:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
        {
          bSetCol = true;
          color.bcolor[0] = (byte)((1.0f - m_RP.m_pCurObject->m_Color[0]) * 255.0f);
          color.bcolor[1] = (byte)((1.0f - m_RP.m_pCurObject->m_Color[1]) * 255.0f);
          color.bcolor[2] = (byte)((1.0f - m_RP.m_pCurObject->m_Color[2]) * 255.0f);
          m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
        }
      }
      else
        m_RP.m_pCurFuncs->ERGB_OneMinusObject();
      break;

    case eERGB_RE:
      if (m_RP.m_pRE && !(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
      {
        bSetCol = true;
        color.bcolor[0] = (byte)(m_RP.m_pRE->m_Color[0] * 255.0f);
        color.bcolor[1] = (byte)(m_RP.m_pRE->m_Color[1] * 255.0f);
        color.bcolor[2] = (byte)(m_RP.m_pRE->m_Color[2] * 255.0f);
        m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      }
      break;
      
    case eERGB_OneMinusRE:
      if (m_RP.m_pRE && !(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
      {
        bSetCol = true;
        color.bcolor[0] = (byte)((1.0f - m_RP.m_pRE->m_Color[0]) * 255.0f);
        color.bcolor[1] = (byte)((1.0f - m_RP.m_pRE->m_Color[1]) * 255.0f);
        color.bcolor[2] = (byte)((1.0f - m_RP.m_pRE->m_Color[2]) * 255.0f);
        m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      }
      break;

    case eERGB_World:
      if (m_RP.m_pRE && !(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
      {
        bSetCol = true;
        color.bcolor[0] = (byte)(m_WorldColor[0] * 255.0f);
        color.bcolor[1] = (byte)(m_WorldColor[1] * 255.0f);
        color.bcolor[2] = (byte)(m_WorldColor[2] * 255.0f);
        m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
      }
      break;

    default:
      assert(0);
  }

  switch(sfm->m_eEvalAlpha)
  {
    case eEALPHA_NoFill:
      break;

    case eEALPHA_Identity:
      if (sfm->m_eEvalRGB!=eERGB_Identity && sfm->m_eEvalRGB!=eERGB_Fixed)
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_RGBGEN))
          {
            color.bcolor[3] = 255;
            bSetCol = true;
            m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
          }
        }
        else
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
          {
            ptr[3] = 255;
          }
        }
      }
      break;

    case eEALPHA_Fixed:
      {
        if (sfm->m_eEvalRGB == eERGB_Fixed)
          break;
        if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          color.bcolor[3] = sfm->m_FixedColor.bcolor[3];
          bSetCol = true;
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
        }
      }
      break;

    case eEALPHA_Style:
      {
        CLightStyle *ls = CLightStyle::mfGetStyle(sfm->m_Style, m_RP.m_RealTime);
        color.bcolor[3] = (byte)((float)sfm->m_FixedColor.bcolor[3] * ls->m_fIntensity);
        bSetCol = true;
        m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
      }
      break;

    case eEALPHA_Comps:
      {
        if (sfm->m_eEvalRGB == eERGB_Comps)
          break;
        if (sfm->m_RGBComps)
        {
          float *vals = sfm->m_RGBComps->mfGet();
          if (m_RP.m_pRE)
          {
            if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
            {
              color.bcolor[3] = (byte)(vals[0] * 255.0f);
              bSetCol = true;
              m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
            }
          }
          else
          {
            byte a = (byte)(vals[0] * 255.0f);
            byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
            for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
            {
              ptr[3] = a;
            }
          }
        }
      }
      break;

    case eEALPHA_Wave:
      if (sfm->m_WaveEvalAlpha)
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
          {
            float val = SEvalFuncs::EvalWaveForm(sfm->m_WaveEvalAlpha);
            if (val < 0)
              val = 0;
            if (val > 1)
              val = 1;
            
            color.bcolor[3] = (int)(val * 255.0f);
            bSetCol = true;
            m_RP.m_FlagsPerFlush = RBSI_ALPHAGEN;
          }
        }
        else
          m_RP.m_pCurFuncs->EALPHA_Wave(sfm->m_WaveEvalAlpha, color);
      }
      break;

    case eEALPHA_Noise:
      if (sfm->m_ANoise)
      {
        if (m_RP.m_pRE)
        {
          if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
          {
            float v = RandomNum();
            byte a = (byte)(CLAMP(v * sfm->m_ANoise->m_RangeA + sfm->m_ANoise->m_ConstA, 0.0f, 1.0f) * 255.0f);
            
            color.bcolor[3] = a;
            bSetCol = true;
            m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
          }
        }
        else
          m_RP.m_pCurFuncs->EALPHA_Noise(sfm->m_ANoise, color);
      }
      break;

    case eEALPHA_Beam:
      m_RP.m_pCurFuncs->EALPHA_Beam();
      break;

    case eEALPHA_Object:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          bSetCol = true;
          color.bcolor[3] = (byte)(m_RP.m_pCurObject->m_Color[3] * 255.0f);
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
        }
      }
      else
        m_RP.m_pCurFuncs->EALPHA_Object();
      break;

    case eEALPHA_OneMinusObject:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          bSetCol = true;
          color.bcolor[3] = (byte)((1.0f - m_RP.m_pCurObject->m_Color[3]) * 255.0f);
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
        }
      }
      else
        m_RP.m_pCurFuncs->EALPHA_OneMinusObject();
      break;

    case eEALPHA_RE:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          bSetCol = true;
          color.bcolor[3] = (byte)(m_RP.m_pRE->m_Color[3] * 255.0f);
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
        }
      }
      break;
      
    case eEALPHA_OneMinusRE:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          bSetCol = true;
          color.bcolor[3] = (byte)((1.0f - m_RP.m_pCurObject->m_Color[3]) * 255.0f);
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
        }
      }
      break;

    case eEALPHA_World:
      if (m_RP.m_pRE)
      {
        if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHAGEN))
        {
          bSetCol = true;
          color.bcolor[3] = (byte)(m_WorldColor[3] * 255.0f);
          m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
        }
      }
      break;

    case eEALPHA_FromClient:
      if (!m_RP.m_pRE)
      {
        if (sfm->m_eEvalRGB!=eERGB_FromClient)
        {
          byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsD;
          for (n=0; n<m_RP.m_RendNumVerts; n++, ptr+=m_RP.m_Stride)
          {
            ptr[3] = m_RP.m_pClientColors[n][3];
          }
        }
      }
      break;

    default:
      assert(0);
  }

  if (bSetCol)
  {
    m_RP.m_NeedGlobalColor = color;
  }
}

// Software Normals generating/modificating modes handling for the current shader
void CGLRenderer::EF_EvalNormalsRB(SShader *ef)
{
  if (!ef->m_NormGen)
    return;

  SNormalsGen *ng = ef->m_NormGen;

  if (ng->m_eNormal == eNORM_Identity || !m_RP.m_OffsN)
    return;

  int i;
  int nums = m_RP.m_RendNumVerts;
  byte *ptr = m_RP.m_Ptr.PtrB+m_RP.m_OffsN;

  switch(ng->m_eNormal)
  {
    case eNORM_Custom:
      for (i=0; i<nums; i++, ptr+=m_RP.m_Stride)
      {
        float *tsnrm = (float *)ptr;
        tsnrm[0] = ng->m_CustomNormal[0];
        tsnrm[1] = ng->m_CustomNormal[1];
        tsnrm[2] = ng->m_CustomNormal[2];
      }
      break;
  }
}

//=================================================================================

// Set current geometry culling modes
void CGLRenderer::GLSetCull(ECull eCull)
{ 
  //eCull = eCULL_Front;
  if (m_RP.m_Flags & RBF_2D)
  {
    glDisable(GL_CULL_FACE);
    m_RP.m_eCull = eCULL_None;
    return;
  }
  if (m_RP.m_eCull == eCull)
    return;

  if (eCull == eCULL_None)
    glDisable(GL_CULL_FACE);
  else
  {
    glEnable(GL_CULL_FACE);

    if ((eCull == eCULL_Back && !(m_RP.m_PersFlags & RBPF_DRAWMIRROR)) || (eCull != eCULL_Back && (m_RP.m_PersFlags & RBPF_DRAWMIRROR)))
      glCullFace(GL_BACK);
    else
      glCullFace(GL_FRONT);
  }
  m_RP.m_eCull = eCull;
}

//=================================================================================
// Set current stencil states 
void SStencil::mfSet()
{
  gRenDev->EF_SetStencilState(m_State, m_FuncRef, m_FuncMask);
}

// Set current stencil states 
void CRenderer::EF_SetStencilState(int st, uint nStencRef, uint nStencMask)
{
  bool bChanged = false;
  if (nStencRef != m_CurStencRef || nStencMask != m_CurStencMask)
  {
    m_CurStencRef = nStencRef;
    m_CurStencMask = nStencMask;
    bChanged = true;
  }
  int Changed = st ^ m_CurStencilState;
  if (!Changed && !bChanged)
    return;
  if (SUPPORTS_GL_ATI_separate_stencil)
  {
    int nFrontFunc, nBackFunc;
    if ((Changed & FSS_STENCFUNC_MASK) || bChanged)
    {
      bChanged = true;
      int nCurFunc = st & FSS_STENCFUNC_MASK;
      switch(nCurFunc)
      {
        case FSS_STENCFUNC_ALWAYS:
      	  nBackFunc = GL_ALWAYS;
    	    break;
        case FSS_STENCFUNC_NEVER:
      	  nBackFunc = GL_NEVER;
    	    break;
        case FSS_STENCFUNC_LESS:
      	  nBackFunc = GL_LESS;
    	    break;
        case FSS_STENCFUNC_LEQUAL:
      	  nBackFunc = GL_LEQUAL;
    	    break;
        case FSS_STENCFUNC_GREATER:
      	  nBackFunc = GL_GREATER;
    	    break;
        case FSS_STENCFUNC_GEQUAL:
      	  nBackFunc = GL_GEQUAL;
    	    break;
        case FSS_STENCFUNC_EQUAL:
      	  nBackFunc = GL_EQUAL;
    	    break;
        case FSS_STENCFUNC_NOTEQUAL:
      	  nBackFunc = GL_NOTEQUAL;
    	    break;
        default:
          assert(false);
      }
    }
    if ((Changed & (FSS_STENCFUNC_MASK << FSS_STENCIL_TWOSIDED)) || bChanged)
    {
      bChanged = true;
      int nCurFunc;
      nCurFunc = (st & (FSS_STENCFUNC_MASK << FSS_CCW_SHIFT));
      switch(nCurFunc >> FSS_CCW_SHIFT)
      {
        case FSS_STENCFUNC_ALWAYS:
      	  nFrontFunc = GL_ALWAYS;
    	    break;
        case FSS_STENCFUNC_NEVER:
      	  nFrontFunc = GL_NEVER;
    	    break;
        case FSS_STENCFUNC_LESS:
      	  nFrontFunc = GL_LESS;
    	    break;
        case FSS_STENCFUNC_LEQUAL:
      	  nFrontFunc = GL_LEQUAL;
    	    break;
        case FSS_STENCFUNC_GREATER:
      	  nFrontFunc = GL_GREATER;
    	    break;
        case FSS_STENCFUNC_GEQUAL:
      	  nFrontFunc = GL_GEQUAL;
    	    break;
        case FSS_STENCFUNC_EQUAL:
      	  nFrontFunc = GL_EQUAL;
    	    break;
        case FSS_STENCFUNC_NOTEQUAL:
      	  nFrontFunc = GL_NOTEQUAL;
    	    break;
        default:
          assert(false);
      }
    }
    if (bChanged)
      glStencilFuncSeparateATI(nBackFunc, nFrontFunc, nStencRef, nStencMask);

    if (Changed & (FSS_STENCFAIL_MASK | FSS_STENCZFAIL_MASK | FSS_STENCPASS_MASK))
    {
      int nFail, nZFail, nPass;
      int nCurOp = (st & FSS_STENCFAIL_MASK);
      switch(nCurOp >> FSS_STENCFAIL_SHIFT)
      {
        case FSS_STENCOP_KEEP:
          nFail = GL_KEEP;
    	    break;
        case FSS_STENCOP_REPLACE:
          nFail = GL_REPLACE;
    	    break;
        case FSS_STENCOP_INCR:
          nFail = GL_INCR;
    	    break;
        case FSS_STENCOP_DECR:
          nFail = GL_DECR;
    	    break;
        case FSS_STENCOP_INCR_WRAP:
          nFail = GL_INCR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_DECR_WRAP:
          nFail = GL_DECR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_ZERO:
          nFail = GL_ZERO;
    	    break;
        default:
          assert(false);
      }
      nCurOp = (st & FSS_STENCZFAIL_MASK);
      switch(nCurOp >> FSS_STENCZFAIL_SHIFT)
      {
        case FSS_STENCOP_KEEP:
          nZFail = GL_KEEP;
    	    break;
        case FSS_STENCOP_REPLACE:
          nZFail = GL_REPLACE;
    	    break;
        case FSS_STENCOP_INCR:
          nZFail = GL_INCR;
    	    break;
        case FSS_STENCOP_DECR:
          nZFail = GL_DECR;
    	    break;
        case FSS_STENCOP_INCR_WRAP:
          nZFail = GL_INCR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_DECR_WRAP:
          nZFail = GL_DECR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_ZERO:
          nZFail = GL_ZERO;
    	    break;
        default:
          assert(false);
      }
      nCurOp = (st & FSS_STENCPASS_MASK);
      switch(nCurOp >> FSS_STENCPASS_SHIFT)
      {
        case FSS_STENCOP_KEEP:
          nPass = GL_KEEP;
    	    break;
        case FSS_STENCOP_REPLACE:
          nPass = GL_REPLACE;
    	    break;
        case FSS_STENCOP_INCR:
          nPass = GL_INCR;
    	    break;
        case FSS_STENCOP_DECR:
          nPass = GL_DECR;
    	    break;
        case FSS_STENCOP_INCR_WRAP:
          nPass = GL_INCR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_DECR_WRAP:
          nPass = GL_DECR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_ZERO:
          nPass = GL_ZERO;
    	    break;
        default:
          assert(false);
      }
		  glStencilOpSeparateATI(GL_FRONT, nFail, nZFail, nPass);
    }

    if (Changed & ((FSS_STENCFAIL_MASK | FSS_STENCZFAIL_MASK | FSS_STENCPASS_MASK) << FSS_CCW_SHIFT))
    {
      int nFail, nZFail, nPass;
      int nCurOp = (st & (FSS_STENCFAIL_MASK<<FSS_CCW_SHIFT));
      switch(nCurOp >> (FSS_STENCFAIL_SHIFT+FSS_CCW_SHIFT))
      {
        case FSS_STENCOP_KEEP:
          nFail = GL_KEEP;
    	    break;
        case FSS_STENCOP_REPLACE:
          nFail = GL_REPLACE;
    	    break;
        case FSS_STENCOP_INCR:
          nFail = GL_INCR;
    	    break;
        case FSS_STENCOP_DECR:
          nFail = GL_DECR;
    	    break;
        case FSS_STENCOP_INCR_WRAP:
          nFail = GL_INCR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_DECR_WRAP:
          nFail = GL_DECR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_ZERO:
          nFail = GL_ZERO;
    	    break;
        default:
          assert(false);
      }
      nCurOp = (st & (FSS_STENCZFAIL_MASK<<FSS_CCW_SHIFT));
      switch(nCurOp >> (FSS_STENCZFAIL_SHIFT+FSS_CCW_SHIFT))
      {
        case FSS_STENCOP_KEEP:
          nZFail = GL_KEEP;
    	    break;
        case FSS_STENCOP_REPLACE:
          nZFail = GL_REPLACE;
    	    break;
        case FSS_STENCOP_INCR:
          nZFail = GL_INCR;
    	    break;
        case FSS_STENCOP_DECR:
          nZFail = GL_DECR;
    	    break;
        case FSS_STENCOP_INCR_WRAP:
          nZFail = GL_INCR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_DECR_WRAP:
          nZFail = GL_DECR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_ZERO:
          nZFail = GL_ZERO;
    	    break;
        default:
          assert(false);
      }
      nCurOp = (st & (FSS_STENCPASS_MASK<<FSS_CCW_SHIFT));
      switch(nCurOp >> (FSS_STENCPASS_SHIFT+FSS_CCW_SHIFT))
      {
        case FSS_STENCOP_KEEP:
          nPass = GL_KEEP;
    	    break;
        case FSS_STENCOP_REPLACE:
          nPass = GL_REPLACE;
    	    break;
        case FSS_STENCOP_INCR:
          nPass = GL_INCR;
    	    break;
        case FSS_STENCOP_DECR:
          nPass = GL_DECR;
    	    break;
        case FSS_STENCOP_INCR_WRAP:
          nPass = GL_INCR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_DECR_WRAP:
          nPass = GL_DECR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_ZERO:
          nPass = GL_ZERO;
    	    break;
        default:
          assert(false);
      }
		  glStencilOpSeparateATI(GL_BACK, nFail, nZFail, nPass);
    }
    m_CurStencilState = st;

    return;
  }

  if (Changed & FSS_STENCIL_TWOSIDED)
  {
    if (SUPPORTS_GL_EXT_stencil_two_side)
    {
      if (st & FSS_STENCIL_TWOSIDED)
        glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
      else
        glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
    }
  }
  if ((Changed & FSS_STENCFUNC_MASK) || bChanged)
  {
    if (SUPPORTS_GL_EXT_stencil_two_side)
    {
      if (m_CurStencilSide != GL_FRONT)
      {
        m_CurStencilSide = GL_FRONT;
        glActiveStencilFaceEXT(GL_FRONT);
      }
    }
    int nCurFunc = st & FSS_STENCFUNC_MASK;
    switch(nCurFunc)
    {
      case FSS_STENCFUNC_ALWAYS:
      	glStencilFunc(GL_ALWAYS, nStencRef, nStencMask);
    	  break;
      case FSS_STENCFUNC_NEVER:
      	glStencilFunc(GL_NEVER, nStencRef, nStencMask);
    	  break;
      case FSS_STENCFUNC_LESS:
      	glStencilFunc(GL_LESS, nStencRef, nStencMask);
    	  break;
      case FSS_STENCFUNC_LEQUAL:
      	glStencilFunc(GL_LEQUAL, nStencRef, nStencMask);
    	  break;
      case FSS_STENCFUNC_GREATER:
      	glStencilFunc(GL_GREATER, nStencRef, nStencMask);
    	  break;
      case FSS_STENCFUNC_GEQUAL:
      	glStencilFunc(GL_GEQUAL, nStencRef, nStencMask);
    	  break;
      case FSS_STENCFUNC_EQUAL:
      	glStencilFunc(GL_EQUAL, nStencRef, nStencMask);
    	  break;
      case FSS_STENCFUNC_NOTEQUAL:
      	glStencilFunc(GL_NOTEQUAL, nStencRef, nStencMask);
    	  break;
      default:
        assert(false);
    }
  }
  if (Changed & (FSS_STENCFAIL_MASK | FSS_STENCZFAIL_MASK | FSS_STENCPASS_MASK))
  {
    if (SUPPORTS_GL_EXT_stencil_two_side)
    {
      if (m_CurStencilSide != GL_FRONT)
      {
        m_CurStencilSide = GL_FRONT;
        glActiveStencilFaceEXT(GL_FRONT);
      }
    }

    int nFail, nZFail, nPass;
    int nCurOp = (st & FSS_STENCFAIL_MASK);
    switch(nCurOp >> FSS_STENCFAIL_SHIFT)
    {
      case FSS_STENCOP_KEEP:
        nFail = GL_KEEP;
    	  break;
      case FSS_STENCOP_REPLACE:
        nFail = GL_REPLACE;
    	  break;
      case FSS_STENCOP_INCR:
        nFail = GL_INCR;
    	  break;
      case FSS_STENCOP_DECR:
        nFail = GL_DECR;
    	  break;
      case FSS_STENCOP_INCR_WRAP:
        nFail = GL_INCR_WRAP_EXT;
    	  break;
      case FSS_STENCOP_DECR_WRAP:
        nFail = GL_DECR_WRAP_EXT;
    	  break;
      case FSS_STENCOP_ZERO:
        nFail = GL_ZERO;
    	  break;
      default:
        assert(false);
    }
    nCurOp = (st & FSS_STENCZFAIL_MASK);
    switch(nCurOp >> FSS_STENCZFAIL_SHIFT)
    {
      case FSS_STENCOP_KEEP:
        nZFail = GL_KEEP;
    	  break;
      case FSS_STENCOP_REPLACE:
        nZFail = GL_REPLACE;
    	  break;
      case FSS_STENCOP_INCR:
        nZFail = GL_INCR;
    	  break;
      case FSS_STENCOP_DECR:
        nZFail = GL_DECR;
    	  break;
      case FSS_STENCOP_INCR_WRAP:
        nZFail = GL_INCR_WRAP_EXT;
    	  break;
      case FSS_STENCOP_DECR_WRAP:
        nZFail = GL_DECR_WRAP_EXT;
    	  break;
      case FSS_STENCOP_ZERO:
        nZFail = GL_ZERO;
    	  break;
      default:
        assert(false);
    }
    nCurOp = (st & FSS_STENCPASS_MASK);
    switch(nCurOp >> FSS_STENCPASS_SHIFT)
    {
      case FSS_STENCOP_KEEP:
        nPass = GL_KEEP;
    	  break;
      case FSS_STENCOP_REPLACE:
        nPass = GL_REPLACE;
    	  break;
      case FSS_STENCOP_INCR:
        nPass = GL_INCR;
    	  break;
      case FSS_STENCOP_DECR:
        nPass = GL_DECR;
    	  break;
      case FSS_STENCOP_INCR_WRAP:
        nPass = GL_INCR_WRAP_EXT;
    	  break;
      case FSS_STENCOP_DECR_WRAP:
        nPass = GL_DECR_WRAP_EXT;
    	  break;
      case FSS_STENCOP_ZERO:
        nPass = GL_ZERO;
    	  break;
      default:
        assert(false);
    }
		glStencilOp(nFail, nZFail, nPass);
  }

  if (SUPPORTS_GL_EXT_stencil_two_side)
  {
    if ((Changed & (FSS_STENCFUNC_MASK << FSS_CCW_SHIFT)) || bChanged)
    {
      if (m_CurStencilSide != GL_BACK)
      {
        m_CurStencilSide = GL_BACK;
        glActiveStencilFaceEXT(GL_BACK);
      }
      int nCurFunc = (st & (FSS_STENCFUNC_MASK << FSS_CCW_SHIFT));
      switch(nCurFunc >> FSS_CCW_SHIFT)
      {
        case FSS_STENCFUNC_ALWAYS:
      	  glStencilFunc(GL_ALWAYS, nStencRef, nStencMask);
    	    break;
        case FSS_STENCFUNC_NEVER:
      	  glStencilFunc(GL_NEVER, nStencRef, nStencMask);
    	    break;
        case FSS_STENCFUNC_LESS:
      	  glStencilFunc(GL_LESS, nStencRef, nStencMask);
    	    break;
        case FSS_STENCFUNC_LEQUAL:
      	  glStencilFunc(GL_LEQUAL, nStencRef, nStencMask);
    	    break;
        case FSS_STENCFUNC_GREATER:
      	  glStencilFunc(GL_GREATER, nStencRef, nStencMask);
    	    break;
        case FSS_STENCFUNC_GEQUAL:
      	  glStencilFunc(GL_GEQUAL, nStencRef, nStencMask);
    	    break;
        case FSS_STENCFUNC_EQUAL:
      	  glStencilFunc(GL_EQUAL, nStencRef, nStencMask);
    	    break;
        case FSS_STENCFUNC_NOTEQUAL:
      	  glStencilFunc(GL_NOTEQUAL, nStencRef, nStencMask);
    	    break;
        default:
          assert(false);
      }
    }
    if (Changed & ((FSS_STENCFAIL_MASK | FSS_STENCZFAIL_MASK | FSS_STENCPASS_MASK) << FSS_CCW_SHIFT))
    {
      if (m_CurStencilSide != GL_BACK)
      {
        m_CurStencilSide = GL_BACK;
        glActiveStencilFaceEXT(GL_BACK);
      }

      int nFail, nZFail, nPass;
      int nCurOp = (st & (FSS_STENCFAIL_MASK << FSS_CCW_SHIFT));
      switch(nCurOp >> (FSS_STENCFAIL_SHIFT+FSS_CCW_SHIFT))
      {
        case FSS_STENCOP_KEEP:
          nFail = GL_KEEP;
    	    break;
        case FSS_STENCOP_REPLACE:
          nFail = GL_REPLACE;
    	    break;
        case FSS_STENCOP_INCR:
          nFail = GL_INCR;
    	    break;
        case FSS_STENCOP_DECR:
          nFail = GL_DECR;
    	    break;
        case FSS_STENCOP_INCR_WRAP:
          nFail = GL_INCR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_DECR_WRAP:
          nFail = GL_DECR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_ZERO:
          nFail = GL_ZERO;
    	    break;
        default:
          assert(false);
      }
      nCurOp = (st & (FSS_STENCZFAIL_MASK << FSS_CCW_SHIFT));
      switch(nCurOp >> (FSS_STENCZFAIL_SHIFT+FSS_CCW_SHIFT))
      {
        case FSS_STENCOP_KEEP:
          nZFail = GL_KEEP;
    	    break;
        case FSS_STENCOP_REPLACE:
          nZFail = GL_REPLACE;
    	    break;
        case FSS_STENCOP_INCR:
          nZFail = GL_INCR;
    	    break;
        case FSS_STENCOP_DECR:
          nZFail = GL_DECR;
    	    break;
        case FSS_STENCOP_INCR_WRAP:
          nZFail = GL_INCR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_DECR_WRAP:
          nZFail = GL_DECR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_ZERO:
          nZFail = GL_ZERO;
    	    break;
        default:
          assert(false);
      }
      nCurOp = (st & (FSS_STENCPASS_MASK << FSS_CCW_SHIFT));
      switch(nCurOp >> (FSS_STENCPASS_SHIFT+FSS_CCW_SHIFT))
      {
        case FSS_STENCOP_KEEP:
          nPass = GL_KEEP;
    	    break;
        case FSS_STENCOP_REPLACE:
          nPass = GL_REPLACE;
    	    break;
        case FSS_STENCOP_INCR:
          nPass = GL_INCR;
    	    break;
        case FSS_STENCOP_DECR:
          nPass = GL_DECR;
    	    break;
        case FSS_STENCOP_INCR_WRAP:
          nPass = GL_INCR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_DECR_WRAP:
          nPass = GL_DECR_WRAP_EXT;
    	    break;
        case FSS_STENCOP_ZERO:
          nPass = GL_ZERO;
    	    break;
        default:
          assert(false);
      }
		  glStencilOp(nFail, nZFail, nPass);
    }
  }

  m_CurStencilState = st;
}

// Set current render states 
void CRenderer::EF_SetState(int st)
{
  int Changed;
  int src, dst;

  if (m_RP.m_Flags & RBF_SHOWLINES)
    st |= GS_NODEPTHTEST;
  
  if (st & (GS_DEPTHWRITE | GS_DEPTHFUNC_EQUAL))
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHABLEND))
      m_RP.m_FlagsPerFlush |= RBSI_WASDEPTHWRITE;
  }
  if (st & GS_DEPTHWRITE)
  {
    if (m_RP.m_LastVP && m_RP.m_pRE)
      m_RP.m_pRE->m_LastVP = m_RP.m_LastVP;
  }

  Changed = st ^ m_CurState;
  if (!Changed)
    return;

  //PROFILE_FRAME(State_RStates);

  if (Changed & (GS_DEPTHFUNC_EQUAL | GS_DEPTHFUNC_GREAT))
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_DEPTHFUNC))
    {
      if (st & (GS_DEPTHFUNC_EQUAL|GS_DEPTHFUNC_GREAT))
      {
        if (st & GS_DEPTHFUNC_EQUAL)
          glDepthFunc(GL_EQUAL);
        else
          glDepthFunc(GL_GREATER);
      }
      else
        glDepthFunc(GL_LEQUAL);
    }
    else
    {
      st &= ~(GS_DEPTHFUNC_EQUAL|GS_DEPTHFUNC_GREAT);
      st |= (m_CurState & (GS_DEPTHFUNC_EQUAL|GS_DEPTHFUNC_GREAT));
    }
  }
  
  if (Changed & (GS_NOCOLMASK|GS_COLMASKONLYALPHA|GS_COLMASKONLYRGB))
  {
    if (st & (GS_NOCOLMASK|GS_COLMASKONLYALPHA|GS_COLMASKONLYRGB))
    {
      if (st & GS_NOCOLMASK)
        glColorMask(0, 0, 0, 0);
      else
      if (st & GS_COLMASKONLYALPHA)
        glColorMask(0, 0, 0, 1);
      else
      if (st & GS_COLMASKONLYRGB)
        glColorMask(1, 1, 1, 0);
    }
    else
      glColorMask(1, 1, 1, 1);
  }
  
  if ((Changed & GS_BLEND_MASK))
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHABLEND))
    {
      if ((st & GS_BLEND_MASK) == GS_BLEND_MASK)
      {
        st &= ~GS_BLEND_MASK;
        st |= (m_CurState & GS_BLEND_MASK);
        goto skip;
      }

      if (st & GS_BLEND_MASK)
      {
        // Src factor
        switch (st & GS_BLSRC_MASK)
        {
          case GS_BLSRC_ZERO:
            src = GL_ZERO;
            break;

          case GS_BLSRC_ONE:
            src = GL_ONE;
            break;

          case GS_BLSRC_DSTCOL:
            src = GL_DST_COLOR;
            break;

          case GS_BLSRC_ONEMINUSDSTCOL:
            src = GL_ONE_MINUS_DST_COLOR;
            break;

          case GS_BLSRC_SRCALPHA:
            src = GL_SRC_ALPHA;
            break;

          case GS_BLSRC_ONEMINUSSRCALPHA:
            src = GL_ONE_MINUS_SRC_ALPHA;
            break;

          case GS_BLSRC_DSTALPHA:
            src = GL_DST_ALPHA;
            break;

          case GS_BLSRC_ONEMINUSDSTALPHA:
            src = GL_ONE_MINUS_DST_ALPHA;
            break;

          case GS_BLSRC_ALPHASATURATE:
            src = GL_SRC_ALPHA_SATURATE;
            break;

          default:
            iLog->Log("CGLRenderer::mfState: invalid src blend state bits '0x%x'", st & GS_BLSRC_MASK);
            src = GL_ONE;
            break;
        }

        // Dst factor
        switch (st & GS_BLDST_MASK)
        {
          case GS_BLDST_ZERO:
            dst = GL_ZERO;
            break;

          case GS_BLDST_ONE:
            dst = GL_ONE;
            break;

          case GS_BLDST_SRCCOL:
            dst = GL_SRC_COLOR;
            break;

          case GS_BLDST_ONEMINUSSRCCOL:
            dst = GL_ONE_MINUS_SRC_COLOR;
            break;

          case GS_BLDST_SRCALPHA:
            dst = GL_SRC_ALPHA;
            break;

          case GS_BLDST_ONEMINUSSRCALPHA:
            dst = GL_ONE_MINUS_SRC_ALPHA;
            break;

          case GS_BLDST_DSTALPHA:
            dst = GL_DST_ALPHA;
            break;

          case GS_BLDST_ONEMINUSDSTALPHA:
            dst = GL_ONE_MINUS_DST_ALPHA;
            break;

          default:
            iLog->Log("CGLRenderer::mfState: invalid dst blend state bits '0x%x'", st & GS_BLDST_MASK);
            src = GL_ZERO;
            break;
        }
        if (!(m_CurState & GS_BLEND_MASK))
          glEnable(GL_BLEND);
        glBlendFunc(src, dst);
      }
      else
        glDisable(GL_BLEND);
    }
    else
    {
      st &= ~GS_BLEND_MASK;
      st |= (m_CurState & GS_BLEND_MASK);
    }
  }
  
skip:
  if (Changed & GS_DEPTHWRITE)
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_DEPTHWRITE))
    {
      if (st & GS_DEPTHWRITE)
        glDepthMask(GL_TRUE);
      else
        glDepthMask(GL_FALSE);
    }
    else
    {
      st &= ~GS_DEPTHWRITE;
      st |= (m_CurState & GS_DEPTHWRITE);
    }
  }
  
  if (Changed & GS_POLYLINE)
  {
    if (m_polygon_mode == R_SOLID_MODE)
    {
      if (st & GS_POLYLINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }
  
  if (Changed & GS_NODEPTHTEST)
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_DEPTHTEST))
    {
      if (st & GS_NODEPTHTEST)
        glDisable(GL_DEPTH_TEST);
      else
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
      st &= ~GS_NODEPTHTEST;
      st |= (m_CurState & GS_NODEPTHTEST);
    }
  }

  if (Changed & GS_STENCIL)
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_STENCIL) && !(m_RP.m_PersFlags & RBPF_MEASUREOVERDRAW))
    {
      if (st & GS_STENCIL)
      	glEnable(GL_STENCIL_TEST);
      else
      	glDisable(GL_STENCIL_TEST);
    }
    else
    {
      st &= ~GS_STENCIL;
      st |= (m_CurState & GS_STENCIL);
    }
  }
  
  if ((Changed & GS_ALPHATEST_MASK))
  {
    if (!(m_RP.m_FlagsPerFlush & RBSI_ALPHATEST))
    {
      if (st & GS_ALPHATEST_MASK)
      {
        if (!(m_CurState & GS_ALPHATEST_MASK))
          glEnable(GL_ALPHA_TEST);
        switch (st & GS_ALPHATEST_MASK)
        {
          case GS_ALPHATEST_GREATER0:
            glAlphaFunc(GL_GREATER, 0);
            break;

          case GS_ALPHATEST_LESS128:
            glAlphaFunc(GL_LESS, 0.5f);
            break;

          case GS_ALPHATEST_GEQUAL128:
            glAlphaFunc(GL_GEQUAL, 0.5f);
            break;

          case GS_ALPHATEST_GEQUAL64:
            glAlphaFunc(GL_GEQUAL, 0.25f);
            break;
        }
      }
      else
        glDisable(GL_ALPHA_TEST);
    }
    else
    {
      st &= ~GS_ALPHATEST_MASK;
      st |= m_CurState & GS_ALPHATEST_MASK;
    }
  }
  m_CurState = st;
}

// Set current texture color op modes (used in fixed pipeline shaders)
void CGLRenderer::SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa)
{
  EF_SetColorOp(eCo, eAo, eCa, eAa);
}

// Set current texture color op modes (used in fixed pipeline shaders)
void CGLRenderer::EF_SetColorOp(byte eCo, byte eAo, byte eCa, byte eAa)
{
  int stage = m_TexMan->m_CurStage;

  if (CGLTexMan::m_TUState[stage].m_Color.dcolor != m_RP.m_CurGlobalColor.dcolor)
  {
    CGLTexMan::m_TUState[stage].m_Color.dcolor = m_RP.m_CurGlobalColor.dcolor;
    CFColor fCol = CFColor(m_RP.m_CurGlobalColor.bcolor);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (GLfloat*)&fCol[0]);
  }

  if (eCo != 255 && eCo != m_eCurColorOp[stage])
  {
    float fScale = 1.0f;
    if (eCo == eCO_MODULATE2X)
      fScale = 2.0f;
    else
    if (eCo == eCO_MODULATE4X)
      fScale = 4.0f;
    if (m_fCurRGBScale[stage] != fScale)
    {
      m_fCurRGBScale[stage] = fScale;
      glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, fScale);
    }
    switch (eCo)
    {
      case eCO_MODULATE:
      default:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
        break;
      case eCO_REPLACE:
      case eCO_DECAL:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
        break;
      case eCO_MODULATE2X:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
        break;
      case eCO_MODULATE4X:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
        break;
      case eCO_ADD:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
        break;
      case eCO_ADDSIGNED:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD_SIGNED_EXT);
        break;
      case eCO_BLENDDIFFUSEALPHA:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_INTERPOLATE_EXT);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PRIMARY_COLOR_EXT);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_ALPHA);
        break;
      case eCO_BLENDTEXTUREALPHA:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_INTERPOLATE_EXT);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_ALPHA);
        break;
      case eCO_MULTIPLYADD:
        break;
      case eCO_BUMPENVMAP:
        break;
      case eCO_DISABLE:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
        break;
    }
    m_eCurColorOp[stage] = eCo;
  }

  if (eAo != 255 && eAo != m_eCurAlphaOp[stage])
  {
    switch (eAo)
    {
      case eCO_MODULATE:
      case eCO_MODULATE2X:
      case eCO_MODULATE4X:
      default:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_MODULATE);
        break;
      case eCO_BLENDDIFFUSEALPHA:
        break;
      case eCO_BLENDTEXTUREALPHA:
        break;
      case eCO_ADD:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD);
        break;
      case eCO_ADDSIGNED:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD_SIGNED_EXT);
        break;
      case eCO_MULTIPLYADD:
        break;
      case eCO_REPLACE:
      case eCO_DECAL:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
        break;
      case eCO_BUMPENVMAP:
        break;
      case eCO_DISABLE:
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE);
        break;
    }
    m_eCurAlphaOp[stage] = eAo;
  }

  if (eCa != 255 && eCa != m_eCurColorArg[stage])
  {
    if ((eCa & 7) != (m_eCurColorArg[stage] & 7))
    {
      switch (eCa & 7)
      {
        case eCA_Texture:
        default:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
          break;
        case eCA_Diffuse:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PRIMARY_COLOR_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
          break;
        case eCA_Specular:
          break;
        case eCA_Previous:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
          break;
        case eCA_Constant:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_CONSTANT_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
          break;
      }
    }
    if (((eCa>>3) & 7) != ((m_eCurColorArg[stage]>>3) & 7))
    {
      switch ((eCa >> 3) & 7)
      {
        case eCA_Texture:
        default:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_TEXTURE);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
          break;
        case eCA_Diffuse:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
          break;
        case eCA_Specular:
          break;
        case eCA_Previous:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
          break;
        case eCA_Constant:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
          break;
      }
    }
    if ((eCa >> 6) != (m_eCurColorArg[stage] >> 6))
    {
      switch (eCa >> 6)
      {
        case eCA_Texture:
        default:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_TEXTURE);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
          break;
        case eCA_Diffuse:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PRIMARY_COLOR_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
          break;
        case eCA_Specular:
          break;
        case eCA_Previous:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PREVIOUS_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
          break;
        case eCA_Constant:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_CONSTANT_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
          break;
      }
    }
    m_eCurColorArg[stage] = eCa;
  }

  if (eAa != 255 && eAa != m_eCurAlphaArg[stage])
  {
    if ((eAa & 7) != (m_eCurAlphaArg[stage] & 7))
    {
      switch (eAa & 7)
      {
        case eCA_Texture:
        default:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
          break;
        case eCA_Diffuse:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PRIMARY_COLOR_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
          break;
        case eCA_Specular:
          break;
        case eCA_Previous:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
          break;
        case eCA_Constant:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_CONSTANT_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
          break;
      }
    }
    if (((eAa>>3) & 7) != ((m_eCurAlphaArg[stage]>>3) & 7))
    {
      switch ((eAa >> 3) & 7)
      {
        case eCA_Texture:
        default:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_TEXTURE);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
          break;
        case eCA_Diffuse:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_PRIMARY_COLOR_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
          break;
        case eCA_Specular:
          break;
        case eCA_Previous:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_PREVIOUS_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
          break;
        case eCA_Constant:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_CONSTANT_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_SRC_ALPHA);
          break;
      }
    }
    if ((eAa >> 6) != (m_eCurAlphaArg[stage] >> 6))
    {
      switch (eAa >> 6)
      {
        case eCA_Texture:
        default:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_TEXTURE);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
          break;
        case eCA_Diffuse:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_PRIMARY_COLOR_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
          break;
        case eCA_Specular:
          break;
        case eCA_Previous:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_PREVIOUS_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
          break;
        case eCA_Constant:
          glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_CONSTANT_EXT);
          glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
          break;
      }
    }
    m_eCurAlphaArg[stage] = eAa;
  }
}

//=================================================================================

// Object changing handling (skinning, shadow maps updating, initial states setting, ...)
bool CGLRenderer::EF_ObjectChange(SShader *Shader, SRenderShaderResources *Res, int nObject, CRendElement *pRE)
{
  //PROFILE_FRAME(Objects_Changes);

  CCObject *obj = m_RP.m_VisObjects[nObject];
  if ((obj->m_ObjFlags & FOB_NEAREST) && ((m_RP.m_PersFlags & RBPF_DONTDRAWNEAREST) || CV_r_nodrawnear))
    return false;
  if (Shader)
  {
    //  if (m_RP.m_IgnoreObject && !(Shader->m_Flags & EF_SKY) && m_RP.m_IgnoreObject->m_Trans == obj->m_Trans)
    if (m_RP.m_pIgnoreObject && !(Shader->m_Flags & EF_SKY) && IsEquivalent(m_RP.m_pIgnoreObject->GetTranslation(),obj->GetTranslation()))
      return false;
    if ((m_RP.m_PersFlags & RBPF_ONLYREFRACTED) && !(obj->m_ObjFlags & FOB_REFRACTED))
      return false;
    if ((m_RP.m_PersFlags & RBPF_IGNOREREFRACTED) && (obj->m_ObjFlags & FOB_REFRACTED))
      return false;
  }

  if (obj == m_RP.m_pPrevObject)
    return true;

  m_RP.m_FrameObject++;
  m_RP.m_pCurObject = obj;

  bool bTheSameCM = false;
  if (m_RP.m_pPrevObject->m_ObjFlags & FOB_CUBE_MASK)
  {
    bTheSameCM = (obj && (obj->m_ObjFlags & FOB_CUBE_MASK) == (m_RP.m_pPrevObject->m_ObjFlags & FOB_CUBE_MASK));
    if (!bTheSameCM)
    {
      bool bNeedClear;
      if (obj && (obj->m_ObjFlags & FOB_CUBE_MASK))
        bNeedClear = false;
      else
        bNeedClear = true;
      m_TexMan->EndCubeSide(m_RP.m_pPrevObject, bNeedClear);
    }
  }
  
  int flags = 0;
  if (nObject)
  { 
    m_RP.m_PS.m_NumRendObjects++;

    // Skinning
    if (obj->m_pCharInstance && !CV_r_character_nodeform)
    {
      PROFILE_FRAME(Objects_ChangesSkinning);
      double time0 = 0;
      ticks(time0);
      m_RP.m_PS.m_NumRendSkinnedObjects++;
      CREOcLeaf *pREOCL = (CREOcLeaf *)pRE;
      int nFlags = 0;
      int nVertFormat = -1;
      for (int i=0; i<pREOCL->m_pBuffer->m_pMats->Count(); i++)
      {
        CMatInfo *mi = pREOCL->m_pBuffer->m_pMats->Get(i);
        if (!mi->pRE || !mi->shaderItem.m_pShader)
          continue;
        SShader *pSH = (SShader *)mi->shaderItem.m_pShader->GetTemplate(-1);
        if (pSH->m_Flags3 & EF3_NODRAW)
          continue;
        if (nVertFormat < 0)
          nVertFormat = pSH->m_VertexFormatId;
        else
          nVertFormat = m_RP.m_VFormatsMerge[nVertFormat][pSH->m_VertexFormatId];

        if (Shader->m_Flags & EF_NEEDTANGENTS)
          nFlags |= SHPF_TANGENTS;
        if (mi->shaderItem.m_pShaderResources && mi->shaderItem.m_pShaderResources->m_bNeedNormals)
          nFlags |= SHPF_NORMALS;
      }
      pRE->mfCheckUpdate(nVertFormat, nFlags | FHF_FORANIM);
      CLeafBuffer *pLB = pREOCL->m_pBuffer->GetVertexContainer();
      bool bForceUpdate = (pLB->m_UpdateFrame == GetFrameID());
      if (pLB->m_pVertexBuffer && pLB->m_pVertexBuffer->m_bFenceSet)
        bForceUpdate = true;
      obj->m_pCharInstance->ProcessSkinning(Vec3(zero),obj->m_Matrix, obj->m_nTemplId, obj->m_nLod, bForceUpdate);
      unticks(time0);
      m_RP.m_PS.m_fSkinningTime += (float)(time0*1000.0*g_SecondsPerCycle);
    }

    if(obj->m_ObjFlags & FOB_NEAREST)
      flags |= RBF_NEAREST;

    if ((flags ^ m_RP.m_Flags) & RBF_NEAREST)
    {
      if (flags & RBF_NEAREST)
      {
        glDepthRange(0, 0.1);
        //glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        CCamera Cam = GetCamera();
        m_RP.m_PrevCamera = Cam;
        Cam.SetZMin(0.01f);
        Cam.SetZMax(40.0f);
        
        // set nice fov for weapons  
        Cam.SetFov( Cam.GetFov()*0.6666f ); 
        Cam.Update();
        SetCamera(Cam);
        m_RP.m_Flags |= RBF_NEAREST;
      }
      else
      {
        glDepthRange(m_RP.m_fMinDepthRange, m_RP.m_fMaxDepthRange);
        SetCamera(m_RP.m_PrevCamera);
        m_RP.m_Flags &= ~RBF_NEAREST;
      }
    }

    if ((obj->m_ObjFlags & FOB_CUBE_MASK) && !bTheSameCM)
      m_TexMan->StartCubeSide(obj);
  }
  else
  {
    if (m_RP.m_Flags & RBF_NEAREST)
    {
      glDepthRange(m_RP.m_fMinDepthRange, m_RP.m_fMaxDepthRange);
      SetCamera(m_RP.m_PrevCamera);
      m_RP.m_Flags &= ~RBF_NEAREST;
    }
    m_RP.m_pCurObject->m_Matrix.SetIdentity();
    m_ViewMatrix = m_CameraMatrix;
    if (!Shader || !(Shader->m_Flags & EF_HASVSHADER))
    {
      m_RP.m_PersFlags &= ~RBPF_MATRIXNOTLOADED;
      glLoadMatrixf(&m_ViewMatrix[0][0]);
    }
    else
      m_RP.m_PersFlags |= RBPF_MATRIXNOTLOADED;
  }

  m_RP.m_pPrevObject = obj;

  return true;
}

_declspec(align(16)) static Matrix44 sIdentityMatrix(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1); 

// Get inverted matrix of the object matrix
// All matrices are 16 bytes alligned to speedup matrix calculations using SSE instructions
Matrix44 &CCObject::GetInvMatrix()
{
  if (m_InvMatrixId == 0)
    return sIdentityMatrix;
  if (m_InvMatrixId > 0)
    return m_ObjMatrices[m_InvMatrixId];

  //PROFILE_FRAME(Objects_ObjInvTransform);

  int n = m_ObjMatrices.size();
  m_ObjMatrices.resize(n+1);
  m_InvMatrixId = n;

  CRenderer *rd = gRenDev;
  Matrix44 &m = m_ObjMatrices[m_InvMatrixId];

  if (m_ObjFlags & FOB_TRANS_ROTATE)
  {
    mathMatrixInverse(m.GetData(), m_Matrix.GetData(), g_CpuFlags);
  }
  else
  if (m_ObjFlags & FOB_TRANS_SCALE)
  {
    float fiScaleX = 1.0f / m_Matrix(0,0);
    float fiScaleY = 1.0f / m_Matrix(1,1);
    float fiScaleZ = 1.0f / m_Matrix(2,2);
    m(0,0) = fiScaleX;
    m(0,1) = m_Matrix(0,1);
    m(0,2) = m_Matrix(0,2);
    m(0,3) = m_Matrix(0,3);

    m(1,0) = m_Matrix(1,0);
    m(1,1) = fiScaleY;
    m(1,2) = m_Matrix(1,2);
    m(1,3) = m_Matrix(1,3);

    m(2,0) = m_Matrix(2,0);
    m(2,1) = m_Matrix(2,1);
    m(2,2) = fiScaleZ;
    m(2,3) = m_Matrix(2,3);

    m(3,0) = -m_Matrix(3,0) * fiScaleX;
    m(3,1) = -m_Matrix(3,1) * fiScaleY;
    m(3,2) = -m_Matrix(3,2) * fiScaleZ;
    m(3,3) = m_Matrix(3,3);
  }
  else
  if (m_ObjFlags & FOB_TRANS_TRANSLATE)
  {
    m(0,0) = m_Matrix(0,0);
    m(0,1) = m_Matrix(0,1);
    m(0,2) = m_Matrix(0,2);
    m(0,3) = m_Matrix(0,3);

    m(1,0) = m_Matrix(1,0);
    m(1,1) = m_Matrix(1,1);
    m(1,2) = m_Matrix(1,2);
    m(1,3) = m_Matrix(1,3);

    m(2,0) = m_Matrix(2,0);
    m(2,1) = m_Matrix(2,1);
    m(2,2) = m_Matrix(2,2);
    m(2,3) = m_Matrix(2,3);

    m(3,0) = -m_Matrix(3,0);
    m(3,1) = -m_Matrix(3,1);
    m(3,2) = -m_Matrix(3,2);
    m(3,3) = m_Matrix(3,3);
  }
  else
    m.SetIdentity();

  return m;
}

// Get Project-ModelView matrix for the current object
// This matrix is passed to the vertex shader
// All matrices are 16 bytes alligned to speedup matrix calculations using SSE instructions
Matrix44 &CCObject::GetVPMatrix()
{
  CRenderer *rd = gRenDev;
  if (m_VPMatrixId == 0)
    return rd->m_CameraProjMatrix;
  if (m_VPMatrixId > 0 && m_VPMatrixFrame == rd->m_RP.m_TransformFrame)
    return m_ObjMatrices[m_VPMatrixId];
  m_VPMatrixFrame = rd->m_RP.m_TransformFrame;

  int n = m_ObjMatrices.size();
  m_ObjMatrices.resize(n+1);
  m_VPMatrixId = n;

  Matrix44 &m = m_ObjMatrices[m_VPMatrixId];

  mathMatrixMultiply(m.GetData(), rd->m_CameraProjMatrix.GetData(), m_Matrix.GetData(), g_CpuFlags);
  //D3DXMatrixMultiplyTranspose((D3DXMATRIX *)m.GetData(), (D3DXMATRIX *)m_Matrix.GetData(), (D3DXMATRIX *)rd->m_CameraProjMatrix.GetData());

  return m;
}

// Set scissor rectangle for the current object
void CCObject::SetScissor()
{
  if (CRenderer::CV_r_scissor)
  {
    if (m_nScissorX2)
    {
      int w = m_nScissorX2 - m_nScissorX1;
      int h = m_nScissorY2 - m_nScissorY1;
      gcpOGL->EF_Scissor(true, m_nScissorX1, m_nScissorY1, w, h);
    }
    else
      gcpOGL->EF_Scissor(false, 0, 0, 0, 0);
  }
}

// Sets per-object alpha blending state to fade-in/fade-out objects on the distance
void CCObject::SetAlphaState(CPShader *pPS, int nCurState)
{
  CGLRenderer *rd = gcpOGL;
  // Fake to prevent light coronas from custom blending
  if (!rd->m_RP.m_pRE || rd->m_RP.m_pRE->mfGetType() != eDATA_OcLeaf)
    return;
  if (rd->m_RP.m_pShader->m_Flags2 & EF2_IGNORERESOURCESTATES)
    return;

  rd->m_RP.m_FlagsPerFlush &= ~(RBSI_ALPHABLEND | RBSI_DEPTHWRITE | RBSI_ALPHAGEN);
  if (!(nCurState & GS_BLEND_MASK) && !(rd->m_RP.m_ResourceState & GS_BLEND_MASK))
  {
    int State = rd->m_CurState &= ~GS_BLEND_MASK;
    State |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
    State &= ~GS_DEPTHWRITE;
    rd->EF_SetState(State);
  }
  else
  if (rd->m_CurState & GS_DEPTHFUNC_EQUAL)
    rd->EF_SetState(rd->m_CurState & ~GS_DEPTHFUNC_EQUAL);

  rd->m_RP.m_fCurOpacity = rd->m_RP.m_pCurObject->m_Color.a;
  if (!pPS)
  {
    float fColor = 1.0f;
    int State = rd->m_RP.m_ResourceState & GS_BLEND_MASK;
    if (!State)
      State = nCurState & GS_BLEND_MASK;
    if (State == (GS_BLSRC_DSTCOL | GS_BLDST_ZERO) || State == (GS_BLSRC_ZERO | GS_BLDST_SRCCOL))
    {
      fColor = rd->m_RP.m_fCurOpacity;
      byte bCol = (byte)((1.0f-fColor)*255.0f);
      rd->m_RP.m_NeedGlobalColor.bcolor[0] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[1] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[2] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[3] = (byte)(rd->m_RP.m_fCurOpacity * 255.0f);
      rd->m_RP.m_TexStages[0].m_CO = eCO_ADD;
      rd->m_RP.m_TexStages[0].m_AO = eCO_MODULATE;
      rd->m_RP.m_TexStages[0].m_CA = eCA_Texture | (eCA_Constant<<3);
      rd->m_RP.m_TexStages[0].m_AA = eCA_Texture | (eCA_Constant<<3);
      rd->m_RP.m_FlagsPerFlush |= RBSI_GLOBALRGB;
    }
    else
    if (State == (GS_BLSRC_ONE | GS_BLDST_ONE))
    {
      fColor = rd->m_RP.m_fCurOpacity;
      byte bCol = (byte)(fColor*255.0f);
      rd->m_RP.m_NeedGlobalColor.bcolor[0] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[1] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[2] = bCol;
      rd->m_RP.m_NeedGlobalColor.bcolor[3] = bCol;
      rd->m_RP.m_TexStages[0].m_CO = eCO_MODULATE;
      rd->m_RP.m_TexStages[0].m_AO = eCO_MODULATE;
      rd->m_RP.m_TexStages[0].m_CA = eCA_Texture | (eCA_Constant<<3);
      rd->m_RP.m_TexStages[0].m_AA = eCA_Texture | (eCA_Constant<<3);
      rd->m_RP.m_FlagsPerFlush |= RBSI_GLOBALRGB;
    }
    else
    {
      rd->m_RP.m_NeedGlobalColor.bcolor[3] = (byte)(rd->m_RP.m_fCurOpacity * 255.0f);
      rd->m_RP.m_TexStages[0].m_AO = eCO_MODULATE;
      rd->m_RP.m_TexStages[0].m_AA = eCA_Texture | (eCA_Constant<<3);
      rd->m_RP.m_FlagsPerFlush |= RBSI_GLOBALALPHA;
    }
  }

  rd->m_RP.m_FlagsPerFlush |= RBSI_ALPHABLEND | RBSI_DEPTHWRITE | RBSI_ALPHAGEN;
}

// Set object transform for fixed pipeline shader
void CGLRenderer::EF_SetObjectTransform(CCObject *obj, SShader *pSH, int nTransFlags)
{
  if (nTransFlags & FOB_TRANS_MASK)
    mathMatrixMultiply(m_ViewMatrix.GetData(), m_CameraMatrix.GetData(), obj->m_Matrix.GetData(), g_CpuFlags);
  else
    m_ViewMatrix = m_CameraMatrix;
  if (!pSH || !(pSH->m_Flags & EF_HASVSHADER))
  {
    glLoadMatrixf(m_ViewMatrix.GetData());
    m_RP.m_PersFlags &= ~RBPF_MATRIXNOTLOADED;
  }
  else
    m_RP.m_PersFlags |= RBPF_MATRIXNOTLOADED;
}

// Calculate current scene node matrices
void CGLRenderer::EF_SetCameraInfo()
{
  m_RP.m_ViewOrg = m_cam.GetPos();

  float fm[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, fm);
  m_ViewMatrix = (Matrix44&)fm;
  m_CameraMatrix = m_ViewMatrix;
  Matrix44 m = m_CameraMatrix;
  m.Transpose();
  m_RP.m_PersFlags &= ~RBPF_MATRIXNOTLOADED;

  // Forward
  m_RP.m_CamVecs[0][0] = -m(2,0);
  m_RP.m_CamVecs[0][1] = -m(2,1);
  m_RP.m_CamVecs[0][2] = -m(2,2);

  // Up
  m_RP.m_CamVecs[1][0] = m(1,0);
  m_RP.m_CamVecs[1][1] = m(1,1);
  m_RP.m_CamVecs[1][2] = m(1,2);

  // Right
  m_RP.m_CamVecs[2][0] = m(0,0);
  m_RP.m_CamVecs[2][1] = m(0,1);
  m_RP.m_CamVecs[2][2] = m(0,2);

  m_RP.m_TransformFrame++;
  m_RP.m_FrameObject++;

  glGetFloatv(GL_PROJECTION_MATRIX, &m_ProjMatrix(0,0));
  mathMatrixMultiply(m_CameraProjMatrix.GetData(), m_ProjMatrix.GetData(), m_CameraMatrix.GetData(), g_CpuFlags);
  mathMatrixInverse(m_InvCameraProjMatrix.GetData(), m_CameraProjMatrix.GetData(), g_CpuFlags);

  m_RP.m_PersFlags &= ~RBPF_WASWORLDSPACE;
  m_RP.m_ObjFlags = FOB_TRANS_MASK;
}

//=================================================================================
// Check buffer overflow during geometry batching
void CGLRenderer::EF_CheckOverflow(int nVerts, int nInds, CRendElement *re)
{
  if (m_RP.m_pRE || (m_RP.m_RendNumVerts+nVerts >= m_RP.m_MaxVerts || m_RP.m_RendNumIndices+nInds >= m_RP.m_MaxTris*3))
  {
    m_RP.m_pRenderFunc();
    if (nVerts >= m_RP.m_MaxVerts)
    {
      iLog->Log("CGLRenderer::EF_CheckOverflow: numVerts > MAX (%d > %d)\n", nVerts, m_RP.m_MaxVerts);
      nVerts = m_RP.m_MaxVerts;
    }
    if (nInds >= m_RP.m_MaxTris*3)
    {
      iLog->Log("CGLRenderer::EF_CheckOverflow: numIndices > MAX (%d > %d)\n", nInds, m_RP.m_MaxTris*3);
      nInds = m_RP.m_MaxTris*3;
    }
    EF_Start(m_RP.m_pShader, m_RP.m_pStateShader, m_RP.m_pShaderResources, m_RP.m_pFogVolume ? (m_RP.m_pFogVolume-&m_RP.m_FogVolumes[0]) : 0, re);
  }
}


// Initialize of the new shader pipeline (only 2d)
void CGLRenderer::EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, CRendElement *re) 
{
  m_RP.m_RendPass = 0;
  m_RP.m_RendNumIndices = 0;
  m_RP.m_RendNumVerts = 0;
  m_RP.m_pShader = ef;
  m_RP.m_RendIndices = m_RP.m_SysRendIndices;
#ifdef PIPE_USE_INSTANCING
  m_RP.m_MergedObjects.SetUse(0);
#endif
  m_RP.m_pCurLightMaterial = NULL;
  m_RP.m_pStateShader = efState;
  m_RP.m_pShaderResources = Res;
  m_RP.m_FlagsPerFlush = 0;
  m_RP.m_FlagsModificators = 0;
  m_RP.m_pFogVolume = NULL;
  m_RP.m_pRE = NULL;
  m_RP.m_fCurOpacity = 1.0f;
  SArrayPointer::m_CurEnabled = 0;

  // Choose appropriate shader technique depend on some input parameters
  if (ef->m_HWTechniques.Num())
  {
    int nHW = EF_SelectHWTechnique(ef);
    if (nHW >= 0)
      m_RP.m_pCurTechnique = ef->m_HWTechniques[nHW];
    else
      m_RP.m_pCurTechnique = NULL;
  }
  else
    m_RP.m_pCurTechnique = NULL;

  m_RP.m_CurVFormat = ef->m_VertexFormatId;
  
  SBufInfoTable *pOffs = &gBufInfoTable[m_RP.m_CurVFormat];
  int Size = m_VertexSize[m_RP.m_CurVFormat];    
  m_RP.m_Stride = Size;
  m_RP.m_OffsD  = pOffs->OffsColor;
  m_RP.m_OffsT  = pOffs->OffsTC;
  m_RP.m_OffsN  = pOffs->OffsNormal;
  m_RP.m_NextPtr = m_RP.m_Ptr;

  m_RP.m_Frame++;
}

// Initialize of the new shader pipeline
void CGLRenderer::EF_Start(SShader *ef, SShader *efState, SRenderShaderResources *Res, int numFog, CRendElement *re)
{
  m_RP.m_RendPass = 0;
  m_RP.m_RendNumIndices = 0;
  m_RP.m_RendNumVerts = 0;
  m_RP.m_RendIndices = m_RP.m_SysRendIndices;
  m_RP.m_pShader = ef;
  m_RP.m_pStateShader = efState;
  m_RP.m_pShaderResources = Res;
  m_RP.m_DynLMask = 0;
  m_RP.m_fCurOpacity = 1.0f;
#ifdef PIPE_USE_INSTANCING
  m_RP.m_MergedObjects.SetUse(0);
#endif
  SArrayPointer::m_CurEnabled = 0;
  m_RP.m_FlagsPerFlush = 0;
  m_RP.m_FlagsModificators = 0;
  m_RP.m_pCurLightMaterial = NULL;
  if (numFog && numFog<m_RP.m_FogVolumes.Num() && CV_r_VolumetricFog)
    m_RP.m_pFogVolume = &m_RP.m_FogVolumes[numFog];
  else
    m_RP.m_pFogVolume = NULL;
  m_RP.m_ObjFlags = m_RP.m_pCurObject->m_ObjFlags;

  m_RP.m_CurVFormat = ef->m_VertexFormatId;

  SBufInfoTable *pOffs = &gBufInfoTable[m_RP.m_CurVFormat];
  int Size = m_VertexSize[m_RP.m_CurVFormat];    
  m_RP.m_Stride = Size;
  m_RP.m_OffsD  = pOffs->OffsColor;
  m_RP.m_OffsT  = pOffs->OffsTC;
  m_RP.m_OffsN  = pOffs->OffsNormal;
  m_RP.m_NextPtr = m_RP.m_Ptr;

  m_RP.m_DynLMask = m_RP.m_pCurObject->m_DynLMMask;
  m_RP.m_MergedREs.SetUse(0);
  m_RP.m_MergedObjs.SetUse(0);

  if (!EF_BuildLightsList())
    iLog->Log("WARNING: CGLRenderer::EF_BuildLightsList: Too many light sources per render item (> 16). Shader: '%s'\n", ef->m_Name.c_str());

  // Choose appropriate shader technique depend on some input parameters
  if (ef->m_HWTechniques.Num())
  {
    m_RP.m_pRE = re;
    int nHW = EF_SelectHWTechnique(ef);
    if (nHW >= 0)
      m_RP.m_pCurTechnique = ef->m_HWTechniques[nHW];
    else
      m_RP.m_pCurTechnique = NULL;
  }
  else
    m_RP.m_pCurTechnique = NULL;

  m_RP.m_pRE = NULL;

  m_RP.m_Frame++;
}

//========================================================================================
// Set current HW light states (used in fixed pipeline shaders)
void CGLRenderer::EF_SetHWLight(int Num, vec4_t Pos, CFColor& Diffuse, CFColor& Specular, float ca, float la, float qa)
{
  GLfloat fAmbientLight[] = { 0.0f, 0.0f, 0.0f, 0.0f };

  m_EnableLights |= 1<<Num;

  Num += GL_LIGHT0;

  glLightfv(Num, GL_POSITION, &Pos[0]);
  glLightfv(Num, GL_AMBIENT, &fAmbientLight[0]);
  glLightfv(Num, GL_DIFFUSE, &Diffuse[0]);
  glLightfv(Num, GL_SPECULAR, &Specular[0]);

  glLightf(Num, GL_CONSTANT_ATTENUATION, ca);
  glLightf(Num, GL_LINEAR_ATTENUATION, la);       
  glLightf(Num, GL_QUADRATIC_ATTENUATION, qa);

  m_RP.m_CurrentVLights |= 1<<Num;
}

static float sAttenuation(float Distance, float Radius)
{
  if(Distance <= Radius)
  {
    float A = Distance / Radius;
    float B = (2 * A * A * A - 3 * A * A + 1);

    return B / A * A * 2.0f;
  }
  else
    return 0.0f;
}

inline void TransformPosition33(Vec3& out, Vec3& in, Matrix33& m)
{
  out.x = in.x * m(0,0) + in.y * m(1,0) + in.z * m(2,0);
  out.y = in.x * m(0,1) + in.y * m(1,1) + in.z * m(2,1);
  out.z = in.x * m(0,2) + in.y * m(1,2) + in.z * m(2,2);
}

// Calculate light parameters used for HW vertex lighting (used in fixed pipeline shaders)
bool CGLRenderer::EF_SetLights(int Flags)
{
  vec4_t Pos;
  Vec3d lpos;
  int n = 0;
  float fCA, fLA, fQA;
  Vec3d vCenterRE;
  float fRadRE;
  bool bCalcDist = false;
  CFColor cDiffuse, cSpecular;
  cSpecular = CFColor(0.0f);
  for (int i=0; i<m_RP.m_NumActiveDLights; i++)
  {
    if (i >= 8)
      break;
    CDLight *dl = m_RP.m_pActiveDLights[i];
    if ((Flags & LMF_IGNOREPROJLIGHTS) && (dl->m_Flags & DLF_PROJECT))
      continue;
    bool bSpecularOnly = false;
    int nlM;
    if (nlM=(Flags & LMF_LIGHT_MASK))
    {
      if ((nlM >> LMF_LIGHT_SHIFT) != i)
        continue;
    }
    if (dl->m_Flags & DLF_LM)
    {
      if ((m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP]) || m_RP.m_pCurObject->m_nLMId)
      {
        if (Flags & LMF_NOSPECULAR)
          continue;
        bSpecularOnly = true;
      }
    }

    // some statistics
    if (!(m_RP.m_StatLightMask & (1<<i)))
    {
      m_RP.m_StatLightMask |= (1<<i);
      m_RP.m_StatNumLights++;
    }
    Matrix44 m = m_RP.m_pCurObject->GetInvMatrix();
    TransformPosition(lpos, dl->m_Origin, m);
    //TransformPosition33(lpos, dl->m_Origin-m_RP.m_pCurObject->m_Matrix.GetTranslationOLD(), Matrix33(GetTransposed44(m_RP.m_pCurObject->m_Matrix)));
    if (dl->m_Flags & DLF_DIRECTIONAL)
    {
      lpos.Normalize();
      Pos[3] = 0.0f;
      fCA = 1.0f;
      fLA = 0.0f;
      fQA = 0.0f;
      cDiffuse = dl->m_Color * 1.5f;
    }
    else
    {
      Pos[3] = 1.0f;
      if (m_RP.m_pRE && !bCalcDist)
      {
        Vec3d vMins, vMaxs;
        bCalcDist = true;
        m_RP.m_pRE->mfGetBBox(vMins, vMaxs);
        vCenterRE = (vMins + vMaxs) * 0.5f;
        vCenterRE += m_RP.m_pCurObject->GetTranslation();
        fRadRE = (vMaxs - vMins).Length() * 0.5f;
      }
      else
      {
        fRadRE = 1000.0f;
        vCenterRE = Vec3(0,0,0);
      }
      float fDist = max(0.1f, (vCenterRE - dl->m_Origin).Length());
      float fMaxDist = CLAMP(fDist + fRadRE, dl->m_fRadius * 0.1f, dl->m_fRadius * 0.99f);
      float fMinDist = CLAMP(fDist - fRadRE, dl->m_fRadius * 0.1f, dl->m_fRadius * 0.99f);
      float fMinAtt = 1.0f / sAttenuation(fMinDist, dl->m_fRadius);
      float fMaxAtt = 1.0f / sAttenuation(fMaxDist, dl->m_fRadius);
      if(fabsf(fMinAtt - fMaxAtt) < 0.00001f)
      {
        fCA = fMinAtt;
        fLA = 0.0f;
      }
      else
      {
        fCA = max(0.01f, fMinAtt - (fMaxAtt - fMinAtt) / (fMaxDist - fMinDist) * fMinDist);
        fLA = max(0.0f, (fMinAtt - fCA) / fMinDist);
      }
      fQA = 0.0f;
      cDiffuse = dl->m_Color; // * 1.25f;
    }
    fCA = max(0.0f, fCA);
    fLA = max(0.0f, fLA);
    fQA = max(0.0f, fQA);
    Pos[0] = lpos.x;
    Pos[1] = lpos.y;
    Pos[2] = lpos.z;
    if (!(Flags & LMF_NOSPECULAR))
      cSpecular = dl->m_SpecColor;
    if (bSpecularOnly)
      cDiffuse = Col_Black;
    cDiffuse.a = 1.0f;
    EF_SetHWLight(n, Pos,
                 cDiffuse,
                 cSpecular,
                 fCA, fLA, fQA);
    n++;
  }

  return true;
}

// Initialize HW vertex lighting states for the fixed pipeline shader
void CGLRenderer::EF_LightMaterial(SLightMaterial *lm, int Flags)
{
  if (!(m_RP.m_pShader->m_Flags & EF_NEEDNORMALS))
    return;
  if (m_RP.m_ObjFlags & FOB_FOGPASS)
    return;

  //PROFILE_FRAME(State_LightStates);

  // Use fake lighting with TFactor
  if (!m_RP.m_NumActiveDLights)
  {
    EF_ConstantLightMaterial(lm, Flags);
    return;
  }

  if (!(Flags & LMF_IGNORELIGHTS) && EF_SetLights(Flags))
  {
    if (!(m_RP.m_CurrentVLights & 0xff))
    {
      EF_ConstantLightMaterial(lm, Flags);
      return;
    }
    m_RP.m_FlagsPerFlush &= ~(RBSI_GLOBALRGB | RBSI_GLOBALALPHA);

    CFColor colAmb = EF_GetCurrentAmbient(lm, Flags);
    CFColor colDif = EF_GetCurrentDiffuse(lm, Flags);
    CFColor colSpec = lm->Front.m_Specular;
    float fPow = lm->Front.m_SpecShininess;
    if (Flags & LMF_NOSPECULAR)
    {
      fPow = 0;
      colSpec = Col_Black;
    }

    glMaterialfv(GL_FRONT, GL_AMBIENT, &colAmb[0]);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, &colDif[0]);
    glMaterialfv(GL_FRONT, GL_SPECULAR, &colSpec[0]);
    glMaterialf(GL_FRONT, GL_SHININESS, fPow);
  }
  m_RP.m_TexStages[0].m_CA = eCA_Texture | (eCA_Diffuse<<3);
  m_RP.m_TexStages[0].m_AA = eCA_Texture | (eCA_Diffuse<<3);

  m_RP.m_CurrentVLights |= 0x80000000;
}

//===================================================================================================
// Calculate matrices (usually texture matrices) for fixed pipeline shaders
// Set texture transform modes
void CGLRenderer::EF_ApplyMatrixOps(TArray<SMatrixTransform>* MatrixOps, bool bEnable)
{
  if (!MatrixOps)
    return;
  
  int CurMatrix = GL_MODELVIEW;
  int PrevStage = m_TexMan->m_CurStage;
  int PrevStage1 = PrevStage;

  for (int i=0; i<MatrixOps->Num(); i++)
  {
    SMatrixTransform *mt = &MatrixOps->Get(i);
    if (mt->m_Stage != PrevStage)
    {
      PrevStage = mt->m_Stage;
      EF_SelectTMU(mt->m_Stage);
    }
    if (mt->m_Matrix != CurMatrix)
    {
      glMatrixMode(mt->m_Matrix);
      if (!bEnable && mt->m_Matrix != GL_PROJECTION && mt->m_Matrix != GL_MODELVIEW)
        glLoadIdentity();
      CurMatrix = mt->m_Matrix;
    }
    if (mt->m_Stage != PrevStage)
    {
      PrevStage = mt->m_Stage;
      EF_SelectTMU(mt->m_Stage);
      if (!bEnable && mt->m_Matrix != GL_PROJECTION && mt->m_Matrix != GL_MODELVIEW)
        glLoadIdentity();
    }
    mt->mfSet(bEnable);
  }
  EF_SelectTMU(PrevStage1);
  if (CurMatrix != GL_MODELVIEW)
    glMatrixMode(GL_MODELVIEW);
}

// Set/Restore shader resources overrided states
bool CGLRenderer::EF_SetResourcesState(bool bSet)
{
  if (m_RP.m_pShader->m_Flags2 & EF2_IGNORERESOURCESTATES)
    return true;

  bool bRes = true;
  if (bSet)
  {
    if (m_RP.m_pShaderResources->m_ResFlags & MTLFLAG_2SIDED)
    {
      GLSetCull(eCULL_None);
      m_RP.m_FlagsPerFlush |= RBSI_NOCULL;
    }
    if (m_RP.m_pShaderResources->m_AlphaRef)
    {
      if (!(m_CurState & GS_ALPHATEST_MASK))
        glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GEQUAL, m_RP.m_pShaderResources->m_AlphaRef);
      m_RP.m_FlagsPerFlush |= RBSI_ALPHATEST;
    }
    else
    if (m_RP.m_pShaderResources->m_Opacity != 1.0f)
    {
      int State = m_CurState &= ~GS_BLEND_MASK;
      byte bCol = (byte)(m_RP.m_pShaderResources->m_Opacity*255.0f);
      if (m_RP.m_pShaderResources->m_ResFlags & MTLFLAG_ADDITIVE)
      {
        State |= GS_BLSRC_ONE | GS_BLDST_ONE;
        State &= ~GS_DEPTHWRITE;
        m_RP.m_FlagsPerFlush |= RBSI_GLOBALRGB;
        m_RP.m_NeedGlobalColor.bcolor[0] = bCol;
        m_RP.m_NeedGlobalColor.bcolor[1] = bCol;
        m_RP.m_NeedGlobalColor.bcolor[2] = bCol;
      }
      else
      {
        State |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
        State &= ~GS_DEPTHWRITE;
        m_RP.m_FlagsPerFlush |= RBSI_GLOBALALPHA;
        m_RP.m_NeedGlobalColor.bcolor[3] = bCol;
      }
      EF_SetState(State);
      m_RP.m_ResourceState = State;

      m_RP.m_fCurOpacity = m_RP.m_pShaderResources->m_Opacity;

      m_RP.m_FlagsPerFlush |= RBSI_ALPHABLEND | RBSI_DEPTHWRITE | RBSI_ALPHAGEN;
      m_RP.m_ObjFlags &= ~FOB_LIGHTPASS;
    }
  }
  else
  {
    if (m_RP.m_pShaderResources->m_AlphaRef)
    {
      switch (m_CurState & GS_ALPHATEST_MASK)
      {
        case GS_ALPHATEST_GREATER0:
          glEnable(GL_ALPHA_TEST);
          glAlphaFunc(GL_GREATER, 0);
          break;

        case GS_ALPHATEST_LESS128:
          glEnable(GL_ALPHA_TEST);
          glAlphaFunc(GL_LESS, 0.5f);
          break;

        case GS_ALPHATEST_GEQUAL128:
          glEnable(GL_ALPHA_TEST);
          glAlphaFunc(GL_GEQUAL, 0.5f);
          break;

        default:
          glDisable(GL_ALPHA_TEST);
          break;
      }
    }
  }
  return true;
}

// Set overrided shader states from State shader
void CGLRenderer::EF_SetStateShaderState()
{
  SShader *ef = m_RP.m_pStateShader;
  SEfState *st = ef->m_State;

  if (st->m_bClearStencil)
  {
    if (CV_r_measureoverdraw)
    {
      iLog->Log("Stencil shadows and overdraw measurement are mutually exclusive\n");
      CV_r_measureoverdraw = 0;
    }
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
  }
  if (st->m_Stencil)
  {
    st->m_Stencil->mfSet();
  }
  if (st->m_Flags & ESF_POLYLINE)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  if (st->m_Flags & ESF_NOCULL)
  {
    GLSetCull(eCULL_None);
    m_RP.m_FlagsPerFlush |= RBSI_NOCULL;
  }
  else
  if (st->m_Flags & ESF_CULLFRONT)
  {
    GLSetCull(eCULL_Front);
    m_RP.m_FlagsPerFlush |= RBSI_NOCULL;
  }

  if (st->m_Flags & ESF_STATE)
  {
    EF_SetState(st->m_State);
    if (st->m_State & GS_NODEPTHTEST)
      m_RP.m_FlagsPerFlush |= RBSI_DEPTHTEST;
    if (st->m_State & (GS_DEPTHFUNC_EQUAL | GS_DEPTHFUNC_GREAT))
      m_RP.m_FlagsPerFlush |= RBSI_DEPTHFUNC;
  }
  bool bSetCol = false;
  UCol Col;
  Col.dcolor = 0;
  if (st->m_Flags & ESF_RGBGEN)
  {
    m_RP.m_FlagsPerFlush |= RBSI_RGBGEN;
    switch (st->m_eEvalRGB)
    {
      case eERGB_Fixed:
        Col.bcolor[0] = st->m_FixedColor[0];
        Col.bcolor[1] = st->m_FixedColor[1];
        Col.bcolor[2] = st->m_FixedColor[2];
        Col.bcolor[3] = st->m_FixedColor[3];
        break;
    }
    bSetCol = true;
  }
  if (st->m_Flags & ESF_ALPHAGEN)
  {
    m_RP.m_FlagsPerFlush |= RBSI_ALPHAGEN;
    switch (st->m_eEvalAlpha)
    {
      case eEALPHA_Fixed:
        Col.bcolor[3] = st->m_FixedColor[3];
        break;
    }
    bSetCol = true;
  }
  if (bSetCol)
  {
    Exchange(Col.bcolor[0], Col.bcolor[3]);
    m_RP.m_NeedGlobalColor = Col;
  }
  if (st->m_Flags & ESF_COLORMASK)
  {
    m_RP.m_FlagsPerFlush |= RBSI_COLORMASK;
    glColorMask(st->m_ColorMask[0], st->m_ColorMask[1], st->m_ColorMask[2], st->m_ColorMask[3]);
  }
}

// Reset overrided shader states from State shader
void CGLRenderer::EF_ResetStateShaderState()
{
  SShader *ef = m_RP.m_pStateShader;
  SEfState *st = ef->m_State;

  if (st->m_Flags & ESF_COLORMASK)
    glColorMask(1, 1, 1, 1);

  if (st->m_Flags & ESF_POLYLINE)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


#include "../Common/NvTriStrip/NVTriStrip.h"

// Used for HW effectors for rendering of tri mesh (vertex array)
void CGLRenderer::EF_DrawIndexedMesh (int nPrimType)
{
  //PROFILE_FRAME(Draw_IndexMesh);

  int nType = -1;
  switch (nPrimType)
  {
    case R_PRIMV_TRIANGLES:
      nType = GL_TRIANGLES;
      m_nPolygons += (m_RP.m_RendNumIndices / 3);
      break;

    case R_PRIMV_TRIANGLE_STRIP:
      nType = GL_TRIANGLE_STRIP;
      m_nPolygons += (m_RP.m_RendNumIndices - 2);
      break;

    case R_PRIMV_TRIANGLE_FAN:
      nType = GL_TRIANGLE_FAN;
      m_nPolygons += (m_RP.m_RendNumIndices - 2);
      break;

    case R_PRIMV_QUADS:
      nType = GL_QUADS;
      m_nPolygons += (m_RP.m_RendNumIndices / 2);
      break;
      
    case R_PRIMV_MULTI_STRIPS:
      {
        list2<CMatInfo> *mats = m_RP.m_pRE->mfGetMatInfoList();
        if (mats)
        {
          CMatInfo *m = mats->Get(0);
          //assert(m_RP.m_RendIndices[0]<60000);
          for (int i=0; i<mats->Count(); i++, m++)
          {
            glDrawElements(GL_TRIANGLE_STRIP, m->nNumIndices, GL_UNSIGNED_SHORT, &m_RP.m_RendIndices[m->nFirstIndexId]);
            m_nPolygons += (m->nNumIndices - 2);
          }
        }
        return;
      }

    case R_PRIMV_MULTI_GROUPS:
      {
        CMatInfo *mi = m_RP.m_pRE->mfGetMatInfo();
        if (mi)
        {
          for (int i=0; i<mi->m_dwNumSections; i++)
          {
            SPrimitiveGroup *g = &mi->m_pPrimitiveGroups[i];
            switch (g->type)
            {
              case PT_STRIP:
                glDrawElements(GL_TRIANGLE_STRIP, g->numIndices, GL_UNSIGNED_SHORT, &m_RP.m_RendIndices[g->offsIndex]);
                break;

              case PT_LIST:
                glDrawElements(GL_TRIANGLES, g->numIndices, GL_UNSIGNED_SHORT, &m_RP.m_RendIndices[g->offsIndex]);
                break;

              case PT_FAN:
                glDrawElements(GL_TRIANGLE_FAN, g->numIndices, GL_UNSIGNED_SHORT, &m_RP.m_RendIndices[g->offsIndex]);
                break;
            }
            m_nPolygons += g->numTris;
          }
        }
        return;
      }
      break;
      
    default:
      iLog->Log("WARNING: CGLRenderer::EF_DrawIndexedMesh: Unknown primitive type %d\n");
      return;
  }

  if (nType >= 0)
  {
    if(m_RP.m_RendNumIndices)
      glDrawElements(nType, m_RP.m_RendNumIndices, GL_UNSIGNED_SHORT, m_RP.m_RendIndices);
    else
    {
      static int tic=10; // variable will be removed when this problem will be fixed
      if(tic>0)
      {
        iLog->Log("CGLRenderer::EF_DrawIndexedMesh: m_RP.mRendNumIndices (Shader: '%s')", m_RP.m_pShader->GetName());
        tic--;
      }
    }
  }
}

bool CGLRenderer::EF_PreDraw(SShaderPass *slw)
{
  bool bRet = true;

  if (!m_RP.m_pRE && m_RP.m_RendNumVerts && m_RP.m_RendNumIndices)
  {
    // Mergable geometry (no single render element)
    int nStart;
    int nCurVB;
    int nSize = m_RP.m_Stride*m_RP.m_RendNumVerts;
    if (!(m_RP.m_FlagsPerFlush & RBSI_VERTSMERGED))
    {
      m_RP.m_FlagsPerFlush |= RBSI_VERTSMERGED;
      int nFormat = m_RP.m_CurVFormat;
      nCurVB = m_RP.m_CurVB;
      if (VB_BytesOffset(nCurVB)+nSize >= VB_BytesCount(nCurVB))
      {
        VB_Reset(nCurVB);
        m_RP.m_CurVB = (nCurVB + 1) & (MAX_DYNVBS-1);
      }
      nCurVB = m_RP.m_CurVB;
      byte *pVB = VB_Lock(nCurVB, nSize, nStart);
      cryMemcpy(pVB, m_RP.m_Ptr.Ptr, nSize);
      VB_Unlock(nCurVB);
      m_RP.m_FirstVertex = 0;
      m_RP.m_MergedStreams[0] = nCurVB;
      m_RP.m_nStreamOffset[0] = nStart;
      if (SUPPORTS_GL_ARB_vertex_buffer_object)
      {
        // Update indices
        int sizeI = m_RP.m_RendNumIndices*sizeof(ushort);
        int nOffsetI;
        if (sizeI+m_RP.m_IBDynOffs > m_RP.m_IBDynSize)
        {
          nOffsetI = 0;
          m_RP.m_IBDynOffs = sizeI;
        }
        else
        {
          nOffsetI = m_RP.m_IBDynOffs;
          m_RP.m_IBDynOffs += sizeI;
        }
        {
          PROFILE_FRAME(Mesh_UpdateIBuffersDynMerge);
          glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_RP.m_IBDyn.m_VertBuf.m_nID);
          glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, nOffsetI, sizeI, m_RP.m_SysRendIndices);
        }
        m_RP.m_RendIndices = (ushort *)(nOffsetI);
        m_RP.m_FlagsPerFlush |= RBSI_INDEXSTREAM;
      }
    }
    VB_Bind(0);

    if (m_RP.m_FlagsModificators & FHF_TANGENTS)
    {
      if (!(m_RP.m_FlagsPerFlush & RBSI_TANGSMERGED))
      {
        m_RP.m_FlagsPerFlush |= RBSI_TANGSMERGED;
        int i;
        int nCurVB = m_RP.m_CurVB;
        int nSize = m_RP.m_RendNumVerts*sizeof(SPipTangents);
        if (VB_BytesOffset(nCurVB)+nSize >= VB_BytesCount(nCurVB))
        {
          VB_Reset(nCurVB);
          m_RP.m_CurVB = (nCurVB + 1) & (MAX_DYNVBS-1);
        }
        nCurVB = m_RP.m_CurVB;
        SPipTangents *dst = (SPipTangents *)VB_Lock(nCurVB, nSize, nStart);
        for (i=0; i<m_RP.m_MergedREs.Num(); i++)
        {
          CREOcLeaf *re = m_RP.m_MergedREs[i];
          CCObject *obj = m_RP.m_MergedObjs[i];
          CLeafBuffer *lb = re->m_pBuffer;
          CMatInfo *mi = re->m_pChunk;
          SPipTangents *src = (SPipTangents *)lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
          src += mi->nFirstVertId;
          int nVerts = mi->nNumVerts;
          if (obj->m_ObjFlags & FOB_TRANS_ROTATE)
          {
            Matrix44 *mat = &obj->m_Matrix;
#ifdef DO_ASM
            _asm
            {
              mov ecx,    nVerts;
              mov eax,    mat
              mov esi,    src
              mov edi,    dst
              movaps      xmm2,xmmword ptr [eax]
              movaps      xmm4,xmmword ptr [eax+10h]
              movaps      xmm6,xmmword ptr [eax+20h]
align 16
_Loop:
              movlps      xmm1,qword ptr [esi]
              movss       xmm0,dword ptr [esi+8]
              shufps      xmm0,xmm0,0
              prefetcht0  [esi+10h] 
              mulps       xmm0,xmm6
              movaps      xmm3,xmm1
              shufps      xmm3,xmm1,55h
              mulps       xmm3,xmm4
              shufps      xmm1,xmm1,0
              mulps       xmm1,xmm2
              addps       xmm3,xmm1
              addps       xmm3,xmm0
              movhlps     xmm5,xmm3
              movlps      qword ptr [edi],xmm3
              movss       xmm0,dword ptr [esi+20]
              movss       dword ptr [edi+8],xmm5

              movlps      xmm1,qword ptr [esi+12]
              shufps      xmm0,xmm0,0
              mulps       xmm0,xmm6
              prefetcht0  [esi+20h] 
              movaps      xmm3,xmm1
              shufps      xmm3,xmm1,55h
              mulps       xmm3,xmm4
              shufps      xmm1,xmm1,0
              mulps       xmm1,xmm2
              addps       xmm3,xmm1
              addps       xmm3,xmm0
              movhlps     xmm5,xmm3
              movlps      qword ptr [edi+12],xmm3
              movss       xmm0,dword ptr [esi+32]
              movss       dword ptr [edi+20],xmm5

              movlps      xmm1,qword ptr [esi+24]
              shufps      xmm0,xmm0,0
              prefetcht0  [esi+30h] 
              mulps       xmm0,xmm6
              movaps      xmm3,xmm1
              shufps      xmm3,xmm1,55h
              mulps       xmm3,xmm4
              shufps      xmm1,xmm1,0
              mulps       xmm1,xmm2
              addps       xmm1,xmm3
              addps       xmm0,xmm1
              add         esi, 36
              movhlps     xmm1,xmm0
              movlps      qword ptr [edi+24],xmm0
              movss       dword ptr [edi+32],xmm1

              add         edi, 36
              dec         ecx
              jne         _Loop
              mov         dst, edi
            }
#else
            for (int j=0; j<mi->nNumVerts; j++)
            {
              dst->m_Tangent = mat->TransformVectorOLD(src->m_Tangent);
              dst->m_Binormal = mat->TransformVectorOLD(src->m_Binormal);
              dst->m_TNormal = mat->TransformVectorOLD(src->m_TNormal);
              src++;
              dst++;
            }
#endif
          }
          else
          {
            for (i=0; i<mi->nNumVerts; i++)
            {
              *dst = *src;
              src++;
              dst++;
            }
          }
        }
        VB_Unlock(nCurVB);
        m_RP.m_MergedStreams[1] = nCurVB;
        m_RP.m_nStreamOffset[1] = nStart;
      }
      VB_Bind(1);
    }

    if (m_RP.m_FlagsModificators & RBMF_LMTCUSED)
    {
      if (!(m_RP.m_FlagsPerFlush & RBSI_LMTCMERGED))
      {
        m_RP.m_FlagsPerFlush |= RBSI_LMTCMERGED;
        nSize = sizeof(struct_VERTEX_FORMAT_TEX2F)*m_RP.m_RendNumVerts;
        int nCurVB = m_RP.m_CurVB;
        if (VB_BytesOffset(nCurVB)+nSize >= VB_BytesCount(nCurVB))
        {
          VB_Reset(nCurVB);
          m_RP.m_CurVB = (nCurVB + 1) & (MAX_DYNVBS-1);
        }
        nCurVB = m_RP.m_CurVB;
        struct_VERTEX_FORMAT_TEX2F *dst = (struct_VERTEX_FORMAT_TEX2F *)VB_Lock(nCurVB, nSize, nStart);
        for (int i=0; i<m_RP.m_MergedREs.Num(); i++)
        {
          CREOcLeaf *re = m_RP.m_MergedREs[i];
          CCObject *pObj = m_RP.m_MergedObjs[i];
          int nNumVerts = re->m_pChunk->nNumVerts;
          if (!pObj->m_pLMTCBufferO)
          {
            dst += nNumVerts;
            continue;
          }
          struct_VERTEX_FORMAT_TEX2F *src = (struct_VERTEX_FORMAT_TEX2F *)pObj->m_pLMTCBufferO->m_pSecVertBuffer->m_VS[0].m_VData;
          src += re->m_pChunk->nFirstVertId;
          for (int n=0; n<nNumVerts; n++)
          {
            *dst++ = *src++;
          }
        }
        VB_Unlock(nCurVB);
        m_RP.m_MergedStreams[2] = nCurVB;
        m_RP.m_nStreamOffset[2] = nStart;
      }
      VB_Bind(2);
    }
  }
  else
  if (m_RP.m_pRE)
    bRet = m_RP.m_pRE->mfPreDraw(slw);

  EF_CommitPS();
  EF_CommitVS();
  EF_CommitStreams();
  EF_CommitVLights();

  return bRet;
}

// Draw detail textures passes (used in FP shaders and in programmable pipeline shaders)
void CGLRenderer::EF_DrawDetailOverlayPasses()
{
  // Usually it means first pass in indoor engine (before shadow pass)
  if (m_RP.m_ObjFlags & FOB_ZPASS)
    return;

  if (!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY])
    return;

  SEfResTexture *rt = m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY];
  SShader *sh = m_RP.m_pShader;
  int i;

  SParamComp_FogMatrix FM;
  vec4_t Vals;
  float fDistToCam = 500.0f;
  float fDist = CV_r_detaildistance;
  static TArray<bool> sbNeedRender;
  if (m_RP.m_pRE)
  {
#ifndef PIPE_USE_INSTANCING
    fDistToCam = m_RP.m_pRE->mfMinDistanceToCamera(m_RP.m_pCurObject);
    if (fDistToCam > fDist+1.0f)
      return;
#else
    CCObject *pObj = gRenDev->m_RP.m_pCurObject;
    int nObj = 0;
    float fDistToCam = 999999.0f;
    sbNeedRender.SetUse(0);
    while (true)
    {
      float fDistObj = m_RP.m_pRE->mfMinDistanceToCamera(pObj);
      if (fDistObj <= fDist+1.0f)
        sbNeedRender.AddElem(true);
      else
        sbNeedRender.AddElem(false);
      fDistToCam = min(fDistToCam, fDistObj);
      nObj++;
      if (nObj >= gRenDev->m_RP.m_MergedObjects.Num())
        break;
      pObj = gRenDev->m_RP.m_MergedObjects[nObj];
    }
    if (fDistToCam > fDist+1.0f)
      return;
    glPushMatrix();
#endif
  }
  else
    return;

  PROFILE_FRAME(DrawShader_DetailPasses);

  CGLTexMan::BindNULL(2);
  EF_SelectTMU(1);
  gRenDev->m_TexMan->m_Text_Fog->Set();
  EnableTMU(true);
  EF_SelectTMU(0);
  rt->m_TU.m_TexPic->Set();

  if (m_RP.m_FlagsPerFlush & RBSI_WASDEPTHWRITE)
    EF_SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL | GS_DEPTHFUNC_EQUAL);
  else
    EF_SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL);

  SMFog fg;
  SMFog *fb = m_RP.m_pFogVolume;
  fg.m_FogInfo.m_WaveFogGen.m_eWFType = eWF_None;
  m_RP.m_pFogVolume = &fg;
  m_RP.m_CurrentVLights = 0;
  m_RP.m_FlagsModificators &= ~(7 | (RBMF_TCM | RBMF_TCG));
  EF_Scissor(false, 0, 0, 0, 0);

  if (!(sh->m_Flags & EF_HASVSHADER))
  {
    m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;

    SArrayPointer_Vertex vpv;
    SArrayPointer_Texture vpt;
    vpv.ePT = eSrcPointer_Vert;
    vpv.eDst = eDstPointer_Vert;
    vpv.NumComponents = 3;
    vpv.Type = GL_FLOAT;
    vpv.mfSet(1);

    vpt.ePT = eSrcPointer_Tex;
    vpt.eDst = eDstPointer_Tex0;
    vpt.NumComponents = 2;
    vpt.Type = GL_FLOAT;
    vpt.mfSet(1);

    int n = CLAMP(CV_r_detailnumlayers, 1, 4);
    float fUScale = rt->m_TexModificator.m_Tiling[0];
    float fVScale = rt->m_TexModificator.m_Tiling[1];
    if (!fUScale)
      fUScale = CV_r_detailscale;
    if (!fVScale)
      fVScale = CV_r_detailscale;

    for (i=0; i<n; i++)
    {
      fg.m_fMaxDist = fDist;

      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glScalef(fUScale, fVScale, 1.0f);
      glMatrixMode(GL_MODELVIEW);

#ifndef PIPE_USE_INSTANCING
      EF_SelectTMU(1);
      FM.m_Offs = 0;
      FM.mfGet4f(Vals);
      glTexGenfv(GL_S, GL_OBJECT_PLANE, Vals);
      FM.m_Offs = 1;
      FM.mfGet4f(Vals);
      glTexGenfv(GL_T, GL_OBJECT_PLANE, Vals);

      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);

      EF_SelectTMU(0);

      if (!m_RP.m_RCDetail)
        m_RP.m_RCDetail = CPShader::mfForName("CGRCDetailAtten", true);
      if (m_RP.m_RCDetail)
        m_RP.m_RCDetail->mfSet(true);

      // Draw primitives
      EF_Draw(sh, NULL);
#else
      int bFogOverrided = 0;
      EF_PreDraw(NULL);
      if (m_FS.m_bEnable)
        bFogOverrided = EF_FogCorrection(false, false);
      if (m_Features & (RFT_HW_RC | RFT_HW_PS20))
      {
        if (!m_RP.m_RCDetail)
          m_RP.m_RCDetail = CPShader::mfForName("CGRCDetailAtten");
        if (m_RP.m_RCDetail)
          m_RP.m_RCDetail->mfSet(true);
        EF_CommitPS();
      }
      else
      {
        m_RP.m_CurGlobalColor.dcolor = 0xff808080;
        EF_SelectTMU(0);
        EF_SetColorOp(eCO_REPLACE, eCO_REPLACE, DEF_TEXARG0, DEF_TEXARG0);
        EF_SelectTMU(1);
        EF_SetColorOp(eCO_BLENDTEXTUREALPHA, eCO_REPLACE, eCA_Constant|(eCA_Previous<<3), DEF_TEXARG0);
      }

      CCObject *pSaveObj = m_RP.m_pCurObject;
      CCObject *pObj = pSaveObj;
      int nObj = 0;
      while (true)
      {      
        if (sbNeedRender[nObj])
        {
          if (nObj)
          {
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            m_RP.m_FrameObject++;
            pObj = m_RP.m_pCurObject;
            EF_SetObjectTransform(pObj, sh, pObj->m_ObjFlags);
          }

          EF_SelectTMU(1);
          FM.m_Offs = 0;
          FM.mfGet4f(Vals);
          glTexGenfv(GL_S, GL_OBJECT_PLANE, Vals);
          FM.m_Offs = 1;
          FM.mfGet4f(Vals);
          glTexGenfv(GL_T, GL_OBJECT_PLANE, Vals);

          glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
          glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
          glEnable(GL_TEXTURE_GEN_S);
          glEnable(GL_TEXTURE_GEN_T);

          EF_SelectTMU(0);

          {
            //PROFILE_FRAME(Draw_EFIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(sh, NULL);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
        }
        nObj++;
        if (nObj >= m_RP.m_MergedObjects.Num())
          break;
      }
      EF_FogCorrectionRestore(bFogOverrided);
      if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
        EF_CommitTexTransforms(false);
      if (pSaveObj != m_RP.m_pCurObject)
      {
        m_RP.m_pCurObject = pSaveObj;
        m_RP.m_FrameObject++;
        EF_SetObjectTransform(m_RP.m_pCurObject, sh, pSaveObj->m_ObjFlags);
      }
#endif

      fDist /= 2.0f;
      if (fDistToCam > fDist+1.0f)
        break;
      fUScale *= 2.0f;
      fVScale *= 2.0f;
    }

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    EF_SelectTMU(1);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    EF_SelectTMU(0);
  }
  else
  {
    float param[4];
    if (!m_RP.m_VPDetail)
      m_RP.m_VPDetail = CVProgram::mfForName("CGVProgDetail");
    if (!m_RP.m_RCDetail)
      m_RP.m_RCDetail = CPShader::mfForName("CGRCDetailAtten");
    if (m_RP.m_RCDetail)
      m_RP.m_RCDetail->mfSet(true);

    CCGVProgram_GL *vpGL = (CCGVProgram_GL *)m_RP.m_VPDetail;
    vpGL->mfSet(true, NULL, VPF_DONTSETMATRICES | VPF_SETPOINTERSFORSHADER);
    vpGL->mfSetVariables(false, NULL);
    SCGBind *pBindScale = vpGL->mfGetParameterBind("DetailScaling");
    SCGBind *pBindTG0 = vpGL->mfGetParameterBind("TexGen00");
    SCGBind *pBindTG1 = vpGL->mfGetParameterBind("TexGen01");

    int n = CLAMP(CV_r_detailnumlayers, 1, 4);
    float fUScale = rt->m_TexModificator.m_Tiling[0];
    float fVScale = rt->m_TexModificator.m_Tiling[1];
    if (!fUScale)
      fUScale = CV_r_detailscale;
    if (!fVScale)
      fVScale = CV_r_detailscale;
    for (i=0; i<n; i++)
    {
      fg.m_fMaxDist = fDist;

      param[0] = fUScale;
      param[1] = fVScale;
      param[2] = 0;
      param[3] = 0;
      if (pBindScale)
        vpGL->mfParameter4f(pBindScale, param);

#ifndef PIPE_USE_INSTANCING
      FM.m_Offs = 0;
      FM.mfGet4f(Vals);
      if (pBindTG0)
        vpGL->mfParameter4f(pBindTG0, Vals);
      FM.m_Offs = 1;
      FM.mfGet4f(Vals);
      if (pBindTG1)
        vpGL->mfParameter4f(pBindTG1, param);

      // Draw primitives
      EF_Draw(sh, NULL);
#else
      int bFogOverrided = 0;
      EF_PreDraw(NULL);
      if (m_FS.m_bEnable)
        bFogOverrided = EF_FogCorrection(false, false);

      CCObject *pSaveObj = m_RP.m_pCurObject;
      CCObject *pObj = pSaveObj;
      int nObj = 0;
      while (true)
      {       
        if (sbNeedRender[nObj])
        {
          if (nObj)
          {
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            m_RP.m_FrameObject++;
            pObj = m_RP.m_pCurObject;
          }

          FM.m_Offs = 0;
          FM.mfGet4f(Vals);
          if (pBindTG0)
            vpGL->mfParameter4f(pBindTG0, Vals);

          FM.m_Offs = 1;
          FM.mfGet4f(Vals);
          if (pBindTG1)
            vpGL->mfParameter4f(pBindTG1, Vals);

          vpGL->mfSetVariables(true, NULL);
          vpGL->mfSetStateMatrices();

          {
            //PROFILE_FRAME(Draw_EFIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(sh, NULL);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
        }
        nObj++;
        if (nObj >= m_RP.m_MergedObjects.Num())
          break;
      }
      EF_FogCorrectionRestore(bFogOverrided);
      if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
        EF_CommitTexTransforms(false);
      if (pSaveObj != m_RP.m_pCurObject)
      {
        m_RP.m_pCurObject = pSaveObj;
        m_RP.m_FrameObject++;
      }
#endif

      fDist /= 2.0f;
      if (fDistToCam > fDist+1.0f)
        break;
      fUScale *= 2.0f;
      fVScale *= 2.0f;
    }
  }
  CGLTexMan::BindNULL(0);
  gRenDev->m_RP.m_pFogVolume = fb;
#ifdef PIPE_USE_INSTANCING
  glPopMatrix();
#endif
}

void CGLRenderer::EF_DrawFogOverlayPasses()
{
  // Usually it means first pass in indoor engine (before shadow pass)
  if (m_RP.m_ObjFlags & FOB_ZPASS)
    return;
  if (m_RP.m_FlagsPerFlush & RBSI_FOGVOLUME)
    return;

  if (CRenderer::CV_r_log >= 3)
    Logv(SRendItem::m_RecurseLevel, "--- Fog Pass ---\n");

  float fWatLevel = iSystem->GetI3DEngine()->GetWaterLevel();
  SMFog *fb = m_RP.m_pFogVolume;
  SShader *sh = NULL;
  bool bVFFP = false;
  if (!(m_Features & RFT_HW_VS) || CV_r_Quality_BumpMapping == 0)
    bVFFP = true;
  if (!bVFFP || (m_RP.m_pShader->m_Flags & EF_HASVSHADER))
  {
    if (m_RP.m_pShader->m_eSort != eS_Water && fabs(fb->m_Dist-fWatLevel) < 0.1f && SRendItem::m_RecurseLevel <= 1)
    {
      if (m_RP.m_pRE)
      {
        float fDist;
#ifndef PIPE_USE_INSTANCING
        fDist = m_RP.m_pRE->mfMinDistanceToCamera(m_RP.m_pCurObject);
#else
        CCObject *pObj = m_RP.m_pCurObject;
        int nObj = 0;
        fDist = 999999.0f;
        while (true)
        {
          fDist = min(fDist, m_RP.m_pRE->mfMinDistanceToCamera(pObj));
          nObj++;
          if (nObj >= m_RP.m_MergedObjects.Num())
            break;
          pObj = m_RP.m_MergedObjects[nObj];
        }
#endif
        if (fDist < 40.0f)
          sh = m_cEF.m_ShaderFogCaust;
        else
          sh = m_cEF.m_ShaderFog;
      }
      else
        sh = gRenDev->m_cEF.m_ShaderFogCaust;
    }
    else
      sh = gRenDev->m_cEF.m_ShaderFog;
  }
  else
    sh = m_cEF.m_ShaderFog_FP;

  if (sh && sh->m_HWTechniques.Num())
  {
    if (m_RP.m_pShader->m_eSort == eS_Water && m_RP.m_pFogVolume)
      m_RP.m_pFogVolume->m_Dist += 0.1f;
    EF_DrawGeneralPasses(sh->m_HWTechniques[0], sh, true, 0, sh->m_HWTechniques[0]->m_Passes.Num()-1);
    if (m_RP.m_pShader->m_eSort == eS_Water && m_RP.m_pFogVolume)
      m_RP.m_pFogVolume->m_Dist -= 0.1f;
  }
}

// Draw light passes (up to 4 light sources per single pass)
void CGLRenderer::EF_DrawLightPasses_PS30(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd)
{
  SShaderPassHW *slw;
  int i;

  m_RP.m_nCurLight = 0;
  CVProgram *curVP = NULL;

  PROFILE_FRAME(DrawShader_LightPasses_PS30);

  int nLight;
  int nLights = m_RP.m_NumActiveDLights;

  // make sure for each pass not more then 1 projected light
  if (gRenDev->m_RP.m_pCurObject->m_ObjFlags & FOB_SELECTED)
  {
    int nnn = 0;
  }

  int nStencState = 0;
  bool bStencState = m_RP.m_pStateShader && m_RP.m_pStateShader->m_State && m_RP.m_pStateShader->m_State->m_Stencil;

  slw = &hs->m_Passes[nStart];
  CDLight *ProjLights[16];
  int nProjLights = 0;
  CDLight *OtherLights[16];
  int nOtherLights = 0;
  CDLight *StencLights[16];
  int nStencLights = 0;
  bool bHasLM = m_RP.m_pCurObject->m_nLMId != 0;
  bool bHasAmb = ((m_RP.m_ObjFlags & FOB_LIGHTPASS) == 0);
  for (nLight=0; nLight<nLights; nLight++)
  {
    CDLight *dl = m_RP.m_pActiveDLights[nLight];
    // ignore LM light sources for diffuse only materials with LM but without specular
    if (bHasLM && (dl->m_Flags & DLF_LM) && (slw->m_LMFlags & LMF_NOSPECULAR))
      continue;
    if (bStencState && (dl->m_Flags & DLF_CASTSHADOW_VOLUME))
      StencLights[nStencLights++] = dl;
    else
    if (dl->m_Flags & DLF_PROJECT)
      ProjLights[nProjLights++] = dl;
    else
      OtherLights[nOtherLights++] = dl;
  }
  if (!bHasAmb && !nOtherLights && !nProjLights && !nStencLights)
    return;
  float fFP = 0.334f;
  float fFO = 1.0f;
  int nO = 0;
  int nP = 0;
  float fO = 0;
  float fP = 0;
  int nAmbLights = NUM_PPLIGHTS_PERPASS_PS30;
  if (bHasAmb)
  {
    for (i=nStart; i<=nEnd; i++, slw++)
    {
      if (slw->m_LMFlags & LMF_HASAMBIENT)
      {
        if (slw->m_LMFlags & LMF_HASDOT3LM)
        {
          if (!m_RP.m_pShaderResources || !m_RP.m_pCurObject->m_nLMDirId)
            continue;
        }
      }
      break;
    }
    nAmbLights = slw->m_nAmbMaxLights;
  }
  slw = &hs->m_Passes[nStart];
  for (i=0; i<nStencLights; i++)
  {
    m_RP.m_LPasses[i].nProjectors = 0;
    m_RP.m_LPasses[i].nLights = 1;
    m_RP.m_LPasses[i].pLights[0] = StencLights[i];
  }
  for (; i<32; i++)
  {
    if (nP >= nProjLights && nO >= nOtherLights)
    {
      if (!i && bHasAmb)
      {
        m_RP.m_LPasses[0].nLights = 0;
        m_RP.m_LPasses[0].nProjectors = 0;
        i++;
      }
      break;
    }
    m_RP.m_LPasses[i].nLights = 0;
    m_RP.m_LPasses[i].nProjectors = 0;
    int j = 0;
    while (true)
    {
      fO += fFO;
      if ((int)fO >= nO+1 && nO < nOtherLights)
      {
        m_RP.m_LPasses[i].nLights++;
        m_RP.m_LPasses[i].pLights[j] = OtherLights[nO];
        j++;
        nO++;
      }
      if (m_RP.m_LPasses[i].nProjectors == 0)
      {
        fP += fFP;
        if ((int)fP >= nP+1 && nP < nProjLights)
        {
          m_RP.m_LPasses[i].nLights++;
          m_RP.m_LPasses[i].nProjectors++;
          m_RP.m_LPasses[i].pLights[j] = ProjLights[nP];
          j++;
          nP++;
        }
      }
      else
      {
        if (nO >= nOtherLights)
          break;
      }
      if (nP >= nProjLights && nO >= nOtherLights)
        break;
      if (j == nAmbLights)
      {
        if (nAmbLights != NUM_PPLIGHTS_PERPASS_PS30)
          nAmbLights = NUM_PPLIGHTS_PERPASS_PS30;
        break;
      }
    }
  }

  int nPasses = i;
  int nPass;
  m_RP.m_nCurLight = 0;
  m_RP.m_PersFlags |= RBPF_MULTILIGHTS;
  for (nPass=0; nPass<nPasses; nPass++)
  {
    m_RP.m_StatNumPasses++;
    m_RP.m_nCurLightPass = nPass;
    SLightPass *lp = &m_RP.m_LPasses[nPass];
    m_RP.m_ShaderLightMask = lp->nLights;
    m_RP.m_pCurLight = NULL;
    int Types[4];
    bool bUseOccl = false;
    for (i=0; i<lp->nLights; i++)
    {
      CDLight *dl = lp->pLights[i];
      if (dl->m_Flags & DLF_POINT)
        Types[i] = SLMF_POINT;
      else
      if (dl->m_Flags & DLF_PROJECT)
      {
        Types[i] = SLMF_PROJECTED;
        m_RP.m_pCurLight = dl;
      }
      else
        Types[i] = SLMF_DIRECT;
      if ((dl->m_Flags & DLF_LM) && bHasLM)
      {
        Types[i] |= SLMF_ONLYSPEC;
        if (*(int *)m_RP.m_pCurObject->m_OcclLights != 0)
        {
          byte nLight = dl->m_Id+1;
          if (m_RP.m_pCurObject->m_OcclLights[0] == nLight || m_RP.m_pCurObject->m_OcclLights[1] == nLight || m_RP.m_pCurObject->m_OcclLights[2] == nLight || m_RP.m_pCurObject->m_OcclLights[3] == nLight)
          {
            Types[i] |= SLMF_SPECOCCLUSION;
            bUseOccl = true;
          }
        }
      }
    }
    switch(lp->nLights)
    {
      case 2:
        if (Types[0] > Types[1])
        {
          Exchange(Types[0], Types[1]);
          Exchange(lp->pLights[0], lp->pLights[1]);
        }
  	    break;
      case 3:
        if (Types[0] > Types[1])
        {
          Exchange(Types[0], Types[1]);
          Exchange(lp->pLights[0], lp->pLights[1]);
        }
        if (Types[0] > Types[2])
        {
          Exchange(Types[0], Types[2]);
          Exchange(lp->pLights[0], lp->pLights[2]);
        }
        if (Types[1] > Types[2])
        {
          Exchange(Types[1], Types[2]);
          Exchange(lp->pLights[1], lp->pLights[2]);
        }
    	  break;
      case 4:
        {
          for (int i=0; i<4; i++)
          {
            for (int j=i; j<4; j++)
            {
              if (Types[i] > Types[j])
              {
                Exchange(Types[i], Types[j]);
                Exchange(lp->pLights[i], lp->pLights[j]);
              }
            }
          }
        }
        break;
    }

    for (i=0; i<lp->nLights; i++)
    {
      m_RP.m_ShaderLightMask |= Types[i] << (SLMF_LTYPE_SHIFT + i*4);
    }
    if (lp->nLights == 1 && bStencState)
    {
      nStencState = (lp->pLights[0]->m_Flags & DLF_CASTSHADOW_VOLUME) ? GS_STENCIL : 0;
    }
    for (i=nStart; i<=nEnd; i++, slw++)
    {
      m_RP.m_CurrPass = slw;
      m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~7) | (slw->m_Flags & 3);
      if (slw->m_LMFlags & LMF_HASAMBIENT)
      {
        if (nPass || !bHasAmb)
          continue;
        if (slw->m_LMFlags & LMF_HASDOT3LM)
        {
          if (!m_RP.m_pShaderResources || !m_RP.m_pCurObject->m_nLMDirId)
            continue;
        }
      }
      if ((slw->m_LMFlags & LMF_USEOCCLUSIONMAP) && !bUseOccl)
        continue;

      EF_Eval_TexGen(slw);
      EF_Eval_RGBAGen(slw);
      EF_SetVertexStreams(slw->m_Pointers, 1);

      // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
      curVP = slw->m_VProgram;      
      if (curVP)
        m_RP.m_FlagsPerFlush |= RBSI_USEVP;
      if (slw->mfSetTextures())
      {
        if (curVP)
          curVP->mfSet(true, slw, VPF_DONTSETMATRICES);

        EF_ApplyMatrixOps(slw->m_MatrixOps, true);

        if (curVP)
        {
  #ifndef PIPE_USE_INSTANCING
          curVP->mfSetStateMatrices();
  #endif
          curVP->mfSetVariables(false, &slw->m_VPParamsNoObj);
        }
        else
        {
          m_RP.m_CurrentVLights = 0;
          m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;
        }

        // Set Pixel shaders and Register combiners for the current pass
        if (slw->m_FShader)
          slw->m_FShader->mfSet(true, slw);
        else
          m_RP.m_PersFlags &= ~RBPF_PS1NEEDSET;

        int State;
        if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
          State = slw->m_SecondRenderState;
        else
          State = slw->m_RenderState;
        EF_SetState(State | nStencState);

  #ifndef PIPE_USE_INSTANCING

  #ifdef DO_RENDERLOG
        if (CRenderer::CV_r_log >= 3)
          Logv(SRendItem::m_RecurseLevel, "+++ Light Pass %d [%d Lights]\n", m_RP.m_RendPass, m_RP.m_nCurLights);
  #endif

        EF_Scissor(false, 0, 0, 0, 0);

        EF_Draw(ef, slw);

        m_RP.m_RendPass++;

  #else //PIPE_USE_INSTANCING
        m_RP.m_RendPass++;

        int bFogOverrided = 0;
        // Unlock all VB (if needed) and set current streams
        EF_PreDraw(slw);
        bool bFogDisable = (curVP && (curVP->m_Flags & VPFI_NOFOG)) || (m_RP.m_PersFlags & RBPF_HDR);
        bool bFogVP = (m_RP.m_PersFlags & RBPF_HDR) || (curVP && (curVP->m_Flags & VPFI_VS30ONLY));
        bFogOverrided = EF_FogCorrection(bFogDisable, bFogVP);

        int nObj = 0;
        CCObject *pSaveObj = m_RP.m_pCurObject;
        CCObject *pObj = pSaveObj;
        SArrayPointer_Texture pt;
        pt.ePT = eSrcPointer_TexLM;
        pt.Stage = m_RP.m_nLMStage;
        pt.Type = GL_FLOAT;
        pt.NumComponents = 2;
        while (true)
        {       
          if (nObj)
          {
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            m_RP.m_FrameObject++;
            pObj = m_RP.m_pCurObject;

            if (m_RP.m_FlagsModificators & (RBMF_LMTCUSED | RBMF_BENDINFOUSED))
            {
              if (pObj->m_pLMTCBufferO && pObj->m_pLMTCBufferO->m_pVertexBuffer)
                pt.mfSet(1);
            }

            if (!curVP)
            {
              EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
              m_RP.m_CurrentVLights = 0;

              if (m_RP.m_FrameGTC == m_RP.m_Frame)
              {
                for (int nt=0; nt<gRenDev->m_TexMan->m_nCurStages; nt++)
                {
                  if (m_RP.m_pGTC[nt])
                  {
                    EF_SelectTMU(nt);
                    m_RP.m_pGTC[nt]->mfSet(true);
                  }
                }
              }
            }
          }

          EF_Scissor(false, 0, 0, 0, 0);

  #ifdef DO_RENDERLOG
          if (CRenderer::CV_r_log >= 3)
          {
            Vec3d vPos = pObj->GetTranslation();
            Logv(SRendItem::m_RecurseLevel, "+++ Light Pass %d [%d Lights] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, lp->nLights, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
          }
  #endif

          if (curVP)
          {
            curVP->mfSetStateMatrices();
            curVP->mfSetVariables(true, &slw->m_VPParamsObj);
          }
          // Set per-object alpha-blending
          if (pObj->m_ObjFlags & FOB_HASALPHA)
            pObj->SetAlphaState(slw->m_FShader, State);

          if (slw->m_FShader)
            slw->m_FShader->mfSetVariables(true, slw->m_CGFSParamsObj);
          else
          {
            if (slw->m_RGBComps && slw->m_RGBComps->m_Comps[0] && slw->m_RGBComps->m_Comps[0]->m_bDependsOnObject)
            {
              float *vals = slw->m_RGBComps->mfGet();
              UCol color;
              color.bcolor[2] = (byte)(vals[0] * 255.0f);
              color.bcolor[1] = (byte)(vals[1] * 255.0f);
              color.bcolor[0] = (byte)(vals[2] * 255.0f);
              color.bcolor[3] = (byte)(vals[3] * 255.0f);
              m_RP.m_NeedGlobalColor = color;
            }
            EF_CommitTexStageState();
          }

          {
            //PROFILE_FRAME(Draw_ShaderIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(ef, slw);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
          nObj++;
          if (nObj >= m_RP.m_MergedObjects.Num())
            break;
        }
        EF_FogCorrectionRestore(bFogOverrided);
        if (!CVProgram::m_LastVP)
        {
          if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
            EF_CommitTexTransforms(false);
        }
        else
          m_RP.m_FlagsModificators &= ~(RBMF_TCM | RBMF_TCG);
        if (pSaveObj != m_RP.m_pCurObject)
        {
          m_RP.m_pCurObject = pSaveObj;
          m_RP.m_FrameObject++;
          if (!curVP)
            EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
        }
  #endif
      }
      slw->mfResetTextures();      
      EF_ApplyMatrixOps(slw->m_MatrixOps, false);
      break;
    }
  }
  m_RP.m_ShaderLightMask = 0;
  m_RP.m_nCurLightPass = 0;
  m_RP.m_PersFlags &= ~RBPF_MULTILIGHTS;
}

// Draw light passes - for each light source (used in FP shaders and in programmable pipeline shaders)
void CGLRenderer::EF_DrawLightPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd)
{
  int i, l;
  int nl = 0;
  int n;

  CVProgram *curVP = NULL;
  CVProgram *newVP;

  // Just single pass for transparent objects
  if (m_RP.m_RendPass && m_RP.m_fCurOpacity != 1.0f)
    return;

  PROFILE_FRAME(DrawShader_LightPasses);

  int nStencState = 0;
  bool bStencState = m_RP.m_pStateShader && m_RP.m_pStateShader->m_State && m_RP.m_pStateShader->m_State->m_Stencil;

  // For each light source
  for (l=0; l<m_RP.m_NumActiveDLights; l++)
  {
    m_RP.m_pCurLight = m_RP.m_pActiveDLights[l];
    m_RP.m_nCurLight = m_RP.m_pCurLight->m_Id;
    SShaderPassHW* slw = &hs->m_Passes[nStart];
    nl++;

    // Ignore diffuse passes for specular only light sources
    if ((m_RP.m_pCurLight->m_Flags & DLF_LM) && slw->m_ePassType == eSHP_DiffuseLight)
    {
      if ((m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP_DIR]) || m_RP.m_pCurObject->m_nLMId)
        continue;
    }

    if (bStencState)
      nStencState = (m_RP.m_pCurLight->m_Flags & DLF_CASTSHADOW_VOLUME) ? GS_STENCIL : 0;

    bool bOnlyLightPass;
    bool bShadowMask = false;

    n = 0;
    SArrayPointer::m_LastEnabledPass = 0;
    bool bBreak = false;

    // For each layer/pass
    for (i=nStart; i<=nEnd; i++, slw++)
    {
      int msk;

      // Check light type and filter passes 
      if (msk = (slw->m_LightFlags & DLF_LIGHTTYPE_MASK))
      {
        if (!(msk & (m_RP.m_pCurLight->m_Flags & DLF_LIGHTTYPE_MASK)))
          continue;  // If light type doesn't match pass type ignore this pass
        if (slw->m_LMFlags & LMF_USEOCCLUSIONMAP)
        {
          if (*(int *)m_RP.m_pCurObject->m_OcclLights == 0)
            continue;
        }
        // If we have only-specular pass and no LM on surface ignore this pass
        if (slw->m_LightFlags & DLF_LM)
        {
          if (!(m_RP.m_pCurLight->m_Flags & DLF_LM))
            continue;
          if ((!m_RP.m_pShaderResources || !m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP_DIR]) && !m_RP.m_pCurObject->m_nLMId)
            continue;
          assert (m_RP.m_pCurLight->m_SpecColor.r!=0 || m_RP.m_pCurLight->m_SpecColor.g!=0 || m_RP.m_pCurLight->m_SpecColor.b!=0);
          // Ignore specular pass if specular is less threshold
          if (m_RP.m_pCurLight->m_SpecColor.r<=0.01f && m_RP.m_pCurLight->m_SpecColor.g<=0.01f && m_RP.m_pCurLight->m_SpecColor.b<=0.01f)
            continue;
        }
      }
      if (slw->m_LMFlags & LMF_NOBUMP)
      {
        if (m_RP.m_pShaderResources && (!m_RP.m_pShaderResources->m_Textures[EFTT_BUMP] || !(m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_nFlags & FTU_NOBUMP)))
          continue;
        bBreak = true;
      }
      else
      if (bBreak)
        break;

      if (EF_RejectLightPass(slw))
        continue;

      bOnlyLightPass = EF_IsOnlyLightPass(slw);

      if (CV_r_bumpselfshadow && bOnlyLightPass && m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_BUMP] && m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_TexPic && m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_TexPic->m_pSH)
      {
        EF_DrawLightShadowMask(l);
        curVP = NULL;
        bShadowMask = true;
      }

      if (!n)
      {
        if (!(m_RP.m_StatLightMask & (1<<m_RP.m_nCurLight)))
        {
          m_RP.m_StatLightMask |= (1<<m_RP.m_nCurLight);
          m_RP.m_StatNumLights++;
        }
      }

      SArrayPointer::m_CurEnabledPass = 0;

      n++;
      m_RP.m_StatNumLightPasses++;      
      m_RP.m_CurrPass = slw;

      if (m_RP.m_pCurLightMaterial)
        m_RP.m_pCurLightMaterial->mfApply(slw->m_LMFlags | (l<<LMF_LIGHT_SHIFT));
      else
        m_RP.m_CurrentVLights = 0;

      m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~15) | (slw->m_Flags & 3);

      // Evaluating of the main shader parameters
      EF_Eval_TexGen(slw);
      EF_Eval_RGBAGen(slw);
      EF_SetVertexStreams(slw->m_Pointers, 1);

      // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
      if (slw->mfSetTextures())
      {
        if (m_RP.m_pCurLightMaterial)
          m_RP.m_pCurLightMaterial->mfApply(slw->m_LMFlags | (l<<LMF_LIGHT_SHIFT));
        else
          m_RP.m_CurrentVLights = 0;

        newVP = slw->m_VProgram;

        // Set vertex program for the current pass if needed
        if (newVP != curVP)
        {
          if (newVP)
          {
            curVP = newVP;
            curVP->mfSet(true, slw, VPF_SETPOINTERSFORPASS | VPF_DONTSETMATRICES);
          }
          else
            curVP = NULL;
        }

        EF_ApplyMatrixOps(slw->m_MatrixOps, true);

        if (curVP)
        {
#ifndef PIPE_USE_INSTANCING
          curVP->mfSetStateMatrices();
#endif
          curVP->mfSetVariables(false, &slw->m_VPParamsNoObj);
        }
        else
          m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;

        if (slw->m_FShader)
          slw->m_FShader->mfSet(true, slw);
        else
          m_RP.m_PersFlags &= ~(RBPF_PS1NEEDSET | RBPF_PS2NEEDSET | RBPF_TSNEEDSET);

        int State;
        // Set Render states for the current pass
        if (m_RP.m_pCurObject->m_RenderState)
          State = m_RP.m_pCurObject->m_RenderState | nStencState;
        else
        {
          if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
            State = bShadowMask ? (slw->m_SecondRenderState & ~GS_BLEND_MASK) | GS_BLSRC_ONEMINUSDSTALPHA | GS_BLDST_ONE : slw->m_SecondRenderState | nStencState;
          else
            State = bShadowMask ? (slw->m_RenderState & ~GS_BLEND_MASK) | GS_BLSRC_ONEMINUSDSTALPHA | GS_BLDST_ONE : slw->m_RenderState | nStencState;
        }
        EF_SetState(State);

#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
        if (CRenderer::CV_r_log >= 3)
          Logv(SRendItem::m_RecurseLevel, "+++ Light Pass %d [Light %d]\n", m_RP.m_RendPass, l);
#endif

        // Set scissor rect
        if (CV_r_scissor)
        {
          if (bOnlyLightPass && m_RP.m_pCurLight->m_sWidth && m_RP.m_pCurLight->m_sHeight)
            EF_Scissor(true, m_RP.m_pCurLight->m_sX, m_RP.m_pCurLight->m_sY, m_RP.m_pCurLight->m_sWidth, m_RP.m_pCurLight->m_sHeight);
          else
            m_RP.m_pCurObject->SetScissor();
        }

        EF_Draw(ef, slw);

        m_RP.m_RendPass++;

#else //PIPE_USE_INSTANCING

        m_RP.m_RendPass++;

        int bFogOverrided = 0;
        // Unlock all VB (if needed) and set current streams
        EF_PreDraw(slw);
        if (m_FS.m_bEnable)
          bFogOverrided = EF_FogCorrection(false, false);

        int nObj = 0;
        CCObject *pSaveObj = m_RP.m_pCurObject;
        CCObject *pObj = pSaveObj;
        bool bLightSicssorWasSet = false;

        // Set scissor rect
        if (CV_r_scissor)
        {
          if (bOnlyLightPass && m_RP.m_pCurLight->m_sWidth && m_RP.m_pCurLight->m_sHeight)
          {
            bLightSicssorWasSet = true;
            if (!(m_RP.m_ObjFlags & FOB_NOSCISSOR))
              EF_Scissor(true, m_RP.m_pCurLight->m_sX, m_RP.m_pCurLight->m_sY, m_RP.m_pCurLight->m_sWidth, m_RP.m_pCurLight->m_sHeight);
            else
              EF_Scissor(false, 0, 0, 0, 0);
          }
          else
            pObj->SetScissor();
        }
        SArrayPointer_Texture pt;
        pt.ePT = eSrcPointer_TexLM;
        pt.Stage = m_RP.m_nLMStage;
        pt.Type = GL_FLOAT;
        pt.NumComponents = 2;
        while (true)
        {       
          if (nObj)
          {
            // Object changed
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            pObj = m_RP.m_pCurObject;
            m_RP.m_FrameObject++;
            if (CV_r_scissor && !bLightSicssorWasSet)
              pObj->SetScissor();
            if (m_RP.m_FlagsModificators & (RBMF_LMTCUSED | RBMF_BENDINFOUSED))
            {
              if (pObj->m_pLMTCBufferO && pObj->m_pLMTCBufferO->m_pVertexBuffer)
              {
                pt.mfSet(1);
              }
            }
            if (!curVP)
            {
              // Specify transform
              EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
              if (m_RP.m_pCurLightMaterial)
                m_RP.m_pCurLightMaterial->mfApply(slw->m_LMFlags | (l<<LMF_LIGHT_SHIFT));
              else
                m_RP.m_CurrentVLights = 0;

              // Set texgen modes for batched objects
              if (m_RP.m_FrameGTC == m_RP.m_Frame)
              {
                for (int nt=0; nt<gRenDev->m_TexMan->m_nCurStages; nt++)
                {
                  if (m_RP.m_pGTC[nt])
                  {
                    EF_SelectTMU(nt);
                    m_RP.m_pGTC[nt]->mfSet(true);
                  }
                }
              }
            }
          }

#ifdef DO_RENDERLOG
          if (CRenderer::CV_r_log >= 3)
          {
            Vec3d vPos = pObj->GetTranslation();
            Logv(SRendItem::m_RecurseLevel, "+++ Light Pass %d [Light %d] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, l, m_RP.m_pCurObject->m_VisId, vPos[0], vPos[1], vPos[2]);
          }
#endif

          if (curVP)
          {
            curVP->mfSetStateMatrices();
            curVP->mfSetVariables(true, &slw->m_VPParamsObj);
          }

          // Set per-object alpha-blending
          if (pObj->m_ObjFlags & FOB_HASALPHA)
            pObj->SetAlphaState(slw->m_FShader, State);

          if (slw->m_FShader)
            slw->m_FShader->mfSetVariables(true, slw->m_CGFSParamsObj);
          else
          {
            if (slw->m_RGBComps && slw->m_RGBComps->m_Comps[0] && slw->m_RGBComps->m_Comps[0]->m_bDependsOnObject)
            {
              float *vals = slw->m_RGBComps->mfGet();
              m_RP.m_NeedGlobalColor.bcolor[0] = (byte)(vals[0] * 255.0f);
              m_RP.m_NeedGlobalColor.bcolor[1] = (byte)(vals[1] * 255.0f);
              m_RP.m_NeedGlobalColor.bcolor[2] = (byte)(vals[2] * 255.0f);
              m_RP.m_NeedGlobalColor.bcolor[3] = (byte)(vals[3] * 255.0f);
            }
            EF_CommitTexStageState();
          }

          {
            //PROFILE_FRAME(Draw_EFIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(ef, slw);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
          nObj++;
          if (nObj >= m_RP.m_MergedObjects.Num())
            break;
        }
        EF_FogCorrectionRestore(bFogOverrided);
        if (!(m_RP.m_PersFlags & RBPF_VSWASSET))
        {
          if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
            EF_CommitTexTransforms(false);
        }
        if (pSaveObj != m_RP.m_pCurObject)
        {
          m_RP.m_pCurObject = pSaveObj;
          m_RP.m_FrameObject++;
          if (!curVP)
            EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
        }
#endif
      }

      slw->mfResetTextures();
      EF_ApplyMatrixOps(slw->m_MatrixOps, false);

      if ((m_RP.m_FlagsPerFlush & RBSI_VERTSMERGED) && SUPPORTS_GL_NV_fence)
        glSetFenceNV(m_RP.m_VidBufs[m_RP.m_MergedStreams[0]].m_Fence, GL_ALL_COMPLETED_NV);

      break;
    }
  }
  m_RP.m_nCurLight = -1;
  m_RP.m_pCurLight = NULL;
}

void CGLRenderer::EF_DrawLightShadowMask(int nLight)
{
  STexPic *pTX = m_RP.m_pShaderResources->m_Textures[EFTT_BUMP]->m_TU.m_TexPic;
  STexShadow *pTSH = pTX->m_pSH;
  EF_SelectTMU(0);
  pTSH->m_pHorizonTex[0]->Set();
  EF_SelectTMU(1);
  pTSH->m_pBasisTex[0]->Set();
  EF_SelectTMU(2);
  pTSH->m_pHorizonTex[1]->Set();
  EF_SelectTMU(3);
  pTSH->m_pBasisTex[1]->Set();
  EF_SetState(GS_COLMASKONLYALPHA);
  if (!m_RP.m_VPTexShadow)
    m_RP.m_VPTexShadow = CVProgram::mfForName("CGVProgTexShadow");
  if (!m_RP.m_RCTexShadow)
    m_RP.m_RCTexShadow = CPShader::mfForName("CGRCTexShadow");

  m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~0xf) | RBMF_TANGENTSUSED;

  m_RP.m_VPTexShadow->mfSet(true, NULL, VPF_DONTSETMATRICES);
  m_RP.m_VPTexShadow->mfSetVariables(false, NULL);
  m_RP.m_RCTexShadow->mfSet(true, NULL);
#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
  if (CRenderer::CV_r_log >= 3)
    Logv(SRendItem::m_RecurseLevel, "+++ Bump shadow pass %d [light %d]\n", m_RP.m_RendPass, nLight);
#endif

  m_RP.m_VPTexShadow->mfSetStateMatrices();
  m_RP.m_VPTexShadow->mfSetVariables(true, NULL);
  m_RP.m_RCTexShadow->mfSetVariables(true, NULL);
  EF_Draw(m_RP.m_pShader, NULL);
#else
  EF_PreDraw(NULL);

  int nObj = 0;
  CCObject *pSaveObj = m_RP.m_pCurObject;
  CCObject *pObj = pSaveObj;
  while (true)
  {       
    if (nObj)
    {
      m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
      pObj = m_RP.m_pCurObject;
      m_RP.m_FrameObject++;
    }

#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log >= 3)
    {
      Vec3d vPos = pObj->GetTranslation();
      Logv(SRendItem::m_RecurseLevel, "+++ Bump shadow pass %d [light %d] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, nLight, m_RP.m_pCurObject->m_VisId, vPos[0], vPos[1], vPos[2]);
    }
#endif

    if (m_RP.m_VPTexShadow)
    {
      m_RP.m_VPTexShadow->mfSetStateMatrices();
      m_RP.m_VPTexShadow->mfSetVariables(true, NULL);
    }
    if (m_RP.m_RCTexShadow)
      m_RP.m_RCTexShadow->mfSetVariables(true, NULL);

    {
      //PROFILE_FRAME(Draw_EFIndexMesh);
      if (m_RP.m_pRE)
        m_RP.m_pRE->mfDraw(m_RP.m_pShader, NULL);
      else
        EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
    }
    nObj++;
    if (nObj >= m_RP.m_MergedObjects.Num())
      break;
  }
  if (pSaveObj != m_RP.m_pCurObject)
  {
    m_RP.m_pCurObject = pSaveObj;
    m_RP.m_FrameObject++;
  }
#endif

  CGLTexMan::BindNULL(1);
}

void CGLRenderer::EF_DrawSubsurfacePasses(SShaderTechnique *hs, SShader *ef)
{
  int l;

  // For each light source
  for (l=0; l<m_RP.m_NumActiveDLights; l++)
  {
    m_RP.m_pCurLight = m_RP.m_pActiveDLights[l];
    m_RP.m_nCurLight = m_RP.m_pCurLight->m_Id;

    // Ignore diffuse passes for specular only light sources
    if ((m_RP.m_pCurLight->m_Flags & DLF_LM))
    {
      if ((m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_LIGHTMAP_DIR]) || m_RP.m_pCurObject->m_nLMId)
        continue;
    }

    STexPic *pTX = m_RP.m_pShaderResources->m_Textures[EFTT_SUBSURFACE]->m_TU.m_TexPic;
    EF_SelectTMU(0);
    pTX->Set();
    EF_SetState(GS_BLSRC_ONE | GS_BLDST_ONE);
    if (!m_RP.m_VPSubSurfaceScatering)
      m_RP.m_VPSubSurfaceScatering= CVProgram::mfForName("CGVProgSubSurface");
    if (!m_RP.m_RCSubSurfaceScatering)
      m_RP.m_RCSubSurfaceScatering= CPShader::mfForName("CGRCSubSurface");

    m_RP.m_VPSubSurfaceScatering->mfSet(true, NULL);  
    m_RP.m_VPSubSurfaceScatering->mfSetVariables(false, NULL);
    m_RP.m_RCSubSurfaceScatering->mfSet(true, NULL);
    m_RP.m_RCSubSurfaceScatering->mfSetVariables(false, NULL);

#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
    if (CRenderer::CV_r_log >= 3)
      Logv(SRendItem::m_RecurseLevel, "+++ Subsurface scattering pass %d [light %d]\n", m_RP.m_RendPass, l);
#endif

    m_RP.m_VPSubSurfaceScatering->mfSetVariables(true, NULL);
    m_RP.m_RCSubSurfaceScatering->mfSetVariables(true, NULL);
    EF_Draw(m_RP.m_pShader, NULL);
#else
    EF_PreDraw(NULL);

    int nObj = 0;
    CCObject *pSaveObj = m_RP.m_pCurObject;
    CCObject *pObj = pSaveObj;
    while (true)
    {       
      if (nObj)
      {
        m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
        pObj = m_RP.m_pCurObject;
        m_RP.m_FrameObject++;
      }

#ifdef DO_RENDERLOG
      if (CRenderer::CV_r_log >= 3)
      {
        Vec3d vPos = pObj->GetTranslation();
        Logv(SRendItem::m_RecurseLevel, "+++ Subsurface scattering pass %d [light %d] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, l, m_RP.m_pCurObject->m_VisId, vPos[0], vPos[1], vPos[2]);
      }
#endif

      if (m_RP.m_VPSubSurfaceScatering)
      {
        m_RP.m_VPSubSurfaceScatering->mfSetStateMatrices();
        m_RP.m_VPSubSurfaceScatering->mfSetVariables(true, NULL);
      }
      if (m_RP.m_RCSubSurfaceScatering)
      {
        m_RP.m_RCSubSurfaceScatering->mfSetVariables(true, NULL);
      }

      {
        //PROFILE_FRAME(Draw_EFIndexMesh);
        if (m_RP.m_pRE)
          m_RP.m_pRE->mfDraw(m_RP.m_pShader, NULL);
        else
          EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
      }
      nObj++;
      if (nObj >= m_RP.m_MergedObjects.Num())
        break;
    }
    if (pSaveObj != m_RP.m_pCurObject)
    {
      m_RP.m_pCurObject = pSaveObj;
      m_RP.m_FrameObject++;
    }
#endif
  }
  CGLTexMan::BindNULL(1);
}

// Draw fur layer passes (used in programmable pipeline shaders only)
void CGLRenderer::EF_DrawFurPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd, EShaderPassType eShPass)
{
  m_RP.m_FlagsPerFlush |= RBSI_FURPASS;

  PROFILE_FRAME(DrawShader_FurPasses);

  m_RP.m_pCurLight = NULL;
}

struct SShadowLight
{
  list2<ShadowMapLightSourceInstance> *pSmLI;
  CDLight *pDL;
};

static _inline int Compare(SShadowLight &a, SShadowLight &b)
{
  if (a.pSmLI->Count() > b.pSmLI->Count())
    return -1;
  if (a.pSmLI->Count() < b.pSmLI->Count())
    return 1;
  return 0;
}

#include <IEntityRenderState.h>

int CGLRenderer::EF_DrawMultiShadowPasses(SShaderTechnique *hs, SShader *ef, int nStart)
{
  int i, j;
  int nStartShadowLM = -1;
  int nEndShadowLM = -1;
  int nStartShadow = -1;
  int nEndShadow = -1;
  int nStartLight = -1;
  int nEndLight = -1;
  int nEnd = -1;
  nStart++;
  for (i=nStart; i<hs->m_Passes.Num(); i++)
  {
    switch (hs->m_Passes[i].m_ePassType)
    {
      case eSHP_SpecularLight:
      case eSHP_DiffuseLight:
      case eSHP_Light:
        if (nStartLight < 0)
          nStartLight = i;
        else
          nEndLight = i;
        break;
      case eSHP_Shadow:
        if ((hs->m_Passes[i].m_SecondRenderState & GS_BLEND_MASK) == (GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA))
        {
          if (nStartShadowLM < 0)
            nStartShadowLM = i;
          else
            nEndShadowLM = i;
        }
        else
        {
          if (nStartShadow < 0)
            nStartShadow = i;
          else
            nEndShadow = i;
        }
        break;
      default:
        nEnd = i;
    }
  }
  if (nEnd < 0)
    nEnd = i-1;

  list2<ShadowMapLightSourceInstance> * lsources = (list2<ShadowMapLightSourceInstance>*)m_RP.m_pCurObject->m_pShadowCasters;
  if (!lsources || !lsources->Count())
    return nEnd;

  if (nEndLight < 0)
    nEndLight = nStartLight;
  if (nEndShadow < 0)
    nEndShadow = nStartShadow;
  if (nEndShadowLM < 0)
    nEndShadowLM = nStartShadowLM;
  assert(nStartShadow>=0);
  assert(nStartLight>=0);

  int nFrustrums = 0;
  list2<ShadowMapLightSourceInstance> SmLI[16];
  list2<ShadowMapLightSourceInstance> SmLI_LM;
  SShadowLight SL[16];
  int nLights = m_RP.m_NumActiveDLights;
  bool bHasDot3LM = m_RP.m_pShaderResources && m_RP.m_pCurObject->m_nLMDirId;
  
  for (i=0; i<nLights; i++)
  {
    SL[i].pSmLI = &SmLI[i];
    SL[i].pDL = m_RP.m_pActiveDLights[i];
    int nLightID = SL[i].pDL->m_Id;
    for (j=0; j<lsources->Count(); j++)
    {
      ShadowMapLightSourceInstance *Inst = lsources->Get(j);
      if (bHasDot3LM && Inst->m_pLS->nDLightId == nLightID && (SL[i].pDL->m_Flags & DLF_LM))
      {
        if (!Inst->m_pLS->m_LightFrustums[0].pOwner || !(Inst->m_pLS->m_LightFrustums[0].pOwner->GetRndFlags() & ERF_CASTSHADOWINTOLIGHTMAP))
          SmLI_LM.Add(*Inst);
      }
      else
      if (Inst->m_pLS->nDLightId == nLightID)
        SmLI[i].Add(*Inst);
    }
  }
  ::Sort(&SL[0], nLights);
  int nStartLightWithoutSC = -1;
  if (SmLI_LM.Count() && nStartShadowLM >= 0)
  {
    m_RP.m_pCurObject->m_pShadowCasters = &SmLI_LM;
    EF_DrawShadowPasses(hs, ef, nStartShadowLM, nEndShadowLM);
  }
  for (i=0; i<nLights; i++)
  {
    if (SL[i].pSmLI->Count())
    {
      m_RP.m_pCurObject->m_pShadowCasters = SL[i].pSmLI;
      EF_DrawShadowPasses(hs, ef, nStartShadow, nEndShadow);
      m_RP.m_pActiveDLights[0] = SL[i].pDL;
      m_RP.m_NumActiveDLights = 1;
      EF_DrawLightPasses(hs, ef, nStartLight, nEndLight);
    }
    else
    {
      if (nStartLightWithoutSC < 0)
        nStartLightWithoutSC = i;
      m_RP.m_pActiveDLights[i-nStartLightWithoutSC] = SL[i].pDL;
    }
  }
  if (nStartLightWithoutSC >= 0)
  {
    m_RP.m_NumActiveDLights = nLights-nStartLightWithoutSC;
    EF_DrawLightPasses(hs, ef, nStartLight, nEndLight);
  }
  m_RP.m_pCurObject->m_pShadowCasters = lsources;
  for (i=0; i<m_RP.m_NumActiveDLights; i++)
  {
    m_RP.m_pActiveDLights[i] = SL[i].pDL;
  }
  m_RP.m_NumActiveDLights = nLights;

  return nEnd;
}

void CGLRenderer::EF_DrawShadowPasses(SShaderTechnique *hs, SShader *ef, int nStart, int nEnd)
{
  SShaderPassHW *slw;
  int i;

  if ((m_RP.m_ObjFlags & FOB_LIGHTPASS) && (ef->m_Flags & EF_USELIGHTS))
    return;

  m_RP.m_nCurLight = 0;
  CVProgram *curVP = NULL;
  CVProgram *newVP;

  SArrayPointer::m_LastEnabledPass = 0;
  list2<ShadowMapLightSourceInstance> * lsources = (list2<ShadowMapLightSourceInstance>*)m_RP.m_pCurObject->m_pShadowCasters;
  if (!lsources)
    return;
  m_RP.m_FlagsPerFlush |= RBSI_SHADOWPASS;

  PROFILE_FRAME(DrawShader_ShadowPasses);

  int nCaster = 0;
  if (ef->m_eSort != eS_TerrainShadowPass)
  {
    if (!CRenderer::CV_r_selfshadow || !(m_Features & RFT_SHADOWMAP_SELFSHADOW))
    {
			if(	lsources->Get(0)->m_pLS->GetShadowMapFrustum()->pOwner == lsources->Get(0)->m_pReceiver && 
				!(lsources->Get(0)->m_pLS->GetShadowMapFrustum()->dwFlags & SMFF_ACTIVE_SHADOW_MAP))
				nCaster++; // skip self shadowing pass
    }
  }
  int nCasters = lsources->Count();
  int nDeltaCasters = 1;

  for (; nCaster<nCasters; nCaster+=nDeltaCasters)
  {
    m_RP.m_nCurStartCaster = nCaster;
    m_RP.m_StatNumPasses++;
    slw = &hs->m_Passes[nStart];
    for (i=nStart; i<=nEnd; i++, slw++)
    {
      int msk;
      if (msk = (slw->m_LightFlags & DLF_LIGHTTYPE_MASK))
      {
        if (!m_RP.m_NumActiveDLights)
          return;
        m_RP.m_pCurLight = m_RP.m_pActiveDLights[0];
        m_RP.m_nCurLight = m_RP.m_pCurLight->m_Id;
        if (!(msk & (m_RP.m_pCurLight->m_Flags & DLF_LIGHTTYPE_MASK)))
          continue;
      }
      if (slw->m_LMFlags & LMF_SAMPLES)
      {
        if (slw->m_LMFlags & LMF_4SAMPLES)
        {
          if (nCasters-nCaster < 4)
            continue;
          nDeltaCasters = 4;
        }
        else
        if (slw->m_LMFlags & LMF_3SAMPLES)
        {
          if (nCasters-nCaster < 3)
            continue;
          nDeltaCasters = 3;
        }
        else
        if (slw->m_LMFlags & LMF_2SAMPLES)
        {
          if (nCasters-nCaster < 2)
            continue;
          nDeltaCasters = 2;
        }
        else
        if (slw->m_LMFlags & LMF_1SAMPLES)
        {
          if (nCasters-nCaster < 1)
            continue;
          nDeltaCasters = 1;
        }
      }
      m_RP.m_CurrPass = slw;
      SArrayPointer::m_CurEnabledPass = 0;

      if((m_RP.m_FlagsPerFlush & RBSI_VERTSMERGED) && SUPPORTS_GL_NV_fence && (m_RP.m_Flags & RBF_MODIF_MASK) && !glTestFenceNV(m_RP.m_VidBufs[m_RP.m_MergedStreams[0]].m_Fence))
        glFinishFenceNV(m_RP.m_VidBufs[m_RP.m_MergedStreams[0]].m_Fence);

      if (slw->m_LMFlags & LMF_POLYOFFSET)
      {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(CV_gl_offsetfactor, CV_gl_offsetunits);
      }

      m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~7) | (slw->m_Flags & 3);

      // Evaluating of the main shader parameters
      EF_Eval_DeformVerts(slw->m_Deforms);
      EF_Eval_TexGen(slw);
      EF_Eval_RGBAGen(slw);

      EF_SetVertexStreams(slw->m_Pointers, 1);

      // Set all textures and HW TexGen modes for the current pass 
      if (slw->mfSetTextures())
      {
        m_RP.m_CurrentVLights = 0;
        newVP = slw->m_VProgram;

        // Set vertex program for the current pass if needed
        if (newVP != curVP)
        {
          if (newVP)
          {
            curVP = newVP;
            curVP->mfSet(true, slw, VPF_SETPOINTERSFORPASS | VPF_DONTSETMATRICES);
          }
          else
            curVP = NULL;
        }
        EF_ApplyMatrixOps(slw->m_MatrixOps, true);
        if (curVP)
        {
#ifndef PIPE_USE_INSTANCING
          curVP->mfSetStateMatrices();
#endif
          curVP->mfSetVariables(false, &slw->m_VPParamsNoObj);
        }
        else
          m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;

        // Set Pixel shaders and Register combiners for the current pass
        if (slw->m_FShader)
          slw->m_FShader->mfSet(true, slw);
        else
          m_RP.m_PersFlags &= ~(RBPF_PS1NEEDSET | RBPF_PS2NEEDSET | RBPF_TSNEEDSET);

        if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
          EF_SetState(slw->m_SecondRenderState);
        else
          EF_SetState(slw->m_RenderState);

#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
          if (CRenderer::CV_r_log >= 3)
            Logv(SRendItem::m_RecurseLevel, "+++ Shadow Pass %d [%d Samples]\n", m_RP.m_RendPass, nDeltaCasters);
#endif
          EF_Scissor(false, 0, 0, 0, 0);

          EF_Draw(ef, slw);

          m_RP.m_RendPass++;

#else //PIPE_USE_INSTANCING

          m_RP.m_RendPass++;

          int bFogOverrided = 0;
          // Unlock all VB (if needed) and set current streams
          EF_PreDraw(slw);
          if (m_FS.m_bEnable)
            bFogOverrided = EF_FogCorrection(false, false);

          int nObj = 0;
          CCObject *pSaveObj = m_RP.m_pCurObject;
          CCObject *pObj = pSaveObj;
          while (true)
          {       
            if (nObj)
            {
              m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
              m_RP.m_FrameObject++;
              pObj = m_RP.m_pCurObject;

              if (!curVP)
              {
                EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
                if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial && !(ef->m_LMFlags & LMF_DISABLE))
                  m_RP.m_pShaderResources->m_LMaterial->mfApply(slw->m_LMFlags);
                else
                  m_RP.m_CurrentVLights = 0;

                if (m_RP.m_FrameGTC == m_RP.m_Frame)
                {
                  for (int nt=0; nt<gRenDev->m_TexMan->m_nCurStages; nt++)
                  {
                    if (m_RP.m_pGTC[nt])
                    {
                      EF_SelectTMU(nt);
                      m_RP.m_pGTC[nt]->mfSet(true);
                    }
                  }
                }
              }
            }
            EF_Scissor(false, 0, 0, 0, 0);

#ifdef DO_RENDERLOG
            if (CRenderer::CV_r_log >= 3)
            {
              Vec3d vPos = pObj->GetTranslation();
              Logv(SRendItem::m_RecurseLevel, "+++ Shadow Pass %d [%d Samples] (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, nDeltaCasters, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
            }
#endif
            if (curVP)
            {
              curVP->mfSetStateMatrices();
              curVP->mfSetVariables(true, &slw->m_VPParamsObj);
            }
            if (slw->m_FShader)
              slw->m_FShader->mfSetVariables(true, slw->m_CGFSParamsObj);
            else
            {
              if (slw->m_RGBComps && slw->m_RGBComps->m_Comps[0] && slw->m_RGBComps->m_Comps[0]->m_bDependsOnObject)
              {
                float *vals = slw->m_RGBComps->mfGet();
                m_RP.m_NeedGlobalColor.bcolor[0] = (byte)(vals[0] * 255.0f);
                m_RP.m_NeedGlobalColor.bcolor[1] = (byte)(vals[1] * 255.0f);
                m_RP.m_NeedGlobalColor.bcolor[2] = (byte)(vals[2] * 255.0f);
                m_RP.m_NeedGlobalColor.bcolor[3] = (byte)(vals[3] * 255.0f);
              }
              EF_CommitTexStageState();
            }

            {
              //PROFILE_FRAME(Draw_EFIndexMesh);
              if (m_RP.m_pRE)
                m_RP.m_pRE->mfDraw(ef, slw);
              else
                EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
            }
            nObj++;
            if (nObj >= m_RP.m_MergedObjects.Num())
              break;
          }
          EF_FogCorrectionRestore(bFogOverrided);
          if (!(m_RP.m_PersFlags & RBPF_VSWASSET))
          {
            if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
              EF_CommitTexTransforms(false);
          }
          if (pSaveObj != m_RP.m_pCurObject)
          {
            m_RP.m_pCurObject = pSaveObj;
            m_RP.m_FrameObject++;
            if (!curVP)
              EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
          }
#endif
      }

      slw->mfResetTextures();
      EF_ApplyMatrixOps(slw->m_MatrixOps, false);

      if ((m_RP.m_FlagsPerFlush & RBSI_VERTSMERGED) && SUPPORTS_GL_NV_fence)
        glSetFenceNV(m_RP.m_VidBufs[m_RP.m_MergedStreams[0]].m_Fence, GL_ALL_COMPLETED_NV);

      if (slw->m_LMFlags & LMF_POLYOFFSET)
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
  }
  m_RP.m_nCurStartCaster = 0;
}

void CGLRenderer::EF_DrawGeneralPasses(SShaderTechnique *hs, SShader *ef, bool bFog, int nStart, int nEnd)
{
  SShaderPassHW *slw;
  int i;

  if ((m_RP.m_ObjFlags & FOB_LIGHTPASS) && (ef->m_Flags & EF_USELIGHTS))
    return;
  if (!bFog && (m_RP.m_ObjFlags & FOB_FOGPASS))
    return;

  m_RP.m_nCurLight = 0;
  CVProgram *curVP = NULL;
  CVProgram *newVP;

  PROFILE_FRAME(DrawShader_GeneralPasses);

  int bFogOverrided = 0;
  SArrayPointer::m_LastEnabledPass = 0;
  slw = &hs->m_Passes[nStart];
  for (i=nStart; i<=nEnd; i++, slw++)
  {
    SArrayPointer::m_CurEnabledPass = 0;

    if((m_RP.m_FlagsPerFlush & RBSI_VERTSMERGED) && SUPPORTS_GL_NV_fence && (m_RP.m_Flags & RBF_MODIF_MASK) && !glTestFenceNV(m_RP.m_VidBufs[m_RP.m_MergedStreams[0]].m_Fence))
      glFinishFenceNV(m_RP.m_VidBufs[m_RP.m_MergedStreams[0]].m_Fence);

    if (slw->m_LMFlags & LMF_POLYOFFSET)
    {
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(CV_gl_offsetfactor, CV_gl_offsetunits);
    }

    m_RP.m_StatNumPasses++;
    m_RP.m_CurrPass = slw;

    m_RP.m_FlagsModificators = (m_RP.m_FlagsModificators & ~7) | (slw->m_Flags & 3);

    // Evaluating of the main shader parameters
    EF_Eval_DeformVerts(slw->m_Deforms);
    EF_Eval_TexGen(slw);
    EF_Eval_RGBAGen(slw);

    EF_SetVertexStreams(slw->m_Pointers, 1);

    // Set all textures and HW TexGen modes for the current pass 
    if (slw->mfSetTextures())
    {
      newVP = slw->m_VProgram;

      if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial && !(ef->m_LMFlags & LMF_DISABLE))
      {
        m_RP.m_pShaderResources->m_LMaterial->mfApply(slw->m_LMFlags);
        if (!newVP && !(slw->m_LMFlags & LMF_DISABLE) && !(m_RP.m_CurrentVLights & 0xff))
          EF_ConstantLightMaterial(m_RP.m_pShaderResources->m_LMaterial, slw->m_LMFlags);
      }
      else
        m_RP.m_CurrentVLights = 0;

      // Set vertex program for the current pass if needed
      if (newVP != curVP)
      {
        if (newVP)
        {
          curVP = newVP;
          curVP->mfSet(true, slw, VPF_SETPOINTERSFORPASS | VPF_DONTSETMATRICES);
        }
        else
          curVP = NULL;
      }
      EF_ApplyMatrixOps(slw->m_MatrixOps, true);
      if (curVP)
      {
#ifndef PIPE_USE_INSTANCING
        curVP->mfSetStateMatrices();
#endif
        curVP->mfSetVariables(false, &slw->m_VPParamsNoObj);
      }
      else
        m_RP.m_PersFlags &= ~RBPF_VSNEEDSET;

      // Set Pixel shaders and Register combiners for the current pass
      if (slw->m_FShader)
        slw->m_FShader->mfSet(true, slw);
      else
        m_RP.m_PersFlags &= ~(RBPF_PS1NEEDSET | RBPF_PS2NEEDSET | RBPF_TSNEEDSET);

      int State;
      // Set Render states for the current pass
      if (bFog)
      {
        State = slw->m_RenderState;
        if (m_RP.m_pShader->m_Flags2 & EF2_OPAQUE)
          State |= GS_DEPTHFUNC_EQUAL;
        EF_SetState(State);
      }
      else
      if (m_RP.m_pCurObject->m_RenderState)
        State = m_RP.m_pCurObject->m_RenderState;
      else
      {
        if (m_RP.m_RendPass || (m_RP.m_ObjFlags & FOB_LIGHTPASS))
          State = slw->m_SecondRenderState;
        else
          State = slw->m_RenderState;
      }
      EF_SetState(State);

      int bFogOverrided = 0;
      bool bFogDisable = (curVP && (curVP->m_Flags & VPFI_NOFOG));
      bool bFogVP = (m_RP.m_PersFlags & RBPF_HDR) || (curVP && (curVP->m_Flags & VPFI_VS30ONLY));
      bFogOverrided = EF_FogCorrection(bFogDisable, bFogVP);

#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
        if (CRenderer::CV_r_log >= 3)
        {
          if (bFog)
            Logv(SRendItem::m_RecurseLevel, "+++ Fog Pass %d\n", m_RP.m_RendPass);
          else
            Logv(SRendItem::m_RecurseLevel, "+++ General Pass %d\n", m_RP.m_RendPass);
        }
#endif
        m_RP.m_pCurObject->SetScissor();

        EF_Draw(ef, slw);

        m_RP.m_RendPass++;

#else //PIPE_USE_INSTANCING

        m_RP.m_RendPass++;

        // Unlock all VB (if needed) and set current streams
        EF_PreDraw(slw);

        int nObj = 0;
        CCObject *pSaveObj = m_RP.m_pCurObject;
        CCObject *pObj = pSaveObj;
        SArrayPointer_Texture pt;
        pt.ePT = eSrcPointer_TexLM;
        pt.Stage = m_RP.m_nLMStage;
        pt.Type = GL_FLOAT;
        pt.NumComponents = 2;
        while (true)
        {       
          if (nObj)
          {
            m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
            m_RP.m_FrameObject++;
            pObj = m_RP.m_pCurObject;

            if (m_RP.m_FlagsModificators & (RBMF_LMTCUSED | RBMF_BENDINFOUSED))
            {
              if (pObj->m_pLMTCBufferO && pObj->m_pLMTCBufferO->m_pVertexBuffer)
              {
                pt.mfSet(1);
              }
            }
            if (!curVP)
            {
              EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
              if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial && !(ef->m_LMFlags & LMF_DISABLE))
                m_RP.m_pShaderResources->m_LMaterial->mfApply(slw->m_LMFlags);
              else
                m_RP.m_CurrentVLights = 0;

              if (m_RP.m_FrameGTC == m_RP.m_Frame)
              {
                for (int nt=0; nt<gRenDev->m_TexMan->m_nCurStages; nt++)
                {
                  if (m_RP.m_pGTC[nt])
                  {
                    EF_SelectTMU(nt);
                    m_RP.m_pGTC[nt]->mfSet(true);
                  }
                }
              }
            }
          }
          pObj->SetScissor();

#ifdef DO_RENDERLOG
          if (CRenderer::CV_r_log >= 3)
          {
            Vec3d vPos = pObj->GetTranslation();
            if (bFog)
              Logv(SRendItem::m_RecurseLevel, "+++ Fog Pass %d (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
            else
              Logv(SRendItem::m_RecurseLevel, "+++ General Pass %d (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, pObj->m_VisId, vPos[0], vPos[1], vPos[2]);
          }
#endif
          // Set per-object alpha-blending
          if (pObj->m_ObjFlags & FOB_HASALPHA)
            pObj->SetAlphaState(slw->m_FShader, State);

          if (curVP)
          {
            curVP->mfSetStateMatrices();
            curVP->mfSetVariables(true, &slw->m_VPParamsObj);
          }

          if (slw->m_FShader)
            slw->m_FShader->mfSetVariables(true, slw->m_CGFSParamsObj);
          else
          {
            if (slw->m_RGBComps && slw->m_RGBComps->m_Comps[0] && slw->m_RGBComps->m_Comps[0]->m_bDependsOnObject)
            {
              float *vals = slw->m_RGBComps->mfGet();
              m_RP.m_NeedGlobalColor.bcolor[0] = (byte)(vals[0] * 255.0f);
              m_RP.m_NeedGlobalColor.bcolor[1] = (byte)(vals[1] * 255.0f);
              m_RP.m_NeedGlobalColor.bcolor[2] = (byte)(vals[2] * 255.0f);
              m_RP.m_NeedGlobalColor.bcolor[3] = (byte)(vals[3] * 255.0f);
            }
            EF_CommitTexStageState();
          }

          {
            //PROFILE_FRAME(Draw_EFIndexMesh);
            if (m_RP.m_pRE)
              m_RP.m_pRE->mfDraw(ef, slw);
            else
              EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
          }
          nObj++;
          if (nObj >= m_RP.m_MergedObjects.Num())
            break;
        }
        EF_FogCorrectionRestore(bFogOverrided);
        if (!(m_RP.m_PersFlags & RBPF_VSWASSET))
        {
          if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
            EF_CommitTexTransforms(false);
        }
        if (pSaveObj != m_RP.m_pCurObject)
        {
          m_RP.m_pCurObject = pSaveObj;
          m_RP.m_FrameObject++;
          if (!curVP)
            EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
        }
#endif
    }

    slw->mfResetTextures();
    EF_ApplyMatrixOps(slw->m_MatrixOps, false);

    if ((m_RP.m_FlagsPerFlush & RBSI_VERTSMERGED) && SUPPORTS_GL_NV_fence)
      glSetFenceNV(m_RP.m_VidBufs[m_RP.m_MergedStreams[0]].m_Fence, GL_ALL_COMPLETED_NV);

    if (slw->m_LMFlags & LMF_POLYOFFSET)
      glDisable(GL_POLYGON_OFFSET_FILL);
  }
}


// Flush HW shader technique (used in FP shaders and in programmable pipeline shaders)
void CGLRenderer::EF_FlushHW()
{
  int i;

  SShader *ef = m_RP.m_pShader;

  int nPolys = m_nPolygons;

  if (m_RP.m_pRE || (m_RP.m_RendNumIndices && m_RP.m_RendNumVerts))
  {
    // Set Z-Bias
    if (ef->m_Flags & EF_POLYGONOFFSET)
    {
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(CV_gl_offsetfactor, CV_gl_offsetunits);
    }

    SShaderTechnique *hs = m_RP.m_pCurTechnique;
    if (!hs)
      return;

    // Check updating of mesh (vertices/indices)
    if (m_RP.m_pRE)
    {
      SRenderShaderResources *Res = m_RP.m_pShaderResources;
      int nFlags = hs->m_Flags;
      if (Res && Res->m_bNeedNormals)
        nFlags |= SHPF_NORMALS;
      if (ef->m_eSort == eS_TerrainLightPass || ef->m_eSort == eS_Terrain)
        nFlags |= FHF_TERRAIN;
      m_RP.m_pRE->mfCheckUpdate(ef->m_VertexFormatId, nFlags);
    }

    if (ef->m_Flags & EF_NEEDNORMALS)
      EF_EvalNormalsRB(ef);
    EF_Eval_DeformVerts(ef->m_Deforms);

    // Set culling mode
    if (!(m_RP.m_FlagsPerFlush & RBSI_NOCULL))
    {
      if (hs->m_eCull != -1)
        GLSetCull(hs->m_eCull);
      else
        GLSetCull(ef->m_eCull);
    }

    if (m_RP.m_pShaderResources)
      m_RP.m_pCurLightMaterial = m_RP.m_pShaderResources->m_LMaterial;
    else
    if (ef->m_Flags2 & EF2_USELIGHTMATERIAL)
      m_RP.m_pCurLightMaterial = &m_RP.m_DefLightMaterial;
    else
      m_RP.m_pCurLightMaterial = NULL;

    EF_SetVertexStreams(hs->m_Pointers, 0);
    EF_ApplyMatrixOps(hs->m_MatrixOps, true);

    // Draw shader passes
    bool bLights = false;
    if (hs->m_Passes.Num())
    {
      int nStart = 0;
      EShaderPassType eShPass = m_SHPTable[hs->m_Passes[0].m_ePassType];

      for (i=1; i<hs->m_Passes.Num(); i++)
      {
        if (m_SHPTable[hs->m_Passes[i].m_ePassType] != eShPass)
        {
          int nEnd = i-1;
          bLights |= EF_DrawPasses(eShPass, hs, ef, nStart, nEnd);
          i = nEnd+1;
          nStart = i;
          if (i >= hs->m_Passes.Num())
            break;
          eShPass = m_SHPTable[hs->m_Passes[i].m_ePassType];
        }
      }
      if (i-nStart > 0)
      {
        int nEnd = i-1;
        bLights |= EF_DrawPasses(eShPass, hs, ef, nStart, nEnd);
      }
    }
    if (bLights)
    {
      if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_SUBSURFACE])
        EF_DrawSubsurfacePasses(hs, ef);
    }

    // Restore matrix transform states
    EF_ApplyMatrixOps(hs->m_MatrixOps, false);

    // Restore Z-Bias
    if (ef->m_Flags & EF_POLYGONOFFSET)
      glDisable(GL_POLYGON_OFFSET_FILL);  

    // Draw detail texture passes
    if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY] && CV_r_detailtextures)
      EF_DrawDetailOverlayPasses();

    // Draw volumetric fog passes
    if (m_RP.m_pFogVolume && CV_r_VolumetricFog)
      EF_DrawFogOverlayPasses();
  }
}

void CGLRenderer::EF_Set2DMode(bool bSet, int orthox, int orthoy)
{
  Set2DMode(bSet, orthox, orthoy);
  EF_SetCameraInfo();
}

// Flush current shader
void CGLRenderer::EF_FlushShader()
{
  SShader *ef = m_RP.m_pShader;
  int i;
  SShaderPass *slw;

  if (m_RP.m_pRE)  // Without buffer fill
    EF_InitEvalFuncs(1);  // VP
  else
  { // Buffer filling (merging case)
    EF_InitEvalFuncs(0);
    if (!m_RP.m_RendNumIndices)
      return;

    if ((ef->m_Flags & EF_FOGSHADER) && !m_RP.m_pFogVolume)
      return; // should never happen

    if (m_RP.m_RendNumIndices > m_RP.m_MaxTris*3)
    {
      iLog->Log("CGLRenderer::EF_FlushShader: Shader MAX_RENDTRIS hit\n");
      m_RP.m_RendNumIndices = m_RP.m_MaxTris*3;
    }
    if (m_RP.m_RendNumVerts > m_RP.m_MaxVerts)
    {
      iLog->Log("CGLRenderer::EF_FlushShader: Shader MAX_RENDVERTS hit\n");
      m_RP.m_RendNumVerts = m_RP.m_MaxVerts;
    }
  }

  m_RP.m_ResourceState = 0;

  // Set overrided states from shader resources
  if (m_RP.m_pShaderResources)
  {
    if (!EF_SetResourcesState(true))
      return;
  }

  // Set overrided states from state shader
  if (m_RP.m_pStateShader && m_RP.m_pStateShader->m_State)
    EF_SetStateShaderState();

  bool bDraw2D = false;
  if (!(m_RP.m_Flags & RBF_2D) && (m_RP.m_FlagsPerFlush & RBSI_DRAWAS2D))
  {
    bDraw2D = true;
    EF_Set2DMode(true,800,600);
  }

  // Draw using HW shader techniques
  if (ef->m_HWTechniques.Num())
  {
    EF_FlushHW();
    if (m_RP.m_pStateShader && m_RP.m_pStateShader->m_State)
      EF_ResetStateShaderState();
    if (m_RP.m_pShaderResources)
      EF_SetResourcesState(false);
    if (bDraw2D)
      EF_Set2DMode(false,800,600);
    return;
  }

  if ((ef->m_Flags & EF_FOGSHADER) && !m_RP.m_pFogVolume)
  {
    // Should never happen
    if (m_RP.m_pStateShader && m_RP.m_pStateShader->m_State)
      EF_ResetStateShaderState();
    if (m_RP.m_pShaderResources)
      EF_SetResourcesState(false);
    if (bDraw2D)
      EF_Set2DMode(false,800,600);
    return;
  }
  // Update mesh if necessary (create video streams / streaming)
  if (m_RP.m_pRE)
  {
    int nFlags = 0;
    if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_bNeedNormals)
      nFlags |= SHPF_NORMALS;
    if (!m_RP.m_pRE->mfCheckUpdate(ef->m_VertexFormatId, nFlags))
      return;
  }

  // In common shaders we don't have pixel or vertex shaders
  m_RP.m_PersFlags &= ~(RBPF_PS1NEEDSET | RBPF_PS2NEEDSET | RBPF_TSNEEDSET | RBPF_VSNEEDSET);
  CGLTexMan::BindNULL(1);
  EF_SelectTMU(0);

  if (ef->m_Flags & EF_NEEDNORMALS)
    EF_EvalNormalsRB(ef);
  EF_Eval_DeformVerts(ef->m_Deforms);

  SArrayPointer::m_CurEnabled = PFE_POINTER_VERT;

  if (ef->m_Flags & EF_POLYGONOFFSET)
  {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(CV_gl_offsetfactor, CV_gl_offsetunits);
  }

  if (!(m_RP.m_FlagsPerFlush & RBSI_NOCULL))
    GLSetCull(ef->m_eCull);

  if (!m_RP.m_pRE)
  {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, m_RP.m_Stride, m_RP.m_Ptr.PtrB + m_RP.m_OffsT);

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, m_RP.m_Stride, m_RP.m_Ptr.PtrB + m_RP.m_OffsD);

    glVertexPointer(3, GL_FLOAT, m_RP.m_Stride, m_RP.m_Ptr.PtrB);
    if (m_RP.m_FT & FLT_N)
    {
      glNormalPointer(GL_FLOAT, m_RP.m_Stride, m_RP.m_Ptr.PtrB+m_RP.m_OffsN);
      glEnableClientState(GL_NORMAL_ARRAY);
    }
  }

  if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial)
    m_RP.m_pShaderResources->m_LMaterial->mfApply(ef->m_LMFlags);
  else
    m_RP.m_CurrentVLights = 0;

  int nPolys = m_nPolygons;

  slw = &ef->m_Passes[0];
  m_RP.m_StatNumPasses += ef->m_Passes.Num();
  // Draw all passes
  for (i=0; i<ef->m_Passes.Num(); i++, slw++)
  {
    EF_Eval_TexGen(slw);
    EF_Eval_RGBAGen(slw);

    // Set all textures and HW TexGen modes for the current pass (ShadeLayer)
    if (slw->mfSetTextures())
    {
      // Set Render states for the current pass
      int nState;
      if (m_RP.m_RendPass || (m_RP.m_pCurObject->m_ObjFlags & FOB_LIGHTPASS))
        nState = slw->m_SecondRenderState;
      else
        nState = slw->m_RenderState;
      EF_SetState(nState);

#ifndef PIPE_USE_INSTANCING

#ifdef DO_RENDERLOG
      if (CRenderer::CV_r_log >= 3)
        Logv(SRendItem::m_RecurseLevel, "+++ Pass %d \n", m_RP.m_RendPass);
#endif
      // Set scissor rect
      m_RP.m_pCurObject->SetScissor();

      EF_Draw(ef, slw);

      m_RP.m_RendPass++;
#else
      m_RP.m_RendPass++;

      int bFogOverrided = 0;
      // Unlock all VB (if needed) and set current streams
      EF_PreDraw(slw);
      if (m_FS.m_bEnable)
        bFogOverrided = EF_FogCorrection(false, false);

      int nObj = 0;
      CCObject *pSaveObj = m_RP.m_pCurObject;
      CCObject *pObj = pSaveObj;
      bool bLightSicssorWasSet = false;

      // Set scissor rect
      pObj->SetScissor();
      while (true)
      {       
        if (nObj)
        {
          // Object changed
          m_RP.m_pCurObject = m_RP.m_MergedObjects[nObj];
          pObj = m_RP.m_pCurObject;
          m_RP.m_FrameObject++;
          pObj->SetScissor();
          EF_SetObjectTransform(pObj, ef, pObj->m_ObjFlags);
          if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_LMaterial)
            m_RP.m_pShaderResources->m_LMaterial->mfApply(ef->m_LMFlags);
        }

#ifdef DO_RENDERLOG
        if (CRenderer::CV_r_log >= 3)
        {
          Vec3d vPos = pObj->GetTranslation();
          Logv(SRendItem::m_RecurseLevel, "+++ Pass %d (Obj: %d [%.3f, %.3f, %.3f])\n", m_RP.m_RendPass, m_RP.m_pCurObject->m_VisId, vPos[0], vPos[1], vPos[2]);
        }
#endif

        // Set per-object alpha blending state
        if (pObj->m_ObjFlags & FOB_HASALPHA)
          pObj->SetAlphaState(NULL, nState);

        // Commit per-texture states / color ops
        EF_CommitTexStageState();

        {
          //PROFILE_FRAME(Draw_EFIndexMesh);
          if (m_RP.m_pRE)
            m_RP.m_pRE->mfDraw(ef, slw);
          else
            EF_DrawIndexedMesh(R_PRIMV_TRIANGLES);
        }
        nObj++;
        if (nObj >= m_RP.m_MergedObjects.Num())
          break;
      }
      EF_FogCorrectionRestore(bFogOverrided);
      if (!m_RP.m_LastVP)
      {
        if (m_RP.m_FlagsModificators & (RBMF_TCM | RBMF_TCG))
          EF_CommitTexTransforms(false);
      }
      if (pSaveObj != m_RP.m_pCurObject)
      {
        m_RP.m_pCurObject = pSaveObj;
        m_RP.m_FrameObject++;
        EF_SetObjectTransform(pSaveObj, ef, pSaveObj->m_ObjFlags);
      }
#endif
    }
    slw->mfResetTextures();
  }

  if (m_RP.m_pShaderResources && m_RP.m_pShaderResources->m_Textures[EFTT_DETAIL_OVERLAY] && CV_r_detailtextures)
    EF_DrawDetailOverlayPasses();

  if (m_RP.m_pFogVolume && CV_r_VolumetricFog)
    EF_DrawFogOverlayPasses();

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  if ((m_RP.m_FT & FLT_N) && !m_RP.m_pRE)
    glDisableClientState(GL_NORMAL_ARRAY);

  if ((m_RP.m_FlagsPerFlush & RBSI_VERTSMERGED) && SUPPORTS_GL_NV_fence)
    glSetFenceNV(m_RP.m_VidBufs[m_RP.m_MergedStreams[0]].m_Fence, GL_ALL_COMPLETED_NV);

  if (ef->m_Flags & EF_POLYGONOFFSET)
    glDisable(GL_POLYGON_OFFSET_FILL);

  if (m_RP.m_pStateShader && m_RP.m_pStateShader->m_State)
    EF_ResetStateShaderState();
  if (m_RP.m_pShaderResources)
    EF_SetResourcesState(false);

  if (bDraw2D)
    EF_Set2DMode(false,800,600);

  SArrayPointer_Texture::m_pLastPointer[0] = NULL;
  SArrayPointer::m_LastEnabled = 0;
  SArrayPointer_Vertex::m_pLastPointer = NULL;
  SArrayPointer_Normal::m_pLastPointer = NULL;
  SArrayPointer_Color::m_pLastPointer = NULL;
}

// Flush current render item
void CGLRenderer::EF_Flush()
{
  CGLRenderer *rd = gcpOGL;
  SShader *ef = rd->m_RP.m_pShader;
  if (rd->m_RP.m_ExcludeShader)
  {
    if (!strcmp(rd->m_RP.m_ExcludeShader, ef->m_Name.c_str()))
      return;
    if(rd->m_RP.m_ExcludeShader[0] == '#')
    if (strstr(ef->m_Name.c_str(), rd->m_RP.m_ExcludeShader))
      return;
  }
  if (rd->m_RP.m_ShowOnlyShader)
  {
    if (strcmp(rd->m_RP.m_ShowOnlyShader, ef->m_Name.c_str()))
      return;
  }
  /*{
    Vec3d vObj = rd->m_RP.m_pCurObject->GetTranslation();
    Vec3d vv = Vec3d(1452.625, 1345.750, 27.500);
    if ((vv-vObj).GetLength() > 0.1f)
    return;
  }*/
  //if ((int)rd->m_RP.m_pRE != 0x1261c840)
  //  return;

  if (ef->m_nPreprocess)
  {
    if (ef->m_nPreprocess & FSPR_PORTAL)
    {
      if (CV_r_portals && (rd->m_RP.m_PersFlags & (RBPF_DRAWPORTAL | RBPF_DRAWMIRROR)))
        return;
    }
    if(ef->m_nPreprocess & FSPR_HEATVIS)
    {
      if (rd->m_RP.m_PersFlags & RBPF_DRAWHEATMAP)
        rd->m_TexMan->EndHeatMap();
    }
    if (ef->m_nPreprocess & FSPR_SCANSCR)
    {
      if (rd->m_RP.m_PersFlags & RBPF_DRAWSCREENMAP)
        rd->m_TexMan->EndScreenMap();
    }
    if (ef->m_nPreprocess & FSPR_SCREENTEXMAP)
    {
      if (rd->m_RP.m_PersFlags & RBPF_DRAWSCREENTEXMAP)
      {
        // check when refraction is active.. 
        rd->m_TexMan->EndScreenTexMap();
        // disable flag
        if(rd->m_RP.m_PersFlags& RBPF_IGNOREREFRACTED)
          rd->m_RP.m_PersFlags&= ~RBPF_IGNOREREFRACTED;
      }
    }
  }

  if (rd->m_LogFile && CV_r_log == 3)
    rd->Logv(SRendItem::m_RecurseLevel, "--------------------------------- Start Flush: '%s' -----------------------------------\n", ef->m_Name.c_str());

  rd->m_RP.m_StatNumLightPasses = 0;
  rd->m_RP.m_StatNumLights = 0;
  rd->m_RP.m_StatNumPasses = 0;
  rd->m_RP.m_StatLightMask = 0;

  rd->m_RP.m_PS.m_NumRendBatches++;
#ifdef PIPE_USE_INSTANCING
  if (rd->m_RP.m_MergedObjects.Num())
    rd->m_RP.m_PS.m_NumRendItems += rd->m_RP.m_MergedObjects.Num();
  else
#endif
    rd->m_RP.m_PS.m_NumRendItems++;

  CCObject *obj = rd->m_RP.m_pCurObject;
  int VPMId, IMId;
  if (!(rd->m_RP.m_ObjFlags & FOB_TRANS_MASK))
  {
    VPMId = rd->m_RP.m_pCurObject->m_VPMatrixId;
    IMId = rd->m_RP.m_pCurObject->m_InvMatrixId;
    rd->m_RP.m_pCurObject->m_VPMatrixId = 0;
    rd->m_RP.m_pCurObject->m_InvMatrixId = 0;
  }

  rd->m_RP.m_DynLMask = CV_r_hwlights ? obj->m_DynLMMask : 0;
  if (!(ef->m_Flags & EF_HASVSHADER))
    rd->EF_SetObjectTransform(obj, ef, obj->m_ObjFlags);
  
  if (CV_r_showlight->GetString()[0] != '0')
  {
    for (int i=0; i<rd->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); i++)
    {
      CDLight *dl = rd->m_RP.m_DLights[SRendItem::m_RecurseLevel][i];
      if (!dl)
        continue;
      char *str = CV_r_showlight->GetString();
      if (strcmp(str, "0") != 0)
      {
        if (!dl->m_Name || !strstr(dl->m_Name, str))
          rd->m_RP.m_DynLMask &= ~(1<<i);
      }
    }
  }
  if ((ef->m_Flags3 & EF3_CLIPPLANE) && !(rd->m_RP.m_PersFlags & RBPF_SETCLIPPLANE) && !rd->m_RP.m_ClipPlaneEnabled)
  {
    switch (ef->m_Flags3 & EF3_CLIPPLANE)
    {
      case EF3_CLIPPLANE_WATER_FRONT:
      case EF3_CLIPPLANE_WATER_BACK:
        {
          SPlane Pl;
          Pl.m_Normal = Vec3d (0, 0, 1);
          Pl.m_Dist = iSystem->GetI3DEngine()->GetWaterLevel();
          if (ef->m_Flags3 & EF3_CLIPPLANE_WATER_BACK)
          {
            Pl.m_Normal = -Pl.m_Normal;
            Pl.m_Dist = -Pl.m_Dist;
          }
          Pl.m_Dist = -Pl.m_Dist;
          rd->EF_SetClipPlane(true, &Pl.m_Normal.x, false);
          rd->m_RP.m_PersFlags |= RBPF_SETCLIPPLANE;
        }
    }
  }
  else
  if (!(ef->m_Flags3 & EF3_CLIPPLANE) && (rd->m_RP.m_PersFlags & RBPF_SETCLIPPLANE))
  {
    rd->EF_SetClipPlane(false, NULL, false);
    rd->m_RP.m_PersFlags &= ~RBPF_SETCLIPPLANE;
  }

  bool bProfile;
  double time0 = 0;
  int nNumPolys = rd->m_nPolygons;
  if (CRenderer::CV_r_profileshaders || CRenderer::CV_r_log>=2)
  {
    ticks(time0);
    bProfile = true;
  }
  else
    bProfile = false;

  if (rd->m_LogFile && CV_r_log >= 3)
    rd->Logv(SRendItem::m_RecurseLevel, "\n");

  rd->EF_FlushShader();

  if (rd->m_RP.m_DynLMask)
    rd->m_RP.m_PS.m_NumLitShaders++;

#ifdef DO_RENDERLOG
  nNumPolys = rd->m_nPolygons - nNumPolys;

  if (bProfile)
  {
    unticks(time0);
    time0 = time0*1000.0*g_SecondsPerCycle;
  }

  if (CRenderer::CV_r_profileshaders)
  {
    SProfInfo pi;
    pi.Time = time0;
    pi.NumPolys = nNumPolys;
    pi.ef = ef;
    pi.m_nItems = 0;
    rd->m_RP.m_Profile.AddElem(pi);
  }

  if (rd->m_RP.m_DynLMask)
    rd->m_RP.m_PS.m_NumLitShaders++;

  if (rd->m_LogFile)
  {
    if (CV_r_log >= 3 && rd->m_RP.m_DynLMask)
    {
      for (int n=0; n<rd->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); n++)
      {
        CDLight *dl = rd->m_RP.m_DLights[SRendItem::m_RecurseLevel][n];
        if (rd->m_RP.m_DynLMask & (1<<n))
          rd->Logv(SRendItem::m_RecurseLevel, "   Light %d (\"%s\")\n", n, dl->m_Name ? dl->m_Name : "<Unknown>");
      }
    }

    GLenum err = glGetError();
    int errnum=0;
    while (err != GL_NO_ERROR)
    {
      char * pErrText = (char*)rd->ErrorString(err);
      rd->Logv(SRendItem::m_RecurseLevel, "glGetError: %s:",  pErrText ? pErrText : "-");
      rd->Logv(SRendItem::m_RecurseLevel, "\n");

      err = glGetError();
      if (++errnum>10)
        break;
    }

    char *str;
    if (ef->m_HWTechniques.Num())
      str = "FlushHW";
    else
      str = "Flush";
    rd->Logv(SRendItem::m_RecurseLevel, "%s: '%s', (St: %s) Id:%d, ResId:%d, Obj:%d, Tech: %d, Cp: %d, Fog:%d, VF:%d, NL:%d, LPas:%d, Pas:%d (time: %f, %d polys)\n", str, ef->m_Name.c_str(), rd->m_RP.m_pStateShader ? rd->m_RP.m_pStateShader->m_Name.c_str() : "NULL", ef->m_Id, rd->m_RP.m_pShaderResources ? rd->m_RP.m_pShaderResources->m_Id : -1, rd->m_RP.m_pCurObject->m_VisId, rd->m_RP.m_pCurTechnique ? rd->m_RP.m_pCurTechnique->m_Id : -1, rd->m_RP.m_ClipPlaneEnabled, rd->m_RP.m_pFogVolume ? (rd->m_RP.m_pFogVolume-&rd->m_RP.m_FogVolumes[0]) : 0, rd->m_RP.m_pShader->m_VertexFormatId, rd->m_RP.m_StatNumLights, rd->m_RP.m_StatNumLightPasses, rd->m_RP.m_StatNumPasses, time0, nNumPolys);
    if (rd->m_RP.m_ObjFlags & FOB_SELECTED)
    {
      if (rd->m_RP.m_FlagsPerFlush & RBSI_ALPHATEST)
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x), LM: %d, (AT) (Selected)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask);
      else
      if (rd->m_RP.m_FlagsPerFlush & RBSI_ALPHABLEND)
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x) (AB), LM: %d (Dist: %.3f) (Selected)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask, rd->m_RP.m_pRE ? rd->m_RP.m_pRE->mfDistanceToCameraSquared(*gRenDev->m_RP.m_pCurObject) : 0);
      else
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x), RE: 0x%x, LM: 0x%x (Selected)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_pRE, rd->m_RP.m_DynLMask);
    }
    else
    {
      if (rd->m_RP.m_FlagsPerFlush & RBSI_ALPHATEST)
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x), LM: %d, (AT)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask);
      else
      if (rd->m_RP.m_FlagsPerFlush & RBSI_ALPHABLEND)
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x) (AB), LM: %d (Dist: %.3f)\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_DynLMask, rd->m_RP.m_pRE ? rd->m_RP.m_pRE->mfDistanceToCameraSquared(*gRenDev->m_RP.m_pCurObject) : 0);
      else
        rd->Logv(SRendItem::m_RecurseLevel, "  %.3f, %.3f, %.3f (0x%x), RE: 0x%x, LM: 0x%x\n", rd->m_RP.m_pCurObject->m_Matrix(3,0), rd->m_RP.m_pCurObject->m_Matrix(3,1), rd->m_RP.m_pCurObject->m_Matrix(3,2), rd->m_RP.m_pCurObject->m_ObjFlags, rd->m_RP.m_pRE, rd->m_RP.m_DynLMask);
    }
  }
#endif
}

// Process all render item lists
void CGLRenderer::EF_EndEf3D(int nFlags)
{
  double time0 = 0;
  ticks(time0);

  assert(SRendItem::m_RecurseLevel >= 1);
  if (SRendItem::m_RecurseLevel < 1)
  {
    iLog->Log("Error: CRenderer::EF_EndEf3D without CRenderer::EF_StartEf");
    return;
  }

  if (CV_r_nodrawshaders == 1)
  {
    SetClearColor(Vec3d(0,0,0));
    EF_ClearBuffers(false, false);
    SRendItem::m_RecurseLevel--;
    return;
  }

  m_RP.m_PersFlags &= ~(RBPF_DRAWNIGHTMAP | RBPF_DRAWHEATMAP);
  m_RP.m_RealTime = iTimer->GetCurrTime();

  if (CV_r_fullbrightness)  
  {
    m_RP.m_NeedGlobalColor.dcolor = -1;
    m_RP.m_FlagsPerFlush |= RBSI_RGBGEN | RBSI_ALPHAGEN;
  }

  if (CV_r_excludeshader->GetString()[0] != '0')
    m_RP.m_ExcludeShader = CV_r_excludeshader->GetString();
  else
    m_RP.m_ExcludeShader = NULL;

  if (CV_r_showonlyshader->GetString()[0] != '0')
    m_RP.m_ShowOnlyShader = CV_r_showonlyshader->GetString();
  else
    m_RP.m_ShowOnlyShader = NULL;

  EF_UpdateSplashes(m_RP.m_RealTime);
  EF_AddClientPolys3D();
  EF_AddClientPolys2D();

  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_PREPROCESS_ID] = SRendItem::m_RendItems[EFSLIST_PREPROCESS_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_STENCIL_ID] = SRendItem::m_RendItems[EFSLIST_STENCIL_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID] = SRendItem::m_RendItems[EFSLIST_GENERAL_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_UNSORTED_ID] = SRendItem::m_RendItems[EFSLIST_UNSORTED_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_DISTSORT_ID] = SRendItem::m_RendItems[EFSLIST_DISTSORT_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_LAST_ID] = SRendItem::m_RendItems[EFSLIST_LAST_ID].Num();

  EF_RenderPipeLine(EF_Flush);

  EF_DrawDebugTools();
  EF_RemovePolysFromScene();
  SRendItem::m_RecurseLevel--;

  unticks(time0);
  m_RP.m_PS.m_fFlushTime += (float)(time0*1000.0*g_SecondsPerCycle);
}

// Process all render item lists
void CGLRenderer::EF_RenderPipeLine(void (*RenderFunc)())
{
  EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_PREPROCESS_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_PREPROCESS_ID], EFSLIST_PREPROCESS_ID, RenderFunc);  // Preprocess and probably sky
  if (!(m_RP.m_PersFlags & RBPF_IGNORERENDERING))
  {
    EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_STENCIL_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_STENCIL_ID], EFSLIST_STENCIL_ID, RenderFunc);   // Unsorted list for indoor
    EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID], EFSLIST_GENERAL_ID, RenderFunc);    // Sorted list without preprocess
    EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_UNSORTED_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_UNSORTED_ID], EFSLIST_UNSORTED_ID, RenderFunc); // Unsorted list
    EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_DISTSORT_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_DISTSORT_ID], EFSLIST_DISTSORT_ID, RenderFunc);   // Sorted by distance elements
    if (SRendItem::m_RecurseLevel <= 1)
      EF_PipeLine(SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_LAST_ID], SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_LAST_ID], EFSLIST_LAST_ID, RenderFunc);       // Sorted list without preprocess of all fog passes and screen shaders
  }
  else
    m_RP.m_PersFlags &= ~RBPF_IGNORERENDERING;
}

static char *sDescList[] = {"General", "DistSort", "Preprocess", "Unsorted", "Last"};

// Process render items list [nList] from [nums] to [nume]
// 1. Sorting of the list
// 2. Preprocess shaders handling
// 3. Process sorted ordered list of render items
void CGLRenderer::EF_PipeLine(int nums, int nume, int nList, void (*RenderFunc)())
{
  int i;
  CCObject *pSavedObj;
  SShader *pShader, *pCurShader, *pShaderState, *pCurShaderState;
  SRenderShaderResources *pRes, *pCurRes;
  int nObject, nCurObject;
  int nFog, nCurFog;

  if (nume-nums < 1)
    return;
  
  m_RP.m_pRenderFunc = RenderFunc;

  m_RP.m_nCurLightParam = -1;
  pSavedObj = m_RP.m_pCurObject;
  m_RP.m_pCurObject = m_RP.m_VisObjects[0];
  m_RP.m_pPrevObject = m_RP.m_pCurObject;
  
  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "*** Start frame for list %s ***\n", sDescList[nList]);

  EF_PreRender(1);

  if (nList==EFSLIST_PREPROCESS_ID || nList==EFSLIST_GENERAL_ID || nList==EFSLIST_LAST_ID)
  {
    {
      PROFILE_FRAME(State_Sorting);
      SRendItem::mfSort(&SRendItem::m_RendItems[nList][nums], nume-nums);
    }
    
    // If sort number of the first shader is 1 (eS_Preprocess)
    // run preprocess operations for the current frame
    if ((SRendItem::m_RendItems[nList][nums].SortVal.i.High >> 26) == eS_PreProcess)
      nums += EF_Preprocess(&SRendItem::m_RendItems[nList][0], nums, nume);
    if (m_RP.m_PersFlags & RBPF_IGNORERENDERING)
      return;
  }
  else
  if (nList==EFSLIST_DISTSORT_ID)
  {
    PROFILE_FRAME(State_SortingDist);
    SRendItem::mfSortByDist(&SRendItem::m_RendItems[nList][nums], nume-nums);
  }
#ifdef PIPE_USE_INSTANCING
  else
  if (nList==EFSLIST_STENCIL_ID)
  {
    PROFILE_FRAME(State_Sorting);
    int nStart = nums;
    for (i=nums; i<nume; i++)
    {
      SRendItemPre *ri = &SRendItem::m_RendItems[nList][i];
      int Type = ri->Item->mfGetType();
      if (Type == eDATA_ClearStencil || Type == eDATA_TriMeshShadow)
      {
        SRendItem::mfSortForStencil(&SRendItem::m_RendItems[nList][nStart], i-nStart);
        do
        {
          i++;
          if (i >= nume)
            break;
          ri = &SRendItem::m_RendItems[nList][i];
          Type = ri->Item->mfGetType();
        } while(Type == eDATA_ClearStencil || Type == eDATA_TriMeshShadow);
        nStart = i;
        i--;
      }
    }
    SRendItem::mfSortForStencil(&SRendItem::m_RendItems[nList][nStart], i-nStart);
  }
#endif

  EF_PreRender(3);

//  if (!m_RP.m_CurPortal)
//    return;

  m_RP.m_Flags |= RBF_3D;

  glPushMatrix();

  /*float plane[4];
  {
    plane[0] = 0;
    plane[1] = 0;
    plane[2] = 1;
    plane[3] = -50;
    EF_SetClipPlane(true, plane, false);
  }*/

  PROFILE_FRAME(DrawShader_PipeLine);

  UnINT64 oldVal;
  oldVal.SortVal = -1;
  nCurObject = -2;
  m_RP.m_pCurObject = m_RP.m_VisObjects[0];
  m_RP.m_pPrevObject = m_RP.m_pCurObject;
  nCurFog = 0;
  pCurShader = NULL;
  pCurShaderState = NULL;
  pCurRes = NULL;
  bool bIgnore = false;
  bool bChanged;
  bool bUseBatching = (RenderFunc == EF_Flush);

  for (i=nums; i<nume; i++)
  {
    SRendItemPre *ri = &SRendItem::m_RendItems[nList][i];
    CRendElement *pRE = ri->Item;
#ifndef PIPE_USE_INSTANCING
    if (ri->SortVal.SortVal == oldVal.SortVal)
    {
      if (bIgnore)
        continue;
      // Optimizations: not necessary to check of changing shaders and objects
      // if sort value is the same (contains the same info about shaders, objects, fog volumes, ...)
      {
        //PROFILE_FRAME_TOTAL(Mesh_REPrepare);
        re->mfPrepare();
      }
      continue;
    }
    oldVal.SortVal = ri->SortVal.SortVal;
    SRendItem::mfGet(ri->SortVal, &nObject, &pShader, &pShaderState, &nFog, &pRes);
    bChanged = (pShader != pCurShader || pCurRes != pRes || pShaderState != pCurShaderState || nFog != nCurFog);
#else
    if (oldVal.i.High == ri->SortVal.i.High && !((oldVal.i.Low ^ ri->SortVal.i.Low) & 0x000fffff))
    {
      SRendItem::mfGetObj(ri->SortVal, &nObject);
      bChanged = false;
    }
    else
    {
      SRendItem::mfGet(ri->SortVal, &nObject, &pShader, &pShaderState, &nFog, &pRes);
      bChanged = true;
    }
    oldVal.SortVal = ri->SortVal.SortVal;
#endif
    if (nObject != nCurObject)
    {
      if (!bChanged && !pShader->m_Deforms && bUseBatching)
      {
        if (EF_TryToMerge(nObject, nCurObject, pRE))
          continue;
      }
      if (pCurShader)
      {
        m_RP.m_pRenderFunc();
        pCurShader = NULL;
        bChanged = true;
      }
      if (!EF_ObjectChange(pShader, pRes, nObject, pRE))
      {
        bIgnore = true;
        continue;
      }
      bIgnore = false;
      nCurObject = nObject;
    }

    if (bChanged)
    {
      if (pCurShader)
        m_RP.m_pRenderFunc();      
      EF_Start(pShader, pShaderState, pRes, nFog, pRE);
      nCurFog = nFog;
      pCurShader = pShader;
      pCurShaderState = pShaderState;
      pCurRes = pRes;
    }

    {
      //PROFILE_FRAME_TOTAL(Mesh_REPrepare);
      pRE->mfPrepare();
    }
  }
  if (pCurShader)
    m_RP.m_pRenderFunc();

  m_RP.m_pRE = NULL;

  if (m_RP.m_PersFlags & RBPF_SETCLIPPLANE)
  {
    EF_SetClipPlane(false, NULL, false);
    m_RP.m_PersFlags &= ~RBPF_SETCLIPPLANE;
  }
  
  glDepthRange(m_RP.m_fMinDepthRange, m_RP.m_fMaxDepthRange);
  
  EF_ObjectChange(NULL, NULL, 0, NULL);

  m_RP.m_PersFlags &= ~RBPF_MATRIXNOTLOADED;
  glPopMatrix();

  //EF_SetClipPlane(false, plane, false);

  SArrayPointer::m_CurEnabled = PFE_POINTER_VERT;
  m_RP.m_PersFlags &= ~(RBPF_PS1NEEDSET | RBPF_PS2NEEDSET | RBPF_TSNEEDSET | RBPF_VSNEEDSET);
  m_RP.m_CurrentVLights = 0;
  m_RP.m_FlagsModificators = 0;
  m_RP.m_FlagsPerFlush = 0;
  EF_CommitPS();
  EF_CommitVS();
  EF_CommitStreams();
  EF_CommitVLights();
  m_RP.m_Flags &= ~RBF_3D;
  CGLTexMan::BindNULL(1);
  EF_SelectTMU(0);
  EF_Scissor(false, 0, 0, 0, 0);
  m_RP.m_pShader = NULL;

#ifdef DO_RENDERLOG
  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "*** End pipeline list ***\n\n");
#endif
  
  m_RP.m_pCurObject = pSavedObj;
}

void CGLRenderer::EF_DrawWire()
{
  if (CV_r_showlines == 1)
    gcpOGL->EF_SetState(GS_POLYLINE | GS_NODEPTHTEST);
  else
  if (CV_r_showlines == 2)
    gcpOGL->EF_SetState(GS_POLYLINE);
  gcpOGL->SetMaterialColor(1,1,1,1);
  gRenDev->m_TexMan->m_Text_White->Set();
  gcpOGL->EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
  if (gcpOGL->m_RP.m_pRE)
    gcpOGL->m_RP.m_pRE->mfCheckUpdate(gcpOGL->m_RP.m_pShader->m_VertexFormatId, 0);
  gcpOGL->EF_SetObjectTransform(gcpOGL->m_RP.m_pCurObject, NULL, gcpOGL->m_RP.m_pCurObject->m_ObjFlags);
  int StrVrt;
  void *verts = (void *)gcpOGL->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, 0);
  glVertexPointer(3, GL_FLOAT, StrVrt, verts);
  SArrayPointer_Vertex::m_pLastPointer = verts;
  glEnableClientState(GL_VERTEX_ARRAY);
  gcpOGL->EF_Draw(gcpOGL->m_RP.m_pShader, NULL);
}

void CGLRenderer::EF_DrawNormals()
{
  gcpOGL->EF_SetObjectTransform(gcpOGL->m_RP.m_pCurObject, NULL, gcpOGL->m_RP.m_pCurObject->m_ObjFlags);
  float len = CRenderer::CV_r_normalslength;
  if (gcpOGL->m_bEditor)
    len *= 100.0f;
  if (gcpOGL->m_RP.m_pRE)
    gcpOGL->m_RP.m_pRE->mfCheckUpdate(gcpOGL->m_RP.m_pShader->m_VertexFormatId, SHPF_NORMALS);
  int StrVrt, StrNrm;
  int flags = CV_r_shownormals==1 ? (FGP_SRC | FGP_REAL) : (FGP_REAL);
  byte *verts = (byte *)gcpOGL->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, flags);
  byte *norms = (byte *)gcpOGL->EF_GetPointer(eSrcPointer_Normal, &StrNrm, GL_FLOAT, eSrcPointer_Normal, flags);
  if (((UINT_PTR)norms|(UINT_PTR)verts) > 256)
  {
    int numVerts = gcpOGL->m_RP.m_RendNumVerts;
    float e[3];

    gRenDev->m_TexMan->m_Text_White->Set();
    gcpOGL->EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
    
    gRenDev->EF_SetState(GS_POLYLINE);
    
    glBegin(GL_LINES);
    for (int v=0; v<numVerts; v++,verts+=StrVrt,norms+=StrNrm)
    {
      float *fverts = (float *)verts;
      float *fnorms = (float *)norms;
      glColor3f(1, 0, 0);
      glVertex3fv(fverts);
      
      e[0] = fverts[0] + fnorms[0]*len;
      e[1] = fverts[1] + fnorms[1]*len;
      e[2] = fverts[2] + fnorms[2]*len;
      glColor3f(1, 1, 1);
      glVertex3fv(e);
    }
    glEnd();

    gRenDev->EF_SetState(GS_DEPTHWRITE);
  }
}

void CGLRenderer::EF_DrawTangents()
{
  gcpOGL->EF_SetObjectTransform(gcpOGL->m_RP.m_pCurObject, NULL, gcpOGL->m_RP.m_pCurObject->m_ObjFlags);
  //tangent, binormal, normal
  float len = CRenderer::CV_r_normalslength;
  if (gcpOGL->m_bEditor)
    len *= 100.0f;
  if (gcpOGL->m_RP.m_pRE)
    gcpOGL->m_RP.m_pRE->mfCheckUpdate(gcpOGL->m_RP.m_pShader->m_VertexFormatId, SHPF_TANGENTS);
  int StrVrt, StrTang, StrBinorm, StrTNorm;
  byte *verts = NULL;
  byte *tangs = NULL;
  byte *binorm = NULL;
  byte *tnorm = NULL;
  int flags = 0;
  if (CRenderer::CV_r_showtangents == 1)
    flags = FGP_SRC | FGP_REAL;
  else
    flags = FGP_REAL;
  verts = (byte *)gcpOGL->EF_GetPointer(eSrcPointer_Vert, &StrVrt, GL_FLOAT, eSrcPointer_Vert, flags);
  tangs = (byte *)gcpOGL->EF_GetPointer(eSrcPointer_Tangent, &StrTang, GL_FLOAT, eSrcPointer_Tangent, flags);
  binorm = (byte *)gcpOGL->EF_GetPointer(eSrcPointer_Binormal, &StrBinorm, GL_FLOAT, eSrcPointer_Binormal, flags);
  tnorm = (byte *)gcpOGL->EF_GetPointer(eSrcPointer_TNormal, &StrTNorm, GL_FLOAT, eSrcPointer_TNormal, flags);
  if (((UINT_PTR)tangs|(UINT_PTR)binorm|(UINT_PTR)tnorm)>256)
  {
    int numVerts = gcpOGL->m_RP.m_RendNumVerts;
    float e[3];
    
    gRenDev->m_TexMan->m_Text_White->Set();
    gcpOGL->EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
    
    if (gcpOGL->m_polygon_mode == R_SOLID_MODE)
      gcpOGL->EF_SetState(GS_POLYLINE);
    else
      gcpOGL->EF_SetState(0);
    
    glBegin(GL_LINES);
    for (int v=0; v<numVerts; v++,verts+=StrVrt,tangs+=StrTang, binorm+=StrBinorm, tnorm+=StrTNorm)
    {
      float *fverts = (float *)verts;
      float *fv = (float *)tangs;
      glColor3f(1, 0, 0);
      glVertex3fv(fverts);
      
      e[0] = fverts[0] + fv[0]*len/2;
      e[1] = fverts[1] + fv[1]*len/2;
      e[2] = fverts[2] + fv[2]*len/2;
      glColor3f(1, 1, 1);
      glVertex3fv(e);

      fv = (float *)binorm;
      glColor3f(0, 1, 0);
      glVertex3fv(fverts);
      
      e[0] = fverts[0] + fv[0]*len/2;
      e[1] = fverts[1] + fv[1]*len/2;
      e[2] = fverts[2] + fv[2]*len/2;
      glColor3f(1, 1, 1);
      glVertex3fv(e);

      fv = (float *)tnorm;
      glColor3f(0, 0, 1);
      glVertex3fv(fverts);
      
      e[0] = fverts[0] + fv[0]*len/2;
      e[1] = fverts[1] + fv[1]*len/2;
      e[2] = fverts[2] + fv[2]*len/2;
      glColor3f(1, 1, 1);
      glVertex3fv(e);
    }
    glEnd();
  }
  gcpOGL->EF_SetState(GS_DEPTHWRITE);
}


void CGLRenderer::EF_DrawDebugLights()
{
  static int sFrame = 0;

  if (m_nFrameID != sFrame)
  {
    int i;
    sFrame = m_nFrameID;

    for (i=0; i<m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); i++)
    {
      CDLight *dl = m_RP.m_DLights[SRendItem::m_RecurseLevel][i];
      if (!dl)
        continue;
      char *str = CV_r_showlight->GetString();
      if (strcmp(str, "0") != 0)
      {
        if (!dl->m_Name || !strstr(dl->m_Name, str))
          continue;
      }
      SetMaterialColor(dl->m_Color[0], dl->m_Color[1], dl->m_Color[2], dl->m_Color[3]);
      if (dl->m_Flags & DLF_DIRECTIONAL)
        DrawPoint(dl->m_Origin[0], dl->m_Origin[1], dl->m_Origin[2], 10);
      else
      if (dl->m_Flags & DLF_POINT)
        DrawBall(dl->m_Origin[0], dl->m_Origin[1], dl->m_Origin[2], 0.05f);
      if (dl->m_Flags & DLF_PROJECT)
      {
        Vec3d dir, rgt, org;
        EF_SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);

        dir = dl->m_Orientation.m_vForward;
        rgt = dl->m_Orientation.m_vRight;
        float ang = dl->m_fLightFrustumAngle;          
        if (ang == 0)
          ang = 45.0f;
        org = dl->m_Origin;
        
        dir *= 0.3f;
        
        CFColor Col = dl->m_Color;
                
        Matrix44 m;
        Vec3d vertex = dir;
     //   vertex = m.GetRotation(rgt, ang)*vertex;
        vertex = Matrix33::CreateRotationAA(DEG2RAD(ang),GetNormalized(rgt)) * vertex; //NOTE: angle need to be in radians
      //  Matrix mat = m.GetRotation(dir, 60);
        Matrix44 mat = Matrix33::CreateRotationAA(DEG2RAD(60),GetNormalized(dir)); //NOTE: angle need to be in radians
        Vec3d tmpvertex;
        int ctr;
        
        //fill the inside of the light
        glDisable(GL_TEXTURE_2D);        
        CFColor cl = Col*0.3f;
        glColor3fv(&cl[0]);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3fv(&org[0]);
        glColor3fv(&Col[0]);
        tmpvertex = org + vertex;
        glVertex3fv((float*) &tmpvertex);
        for (ctr=0; ctr<6; ctr++)
        {
          vertex = mat * vertex;
          Vec3d tmpvertex = org + vertex;
          glVertex3fv((float*) &tmpvertex);
        }
        glEnd();
        
        //draw the inside of the light with lines and the outside filled
        glPolygonMode(GL_FRONT, GL_LINE);
        glDisable(GL_CULL_FACE);
        glColor3f(0.3f,0.3f, 0.3f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3fv(&org[0]);
        glColor3f(1,1,1);
        tmpvertex = org + vertex;
        glVertex3fv(&tmpvertex[0]);
        for (ctr=0; ctr<6; ctr++)
        {
          vertex = mat * vertex;
          Vec3d tmpvertex = org + vertex;
          glVertex3fv(&tmpvertex[0]);
        }
        glEnd();
        glEnable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT, GL_FILL);
        
        //set the color to the color of the light
        glColor3fv(&Col[0]);
        
        //draw a point at the origin of the light
        glBegin(GL_POINTS);
        glVertex3fv(&org[0]);
        glEnd();
        
        //draw a line in the center of the light
        glBegin(GL_LINES);
        glVertex3fv(&org[0]);
        glVertex3fv(&(org + dir)[0]);
        glEnd();
        
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_TEXTURE_2D);
      }
      if (((CV_r_debuglights==2) || (CV_r_debuglights==3)) && !(dl->m_Flags & DLF_DIRECTIONAL))
      {
        EF_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
        SetCullMode(R_CULL_NONE);
        SetMaterialColor(dl->m_Color[0], dl->m_Color[1], dl->m_Color[2], 0.25f);
        DrawBall(dl->m_Origin[0], dl->m_Origin[1], dl->m_Origin[2], dl->m_fRadius);
      }
    }
  }
}

void CGLRenderer::EF_DrawDebugTools()
{
  //ResetToDefault();

  if (CV_r_showlines)
  {
    if (m_LogFile)
      Logv(SRendItem::m_RecurseLevel, "*** Restart pipeline for wire-frame ***\n");
    EF_RenderPipeLine(EF_DrawWire);
  }

  if (CV_r_shownormals)
    EF_RenderPipeLine(EF_DrawNormals);

  if (CV_r_showtangents)
    EF_RenderPipeLine(EF_DrawTangents);

  if (SRendItem::m_RecurseLevel==1 && CV_r_debuglights)
    EF_DrawDebugLights();
}


int TimeProfCallback( const VOID* arg1, const VOID* arg2 )
{
  SProfInfo *pi1 = (SProfInfo *)arg1;
  SProfInfo *pi2 = (SProfInfo *)arg2;
  if (pi1->ef->m_fProfileTime > pi2->ef->m_fProfileTime)
    return -1;
  if (pi1->ef->m_fProfileTime < pi2->ef->m_fProfileTime)
    return 1;
  return 0;
}

int NameCallback( const VOID* arg1, const VOID* arg2 )
{
  SProfInfo *pi1 = (SProfInfo *)arg1;
  SProfInfo *pi2 = (SProfInfo *)arg2;
  if (pi1->ef > pi2->ef)
    return -1;
  if (pi1->ef < pi2->ef)
    return 1;
  return 0;
}

Matrix33 m33;
void CGLRenderer::EF_PrintProfileInfo()
{
  double fTime = 0;
  int i;
  if (CV_r_profileshaders)
  { // group by name
    qsort(&m_RP.m_Profile[0], m_RP.m_Profile.Num(), sizeof(SProfInfo), NameCallback );

    for(i=0; i<m_RP.m_Profile.Num(); i++)
    {
      // if next is the same
      if( i<(m_RP.m_Profile.Num()-1) && m_RP.m_Profile[i].ef == m_RP.m_Profile[i+1].ef )
      {
        m_RP.m_Profile[i].Time += m_RP.m_Profile[i+1].Time;
        m_RP.m_Profile[i].m_nItems++;
        m_RP.m_Profile[i].NumPolys += m_RP.m_Profile[i+1].NumPolys;
        m_RP.m_Profile.DelElem(i+1);
        i--;
      }
    }
  }
  for (i=0; i<m_RP.m_Profile.Num(); i++)
  {
    m_RP.m_Profile[i].ef->m_fProfileTime = (float)(m_RP.m_Profile[i].Time+m_RP.m_Profile[i].ef->m_fProfileTime*50)/51.0f;
  }

  TextToScreenColor(1,14, 0,2,0,1, "Items: %d, Batches: %d, Obj: %d, DrawCalls: %d, Text: %d, Stat: %d, PShad: %d, VShad: %d",  m_RP.m_PS.m_NumRendItems, m_RP.m_PS.m_NumRendBatches, m_RP.m_PS.m_NumRendObjects, m_RP.m_PS.m_NumDrawCalls, m_RP.m_PS.m_NumTextChanges, m_RP.m_PS.m_NumStateChanges, m_RP.m_PS.m_NumPShadChanges, m_RP.m_PS.m_NumVShadChanges);
  TextToScreenColor(1,17, 0,2,0,1, "VShad: %d, PShad: %d, Text: %d, Lit Shaders: %d",  m_RP.m_PS.m_NumVShaders, m_RP.m_PS.m_NumPShaders, m_RP.m_PS.m_NumTextures, m_RP.m_PS.m_NumLitShaders);
  TextToScreenColor(1,20, 0,2,0,1, "Preprocess: %8.02f ms, Occl. queries: %8.02f ms",  m_RP.m_PS.m_fPreprocessTime, m_RP.m_PS.m_fOcclusionTime);
  TextToScreenColor(1,23, 0,2,0,1, "Skinning:   %8.02f ms (Skinned Objects: %d)",  m_RP.m_PS.m_fSkinningTime, m_RP.m_PS.m_NumRendSkinnedObjects);

  int nLine;
  { // sort by interpolated time and print
    qsort(&m_RP.m_Profile[0], m_RP.m_Profile.Num(), sizeof(SProfInfo), TimeProfCallback );

    for(nLine=0; nLine<m_RP.m_Profile.Num(); nLine++)
    {
      fTime += m_RP.m_Profile[nLine].Time;
      if (nLine >= 18)
        continue;
      m_RP.m_Profile[nLine].ef->m_fProfileTime = (float)(m_RP.m_Profile[nLine].ef->m_fProfileTime + m_RP.m_Profile[nLine].Time);
      TextToScreenColor(4,(24+(nLine*3)), 1,0,0,1, "%8.02f ms, %5d polys, '%s', %d item(s)", 
        m_RP.m_Profile[nLine].Time, 
        m_RP.m_Profile[nLine].NumPolys,
        m_RP.m_Profile[nLine].ef->m_Name.c_str(),
        m_RP.m_Profile[nLine].m_nItems+1);
    }
  }
  int nShaders = nLine;
  if (nLine > 18)
    nLine = 18;
  TextToScreenColor(8,(26+(nLine*3)), 0,2,0,1, "Total unique shaders:   %d",  nShaders);
  TextToScreenColor(8,(29+(nLine*3)), 0,2,0,1, "Total flush time:   %8.02f ms",  fTime);
  TextToScreenColor(8,(32+(nLine*3)), 0,2,0,1, "Total shaders processing time:   %8.02f ms", m_RP.m_PS.m_fFlushTime);

  /*{
    int i = 0;
    double timeC = 0;
    double timeC33 = 0;
    double timeSSE = 0;
    double time3DN = 0;
    _declspec(align(16)) Matrix m;
    m = m_cam.GetMatrix();
    int ft = m_Cpu->mCpu[0].mFeatures;
    m_Cpu->mCpu[0].mFeatures &= ~(CFI_SSE | CFI_3DNOW);
    m_RP.m_pCurObject = m_RP.m_VisObjects[0];
    Matrix33 m31;
    Matrix33 m32;
    ticks(timeC33);
    for (i=0; i<100000; i++)
    {
      SGLFuncs::glMultMatrix33(m33, m31, m32);
    }
    unticks(timeC33);

    ticks(timeC);
    for (i=0; i<100000; i++)
    {
      SGLFuncs::glMultMatrix(m_ViewMatrix.GetData(), &m[0][0], m_RP.m_pCurObject->m_Matrix.GetData());
    }
    unticks(timeC);

    if (ft & CFI_SSE)
    {
      ticks(timeSSE);
      m_Cpu->mCpu[0].mFeatures |= CFI_SSE;
      for (i=0; i<100000; i++)
      {
        _declspec(align(16)) Matrix m;
        SGLFuncs::glMultMatrix(m_ViewMatrix.GetData(), &m[0][0], m_RP.m_pCurObject->m_Matrix.GetData());
      }
      unticks(timeSSE);
    }

    if (ft & CFI_3DNOW)
    {
      ticks(time3DN);
      m_Cpu->mCpu[0].mFeatures &= ~CFI_SSE;
      m_Cpu->mCpu[0].mFeatures |= CFI_3DNOW;
      for (i=0; i<100000; i++)
      {
        _declspec(align(16)) Matrix m;
        SGLFuncs::glMultMatrix(m_ViewMatrix.GetData(), &m[0][0], m_RP.m_pCurObject->m_Matrix.GetData());
      }
      unticks(time3DN);
    }

    m_Cpu->mCpu[0].mFeatures = ft;

    TextToScreenColor(8,(36+(nLine*3)), 0,2,0,1, "TimeC %8.02f ms, TimeC33 %8.02f ms,, TimeSSE %8.02f ms, Time3DN %8.02f ms", (float)(timeC*1000.0*m_RP.m_SecondsPerCycle), (float)(timeC33*1000.0*m_RP.m_SecondsPerCycle), (float)(timeSSE*1000.0*m_RP.m_SecondsPerCycle), (float)(time3DN*1000.0*m_RP.m_SecondsPerCycle));
  }*/
  /*{
    int i = 0;
    double timeC = 0;
    double timeC33 = 0;
    double time3DN = 0;
    double timeSSE = 0;
    _declspec(align(16)) Matrix m, mOut;
    m = m_cam.GetMatrix();
    m_RP.m_pCurObject = m_RP.m_VisObjects[0];
    Matrix33 m31;
    Matrix33 m32;
    m31.SetIdentity33();

    ticks(timeC33);
    for (i=0; i<100000; i++)
    {
      m31.Invert33();
    }
    unticks(timeC33);

    ticks(timeC);
    for (i=0; i<100000; i++)
    {
      QQinvertMatrixf(mOut.GetData(), m.GetData());
    }
    unticks(timeC);

    if (m_Cpu->mCpu[0].mFeatures & CFI_3DNOW)
    {
      ticks(time3DN);
      for (i=0; i<100000; i++)
      {
        invertMatrixf_3DNow(mOut.GetData(), m.GetData());
      }
      unticks(time3DN);
    }

    ticks(timeSSE);
    for (i=0; i<100000; i++)
    {
      invertMatrixf_SSE(mOut.GetData(), m.GetData());
    }
    unticks(timeSSE);

    TextToScreenColor(8,(36+(nLine*3)), 0,2,0,1, "TimeC: %8.02f ms, TimeC33: %8.02f ms,, Time3DN: %8.02f ms, TimeSSE: %8.02f ms", (float)(timeC*1000.0*m_RP.m_SecondsPerCycle), (float)(timeC33*1000.0*m_RP.m_SecondsPerCycle), (float)(time3DN*1000.0*m_RP.m_SecondsPerCycle), (float)(timeSSE*1000.0*m_RP.m_SecondsPerCycle));
  }*/
  /*{
    int i = 0;
    double timeC = 0;
    double timeMMX = 0;
    byte *dataSrc = new byte[1024*1024*10];
    byte *dataDst = new byte[1024*1024*10];

    ticks(timeC);
    for (i=0; i<10; i++)
    {
      memcpy(dataDst, dataSrc, 1024*1024*10);
    }
    unticks(timeC);

    ticks(timeMMX);
    for (i=0; i<10; i++)
    {
      cryMemcpy(dataDst, dataSrc, 1024*1024*10);
    }
    unticks(timeMMX);

    delete [] dataSrc;
    delete [] dataDst;

    TextToScreenColor(8,(36+(nLine*3)), 0,2,0,1, "TimeC: %8.02f ms, TimeMMX: %8.02f ms: %8.02f ms", (float)(timeC*1000.0*m_RP.m_SecondsPerCycle), (float)(timeMMX*1000.0*m_RP.m_SecondsPerCycle));
  }*/
}

void CGLRenderer::EF_DrawREPreprocess(SRendItemPreprocess *ris, int Nums)
{
  int i;
  CCObject *savedObj;
  SShader *Shader, *CurShader, *ShaderState, *CurShaderState;
  SRenderShaderResources *Res, *CurRes;
  int nObject, nCurObject;
  int nFog, nCurFog;

  if (Nums < 1)
    return;

  savedObj = m_RP.m_pCurObject;

  EF_PreRender(3);
    
  glPushMatrix();

  UnINT64 oldVal;
  oldVal.SortVal = -1;
  nCurObject = -2;
  m_RP.m_pCurObject = m_RP.m_VisObjects[0];
  m_RP.m_pPrevObject = m_RP.m_pCurObject;
  nCurFog = 0;
  CurShader = NULL;
  CurShaderState = NULL;
  CurRes = NULL;

  for (i=0; i<Nums; i++)
  {
    SRendItemPreprocess *ri = &ris[i];
    CRendElement *re = ri->Item;
    if (ri->SortVal.SortVal == oldVal.SortVal)
    {
      // Optimizations: not necessary to check of changing shaders and objects
      // if sort value is the same (consist the same info about shaders, objects, fog volumes, ...)
      re->mfPrepare();
      continue;
    }
    oldVal.SortVal = ri->SortVal.SortVal;

    SRendItem::mfGet(ri->SortVal, &nObject, &Shader, &ShaderState, &nFog, &Res);

    if (nObject != nCurObject)
    {
      if (CurShader)
        EF_Flush();
      if (!EF_ObjectChange(Shader, Res, nObject, re))
        continue;
      nCurObject = nObject;
    }

    if (Shader != CurShader || Res != CurRes || ShaderState != CurShaderState || nFog != nCurFog)
    {
      if (CurShader)
        EF_Flush();

      EF_Start(Shader, ShaderState, Res, nFog, re);

      nCurFog = nFog;
      CurShader = Shader;
      CurShaderState = ShaderState;
      CurRes = Res;
    }

    re->mfPrepare();
  }
  if (CurShader)
    EF_Flush();
  
  glDepthRange(m_RP.m_fMinDepthRange, m_RP.m_fMaxDepthRange);

  //ResetToDefault();

  glPopMatrix();

  m_RP.m_pCurObject = savedObj;
}

struct SPreprocess
{
  int m_nPreprocess;
  int m_Num;
  int m_nObject;
  SShader *m_Shader;
  SRenderShaderResources *m_pRes;
  CRendElement *m_RE;
};

STWarpZone *CGLRenderer::EF_SetWarpZone(SWarpSurf *sf, int *NumWarps, STWarpZone Warps[])
{
  int i;
  STWarpZone *wp;
  Plane p, pl;

  sf->srf->mfGetPlane(pl);
  if (sf->nobj > 0)
  {
    p = pl;
    CCObject *obj = m_RP.m_VisObjects[sf->nobj];
    pl = TransformPlane(obj->GetMatrix(), p);
  }
  for (i=0; i<NumWarps[0]; i++)
  {
//    if (pl.d == Warps[i].plane.d && pl.n == Warps[i].plane.n)
    if (pl.d == Warps[i].plane.d && IsEquivalent(pl.n,Warps[i].plane.n))
      break;
  }
  if (i == NumWarps[0])
  {
    NumWarps[0]++;
    wp = &Warps[i];
    wp->numSrf = 0;
    wp->plane.d = pl.d;
    wp->plane.n = pl.n;
  }
  EF_UpdateWarpZone(&Warps[i], sf);

  return &Warps[i];
}

void CGLRenderer::EF_UpdateWarpZone(STWarpZone *wp, SWarpSurf *srf)
{
  if (wp->numSrf == MAX_WARPSURFS)
    return;
  wp->Surfs[wp->numSrf].nobj = srf->nobj;
  wp->Surfs[wp->numSrf].Shader = srf->Shader;
  wp->Surfs[wp->numSrf].ShaderRes = srf->ShaderRes;
  wp->Surfs[wp->numSrf].srf = srf->srf;
  wp->numSrf++;
}

bool CGLRenderer::EF_CalcWarpCamera(STWarpZone *wp, int nObject, CCamera& prevCam, CCamera& newCam)
{
  Vec3d vPrevPos = prevCam.GetPos();
  Vec3d vPrevAngs = prevCam.GetAngles();
  Vec3d vNewPos, vNewOccPos, vNewAngs;
  bool bMirror = true;
  if (nObject > 0)
  {
    CCObject *pObj = m_RP.m_VisObjects[nObject];
    if (pObj->m_ObjFlags & FOB_PORTAL)
    {
      bMirror = false;
      m_RP.m_PersFlags &= ~RBPF_DRAWMIRROR;
      m_RP.m_PersFlags |= RBPF_DRAWPORTAL;
      vNewPos = Vec3d(pObj->m_Trans2[0], pObj->m_Trans2[1], pObj->m_Trans2[2]);
      vNewOccPos = vNewPos;
      vNewAngs = Vec3d(pObj->m_Angs2[0], pObj->m_Angs2[1], pObj->m_Angs2[2]);
    }
  }
  if (bMirror)
  {
    float fDot = vPrevPos.Dot(wp->plane.n) - wp->plane.d;
    vNewPos = vPrevPos + wp->plane.n * -2.0f*fDot;
    vNewOccPos = vPrevPos + wp->plane.n * -0.99f*fDot;

    if (fDot < 0)
      m_RP.m_MirrorClipSide = 1;
    else
      m_RP.m_MirrorClipSide = 2;

    fDot = m_RP.m_CamVecs[0].Dot(wp->plane.n);
    Vec3d vNewDir = m_RP.m_CamVecs[0] + wp->plane.n * -2.0f*fDot;

    vNewAngs[0] = cry_asinf (vNewDir[2])/M_PI*180.0f;
    vNewAngs[1] = vPrevAngs[1] + 180.0f;
    vNewAngs[2] = -cry_atan2f (vNewDir[0], -vNewDir[1])/M_PI*180.0f;

    m_RP.m_PersFlags &= ~RBPF_DRAWPORTAL;
    m_RP.m_PersFlags |= RBPF_DRAWMIRROR;
  }

  newCam.SetPos(vNewPos);
  newCam.SetOccPos(vNewOccPos);
  newCam.SetAngle(vNewAngs);
  if (bMirror)
    newCam.SetScale(Vec3d(-1.0f, 1.0f, 1.0f));
  newCam.Update();  

  return true;
}

bool CGLRenderer::EF_RenderWarpZone(STWarpZone *wp)
{
  if (!wp->numSrf)
    return false;

  CCamera prevCam = GetCamera();
  CCamera newCam = prevCam;

  int prevFlags = m_RP.m_PersFlags;

  if (!EF_CalcWarpCamera(wp, wp->Surfs[0].nobj, prevCam, newCam))
    return false;

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "\n    ********** Start Render Portal **********\n");

  STWarpZone *prevWarp = m_RP.m_CurWarp;
  m_RP.m_CurWarp = wp;

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  iSystem->SetViewCamera(newCam);
  SetCamera(newCam);

  float plane[4];
  plane[0] = wp->plane.n[0];
  plane[1] = wp->plane.n[1];
  plane[2] = wp->plane.n[2];
  plane[3] = -wp->plane.d;

  EF_PushFog();

  EF_SetClipPlane(true, plane, false);

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  eng->DrawLowDetail(DLD_TERRAIN_WATER | DLD_DETAIL_TEXTURES | DLD_DETAIL_OBJECTS | DLD_FAR_SPRITES | DLD_ENTITIES | DLD_STATIC_OBJECTS);

  /*unsigned char *pic=new unsigned char [m_width*m_height*4];
  glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
  WriteTGA(pic, m_width, m_height,"Warp.tga");
  delete [] pic;*/

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "\n    ********** End Render Portal **********\n");

  m_RP.m_PersFlags &= ~(RBPF_DRAWMIRROR | RBPF_DRAWPORTAL);
  m_RP.m_PersFlags |= prevFlags & (RBPF_DRAWMIRROR | RBPF_DRAWPORTAL);

  EF_SetClipPlane(false, plane, false);

  EF_PopFog();

  m_RP.m_CurWarp = prevWarp;

  iSystem->SetViewCamera(prevCam);
  SetCamera(prevCam);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  return true;
}

static _inline int Compare(SPreprocess &a, SPreprocess &b)
{
  int nPr1 = a.m_nPreprocess;
  int nPr2 = b.m_nPreprocess;

  if (nPr1 < nPr2)
    return -1;
  if (nPr1 > nPr2)
    return 1;
  return 0;
}

void CGLRenderer::EF_FlushRefractedObjects(SShader *pSHRefr[], CRendElement *pRERefr[], CCObject *pObjRefr[], int nRefrObjects, int nRefrFlags, int DLDFlags)
{
  if (nRefrFlags & 2)
  {
    Vec3d Angs = GetCamera().GetAngles();
    Vec3d Pos = GetCamera().GetPos();
    m_RP.m_PersFlags |= RBPF_IGNOREREFRACTED;
    if (nRefrObjects > 1)
      pSHRefr[0] = NULL;
    float fMinDist = 99999.0f;
    int nSelectObj = -1;
    for (int i=0; i<nRefrObjects; i++)
    {
      float fDist = pRERefr[i]->mfMinDistanceToCamera(pObjRefr[i]);
      if (fMinDist > fDist)
      {
        fMinDist = fDist;
        nSelectObj = i;
      }
    }
    if (nSelectObj >= 0)
    {
      m_RP.m_pRE = pRERefr[nSelectObj];
      m_RP.m_pCurObject = pObjRefr[nSelectObj];
      SEnvTexture *cm = gRenDev->m_cEF.mfFindSuitableEnvTex(Pos, Angs, false, DLDFlags, true, pSHRefr[0], NULL, NULL, false, NULL);
    }
    m_RP.m_PersFlags &= ~RBPF_IGNOREREFRACTED;
  }

  if (nRefrFlags & 1)
  {
    m_RP.m_bDrawToTexture = true;
    m_TexMan->StartRefractMap(TO_REFRACTMAP);
    m_RP.m_PersFlags |= RBPF_ONLYREFRACTED;
    EF_RenderPipeLine(EF_Flush);
    m_RP.m_PersFlags &= ~RBPF_ONLYREFRACTED;
    m_TexMan->EndRefractMap();
//    m_TexMan->StartScreenTexMap(TO_SCREENMAP);
//    EF_RenderPipeLine(EF_Flush);
//    m_TexMan->EndScreenTexMap();
    m_RP.m_bDrawToTexture = false;
    //m_RP.m_PersFlags |= RBPF_ONLYREFRACTED;
    //m_RP.m_PersFlags |= RBPF_IGNORERENDERING;
  }
}

int CGLRenderer::EF_Preprocess(SRendItemPre *ri, int nums, int nume)
{
  int i, j, nNumItems;
  SShader *Shader;
  SShader *ShaderState;
  SRenderShaderResources *Res;
  int nObject;
  int nFog;

  TArray<SPreprocess> Procs;
  SPreprocess Proc;
  TArray<SRendItemPreprocess> RIs;
  SWarpSurf srfs[1024];
  int nPortals = 0;
  int nRefractedStuff = 0;

  int NumWarps = 0;
  STWarpZone Warps[MAX_WARPS];
  STWarpZone *wp;

  double time0 = 0;
  ticks(time0);

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "\n\n*** Start preprocess frame ***\n");

  int nReturn = 0;

  for (i=nums; i<nume; i++)
  {
    SRendItem::mfGet(ri[i].SortVal, &nObject, &Shader, &ShaderState, &nFog, &Res);
    if ((ri[i].SortVal.i.High >> 26) != eS_PreProcess)
      break;
    nReturn++;
    for (j=0; j<32; j++)
    {
      int nMask = 1<<j;
      if (nMask >= FSPR_MAX || nMask > Shader->m_nPreprocess)
        break;
      if (nMask & Shader->m_nPreprocess)
      {
        Proc.m_nPreprocess = j;
        Proc.m_Num = i;
        Proc.m_Shader = Shader;
        Proc.m_pRes = Res;
        Proc.m_RE = ri[i].Item;
        Proc.m_nObject = nObject;
        Procs.AddElem(Proc);
      }
    }
  }
  if (!Procs.Num())
    return 0;
  nNumItems = Procs.Num();
  ::Sort(&Procs[0], nNumItems);
  
  if (m_RP.m_pRenderFunc != EF_Flush || CV_r_nopreprocess)
    return nReturn;

  int DLDFlags = 0;
  int nRefrObjects = 0;
  SShader *pSHRefr[8];
  CRendElement *pRERefr[8];
  CCObject *pObjRefr[8];

  for (i=0; i<nNumItems; i++)
  {
    SPreprocess *pr = &Procs[i];
    if (!pr->m_Shader)
      continue;
    switch (pr->m_nPreprocess)
    {
      case SPRID_SCANCM:
        // Draw environment to cube texture
        if (!m_RP.m_bDrawToTexture)
        {
          CCObject *objIgn = m_RP.m_pIgnoreObject;
          Vec3d Pos;
          m_RP.m_pRE = pr->m_RE;
          if (pr->m_nObject < 0)
            Pos(0,0,0);
          else
          {
            CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
            if (!IsEquivalent(obj->GetTranslation(), Vec3d(0,0,0)))
              m_RP.m_pIgnoreObject = obj;
            Pos = obj->GetTranslation();
          }
          if (m_LogFile)
            Logv(SRendItem::m_RecurseLevel, "*** Draw environment to cube-map ***\n");
          if (!IsEquivalent(Pos,Vec3d(0,0,0)))
          {
            float fDistToCam = (Pos-m_cam.GetPos()).Length();
            SEnvTexture *cm = gRenDev->m_cEF.mfFindSuitableEnvCMap(Pos, false, pr->m_Shader->m_DLDFlags, fDistToCam);
          }
          m_RP.m_pIgnoreObject = objIgn;
        }
        break;
      case SPRID_SCANLCM:
        // Draw environment to cube texture
        if (!m_RP.m_bDrawToTexture)
        {
          CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
          if ((obj->m_ObjFlags & FOB_ENVLIGHTING) && CV_r_envlighting)
          {
            CCObject *objIgn = m_RP.m_pIgnoreObject;
            Vec3d Pos;
            m_RP.m_pRE = pr->m_RE;
            if (!IsEquivalent(obj->GetTranslation(), Vec3d(0,0,0)))
              m_RP.m_pIgnoreObject = obj;
            Pos = obj->GetTranslation();
            if (m_LogFile)
              Logv(SRendItem::m_RecurseLevel, "*** Draw environment to light cube-map ***\n");
            if (!IsEquivalent(Pos,Vec3d(0,0,0)))
            {
              float fDistToCam = (Pos-m_cam.GetPos()).Length();
              SEnvTexture *cm = gRenDev->m_cEF.mfFindSuitableEnvLCMap(Pos, false, pr->m_Shader->m_DLDFlags, fDistToCam);
            }
            m_RP.m_pIgnoreObject = objIgn;
          }
        }
        break;
      case SPRID_SCANSCR:
        m_TexMan->StartScreenMap(TO_ENVIRONMENT_SCR);
        break;
      case SPRID_SCANTEXWATER:
        if (!m_RP.m_bDrawToTexture && CV_r_waterreflections)
        {
          if (!(gRenDev->m_TexMan->m_Text_WaterMap->m_Flags & FT_BUILD))
          {
            I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
            float fTimeUpd = eng->GetDistanceToSectorWithWater();
            fTimeUpd *= CV_r_waterupdateFactor;
            if (fTimeUpd > 0.3f)
              fTimeUpd = 0.3f;
            Vec3d camAngs = GetCamera().GetAngles();
            Vec3d camPos = GetCamera().GetPos();
            float fDistCam = (camPos - m_RP.m_LastWaterPosUpdate).Length();
            float fDistAng = (camAngs - m_RP.m_LastWaterAngleUpdate).Length();
            float fFOV = GetCamera().GetFov();
            if (m_RP.m_fLastWaterUpdate-1.0f > m_RP.m_RealTime)
              m_RP.m_fLastWaterUpdate = m_RP.m_RealTime;
            if (m_RP.m_RealTime-m_RP.m_fLastWaterUpdate > fTimeUpd || fDistCam > CV_r_waterupdateDistance || fDistAng>CV_r_waterupdateDeltaAngle || fFOV != m_RP.m_fLastWaterFOVUpdate)
            {
              m_RP.m_fLastWaterUpdate = m_RP.m_RealTime;
              m_RP.m_LastWaterAngleUpdate = camAngs;
              m_RP.m_fLastWaterFOVUpdate = fFOV;
              m_RP.m_LastWaterPosUpdate = camPos;

              CCamera tmp_cam = GetCamera();
              Plane Pl;
              Pl.n = Vec3d(0,0,1);
              Pl.d = eng->GetWaterLevel();
              Vec3d vCamPos = tmp_cam.GetPos();
              if ((vCamPos | Pl.n) - Pl.d < 0)
              {
                Pl.d = -Pl.d;
                Pl.n = -Pl.n;
              }
              CCObject *savedObj = m_RP.m_pCurObject;
              CCObject *objIgn = m_RP.m_pIgnoreObject;
              CCObject *obj = NULL;
              if (pr->m_nObject > 0)
              {
                obj = m_RP.m_VisObjects[pr->m_nObject];
              }
              m_RP.m_PersFlags |= RBPF_DONTDRAWSUN;
              m_RP.m_pCurObject = obj;
              if (!IsEquivalent(obj->GetTranslation(), Vec3d(0,0,0)))
                m_RP.m_pIgnoreObject = obj;
              if (m_LogFile)
                Logv(SRendItem::m_RecurseLevel, "*** Draw environment to texture for water reflections ***\n");
              m_TexMan->DrawToTexture(Pl, gRenDev->m_TexMan->m_Text_WaterMap, pr->m_Shader->m_DLDFlags);
              m_RP.m_pCurObject = savedObj;
              m_RP.m_pIgnoreObject = objIgn;
              m_RP.m_PersFlags &= ~RBPF_DONTDRAWSUN;
            }
          }
        }
        break;
      case SPRID_SCANTEX:
        if (!m_RP.m_bDrawToTexture)
        { // Draw environment to texture
          bool bWater = (pr->m_Shader->m_nPreprocess & FSPR_SCANTEXWATER) != 0;
          bool bDraw = true;
          if (!bWater || CV_r_waterrefractions)
          {
            if (pr->m_Shader->m_fUpdateFactor > 0)
            {
              float fDist = pr->m_RE->mfMinDistanceToCamera(m_RP.m_VisObjects[pr->m_nObject]);
              if (fDist > 0)
              {
                fDist *= pr->m_Shader->m_fUpdateFactor;
                if (fDist > 0.5f)
                  fDist = 0.5f;
                if (m_RP.m_RealTime-m_RP.m_pCurObject->m_fLastUpdate < fDist)
                  bDraw = false;
                else
                  m_RP.m_pCurObject->m_fLastUpdate = m_RP.m_RealTime;
              }
            }
            if (bDraw)
            {
              CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
              Vec3d Angs = GetCamera().GetAngles();
              Vec3d Pos = GetCamera().GetPos();
              m_RP.m_pRE = pr->m_RE;
              if (m_LogFile)
                Logv(SRendItem::m_RecurseLevel, "*** Draw environment to texture ***\n");

              bool bReflect = false;
              if ((pr->m_Shader->m_Flags3 & (EF3_CLIPPLANE_FRONT | EF3_REFLECTION)))
                bReflect = true;
              if (bReflect)
              {
                m_RP.m_PersFlags |= RBPF_DONTDRAWSUN;
                m_RP.m_pIgnoreObject = obj;
                m_RP.m_pCurObject = obj;
                SEnvTexture *cm = gRenDev->m_cEF.mfFindSuitableEnvTex(Pos, Angs, false, pr->m_Shader->m_DLDFlags, false, pr->m_Shader, pr->m_pRes, obj, bReflect, pr->m_RE);
                m_RP.m_pIgnoreObject = NULL;
              }
              else
              {
                nRefractedStuff |= 2;
                DLDFlags |= pr->m_Shader->m_DLDFlags;
                obj->m_ObjFlags |= FOB_REFRACTED;
                pSHRefr[nRefrObjects] = pr->m_Shader;
                pRERefr[nRefrObjects] = pr->m_RE;
                pObjRefr[nRefrObjects] = obj;
                if (nRefrObjects != 7)
                  nRefrObjects++;
              }
            }
          }
        }
        break;
      case SPRID_RAINOVERLAY:
        if (!m_RP.m_bDrawToTexture)
          m_TexMan->DrawToTextureForRainMap(TO_RAINMAP);
        break;
      case SPRID_REFRACTED:
        if (!m_RP.m_bDrawToTexture)
        {
          nRefractedStuff |= 1;
          CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
          obj->m_ObjFlags |= FOB_REFRACTED;
        }
        break;
      case SPRID_SCREENTEXMAP:
        EF_FlushRefractedObjects(pSHRefr, pRERefr, pObjRefr, nRefrObjects, nRefractedStuff, DLDFlags);
        nRefractedStuff = 0;
        nRefrObjects = 0;
        // copy screen to texture
        if (!m_RP.m_bDrawToTexture)
        {
          m_TexMan->StartScreenTexMap(TO_SCREENMAP);                    
          // check when refraction is active.. 
          if(pr->m_RE->mfGetType() != eDATA_Dummy)
          { 
            m_RP.m_PersFlags |= RBPF_IGNOREREFRACTED;
            
            m_bRefraction=1;
          }
        }
        break;
      case SPRID_SHADOWMAPGEN:
        {
          CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
          m_RP.m_pCurObject = obj;
          if (pr->m_RE->mfGetType() == eDATA_OcLeaf || pr->m_RE->mfGetType() == eDATA_ShadowMapGen)
          {
            if(!m_RP.m_bDrawToTexture && obj->m_pShadowCasters)
            {
              if (m_LogFile)
                Logv(SRendItem::m_RecurseLevel, "*** Prepare shadow maps for REOcLeaf***\n");

              // prepare casters frustums
              for(int i=0; i<obj->m_pShadowCasters->Count(); i++)
              {
                if(obj->m_pShadowCasters->GetAt(i).m_pLS)
                {
                  if(obj->m_pShadowCasters->GetAt(i).m_pLS->GetShadowMapFrustum())
                    PrepareDepthMap(obj->m_pShadowCasters->GetAt(i).m_pLS->GetShadowMapFrustum(), false);
                  m_RP.m_pCurObject = obj;

//                  if(obj->m_pShadowCasters->GetAt(i).m_pLS->GetShadowMapFrustumPassiveCasters())
  //                  PrepareDepthMap(obj->m_pShadowCasters->GetAt(i).m_pLS->GetShadowMapFrustumPassiveCasters(), false);
    //              m_RP.m_pCurObject = obj;
                }
              }
            }
          }
        }
        break;
      case SPRID_CORONA:
        if (pr->m_nObject>0)
        {
          if (pr->m_RE->mfGetType() == eDATA_Flare)
          {
            CREFlare *fl = (CREFlare *)pr->m_RE;
            CCObject *obj = m_RP.m_VisObjects[pr->m_nObject];
            if (m_LogFile)
              Logv(SRendItem::m_RecurseLevel, "*** Check corona visibility ***\n");
            bool bVis = fl->mfCheckVis(obj);
          }
        }
        break;
      case SPRID_PORTAL:
        {
          if (!CV_r_portals)
            continue;

          if (m_RP.m_RecurseLevel+1 >= MAX_PORTAL_RECURSES)
            continue;

          if (!CV_r_portalsrecursive && m_RP.m_CurWarp)
            continue;

          if (!m_RP.m_CurWarp)
          {
            srfs[nPortals].nobj = pr->m_nObject;
            srfs[nPortals].srf = pr->m_RE;
            srfs[nPortals].Shader = pr->m_Shader;
            nPortals++;
          }
          else
          {
            Plane pl;
            pr->m_RE->mfGetPlane(pl);
            if (m_RP.m_CurWarp->plane.d != pl.d || !IsEquivalent(m_RP.m_CurWarp->plane.n,pl.n))
            {
              srfs[nPortals].nobj = pr->m_nObject;
              srfs[nPortals].srf = pr->m_RE;
              srfs[nPortals].Shader = pr->m_Shader;
              nPortals++;
            }
          }
        }
        break;
      default:
        {
          if (pr->m_nObject < 0)
            continue;
          SRendItemPreprocess rip;
          SRendItemPre *r = &ri[pr->m_Num];
          rip.Item = r->Item;
          rip.m_Object = m_RP.m_VisObjects[pr->m_nObject];
          if (rip.m_Object->m_ObjFlags & FOB_CUBE_MASK)
            RIs.AddElem(rip);
        }
        break;
    }
  }
  if (RIs.Num())
  {
    if (m_LogFile)
      Logv(SRendItem::m_RecurseLevel, "*** Preprocess pipeline ***\n");
    SRendItemPreprocess::mfSort(&RIs[0], RIs.Num());
    EF_DrawREPreprocess(&RIs[0], RIs.Num());
  }
  EF_FlushRefractedObjects(pSHRefr, pRERefr, pObjRefr, nRefrObjects, nRefractedStuff, DLDFlags);

  if (nPortals)
  {
    for (i=0; i<nPortals; i++)
    {
      EF_SetWarpZone(&srfs[i], &NumWarps, Warps);
    }

    m_RP.m_RecurseLevel++;
    m_RP.m_WasPortals += NumWarps;
    m_RP.m_CurPortal += NumWarps;

    for (i=0; i<NumWarps; i++)
    {
      wp = &Warps[i];

      if (m_LogFile)
        Logv(SRendItem::m_RecurseLevel, "*** Start rendering of warp zone %d ***\n", i);
      EF_RenderWarpZone(wp);
      m_RP.m_CurPortal--;

      EF_PreRender(3);

      // Render translucent mirror surface(s)
      int nObj = -1;
      m_RP.m_pPrevObject = m_RP.m_VisObjects[0];
      SShader *sh = NULL;
      SRenderShaderResources *Res = NULL;
      for (int j=0; j<wp->numSrf; j++)
      {
        SWarpSurf *ws = &wp->Surfs[j];
        if (nObj != ws->nobj)
        {
          if (sh)
            EF_Flush();

          if (!EF_ObjectChange(sh, Res, ws->nobj, ws->srf))
            continue;
          nObj = ws->nobj;
        }

        if (ws->Shader != sh || ws->ShaderRes != Res)
        {
          if (sh)
            EF_Flush();

          EF_Start(ws->Shader, NULL, ws->ShaderRes, 0, ws->srf);

          sh = ws->Shader;
          Res = ws->ShaderRes;
        }

        ws->srf->mfPrepare();
      }
      if (sh)
        EF_Flush();
    }
    m_RP.m_RecurseLevel--;
  }

  if (m_LogFile)
    Logv(SRendItem::m_RecurseLevel, "*** End preprocess frame ***\n");

  unticks(time0);
  m_RP.m_PS.m_fPreprocessTime += (float)(time0*1000.0*g_SecondsPerCycle);

  return nReturn;
}

void CGLRenderer::EF_EndEf2D(bool bSort)
{
  int i;
  SShader *Shader, *CurShader, *ShaderState, *CurShaderState;
  SRenderShaderResources *Res, *CurRes;

  assert(SRendItem::m_RecurseLevel >= 1);
  if (SRendItem::m_RecurseLevel < 1)
  {
    iLog->Log("Error: CRenderer::EF_EndEf2D without CRenderer::EF_StartEf");
    return;
  }

  EF_AddClientPolys2D();

  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_PREPROCESS_ID] = SRendItem::m_RendItems[EFSLIST_PREPROCESS_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_STENCIL_ID] = SRendItem::m_RendItems[EFSLIST_STENCIL_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID] = SRendItem::m_RendItems[EFSLIST_GENERAL_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_UNSORTED_ID] = SRendItem::m_RendItems[EFSLIST_UNSORTED_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_DISTSORT_ID] = SRendItem::m_RendItems[EFSLIST_DISTSORT_ID].Num();
  SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_LAST_ID] = SRendItem::m_RendItems[EFSLIST_LAST_ID].Num();
  int nums = SRendItem::m_StartRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID];
  int nume = SRendItem::m_EndRI[SRendItem::m_RecurseLevel-1][EFSLIST_GENERAL_ID];
  if (bSort)
  {
    SRendItem::mfSort(&SRendItem::m_RendItems[0][0], nume);
    // If sort number of the first shader is 1 (eS_Preprocess)
    // run preprocess operations for the current list
    if ((SRendItem::m_RendItems[0][nums].SortVal.i.High >> 26) == eS_PreProcess)
      nums += EF_Preprocess(&SRendItem::m_RendItems[0][0], nums, nume);
  }

  EF_PreRender(3);

  CurShader = NULL;
  CurRes = NULL;
  CurShaderState = NULL;
  UnINT64 oldVal;
  oldVal.SortVal = -1;
  m_RP.m_Flags = RBF_2D;

  for (i=nums; i<nume; i++)
  {
    SRendItemPre *ri = &SRendItem::m_RendItems[0][i];
    CRendElement *re = ri->Item;
    if (ri->SortVal.SortVal == oldVal.SortVal)
    {
      re->mfPrepare();
      continue;
    }
    oldVal.SortVal = ri->SortVal.SortVal;

    SRendItem::mfGet(ri->SortVal, &Shader, &ShaderState, &Res);
    if (Shader != CurShader || Res != CurRes || ShaderState != CurShaderState)
    {
      if (CurShader)
        EF_Flush();

      EF_Start(Shader, ShaderState, Res, re);

      CurShader = Shader;
      CurShaderState = ShaderState;
      CurRes = Res;
    }

    re->mfPrepare();
  }
  if (CurShader)
    EF_Flush();

  //ResetToDefault();

  EF_RemovePolysFromScene();
}

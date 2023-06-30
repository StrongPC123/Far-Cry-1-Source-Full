#include "RenderPCH.h"
#include "RendElement.h"
#include "CRETriMeshShadow.h"
#include "I3DEngine.h"

// shadow volumes rendering modified by vlad

// shadow volumes statistics
int CRETriMeshShadow::m_nCRETriMeshShadowRebuildsPerFrrame = 0;
int CRETriMeshShadow::m_nCRETriMeshShadowShadowsPerFrrame = 0;
int CRETriMeshShadow::m_nCRETriMeshShadowAloocatedShadows = 0;

//////////////////////////////////////////////////////////////////////
void CRETriMeshShadow::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);
  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 0;
}

//////////////////////////////////////////////////////////////////////
void *CRETriMeshShadow::mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags)
{
  switch (ePT)
  {
    case eSrcPointer_Vert:
			{
        if(m_nCurrInst<0)
       {
//          Warning( 0,0,"Error: CRETriMeshShadow::mfGetPointer: m_nCurrInst<0");
          return NULL;
        }

        *Stride = m_VertexSize[m_arrLBuffers[m_nCurrInst].pVB->m_pVertexBuffer->m_vertexformat];
        if(m_arrLBuffers[m_nCurrInst].pVB->m_pVertexBuffer)
          return m_arrLBuffers[m_nCurrInst].pVB->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
				else
					Warning( 0,0,"Error: CRETriMeshShadow::mfGetPointer: m_pBuffer->m_pVertexBuffer == 0");
			}
			break;
  }

  return NULL;
}

int CRETriMeshShadow::GetAndResetRebuildsPerFrrameCounter()
{
  int n = m_nCRETriMeshShadowRebuildsPerFrrame;
  m_nCRETriMeshShadowRebuildsPerFrrame = 0;
  return n;
}

int CRETriMeshShadow::GetAndResetShadowVolumesPerFrrameCounter()
{
  int n = m_nCRETriMeshShadowShadowsPerFrrame;
  m_nCRETriMeshShadowShadowsPerFrrame = 0;
  return n;
}

int CRETriMeshShadow::GetShadowVolumesAllocatedCounter()
{
  return m_nCRETriMeshShadowAloocatedShadows;
}

bool CRETriMeshShadow::mfCheckUpdate(int nVertFormat, int Flags)
{  
  if(m_bAnimatedObject)
  { // character animation: everything already calculated, always use slot 0
    m_nCurrInst = 0; // will be used during drawing
    ShadVolInstanceInfo * pSVInfo = &m_arrLBuffers[0];
    if (pSVInfo->pVB->m_pMats && pSVInfo->pVB->m_pMats->Count() && pSVInfo->pVB->m_pMats->Get(0)->pRE)
      return pSVInfo->pVB->m_pMats->Get(0)->pRE->mfCheckUpdate(nVertFormat, Flags);
    
    return false;
  }

  m_nCRETriMeshShadowShadowsPerFrrame++;

  assert(m_nCurrInst == -1);

  // find light of this shadow
  CDLight * pDLight = NULL;
  if (gRenDev->m_RP.m_DynLMask)
  {
    for (int n=0; n<gRenDev->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); n++)
    {
      if (gRenDev->m_RP.m_DynLMask & (1<<n))
      {
        pDLight = gRenDev->m_RP.m_DLights[SRendItem::m_RecurseLevel][n];
        break;
      }
    }
  }
  if (!pDLight)
    return true;

  CDLight fakeLight;
  fakeLight = *pDLight; // do not copy

  // get obj space light pos
  CCObject *pObj = gRenDev->m_RP.m_pCurObject;
  Matrix44& tInvRot = pObj->GetInvMatrix();

  fakeLight.m_vObjectSpacePos = tInvRot.TransformPointOLD(fakeLight.m_Origin);

//  assert(m_nCurrInst==-1);

  // todo: take radius into account

  //assert(pObj->m_CustomData);

  // find buffer for this case
  ShadVolInstanceInfo * pSVInfo = 0;
  for(int i=0; i<MAX_SV_INSTANCES; i++) // find static volume by light objspace position
  if(m_arrLBuffers[i].pVB && IsEquivalent(m_arrLBuffers[i].vObjSpaceLightPos, fakeLight.m_vObjectSpacePos, 0.001f))
  {
    pSVInfo = &m_arrLBuffers[i];
    pSVInfo->nFrameId = gRenDev->GetFrameID();
    m_nCurrInst = i;
    break;
  }
  
  if(!pSVInfo) // if not found - select new slot
  {
    // now find same combination of light and shadow caster 
    // but only if slot was not used in prev frame -
    // outomagic double buffering of dynamic shadows
    for(int i=0; i<MAX_SV_INSTANCES; i++)
    if( m_arrLBuffers[i].pVB && 
        m_arrLBuffers[i].pLightOwner == fakeLight.m_pOwner && 
        m_arrLBuffers[i].pShadowCaster == pObj->m_CustomData &&
        m_arrLBuffers[i].nFrameId+1 < gRenDev->GetFrameID())
    {
      pSVInfo = &m_arrLBuffers[i];
      m_nCurrInst = i;
      break;
    }

    if(!pSVInfo) // if still not found 
    { // find empty slot or slot with smallest frame id
      int nSmalestFrameId = gRenDev->GetFrameID()+1;
      for(int i=0; i<MAX_SV_INSTANCES; i++)
      {
        if(!m_arrLBuffers[i].pVB)
        { // search for free slot
          pSVInfo = &m_arrLBuffers[i];
          m_nCurrInst = i;
          nSmalestFrameId=0;
          break;
        }

        if(m_arrLBuffers[i].nFrameId<nSmalestFrameId)
        { // search for oldest slot at the same time
          nSmalestFrameId = m_arrLBuffers[i].nFrameId;
          pSVInfo = &m_arrLBuffers[i];
          m_nCurrInst = i;
        }
      }
    }

    if(pSVInfo)
    {
      pSVInfo->nFrameId = gRenDev->GetFrameID();
      pSVInfo->vObjSpaceLightPos = fakeLight.m_vObjectSpacePos;
      pSVInfo->pLightOwner = fakeLight.m_pOwner;
      pSVInfo->pShadowCaster = (IEntityRender*)pObj->m_CustomData;
    }

    if(!pSVInfo->pVB)
      m_nCRETriMeshShadowAloocatedShadows++; // will be created now

    // pSVInfo will be used in RebuildDynamicShadowVolumeBuffer
    m_pSvObj->RebuildShadowVolumeBuffer(fakeLight, pObj->m_TempVars[0]);
    m_nCRETriMeshShadowRebuildsPerFrrame++;

  //  if(!pSVInfo->pVB)
//      iLog->Log("Warning: CRETriMeshShadow::mfCheckUpdate: !pSVInfo->pVB");

/*
    ICVar *pVar = iConsole->GetCVar("e_stencil_shadows");
    if(pVar && pVar->GetIVal()==3 && pSVInfo->pVB)
      iLog->Log("CRETriMeshShadow: Static shadow volume created: %d faces", pSVInfo->pVB->GetIndices().Count()/3);*/
  }
   
  if (pSVInfo->pVB &&
      pSVInfo->pVB->m_pMats && 
      pSVInfo->pVB->m_pMats->Count() &&
      pSVInfo->pVB->m_pMats->Get(0)->pRE)
    pSVInfo->pVB->m_pMats->Get(0)->pRE->mfCheckUpdate(nVertFormat, Flags);
  else
    m_nCurrInst = -1;

  fakeLight.m_vObjectSpacePos = Vec3d(0,0,0);

#ifdef DIRECT3D8
  assert(0); // not tested, what this line do?
  gRenDev->m_RP.m_CurD3DVFormat = pSVInfo->pVB->m_pSecVertBuffer->m_vertexformat + 16;
#endif
  return true;
}

CRETriMeshShadow::~CRETriMeshShadow()
{
  for(int i=0; i<MAX_SV_INSTANCES; i++)
  {
    gRenDev->DeleteLeafBuffer(m_arrLBuffers[i].pVB);
    m_arrLBuffers[i].pVB=0;
  }
}

bool CRETriMeshShadow::mfCheckUnload()
{ // remove all not used leafbuffers
  for(int i=0; i<MAX_SV_INSTANCES; i++)
  if(m_arrLBuffers[i].pVB)
  {
    if(m_arrLBuffers[i].nFrameId < gRenDev->GetFrameID()-100)
    {
      gRenDev->DeleteLeafBuffer(m_arrLBuffers[i].pVB);
      m_arrLBuffers[i].pVB=0;
      m_arrLBuffers[i].nFrameId=0;
      m_nCRETriMeshShadowAloocatedShadows--;
    }
  }

/*
  if(nLastCRETriMeshShadowLogFrame != gRenDev->GetFrameID())
  {
    int nRebNum  = GetRETriMeshShadowRebuildsPerFrrame()/32;;
    int nShadNum = GetRETriMeshShadowShadowsPerFrrame()/32;
    ICVar *pVar = iConsole->GetCVar("e_stencil_shadows");
    if(pVar && pVar->GetIVal()==3)
      iLog->Log("CRETriMeshShadow: Allocated: %d, Used: %d, Updated: %d", 
      nCRETriMeshShadowAloocatedShadows, nShadNum, nRebNum);
    nLastCRETriMeshShadowLogFrame = gRenDev->GetFrameID();
  }*/

  return true;
}

void CRETriMeshShadow::PrintStats()
{
  gRenDev->TextToScreenColor(8,20, 0,2,0,1, "Shadow volumes stats: Rebuild: %d, Used: %d, Allocated: %d",
    CRETriMeshShadow::GetAndResetRebuildsPerFrrameCounter(),
    CRETriMeshShadow::GetAndResetShadowVolumesPerFrrameCounter(),
    CRETriMeshShadow::GetShadowVolumesAllocatedCounter());
}
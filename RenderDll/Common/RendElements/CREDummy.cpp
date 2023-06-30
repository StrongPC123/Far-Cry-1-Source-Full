#include "RenderPCH.h"
#include "RendElement.h"

void CREDummy::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 0;
}

bool CREDummy::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  return true;
}


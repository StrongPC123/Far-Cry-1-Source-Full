#include "RenderPCH.h"
#include "RendElement.h"
#include "CRETerrainSector.h"
#include "I3DEngine.h"

void CRECommon::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 0;
}

bool CREFarTreeSprites::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  iSystem->GetI3DEngine()->DrawFarTrees();
  return true;
}

bool CRETerrainDetailTextureLayers::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  iSystem->GetI3DEngine()->DrawTerrainDetailTextureLayers();
  return true;
}

bool CRETerrainParticles::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  iSystem->GetI3DEngine()->DrawTerrainParticles(ef);
  return true;
}


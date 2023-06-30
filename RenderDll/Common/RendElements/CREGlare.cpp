#include "RenderPCH.h"
#include "RendElement.h"

void CREGlare:: mfInit()
{
  m_GlareWidth = 128;
  m_GlareHeight = 128; 
  m_fGlareAmount=0;

  mfSetType(eDATA_Glare);
  mfUpdateFlags(FCEF_TRANSFORM);
}


void CREGlare::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 4;
  gRenDev->m_RP.m_FirstVertex = 0;
}


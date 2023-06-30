#include "RenderPCH.h"
#include "RendElement.h"

#if !defined(LINUX)

void CREFlashBang::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  gRenDev->m_RP.m_FlagsPerFlush |= RBSI_DRAWAS2D;

  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 0;
}

#endif // !defined(LINUX)
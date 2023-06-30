#include "RenderPCH.h"
#include "RendElement.h"

void CRETempMesh::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 6;
  gRenDev->m_RP.m_RendNumVerts = 4;
  gRenDev->m_RP.m_FirstVertex = 0;
  gRenDev->m_RP.m_FirstIndex = 0;
}

void *CRETempMesh::mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags)
{
  *Stride = sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F);
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pVertices = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)m_VBuffer->m_VS[VSF_GENERAL].m_VData;
  gRenDev->m_RP.m_nCurBufferID = m_VBuffer->m_VS[VSF_GENERAL].m_VertBuf.m_nID;
  SBufInfoTable *pOffs = &gBufInfoTable[m_VBuffer->m_vertexformat];

  switch(ePT) 
  {
    case eSrcPointer_Vert:
      gRenDev->m_RP.m_nCurBufferOffset = 0;
      return &pVertices->xyz.x;
    case eSrcPointer_Tex:
      gRenDev->m_RP.m_nCurBufferOffset = pOffs->OffsTC;
      return &pVertices->st[0];
    case eSrcPointer_Color:
      gRenDev->m_RP.m_nCurBufferOffset = pOffs->OffsColor;
      return &pVertices->color.dcolor;
  }
  return NULL;
}



#ifndef __CRETEMPMESH_H__
#define __CRETEMPMESH_H__

//=============================================================


class CRETempMesh : public CRendElement
{
public:
  CVertexBuffer *m_VBuffer;
  SVertexStream m_Inds;

public:
  CRETempMesh()
  {
    m_VBuffer = NULL;
    m_Inds.Reset();
    mfSetType(eDATA_TempMesh);
    mfUpdateFlags(FCEF_TRANSFORM);
  }

  virtual ~CRETempMesh()
  {
    if (m_VBuffer)
    {
      gRenDev->ReleaseBuffer(m_VBuffer);
      m_VBuffer = NULL;
    }
    gRenDev->ReleaseIndexBuffer(&m_Inds);
    m_Inds.Reset();
  }

  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);
  virtual void *mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags);
  virtual bool mfPreDraw(SShaderPass *sl);
  virtual void mfReset();
  virtual int Size()
  {
    int nSize = sizeof(*this);
    if (m_VBuffer)
      nSize += m_VBuffer->Size(0, m_VBuffer->m_NumVerts);
    return nSize;
  }
};

#endif  // __CRETEMPMESH_H__

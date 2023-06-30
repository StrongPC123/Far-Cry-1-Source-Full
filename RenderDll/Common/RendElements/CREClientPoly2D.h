
#ifndef __CRECLIENTPOLY2D_H__
#define __CRECLIENTPOLY2D_H__

//=============================================================

struct SClientPolyStat2D
{
  int NumOccPolys;
  int NumRendPolys;
  int NumVerts;
  int NumIndices;
};


class CREClientPoly2D : public CRendElement
{
public:
  SShader *mEf;
  SRenderShaderResources *m_pShaderResources;
  short mNumVerts;
  short mNumIndices;
  SColorVert2D mVerts[16];
  byte mIndices[(16-2)*3];

  static SClientPolyStat2D mRS;
  static void mfPrintStat();

public:
  CREClientPoly2D()
  {
    mfSetType(eDATA_ClientPoly2D);
    mNumVerts = 0;
    mEf = NULL;
    m_pShaderResources = NULL;
    mfUpdateFlags(FCEF_TRANSFORM | FCEF_NEEDFILLBUF);
  }

  virtual ~CREClientPoly2D() {};

  virtual void mfPrepare();
  virtual CRendElement *mfCopyConstruct(void);

  static TArray<CREClientPoly2D *> mPolysStorage;
};

#endif  // __CRECLIENTPOLY2D_H__

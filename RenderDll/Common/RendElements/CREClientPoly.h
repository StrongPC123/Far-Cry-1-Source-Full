
#ifndef __CRECLIENTPOLY_H__
#define __CRECLIENTPOLY_H__

//=============================================================

struct SClientPolyStat
{
  int NumOccPolys;
  int NumRendPolys;
  int NumVerts;
  int NumIndices;
};

//#define MAX_CLIENTPOLY_VERTS 16
#define MAX_CLIENTPOLY_VERTS 32

class CREClientPoly : public CRendElement
{
public:
  SShader *mEf;
  int m_nFogID;
  short mNumVerts;
  short mNumIndices;
  CCObject *m_pObject;
  SColorVert mVerts[MAX_CLIENTPOLY_VERTS];
  byte mIndices[(MAX_CLIENTPOLY_VERTS-2)*3];
  float m_fDistance;

  static SClientPolyStat mRS;
  static void mfPrintStat();

public:
  CREClientPoly()
  {
    mfSetType(eDATA_ClientPoly);
    mNumVerts = 0;
    mEf = NULL;
    m_pObject = NULL;
    mfSetFlags(FCEF_TRANSFORM | FCEF_NEEDFILLBUF);
  }

  virtual ~CREClientPoly() {};

  virtual void mfPrepare();

  bool mfCullBox(Vec3d& vmin, Vec3d& vmax);
  float mfDistanceToCameraSquared(const CCObject & thisObject);
  virtual CRendElement *mfCopyConstruct(void);

  static TArray<CREClientPoly *> mPolysStorage[4];
};

#endif  // __CRECLIENTPOLY_H__

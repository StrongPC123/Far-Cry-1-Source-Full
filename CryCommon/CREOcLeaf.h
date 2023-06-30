#ifndef __CREOCLEAF_H__
#define __CREOCLEAF_H__

//=============================================================

struct SLightIndicies
{
  CDLight m_AssociatedLight;
  TArray<ushort> m_pIndicies;
  TArray<ushort> m_pIndiciesAttenFrustr;
  SVertexStream m_IndexBuffer;            // used for DirectX only
  unsigned short *GetIndices(int& nInds)
  {
    if (m_AssociatedLight.m_NumCM & 2)
    {
      nInds = m_pIndiciesAttenFrustr.Num();
      if (nInds)
        return &m_pIndiciesAttenFrustr[0];
      return NULL;
    }
    else
    {
      nInds = m_pIndicies.Num();
      if (nInds)
        return &m_pIndicies[0];
      return NULL;
    }
  }
  SVertexStream *GetIndexBuffer()
  {
    return &m_IndexBuffer;
  }
};

struct SMeshFace
{
  Vec3 m_Normal;
  Vec3 m_Middle; 
};


#define FCEF_CALCCENTER 0x10000
#define FCEF_DYNAMIC    0x20000

class CREOcLeaf : public CRendElement
{
  friend class CRenderer;

  int mFrameCalcLight;
  int mFrameCalcLightNoAtt;
  int mFrameCalcAtten;
  int mFrameCalcRefract;
  int mFrameCalcHalfAngle;
  int mFrameCalcHalfAngleNoAtt;
  int mFrameCalcProject;
  int mFrameCalcLAttenSpec0;
  int mFrameCalcLAttenSpec1;
  int mMaskLight0;  // Light Vectors
  int mMaskLight1;  // HalfAngle Vectors
  int mMaskLight2;  // Light Attenuation ( LightVector/(brightness*2.0)+0.5 )
  int mMaskLight3;  // Light Attenuation ( (LightVector*0.5)+0.5 )
  int mMaskLight4;  // Attenuation
  int mMaskLight5;
  Vec3 m_Center;

public:
  static CREOcLeaf *m_pLastRE;
  struct CMatInfo  *m_pChunk;
  struct CLeafBuffer * m_pBuffer;

  TArray<SMeshFace> *m_Faces;
  TArray<SLightIndicies *> *m_LIndicies;

  CREOcLeaf()
  {
    mfSetType(eDATA_OcLeaf);
    mfUpdateFlags(FCEF_TRANSFORM);
    m_Faces = NULL;
    m_LIndicies = NULL;
    m_pChunk = NULL;
    m_pBuffer = NULL;
    mFrameCalcLight = -1;
    mFrameCalcAtten = -1;
    mFrameCalcRefract = -1;
    mFrameCalcHalfAngle = -1;
    mMaskLight0 = 0;
    mMaskLight1 = 0;
    mMaskLight2 = 0;
    m_Center.Set(0,0,0);
  }
  SLightIndicies *mfGetIndiciesForLight(CDLight *pDLight);
  void mfGenerateIndicesInsideFrustrum(SLightIndicies *li, CDLight *pDLight);
  void mfGenerateIndicesForAttenuation(SLightIndicies *li, CDLight *pDLight);
  void mfFillRB(CCObject *pObj);

  virtual ~CREOcLeaf()
  {
    SAFE_DELETE(m_Faces);
    if (m_LIndicies)
    {
      for (int i=0; i<m_LIndicies->Num(); i++)
      {
        SLightIndicies *li = m_LIndicies->Get(i);
        SAFE_DELETE(li);
      }
      SAFE_DELETE(m_LIndicies);
    }
  }
  virtual struct CMatInfo *mfGetMatInfo();
  virtual list2<struct CMatInfo> *mfGetMatInfoList();
  virtual int mfGetMatId();
  virtual bool mfPreDraw(SShaderPass *sl);
  virtual void mfGetPlane(Plane& pl);
  virtual void mfPrepare();
  virtual bool mfCullByClipPlane(CCObject *pObj);
  virtual void mfCenter(Vec3& Pos, CCObject*pObj);
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);
  virtual void *mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags);
  virtual void mfEndFlush();
  virtual float mfMinDistanceToCamera(CCObject *pObj);
  virtual float mfDistanceToCameraSquared(const CCObject & thisObject);
  virtual bool mfCheckUpdate(int nVertFormat, int Flags);
  virtual void mfGetBBox(Vec3& vMins, Vec3& vMaxs);
  virtual int Size()
  {
    int nSize = sizeof(*this);
    if (m_Faces)
      nSize += m_Faces->GetMemoryUsage();
    if (m_LIndicies)
    {
      nSize += m_LIndicies->GetMemoryUsage();
      for (int i=0; i<m_LIndicies->Num(); i++)
      {
        SLightIndicies *li = m_LIndicies->Get(i);
        nSize += sizeof(*li);
        nSize += li->m_pIndicies.GetMemoryUsage();
        nSize += li->m_pIndiciesAttenFrustr.GetMemoryUsage();
      }
    }
    return nSize;
  }
};

#endif  // __CREOCLEAF_H__


#ifndef __CRETRIMESHSHADOW_H__
#define __CRETRIMESHSHADOW_H__

//=============================================================
#define MAX_SV_INSTANCES	32

//=============================================================
struct	ItShadowVolume;
class		CDLight;

class CRETriMeshShadow : public CRendElement
{
public:

  CRETriMeshShadow()
  {
    mfSetType(eDATA_TriMeshShadow);
    mfUpdateFlags(FCEF_TRANSFORM | FCEF_NODEL);
    memset(m_arrLBuffers,0,sizeof(m_arrLBuffers));
		m_pSvObj=NULL;						
    m_nCurrInst=0;
    m_nRendIndices = 0;
    m_bAnimatedObject = false;
  }

  virtual ~CRETriMeshShadow();

  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);
  virtual void * mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags);
  virtual bool mfCheckUpdate(int nVertFormat, int Flags);
  virtual bool mfCheckUnload();

  //! shadow volume info
  struct ShadVolInstanceInfo
  {
    ShadVolInstanceInfo() { memset(this,0,sizeof(*this)); }
    CLeafBuffer * pVB;
    int nFrameId;
    Vec3 vObjSpaceLightPos;
    IEntityRender * pLightOwner;
    IEntityRender * pShadowCaster;
  };

  //! array of ready shadow volumes
  ShadVolInstanceInfo m_arrLBuffers[MAX_SV_INSTANCES]; // stores shadow volumes
  int m_nCurrInst; // used to pass selected slot from mfCheckUpdate to mfDraw
  int m_nRendIndices;

  bool m_bAnimatedObject;

	//! vertex buffer data set by shadowvolobject	

	ItShadowVolume	*m_pSvObj;

  static int GetAndResetRebuildsPerFrrameCounter();
  static int GetAndResetShadowVolumesPerFrrameCounter();
  static int GetShadowVolumesAllocatedCounter();
  static void PrintStats();

private:	
  static int m_nCRETriMeshShadowRebuildsPerFrrame;
  static int m_nCRETriMeshShadowShadowsPerFrrame;
  static int m_nCRETriMeshShadowAloocatedShadows;
};

#endif  // __CRETRIMESHSHADOW_H__

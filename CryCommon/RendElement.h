
#ifndef __RENDELEMENT_H__
#define __RENDELEMENT_H__

//=============================================================

struct SMsurface;
class CRendElement;
struct CMatInfo;
struct PrimitiveGroup;
struct SShader;
struct SShaderTechnique;
struct Plane;

enum EDataType
{
  eDATA_Unknown = 0,
  eDATA_Dummy,
  eDATA_Sky,		
  eDATA_Beam,		
  eDATA_Poly,
  eDATA_Curve,
  eDATA_MotModel,
  eDATA_MeshModel,
  eDATA_PolyBlend,
  eDATA_AnimPolyBlend,
  eDATA_ClientPoly,
  eDATA_ClientPoly2D,
  eDATA_ParticleSpray,
  eDATA_TriMesh,
  eDATA_TriMeshShadow,
  eDATA_Prefab,
  eDATA_Flare,
  eDATA_FlareGeom,
  eDATA_FlareProp,
  eDATA_Tree,
  eDATA_Tree_Leaves,
  eDATA_Tree_Branches,
  eDATA_Terrain,
  eDATA_SkyZone,
  eDATA_OcLeaf,
  eDATA_TerrainSector,
  eDATA_2DQuad,
  eDATA_FarTreeSprites,
//  eDATA_TriMeshAdditionalShadow,
  eDATA_AnimModel,
  eDATA_MotionBlur,
  eDATA_ShadowMapGen,
  eDATA_TerrainDetailTextureLayers,
  eDATA_TerrainParticles,
  eDATA_Ocean,
  eDATA_Glare,
  eDATA_OcclusionQuery,
  eDATA_TempMesh,
	eDATA_ClearStencil,
  eDATA_FlashBang,

  // tiago: added
  eDATA_ScreenProcess,  
  eDATA_HDRProcess,  
};

#include <ColorDefs.h>

//=======================================================

struct SInpData
{
  Vec3 Org;
  Vec3 Normal;
  CFColor Color;
  int UniqLightStyle;
  int OrigLightStyle;
};

struct SMRendVert
{
  SMRendVert () {}
  SMRendVert (float x, float y, float z) { vert[0] = x; vert[1] = y; vert[2] = z; }
  Vec3 vert;
  union
  {
    uint m_uiInfo;
    uchar m_bInfo[4];
  };
};

struct SMRendTexVert
{
  SMRendTexVert() {}
  SMRendTexVert (float u, float t) { vert[0] = u; vert[1] = t; }
  float vert[2];
};

struct SColorVert
{
  Vec3 vert;
  float dTC[2];
  UCol color;
};

struct SColorVert2D
{
  float vert[2];
  float dTC[2];
  UCol color;
};


//=========================================================

#define FCEF_TRANSFORM 1
#define FCEF_TRACE     2
#define FCEF_NODEL     4

#define FCEF_MODIF_TC   0x10
#define FCEF_MODIF_VERT 0x20
#define FCEF_MODIF_COL  0x40
#define FCEF_MODIF_MASK 0xf0

#define FCEF_NEEDFILLBUF 0x100
#define FCEF_ALLOC_CUST_FLOAT_DATA 0x200
#define FCEF_MERGABLE    0x400

#define FGP_NOCALC 1
#define FGP_SRC    2
#define FGP_REAL   4
#define FGP_WAIT   8

#define FGP_STAGE_SHIFT 0x10

struct SVertBufComps
{
  bool m_bHasTC;
  bool m_bHasColors;
  bool m_bHasSecColors;
  bool m_bHasNormals;
};

#define MAX_CUSTOM_TEX_BINDS_NUM 8

class CRendElement
{
public:
  EDataType m_Type;
  uint m_Flags;

public:
  int m_nCountCustomData;
  void *m_CustomData;
  float m_fFogScale;
  int m_CustomTexBind[MAX_CUSTOM_TEX_BINDS_NUM];
  CFColor m_Color;
  int m_SortId;

  static CRendElement m_RootGlobal;
  CRendElement *m_NextGlobal;
  CRendElement *m_PrevGlobal;

  class CVProgram *m_LastVP;      // Last Vertex program which updates Z buffer

protected:


  _inline void UnlinkGlobal()
  {
    if (!m_NextGlobal || !m_PrevGlobal)
      return;
    m_NextGlobal->m_PrevGlobal = m_PrevGlobal;
    m_PrevGlobal->m_NextGlobal = m_NextGlobal;
    m_NextGlobal = m_PrevGlobal = NULL;
  }
  _inline void LinkGlobal( CRendElement* Before )
  {
    if (m_NextGlobal || m_PrevGlobal)
      return;
    m_NextGlobal = Before->m_NextGlobal;
    Before->m_NextGlobal->m_PrevGlobal = this;
    Before->m_NextGlobal = this;
    m_PrevGlobal = Before;
  }

public:
  CRendElement()
  {
    m_Type = eDATA_Unknown;
    m_NextGlobal = NULL;
    m_PrevGlobal = NULL;
    m_Flags = 0;
    m_CustomData = NULL;
		for(int i=0; i<MAX_CUSTOM_TEX_BINDS_NUM; i++)
	    m_CustomTexBind[i] = -1;
    m_fFogScale=0;
    m_SortId = 0;
    m_LastVP = NULL;
    if (!m_RootGlobal.m_NextGlobal)
    {
      m_RootGlobal.m_NextGlobal = &m_RootGlobal;
      m_RootGlobal.m_PrevGlobal = &m_RootGlobal;
    }
    if (this != &m_RootGlobal)
      LinkGlobal(&m_RootGlobal);
  }

  virtual ~CRendElement()
  {
    if ((m_Flags & FCEF_ALLOC_CUST_FLOAT_DATA) && m_CustomData)
		{
			delete [] ((float*)m_CustomData);
			m_CustomData=0;
		}
    UnlinkGlobal();
  }

  const char *mfTypeString();

  EDataType mfGetType() { return m_Type; }

  void mfSetType(EDataType t) { m_Type = t; }

  uint mfGetFlags(void) { return m_Flags; }
  void mfSetFlags(uint fl) { m_Flags = fl; }
  void mfUpdateFlags(uint fl) { m_Flags |= fl; }
  void mfClearFlags(uint fl) { m_Flags &= ~fl; }

  virtual void mfPrepare();
  virtual bool mfCullByClipPlane(CCObject *pObj);
  virtual CMatInfo *mfGetMatInfo();
  virtual list2<CMatInfo> *mfGetMatInfoList();
  virtual int mfGetMatId();
  virtual bool mfCull(CCObject *obj);
  virtual bool mfCull(CCObject *obj, SShader *ef);
  virtual void mfReset();
  virtual CRendElement *mfCopyConstruct(void);
  virtual void mfCenter(Vec3& centr, CCObject*pObj);
  virtual void mfGetBBox(Vec3& vMins, Vec3& vMaxs)
  {
    vMins.Set(0,0,0);
    vMaxs.Set(0,0,0);
  }
  virtual void mfGetPlane(Plane& pl);
  virtual float mfDistanceToCameraSquared(const CCObject & thisObject);
  virtual void mfEndFlush();
  virtual void Release();
  virtual int mfTransform(Matrix44& ViewMatr, Matrix44& ProjMatr, vec4_t *verts, vec4_t *vertsp, int Num);
  virtual bool mfIsValidTime(SShader *ef, CCObject *obj, float curtime);
  virtual void mfBuildGeometry(SShader *ef);
  virtual bool mfCompile(SShader *ef, char *scr);
  virtual CRendElement *mfCreateWorldRE(SShader *ef, SInpData *ds);
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);
  virtual void *mfGetPointer(ESrcPointer ePT, int *Stride, int Type, ESrcPointer Dst, int Flags);
  virtual bool mfPreDraw(SShaderPass *sl) { return true; }
  virtual float mfMinDistanceToCamera(CCObject *pObj) {return -1;};
  virtual bool mfCheckUpdate(int nVertFormat, int Flags) {int i=Flags; return true;}
  virtual int Size() {return 0;}
};

#include "CREOcLeaf.h"
#include "CRESky.h"
#include "CRE2DQuad.h"
#include "CREDummy.h"
#include "CRETerrainSector.h"
#include "CRETriMeshShadow.h"
#include "CRETriMeshAdditionalShadow.h"
#include "CREShadowMap.h"
#include "CREOcclusionQuery.h"
#include "CREFlashBang.h"

// tiago: added
#include "CREGlare.h"  
#include "CREScreenProcess.h" 

//==========================================================

#endif  // __RENDELEMENT_H__

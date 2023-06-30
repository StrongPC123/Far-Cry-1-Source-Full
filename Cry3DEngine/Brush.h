#ifndef _3DENGINE_BRUSH_H_
#define _3DENGINE_BRUSH_H_

#include "ObjMan.H"

#if defined(LINUX)
#include "LMCompStructures.h"
#include "platform.h"
#else
struct RenderLMData;
#endif
TYPEDEF_AUTOPTR(RenderLMData);

struct SLMData
{
	SLMData() { m_pLMTCBuffer=0; }
	RenderLMData_AutoPtr m_pLMData;
	struct CLeafBuffer * m_pLMTCBuffer;
};

class CBrush : public IEntityRender, public Cry3DEngineBase
{
public:
  CBrush();
  virtual ~CBrush();
  virtual const char * GetEntityClassName() const;
  virtual const Vec3d & GetPos(bool bWorldOnly = true) const;
  virtual const Vec3d & GetAngles(int realA=0) const;
  virtual float GetScale() const;
  virtual const char *GetName() const;
  virtual void	GetRenderBBox( Vec3d &mins,Vec3d &maxs );
  virtual float GetRenderRadius() const;
  virtual bool HasChanged();
  virtual bool DrawEntity(const struct SRendParams & EntDrawParams);
  virtual bool IsStatic() const;
  virtual struct IStatObj * GetEntityStatObj( unsigned int nSlot, Matrix44 * pMatrix = NULL, bool bReturnOnlyVisible = false);
  virtual struct ICryCharInstance* GetEntityCharacter( unsigned int nSlot, Matrix44 * pMatrix = NULL );

  virtual void SetEntityStatObj( unsigned int nSlot, IStatObj * pStatObj, Matrix44 * pMatrix = NULL );

  virtual void SetLightmap(RenderLMData *pLMData, float *pTexCoords, UINT iNumTexCoords, int nLod);
  //special call from lightmap serializer/compiler to set occlusion map values
  virtual void SetLightmap(RenderLMData *pLMData, float *pTexCoords, UINT iNumTexCoords, const unsigned char cucOcclIDCount, const std::vector<std::pair<EntityId, EntityId> >& aIDs);

  virtual bool HasLightmap(int nLod)
  {
    // only 2 conditions are valid
    //assert((m_pLMData != NULL && m_pLMTCBuffer != NULL) || (m_pLMData == NULL && m_pLMTCBuffer == NULL));
#if !defined(LINUX64)
    if (m_arrLMData[nLod].m_pLMData == NULL || m_arrLMData[nLod].m_pLMTCBuffer == NULL)
#else
    if (m_arrLMData[nLod].m_pLMData == 0 || m_arrLMData[nLod].m_pLMTCBuffer == 0)
#endif
      return false; // return to avoid crash somewhere if in release mode
    return true;
  };
  virtual RenderLMData * GetLightmap(int nLod) { return m_arrLMData[nLod].m_pLMData; };
  virtual struct CLeafBuffer * GetLightmapTexCoord(int nLod) { return m_arrLMData[nLod].m_pLMTCBuffer; };

  virtual bool IsEntityHasSomethingToRender();
  virtual bool IsEntityAreasVisible();
  virtual IPhysicalEntity* GetPhysics() const ;
  virtual void SetPhysics( IPhysicalEntity* pPhys );
  static bool IsMatrixValid(const Matrix44 & mat);
  void DeleteLMTC();
  void Dephysicalize( );
	virtual void Physicalize(bool bInstant=false);

  //! Assign override material to this entity.
  virtual void SetMaterial( IMatInfo *pMatInfo );
  virtual void SetMaterialId( int nId ) { m_nMaterialId = nId; }
  virtual IMatInfo* GetMaterial() const;
  int GetEditorObjectId() { return m_nEditorObjectId; }
  void SetEditorObjectId(int nEditorObjectId) { m_nEditorObjectId = nEditorObjectId; }
  virtual void CheckPhysicalized();
  int DestroyPhysicalEntityCallback(IPhysicalEntity *pent);
//  int GetPhysGeomId(int nSlotId) { return m_arrPhysGeomId[nSlotId]; }

  virtual float GetMaxViewDist();
  virtual void Serialize(bool bSave, ICryPak * pPak, FILE * f);
  virtual EERType GetEntityRenderType() { return eERType_Brush; }

  void SetStatObjGroupId(int nStatObjInstanceGroupId) { m_nObjectTypeID = nStatObjInstanceGroupId; }
  void SetMergeId(int nId) { m_nMergeID = nId; }
  int GetMergeId() { return m_nMergeID; }

  static list2<IStatObj*> m_lstBrushTypes;

  void SetMatrix( Matrix44* pMatrix );
  void Dematerialize( );
  virtual int GetMemoryUsage();
  static list2<SExportedBrushMaterial> m_lstSExportedBrushMaterials;
	virtual void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime);

protected:
  void CalcWholeBBox();

  Vec3d m_vPos, m_vAngles;
  float m_fScale;
//  struct IStatObj * m_pStatObj;
  Matrix44 m_Matrix;
  IPhysicalEntity * m_pPhysEnt;

  //! Override material.
  _smart_ptr<IMatInfo> m_pMaterial;
  int m_nMaterialId;

#define MAX_BRUSH_LODS_NUM 3
	SLMData m_arrLMData[MAX_BRUSH_LODS_NUM];

  int m_nEditorObjectId;
  int m_nObjectTypeID;
  int m_nMergeID;

  //needed for occlusion maps
//  std::vector<std::pair<EntityId, EntityId> > m_vnOcclIndices;//first one is real EntityID, second one is (entity)light index in StatLights.dat
	IEntityRender * m_arrOcclusionLightOwners[4];
};

#endif // _3DENGINE_BRUSH_H_

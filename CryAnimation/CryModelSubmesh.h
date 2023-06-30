#ifndef _CRY_ANIMATION_CRY_MODEL_SUBMESH_HDR_
#define _CRY_ANIMATION_CRY_MODEL_SUBMESH_HDR_

class CryCharBody;
TYPEDEF_AUTOPTR(CryCharBody);
class CryModEffMorph;
class CryCharDecalManager;
#include "CryAnimationBase.h"
#include "CryCharReShadowManager.h"
#include "CryModelState.h"

//////////////////////////////////////////////////////////////////////////
// Instance of this class represents the subskin of a character,
// that can be turned on and off by will (to render/not to render),
// has its own morph targets and morph effectors, and leaf buffers and the material list
//
// It does NOT have (they're all in CryModelState):
//  animation effectors
//  bones
//
// Although the aniimations themselves (the controllers) are taken from one
// of the cryModels referred to by one of the CryModelState submeshes,


class CryModelSubmesh:
	public ICryCharSubmesh,       // interface to the CryEngine
	public IDeformableRenderMesh // interface with the Renderer
{
	friend class CryModelState;
	friend class CryModel;
public:
	enum {g_nMaxLods = CryModelState::g_nMaxLods};
	// initializes and binds the submesh to the given model
	// there's no way to change the model at runtime
	CryModelSubmesh(CryModelState* pParent, CryModel* pMesh);
	~CryModelSubmesh();


	// this is the array that's returned from the LeafBuffer
	list2<CMatInfo>* getLeafBufferMaterials();

	// Generates from geom data arrays which can be used  
	void GenerateRenderArrays(const char * szFileName);

	// Creates Leaf buffers for all LODs
	// szTextureDir is the texture directory used to create the shaders
	void GenerateRenderArraysCCG(const char * szTextureDir);

	void CopyLeafBuffers (CLeafBuffer** pLeafBuffers);

	// returns true if the given submesh is visible
	virtual bool IsVisible();

	// depending on bVisible, either makes the submesh visible or invisible
	virtual void SetVisible(bool bVisible = true);

	// returns the model of the submesh, or NULL in case of failure
	virtual ICryCharModel* GetModel ();
	CryModel* GetCryModel(){return m_pMesh;}
	const CryModel* GetCryModel()const {return m_pMesh;}

	//! Start the specified by parameters morph target
	virtual void StartMorph (const char* szMorphTarget, const CryCharMorphParams& params) {RunMorph(szMorphTarget, params);}

	//! Start the specified by parameters morph target
	virtual void StartMorph (int nMorphTargetId, const CryCharMorphParams& params) {RunMorph (nMorphTargetId, params);}

	//! Finds the morph with the given id and sets its relative time.
	//! Returns false if the operation can't be performed (no morph)
	//! The default implementation for objects that don't implement morph targets is to always fail
	virtual bool SetMorphTime (int nMorphTargetId, float fTime);

	//! Set morph speed scale
	//! Finds the morph target with the given id, sets its morphing speed and returns true;
	//! if there's no such morph target currently playing, returns false
	virtual bool SetMorphSpeed (int nMorphTargetId, float fSpeed);

	//! Stops morph by target id
	virtual bool StopMorph (int nMorphTargetId);

	//! Stops all morphs
	virtual void StopAllMorphs();
	void FreezeAllMorphs();

	// returns true if calling Morph () is really necessary (there are any pending morphs)
	bool NeedMorph ();

	// morphs the LOD 0 into the given destination (already skinned) buffer
	// pDstNormlas can be non-NULL, in which case the normals are (fakely) modified
	void MorphWithSkin (Vec3d* pDst, Vec3dA16* pDstNormalsA16 = NULL);

	void AddCurrentRenderData(CCObject *obj, CCObject *obj1, const SRendParams & rParams);

	// renders the decals - adds the render element to the renderer
	void AddDecalRenderData (CCObject *pObj, const SRendParams & rRendParams);

	// Adds a decal to the character
	void AddDecal (CryEngineDecalInfo& Decal);

	void ClearDecals();

	void ShrinkShadowPool()
	{
		m_ReShadowManager.shrink();
	}

	//this is called only at the beginning once for each LOD
	void DeformFirst();

	// Normal-skins this model (into the given storage)
	Vec3dA16* SelfNormalSkin (int nLOD, Vec3dA16* pNormals);

	// skins this model (into the given temporary storage, if needed)
	// if no storage is provided, then allocates its own storage (g_Temp.data())
	// can return the model pre-skinned data (i.e. the returned value is not guaranteed to 
	// be the same as the buffer passed
	// The normals, if not NULL, are modified according to the vertex movements
	const Vec3d* SelfSkin(int nLOD, Vec3d*pVertices = NULL, Vec3dA16* pNormalsA16 = NULL);

	// do not calculate normals and do not copy into video buffers
	// Returns: number of vertices
	const Vec3d* DeformForShadowVolume( int nLodToDeform );

	enum
	{
		FLAG_DEFORM_UPDATE_TANGENTS = 1,
		FLAG_DEFORM_UPDATE_NORMALS  = 1 << 1,
		FLAG_DEFORM_UPDATE_VERTICES = 1 << 2,
		FLAG_DEFORM_FORCE_UPDATE    = 1 << 3
	};

	// Software skinning: calculate positions and normals
	// the flags are the same as in CryModelState
	void Deform( int nLodToDeform, unsigned nFlags);

	//Calculate shadow volumes,fill buffers and render shadow volumes into the stencil buffer
	//TODO: Optimize everything
	void RenderShadowVolumes (const SRendParams *rParams, int nLimitLOD);

	// Compute all effectors and remove those whose life time has ended, then apply effectors to the bones
	void ProcessSkinning(const Vec3& t, const Matrix44& mtxModel, int nTemplate, int nLod, bool bForceUpdate);

	void Render(const struct SRendParams & RendParams, Matrix44& mtxObjMatrix, CryCharInstanceRenderParams& rCharParams, const Vec3& t);

	// returns true if the decals need realization (otherwise RealizeDecals() might not be called)
	bool NeedRealizeDecals ();

	// the modelstate gives itself a chance to process decals (realize)
	// in this call. The decal realization means creation of geometry that carries the decal
	void RealizeDecals (const Vec3d* pPositions);

	// discards all outstanding decal requests - the decals that have not been meshed (realized) yet
	// and have no chance to be correctly meshed in the future
	void DiscardDecalRequests();

	bool SetShaderTemplateName (const char *TemplName, int Id, const char *ShaderName,IMatInfo *pCustomMaterial,unsigned nFlags);

	CLeafBuffer* GetLeafBuffer ();

	void SetShaderFloat (const char *Name, float fVal, const char *ShaderName=NULL);

	void UpdateMorphEffectors (float fDeltaTimeSec);

	bool RunMorph (const char* szMorphTarget, const CryCharMorphParams& Params, bool bShowNotFoundWarning = true);
	void RunMorph (int nMorphTargetId, const CryCharMorphParams& Params);

	//returns the morph target effector, or NULL if no such effector found
	CryModEffMorph* getMorphTarget (int nMorphTargetId);

	void PreloadResources(float fDistance, float fTime, int nFlags);
protected:
	void DeleteLeafBuffers();

	CryModel * m_pMesh;
	CLeafBuffer * m_pLeafBuffers[g_nMaxGeomLodLevels];

	typedef std::vector<CryModEffMorph> MorphEffectorArray;
	MorphEffectorArray m_arrMorphEffectors;

	// this is the decal manager singletone: created only when first needed
	CryCharDecalManager *m_pDecalManager;

	// this is the shadow volume helper
	CryCharReShadowManager m_ReShadowManager;

	CryModelState* m_pParent;

	CryAABB m_SubBBox;
	int m_nLastSkinBBoxUpdateFrameId, m_nLastTangentsUpdatedFrameId, m_nLastTangentsUpdatedLOD;
	int m_nLastSkinnedFrameID[g_nMaxLods];
	int m_nLastTangedFrameID[g_nMaxLods];
	int m_nLastUpdatedLodLevel;

	// array of shader templates: initialized to keep the track of the allocated memory
	struct ShaderTemplates: public TFixedArray<int>
	{
		ShaderTemplates(): TFixedArray<int>("CryModelState.ShaderTemplates") {}
	};
	ShaderTemplates m_arrShaderTemplates[2];
	ShaderTemplates& getShaderTemplates (int nId)
	{
		assert (nId >= 0 && nId < 2);
		return m_arrShaderTemplates[nId];
	}

	// this array needs to be freed up upon destruction
	typedef TArray<SShaderParam> SShaderParamArray;
	SShaderParamArray m_ShaderParams;

	enum
	{
		FLAG_VISIBLE       = 1,
		FLAG_ENABLE_DECALS = 1 << 1,

		FLAG_DEFAULT_FLAGS = FLAG_ENABLE_DECALS
	};
	// static flags
	unsigned m_nFlags;

	// this autopointer locks the body and model in memory for all the time of 
	// this submesh existence. Due to the initial design, Body owns CryModel, so
	// we have to lock it. This pointer may not be used at all, but it's very important.
	// an alternative would be to call AddRef on it in the constructor and Release() in destructor
	//CryCharBody_AutoPtr m_pBody;
};

#endif
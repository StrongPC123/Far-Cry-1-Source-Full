/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Oscar Blasco
//	Taken over by Vladimir Kajalin, Andrey Honich
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_MODEL_STATE_HEADER_
#define _CRY_MODEL_STATE_HEADER_

#include "SSEUtils.h"
#include "ICryAnimation.h"
#include "CryModel.h"
#include "MathUtils.h"
#include "CryCharReShadowManager.h"
#include "CryCharParticleManager.h"
#include "CryCharAnimationParams.h"

class CryCharFxTrail;
TYPEDEF_AUTOPTR(CryCharFxTrail);

struct CryCharMorphParams;
class CCryModEffAnimation;
TYPEDEF_AUTOPTR(CCryModEffAnimation);

class CController;

class CryCharDecalManager;
class CBoneLightDynamicBind;

//forward declarations
//////////////////////////////////////////////////////////////////////////////////////////
class		CryCharInstance;
class		CryModel;
struct	CObjFace;
struct	SRendParams;
class CryModEffMorph;
//////////////////////////////////////////////////////////////////////////////////////////
// The CryModelState class 
//////////////////////////////////////////////////////////////////////////////////////////

// currently bbox from skin is unsupported
#define BBOX_FROM_SKIN 0

struct aux_bone_info {
	int iBone;
	vectorf dir0;
	float rlen0;
	quaternionf quat0;
};

struct aux_phys_data {
	IPhysicalEntity *pPhysEnt;
	const char *strName;
	int nBones;
	aux_bone_info *pauxBoneInfo;
};

class CryModelSubmesh;
TYPEDEF_AUTOPTR(CryModelSubmesh);


//////////////////////////////////////////////////////////////////////////////////////////
// Contains current state of character ( all unique data for character instance ):
// Bones, physical representation, reference to vertex buffers allocated in renderer, shadow volume data.
// It performs bone loading, character skinning, some physical calculations, rendering character and shadow volumes.
class CryModelState
#ifdef _DEBUG
//	,public IRendererCallbackClient
#endif
{
friend class CryModel; 
friend class CryModelGeometryLoader; 
friend class CryCharInstance;
friend class CryModelSubmesh;
public:
	static void initClass();
	static void deinitClass();

	// this is the array that's returned from the LeafBuffer
	list2<CMatInfo>* getLeafBufferMaterials();

  CryModelState(CryModel* pMesh);
  ~CryModelState();

  //////////////////////////////////////////////////////////////////////////////////////////
  // ------------------------------- BASIC CONTROL FUNCS -------------------------------- //
  //////////////////////////////////////////////////////////////////////////////////////////
    
//  void PushEffector(CCryModEffector *eff);      // Puts an effector in list 
  void ProcessSkinning(const Vec3& t,const Matrix44& mtxModel,int nTemplate, int nLod=-1, bool bForceUpdate=false);

	void Render(const struct SRendParams & RendParams, Matrix44& mtxObjMatrix, struct CryCharInstanceRenderParams& rCharParams, const Vec3& t);
  
  void ProcessAnimations(float deltatime_anim, bool bUpdateBones, CryCharInstance* instance); // Process this model's animations

  CryModelState* MakeCopy();    // Makes an exact copy of this 
                                // model and returns it

	int numLODs(); // number of LODs in the 0th submesh

	// copies the given leaf buffers to this instance leaf buffers
	void CopyLeafBuffers (CLeafBuffer** pLeafBuffers);

  ICryBone * GetBoneByName(const char * szBoneName);
  bool SetAnimationFrame(const char * szString, int nFrame);

	void BuildPhysicalEntity(IPhysicalEntity *pent,float mass,int surface_idx,float stiffness_scale, float scale,Vec3 offset, int nLod=0);
	IPhysicalEntity *CreateCharacterPhysics(IPhysicalEntity *pHost, float mass,int surface_idx,float stiffness_scale, float scale,Vec3 offset, int nLod=0);
	int CreateAuxilaryPhysics(IPhysicalEntity *pHost, float scale,Vec3 offset, int nLod=0);
	void SynchronizeWithPhysicalEntity(IPhysicalEntity *pent, const Vec3& posMaster,const Quat& qMaster, Vec3 offset,int iDir=-1);
	// Forces skinning on the next frame
	void ForceReskin();
	IPhysicalEntity *RelinquishCharacterPhysics();
	void SetCharacterPhysParams(float mass,int surface_idx,float scale) { m_fMass=mass; m_iSurfaceIdx=surface_idx; m_fScale=scale; }
  void ProcessPhysics(float fDeltaTimePhys, int nNeff);
	IPhysicalEntity *GetCharacterPhysics() { return m_pCharPhysics; }
	IPhysicalEntity *GetCharacterPhysics(const char *pRootBoneName);
	IPhysicalEntity *GetCharacterPhysics(int iAuxPhys);
	void DestroyCharacterPhysics(int iMode=0);

	void ResetNonphysicalBoneRotations (int nLOD, float fBlend);

  bool IsAnimStopped();

	void DumpState();

	void ResetBBoxCache()
	{
		m_nLastSkinBBoxUpdateFrameId = 0;
	}
  //////////////////////////////////////////////////////////////////////////////////////////

  
  //////////////////////////////////////////////////////////////////////////////////////////
  // ------------------------------- EFFECTORS FUNCTIONS -------------------------------- //
  //////////////////////////////////////////////////////////////////////////////////////////
  //                                            
  //////////////////////////////////////////////////////////////////////////////////////////
  // RunAnimation:                                    
  //   Searchs for the animation in animations list and starts palying it.        
  //////////////////////////////////////////////////////////////////////////////////////////
	bool RunAnimation (const char * szAnimName, const struct CryCharAnimationParams& Params, float fSpeed);
	bool RunAnimation (int nAnimID, const CryCharAnimationParams& Params, float fSpeed, bool bInternal = false);
	bool RunMorph (const char* szMorphTarget, const CryCharMorphParams& Params);
	//! Finds the morph with the given id and sets its relative time.
	//! Returns false if the operation can't be performed (no morph)
	bool StopAnimation (int nLayer);
	void StopAllMorphs();
	void FreezeAllMorphs();

 

	// returns the animation currently being played in the given layer, or -1
	int GetCurrentAnimation (unsigned nLayer);

	bool RunMorph (int nMorphTargetId, float fBlendInTime, float fBlendOutTime);
	
	// Enables/Disables the Default Idle Animation restart.
	// If this feature is enabled, then the last looped animation will be played back after the current (non-loop) animation is finished.
	// Only those animations started with the flag bTreatAsDefaultIdleAnimation == true will be taken into account
	void EnableIdleAnimationRestart (unsigned nLayer, bool bEnable = true);

	// forgets about all default idle animations
	void ForgetDefaultIdleAnimations();

	// sets the given aniimation to the given layer as the default
	void SetDefaultIdleAnimation(unsigned nLayer, const char* szAnimName);

//  void ChangeBlendSpeedFactor(float factor, int layer);
  //                                            
  // ==================================================================================== //
  // ResetAllAnimations:                                                                  //
  //   Stops all running anims and sets the model to the default pose                     //
  // ==================================================================================== //
  void ResetAllAnimations();
  //                                            //
  //////////////////////////////////////////////////////////////////////////////////////////
	bool AddImpact(const int partid, vectorf point,vectorf impact,float scale);
	int TranslatePartIdToDeadBody(int partid);
	//////////////////////////////////////////////////////////////////////////////////////////
	void SetLimbIKGoal(int limbid, float scale, vectorf ptgoal=vectorf(1E10f,0,0), int ik_flags=0, float addlen=0, vectorf goal_normal=vectorf(zero));
	vectorf GetLimbEndPos(int limbid,float scale);
	//////////////////////////////////////////////////////////////////////////////////////////

	// adds all heat sources ( currently head and heart) to the given object via AddHeatSource interface (member function)
	void UpdateHeatSources (CCObject * pObj, const SRendParams & RendParams);

	// updates the dynamically (via ICryCharInstance at runtime) bound lights
	void UpdateDynBoundLights (CCObject * pObj, const SRendParams & RendParams);

	// adds a new dynamically bound light
	CDLight* AddDynBoundLight (ICryCharInstance* pParent,CDLight* pDLight, unsigned nBone, bool bCopyLight);
	// removes the dynamically bound light
	void RemoveDynBoundLight (CDLight* pDLight);
	unsigned numDynBoundLights()const;
	CDLight* getDynBoundLight(unsigned i);
	void RemoveAllDynBoundLights();
	// checks if such light is already bound
	bool IsDynLightBound (CDLight*pDLight);

	// Interface for the renderer - returns the CDLight describing the light in this character;
	// returns NULL if there's no light with such index
	const CDLight* getGlobalBoundLight (unsigned nIndex);


	//! render character's shadow volume 
	void	RenderShadowVolumes(const SRendParams *rParams, int nLimitLOD);
  
	// Set the current time of the given layer, in seconds
	void SetLayerTime (unsigned nLayer, float fTimeSeconds);
	float GetLayerTime (unsigned nLayer) const;

	// checks for possible memory corruptions in this object and its children
	void SelfValidate ()const
#ifndef _DEBUG
	{}
#endif
		;

	unsigned getInstanceNumber()const {return m_nInstanceNumber;}
	// returns the root bone
	inline CryBone* getRootBone(){return m_arrBones.empty()?NULL:&m_arrBones[0];}
	inline const CryBone* getRootBone()const{return m_arrBones.empty()?NULL:&m_arrBones[0];}

	DECLARE_ARRAY_GETTER_METHODS(CryBone, Bone, Bones, m_arrBones)

	// returns the approximate bounding box for this character in the passed in vectors
	void GetBoundingBox (Vec3& vMin, Vec3& vMax) const;

	Vec3 GetCenter()const;

	// discards all outstanding decal requests - the decals that have not been meshed (realized) yet
	// and have no chance to be correctly meshed in the future
	void DiscardDecalRequests();

	// Disposes the vertex buffers allocated for this character
	void DeleteLeafBuffers();

	// Loads the caf file
	void InitBones(bool bBoneInfoDefPoseNeedInitialization);

	// Creates the bones
	void CreateBones();

	// Adds a decal to the character
	void AddDecal (CryEngineDecalInfo& Decal);
	// cleans up all decals
	void ClearDecals();
	void DumpDecals();

	// creates the initial pose, initializes the bones (builds the inverse transform of the global) and IK limb pose
	void InitDefaultPose(bool bInitBoneInfos, bool bTakePoseFromBoneInfos = false);
	// sets the default position, getting it from the inverse default matrix in the bone infos
	void SetPoseFromBoneInfos();

	// given the bone index, (INDEX, NOT ID), returns this bone's parent index
	int getBoneParentIndex (int nBoneIndex);
	int getBoneParentIndex (const CryBone* pBone) {return getBoneParentIndex(pBone - getBones());}
	int getBonePhysParentIndex (int nBoneIndex, int nLod=0); // same, but finds the first ancestor that has physical geometry
	int getBonePhysChildIndex (int nBoneIndex, int nLod=0);

	unsigned numBoneChildren (int nBoneIndex) {return getBoneInfo(nBoneIndex)->numChildren();}

	//returns the i-th child of the given bone
	int getBoneChildIndex (int nBone, int i);
	CryBone* getBoneChild (int nBone, int i);

	//returns the j-th child of i-th child of the given bone
	int getBoneGrandChildIndex (int nBone, int i, int j);
	CryBone* getBoneGrandChild (int nBone, int i, int j);

	CryBoneInfo* getBoneInfo(int nBone) {return &GetModel()->getBoneInfo(nBone);}
	const CryBoneInfo* getBoneInfo(int nBone) const {return &GetModel()->getBoneInfo(nBone);}
	CryBoneInfo* getBoneInfo(const CryBone* pBone) {return getBoneInfo(pBone - &getBone(0));}
	const CryBoneInfo* getBoneInfo(const CryBone* pBone) const {return getBoneInfo(pBone - getBones());}

	void setBoneParent();

	ICharInstanceSink* getAnimationEventSink (int nAnimId);
	void setAnimationEventSink (int nAnimId, ICharInstanceSink* pSink);
	void removeAllAnimationEvents();
	void removeAnimationEventSink(ICharInstanceSink * pCharInstanceSink);

	CryModel* getAnimationSet()
	{
		SelfValidate();
		return GetModel();
	}

	// this is no more than array of AnimEvent's
	// the class is derived from it to make it easier debugging
	// the memory allocations with the default mem owner name
	typedef std::vector<AnimEvent> AnimEventArray;

	AnimEventArray& getAnimEvents(int nAnimId);
	void addAnimEvent (int nAnimId, int nFrame, AnimSinkEventData UserData);
	void removeAnimEvent (int nAnimId, int nFrame, AnimSinkEventData UserData);

	const Matrix44* getBoneGlobalMatrices() const {return &m_arrBoneGlobalMatrices[0];}
	const Matrix44& getBoneMatrixGlobal (int nBone) const {return m_arrBoneGlobalMatrices[nBone];}
	Matrix44& getBoneMatrixGlobal (int nBone) {return m_arrBoneGlobalMatrices[nBone];}
	Matrix44& getBoneMatrixGlobal (const CryBone* pBone) { return getBoneMatrixGlobal (pBone - getBones()); }

	// calculates the global matrices
	// from relative to parent matrices
	void UpdateBoneMatricesGlobal();

	// Replaces each bone global matrix with the relative matrix.
	void ConvertBoneGlobalToRelativeMatrices ();

	// Multiplies each bone global matrix with the parent global matrix,
	// and calculates the relative-to-default-pos matrix. This is essentially
	// the process opposite to conversion of global matrices to relative form
	// performed by ConvertBoneGlobalToRelativeMatrices()
	void UnconvertBoneGlobalFromRelativeForm(bool bNonphysicalOnly, int nLod = 0);

	// draws the skeleton
	void debugDrawBones(const Matrix44* pModelMatrix = NULL);
	void debugDrawDynBoundLights(const Matrix44* pModelMatrix);

	void debugDrawBoundingBox (const Matrix44* pModelMatrix, int nBBoxSegments = 1);

	// calculates the mem usage
	void GetSize(ICrySizer* pSizer);


	void setAnimationSpeed (unsigned nLayer, float fSpeed);
	float getAnimationSpeed (unsigned nLayer);

	const CryAABB& getBBox() const{return m_BBox;}
	void InitBBox();
	// notifies the renderer that the character will soon be rendered
	void PreloadResources ( float fDistance, float fTime, int nFlags);

	// the model that's coherent with the current model state: bones etc. are taken from there
	CryModel* GetModel();
	const CryModel* GetModel()const;

	// adds a submesh, returns handle to it which can be used to delete the submesh
	// submesh is created either visible or invisible
	// submesh creation/destruction is heavy operations, so the clients must use they rarely,
	// and set visible/invisible when they need to turn them on/off
	// But creating many submeshes is memory-consuming so the number of them must be kept low at all times
	CryModelSubmesh* AddSubmesh (ICryCharModel* pModel, bool bVisible = false);
	CryModelSubmesh* SetSubmesh (unsigned nSlot, ICryCharModel* pModel, bool bVisible = false);
	CryModelSubmesh* SetSubmesh (unsigned nSlot, CryModel* pModel, bool bVisible = false);
	CryModelSubmesh* AddSubmesh (CryModel* pModel, bool bVisible = false);

	// removes submesh from the character
	void RemoveSubmesh (ICryCharSubmesh* pSubmesh);
	void RemoveSubmesh (unsigned nSlot);

	// enumeration of submeshes
	size_t NumSubmeshes() {return m_arrSubmeshes.size();}
	ICryCharSubmesh* GetSubmesh(unsigned i);
	CryModelSubmesh* GetCryModelSubmesh(unsigned i);

	CryModel* GetMesh();

	// returns true if there are no submeshes yet
	bool IsEmpty()const {return m_arrSubmeshes.empty();}

	bool SetShaderTemplateName (const char *TemplName, int Id, const char *ShaderName,IMatInfo *pCustomMaterial,unsigned nFlags);
	CLeafBuffer* GetLeafBuffer ();

	void SetShaderFloat(const char *Name, float fVal, const char *ShaderName=NULL);


private:
	// sets the animation speed scale for layers
	std::vector<float> m_arrLayerSpeedScale;


	// this array is used for temporal storage many times on every frame to compute the animations
	// to apply to the character instance. In order to avoid frequent reallocations, it's here
	typedef std::vector<CAnimationLayerInfo> ActiveLayerArray;
	static ActiveLayerArray g_arrActiveLayers;

	// updates the *ModEff* - adds the given delta to the current time,
	// calls the callbacks, etc. Returns the array describing the updated anim layers,
	// it can be applied to the bones
	void UpdateAnimatedEffectors (float fDeltaTimeSec, ActiveLayerArray& arrActiveLayers);

	// updates the bone matrices using the given array of animations -
	// applies the animation layers to the bones
	void UpdateBones (const ActiveLayerArray& arrActiveLayers);

	void ApplyAnimationToBones (CAnimationLayerInfo AnimLayer);
	void ApplyAnimationsToBones (const CAnimationLayerInfo* pAnims, unsigned numAnims);

	// out of the bone positions, calculates the bounding box for this character and puts it
	// into m_vBoxMin,m_vBoxMax
  void UpdateBBox();
  
	void ValidateBBox();


	// renders the decals - adds the render element to the renderer
	void AddDecalRenderData (CCObject *obj, const SRendParams & RendParams);
	void AddFxRenderData (CCObject *obj, const SRendParams & RendParams);

	CryCharFxTrail* NewFxTrail (unsigned nSlot, const struct CryCharFxTrailParams& rParams);
	void RemoveFxTrail(unsigned nSlot);

	// if bForceUpdate , then all the requested buffers are updated immediately (delayed update is not used), every frame
  void Deform( int nLodToDeform, unsigned nFlags);

	// skins the modelstate into the g_Temp (reserving it appropriately) if necessary
	// returns the pointer to the deformed vertices (just the geometry info get verts if the model isn't deformable)
	// returns NULL if can't deform
  const Vec3* DeformForShadowVolume( int nLodToDeform);

	// skins this model (into the given temporary storage, if needed)
	// if no storage is provided, then allocates its own storage (g_Temp.data())
	// the normals must already be valid, they'll be only slightly modified
	const Vec3* SelfSkin(int nLOD, Vec3*pBuffer = NULL, Vec3dA16* pNormalsA16 = NULL);

	Vec3dA16* SelfNormalSkin(int nLOD, Vec3dA16*pBuffer);

  void DeformFirst();

	void setScale (const Vec3d& vScale);

	// applies necessary offsets to the root bone (only the relative to parent matrix)
	void AddModelOffsets(CryBone* pBone);

	// the record about animation that may be kept and played back later
	struct AnimationRecord:public CryCharAnimationParams
	{
		AnimationRecord(){}
		AnimationRecord (int _nAnimId, const CryCharAnimationParams& Params, float _fSpeed):
		nAnimId(_nAnimId), fSpeed(_fSpeed),
			CryCharAnimationParams(Params){}

			void assign (int _nAnimId, const CryCharAnimationParams& Params, float _fSpeed)
			{
				*static_cast<CryCharAnimationParams*>(this) = Params;
				nAnimId = _nAnimId;
				fSpeed  = _fSpeed;
			}

			int nAnimId;
			float fSpeed;
	};
	typedef std::deque<AnimationRecord> AnimationRecordArray;

	
	// effector layer. On some layers, the effectors may not be present (NULL)
	// On some layers, there may be a default idle animation. This animation is played
	// back when there's nothing else to play.
	struct AnimationLayer
	{
		CCryModEffAnimation_AutoPtr pEffector;
		// the default idle animation that gets started upon finish of non-looped animation
		int nDefaultIdleAnimID;
		// the blending time between the previous and the default idle animation.
		float fDefaultAnimBlendTime;
		// is the animation restart on this layer enabled or not?
		bool bEnableDefaultIdleAnimRestart;
		// should the default animation restart with the same phase as the Layer0 animation (if it's not layer 0)
		bool bKeepLayer0Phase;

		// the queue of animations that must be started upon the end of current animation
		AnimationRecordArray queDelayed;

		AnimationLayer ();

		// forgets about the default idle animation
		void ForgetDefaultIdleAnimation();
	};

	// the number of layers is dynamic. These are the layers
	typedef std::vector<AnimationLayer> AnimationLayerArray;
	AnimationLayerArray m_arrAnimationLayers;

	// This is the bone hierarchy. All the bones of the hierarchy are present in this array
	typedef TFixedArray<CryBone> CryBoneArray;
	CryBoneArray m_arrBones;
	typedef TAllocator16<Matrix44> MatrixSSEAllocator;
	typedef TElementaryArray<Matrix44, MatrixSSEAllocator> MatrixSSEArray;
	MatrixSSEArray m_arrBoneGlobalMatrices;

	// animation events
	typedef std::map<int, AnimEventArray> AnimEventMap;
	AnimEventMap m_mapAnimEvents;

	// This is the global matrix that is passed to the character in ProcessSkinning() by the renderer from EF_ObjectChange()
	// It is used by the decal manager to calculate the local coordinates of the hit point
  Matrix44 m_ModelMatrix44;

  CryAABB m_BBox;

	CCryModEffIKSolver *m_pIKEffectors[4];
	Vec3 m_IKpos0[4];
	IPhysicalEntity *m_pCharPhysics;
	aux_phys_data m_auxPhys[8];
	int m_nAuxPhys;
	char m_bHasPhysics, m_bPhysicsAwake, m_bPhysicsWasAwake;
	Vec3 m_vOffset;
	float m_fScale;
	float m_fMass;
	int m_iSurfaceIdx;
	float m_fPhysBlendTime,m_fPhysBlendMaxTime,m_frPhysBlendMaxTime;

  int m_nLodLevel;

  int GetDamageTableValue (int nId);

	// based on the distance from camera, determines the best LOD
	// for this character and memorizes it in the m_nLodLevel member
  void CalculateLOD (float fDistance);

	enum {g_nMaxLods = 3};

  bool IsCharacterActive();

	// morphs the LOD 0 into the given destination (already skinned) buffer
	// pDstNormlas can be non-NULL, in which case the normals are (fakely) modified
	//void MorphWithSkin (Vec3d* pDst, Vec3dA16* pDstNormalsA16 = NULL);


	// cached bones for heat source calculations
	//CryBone *m_pBoneHead, *m_pBoneSpine2, *m_pBoneLeftArm;

	CryCharParticleManager* getParticleManager() {return &m_ParticleManager;}


	// the lights attached to bones; their properties get
	// loaded upon CGF load and reside in CryModel
	TFixedArray<CDLight> m_arrHeatSources;
	// the external lights that are added by the entity throug 3D Engine
	typedef std::vector<CBoneLightDynamicBind> DynamicBoundLightArray;
	DynamicBoundLightArray m_arrDynBoundLights;

protected:
	std::vector<ICharInstanceSink*> m_arrSinks;

	CryCharParticleManager m_ParticleManager;

	enum FlagEnum {
		// if this flag is set, it means the last time the animation was updated,
		// the bones were not updated. So if skinning is required, then bone update will 
		// need to be called
		nFlagNeedBoneUpdate = 1,
		// if this is set, then the animation was applied after the last skinning
		nFlagNeedReskinLOD  = 1<<1,
		nFlagsNeedReskinAllLODs = nFlagNeedReskinLOD | (nFlagNeedReskinLOD<<1) | (nFlagNeedReskinLOD<<2)

	};
	// misc. internal flags - combination of FlagEnum values
	unsigned m_uFlags;
#ifdef _DEBUG
	IController::PQLog m_pqLast;
	int m_nLastFrameAnimationChanged;
#endif

	// this is the count of model states created so far
	static unsigned g_nInstanceCount;
	// this is the "slot" allocated for calculating the tangents:
	// within the sequence of the frames, tangent bases are calculated each n-th frame
	unsigned m_nInstanceNumber;

	// this is the last frame id when the tangents were updated
	unsigned m_nLastTangentsUpdatedFrameId;
	unsigned m_nLastTangentsUpdatedLOD;
	unsigned m_nLastSkinBBoxUpdateFrameId;

	Vec3d m_vRuntimeScale;

	// this is NULL for objects with even parity of scale matrix (that is, non-flipped)
	// for flipped objects, it's FrontCull state shader
	IShader* m_pShaderStateCull;
	IShader* m_pShaderStateShadowCull;
#ifdef _DEBUG
	// this is true until the first non-default animation is played
	bool m_bOriginalPose;
#endif

	// the submeshes of this model state (all body parts)
	// the main submesh is the one with index [0], if any
	// this array may be empty meaning an empty character (no geometry or skeleton)
	typedef CryModelSubmesh_AutoArray SubmeshArray;
	SubmeshArray m_arrSubmeshes;

	// the Fx's
	typedef CryCharFxTrail_AutoArray CryCharFxTrailArray;
	CryCharFxTrailArray m_arrFxTrails;
};


#endif // _MODELSTATE_H

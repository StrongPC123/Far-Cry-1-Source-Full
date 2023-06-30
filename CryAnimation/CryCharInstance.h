//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:CryCharInstance.h
//  Declaration of CryCharInstance class
//
//	History:
//	August 16, 2002: Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////
#ifndef _CRY_CHAR_INSTANCE_HEADER_
#define _CRY_CHAR_INSTANCE_HEADER_

struct CryEngineDecalInfo;
struct CryParticleSpawnInfo;

#include <ICryAnimation.h>
#include "CryModel.h"
#include "CryCharBody.h"
#include "IBindable.h"
#include "CryCharInstanceRenderParams.h"

//////////////////////////////////////////////////////////////////////
struct AnimData;

class CryCharManager;

class CryCharInstanceBase: public ICryCharInstance
{
public:
	~CryCharInstanceBase()
	{
		DetachAll();
	}
	struct StatObjBind
	{
		IBindable* pObj;
		unsigned nBone;
		unsigned nFlags;
		StatObjBind(IBindable* obj, unsigned bone,unsigned flags):
		pObj(obj), nBone(bone), nFlags(flags) {assert (pObj);}
	};
	typedef std::vector<StatObjBind*> BindArray;
	BindArray m_arrBinds;

	// detaches all objects from bones; returns the nubmer of bindings deleted
	unsigned DetachAll();

	// detach all bindings to the given bone; returns the nubmer of bindings deleted
	unsigned DetachAllFromBone(unsigned nBone);

	// attaches the object to the given bone. The returned value is the handle of the binding,
	// that can be used to examine the binding and delete it
	ObjectBindingHandle AttachToBone (IBindable*pObj, unsigned nBone, unsigned nFlags = 0);

	// detaches the given binding; if it returns false, the binding handle is invalid
	// the binding becomes invalid immediately after detach
	bool Detach (ObjectBindingHandle nHandle);

	// checks if the given binding is valid
	bool IsBindingValid (ObjectBindingHandle nHandle);

	void PreloadResources ( float fDistance, float fTime, int nFlags );
};


//////////////////////////////////////////////////////////////////////
// Implementation of ICryCharInstance interface, the main interface
// in the Animation module
class CryCharInstance : public CryCharInstanceBase, protected CryCharInstanceRenderParams
{
public:
	// Releases this instance.
	virtual void Release()
	{
		delete this;
	}

	CryCharBody *GetBody()
	{
		return m_pCryCharBody;
	}

	//! Returns the model interface
	virtual ICryCharModel* GetModel();

	virtual void Draw(const SRendParams & RendParams,const Vec3& t);
	//there must be only one function
	//! Render object ( register render elements into renderer )
	virtual void Render(const struct SRendParams & rParams, const Vec3& t, int nLodLevel);

	//! marks all LODs as needed to be reskinned
	virtual void ForceReskin ();

	//! returns the leaf buffer materials in this character (as they are used in the renderer)
	virtual const list2<CMatInfo>*getLeafBufferMaterials();


	//! Interface for the renderer - returns the CDLight describing the light in this character;
	//! returns NULL if there's no light with such index
	//! ICryCharInstance owns this light. This light may be destructed without notice upon next call to 
	//! any other function but GetLight(). Nobody may every modify it.
	const CDLight* GetBoundLight (int nIndex);

	//! Draw the character shadow volumes into the stencil buffer using a set of specified 
	//! rendering parameters 
	virtual void RenderShadowVolumes(const SRendParams *rParams, int nLimitLOD = 0);

	virtual const char * GetShaderTemplateName();

	virtual bool SetShaderTemplateName(const char *TemplName, int Id, const char *ShaderName=0,IMatInfo *pCustomMaterial=0,unsigned nFlags = 0);
	//! Sets shader template for rendering
  virtual bool SetShaderTemplate(int nTemplate, const char *TemplName, const char *ShaderName, bool bOnlyRegister=false, int * pnNewTemplateId=NULL);


	virtual void SetShaderFloat(const char *Name, float fVal, const char *ShaderName=NULL);
  //! Sets color parameter
  virtual void SetColor(float fR, float fG, float fB, float fA);

	virtual const Vec3d GetCenter();
	virtual const float GetRadius();
	virtual void GetBBox(Vec3d& Mins, Vec3d& Maxs);

	virtual bool SetAnimationFrame(const char * szString, int nFrame);
	virtual ICryBone * GetBoneByName(const char * szName);

	// Attaches a static object to the bone (given the bone name) or detaches previous object from the bone.
	virtual ObjectBindingHandle AttachObjectToBone(IBindable * pWeaponModel, const char * szBoneName, bool bUseRelativeToDefPoseMatrix, unsigned nFlags);

	// returns the number of bindings; valid until the next attach/detach operation
	virtual size_t GetBindingCount();

	// fills the given array with  GetBindingCount() pointers to IBindable
	virtual void EnumBindables(IBindable** pResult);

	//! attach a light to a bone
	virtual LightHandle AttachLight (CDLight* pDLight, unsigned nBone, bool bCopyLight = false);
	//! detach the light from the bone
	virtual void DetachLight (CDLight* pDLight);

	//! Attach a light (copying the light actually) to the bone
	//! Returns the handle identifying the light. With this handle, you can either
	//! Retrieve the light information or detach it.
	virtual LightHandle AttachLight (const CDLight& rDLight, const char* szBoneName);
	//! Detaches the light by the handle retuned by AttachLight
	virtual void DetachLight (LightHandle nHandle);
	//! Returns the light by the light handle; returns NULL if no such light found
	virtual CDLight* GetLight(LightHandle nHandle);
	//! Returns the light handle if the light is attached; returns invalid handle, if this light is not attached
	//! NOTE: if your light was attached with copying, then you won't get the handle for the original light pointer
	//! because the original light might have been attached several times and have several pointers in this case
	virtual LightHandle GetLightHandle (CDLight* pLight);

	//  virtual bool BindStatObjToBone(IBindable * pStatObj, const char * szBoneName);

	////////////////////////////////////////////////////////////////////////
	// StartAnimation:																		
	// Searchs for the animation in animations list and starts palying it.				
	////////////////////////////////////////////////////////////////////////

	virtual bool StartAnimation (const char* szAnimName, const struct CryCharAnimationParams& Params);

	//! Start the specified by parameters morph target
	virtual void StartMorph (const char* szMorphTarget,const CryCharMorphParams& params);

	//! Start the morph target
	virtual void StartMorph (int nMorphTargetId, const CryCharMorphParams& params);

	//! Finds the morph with the given id and sets its relative time.
	//! Returns false if the operation can't be performed (no morph)
	virtual bool SetMorphTime (int nMorphTargetId, float fTime);

	//! Stops the animation at the specified layer. Returns true if there was some animation on that layer, and false otherwise
	virtual bool StopAnimation (int nLayer);

	virtual bool StopMorph (int nMorphTargetId);
	virtual void StopAllMorphs();
	//! freezes all currently playing morphs at the point they're at
	virtual void FreezeAllMorphs();


	//! Enables/Disables the Default Idle Animation restart.
	//! If this feature is enabled, then the last looped animation will be played back after the current (non-loop) animation is finished.
	//! Only those animations started with the flag bTreatAsDefaultIdleAnimation == true will be taken into account
	virtual void EnableLastIdleAnimationRestart (unsigned nLayer, bool bEnable = true);

	// Checks if the animation with the given name exists, returns true if it does
	virtual bool IsAnimationPresent(const char* szAnimName); 


	virtual const char * GetCurAnimation();

	//! Returns the current animation in the layer or -1 if no animation is being played 
	//! in this layer (or if there's no such layer)
	virtual int GetCurrentAnimation (unsigned nLayer);

	virtual void ResetAnimations();

	// calculates the mask ANDed with the frame id that's used to determine whether to skin the character on this frame or not.
	int GetUpdateFrequencyMask(Vec3d vPos, float fRadius);
	virtual void	Update(Vec3d vPos, float fRadius, unsigned uFlags); // processes animations and recalc bones
	//! Updates the bones and the bounding box. Should be called if animation update
	//! cycle in EntityUpdate has already passed but you need the result of new animatmions
	//! started after Update right now.
	virtual void ForceUpdate();

	virtual void	UpdatePhysics( float fScale=1.0f ); // updates model physics (if any); is separate from Update

	virtual void SetFlags(int nFlags) { m_nFlags=nFlags; }
	virtual int  GetFlags() { return m_nFlags; }

	//! Enables receiving OnStart/OnEnd of all animations from this character instance
	//! THe specified sink also receives the additional animation events specified through AddAnimationEvent interface
  virtual void AddAnimationEventSink(ICharInstanceSink * pCharInstanceSink);

	//! Counterpart to AddAnimationEventSink
	virtual void RemoveAnimationEventSink(ICharInstanceSink * pCharInstanceSink);

	//! Deletes all animation events
	virtual void RemoveAllAnimationEvents();


  //! Enables receiving OnStart/OnEnd of specified animation from this character instance
	//! The specified sink also receives the additional animation events specified through AddAnimationEvent interface for this animation
  virtual void AddAnimationEventSink(const char* szAnimName, ICharInstanceSink * pCharInstanceSink);

	//! Counterpart to the AddAnimationEventSink
  virtual void RemoveAnimationEventSink(const char* szAnimName, ICharInstanceSink * pCharInstanceSink);

	//! Adds an animation event; whenever the character plays the specified frame of the specified animation,
	//! it calls back the animation event sinkreceiving OnEvent notification of specified animation for all instances using this model
  virtual bool AddAnimationEvent(const char * szAnimName, int nFrameID, AnimSinkEventData UserData);

  //! Deletes the animation event; from now on the sink won't be receiving the animation this event
	virtual bool RemoveAnimationEvent (const char* szAnimName, int nFrameID, AnimSinkEventData UserData);

	//! Enable object animation time update. If the bUpdate flag is false, subsequent calls to Update will not animate the character
	virtual void EnableTimeUpdate (bool bUpdate);

	//! Set the current time of the given layer, in seconds
	virtual void SetLayerTime (int nLayer, float fTimeSeconds);
	virtual float GetLayerTime (int nLayer);

	// sets the given aniimation to the given layer as the default
	void SetDefaultIdleAnimation(unsigned nLayer, const char* szAnimName);

	CryCharInstance (CryCharBody * pBody);
	~CryCharInstance ();

	// FOR TEST ONLY enables/disables StartAnimation* calls; puts warning into the log if StartAnimation* is called while disabled
	void EnableStartAnimation (bool bEnable) ;

	//Executes a per-character script command
	bool ExecScriptCommand (int nCommand, void* pParams, void* pResult);
private:

	void Create(const char * fname, CryCharBody * _pCryCharBody);

	const Vec3d & GetSpitFirePos() { return m_v3pvSpitFirePosTranslated; }

//! Return the length of the given animation in seconds; 0 if no such animation found
	virtual void SetAnimationSpeed(float speed) { m_fAnimSpeedScale = speed; }
	virtual float GetAnimationSpeed() { return m_fAnimSpeedScale; }
	virtual void SetAnimationSpeed(int nLayer, float fSpeed);
	//! Set morph speed scale
	//! Finds the morph target with the given id, sets its morphing speed and returns true;
	//! if there's no such morph target currently playing, returns false
	virtual bool SetMorphSpeed (int nMorphTargetId, float fSpeed);

	virtual void BuildPhysicalEntity(IPhysicalEntity *pent,float mass,int surface_idx,float stiffness_scale,int nLod=0);
	virtual IPhysicalEntity *CreateCharacterPhysics(IPhysicalEntity *pHost, float mass,int surface_idx,float stiffness_scale, int nLod=0);
	virtual int CreateAuxilaryPhysics(IPhysicalEntity *pHost, int nLod=0);
	virtual void SynchronizeWithPhysicalEntity(IPhysicalEntity *pent,const Vec3& posMaster,const Quat& qMaster);
	virtual IPhysicalEntity *RelinquishCharacterPhysics();
	virtual void SetCharacterPhysParams(float mass,int surface_idx);
	virtual IPhysicalEntity *GetCharacterPhysics();
	virtual IPhysicalEntity *GetCharacterPhysics(const char *pRootBoneName);
	virtual IPhysicalEntity *GetCharacterPhysics(int iAuxPhys);
	virtual void DestroyCharacterPhysics(int iMode=0);

	virtual void SetLimbIKGoal(int limbid, vectorf ptgoal=vectorf(1E10f,0,0), int ik_flags=0, float addlen=0, vectorf goal_normal=vectorf(zero));
	virtual vectorf GetLimbEndPos(int limbid);

	virtual void AddImpact(int partid, vectorf point,vectorf impact);
	virtual int TranslatePartIdToDeadBody(int partid);

	virtual bool IsAnimStopped();

	virtual vectorf GetOffset();
	virtual void SetOffset(vectorf offset);

	// Spawn decal on the character
	virtual void CreateDecal(CryEngineDecalInfo& Decal);

	//! cleans up all decals in this character
	virtual void ClearDecals();


private:
	CryCharBody_AutoPtr m_pCryCharBody;
	CryModelState *m_pModelState;

protected:
	// the time of the last animation update call.
	// This is the time returned by ITimer::GetCurrTime()
	// If it's <=0, it means that update was never called
	float m_fLastAnimUpdateTime;

	// This is the scale factor that affects the animation speed of the character.
	// All the animations are played with the constant real-time speed multiplied by this factor.
	// So, 0 means still animations (stuck at some frame), 1 - normal, 2 - twice as fast, 0.5 - twice slower than normal.
	float m_fAnimSpeedScale;


	char m_sShaderTemplateName[2][64];
	unsigned m_nShaderTemplateFlags;


	// m_matTranRotMatrix is implemented in a dangerous way - this should be changed

	Matrix44 m_matTranRotMatrix;							//!< this matrix is updated during rendering and only valid right after rendering

	// the animation currently being played
	char m_sCurAnimation[32];

	Vec3d m_v3pvSpitFirePos, m_v3pvSpitFirePosTranslated;    

	//bool m_bFirstUpdate; // prevent big fFrameTime at editor startup

	CLeafBuffer * GetLeafBuffer();
	//  Matrix m_matAttachedObjectMatrix;

	// checks for possible memory corruptions in this object and its children
	void SelfValidate ()const;
public:
	virtual Vec3d GetTPVWeaponHelper(const char * szHelperName, ObjectBindingHandle hInfo);
	virtual bool GetTPVWeaponHelperMatrix(const char * szHelperName, ObjectBindingHandle nHandle, Matrix44& matOut);
	//! Returns position of specified helper ( exported into cgf file )
	//! Actually returns the given bone's position
	//! Default implementation: 000
	virtual Vec3d GetHelperPos(const char * szHelperName);
	//! Returns the matrix of the specified helper ( exported into cgf file )
	//! Actually returns the given bone's matrix
	virtual const Matrix44 * GetHelperMatrixByName(const char * szHelperName);

	virtual void SetTwiningMode(AnimTwinMode eTwinMode);
	virtual int GetDamageTableValue(int nId);

	// int nDynLightMask= RendParams.nDLightMask;
	// int nTemplID = RendParams.nShaderTemplate;
	void DrawBoundObjects(const SRendParams & rRendParams, Matrix44 &inmatTranRotMatrix, int nLOD);

	virtual bool IsCharacterActive();

	// notifies the renderer that the character will soon be rendered
	void PreloadResources ( float fDistance, float fTime, int nFlags);

	// Returns true if this character was created from the file the path refers to.
	// If this is true, then there's no need to reload the character if you need to change its model to this one.
	virtual bool IsModelFileEqual (const char* szFileName);

	void ValidateBoundObjects();

	//! Sets up particle spawning. After this funtion is called, every subsequenc frame,
	//! During the character deformation, particles will be spawned in the given characteristics.
	//! The returned handle is to be used to stop particle spawning
	//! -1 means invalid handle value (couldn't add the particle spawn task, or not implemented)
	virtual int AddParticleEmitter(ParticleParams& rParticleInfo, const CryParticleSpawnInfo& rSpawnInfo);

	//! Stops particle spawning started with StartParticleSpawn that returned the parameter
	//! Returns true if the particle spawn was stopped, or false if the handle is invalid
	//! -1 means remove all particle emitters
	virtual bool RemoveParticleEmitter (int nHandle);

	//! Sets the character scale relative to the model
	virtual void SetScale (const Vec3d& vScale);

	void GetMemoryUsage(ICrySizer* pSizer)const;
	// adds a submesh, returns handle to it which can be used to delete the submesh
	// submesh is created either visible or invisible
	// submesh creation/destruction is heavy operations, so the clients must use they rarely,
	// and set visible/invisible when they need to turn them on/off
	// But creating many submeshes is memory-consuming so the number of them must be kept low at all times
	ICryCharSubmesh* NewSubmesh (ICryCharModel* pModel, bool bVisible = false);

	// adds submesh to the specified slot; replaces submesh if there's some there
	ICryCharSubmesh* NewSubmesh (unsigned nSlot, ICryCharModel* pModel, bool bVisible = false);

	// removes submesh from the character
	void RemoveSubmesh (ICryCharSubmesh* pSubmesh);
	void RemoveSubmesh (unsigned nSlot);

	// enumeration of submeshes
	size_t NumSubmeshes();

	ICryCharSubmesh* GetSubmesh(unsigned i);

	ICryCharFxTrail* NewFxTrail (unsigned nSlot, const struct CryCharFxTrailParams&);
	void RemoveFxTrail(unsigned nSlot);

protected:
	CryCharManager* m_pManager;

	bool m_bEnableStartAnimation;

#ifdef _DEBUG
	// the last frame angles (orientation for this instance)
	Vec3d m_vLastAngles, m_vLastPos;
#endif
};


#endif
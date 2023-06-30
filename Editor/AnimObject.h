#ifndef __AnimObject_h__
#define __AnimObject_h__
#pragma once

struct IAnimTrack;

#include "ICryAnimation.h"

//////////////////////////////////////////////////////////////////////////
class CAnimObject : public ICryCharInstance
{
public:
	// Node in animated object.
	struct Node
	{
		std::string m_name; // Node name.
		//! Current node position (in object space).
		Vec3 m_pos;
		//! Current node rotation (in object space).
		Quat m_rotate;
		//! Original node scale (in object space).
		Vec3 m_scale;
		//! Node transformation matrix in world space.
		Matrix m_tm;
		//! Inverse tranform of Original node transformation.
		Matrix m_invOrigTM;
		//! True if current matrix is valid.
		bool m_bMatrixValid;
		//! parent node.
		Node* m_parent;
		//! Static object controlled by this node.
		IStatObj* m_object;

		// Animation tracks.
		IAnimTrack* m_posTrack;
		IAnimTrack* m_rotTrack;
		IAnimTrack* m_scaleTrack;
		
		Node()
		{
			m_tm.Identity();
			m_invOrigTM.Identity();
			m_parent = 0;
			m_object = 0;
			m_posTrack = 0;
			m_rotTrack = 0;
			m_scaleTrack = 0;
			m_bMatrixValid = false;
			m_pos.Set(0,0,0);
			m_rotate.Identity();
			m_scale.Set(1,1,1);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	CAnimObject();
	virtual ~CAnimObject();

	//! Creates new node.
	Node* CreateNode( const char *szNodeName );

	//! Animate object to specified time.
	void Animate( float time );
	bool Load( const char *fileName,const char *aseFile );

	virtual void Release()
	{
		delete this;
	}

	//////////////////////////////////////////////////////////////////////////
	// ICryCharInstance implementation.
	//////////////////////////////////////////////////////////////////////////
	//! Returns the interface for animations applicable to this model
	
	// Set rendering flags like (draw/notdraw) and position offset
	virtual void SetFlags(int nFlags)  {};
	virtual int  GetFlags() { return 0; };

	//! Set shader template to be used with character
	virtual bool SetShaderTemplateName(const char *TemplName, int Id, const char *ShaderName=0) { return true; };
	//! Get shader template 
	virtual const char * GetShaderTemplateName() { return ""; };
	//! Set refract coef. for refractive shader
	virtual void SetShaderFloat(const char *Name, float fVal, const char *ShaderName=NULL) {};

	//! Draw the character using a set of specified rendering parameters ( for outdoors )
	virtual void Draw(const SRendParams & RendParams);

	//! Interface for the renderer - returns the CDLight describing the light in this character;
	//! returns NULL if there's no light with such index
	//! ICryCharInstance owns this light. This light may be destructed without notice upon next call to 
	//! any other function but GetLight(). Nobody may every modify it.
	virtual const class CDLight* GetBoundLight (int nIndex);

	//! Draw the character shadow volumes into the stencil buffer using a set of specified 
	//! rendering parameters ( for indoors )
	virtual void RenderShadowVolumes(const SRendParams *rParams);

	//! Draw the character without shaders for shadow mapping
	virtual void DrawForShadow(const Vec3d & vTranslationPlus = Vec3d(0,0,0));

	//! Return dynamic bbox of object
	virtual void GetBBox(Vec3d& Mins, Vec3d& Maxs)
	{ 
		Mins = m_bbox[0];
		Maxs = m_bbox[1];
	}
	//! Return dynamic center of object
	virtual const Vec3d GetCenter()
	{
		return 0.5f*(m_bbox[1] + m_bbox[0]);
	}
	//! Return dynamic radius of object
	virtual const float GetRadius()
	{
		return (m_bbox[1] - m_bbox[0]).Length();
	}

	//! Attach object to bone (Return false if bone not found)
	ObjectBindingHandle AttachObjectToBone(IStatObj * pWeaponModel, const char * szBoneName, bool bUseRelativeToDefPoseMatrix = true)
	{
		return 0;
	}

	//! Enables/Disables the Default Idle Animation restart.
	//! If this feature is enabled, then the last looped animation will be played back after the current (non-loop) animation is finished.
	//! Only those animations started with the flag bTreatAsDefaultIdleAnimation == true will be taken into account
	void EnableLastIdleAnimationRestart (unsigned nLayer, bool bEnable = true) {}

	//! Start specified animation
	bool StartAnimation (const char * szAnimName, float fBlendTime=0.125f, int nLayerID=0, bool bTreatAsDefaultIdleAnimation = true)
	{
		return false;
	}

	//! Start specified animation. THe name StartAnimation2 suggest that this is the new version of StartAnimation, while
	//! the old StartAnimation is kept for backward compatibility. This version introduces the animation blend-in and blend-out times (different)
	bool StartAnimation2 (const char* szAnimName, float fBlendInTime = 0.125f, float fBlendOutTime = 0.125f, int nLayerID=0, bool bTreatAsDefaultIdleAnimation = true)
	{
		return false;
	}

	//! Return current animation name ( Return 0 if animations stoped )
	const char* GetCurAnimation() { return ""; };

	//! Resets all animation layers ( stops all animations )
	virtual void ResetAnimations() {};

	//! Set animations speed scale
	//! This is the scale factor that affects the animation speed of the character.
	//! All the animations are played with the constant real-time speed multiplied by this factor.
	//! So, 0 means still animations (stuck at some frame), 1 - normal, 2 - twice as fast, 0.5 - twice slower than normal.
	virtual void SetAnimationSpeed(float speed)
	{
		m_animSpeed = speed;
	}

	//! Enable object animation time update. If the bUpdate flag is false, subsequent calls to Update will not animate the character
	virtual void EnableTimeUpdate(bool bUpdate)
	{
		m_bNoUpdate = bUpdate;
	}

	//! Set the current time of the given layer, in seconds
	virtual void SetLayerTime (int nLayer, float fTimeSeconds) {};

	//! Step animation (call this function every frame to animate character)
	virtual void Update( Vec3d vPos = Vec3d(0,0,0), float fRadius=0, unsigned uFlags = 0);

	//! Synchronizes state with character physical animation; should be called after all updates (such as animation, game bones updates, etc.)
	virtual void UpdatePhysics() {};

	//! IK (Used by physics engine)
	virtual void BuildPhysicalEntity(IPhysicalEntity *pent,float mass,int surface_idx,float stiffness_scale=1.0f,int nLod=0)
	{};
	virtual IPhysicalEntity *CreateCharacterPhysics(IPhysicalEntity *pHost, float mass,int surface_idx,float stiffness_scale, int nLod=0)
	{
		return 0;
	}
	virtual int CreateAuxilaryPhysics(IPhysicalEntity *pHost, int nLod=0)
	{
		return false;
	};
	virtual IPhysicalEntity *GetCharacterPhysics(const char *pRootBoneName=0) { return 0; };
	virtual void SynchronizeWithPhysicalEntity(IPhysicalEntity *pent) {};
	virtual IPhysicalEntity *RelinquishCharacterPhysics() { return 0; };
	virtual void SetLimbIKGoal(int limbid, vectorf ptgoal=vectorf(1E10f,0,0), int ik_flags=0, float addlen=0, vectorf goal_normal=vectorf(zero)) {};
	virtual vectorf GetLimbEndPos(int limbid) { return vectorf(0,0,0); };
	virtual void AddImpact(int partid, vectorf point,vectorf impact) {};
	virtual int TranslatePartIdToDeadBody(int partid) { return 0; };
	virtual vectorf GetOffset() { return vectorf(0,0,0); };
	virtual void SetOffset(vectorf offset) {};

	//! Direct access to the specified  bone
	virtual ICryBone * GetBoneByName(const char * szName) { return 0; };

	//! Pose character bones
	virtual bool SetAnimationFrame(const char * szString, int nFrame) { return false; };

	virtual void AddAnimationEventSink(ICharInstanceSink * pCharInstanceSink) {};

	//! Counterpart to AddAnimationEventSink
	virtual void RemoveAnimationEventSink(ICharInstanceSink * pCharInstanceSink) {};

  //! Enables receiving OnStart/OnEnd of specified animation from this character instance
	//! The specified sink also receives the additional animation events specified through AddAnimationEvent interface for this animation
	virtual void AddAnimationEventSink(const char* szAnimName, ICharInstanceSink * pCharInstanceSink) {};

	//! Counterpart to the AddAnimationEventSink
	virtual void RemoveAnimationEventSink(const char* szAnimName, ICharInstanceSink * pCharInstanceSink) {};

	//! Adds an animation event; whenever the character plays the specified frame of the specified animation,
	//! it calls back the animation event sinkreceiving OnEvent notification of specified animation for all instances using this model
	virtual bool AddAnimationEvent(const char * szAnimName, int nFrameID, int nUserData) { return true; };

  //! Deletes the animation event; from now on the sink won't be receiving the animation this event
	virtual bool RemoveAnimationEvent (const char* szAnimName, int nFrameID, int nUserData) { return true; };

	//! Callback interface
	//! Enables receiving OnStart/OnEnd of specified animation from this character instance
	virtual void SetAnimationSinkForInstance(const char * szAnimName, ICharInstanceSink * pCharInstanceSink) {};

	//! Returns the model interface
	virtual ICryCharModel* GetModel()
	{
		return &m_characterModel;
	}

	//! Start/stop to spawn particles on character body
	virtual void EnableParticleEmitter(struct ParticleParams & SpawnParticleParams, bool bEnable, bool bOnlyUpLookingFaces=false) {};

	//! Return position of helper of the static object which is attached to animated object
	virtual Vec3d GetTPVWeaponHelper(const char * szHelperName, ObjectBindingHandle pInfo ) { return Vec3d(0,0,0); };

	//! Set the twining type. If replace - animations of second layer will overwrite first, otherwise it will sum
	virtual void SetTwiningMode(AnimTwinMode eTwinMode ) {};

	//! Return damage zone id for specified bone id
	virtual int GetDamageTableValue (int nId) { return 0; };

	//! Renderer calls this function to allow update the video vertex buffers right before the rendering
	virtual void ProcessSkinning(float modelmatrix[], int nTemplate) {};

	//! returns true if the character is playing any animation now (so bbox is changing)
	virtual bool IsCharacterActive() { return true; };

	void		SetAngles( const Vec3d& angles ) { m_angles = angles; }
	Vec3d&	GetAngles() { return m_angles; }

	//! Spawn decal on the walls, static objects, terrain and entities
	virtual void CreateDecal(CryEngineDecalInfo& Decal) {};

	virtual bool IsModelFileEqual (const char* szFileName);
	//////////////////////////////////////////////////////////////////////////

private:
	void ReleaseNodes();
	Matrix& GetNodeMatrix( Node *node );

	//! Name of cgf.
	std::string m_fileName;
	// Animated nodes.
	std::vector<Node*> m_nodes;

	I3DEngine *m_3dEngine;

	//! Animation speed.
	float m_animSpeed;

	bool m_bNoUpdate;

	// Bounding box.
	Vec3d m_bbox[2];

	Vec3d m_angles;

	struct SCharModel : public ICryCharModel
	{
		virtual ICryAnimationSet* GetAnimationSet () { return 0; };
		virtual const char * GetBoneName(int nId) const { return ""; };
		virtual int NumBones() const { return 0; };
		virtual float GetScale() const { return 0; };
	};
	SCharModel m_characterModel;
};

#endif // __AnimObject_h__

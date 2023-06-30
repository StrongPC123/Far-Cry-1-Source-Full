#ifndef __AnimObject_h__
#define __AnimObject_h__
#pragma once

struct IAnimTrack;
class IPhysicalEntity;

#include "ICryAnimation.h"
#include "Controller.h"
#include "CryCharInstance.h"

//////////////////////////////////////////////////////////////////////////
class CAnimObject : public CryCharInstanceBase
{
public:
//	CAnimObject *obj;


	//! Node in animated object.
	//! Implements Bone interface.
	struct Node : public ICryBone
	{
		string m_name; // Node name.
		// Node id, (Assigned at node creation time).
		int m_id;
		//! Current node position (in object space).
		Vec3 m_pos;
		//! Current node rotation (in object space).
		CryQuat m_rotate;
		//! Original node scale (in object space).
		Vec3 m_scale;
		//! Node transformation matrix in world space.
		Matrix44 m_tm;
		//! True if current matrix is valid.
		bool m_bMatrixValid;
		//! parent node.
		Node* m_parent;
		//! Static object controlled by this node.
		IStatObj* m_object;
		//! True if have physics.
		bool bPhysics;
		
		Node()
		{
			m_id = 0;
			m_tm.SetIdentity();
			m_parent = 0;
			m_object = 0;
			m_bMatrixValid = false;
			m_pos(0,0,0);
			m_rotate.SetIdentity();
			m_scale(1,1,1);
			bPhysics = false;
		}
		void GetSize (ICrySizer* pSizer)const;

		//////////////////////////////////////////////////////////////////////////
		// ICryBone interface implementation.
		//////////////////////////////////////////////////////////////////////////
		virtual void DoNotCalculateBoneRelativeMatrix(bool bDoNotCalculate) {};
		virtual void FixBoneMatrix (const Matrix44& mtxBone) {};
		virtual void FixBoneOriginInWorld (const Vec3& vCharPos, const Vec3& vCharAngles, const Vec3& vTargetOrigin) {};
		virtual void SetBoneOriginInWorld (const Vec3& vCharPos, const Vec3& vCharAngles, const Vec3& vTargetOrigin) {};
		virtual void SetPlusRotation(float x, float y, float z) {};
		virtual void SetPlusRotation(const CryQuat& qRotation) {};
		virtual void ResetPlusRotation() {};
		virtual Vec3 GetBonePosition()
		{
			return m_tm.GetTranslationOLD();
		};
		virtual Vec3 GetBoneAxis(char cAxis)
		{
			switch (cAxis)
			{
				//[Timur] Not sure if its correct axises.
			case 0: return Vec3(m_tm(0,0),m_tm(1,0),m_tm(2,0));
			case 1: return Vec3(m_tm(0,1),m_tm(1,1),m_tm(2,1));
			//case 2: return Vec3(m_tm(0,2),m_tm(1,2),m_tm(2,2));
			}
			return Vec3(m_tm(0,2),m_tm(1,2),m_tm(2,2));
		};
		virtual CryQuat GetParentWQuat () { 
			if (m_parent)
				m_parent->m_rotate;
			return Quat(1,0,0,0);
		}; //[Timur] Wrong implementation.
		virtual ICryBone* GetParent() { return m_parent; };
		virtual const Matrix44& GetRelativeMatrix() { return m_tm; };
		virtual const Matrix44& GetAbsoluteMatrix() { return m_tm; }; //[Timur] Wrong implementation.
		//////////////////////////////////////////////////////////////////////////
	};

	// Node animation.
	struct NodeAnim
	{
		//! Current node position (in object space).
		Vec3 m_pos;
		//! Current node rotation (in object space).
		CryQuat m_rotate;
		//! Original node scale (in object space).
		Vec3 m_scale;
		IController* m_posTrack;
		IController* m_rotTrack;
		IController* m_scaleTrack;

		NodeAnim()
		{
			m_pos(0,0,0);
			m_rotate.SetIdentity();
			m_scale(1,1,1);
			m_posTrack = 0;
			m_rotTrack = 0;
			m_scaleTrack = 0;
		}

		// deletes the controllers belonging ot this node
		void clearControllers()
		{
			if (m_posTrack)
			{
				delete m_posTrack;
				m_posTrack = NULL;
			}
			if (m_rotTrack)
			{
				delete m_rotTrack;
				m_rotTrack = NULL;
			}

			if (m_scaleTrack)
			{
				delete m_scaleTrack;
				m_scaleTrack = NULL;
			}
		}
	};
	// Single animation description.
	struct Animation
	{
		string name;
		bool loop;
		bool haveLoopingController;
		float startTime;		// Start time in seconds.
		float endTime;			// End time in seconds.
		float secsPerFrame; // seconds per frame.
		typedef std::vector<NodeAnim> NodeAnimArray;
		NodeAnimArray nodeAnims;
		
		// Ctor
		Animation()
		{
			haveLoopingController = false;
			loop = false;
			startTime = endTime = 0;
			secsPerFrame = 1;
		}

		~Animation()
		{
			for (NodeAnimArray::iterator it = nodeAnims.begin(); it != nodeAnims.end(); ++it)
			{
				it->clearControllers();
			}
		}
	};

	//////////////////////////////////////////////////////////////////////////
	CAnimObject();
	virtual ~CAnimObject();

	//! Set name of file for this anmated object.
	void SetFileName( const char *szFileName );
	const char* GetFileName() { return m_fileName.c_str(); };

	//! Creates new node.
	Node* CreateNode( const char *szNodeName );

	//! Animate object to specified time.
	void Animate( float time );

	//! Get matrix of node.
	Matrix44& GetNodeMatrix( Node *node );

	//! Get node animation for current animation.
	
	NodeAnim* GetNodeAnim( Node *node );
	//! Set animation of specified node.
	void AddAnimation( Animation* anim );
	//! Remove animation.
	void RemoveAnimation( Animation* anim );
	//! Sets current animation. (Can be NULL).
	void SetCurrent( Animation *anim );

	//! Find animation by name.
	Animation* FindAnimation( const char *szAnimationName );
	//! Find Node by name, return its index.
	//! @return -1 if not found, or node index.
	int FindNodeByName( const char *szNodeName );
	//! Get name of node by node id.
	const char* GetNodeName( int nodeId );
	//! Get number of nodes.
	int GetNumNodes() const { return (int)m_nodes.size(); }

	void RecalcBBox();

	//////////////////////////////////////////////////////////////////////////
	// ICryCharInstance implementation.
	//////////////////////////////////////////////////////////////////////////
	//! Returns the interface for animations applicable to this model
	virtual void AddRef() { m_nRefCount++; }
	virtual void Release()
	{
		if (--m_nRefCount <= 0)
			delete this;
	}
	
	// Set rendering flags like (draw/notdraw) and position offset
	virtual void SetFlags(int nFlags)  { m_nFlags = nFlags; };
	virtual int  GetFlags() { return m_nFlags; };

	//! Set shader template to be used with character
	virtual bool SetShaderTemplateName(const char *TemplName, int Id, const char *ShaderName=0,IMatInfo *pCustomMaterial=0,unsigned nFlags = 0) { return true; };
	//! Sets shader template for rendering
	virtual bool SetShaderTemplate(int nTemplate, const char *TemplName, const char *ShaderName, bool bOnlyRegister, int * pnNewTemplateId=NULL)
	{
		// This gets called when character are attached recursively to each other's bones
		// What to do?
		return true;
	}
	//! Get shader template 
	virtual const char * GetShaderTemplateName() { return ""; };
	//! Set refract coef. for refractive shader
	virtual void SetShaderFloat(const char *Name, float fVal, const char *ShaderName=NULL) {};
  //! Sets color parameter
  virtual void SetColor(float fR, float fG, float fB, float fA) {};

	//! Draw the character using a set of specified rendering parameters ( for outdoors )
	virtual void Draw(const SRendParams & RendParams,const Vec3& t);
	//! Render object ( register render elements into renderer )
	void Render(const struct SRendParams & rParams,const Vec3& t, int nLodLevel);

	//! Interface for the renderer - returns the CDLight describing the light in this character;
	//! returns NULL if there's no light with such index
	//! ICryCharInstance owns this light. This light may be destructed without notice upon next call to 
	//! any other function but GetLight(). Nobody may every modify it.
	virtual const class CDLight* GetBoundLight (int nIndex);

	//! Draw the character shadow volumes into the stencil buffer using a set of specified 
	//! rendering parameters ( for indoors )
	virtual void RenderShadowVolumes(const SRendParams *rParams, int nLimitLOD);

	//! Draw the character without shaders for shadow mapping
	//virtual void DrawForShadow(const Vec3 & vTranslationPlus = Vec3(0,0,0));

	//! Return dynamic bbox of object
	virtual void GetBBox(Vec3& Mins, Vec3& Maxs)
	{ 
		if (!m_bboxValid)
			RecalcBBox();
		Mins = m_bbox[0];
		Maxs = m_bbox[1];
	}
	//! Return dynamic center of object
	virtual const Vec3 GetCenter()
	{
		return 0.5f*(m_bbox[1] + m_bbox[0]);
	}
	//! Return dynamic radius of object
	virtual const float GetRadius()
	{
		return (m_bbox[1] - m_bbox[0]).Length();
	}

	//! Enables/Disables the Default Idle Animation restart.
	//! If this feature is enabled, then the last looped animation will be played back after the current (non-loop) animation is finished.
	//! Only those animations started with the flag bTreatAsDefaultIdleAnimation == true will be taken into account
	void EnableLastIdleAnimationRestart (unsigned nLayer, bool bEnable = true) {}

	//! Start specified animation
	bool StartAnimation (const char* szAnimName, const struct CryCharAnimationParams& Params)
	{
		SetCurrent( FindAnimation(szAnimName) );
		m_time = 0;
		return false;
	}

	bool StopAnimation (int nLayer)
	{
		if (!m_currAnimation)
			return false;
		SetCurrent(0);
		return true;
	}


	//! Return current animation name ( Return 0 if animations stoped )
	const char* GetCurAnimation()
	{
		if (m_currAnimation)
			return m_currAnimation->name.c_str();
		return 0;
	};

	int GetCurrentAnimation(unsigned nLayer)
	{
		// return fake animation id
		return m_currAnimation?0:-1;
	}

	//! Resets all animation layers ( stops all animations )
	virtual void ResetAnimations();

	//! Set animations speed scale
	//! This is the scale factor that affects the animation speed of the character.
	//! All the animations are played with the constant real-time speed multiplied by this factor.
	//! So, 0 means still animations (stuck at some frame), 1 - normal, 2 - twice as fast, 0.5 - twice slower than normal.
	virtual void SetAnimationSpeed(float speed)
	{
		m_animSpeed = speed;
	}

	virtual float GetAnimationSpeed()
	{
		return m_animSpeed;
	}
	virtual void SetAnimationSpeed(int nLayer, float fSpeed) 
	{
		m_animSpeed = fSpeed;
	}



	//! Enable object animation time update. If the bUpdate flag is false, subsequent calls to Update will not animate the character
	virtual void EnableTimeUpdate(bool bUpdate)
	{
		m_bNoTimeUpdate = !bUpdate;
	}

	//! Set the current time of the given layer, in seconds
	virtual void SetLayerTime(int nLayer, float fTimeSeconds);
	virtual float GetLayerTime(int nLayer) { return m_time; };

	//! Step animation (call this function every frame to animate character)
	virtual void Update( Vec3 vPos = Vec3(0,0,0), float fRadius=0, unsigned uFlags = 0);

	//! Synchronizes state with character physical animation; should be called after all updates (such as animation, game bones updates, etc.)
	virtual void UpdatePhysics( float fScale=1.0f );

	//! IK (Used by physics engine)
	virtual void BuildPhysicalEntity( IPhysicalEntity *pent,float mass,int surface_idx,float stiffness_scale=1.0f,int nLod=0 );
	virtual IPhysicalEntity* CreateCharacterPhysics(IPhysicalEntity *pHost, float mass,int surface_idx,float stiffness_scale, int nLod=0);
	virtual int CreateAuxilaryPhysics(IPhysicalEntity *pHost, int nLod=0);
	virtual IPhysicalEntity *GetCharacterPhysics() { return m_physic; }
	virtual IPhysicalEntity *GetCharacterPhysics(const char *pRootBoneName) { return m_physic; }
	virtual IPhysicalEntity *GetCharacterPhysics(int iAuxPhys) { return m_physic; }
	virtual void SynchronizeWithPhysicalEntity(IPhysicalEntity *pent,const Vec3& posMaster,const Quat& qMaster) {};
	virtual void DestroyCharacterPhysics(int iMode=0) {}
	virtual void SetCharacterPhysParams(float mass,int surface_idx) {}
	virtual IPhysicalEntity *RelinquishCharacterPhysics() { return 0; };
	virtual void SetLimbIKGoal(int limbid, vectorf ptgoal=vectorf(1E10f,0,0), int ik_flags=0, float addlen=0, vectorf goal_normal=vectorf(zero)) {};
	virtual vectorf GetLimbEndPos(int limbid) { return vectorf(0,0,0); };
	virtual void AddImpact(int partid, vectorf point,vectorf impact);
	virtual int TranslatePartIdToDeadBody(int partid) { return 0; };
	virtual vectorf GetOffset() { return vectorf(0,0,0); };
	virtual void SetOffset(vectorf offset) {};

	//! Direct access to the specified  bone
	virtual ICryBone * GetBoneByName(const char * szName);

	// Attaches a static object to the bone (given the bone name) or detaches previous object from the bone.
	virtual ObjectBindingHandle AttachObjectToBone(IBindable * pWeaponModel, const char * bone_name, bool bUseRelativeToDefPoseMatrix , unsigned nFlags = 0);

	//! Pose character bones
	virtual bool SetAnimationFrame(const char * szString, int nFrame)
	{
		if (m_currAnimation)
		{
			m_time = m_currAnimation->secsPerFrame * nFrame;
		}
		return false;
	};

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
	virtual bool AddAnimationEvent(const char * szAnimName, int nFrameID, AnimSinkEventData UserData) { return true; };

  //! Deletes the animation event; from now on the sink won't be receiving the animation this event
	virtual bool RemoveAnimationEvent (const char* szAnimName, int nFrameID, AnimSinkEventData UserData) { return true; };

	//! Callback interface
	//! Enables receiving OnStart/OnEnd of specified animation from this character instance
	virtual void SetAnimationSinkForInstance(const char * szAnimName, ICharInstanceSink * pCharInstanceSink) {};

	//! Returns the model interface
	virtual ICryCharModel* GetModel()
	{
		return &m_characterModel;
	}

	//! Return position of helper of the static object which is attached to animated object
	virtual Vec3 GetTPVWeaponHelper(const char * szHelperName, ObjectBindingHandle pInfo ) { return Vec3(0,0,0); };

	//! Set the twining type. If replace - animations of second layer will overwrite first, otherwise it will sum
	virtual void SetTwiningMode(AnimTwinMode eTwinMode ) {};

	//! Return damage zone id for specified bone id
	virtual int GetDamageTableValue (int nId) { return 0; };

	//! Renderer calls this function to allow update the video vertex buffers right before the rendering
	virtual void ProcessSkinning(const Vec3& t,const Matrix44& mtxModel, int nTemplate, int nLod=-1, bool bForceUpdate=false) {};

	//! returns true if the character is playing any animation now (so bbox is changing)
	virtual bool IsCharacterActive() { return m_currAnimation != 0; };

	void		SetAngles( const Vec3& angles ) { m_angles = angles; }
	Vec3&	GetAngles() { return m_angles; }

	//! Spawn decal on the walls, static objects, terrain and entities
	virtual void CreateDecal(CryEngineDecalInfo& Decal);

	virtual bool IsModelFileEqual (const char* szFileName);
	//////////////////////////////////////////////////////////////////////////

	// Pushes the underlying tree of objects into the given Sizer object for statistics gathering
	void GetMemoryUsage(class ICrySizer* pSizer)const;



private:


	void ReleaseNodes();
	void ReleaseAnims();

	// Reference counter for this object.
	int m_nRefCount;

	//! Name of cgf.
	string m_fileName;

	// Animated nodes.
	std::vector<Node*> m_nodes;
	std::vector<Animation*> m_animations;
	Animation* m_currAnimation;

	// Some general character flags.
	int m_nFlags;

	//! Animation speed.
	float m_animSpeed;

	//! Last time at which this character was animated.
	float m_lastAnimTime;

	//! Current animation time.
	float m_time;
	//! Time of animation itself.
	float m_fAnimTime;
	//! True if no time step.
	bool m_bNoTimeUpdate;
	//! True if this animation was already played once.
	bool m_bAllNodesValid;
	//! True if bbox is valid.
	bool m_bboxValid;

	// Bounding box.
	Vec3 m_bbox[2];

	Vec3 m_angles;

	// Physics.
	IPhysicalEntity *m_physic;
	float m_lastScale;


	//////////////////////////////////////////////////////////////////////////
	struct AnimationSet : public ICryAnimationSet
	{
		//! Returns the number of animations in this set
		virtual int Count() { return (int)obj->m_animations.size(); };

		//! Returns the index of the animation in the set, -1 if there's no such animation
		virtual int Find (const char* szAnimationName)
		{
			for (unsigned int i = 0; i < obj->m_animations.size(); i++)
			{
				if (stricmp(obj->m_animations[i]->name.c_str(),szAnimationName)==0)
				{
					return i;
				}
			}
			return -1;
		}

		//! Returns the given animation length, in seconds
		virtual float GetLength (int nAnimationId)
		{
			if (nAnimationId < 0)
				return 0;
			return obj->m_animations[nAnimationId]->endTime - obj->m_animations[nAnimationId]->startTime;
		}

		//! Returns the given animation length, in seconds
		virtual float GetLength (const char* szAnimationName)
		{
			return GetLength(Find(szAnimationName));
		}

		//! Returns the given animation name
		virtual const char* GetName (int nAnimationId)
		{
			if (nAnimationId < 0)
				return "";
			return obj->m_animations[nAnimationId]->name.c_str();
		}

		//! Retrieves the animation loop flag
		virtual bool IsLoop (int nAnimationId)
		{
			if (nAnimationId < 0)
				return false;
			return obj->m_animations[nAnimationId]->loop;
		}

		//! Retrieves the animation loop flag
		virtual bool IsLoop (const char* szAnimationName)
		{
			return IsLoop(Find(szAnimationName));
		}
		virtual void SetLoop (int nAnimationId, bool bIsLooped)
		{
			if (nAnimationId < 0)
				return;
			obj->m_animations[nAnimationId]->loop	= bIsLooped;
		};

		CAnimObject *obj;
	};


	friend struct AnimationSet;

	//////////////////////////////////////////////////////////////////////////
	struct SCharModel : public ICryCharModel
	{
		virtual ICryAnimationSet* GetAnimationSet () { return &animSet; };
		virtual float GetScale() const { return 1; };

		//! Return name of bone from bone table, return zero if the nId is out of range (the game gets this id from physics)
		virtual const char * GetBoneName(int nId) const { return pAnimObject->GetNodeName( nId ); }

		//! Returns the index of the bone by its name or -1 if no such bone exists; this is Case-Sensitive
		virtual int GetBoneByName (const char* szName) { return pAnimObject->FindNodeByName( szName ); }

		//! Returns the number of bones; all bone ids are in the range from 0 to this number exclusive; 0th bone is the root
		virtual int NumBones() const { return pAnimObject->GetNumNodes(); };

		//! returns the file name of the character model
		virtual const char* GetFileName() { return pAnimObject->GetFileName(); }

		AnimationSet animSet;
		CAnimObject *pAnimObject;
	};
	SCharModel m_characterModel;


	// sets the given aniimation to the given layer as the default
	virtual void SetDefaultIdleAnimation(unsigned nLayer, const char* szAnimName) {}

	// preload textures
	virtual void PreloadResources ( float fDistance, float fTime, int nFlags);

	//////////////////////////////////////////////////////////////////////////
	// Darw bound objects.
	void DrawBoundObjects(const SRendParams & rRendParams, Matrix44 &inmatTranRotMatrix, int nLOD);
};

#endif // __AnimObject_h__

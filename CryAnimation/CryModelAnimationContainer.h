#ifndef _CRY_MODEL_ANIMTATION_CONTAINER_HDR_
#define _CRY_MODEL_ANIMTATION_CONTAINER_HDR_

#include "CryAnimationInfo.h"
#include "CryBoneInfo.h"

template<class T>
struct LessString
{
	bool operator () (const char* left, const char* right)const
	{
		return strcmp(left, right) < 0;
	}
	bool operator () (const string& left, const string& right)const
	{
		return left < right;
	}
};

template<class Key, class Traits = LessString<Key> >
class BoneNameHashCompare
{
	Traits comp;
public:
	enum { bucket_size = 8 };
	enum { min_buckets = 16 };
	BoneNameHashCompare(){}
	BoneNameHashCompare(Traits pred): comp(pred) {}

	inline size_t operator( )( const Key& key ) const
	{
		int h = 5381;

		for(int i = 0, k; k = key[i]; i++)
			h = ((h<<5)+h)^k;    // bernstein k=33 xor

		return h;
	}

	inline bool operator( )( 
		const Key& _Key1,
		const Key& _Key2
		) const
	{
		return comp (_Key1, _Key2);
	}


};

class CryGeomMorphTarget;
class CrySkinMorph;

//////////////////////////////////////////////////////////////////////////
// Implementaiton of ICryAnimationSet, holding the information about animations
// and bones for a single model. Animations also include the subclass of morph targets
class CryModelAnimationContainer:	public ICryAnimationSet
{
public:
	CryModelAnimationContainer (class CControllerManager * pControllerManager);
	~CryModelAnimationContainer();

	// gets called when the given animation (by global id) is unloaded.
	// the animation controls need to be unbound to free up the memory
	void OnAnimationGlobalUnload(int nGlobalAnimId);

	// gets called when the given animation (by global id) is loaded
	// the animation controls need to be bound at this point
	void OnAnimationGlobalLoad (int nGlobalAnimId);

	// for internal use only
	//DECLARE_ARRAY_GETTER_METHODS (AnimData, AnimationInfo, Animations, m_arrAnimations);
	size_t numAnimations() const {return m_arrAnimations.size();}
	// tries to load the animation info if isn't present in memory; returns NULL if can't load
	const AnimData* getAnimationInfo(unsigned i);

	//! Unloads animation from memory
	//! The client must take into account that the animation can be shared and, if unloaded, the other
	//! character models (animation sets) will have to load it back to use.
	virtual void UnloadAnimation (int nAnimId);

	//! Loads the animation data in memory. fWhenRequired is the timeout in seconds from current moment when
	//! the animation data will actually be required
	virtual void StartLoadAnimation (int nAnimId, float fWhenRequired);

	// returns the number of animations that aren't shared
	unsigned numUniqueAnimations();

	// Returns the index of the animation in the set, -1 if there's no such animation
	virtual int Find (const char* szAnimationName);

	//! Returns the index of the morph target in the set, -1 if there's no such morph target
	virtual int FindMorphTarget (const char* szMorphTarget);

	// returns the index of the animation
	int findAnimation (const char*szAnimationName);

	// returns the index of the morph target, in the indexation of the array of morph targets
	int findMorphTarget (const char* szMorphTargetName);

	// Returns the number of animations in this set
	virtual int Count();

	//! Returns the number of morph targets in the set
	virtual int CountMorphTargets();

	// Returns the given animation length, in seconds
	virtual float GetLength (int nAnimationId);

	//! Returns the given animation's start, in seconds; 0 if the id is invalid
	virtual float GetStart (int nAnimationId);

	// Returns the given animation name
	virtual const char* GetName (int nAnimationId);
	//! Returns the name of the morph target
	const char* GetNameMorphTarget (int nMorphTargetId);

	// Retrieves the animation loop flag
	virtual bool IsLoop (int nAnimationId);


	//! Modifies the animation loop flag
	virtual void SetLoop (int nAnimationId, bool bIsLooped);

	//! Enables receiving OnEvent notification of specified animation for all instances using this model
  //virtual bool SetAnimationEvent(const char * szAnimName, int nFrameID, int nUserData, ICharInstanceSink * pCharInstanceSink);

	DECLARE_ARRAY_GETTER_METHODS(CryBoneInfo, BoneInfo, BoneInfos, m_arrBones);

	CryBoneInfo& getRootBoneInfo() {return getBoneInfo(0);}

	// updates the physics info of the given lod from the given bone animation descriptor
	void UpdateRootBonePhysics (const BONEANIM_CHUNK_DESC* pChunk, unsigned nChunkSize, int nLodLevel);

	// loads the root bone (and the hierarchy) and returns true if loaded successfully
	bool LoadBones (const BONEANIM_CHUNK_DESC* pChunk, unsigned nChunkSize, const TFixedArray<const char*>& arrBoneNames);

	// finds the bone by its name; returns the index of the bone
	int findBone (const char* szName) const;

	// FOR INTERNAL USE ONLY (Inside CryAnimation system)

	// returns the reference to the given animation , or the default animation, if the animation
	// id is out of range
	const AnimData &getAnimation (int nAnimationId) const;
	// records statistics about the given animation (the id is the same as for getAnimation()):
	// should be called upon ticking the animation
	void OnAnimationTick(int nAnimationId);
	void OnAnimationApply(int nAnimationId);
	// records statistics about the given animation (the id is the same as for getAnimation()):
	// should be called upon starting the animation
	void OnAnimationStart(int nAnimationId);

	// returns the reference to the given morph target, or the default morph target if the
	// morph target id is out of range
	const CryGeomMorphTarget& getMorphTarget (int nMorphTargetId) const;
	unsigned numMorphTargets() const;

	float getTicksPerFrame()const {return m_fTicksPerFrame;}
	float getSecondsPerTick () const {return m_fSecsPerTick;}
	float getSecondsPerFrame () const {return m_fTicksPerFrame*m_fSecsPerTick;}
	float getTicksPerSecond () const {return float(1.0 / m_fSecsPerTick);}

	// cleans up the unused memory in the arrays of controllers in the bones
	void shrinkControllerArrays();
	// prepares to load the specified number of CAFs by reserving the space for the controller pointers
	void prepareLoadCAFs (unsigned nReserveAnimations);

	bool isDummyAnimation (const char* szAnimName) {return m_setDummyAnimations.find (szAnimName) != m_setDummyAnimations.end();}
protected:
	void selfValidate();

	// adds an animation record to the animation set
	void RegisterAnimation(const char * szFileName, int nGlobalAnimId, const char* szAnimName);
	void RegisterDummyAnimation (const char* szAnimName) {m_setDummyAnimations.insert (szAnimName);}

	// when the animinfo is given, it's used to set the values of the global animation as if the animation has already been loaded once -
	// so that the next time the anim info is available and there's no need to load the animation synchronously
	// Returns the global anim id of the file, or -1 if error
	int LoadCAF (const char * szFileName, float fScale, int nAnimID, const char * szAnimName, unsigned nGlobalAnimFlags);

	//! Performs post-initialization. This step is requred to initialize the pPhysGeom of the bones
	//! After the bone has been loaded but before it is first used. When the bone is first loaded, pPhysGeom
	//! is set to the value equal to the chunk id in the file where the physical geometry (BoneMesh) chunk is kept.
	//! After those chunks are loaded, and chunk ids are mapped to the registered physical geometry objects,
	//! call this function to replace pPhysGeom chunk ids with the actual physical geometry object pointers.
	//!	NOTE:
	//!	The entries of the map that were used are deleted
	typedef CryBoneInfo::ChunkIdToPhysGeomMap ChunkIdToPhysGeomMap;
	bool PostInitBonePhysGeom (ChunkIdToPhysGeomMap& mapChunkIdToPhysGeom, int nLodLevel);

	//! Deserializes the bones from the CCF chunk using serialization function from CryBoneInfo
	//! THe serialized data follows the given header; the total size (including the header) is passed
	bool loadCCGBones (const struct CCFBoneDescArrayHeader*pHeader, unsigned nSize);
	// scales the skeleton (its initial pose)
	void scaleBones (float fScale);

	void onBonesChanged();
	void onBonePhysicsChanged();
protected:
	// the dummy animations: when they are requested, nothing should be played
	std::set<string> m_setDummyAnimations;
  // Animations List
	typedef std::vector<AnimData> AnimationArray;
  AnimationArray m_arrAnimations;
	struct LocalAnimId{
		int nAnimId;
		explicit LocalAnimId(int n):nAnimId(n){}
	};
	// the index vector: the animations in order of nGlobalAnimId
	std::vector<LocalAnimId> m_arrAnimByGlobalId;

	// used to sort the m_arrAnimByGlobalId
	struct AnimationGlobIdPred
	{
		const std::vector<AnimData>& m_arrAnims;
		AnimationGlobIdPred(const std::vector<AnimData>& arrAnims):
			m_arrAnims(arrAnims)
		{
		}

		bool operator () (LocalAnimId left, LocalAnimId right)const 
		{
			return m_arrAnims[left.nAnimId].nGlobalAnimId < m_arrAnims[right.nAnimId].nGlobalAnimId;
		}
		bool operator () (int left, LocalAnimId right)const 
		{
			return left < m_arrAnims[right.nAnimId].nGlobalAnimId;
		}
		bool operator () (LocalAnimId left, int right)const 
		{
			return m_arrAnims[left.nAnimId].nGlobalAnimId < right;
		}
	};

	// the index vector: the animations in order of their local names
	std::vector<int> m_arrAnimByLocalName;

	struct AnimationNamePred
	{
		const std::vector<AnimData>& m_arrAnims;
		AnimationNamePred(const std::vector<AnimData>& arrAnims):
		m_arrAnims(arrAnims)
		{
		}

		bool operator () (int left, int right)const 
		{
			return stricmp(m_arrAnims[left].strName.c_str(),m_arrAnims[right].strName.c_str()) < 0;
		}
		bool operator () (const char* left, int right)const 
		{
			return stricmp(left,m_arrAnims[right].strName.c_str()) < 0;
		}
		bool operator () (int left, const char* right)const 
		{
			return stricmp(m_arrAnims[left].strName.c_str(),right) < 0;
		}
	};

	// the controller manager used for loading the animations
  class CControllerManager * m_pControllerManager;

	// This is the bone hierarchy. All the bones of the hierarchy are present in this array
	TFixedArray<CryBoneInfo> m_arrBones;

	typedef std::map<const char*, unsigned, LessString<const char*> > MapBoneNameIndex;
	MapBoneNameIndex m_mapBoneNameIndex;

	// the array of morph targets for this (also currently treated as animations)
	typedef TFixedArray<CryGeomMorphTarget> CryGeomMorphTargetArray;
	CryGeomMorphTargetArray m_arrMorphTargets;

	// the array of optimized morph targets, working on another skinning algorithm
	// only LOD-0 skins
	typedef TFixedArray<CrySkinMorph> CrySkinMorphArray;
	CrySkinMorphArray m_arrMorphSkins;

	// resets the morph target array, and allocates the given number of targets, with the given number of LODs each
	void reinitMorphTargets (unsigned numMorphTargets, unsigned numLODs);

  float m_fTicksPerFrame;
  float m_fSecsPerTick;


	// NOTE: this is to be deprecated
	TFixedArray<unsigned> m_arrTempBoneIdToIndex;

	size_t sizeofThis()const;
};

#endif
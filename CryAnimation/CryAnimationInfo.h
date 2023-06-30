#ifndef _CRY_ANIMATION_CRY_ANIMATION_INFO_HDR_
#define _CRY_ANIMATION_CRY_ANIMATION_INFO_HDR_

struct AnimEvent { AnimSinkEventData UserData; float fTime; };
inline bool operator < (const AnimEvent& left, const AnimEvent& right)
{
	return left.fTime < right.fTime;
}
// returns true if the given events are equal with the given time tolerance
inline bool isEqual (const AnimEvent& left, const AnimEvent& right, float fTolerance = 1e-3f)
{
	return left.UserData == right.UserData
		&& fabs(left.fTime - right.fTime) < fTolerance;
}

//! this structure contains info about loaded animations
struct AnimData
{
	string strName; // the name of the animation (not the name of the file) - unique per-model
	float fStart, fStop; // start and stop time, in seconds
	int nGlobalAnimId;
  bool  bLoop;

	AnimData():
		fStart(0), fStop(0), bLoop(false),
		nGlobalAnimId (-1)
	{
	}
	bool isStatic () const {return fStart == fStop;}
	float getLength() const {return fStop - fStart;}

	size_t sizeofThis()const
	{
		return sizeof(*this) + strName.capacity();
	}
};

struct MiniRangeEntity {
	int start;
	int end;
	MiniRangeEntity()
	{
		start = end = 0;
	}
	void operator = (const RANGE_ENTITY& right)
	{
		this->start = right.start;
		this->end = right.end;
	}
};



//////////////////////////////////////////////////////////////////////////
// this is the animation information on the module level (not on the per-model level)
// it doesn't know the name of the animation (which is model-specific), but does know the file name
// Implements some services for bone binding and ref counting
struct GlobalAnimation
{
	// Since we know the number of controllers per animation from the beginning and don't
	// change it, we could use more econimical TFixedArray here instead of STL or other array.
	typedef IController_AutoArray ControllerArray;

	// the flags used in the nFlags member
	enum
	{
		// if this is true, then the animation has valid info data (has been loaded at least once)
		FLAGS_INFO_LOADED = 1,
		// this doesn't allow the auto-unloading to happen
		FLAGS_DISABLE_AUTO_UNLOAD = 1 << 1,
		// this forces the animation to be loaded immediately
		FLAGS_DISABLE_DELAY_LOAD = 1 << 2,
		// this disables the error log in LoadAnimation() in case the animation wasn't found on disk
		FLAGS_DISABLE_LOAD_ERROR_LOG = 1 << 3,

		// if this flag is set, it means that the animation is being loaded right now via asynchronous operation
		// and shouldn't be attempted to be played back or loaded
		FLAGS_LOAD_PENDING = 1 << 4,

		// this is the flag combination that should be applied only to default animations
		FLAGS_DEFAULT_ANIMATION = FLAGS_DISABLE_DELAY_LOAD|FLAGS_DISABLE_LOAD_ERROR_LOG,

		// combination of all possible flags
		FLAGS_ALL_FLAGS = (1<<5) - 1,

		// the flags by default
		FLAGS_DEFAULT_FLAGS = 0
	};

	GlobalAnimation ()
	{
		// some standard values to fill in before the animation will be loaded
		nRefCount = 0;
		nFlags = FLAGS_DEFAULT_FLAGS;
		fSecsPerTick = 0.000208f;
		nLastAccessFrameId = g_nFrameID;
		nTickCount = nStartCount = nApplyCount = 0;
		nTicksPerFrame = 160;
		// the defaults that are anyway immediately overridden
		fScale = 0.01f;
		rangeGlobal.end = 900; // this is in ticks, means 30 frames
	}
	
	IController*GetController(unsigned nControllerID)
	{
		ControllerArray::iterator it = std::lower_bound(arrCtrls.begin(), arrCtrls.end(), nControllerID, AnimCtrlSortPred());
		if (it != arrCtrls.end() && (*it)->GetID() == nControllerID)
		{
			IController *c = *it;
#ifdef _DEBUG
			// set this to true in the debugger to obtain the 0th frame p and q
			bool bCheck = false;
			if (bCheck)
			{
				CryQuat q; Vec3d p;
				c->GetValue(0, q, p);
			}
#endif
			return c;
		}
		return NULL;
	}

	bool IsLoaded()const {return !arrCtrls.empty();}

	bool IsInfoLoaded()const {return (nFlags&FLAGS_INFO_LOADED) != 0;}
	void OnInfoLoaded() {nFlags |= FLAGS_INFO_LOADED;}
	bool IsAutoUnload()const {return (nFlags&FLAGS_DISABLE_AUTO_UNLOAD)== 0;}

	void OnTick ()
	{
		++nTickCount;
		nLastAccessFrameId = g_nFrameID;
	}
	void OnStart()
	{
		++nStartCount;
		nLastAccessFrameId = g_nFrameID;
	}
	void OnApply()
	{
		++nApplyCount;
		nLastAccessFrameId = g_nFrameID;
	}

	void AddRef()
	{
		++nRefCount;
	}

	void Release()
	{
		if (!--nRefCount)
		{
#ifdef _DEBUG
			for (ControllerArray::iterator it = arrCtrls.begin(); it!= arrCtrls.end(); ++it)
				assert ((*it)->NumRefs()==1); // only this object references the controllers now
#endif
			arrCtrls.clear(); // nobody uses the controllers; clean them up. This makes the animation effectively unloaded
		}
	}

#ifdef _DEBUG
	// returns the maximum reference counter from all controllers. 1 means that nobody but this animation
	// structure refers to them
	int MaxControllerRefCount()
	{
		if (arrCtrls.empty())
			return 0;
		int nMax = arrCtrls[0]->NumRefs();
		for (ControllerArray::iterator it = arrCtrls.begin()+1; it!= arrCtrls.end(); ++it)
			if((*it)->NumRefs() > nMax)
				nMax = (*it)->NumRefs();
		return nMax;
	}
#endif
	size_t sizeofThis ()const;



	// controllers comprising the animation; within the animation, they're sorted by ids
	ControllerArray arrCtrls;
	// timing data, retrieved from the timing_chunk_desc
	int nTicksPerFrame;
	float fSecsPerTick;
	float fScale; // the parameter from the initial load animation, used for reloadin

	MiniRangeEntity rangeGlobal;

	// the file name of the animation
	string strFileName;

	// the number of times this animation has been accessed
	unsigned nTickCount;
	// the number of times this animation has been started
	unsigned nStartCount;
	// the number of times this animation has been applied to bones
	unsigned nApplyCount;
	// the last time the animation has been accessed
	int nLastAccessFrameId;
	// the number of referrers to this global animation record (doesn't matter if the controllers are currently loaded)
	int nRefCount;
	// the flags (see the enum in the top of the declaration)
	unsigned nFlags;
};

#endif
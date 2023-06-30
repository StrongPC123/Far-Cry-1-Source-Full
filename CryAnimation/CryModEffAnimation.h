/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Oscar Blasco
//	Taken over by Vladimir Kajalin, Andrey Honich
//  Taken over by Sergiy Migdalskiy
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _EFFECTOR_ANIM_H
#define _EFFECTOR_ANIM_H

#include "AnimationLayerInfo.h"
#include "CryAnimationInfo.h"

class CryModelAnimationContainer;
//////////////////////////////////////////////////////////////////////////
// Animation system supports playback of several animations at the same time.
// It allows to blend for example “Walk” animation and “Reload” animation.
// CCryModEffAnimation contains current state of this layers.
// It performs animations timing and bone animation calculations. 
//
class CCryModEffAnimation:
	public _i_reference_target_t
{
protected:
	float m_fAnimTime;
	int   m_nAnimId;

	// the blend time
	float m_fBlendInTime, m_fBlendOutTime;
	// the original blend-out time, used in STopAnimation()
	float m_fOrigBlendOutTime;

	// the countdown that counts from blendin to 0:
	// the animation may sometimes start not from the beginning, but from the middle,
	// and we won't be able to determine the correct blending - that's what this is needed for
	float m_fBlendInCountdown;

	// the animation start/stop time. Normally, it should be the same as the animation info start/stop time,
	// but sometimes we can wish to stop the animation before it reaches the start/end; in this case, we
	// set this start/stop time to the current time and the animation starts fading out immediately
	float m_fStopTime, m_fStartTime;

	//float m_fTicksPerSecond; // 1/m_pAnimations->getSecsPerTick()
	CryModelState* m_pParent;
	unsigned m_nStartAnimFlags;

	struct SFlags{
		unsigned bLoop:1;
	};
	
	SFlags m_uFlags;

	// this calls all the necessary callbacks
	void OnTimeChanged (int nAnimId, float fPrevTime, float m_fAnimTime,float fSpeed);

	float getTicksPerSecond()const;

	// the animation that is fading out
	struct FadingAnim
	{
		int nAnimId;
		float fTime; // current animation time
		float fBlending; // 1..0 - this gets decreased with each tick
		float fBlendOutSpeed; // - this is the speed fBlending decreased with each tick
		float fLoopLength, fLoopEnd;
		bool bRun; // if this is true, the time gets increased with each tick
		bool bLoop; // if this is true, and bRun, then the time runs within the given loop interval

		bool Tick (float fDeltaTime, bool bFade);
	};
	typedef std::vector<FadingAnim> FadingAnimArray;
	// the died animations
	FadingAnimArray m_arrFadeAnims;

	// ticks the fading animations and adds them to the animation layer array
	unsigned TickFadingAnims (float fDelta, float fWeightLeft, std::vector<CAnimationLayerInfo>& arrLayers, bool bFade = true);

	float getBlending()const;

	static std::vector<CAnimationLayerInfo>g_arrLocalLayers;

public:
	class Validator
	{
	public:
		Validator(CCryModEffAnimation* pParent):
			m_pParent(pParent)
		{
			m_pParent->selfValidate();
		}
		~Validator()
		{
			m_pParent->selfValidate();
		}
		CCryModEffAnimation* m_pParent;
	};
	friend class Validator;

	void selfValidate()
	{
		assert (m_fAnimTime <= m_fStopTime + 0.01f && m_fAnimTime >= m_fStartTime - 0.01f );
	}

	string dump();
	unsigned getStartAnimFlags ()const {return m_nStartAnimFlags;}

	static void initClass();
	static void deinitClass();

	CCryModEffAnimation(CryModelState* pParent);

	// advances the current time of the played animation and returns the blending factor by which this animation affects the bone pose
	unsigned Tick (float deltatime, const std::vector<ICharInstanceSink *>& arrSinks, std::vector<CAnimationLayerInfo>& arrLayers);

	void SetNoLoopNoBlendOut();
	void SetNoLoop();

	// forcibly sets the current time of the animation, in seconds
	void SetCurrentTime (float fTime);
	float GetCurrentTime() const { return m_fAnimTime; };
	void Reset();
	void StartAnimation(unsigned nAnimID, float fBlendInTime, float fBlendOutTime, CCryModEffAnimation* pSynchronizeWith,float fSpeed, unsigned nStartAnimFlags = 0);
	void stop ();
	float GetPhase()const;

	float GetTimeTillEnd() {return m_fStopTime - m_fAnimTime;}

	// returns the number of any animation currently being played
	// this can be the current animation, including animation of fadeout, if the 
	// layer doesn't have the current animation played, but has an animation fading out
	int GetAnyCurrentAnimation()const;

	// is this animation effector completely stopped?
  bool IsStopped();

	// does this animation effector actively plays some animations?
	bool isActive ();
};

TYPEDEF_AUTOPTR(CCryModEffAnimation);

#endif
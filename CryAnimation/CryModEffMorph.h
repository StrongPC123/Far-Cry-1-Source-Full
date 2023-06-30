#ifndef _CRY_MODEL_EFFECTOR_ANIMATION_MORPH_HDR_
#define _CRY_MODEL_EFFECTOR_ANIMATION_MORPH_HDR_

#include "CryAnimationBase.h"
#include <CryCharMorphParams.h>

class CryModelAnimationContainer;

class CryModEffMorph
{
public:
	CryModEffMorph(/*CryModelAnimationContainer* pAnimations*/);

	// advances the current time of the played animation and returns the blending factor by which this animation affects the bone pose
	void Tick (float fDeltaTime);

	// starts the morphing sequence
	void StartMorph (int nMorphTargetId, const CryCharMorphParams& rParams);

	// returns false when this morph target is inactive
	bool isActive()const ;

	// returns the blending factor for this morph target
	float getBlending()const;

	// returns the morph target
	int getMorphTargetId () const;

	void setTime(float fTime) {m_fTime = fTime;}
	void setSpeed (float fSpeed) {m_Params.fSpeed = fSpeed;}
	void stop();

	float getTime() const {return m_fTime;}
	void freeze() {m_nFlags |= m_Params.FLAGS_FREEZE;}
protected:
	
	// the animation container that will answer all questions regarding the morph target
	//CryModelAnimationContainer* m_pAnimations;

	// the blend time
	CryCharMorphParams m_Params;
	// time of morphing
	float m_fTime;
	// morph target id
	int m_nMorphTargetId;
	unsigned m_nFlags; // the copy of the flags from m_Params
};

#endif
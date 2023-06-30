/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Oscar Blasco
//	Taken over by Vladimir Kajalin, Andrey Honich
//  Taken over by Sergiy Migdalskiy <sergiy@crytek.de>
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _EFFECTOR_H
#define _EFFECTOR_H


class ICryModEffector: public _i_reference_target_t
{
public:
	// changes the curernt time by the given value, given in ticks
	virtual void Tick (float deltatime, float fBlendSpeed) = 0; // If 0 the effector will be removed fron list.
	// forcibly sets the current time of the animation, in frames
	virtual void SetCurrentTime (float fTime, float fBlendSpeed) {}
	virtual void ApplyToBone(CryBone *bone, unsigned layer, AnimTwinMode eTwinMode) = 0;
  virtual bool IsStopped() { return true; };
  virtual bool IsProceduralAnimsStopped() { return true; };
	virtual void Reset() = 0;
};

TYPEDEF_AUTOPTR(ICryModEffector);

#endif // _EFFECTOR_H

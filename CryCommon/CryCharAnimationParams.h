#ifndef _CRY_COMMON_CRY_CHAR_ANIMATION_PARAMS_HDR_
#define _CRY_COMMON_CRY_CHAR_ANIMATION_PARAMS_HDR_

//////////////////////////////////////////////////////////////////////////
// This structure describes the parameters used to start an animation
// on a character.
struct CryCharAnimationParams
{
	CryCharAnimationParams(
		float _fBlendInTime = 0.125f,
		float _fBlendOutTime = 0.125f,
		int _nLayerID = 0,
		unsigned _nFlags = 0):
		fBlendInTime (_fBlendInTime),
		fBlendOutTime (_fBlendOutTime),
		nLayerID (_nLayerID),
		nFlags (_nFlags)
	{
	}

	// Summary:
	//   Flags used by CryCharAnimationParams to set the playback and synchronization of the animation.
	enum EFlagsEnum
	{
		// Means that the corresponding animation for attachments will be played
		// (by default, no animated attachments receive the command to play this animation).
		FLAGS_RECURSIVE                = 1,

		// If set, the animation is started at the same phase as the one in layer 0.
		FLAGS_SYNCHRONIZE_WITH_LAYER_0 = 1 << 1,

		// Means that the animation will be queued until the current one is finished
		// and that the next animation will also be queued until this (aligned) one is finished.
		FLAGS_ALIGNED                    = 1 << 2,

		// If set, the animation is not treated as default idle animation
		// otherwise, if it's looped, it'll be used as default idle animation.
		FLAGS_NO_DEFAULT_IDLE          = 1 << 3
	};

	// blendin and out times of the animation

	// Used to specify the blend-in length of the animation.
	float fBlendInTime;
	// Used to specify the blend-out length of the animation.
	float fBlendOutTime;
	
	// Specify the layer where to start the animation.
	int nLayerID;

	// Combination of flags defined above.
	unsigned nFlags;
};

#endif
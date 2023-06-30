//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:CryCharMorphParams.h
//  Parameters for the morphing functions in ICryCharInstance interface
//
//	History:
//	-March 3,2003:Created by Sergiy Migdalskiy
//
//////////////////////////////////////////////////////////////////////

#ifndef _CRY_CHAR_MORPH_PARAMS_HDR_
#define _CRY_CHAR_MORPH_PARAMS_HDR_

// StartMorph will accept this
struct CryCharMorphParams
{
	CryCharMorphParams (
			float _fBlendIn = 0.15f,
			float _fLength = 0,
			float _fBlendOut = 0.15f,
			float _fAmplitude = 1,
			float _fStartTime = 0,
			float _fSpeed = 1,
			unsigned _nFlags = 0
		):
		fBlendIn (_fBlendIn),
		fLength (_fLength),
		fBlendOut (_fBlendOut),
		fAmplitude (_fAmplitude),
		fStartTime (_fStartTime),
		fSpeed (_fSpeed),
		nFlags(_nFlags)
	{
	}
	// the blend-in time
	float fBlendIn;
	// the time to stay in static position
	float fLength;
	// the blend-out time
	float fBlendOut;
	// the maximum amplitude
	float fAmplitude;
	// the initial time phase from which to start morphing, within the cycle
	float fStartTime;
	// multiplier of speed of the update; 1 is default:
	float fSpeed;

	enum FlagsEnum
	{
		// with this flag set, the attachments will be traversed to attempt to start the same morph target
		FLAGS_RECURSIVE = 1,
		// with this flag set, the morph will not be time-updated (it'll be frozen at the point where it is)
		FLAGS_FREEZE    = 1 << 1,
		FLAGS_NO_BLENDOUT = 1 << 2
	};

	// optional flags, as specified by the FlagsEnum
	unsigned nFlags;
};

#endif
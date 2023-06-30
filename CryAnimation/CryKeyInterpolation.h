/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

//
// This is the declaration of key interpolation and optimization functions
// This may be used in the engine and in the export plugin
// CryBoneKey is assumed to be already declared when you include this header.

#ifndef _CRY_KEY_INTERPOLATION_HEADER_
#define _CRY_KEY_INTERPOLATION_HEADER_

// interpolates the given key linearly out of the given left and right keys, given the time
extern void InterpolateCryBoneKey (const CryBoneKey& keyLeft, const CryBoneKey& keyRight, int nTime, CryBoneKey& keyOutput);

// check whether the difference between the two keys is within the specified bounds
extern bool IsErrorSmall (const CryBoneKey& key1, const CryBoneKey& key2, float fMaxPosDelta2, float fMinQuatDot);

// Reduces keyframes that can be interpolated by surrounding keys.
extern unsigned OptimizeKeys (CryBoneKey* pBoneKeys, unsigned nNumKeys, float fPosError = 1e-6f, float fQuatError = 0.9999f);


#endif
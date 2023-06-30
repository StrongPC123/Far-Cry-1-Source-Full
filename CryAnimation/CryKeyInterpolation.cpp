// Actually, the following files might have been made static methods of corresponding
// key structures; but the key structures are declared in a common interface file for now.

#include "stdafx.h"
#include "CryKeyInterpolation.h"


// interpolates the given key linearly out of the given left and right keys, given the time
void InterpolateCryBoneKey (const CryBoneKey& keyLeft, const CryBoneKey& keyRight, int nTime, CryBoneKey& keyOutput)
{
	keyOutput.time = nTime;
	float fTime = float (nTime-keyLeft.time) / (keyRight.time-keyLeft.time);
	keyOutput.relpos = keyLeft.relpos + (keyRight.relpos - keyLeft.relpos) * fTime;
	keyOutput.relquat = Slerp (keyLeft.relquat, keyRight.relquat, fTime);
	keyOutput.relquat.Normalize();
}

// check whether the difference between the two keys is within the specified bounds:
// the Dot product of the quaternions (1==none), and the difference between positions (0=none)
// NOTE: the position delta is defined as a square
bool IsErrorSmall (const CryBoneKey& key1, const CryBoneKey& key2, float fMaxPosDelta2, float fMinQuatDot)
{
	return GetLengthSquared((key1.relpos - key2.relpos)) < fMaxPosDelta2
		&& fabs(key1.relquat|(key2.relquat)) > fMinQuatDot;
}


// Reduces keyframes that can be interpolated by surrounding keys.
// The error metric is max[1-|QDot|] max[]
// Returns the count of optimized keys, and the reduced keys themselves in the passed in array.
unsigned OptimizeKeys (CryBoneKey* pBoneKeys, unsigned nNumKeys, float fPosError, float fQuatError)
{
	// for each starting interval key
	if (nNumKeys > 2)
	for (unsigned i = 0; i < nNumKeys-2; ++i)
  {
    CryBoneKey& keyLeft = pBoneKeys[i];
		unsigned nBest = i + 1; // best interval end
		// for each ending interval key
		for (unsigned j = i+2; j < nNumKeys; ++j)
		{
			CryBoneKey& keyRight = pBoneKeys[j];
			
			nBest = j; // starting with empty interval (no keys to reduce)

			// for each key inside the interval
			for (unsigned k = i+1; k < j; ++k)
			{
				CryBoneKey& keyMiddle = pBoneKeys[k];
				CryBoneKey keyInterp;
				InterpolateCryBoneKey(keyLeft, keyRight, keyMiddle.time, keyInterp);
				if (!IsErrorSmall(keyMiddle, keyInterp, fPosError, fQuatError))
				{
					nBest = j - 1; // this interval is bad, but the previous one was good
					j = nNumKeys; // stop iterating
					break;
				}
			}
		}
		if (nBest > i+1)
		{
      memmove (&pBoneKeys[i+1], &pBoneKeys[nBest], (nNumKeys-nBest)*sizeof(CryBoneKey));
			nNumKeys -= nBest - i - 1;
		}
  }

	while (nNumKeys > 1 && IsErrorSmall(pBoneKeys[nNumKeys-1], pBoneKeys[nNumKeys-2], fPosError, fQuatError))
	{
		nNumKeys--;
	}

	while (nNumKeys > 1 && IsErrorSmall(pBoneKeys[0], pBoneKeys[1], fPosError, fQuatError))
	{
		nNumKeys--;
		memmove (pBoneKeys, pBoneKeys+1, nNumKeys*sizeof(CryBoneKey));
	}

	return nNumKeys;
}

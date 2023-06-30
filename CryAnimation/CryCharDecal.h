/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	  Sep 25 2002 :- Created by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_ANIMATION_CRY_CHAR_DECAL_HDR_
#define _CRY_ANIMATION_CRY_CHAR_DECAL_HDR_

#define DECAL_USE_HELPERS 0

#include <TArray.h>
#include "SparseArrayDriver.h"
#include "CryCharDecalCommon.h"


//////////////////////////////////////////////////////////////////////////
// the decal structure describing the realized decal with the geometry
// it consists of primary faces/vertices (with the full set of getter methods declared with the special macro)
// and helper faces/vertices. Primary faces index into the array of primary vertices. Primary vertices
// refer to the indices of the character vertices and contain the new UVs
// Helper vertices are explicitly set in the object CS, and helper faces index within those helper vertex array.
class CryCharDecal
{
public:
	// sets the current game time for decals (for fading in / out)
	static void setGlobalTime (float fTime) {g_fCurrTime = fTime;}

	// initializes it; expects the decal to come with vPos in LCS rather than WCS
	CryCharDecal (){}
	void buildFrom (class CryCharDecalBuilder& builder);

	// starts fading out the decal from this moment on
	void startFadeOut(float fTimeout);

	// returns true if this is a dead/empty decal and should be discarded
	bool isDead();

	// natural order of decal sorting is by material (texture id)
	bool operator < (const CryCharDecal& rThat)const
	{
		return getTextureId() < rThat.getTextureId();
	}
	

	DECLARE_VECTOR_GETTER_METHODS(CryCharDecalFace, Face, Faces, m_arrFaces);
	DECLARE_VECTOR_GETTER_METHODS(CryCharDecalVertex, Vertex, Vertices, m_arrVertices);

#if DECAL_USE_HELPERS
	// helper vertex access methods - see the class header comment for more info
	unsigned numHelperFaces () const;
	unsigned numHelperVertices() const;
  Vec3 getHelperVertex (unsigned i) const;
	CryUV getHelperUV (unsigned i) const;
	CryCharDecalFace getHelperFace (unsigned i) const;

	const Vec3& getSourceWCS()const;

	void debugDraw(const Matrix& matCharacter);
#endif
	const Vec3& getSourceLCS () const;

	int getTextureId ()const {return m_nTextureId;}

	// returns the decal multiplier: 0 - no decal, 1 - full decal size
	float getIntensity()const;
protected:
	// the faces comprising the decal
	// the face that's the center of the decal is 0th
	std::vector<CryCharDecalFace> m_arrFaces;

	// the mapping of the new vertices to the old ones
	std::vector<CryCharDecalVertex> m_arrVertices;

	// the texture of the decal
	int m_nTextureId;
	
	// the point, in LCS , where the shot was made
	Vec3 m_ptSource;

#if DECAL_USE_HELPERS
	// the Transformation Matrix that transforms the point in helper CS
	// into the character CS; Z axis is the direction of the bullet
	Matrix m_matBullet;
	float m_fSize;
#endif

	// this is the time when the decal was built, in seconds
	float m_fBuildTime;

	// this is the fade in time, in seconds
	float m_fFadeInTime;

	// this is the fade out time, in seconds; <0 at the start (means no fade out)
	float m_fFadeOutTime;

	// this is the time by which the decal should be faded out; 1e+30 at the start (very long life)
	float m_fDeathTime;

protected:
	static float g_fCurrTime;
};

#endif
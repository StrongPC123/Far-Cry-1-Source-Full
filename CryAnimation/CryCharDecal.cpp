#include "stdafx.h"
#include "DebugUtils.h"
#include "CryCharDecalBuilder.h"
#include "CryCharDecal.h"

//////////////////////////////////////////////////////////////////////////
// creates the decal topology: finds the faces and vertices and connects
// them into a small mesh representing the decal, calculate the UV coordinates.
// the vertex coordinate in the decal info is in WCS; the LCS version is passed through ptSourceLCS
void CryCharDecal::buildFrom (CryCharDecalBuilder& builder)
{
	m_ptSource    = builder.getSourceLCS();
#if DECAL_USE_HELPERS
	m_matBullet   = builder.getBulletMatrix();
	m_fSize = builder.getDecalInfo().fSize;
#endif

	m_fBuildTime = g_fCurrTime;
	m_fFadeInTime = builder.getDecalInfo().fSize / (0.04f + 0.02f*(rand()%0xFF)/256.0f); // suppose the speed is like this

	m_fDeathTime = m_fBuildTime + 60*60*24; // die in very distant future - in one day
	m_fFadeOutTime = -1; // no fade out

	assert (builder.numDecalFaces());
	assert (builder.numDecalVertices());

	m_nTextureId = builder.getDecalInfo().nTid;
	m_arrFaces.resize (builder.numDecalFaces());
	memcpy (&m_arrFaces[0], builder.getDecalFaces(), builder.numDecalFaces()*sizeof(m_arrFaces[0]));
	m_arrVertices.resize (builder.numDecalVertices());
	memcpy (&m_arrVertices[0], builder.getDecalVertices(), builder.numDecalVertices()*sizeof(m_arrVertices[0]));
}


#if DECAL_USE_HELPERS

unsigned CryCharDecal::numHelperFaces () const
{
	return 8;
}

unsigned CryCharDecal::numHelperVertices() const
{
	return 6;
}

void CryCharDecal::debugDraw(const Matrix& matCharacter)
{
	static const float fColorDecalOuter[4] = {1,1,0.5,1};
	static const float fColorDecalInner[4] = {1,0.5,0.5,1};
	debugDrawSphere(m_matBullet*matCharacter, m_fSize * getIntensity(), fColorDecalInner);
	debugDrawSphere(m_matBullet*matCharacter, m_fSize*1.02f, fColorDecalOuter);
}

Vec3d CryCharDecal::getHelperVertex (unsigned i) const
{
	assert (i < numHelperVertices());
	static const float fForward = 0.18f, fUp = fForward/10, fRight = fUp;
	static const Vec3d arrOctahedronVtx[6] = {
		Vec3d(0,0,fForward/10),
		Vec3d(0,fUp,0),
		Vec3d(fRight,0,0),
		Vec3d(0,-fUp,0),
		Vec3d(-fRight,0,0),
		Vec3d(0,0,-fForward)
	};

	return m_matBullet.TransformPoint(arrOctahedronVtx[i]);
}

CryCharDecalFace CryCharDecal::getHelperFace (unsigned i) const
{
	assert (i < numHelperFaces());
	if (i < 4)
		return CryCharDecalFace (0, i+1, (i+1)%4 + 1);
	else
		return CryCharDecalFace (5, (i-4+1)%4 + 1, i-4+1);
}

CryUV CryCharDecal::getHelperUV (unsigned i) const
{
	assert (i < numHelperVertices());
	CryUV uv;
	if (i == 0 || i == 5)
		uv.u = uv.v = 0;
	else
	{
		if (i & 1)
			uv.u = 1, uv.v = 0;
		else
			uv.u = uv.v = 1;
	}
	return uv;
}
#endif

// returns the decal multiplier: 0 - no decal, 1 - full decal size
float CryCharDecal::getIntensity()const
{
	if (g_fCurrTime >= m_fDeathTime)
		// we've faded out
		return 0;

	if (g_fCurrTime > m_fDeathTime - m_fFadeOutTime)
		// we're fading out
		return 1-sqr(1-(m_fDeathTime - g_fCurrTime) / m_fFadeOutTime);


	float fLifeTime = (g_fCurrTime - m_fBuildTime);
	if (fLifeTime > m_fFadeInTime)
		// we've faded in
		return 1;
	else
		// we're fading in
		return 1 - sqr(1 - fLifeTime / m_fFadeInTime);
}

const Vec3d& CryCharDecal::getSourceLCS()const
{
	return m_ptSource;
}

// returns true if this is a dead/empty decal and should be discarded
bool CryCharDecal::isDead()
{
	return g_fCurrTime >= m_fDeathTime;
}

// starts fading out the decal from this moment on
void CryCharDecal::startFadeOut(float fTimeout)
{
	m_fFadeOutTime = fTimeout;
	m_fDeathTime = g_fCurrTime + fTimeout;
}


float CryCharDecal::g_fCurrTime = 0;
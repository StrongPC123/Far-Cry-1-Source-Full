#include "stdafx.h"
#include "drand.h"
#include "MathUtils.h"
#include "CryCharParticleManager.h"
#include "CVars.h"

CryCharParticleManager::CryCharParticleManager():
	m_numActive (0),
	m_nLastFrame (0)
{
}


//////////////////////////////////////////////////////////////////////////
// adds a particle spawn task, returns a handle to be used to 
int CryCharParticleManager::add (const ParticleParams& rParticleInfo, const CryParticleSpawnInfo& rSpawnInfo)
{
	validateThis();
	// only 1 emitter is supported
	unsigned nHandle = 0;
	while (nHandle < m_arrEmitters.size() && m_arrEmitters[nHandle].m_bActive)
		++nHandle;

	if (nHandle >= m_arrEmitters.size())
		m_arrEmitters.resize(nHandle + 1);
	m_arrEmitters[nHandle].m_ParticleInfo = rParticleInfo;
	m_arrEmitters[nHandle].m_SpawnInfo    = rSpawnInfo;
	m_arrEmitters[nHandle].m_bActive      = true;

	++m_numActive;
	validateThis();

	return nHandle;
}

//////////////////////////////////////////////////////////////////////////
// deletes a particle spawn task by the handle
bool CryCharParticleManager::remove (int nHandle)
{
	validateThis();
	if (nHandle == -1)
	{
		bool bResult = m_arrEmitters.empty();
		m_arrEmitters.clear();
		m_numActive = 0;
		return bResult;
	}

	if (nHandle >= (int)m_arrEmitters.size() || nHandle < 0)
		return false;

	if (!m_arrEmitters[nHandle].m_bActive)
		return false; // already deactivated
	
	m_arrEmitters[nHandle].m_bActive = false;

	size_t nSize = m_arrEmitters.size();
	while (nSize > 0 && !m_arrEmitters[nSize-1].m_bActive)
		--nSize;
	m_arrEmitters.resize (nSize);

	--m_numActive;
	validateThis();

	if (empty())
		m_nLastFrame = 0;

	return true;
}

// returns true if there are no emitters
bool CryCharParticleManager::empty() const
{
	return m_arrEmitters.empty();
}


// spawn the particles (using the external tangent info and mapping)
void CryCharParticleManager::spawn (const SpawnParams& params)
{
	float fTime = g_GetTimer()->GetCurrTime();
	if (g_nFrameID == m_nLastFrame)
		return;
	m_nLastFrame = g_nFrameID;
	
	float fDeltaTime = g_GetTimer()->GetFrameTime();

	for (unsigned i = 0; i < m_arrEmitters.size(); ++i)
	{
		Emitter& emitter = m_arrEmitters[i];
		if (emitter.m_bActive)
		{
			emitter.spawn (params, fDeltaTime);
			if (emitter.m_SpawnInfo.nFlags & CryParticleSpawnInfo::FLAGS_ONE_TIME_SPAWN)
				remove (i);
		}
	}
}


// evaluates if a vertex with such base is valid for spawning a particle
bool CryCharParticleManager::Emitter::isValid (const SPipTangents& rBase)
{
	if (!(m_SpawnInfo.nFlags & CryParticleSpawnInfo::FLAGS_RAIN_MODE))
		return true;

	return (rBase.m_TNormal * m_ParticleInfo.vGravity < 0);
}


void CryCharParticleManager::Emitter::spawnFromBone(const SpawnParams& params)
{
	// both position and normal (and later the gravity/wind direction) are in WCS
	
	ParticleParams Particle = m_ParticleInfo;
	Particle.vPosition = m_SpawnInfo.vBonePos;

	if ((unsigned)m_SpawnInfo.nBone < params.numBoneMatrices)
	{
		const Matrix44& mxBone = params.pBoneGlobalMatrices[m_SpawnInfo.nBone];
		Particle.vPosition = mxBone.TransformPointOLD(Particle.vPosition);
		Particle.vDirection = mxBone.TransformVectorOLD(Particle.vDirection);
	}

	Particle.vPosition = params.pModelMatrix->TransformPointOLD (Particle.vPosition);
	Particle.vDirection = params.pModelMatrix->TransformVectorOLD (Particle.vDirection);

	/*
	float fPitchCos = (float)(drand()*2-1);
	float fPitchSin = (float)sqrt(1-fPitchCos*fPitchCos);
	float fYaw = (float)(drand() * 2 * gPi);
	struct {float fCos, fSin;} Yaw;
	CosSin (fYaw, &Yaw.fCos);

	m_ParticleInfo.vDirection.x = Yaw.fSin * fPitchSin;
	m_ParticleInfo.vDirection.y = fPitchCos;
	m_ParticleInfo.vDirection.z = Yaw.fCos * fPitchSin;
	*/

	Get3DEngine()->SpawnParticles(Particle);
}

// spawns one particle from the skin
void CryCharParticleManager::Emitter::spawnFromSkin(const SpawnParams& params)
{
	// find the face that's ok for spawning the particle
	Vec3 arrFace[2][4]; // the first [0..2] are the vertices of the face, the [3] one is the normal
	Vec3* pBestFace = arrFace[0], *pTempFace = arrFace[1];

	Vec3 vWind = m_ParticleInfo.vGravity;

	if (m_SpawnInfo.nFlags & m_SpawnInfo.FLAGS_RAIN_MODE)
	{
		float fBestBet; // minimize this dot product of gravity and normal
		Vec3 vWindLCS = UntransformVector (*params.pModelMatrix, vWind); // gravity in the LCS of the character

		int nRainPower = g_GetCVars()->ca_RainPower_Clamp (3,40);

		// attempt #0
		params.getFaceVN (irand() % params.numFaces, pBestFace);
		fBestBet = vWindLCS * pTempFace[3];

		for (int nAttempt = 1; nAttempt < nRainPower; ++nAttempt)
		{
			params.getFaceVN(irand() % params.numFaces, pTempFace);

			float fContraWind = vWindLCS * pTempFace[3];
			if (fContraWind < fBestBet)
			{
				fBestBet = fContraWind;
				std::swap (pBestFace, pTempFace);
			}
		} 
	}
	else
	{
		params.getFaceVN (irand() % params.numFaces, pBestFace);
	}

	// get random point (in barycentric coordinates) on the triangle
	Vec3 vBR ((float)drand()+0.001f,(float)drand(),(float)drand());
	vBR /= vBR.x+vBR.y+vBR.z;
	Vec3 vSpawnPoint = vBR.x *	pBestFace[0] + vBR.y * pBestFace[1] + vBR.z * pBestFace[2];

	// both position and normal (and later the gravity/wind direction) are in WCS
	m_ParticleInfo.vPosition = params.pModelMatrix->TransformPointOLD (vSpawnPoint);

	//CHANGED_BY_IVO - INVALID CHANGE, PLEASE REVISE
	Vec3 vNormal = params.pModelMatrix->TransformVectorOLD(pBestFace[3]);

	// we've found something appropriate
	if (m_SpawnInfo.nFlags & m_SpawnInfo.FLAGS_RAIN_MODE)
	{
		m_ParticleInfo.vDirection = 
			(2*(vNormal*vWind))*vNormal - vWind
			//vNormal
			;
	}
	else
	{
		m_ParticleInfo.vDirection = vNormal;
	}

	Get3DEngine()->SpawnParticles(m_ParticleInfo);
}

// spawn the particles (using the external tangent info and mapping)
void CryCharParticleManager::Emitter::spawn (const SpawnParams& params, float fDeltaTime)
{
	if (!m_bActive)
		return;
	for (m_fParticleAccumulator += fDeltaTime * m_SpawnInfo.fSpawnRate; m_fParticleAccumulator > 1; m_fParticleAccumulator -= 1)
	{
		spawnSingleParticle(params);
	}

	if (m_fParticleAccumulator >= 0)
	{
		// dither the particles by time: if there are 1.9 particles to be spawned, the 1st particle is spawned and
		// the second is spawned in 90% of cases. If this emitter is re-added on the next frame anew, this will look in
		// the average as good as if it's here all the time. If it's kept, then the next frame it'll be spawned with less probability
		// (because of negative accumulator number)
		if (drand () < m_fParticleAccumulator)
		{
			m_fParticleAccumulator -= 1;
			spawnSingleParticle (params);
		}
	}
}

// spawns only one particle with the params
void CryCharParticleManager::Emitter::spawnSingleParticle (const SpawnParams& params)
{
	if (m_SpawnInfo.nFlags & CryParticleSpawnInfo::FLAGS_SPAWN_FROM_BONE)
		spawnFromBone (params);
	else
		spawnFromSkin(params);
}

void CryCharParticleManager::validateThis()
{
#ifdef _DEBUG
	// check the coherence of the number of active emitters
	unsigned numActiveEmitters = 0;
	for (unsigned i = 0; i < m_arrEmitters.size(); ++i)
		if (m_arrEmitters[i].m_bActive)
			++numActiveEmitters;
	assert (numActiveEmitters == m_numActive);
#endif
}

void CryCharParticleManager::GetMemoryUsage (ICrySizer* pSizer)
{
	size_t nSize = sizeof(*this);
	nSize += m_arrEmitters.size() * sizeof(Emitter);
	pSizer->AddObject(this, nSize);
}
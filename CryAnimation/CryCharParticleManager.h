/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	  Jan 15 2003 :- Created by Sergiy Migdalskiy
//
//  Notes:
//    This class is used to isolate particle stuff from the ModelState
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CRY_ANIMATION_CRY_CHAR_PARTICLE_MANAGER_HDR__
#define __CRY_ANIMATION_CRY_CHAR_PARTICLE_MANAGER_HDR__

#include "I3DEngine.h"
#include "CryParticleSpawnInfo.h"
#include "GeomCommon.h"

// This class is used for spawning particles (misc. types simultaneously, if needed)
// from an animated character
class CryCharParticleManager
{
public:

	CryCharParticleManager();

	// adds a particle spawn task, returns a handle to be used to 
	int add (const ParticleParams& rParticleInfo, const CryParticleSpawnInfo& rSpawnInfo);

	// deletes a particle spawn task by the handle
	bool remove (int nHandle);

	// returns true if there are no emitters; when there are no emitters, it's not necessary to call spawn()
	bool empty() const;

	// the spawn parameters
	struct SpawnParams
	{
		// the pointer to the (internal indexation) vertices
		const Vec3* pVertices;
		// the pointer to the (internal indexation) normals; 
		// NOTE: this is optional and IS sometimes NULL.
		const class Vec3dA16* pNormalsA16;
		// the number of vertices (internal indexation) in pVertices array
		unsigned numVertices;

		// faces, in internal indexation
		const GeomFace* pFaces;
		// number of faces
		unsigned numFaces;

		// the model matrix of the character - transforms points to world
		const Matrix44* pModelMatrix;
		// the array of global matrices of bones
		const Matrix44* pBoneGlobalMatrices;
		// the number of bone matrices
		unsigned numBoneMatrices;

		void setVertices (const Vec3* _pVertices, unsigned _numVertices)
		{
			this->pVertices   = _pVertices;
			this->numVertices = _numVertices;
		}

		void setFaces (const GeomFace* _pFaces, unsigned _numFaces)
		{
			this->pFaces   = _pFaces;
			this->numFaces = _numFaces;
		}

		// retrieves the vertices of the given face into [0..2] and the normal into [3]
		void getFaceVN (unsigned nFace, Vec3* pVerts)const
		{
			for (int v = 0; v< 3; ++v)
				pVerts[v] = this->pVertices[this->pFaces[nFace][v]];
			pVerts[3] = (pVerts[2]-pVerts[0])^(pVerts[1]-pVerts[0]);
		}
	};


	// spawn the particles (using the external tangent info and mapping)
	void spawn (const SpawnParams& params);

	void validateThis();

	void GetMemoryUsage (ICrySizer* pSizer);
protected:
	struct Emitter
	{
		ParticleParams m_ParticleInfo;
		CryParticleSpawnInfo m_SpawnInfo;
		
		// the quantity accumulator, to enable < 1 particles per frame
		// the number of particles accumulates here over time
		float m_fParticleAccumulator; 

		// if false, this entry is reserved (doesn't emit any particles)
		bool m_bActive; 
		
		// evaluates if a vertex with such base is valid for spawning a particle
		bool isValid (const SPipTangents& rBase);

		Emitter () :
			m_fParticleAccumulator(0),
			m_bActive (false)
			{
			}

		// spawn the necessary number of particles
		void spawn (const SpawnParams& params, float fTimeDelta);
		// spawns only one particle with the params
		void spawnSingleParticle (const SpawnParams& params);
		// spawns one particle from the skin
		void spawnFromSkin(const SpawnParams& params);
		// spawns one particle from bone
		void spawnFromBone(const SpawnParams& params);
	};
	std::vector<Emitter> m_arrEmitters;
	unsigned m_numActive; // number of active emitters in m_arrEmitters
	int m_nLastFrame;
};

#endif
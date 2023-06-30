/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	  Sep 25 2002 :- Created by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_ANIMATION_CRY_CHAR_DECAL_BUILDER_HDR_
#define _CRY_ANIMATION_CRY_CHAR_DECAL_BUILDER_HDR_

#include "GeomCommon.h"
#include "CryCharDecalCommon.h"
#include "VertexBufferArrayDrivers.h"

//////////////////////////////////////////////////////////////////////////
// class that's used to build the decals for decal manager.
// This is just the builder (factory) class, it's only used to build the instance of decal and is not
// kept in memory
// NOTE: This object memorizes all the references, they must be alive during the life cycle of this object
class CryCharDecalBuilder
{
public:
	// remembers the parameters and gets ready to initialize the Decal upon request
	// memorizes all the references, they must be alive during the life cycle of this object
	CryCharDecalBuilder (struct CryEngineDecalInfo& rDecal, class CryGeometryInfo* pGeometry, const Vec3* pVertices);

	// returns the coordinate of the bullet in the LCS of the character
	const Vec3& getSourceLCS () const {return m_ptSourceLCS;}
	// returns the coordinates of the bullet in the WCS
	//const Vec3& getSourceWCS () const {return m_rDecal.vPos;}
	// returns the matrix of the bullet in LCS
	const Matrix44& getBulletMatrix()const {return m_matBullet;}

	// returns the original decal descriptor structure
	const CryEngineDecalInfo& getDecalInfo () const {return m_rDecal;}

	DECLARE_VECTOR_GETTER_METHODS(CryCharDecalFace, DecalFace, DecalFaces, m_arrDecalFaces);
	DECLARE_VECTOR_GETTER_METHODS(CryCharDecalVertex, DecalVertex, DecalVertices, m_arrDecalVertices);
protected:
	// calculate distances to each vertex
	void initVertexDistances();
	void findParticipatingFaces();

	// adds the face, with the vertices if needeed
	void addFaceCCS (int nVertex[3], Vec3 vVertices[3]); // in Character Coordinate System
	void addFaceBCS (GeomFace GeomIntFace); // in the Bullet coordinate system

	// maps the vertex and returns the interan index
	unsigned addVertexCCS (int nVertexExtIndex, const Vec3& vVertex); // in Character Coordinate System
	// maps the vertex and returns the interan index
	unsigned addVertexBCS (int nVertexExtIndex); // in Bullet Coordinate System

	// initializes the VerticesBCS array - vertices in the bullet coordinate system
	void initVerticesBCS();

	// returns the distance of the given triangle from the origin (bullet)
	float GetDistanceToTriangleBCS (GeomFace nVertex);
protected:
	// the original structure describing the decal, with the vPos set to the position of the bullet in WCS
	CryEngineDecalInfo& m_rDecal;

	// the inverse model TM. Model TM is the TM of the character. This inverse is used to transform from 
	// the World space back into the character model space
	//const Matrix& m_matInvModel;
	
	// the geometry of the character
	CryGeometryInfo* m_pGeometry;
	
	// the temporary vertex array of the skinned character
	const Vec3* m_pSkinVertices;

	// coordinates of the hit (the center of the bullet) in the Local Coordinate system of the character model
	Vec3 m_ptSourceLCS;
	// hit direction (the direction in which the bullet flies) in the LCS of the model
	Vec3 m_ptHitDirectionLCS;

	// bullet TM and inverse TM, relative to the Local coordinate system of the character
	// these matrices are used to project the whole model skin in the bullet CS, the skin being
	// in the LCS of the character, so that the bullet appears in the (0,0,0)
	// Z axis looks in the direction of the hit
	Matrix44 m_matInvBullet, m_matBullet;

	// the array of transformed vertices (where 0,0,0 is the hit point, and 0,0,1 is the direction of the bullet)
	// in other words, it's the Vertices in Bullet Coordinate System
	TElementaryArray<Vec3> m_arrVerticesBCS;

	// distances to each vertex
	TElementaryArray<float> m_arrVertexDistance;
	unsigned m_nNearestVertex; // the nearest vertex, in the external indexation of m_arrVertices

	// faces that will take part in the decal rendering
	std::vector<CryCharDecalFace> m_arrDecalFaces;
	// vertices that will take part in the rendering (referred to by the face array)
	std::vector<CryCharDecalVertex> m_arrDecalVertices;

	// the mapping from the character vertices (in the external indexation) to the decal vertices
	// -1 means the vertex has not yet been mapped
	// It is always true that an element is >= -1 and < m_arrDecalVertices.size()
	TElementaryArray<int> m_arrDecalVertexMapping;
};

#endif
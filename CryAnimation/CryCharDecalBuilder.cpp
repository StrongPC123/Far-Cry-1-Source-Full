#include "stdafx.h"
#include "CryGeometryInfo.h"
#include "MathUtils.h"
#include "CryCharDecalBuilder.h"

#include "CVars.h"

CryCharDecalBuilder::CryCharDecalBuilder (CryEngineDecalInfo& rDecal, CryGeometryInfo* pGeometry, const Vec3d* pVertices):
	m_rDecal (rDecal),
	m_pGeometry (pGeometry),
	m_pSkinVertices (pVertices)
{
	// transform the bullet position to the local coordinates
	m_ptSourceLCS = rDecal.vPos;

	// // normalize the direction vector, if it's present
	float dHitDirectionLength = rDecal.vHitDirection.Length();
	if (dHitDirectionLength > 0.01)
	{
		// we need the hit direction and source in the LCS to project the character
		m_ptHitDirectionLCS = rDecal.vHitDirection / dHitDirectionLength;

		m_ptSourceLCS = m_ptSourceLCS;

		// to project the character in the bullet CS, we need some fake TM (fake because we just choose any X and Y axes)
		// Z axis looks in the direction of the hit
		BuildMatrixFromFwdZRot (m_ptHitDirectionLCS, m_ptSourceLCS, rDecal.fAngle, m_matBullet);
		
		for (int i = 0; i < 3; ++i)
			m_matBullet(2,i) *= 2.0f;
		m_matInvBullet = GetInverted44(m_matBullet);

		initVerticesBCS();
		//initVertexDistances();

		findParticipatingFaces();
	}
	else
	{
		g_GetLog()->LogWarning ("\003CryCharDecalBuilder::CryCharDecalBuilder: hit has unknown direction");
	}
}

// calculate distances to each vertex
void CryCharDecalBuilder::initVertexDistances()
{
	m_arrVertexDistance.reinit (m_pGeometry->numExtToIntMapEntries());
	
	// the index of the vertex nearest to the hit (in External indexation)
	m_nNearestVertex = 0;
	float fNearestVertexDistance2 = 0;
	for (unsigned nVertex = 0; nVertex < m_pGeometry->numExtToIntMapEntries(); ++nVertex)
	{
		const Vec3d& ptVertex = m_pSkinVertices[nVertex];
		float fDistance2 = m_arrVertexDistance[nVertex] = GetSquaredDistance(ptVertex,m_ptSourceLCS);
		if (!nVertex || fNearestVertexDistance2 > fDistance2)
		{
			fNearestVertexDistance2 = fDistance2;
			m_nNearestVertex = nVertex;
		}
	}
}


// find the vertices that participate in decal generation and add them to the array of vertices
// Uses m_arrVerticesBCS
void CryCharDecalBuilder::findParticipatingFaces()
{
	m_arrDecalVertexMapping.reinit (m_pGeometry->numUsedVertices(), -1);
	float fSize2 = g_GetCVars()->ca_EnableDecals() == 2 ? sqr(m_rDecal.fSize*3) : m_rDecal.fSize;

	// find the faces to which the distance near enough
	for (unsigned nFace = 0; nFace < m_pGeometry->numFaces(); ++nFace)
	{
		GeomFace Face = m_pGeometry->getFace(nFace);
		// vertices belonging to the face, in external indexation

		float fTriangleDistance = GetDistanceToTriangleBCS (Face);
		if (fTriangleDistance > -fSize2 && fTriangleDistance <= fSize2)
		{
			// the triangle participates
			addFaceBCS (Face);
		}
	}
}

// returns the distance to the given 2D point in the plane xy
// from the point (0,0)
double distanceToPoint2Dxy (const Vec3d& v)
{
	return sqrt(sqr((double)v.x)+sqr((double)v.y));
}

// returns the distance to the given 2D line in the plane xy
// from the point (0,0). The line is FINITE, and limited by v0 and v1
// the hints are the distances to the corresponding vertices
float distanceToLine2DxyHint (const Vec3d& v0, float d0, const Vec3d& v1, float d1)
{
	// the coefficients of the line equation x -> ax + bx t,  y -> ay + by t
	float dx = v1.x - v0.x, dy = v1.y - v0.y;
	double f2 = dx*(double)dx+dy*(double)dy;
	if (f2 < 0.005)
		return (d0+d1)/2;

	// the point of intersection of the normal from 00 with the line, 0 means v0, 1 means v1, > 1 means beyond v1, <0 means before v0
	double t = -(v0.x*dx + v0.y*dy)/f2;

	if (t <= 0)
	{
		float fDistance = d0;
		assert (d1 > fDistance);
		return fDistance;
	}
	else
	if (t >= 1)
	{
		float fDistance = d1;
		assert (d0 > fDistance);
		return fDistance;
	}
	else
	{
		float fDistance = (float)fabs((v0.x*dy-v0.y*dx) / sqrt (f2));
		assert (fabs(fDistance - (float)fabs((v1.x*dy-v1.y*dx) / sqrt (f2))) < 1e-3);
		return fDistance;
	}
}

// returns the distance to the given 2D line in the plane xy
// from the point (0,0). The line is FINITE, and limited by v0 and v1
float distanceToLine2Dxy (const Vec3d& v0, const Vec3d& v1)
{
	// the coefficients of the line equation x -> v0.x + dx t,  y -> v0.y + dy t
	float dx = v1.x - v0.x, dy = v1.y - v0.y;
	double f2 = dx*(double)dx+dy*(double)dy;
	if (f2 < 0.005)
		// distance to the middle of the interval
		return float(cry_sqrtf(sqr(v0.x+v1.x)+sqr(v0.y+v1.y))/2);

	// the point of intersection of the normal from 00 with the line, 0 means v0, 1 means v1, > 1 means beyond v1, <0 means before v0
	double t = -(v0.x*dx + v0.y*dy)/f2;
	assert (fabs ((v0.x+dx*t)*dx+(v0.y+dy*t)*dy) < cry_sqrtf(sqr(v0.x+v1.x)+sqr(v0.y+v1.y))/200000);

	if (t <= 0)
	{
		double fDistance = distanceToPoint2Dxy(v0);
		assert (distanceToPoint2Dxy(v1) > fDistance);
		return (float)fDistance;
	}
	else
	if (t >= 1)
	{
		double fDistance = distanceToPoint2Dxy(v1);
		assert (distanceToPoint2Dxy(v0) > fDistance);
		return (float)fDistance;
	}
	else
	{
		float fDistance = (float)fabs((v0.x*dy-v0.y*dx) / sqrt (f2));
		assert (fabs(fDistance - (float)fabs((v1.x*dy-v1.y*dx) / sqrt (f2))) < 1e-3);
		return fDistance;
	}
}

// returns the distance to a very thin triangle - that is,
// it's so thin that it's hard to determine whether it contains 0 or not,
// but it shouldn't really matter because it's possible to determine the
// distance to each of the sides (and if some side is degenerate, then
// to the vertices of that side)
float distanceToThinTriangle (const Vec3d v[3])
{
	float d[3] = {
		(float)distanceToPoint2Dxy(v[0]),
		(float)distanceToPoint2Dxy(v[1]),
		(float)distanceToPoint2Dxy(v[2])
	};
	return min3 (distanceToLine2DxyHint (v[0],d[0],v[1],d[1]), distanceToLine2DxyHint (v[1],d[1],v[2],d[2]), distanceToLine2DxyHint (v[2],d[2],v[0],d[0]));
}

// returns the distance of the given triangle from the origin (bullet)
float CryCharDecalBuilder::GetDistanceToTriangleBCS (GeomFace nVertex)
{
	Vec3d v[3] = {m_arrVerticesBCS[nVertex[0]], m_arrVerticesBCS[nVertex[1]], m_arrVerticesBCS[nVertex[2]]};

	if (g_GetCVars()->ca_EnableDecals() == 2)
		return sqr(v[0].x)+sqr(v[0].y)+sqr(v[1].x)+sqr(v[1].y)+sqr(v[2].x)+sqr(v[2].y);

	// calculate the barycentric coordinates of the triangle
	float b0 =  (v[1].x - v[0].x) * (v[2].y - v[0].y) - (v[2].x - v[0].x) * (v[1].y - v[0].y);

	// the triangle either back-faces the bullet or has a negligible area
	// - just return a negative not to use this triangle
	if (g_GetCVars()->ca_PerforatingDecals())
	{
		if (fabs(b0) < 1e-3)  // should be b0 < 1e-4 to return to the previous version
			// return -100000; 
			return distanceToThinTriangle(v);
	}
	else
	{
		if (b0 < 1e-3)
			return -100000;
	}

	float b1 = ( v[1].x * v[2].y - v[2].x * v[1].y ) / b0 ;
	float b2 = ( v[2].x * v[0].y - v[0].x * v[2].y ) / b0 ;
	float b3 = ( v[0].x * v[1].y - v[1].x * v[0].y ) / b0 ;

	if (fabs(b1+b2+b3-1) > 0.03f)
		// this is some almost degenerate triangle, or something else is unstable
	if (g_GetCVars()->ca_PerforatingDecals())
		return distanceToThinTriangle(v);
	else
		return -100000; 

	//assert (fabs(b1+b2+b3-1) < 0.01f);
#ifdef _DEBUG
	// the reconstructed by the barycentric coordinates point
	//Vec3d ptReconstructed = b1 * v[0] + b2 * v[1] + b3 * v[2];
	//assert (ptReconstructed.x < 1e-7/b0 && ptReconstructed.x > -1e-7/b0 && ptReconstructed.y < 1e-7/b0 && ptReconstructed.y > -1e-7/b0);
#endif
	// b1, b2 and b3 are the barycentric coordinates of the point 0
	// if they're all > 0, the point lies inside the triangle (triangle is on the way of the bullet

	if (b1 > 0)
	{
		if (b2 > 0)
		{
			if (b3 > 0)
			{
				// the triangle intersects Oz
				// Determine the intersection point - this will be the distance
				// the triangle faces the bullet - calculate the point of intersection
				return 0;//b1*v[0].z+b2*v[1].z+b3*v[2].z;
			}
			else
			{
				// 0 lies outside the edge v0-v1, so we should calculate the distance to this line
				return distanceToLine2Dxy (v[0],v[1]);
				return max2(distanceToLine2Dxy (v[0],v[1]), min3(v[0].z,v[1].z,v[2].z));
			}
		}
		else
		{
			if (b3 > 0)
			{
				// 0 lies outside the edge v0-v2, so we should calculate the distance to this line
				return distanceToLine2Dxy (v[0],v[2]);
				return max2(distanceToLine2Dxy (v[0],v[2]), min3(v[0].z,v[1].z,v[2].z));
			}
			else
			{
				// 0 lies beside v0
				return (float)distanceToPoint2Dxy (v[0]);
				return max2((float)distanceToPoint2Dxy (v[0]), min3(v[0].z,v[1].z,v[2].z));
			}
		}
	}
	else
	{
		if (b2 > 0)
		{
			if (b3 > 0)
			{
				// 0 lies outside the edge v1-v2, so we should calculate the distance to this line
				return distanceToLine2Dxy(v[1],v[2]);
				return max2(distanceToLine2Dxy(v[1],v[2]), min3(v[0].z,v[1].z,v[2].z));
			}
			else
			{
				// 0 lies beside v[1]
				return (float)distanceToPoint2Dxy(v[1]);
				return max2((float)distanceToPoint2Dxy(v[1]), min3(v[0].z,v[1].z,v[2].z));
			}
		}
		else
		{
			assert (b3 > 0);
			// 0 lies beside v2
			return (float)distanceToPoint2Dxy(v[2]);
			return max2((float)distanceToPoint2Dxy(v[2]), min3(v[0].z,v[1].z,v[2].z));
		}
	}
}

// initializes the VerticesBCS array - vertices in the bullet coordinate system
void CryCharDecalBuilder::initVerticesBCS()
{
	unsigned numVertices = m_pGeometry->numVertices();
	m_arrVerticesBCS.reinit (numVertices);
  for (unsigned i = 0; i < numVertices; ++i)
	{
		m_arrVerticesBCS[i] = m_matInvBullet.TransformPointOLD(m_pSkinVertices[i]);
	}
}


// adds the face, with the vertices if needed
// in Character Coordinate System
void CryCharDecalBuilder::addFaceCCS (int nVertex[3], Vec3d vVertices[3])
{
	m_arrDecalFaces.push_back (CryCharDecalFace(addVertexCCS(nVertex[0], vVertices[0]), addVertexCCS(nVertex[1], vVertices[1]), addVertexCCS(nVertex[2], vVertices[2])));
}

void CryCharDecalBuilder::addFaceBCS (CryCharDecalFace GeomIntFace)
{
	m_arrDecalFaces.push_back (CryCharDecalFace(addVertexBCS(GeomIntFace[0]), addVertexBCS(GeomIntFace[1]), addVertexBCS(GeomIntFace[2])));
}


// maps the vertex and returns the internal index
unsigned CryCharDecalBuilder::addVertexCCS (int nVertexExtIndex, const Vec3d& vVertex)
{
	int& nVertexNewIndex = m_arrDecalVertexMapping[nVertexExtIndex];
	if (nVertexNewIndex == -1)
	{
		// the vertex in bullet coordinates will be the new UVs
		Vec3d vUVW = m_matInvBullet.TransformPointOLD(vVertex);
		m_arrDecalVertices.push_back (CryCharDecalVertex (nVertexExtIndex, (vUVW.x/m_rDecal.fSize+1)*0.5f, (vUVW.y/m_rDecal.fSize+1)*0.5f));
		nVertexNewIndex = int(m_arrDecalVertices.size()-1);
	}

	return nVertexNewIndex;
}

unsigned CryCharDecalBuilder::addVertexBCS (int nSkinVertexIndex)
{
	int& nVertexNewIndex = m_arrDecalVertexMapping[nSkinVertexIndex];
	if (nVertexNewIndex == -1)
	{
		// the vertex in bullet coordinates will be the new UVs
		const Vec3d& vUVW = m_arrVerticesBCS[nSkinVertexIndex];
		m_arrDecalVertices.push_back (CryCharDecalVertex (nSkinVertexIndex, (vUVW.x/m_rDecal.fSize+1)*0.5f, (vUVW.y/m_rDecal.fSize+1)*0.5f));
		nVertexNewIndex = int(m_arrDecalVertices.size()-1);
	}

	return nVertexNewIndex;
}

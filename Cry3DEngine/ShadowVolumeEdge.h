
//////////////////////////////////////////////////////////////////////
//
//	Crytek Indoor Engine DLL source code
//	
//	File: ShadowVolumeEdge.h
//
//	History:
//	-September 29,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef SHADOWVOLEDGE_H
#define SHADOWVOLEDGE_H

#if _MSC_VER > 1000
# pragma once
#endif

//////////////////////////////////////////////////////////////////////
//TODO: Move into Irenderer.h when changing that file it will not cause
//the entire workspace to be recompiled


//////////////////////////////////////////////////////////////////////
#define	FLAG_FALSE_EDGE_SILOHUETTE	1
#define FLAG_IN_SURFACE_CONTACT_V0	2
#define FLAG_IN_SURFACE_CONTACT_V1	4
#define FLAG_BLACK_FACE							8

//////////////////////////////////////////////////////////////////////
struct	CObjFace;
class		CShadowVolEdge;

//this is put into a structure and allocated if necessary to 
//save memory
//////////////////////////////////////////////////////////////////////
struct tAddEdgeInfo
{

	tAddEdgeInfo()
	{
		m_dwFlags=0;
	}
	
	//! shadow plane + 4 volume edges plane's equation	
	Plane			m_Planes[1];

	//! silhouette direction	
	Vec3d			m_vUp;

	/*! since the edge is shared by more than 1 face, i have to keep
	the right vectors here and not in the face
	each face has vright information for both points of the edge
	original vectors	*/	
	Vec3d			m_vRight1[2];
	Vec3d			m_vRight2[2]; 

	//! after interpolation,based on the faces lit by light
	Vec3d			m_vRightInterp1[2];
	Vec3d			m_vRightInterp2[2];
	float			m_fDist1[2];
	float			m_fDist2[2];

	//! extruded edge (1 face only-in and out)
	Vec3d			m_vExtrOut[2];
	Vec3d			m_vExtrIn[2];
	//center	of the projected edge
	Vec3d			m_vCenter; 

	std::vector<CShadowVolEdge*> m_lstConnectedEdges1;
	std::vector<CShadowVolEdge*> m_lstConnectedEdges2;

	unsigned	char m_dwFlags;		
};

//shadow volume edge,used to build static or dynamic silouhette
//////////////////////////////////////////////////////////////////////
class CShadowVolEdge
{
public:	
  CShadowVolEdge()
  {
    m_pFace1=NULL;
    m_pFace2=NULL;   		
		m_nVNum0=m_nVNum1=0;
		m_pAddEdge=NULL;
  }	

	~CShadowVolEdge()
	{
		if (m_pAddEdge)
		{
			delete [] m_pAddEdge;
			m_pAddEdge=NULL;
		}
	}

	/*
	//check if the point is on the extent of the shadow volume edge
	bool	PointOnEdge(const Vec3d &vPoint);
	//clip with planes formed with the edges
	bool	ClipAdditionalEdge(Vec3d &vP1,Vec3d &vP2);
	//to call once the edges and face plane are defined
	void	CreateAdditionalPlanes();	
	*/

	//! orginal ednpoints of the silouhette edge 
	Vec3d			m_vecV0;
	Vec3d			m_vecV1;

	//! for dynamic models, to quick access modified vertices
	int				m_nVNum0;
	int				m_nVNum1; 

	//! projected edge
	//Vec3d			m_vecEv0,m_vecEv1,m_vecEv2,m_vecEv3;
	Vec3d			m_vecEv[4];
	//! the two faces the edge belongs to
	CObjFace	*m_pFace1;
	CObjFace	*m_pFace2;  	

	tAddEdgeInfo	*m_pAddEdge;
};

typedef std::vector<CShadowVolEdge *>::iterator connedgeit;

#endif //shadowvoledge
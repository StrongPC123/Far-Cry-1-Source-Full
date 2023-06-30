
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//	
//	File:Cry_Camera.h
//	Description: Common Camera class implementation
//
//	History:
//	-Feb 08,2001: Created by Marco Corbetta
//	-Jan 20,2003: Taken over by Ivo Herzeg
//
//////////////////////////////////////////////////////////////////////

#ifndef CAMERA_H
#define CAMERA_H

#if _MSC_VER > 1000
# pragma once
#endif


//DOC-IGNORE-BEGIN
#include "Cry_Math.h"
#include "Cry_Geo.h"
//DOC-IGNORE-END

#ifdef WIN64
#include "Cry_XOptimise.h" // workaround for Amd64 compiler
#endif
//////////////////////////////////////////////////////////////////////

#define DEFAULT_ZMAX	1024.0f
#define DEFAULT_ZMIN	0.25f
#define DEFAULT_FOV		gf_PI/2

//////////////////////////////////////////////////////////////////////

enum {
	FR_PLANE_NEAR,
	FR_PLANE_FAR,
	FR_PLANE_RIGHT,
	FR_PLANE_LEFT,
	FR_PLANE_TOP,
	FR_PLANE_BOTTOM,
	FRUSTUM_PLANES
};

//////////////////////////////////////////////////////////////////////

enum cull {
	CULL_EXCLUSION,		//the whole object is outside of frustum
	CULL_OVERLAP,			//the object &  frustum overlap
	CULL_INCLUSION		//the whole object is inside frustum
};




#define YAW		(0)  
#define PITCH	(1)    
#define ROLL	(2)   


//inline Matrix44	ViewMatrix(const Ang3 &angle);
inline Matrix33	CryViewMatrixYPR(const Ang3 &angle);
inline Matrix33	CryViewMatrix(const Ang3 &angle);
inline Ang3 ConvertToRad( const Ang3& v );


///////////////////////////////////////////////////////////////////////////////
// * CCamera *
//
// Implements essential operations like calculation of a view-matrix and 
// frustum-culling with simple geometric primitives (Point, Sphere, AABB, COBB).  
// All calculation are based on the CRYENGINE coordinate-system<P><P>       
//
// We are using a "right-handed" coordinate systems, where the positive X-Axis points 
// to the right, the positive Y-Axis points away from the viewer and the positive 
// Z-Axis points up. The following illustration shows our coordinate system.<P>
//                                   
//##  z-axis                                  
//##    ^                               
//##    |                               
//##    |   y-axis                   
//##    |  /                         
//##    | /                           
//##    |/                             
//##    +---------------->   x-axis     
// <IMAGE 3DEngine-CameraAxis>
// 
// This same system is also used in MAX. It is not unusual for 3D-APIs like D3D9 or 
// OpenGL to use a different coordinate system.  Currently in D3D9 we use a coordinate system 
// in which the X-Axis points to the right, the Y-Axis points down and the Z-Axis points away 
// from the viewer.<P><P>
//
// To convert from the CryEngine system into D3D9 we are just doing a clockwise rotation of 
// 90 about the X-Axis. This conversion will be moved into renderer.<P><P> 
//
///////////////////////////////////////////////////////////////////////////////
class CCamera
{

private:		

	Vec3	m_Position,m_OldPosition;	

	Vec3	m_GameAnglesDeg;		
	Vec3	m_CameraAnglesRad;		

	float	m_fov;

	Vec3	m_Scale;

	float m_ViewSurfaceX;			//suface width-resolution
	float m_ViewSurfaceZ;			//suface height-resolution
	float	m_ProjectionRatio;	//ratio between width and height of view-surface

	Vec3 m_edge_nlt;		//this is the left/upper vertex of the near-plane
	Vec3 m_edge_plt;		//this is the left/upper vertex of the projection-plane 
	Vec3 m_edge_flt;		//this is the left/upper vertex of the far-clip-plane
	float m_ZMax;				//this is the left/upper vertex of the far-plane

	Plane	m_frustum [FRUSTUM_PLANES]; //
	Plane	m_fp[FRUSTUM_PLANES];	//this are the 6 planes view-frustum in world-space
	Vec3 cltp,crtp,clbp,crbp;		//this are the 4 vertices of the projection-plane in cam-space

	Matrix34	m_VMat;		//this is the "pure" view-matrix. view-vector (0,1,0) = identity		
	Matrix44	m_VCMatD3D9;	//concatenation of view- and conversion matrix			


	Vec3	m_OccPosition;		//Position for calculate occlusions (needed for portals rendering)

public:

	// access to frustum vertices, 
	// in order to make exact frustum test work it needs to be updated from outside 
	// if frustum planes was updated from outside of the camera
	void SetFrustumVertices(Vec3d * arrvVerts)
	{
		clbp = arrvVerts[0];
		cltp = arrvVerts[1];
		crtp = arrvVerts[2];
		crbp = arrvVerts[3];
	}

	Vec3d GetFrustumVertex(int nId) const
	{
		switch(nId)
		{
		case 0:	return clbp;
		case 1:	return cltp;
		case 2:	return crtp;
		case 3:	return crbp;
		}
		assert(0);
		return Vec3d(0,0,0);
	}

	void SetFrustumVertex(int nId, const Vec3d & vVert)
	{
		switch(nId)
		{
		case 0:	clbp = vVert;		return;
		case 1:	cltp = vVert;		return;
		case 2:	crtp = vVert;		return;
		case 3:	crbp = vVert;		return;
		}
		assert(0);
	}

	Vec3 m_vOffset;
	struct IVisArea * m_pPortal; // pointer to portal used to create this camera
	struct ScissorInfo{
		ScissorInfo() { x1=y1=x2=y2=0; }
		unsigned short x1,y1,x2,y2;
	};
	ScissorInfo m_ScissorInfo;
	ScissorInfo m_ScissorInfoParent;



public:
	//! constructor/destructor
	CCamera()
	{

		m_Position(0,0,0);
		m_OldPosition(0,0,0);

		m_GameAnglesDeg(0,0,0);
		m_CameraAnglesRad(0,0,0);

		m_fov=DEFAULT_FOV;

    m_Scale = Vec3(1.0f, 1.0f, 1.0f);

		m_ViewSurfaceX=640.0f;			//suface width-resolution
		m_ViewSurfaceZ=480.0f;			//suface height-resolution

		m_VMat.SetIdentity();
//		m_CMat.SetIdentity33();
	  m_VCMatD3D9.SetIdentity();
		m_edge_nlt.y			=	DEFAULT_ZMIN;		//this is the left/upper vertex of the near-plane
		m_edge_flt.y			=	DEFAULT_ZMAX;		//this is the left/upper vertex of the far-plane
		m_ZMax						=	DEFAULT_ZMAX;		//this is the left/upper vertex of the far-plane
		m_ProjectionRatio	=	m_ViewSurfaceZ/m_ViewSurfaceX;

		for (int k=0;k<FRUSTUM_PLANES;k++)	{		m_frustum[k].Set(Vec3(0,0,0),0);	}

		m_vOffset.Set(0,0,0);
		m_pPortal=0;
		m_OccPosition(0,0,0);
	}

	~CCamera() {}

	//Init the camera
	void	Init(int nWidth,int nHeight,float fFova,float fZMAX,float fProjectionRatio,float fZMIN);
	//update camera matrix and frustum-planes		
	void	Update(int nWidth,int nHeight);

	//////////////////////////////////////////////////////////////////////

	inline void	SetAngle(const Vec3 &newangles)	{ 
		m_GameAnglesDeg=newangles; 
		//get angles to calculate view-matrix. (0,0,0) = neutral position
		m_CameraAnglesRad.x = DEG2RAD(m_GameAnglesDeg.x);
		m_CameraAnglesRad.y = DEG2RAD(m_GameAnglesDeg.y);
		m_CameraAnglesRad.z = DEG2RAD(-m_GameAnglesDeg.z+180.0f);
	}
	inline Ang3 GetAngles()		const { return(m_GameAnglesDeg); }	

//	inline void	SetVCMatrix(const Matrix44& mat) { m_VCMatD3D9=mat; }
	inline const Matrix34	GetVMatrix() const { return(m_VMat); }	
	inline const Matrix44	GetVCMatrixD3D9() const { return(m_VCMatD3D9); }	



	inline void	SetPos(const Vec3 &newpos)	{  m_OldPosition=m_Position;  m_Position=newpos;  m_OccPosition = newpos;	}
	inline const Vec3& GetPos() const { return(m_Position); }
	inline const Vec3& GetPosPrec()	const { return(m_OldPosition); }

	inline void	SetOccPos(const Vec3 &newpos) {   m_OccPosition = newpos;	}
	inline const Vec3& GetOccPos() const { return(m_OccPosition); }

	inline void SetFrustumPlane(int numplane, const Plane	&plane) { m_frustum[numplane] = plane; }
	inline const Plane *GetFrustumPlane(int numplane)	const		{ return(&m_frustum[numplane]); }

	inline void	SetZMin(float zmin) { m_edge_nlt.y=zmin; }
	inline float GetZMin() const { return (m_edge_nlt.y); }

	inline void	SetZClip(float zmax) { m_edge_flt.y=zmax; }
	inline float GetZClip() const { return m_edge_flt.y; }

	inline void	SetZMax(float zmax) { m_ZMax=zmax; }
	inline float GetZMax() const { return m_ZMax; }

	inline Vec3 GetEdgeP() const { return m_edge_plt; }
	inline Vec3 GetEdgeN() const { return m_edge_nlt; }
	inline Vec3 GetEdgeF() const { return m_edge_flt; }

	inline void	SetFov(float fov)	{ m_fov=fov; }
	inline float GetFov() const { return(m_fov); }	

	inline float GetProjRatio() const { return(m_ProjectionRatio); }
	inline void	SetProjRatio(float	fProjectionRatio) { m_ProjectionRatio=fProjectionRatio; }

	inline void	SetScale(const Vec3 &newscale)	{ m_Scale=newscale; }

	inline float GetViewSurfaceX() const { return(m_ViewSurfaceX); }	
	inline float GetViewSurfaceZ() const { return(m_ViewSurfaceZ); }	


	//-----------------------------------------------------------------------------------
	//--------                Frustum-Culling                ----------------------------
	//-----------------------------------------------------------------------------------

	//Check if a point lies within camera's frustum
	bool IsPointVisible(const Vec3 &vPoint) const;


	//sphere-frustum test
	bool IsSphereVisibleFast( const Sphere &s ) const;
	char IsSphereVisible_hierarchical( const Sphere &s, bool *bAllIn ) const; //this is going to be the exact version of sphere-culling


	//AABB-frustum test 
	bool IsAABBVisibleFast( const AABB& aabb ) const;
	bool IsAABBVisible_exact(const AABB& aabb) const;
	char IsAABBVisible_hierarchical( const AABB& aabb, bool *bAllIn ) const;


};



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


/*! Init the camera
@param	nWidth	Screen width
@param	nHeight Screen height
@param	fFova		Fov angle, in radians
@param	fZMAX		camera's ZMax
@param	fProjectionRatio	set a custom projection ratio (should not be used)
@param	fZMIN		camera's ZMin
*/
inline void	CCamera::Init(int nWidth,int nHeight,float fFova=DEFAULT_FOV,float fZMAX=DEFAULT_ZMAX,float fProjectionRatio=0,float fZMIN=DEFAULT_ZMIN)
{	
	assert (fZMIN>0.01f); //check if near-plane is valid
	assert (fZMAX>0.01f); //check if far-plane is valid
	assert (fZMAX>fZMIN); //check if far-plane bigger then near-plane

	m_ViewSurfaceX	=(float)(nWidth);		//suface x-resolution
	m_ViewSurfaceZ	=(float)(nHeight);	//suface z-resolution

	m_fov						= fFova;

	m_edge_nlt.y		=	fZMIN; 
	m_edge_flt.y		=	fZMAX; 
	m_ZMax					= fZMAX;	  

	// projection ratio (1.0 for square pixels)
	m_ProjectionRatio = m_ViewSurfaceZ/m_ViewSurfaceX;		
  if (fProjectionRatio)	m_ProjectionRatio = fProjectionRatio;
}



/*!
 *
 *  Updates all parameters required by the render-engine:
 *
 *	3d-view-frustum and all matrices
 *
 */
inline void CCamera::Update(int nWidth=-1,int nHeight=-1) {

	if (nWidth  != -1) m_ViewSurfaceX=(float)(nWidth);		//suface x-resolution
	if (nHeight != -1) m_ViewSurfaceZ=(float)(nHeight);	//suface z-resolution

	assert (m_edge_nlt.y>0.009f);	//check if near-plane is valid
	assert (m_ZMax>0.01f);				//check if far-plane is valid
	assert (m_ZMax>m_edge_nlt.y); //check if far-plane bigger then near-plane

	Ang3 ca							= ConvertToRad(m_GameAnglesDeg);  
	Matrix34 t					=	Matrix34::CreateTranslationMat(-m_Position);
	Matrix33diag diag		=	m_Scale;	
	Matrix33 v					=	CryViewMatrix(ca);  //with this approach its impossible to do a roll
	m_VMat							=	v*diag*t;	

	//concatenate the conversion-matrix with the view-matrix
	m_VCMatD3D9	=	Matrix33::CreateRotationX( -gf_PI/2 ) * m_VMat;
	m_VCMatD3D9	=	GetTransposed44(m_VCMatD3D9);		//TODO: remove this after E3 and use Matrix34 instead of Matrix44

	//---------------------------------------------------------------------------------------------------

	//calculate the Left/Top edge of the Projection plane (relative to camerapos (0,0,0) and not rotated) 
	m_edge_plt.x	=-m_ViewSurfaceX*0.5f; 
	m_edge_plt.y	= cry_cosf(m_fov*m_ProjectionRatio*0.5f) / cry_sinf(m_fov*m_ProjectionRatio*0.5f) * m_ViewSurfaceZ*0.50f;
	m_edge_plt.z	= m_ViewSurfaceZ*0.5f;

	//	m_edge_plt.x	+=100;
	//	m_edge_plt.z	-=100;

	//calculate the left/upper edge of the near-plane (=not rotated)  
	//the depth-value is set by Init(), thus all we need is x and z 
	m_edge_nlt.x	= (m_edge_nlt.y/m_edge_plt.y)*m_edge_plt.x;   
	m_edge_nlt.z	= (m_edge_nlt.y/m_edge_plt.y)*m_edge_plt.z;  

	//calculate the left/upper edge of the far-clip-plane (=not rotated) 
	//the depth-value is set by Init(), thus all we need is x and z 
	m_edge_flt.x	= (m_edge_flt.y/m_edge_plt.y)*m_edge_plt.x;	 
	m_edge_flt.z	= (m_edge_flt.y/m_edge_plt.y)*m_edge_plt.z;  



	//we get the real-frustum edges, if we multiply the not-rotated edges by the inverse VMat
	//note: the rotated frustum-eges are always in camera-space.
	Matrix34 iVMat=m_VMat.GetInverted();

	Vec3 ltp,rtp,lbp,rbp;	//this are the 4 vertices of the projection-plane in cam-space
	Vec3 ltn,rtn,lbn,rbn;	//this are the 4 vertices of the near-plane in cam-space
	Vec3 ltf,rtf,lbf,rbf;	//this are the 4 vertices of the far-clip-plane in cam-space


	//-----------------------------------------------------
	//--- calculate frustum-edges of projection-plane   ---
	//-----------------------------------------------------
	ltp=iVMat*Vec3(+m_edge_plt.x,+m_edge_plt.y,+m_edge_plt.z);
	rtp=iVMat*Vec3(-m_edge_plt.x,+m_edge_plt.y,+m_edge_plt.z);
	lbp=iVMat*Vec3(+m_edge_plt.x,+m_edge_plt.y,-m_edge_plt.z);
	rbp=iVMat*Vec3(-m_edge_plt.x,+m_edge_plt.y,-m_edge_plt.z);

	//-----------------------------------------------------
	//--- calculate frustum-edges of projection-plane   ---
	//-----------------------------------------------------
	cltp=Vec3(+m_edge_plt.x,+m_edge_plt.y,+m_edge_plt.z)*Matrix33(m_VMat);
	crtp=Vec3(-m_edge_plt.x,+m_edge_plt.y,+m_edge_plt.z)*Matrix33(m_VMat);
	clbp=Vec3(+m_edge_plt.x,+m_edge_plt.y,-m_edge_plt.z)*Matrix33(m_VMat);
	crbp=Vec3(-m_edge_plt.x,+m_edge_plt.y,-m_edge_plt.z)*Matrix33(m_VMat);

	//-----------------------------------------------------
	//---     calculate frustum-edges of near-plane     ---
	//-----------------------------------------------------
	ltn=iVMat*Vec3(+m_edge_nlt.x,+m_edge_nlt.y,+m_edge_nlt.z);
	rtn=iVMat*Vec3(-m_edge_nlt.x,+m_edge_nlt.y,+m_edge_nlt.z);
	lbn=iVMat*Vec3(+m_edge_nlt.x,+m_edge_nlt.y,-m_edge_nlt.z);
	rbn=iVMat*Vec3(-m_edge_nlt.x,+m_edge_nlt.y,-m_edge_nlt.z);

	//-----------------------------------------------------
	//---  calculate frustum-edges of far-clip-plane    ---
	//-----------------------------------------------------
	ltf=iVMat*Vec3(+m_edge_flt.x,+m_edge_flt.y,+m_edge_flt.z);
	rtf=iVMat*Vec3(-m_edge_flt.x,+m_edge_flt.y,+m_edge_flt.z);
	lbf=iVMat*Vec3(+m_edge_flt.x,+m_edge_flt.y,-m_edge_flt.z);
	rbf=iVMat*Vec3(-m_edge_flt.x,+m_edge_flt.y,-m_edge_flt.z);



	//-------------------------------------------------------------------------
	//---  calculate the six frustum-planes using the rotated fustum edges  ---
	//-------------------------------------------------------------------------
	m_fp[FR_PLANE_NEAR  ]	=	GetPlane( ltn,rtn,rbn );
	m_fp[FR_PLANE_RIGHT ]	=	GetPlane( rtf,rbf, m_Position );
	m_fp[FR_PLANE_LEFT  ]	=	GetPlane( lbf,ltf, m_Position );
	m_fp[FR_PLANE_TOP   ]	=	GetPlane( ltf,rtf, m_Position );
	m_fp[FR_PLANE_BOTTOM]	=	GetPlane( rbf,lbf, m_Position );
	m_fp[FR_PLANE_FAR   ]	=	GetPlane( rbf,rtf,ltf );  //clip-plane

//maybe this makes a problem (but I dont think so.....)
//here I'm just overwriting the old frustum calculation to fix the frustum bug  
if (1) {
	m_frustum[FR_PLANE_NEAR  ]	=	m_fp[FR_PLANE_NEAR];		m_frustum[FR_PLANE_NEAR  ].d *=-1;
	m_frustum[FR_PLANE_LEFT  ]	=	m_fp[FR_PLANE_LEFT];		m_frustum[FR_PLANE_LEFT  ].d *=-1;
	m_frustum[FR_PLANE_RIGHT ]	=	m_fp[FR_PLANE_RIGHT];		m_frustum[FR_PLANE_RIGHT ].d *=-1;
	m_frustum[FR_PLANE_TOP   ]	=	m_fp[FR_PLANE_TOP];			m_frustum[FR_PLANE_TOP   ].d *=-1;
	m_frustum[FR_PLANE_BOTTOM]	=	m_fp[FR_PLANE_BOTTOM];	m_frustum[FR_PLANE_BOTTOM].d *=-1;
	m_frustum[FR_PLANE_FAR   ]	=	m_fp[FR_PLANE_FAR];			m_frustum[FR_PLANE_FAR   ].d *=-1;
}

}



/*!
* Check if a point lies within camera's frustum
*
* Example:
*  unsigned char InOut=camera.IsPointVisible(point);
*
* return values:
*  CULL_EXCLUSION = point outside of frustum      
*  CULL_INTERSECT = point inside of frustum
*/
inline bool	CCamera::IsPointVisible(const Vec3 &p) const {
	if ((m_fp[FR_PLANE_NEAR  ]|p) > 0) return CULL_EXCLUSION;
	if ((m_fp[FR_PLANE_RIGHT ]|p) > 0) return CULL_EXCLUSION;
	if ((m_fp[FR_PLANE_LEFT  ]|p) > 0) return CULL_EXCLUSION;
	if ((m_fp[FR_PLANE_TOP   ]|p) > 0) return CULL_EXCLUSION;
	if ((m_fp[FR_PLANE_BOTTOM]|p) > 0) return CULL_EXCLUSION;
	if ((m_fp[FR_PLANE_FAR   ]|p) > 0) return CULL_EXCLUSION;
	return CULL_OVERLAP;
}







/*!
* Conventional method to check if a sphere and the camera-frustum intersect 
* The center of the sphere is assumed to be in world-space.
*
* Example:
*  unsigned char InOut=camera.IsSphereVisibleFast(sphere);
*
* return values:
*  CULL_EXCLUSION = sphere outside of frustum (very fast rejection-test)      
*  CULL_OVERLAP = sphere and frustum intersects or sphere in completely inside frustum
*/
inline bool	CCamera::IsSphereVisibleFast( const Sphere &s ) const
{
	float dist;
	Plane *frus_ptr=(Plane *)m_frustum;

	for (int i=0;i<FRUSTUM_PLANES;i++,frus_ptr++) 	
	{
		dist=frus_ptr->DistFromPlane(s.center);
		if (dist>s.radius) 
			return (false);
	}

	return (true);	
}


/*!
* Conventional method to check if a sphere and the camera-frustum overlap 
* The center of the sphere is assumed to be in world-space.
* NOTE: even if the sphere is totally inside the frustum, this function returns CULL_INTERSECT 
* For hirarchical frustum-culling this function is not perfect.
*
* Example:
*  unsigned char InOut=camera.IsSphereVisibleFast(sphere);
*
* return values:
*  CULL_EXCLUSION = sphere outside of frustum (very fast rejection-test)      
*  CULL_INTERSECT = sphere and frustum intersects or sphere in completely inside frustum
*/
/*__forceinline bool CCamera::IsSphereVisibleFast( const Sphere &s ) const {
	if ((m_fp[FR_PLANE_NEAR  ]|s.center) > s.radius) return CULL_EXCLUSION;
	if ((m_fp[FR_PLANE_LEFT  ]|s.center) > s.radius) return CULL_EXCLUSION;
	if ((m_fp[FR_PLANE_RIGHT ]|s.center) > s.radius) return CULL_EXCLUSION;
	if ((m_fp[FR_PLANE_TOP   ]|s.center) > s.radius) return CULL_EXCLUSION;
	if ((m_fp[FR_PLANE_BOTTOM]|s.center) > s.radius) return CULL_EXCLUSION;
	if ((m_fp[FR_PLANE_FAR   ]|s.center) > s.radius) return CULL_EXCLUSION;
	return CULL_OVERLAP;
}*/




/*!
 *
 * conventional method to check if a sphere and the camera-frustum intersect, 
 * or if the sphere is completely inside the camera-frustum.
 *	The center of Sphere is assumed to be in world-space.
 *
 * Example:
 *  unsigned char InOut=camera.IsSphereVisible_hierarchical(sphere);
 *
 * return values:
 *  CULL_EXCLUSION   = sphere outside of frustum (very fast rejection-test)      
 *  CULL_INTERSECT   = sphere intersects the borders of the frustum, further checks necessary
 *  CULL_INCLUSION   = sphere is complete inside the frustum, no further checks necessary
 */
__forceinline char CCamera::IsSphereVisible_hierarchical( const Sphere &s, bool *bAllIn=0 ) const {
	float _nc,_rc,_lc,_tc,_bc,_cc;

	if (bAllIn)	*bAllIn=false;
	if ((_nc=m_fp[FR_PLANE_NEAR  ]|s.center)>s.radius) return CULL_EXCLUSION;
	if ((_rc=m_fp[FR_PLANE_RIGHT ]|s.center)>s.radius) return CULL_EXCLUSION;
	if ((_lc=m_fp[FR_PLANE_LEFT  ]|s.center)>s.radius) return CULL_EXCLUSION;
	if ((_tc=m_fp[FR_PLANE_TOP   ]|s.center)>s.radius) return CULL_EXCLUSION;
	if ((_bc=m_fp[FR_PLANE_BOTTOM]|s.center)>s.radius) return CULL_EXCLUSION;
	if ((_cc=m_fp[FR_PLANE_FAR   ]|s.center)>s.radius) return CULL_EXCLUSION;

	//now we have to check if it is completely in frustum
	bool nc	=	(_nc<=(-s.radius));
	bool lc	=	(_lc<=(-s.radius));
	bool rc	=	(_rc<=(-s.radius));
	bool tc	=	(_tc<=(-s.radius));
	bool bc	=	(_bc<=(-s.radius));
	bool cc	=	(_cc<=(-s.radius));
	if (tc&lc&rc&bc&nc&cc)	{ 
			if (bAllIn)	*bAllIn=true;
			return CULL_INCLUSION;
		}

	return CULL_OVERLAP;

}




extern char BoxSides[0x40*8];


/*!
* Very fast approach to check if an AABB and the camera-frustum overlap, or if the AABB 
* is totally inside the camera-frustum. The bounding-box of the AABB is assumed to be 
* in world-space. This test can reject even such AABBs that overlap a frustum-plane far 
* outside the view-frustum. 
* IMPORTANT: this function is only usefull if you really need hierachical-culling. 
* It is about 30% slower then "IsAABBVisibleFast(aabb)"   
*
* Example:
*  int InOut=camera.IsAABBVisible_hierarchical(aabb);
*
* return values:
*  CULL_EXCLUSION   = AABB outside of frustum (very fast rejection-test)      
*  CULL_OVERLAP     = AABB intersects the borders of the frustum, further checks necessary
*/
__forceinline bool CCamera::IsAABBVisibleFast( const AABB& aabb ) const
{ 
	float d;
	const Vec3* pAABB=&aabb.min;
	for (int i=0;i<FRUSTUM_PLANES;i++) 	  
	{				
/*
    d	=-m_frustum[i].d;		
		d += m_frustum[i].n.x * pAABB[(*((uint32*)&m_frustum[i].n.x)>>31)].x;
		d += m_frustum[i].n.y * pAABB[(*((uint32*)&m_frustum[i].n.y)>>31)].y;
		d += m_frustum[i].n.z * pAABB[(*((uint32*)&m_frustum[i].n.z)>>31)].z;
		if (d>0) return CULL_EXCLUSION;		
*/
 
		d=-m_frustum[i].d;		
		if (m_frustum[i].n.x>=0)	d+=m_frustum[i].n.x*aabb.min.x;
		else d+=m_frustum[i].n.x*aabb.max.x;					
		if (m_frustum[i].n.y>=0)	d+=m_frustum[i].n.y*aabb.min.y; 						
		else d+=m_frustum[i].n.y*aabb.max.y;
		if (m_frustum[i].n.z>=0)	d+=m_frustum[i].n.z*aabb.min.z; 
		else d+=m_frustum[i].n.z*aabb.max.z;		
		if (d>0) return CULL_EXCLUSION;

	}
	return CULL_OVERLAP;	  
}



/*!
* This function checks if an AABB and the camera-frustum overlap. 
* The bounding-box of the AABB is assumed to be in world-space. This test can reject 
* even such AABBs that overlap a frustum-plane far outside the view-frustum. 
* IMPORTANT: It is about 10% slower then "IsAABBVisibleFast(aabb)"   
*
* Example:
*  int InOut=camera.IsAABBVisible_exact(aabb);
*
* return values:
*  CULL_EXCLUSION   = AABB outside of frustum (very fast rejection-test)      
*  CULL_OVERLAP     = AABB intersects the borders of the frustum or is totally inside
*/
inline bool CCamera::IsAABBVisible_exact(const AABB& aabb) const
{
	float d;
	const Vec3* pAABB=&aabb.min;

	//------------------------------------------------------------------------------
	//---  loop over all 6 frustum-planes and do an early rejection the AABB is  --- 
	//---       completely on the positive side of any one of the 6 planes       ---
	//------------------------------------------------------------------------------
	for (int i=0; i<FRUSTUM_PLANES; i++) 	
	{				
/*
		d=-m_frustum[i].d;		
		d += m_frustum[i].n.x * pAABB[(*((uint32*)&m_frustum[i].n.x)>>31)].x;
		d += m_frustum[i].n.y * pAABB[(*((uint32*)&m_frustum[i].n.y)>>31)].y;
		d += m_frustum[i].n.z * pAABB[(*((uint32*)&m_frustum[i].n.z)>>31)].z;
		if (d>0) return CULL_EXCLUSION;		
*/
		d=-m_frustum[i].d;		
		if (m_frustum[i].n.x>=0)	d+=m_frustum[i].n.x*aabb.min.x;
		else d+=m_frustum[i].n.x*aabb.max.x;					
		if (m_frustum[i].n.y>=0)	d+=m_frustum[i].n.y*aabb.min.y; 						
		else d+=m_frustum[i].n.y*aabb.max.y;
		if (m_frustum[i].n.z>=0)	d+=m_frustum[i].n.z*aabb.min.z; 
		else d+=m_frustum[i].n.z*aabb.max.z;		
		if (d>0) return CULL_EXCLUSION;
		

	}

	Vec3 p = (aabb.min+aabb.max)*0.5f;

	if ( (m_frustum[FR_PLANE_NEAR  ].DistFromPlane(p) ) < 0) {
		if ( (m_frustum[FR_PLANE_RIGHT ].DistFromPlane(p) ) < 0) {
			if ( (m_frustum[FR_PLANE_LEFT  ].DistFromPlane(p) ) < 0) {
				if ( (m_frustum[FR_PLANE_TOP   ].DistFromPlane(p) ) < 0) {
					if ( (m_frustum[FR_PLANE_BOTTOM].DistFromPlane(p) ) < 0) {
						if ( (m_frustum[FR_PLANE_FAR   ].DistFromPlane(p) ) < 0) {
							return CULL_OVERLAP; //AABB is patially visible
						}
					}
				}
			}
		}
	}
	//------------------------------------------------------------------------------
	//---                         ADDITIONAL-TEST                                ---
	//---      a box can easily straddle one of the view-frustum planes far      ---
	//---   outside the view-frustum and in this case the previous test would    ---
	//---                       return CULL_OVERLAP                              ---
	//------------------------------------------------------------------------------
	//-----    With this check, we make sure the AABB is really not visble    ------
	//------------------------------------------------------------------------------

	AABB caabb(aabb.min-m_Position,aabb.max-m_Position);  //caabb in camera-space

	unsigned long front=0;
	if (caabb.min.x>0.0f)  front|=0x01;
	if (caabb.max.x<0.0f)  front|=0x02;
	if (caabb.min.y>0.0f)  front|=0x04;
	if (caabb.max.y<0.0f)  front|=0x08;
	if (caabb.min.z>0.0f)  front|=0x10;
	if (caabb.max.z<0.0f)  front|=0x20;

	//check if camera is inside the aabb
	if (front==0)	return CULL_OVERLAP; //AABB is patially visible

	Vec3 v[8];
	Vec3 s0,s1,s2,s3,s4,s5;	//caabb-side-normals (....but not normalized)

	v[0] =	Vec3(caabb.min.x,caabb.min.y,caabb.min.z);
	v[1] =	Vec3(caabb.max.x,caabb.min.y,caabb.min.z);
	v[2] =	Vec3(caabb.min.x,caabb.max.y,caabb.min.z);
	v[3] =	Vec3(caabb.max.x,caabb.max.y,caabb.min.z);
	v[4] =	Vec3(caabb.min.x,caabb.min.y,caabb.max.z);
	v[5] =	Vec3(caabb.max.x,caabb.min.y,caabb.max.z);
	v[6] =	Vec3(caabb.min.x,caabb.max.y,caabb.max.z);
	v[7] =  Vec3(caabb.max.x,caabb.max.y,caabb.max.z);
	//---------------------------------------------------------------------
	//---            find the silhouette-vertices of the AABB            ---
	//---------------------------------------------------------------------
	unsigned long p0,p1,p2,p3,p4,p5,sideamount;
	p0	=	BoxSides[(front<<3)+0];
	p1	=	BoxSides[(front<<3)+1];
	p2	=	BoxSides[(front<<3)+2];
	p3	=	BoxSides[(front<<3)+3];
	p4	=	BoxSides[(front<<3)+4];
	p5	=	BoxSides[(front<<3)+5];
	sideamount=BoxSides[(front<<3)+7];

	if(sideamount==4) {
		//--------------------------------------------------------------------------
		//---                calculate the 4 "planes" for the AABB                ---
		//--------------------------------------------------------------------------
		s0	=	v[p0] % v[p1];
		s1	=	v[p1] % v[p2];
		s2	=	v[p2] % v[p3];
		s3	=	v[p3] % v[p0];
		//--------------------------------------------------------------------------
		//---     we take the 4 vertices of projection-plane in cam-space,       ---
		//-----  and clip them against the 4 side-frustum-planes of the AABB        -
		//--------------------------------------------------------------------------
		if (!(  ((s0|cltp)<=0.0f)|((s0|crtp)<=0.0f)|((s0|crbp)<=0.0f)|((s0|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s1|cltp)<=0.0f)|((s1|crtp)<=0.0f)|((s1|crbp)<=0.0f)|((s1|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s2|cltp)<=0.0f)|((s2|crtp)<=0.0f)|((s2|crbp)<=0.0f)|((s2|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s3|cltp)<=0.0f)|((s3|crtp)<=0.0f)|((s3|crbp)<=0.0f)|((s3|clbp)<=0.0f)  )) return CULL_EXCLUSION;
	}

	if(sideamount==6) {
		//--------------------------------------------------------------------------
		//---               calculate the 6 "planes" for the AABB                 ---
		//--------------------------------------------------------------------------
		s0=v[p0]%v[p1];
		s1=v[p1]%v[p2];
		s2=v[p2]%v[p3];
		s3=v[p3]%v[p4];
		s4=v[p4]%v[p5];
		s5=v[p5]%v[p0];
		//--------------------------------------------------------------------------
		//---     we take the 4 vertices of projection-plane in cam-space,       ---
		//---    and clip them against the 6 side-frustum-planes of the AABB      ---
		//--------------------------------------------------------------------------
		if (!(  ((s0|cltp)<=0.0f)|((s0|crtp)<=0.0f)|((s0|crbp)<=0.0f)|((s0|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s1|cltp)<=0.0f)|((s1|crtp)<=0.0f)|((s1|crbp)<=0.0f)|((s1|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s2|cltp)<=0.0f)|((s2|crtp)<=0.0f)|((s2|crbp)<=0.0f)|((s2|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s3|cltp)<=0.0f)|((s3|crtp)<=0.0f)|((s3|crbp)<=0.0f)|((s3|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s4|cltp)<=0.0f)|((s4|crtp)<=0.0f)|((s4|crbp)<=0.0f)|((s4|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s5|cltp)<=0.0f)|((s5|crtp)<=0.0f)|((s5|crbp)<=0.0f)|((s5|clbp)<=0.0f)  )) return CULL_EXCLUSION;
	}

	return CULL_OVERLAP; //AABB is patially visible

}






/*!
* Improved approach to check if an AABB and the camera-frustum overlap, or if the AABB 
* is totally inside the camera-frustum. The bounding-box of the AABB is assumed to be 
* in world-space. This test can reject even such AABBs that overlap a frustum-plane far 
* outside the view-frustum. 
* IMPORTANT: this function is only usefull if you really need hierarchical-culling. 
* It is about 30% slower then "IsAABBVisibleFast(aabb)"   
*
* Example:
*  int InOut=camera.IsAABBVisible_hierarchical(aabb);
*
* return values:
*  CULL_EXCLUSION   = AABB outside of frustum (very fast rejection-test)      
*  CULL_OVERLAP     = AABB intersects the borders of the frustum, further checks necessary
*  CULL_INCLUSION   = AABB is complete inside the frustum, no further checks necessary
*/
inline char CCamera::IsAABBVisible_hierarchical(const AABB& aabb, bool *bAllIn=0 ) const
{

/*
	float dot1,dot2;
	unsigned long notOverlap = 0x80000000; // will be reset to 0 if there's at least one overlapping
	const Vec3* pAABB=&aabb.min;
	if (bAllIn)	*bAllIn=false;

	//------------------------------------------------------------------------------
	//---  loop over all 6 frustum-planes and do an early rejection the AABB is  --- 
	//---       completely on the positive side of any one of the 6 planes       ---
	//------------------------------------------------------------------------------
	for (int i=0;i<FRUSTUM_PLANES;i++) 	
	{				
		dot1=dot2=-m_frustum[i].d;		
		dot1 += m_frustum[i].n.x * pAABB[0+(*((uint32*)&m_frustum[i].n.x)>>31)].x;
		dot2 += m_frustum[i].n.x * pAABB[1-(*((uint32*)&m_frustum[i].n.x)>>31)].x;
		dot1 += m_frustum[i].n.y * pAABB[0+(*((uint32*)&m_frustum[i].n.y)>>31)].y;
		dot2 += m_frustum[i].n.y * pAABB[1-(*((uint32*)&m_frustum[i].n.y)>>31)].y;
		dot1 += m_frustum[i].n.z * pAABB[0+(*((uint32*)&m_frustum[i].n.z)>>31)].z;
		dot2 += m_frustum[i].n.z * pAABB[1-(*((uint32*)&m_frustum[i].n.z)>>31)].z;
		if ( !(((int32&)dot1)&0x80000000) ) return CULL_EXCLUSION;		
		notOverlap &= (int32&)dot2;
	}

	if (notOverlap) {
		if (bAllIn)	*bAllIn=true;
		return CULL_INCLUSION; 
	}*/




	float dot1,dot2;
	unsigned long overlap=false;
	if (bAllIn)	*bAllIn=false;

	//------------------------------------------------------------------------------
	//---  loop over all 6 frustum-planes and do an early rejection the AABB is  --- 
	//---       completely on the positive side of any one of the 6 planes       ---
	//------------------------------------------------------------------------------
	for (int i=0;i<FRUSTUM_PLANES;i++) 	
	{				
		dot1=dot2=-m_frustum[i].d;		

		if (m_frustum[i].n.x>=0)	{	dot1+=m_frustum[i].n.x*aabb.min.x;	dot2+=m_frustum[i].n.x*aabb.max.x;	}			
		else	{	dot1+=m_frustum[i].n.x*aabb.max.x;	dot2+=m_frustum[i].n.x*aabb.min.x;	}

		if (m_frustum[i].n.y>=0) {	dot1+=m_frustum[i].n.y*aabb.min.y;	dot2+=m_frustum[i].n.y*aabb.max.y;	}			
		else	{	dot1+=m_frustum[i].n.y*aabb.max.y;	dot2+=m_frustum[i].n.y*aabb.min.y;	}

		if (m_frustum[i].n.z>=0)	{	dot1+=m_frustum[i].n.z*aabb.min.z;	dot2+=m_frustum[i].n.z*aabb.max.z;	}	
		else	{	dot1+=m_frustum[i].n.z*aabb.max.z;	dot2+=m_frustum[i].n.z*aabb.min.z;	}

		if (dot1>0)	return CULL_EXCLUSION;		
		if (dot2>0)	overlap=true;
	}

	if (!overlap) {
		if (bAllIn)	*bAllIn=true;
		return CULL_INCLUSION; 
	}

	//------------------------------------------------------------------------------
	//---                         ADDITIONAL-TEST                                ---
	//---      a box can easily straddle one of the view-frustum planes far      ---
	//---   outside the view-frustum and in this case the previous test would    ---
	//---                       return CULL_OVERLAP                              ---
	//------------------------------------------------------------------------------
	//-----    With this check, we make sure the AABB is really not visble    ------
	//------------------------------------------------------------------------------
	AABB AABB(aabb.min-m_Position,aabb.max-m_Position);  //AABB in camera-space

	unsigned long frontx8=0; // make the flags using the fact that the upper bit in float is its sign
	frontx8 |= (unsigned(-(int&)AABB.min.x)>>5)&(0x01<<26); //if (AABB.min.x>0.0f)  frontx8|=0x01;
	frontx8 |= (unsigned( (int&)AABB.max.x)>>4)&(0x02<<26); //if (AABB.max.x<0.0f)  frontx8|=0x02;
	frontx8 |= (unsigned(-(int&)AABB.min.y)>>3)&(0x04<<26); //if (AABB.min.y>0.0f)  frontx8|=0x04;
	frontx8 |= (unsigned( (int&)AABB.max.y)>>2)&(0x08<<26); //if (AABB.max.y<0.0f)  frontx8|=0x08;
	frontx8 |= (unsigned(-(int&)AABB.min.z)>>1)&(0x10<<26); // if (AABB.min.z>0.0f)  frontx8|=0x10;
	frontx8 |= (unsigned( (int&)AABB.max.z))&(0x20<<26); // if (AABB.max.z<0.0f)  frontx8|=0x20;
	//frontx8 |= (frontx8 + frontx8) ^ 0x2A;
	frontx8 = (frontx8>>23) & (0x3F << 3);

	//check if camera is inside the aabb
	if (frontx8==0)	return CULL_OVERLAP; //AABB is patially visible

	Vec3 v[8];
	Vec3 s0,s1,s2,s3,s4,s5;	//AABB-side-normals (....but not normalized)

	v[0] =	Vec3(AABB.min.x,AABB.min.y,AABB.min.z);
	v[1] =	Vec3(AABB.max.x,AABB.min.y,AABB.min.z);
	v[2] =	Vec3(AABB.min.x,AABB.max.y,AABB.min.z);
	v[3] =	Vec3(AABB.max.x,AABB.max.y,AABB.min.z);
	v[4] =	Vec3(AABB.min.x,AABB.min.y,AABB.max.z);
	v[5] =	Vec3(AABB.max.x,AABB.min.y,AABB.max.z);
	v[6] =	Vec3(AABB.min.x,AABB.max.y,AABB.max.z);
	v[7] =  Vec3(AABB.max.x,AABB.max.y,AABB.max.z);
	//---------------------------------------------------------------------
	//---            find the silhouette-vertices of the AABB            ---
	//---------------------------------------------------------------------
	unsigned long p0,p1,p2,p3,p4,p5,sideamount;
	p0	=	BoxSides[frontx8+0];
	p1	=	BoxSides[frontx8+1];
	p2	=	BoxSides[frontx8+2];
	p3	=	BoxSides[frontx8+3];
	p4	=	BoxSides[frontx8+4];
	p5	=	BoxSides[frontx8+5];
	sideamount=BoxSides[frontx8+7];


	if(sideamount==4) {
		//--------------------------------------------------------------------------
		//---                calculate the 4 "planes" for the AABB                ---
		//--------------------------------------------------------------------------
		s0	=	v[p0] % v[p1];
		s1	=	v[p1] % v[p2];
		s2	=	v[p2] % v[p3];
		s3	=	v[p3] % v[p0];
		//--------------------------------------------------------------------------
		//---     we take the 4 vertices of projection-plane in cam-space,       ---
		//-----  and clip them against the 4 side-frustum-planes of the AABB        -
		//--------------------------------------------------------------------------
		if (!(  ((s0|cltp)<=0.0f)|((s0|crtp)<=0.0f)|((s0|crbp)<=0.0f)|((s0|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s1|cltp)<=0.0f)|((s1|crtp)<=0.0f)|((s1|crbp)<=0.0f)|((s1|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s2|cltp)<=0.0f)|((s2|crtp)<=0.0f)|((s2|crbp)<=0.0f)|((s2|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s3|cltp)<=0.0f)|((s3|crtp)<=0.0f)|((s3|crbp)<=0.0f)|((s3|clbp)<=0.0f)  )) return CULL_EXCLUSION;
	}

	if(sideamount==6) {
		//--------------------------------------------------------------------------
		//---               calculate the 6 "planes" for the AABB                 ---
		//--------------------------------------------------------------------------
		s0=v[p0]%v[p1];
		s1=v[p1]%v[p2];
		s2=v[p2]%v[p3];
		s3=v[p3]%v[p4];
		s4=v[p4]%v[p5];
		s5=v[p5]%v[p0];
		//--------------------------------------------------------------------------
		//---     we take the 4 vertices of projection-plane in cam-space,       ---
		//---    and clip them against the 6 side-frustum-planes of the AABB      ---
		//--------------------------------------------------------------------------
		if (!(  ((s0|cltp)<=0.0f)|((s0|crtp)<=0.0f)|((s0|crbp)<=0.0f)|((s0|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s1|cltp)<=0.0f)|((s1|crtp)<=0.0f)|((s1|crbp)<=0.0f)|((s1|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s2|cltp)<=0.0f)|((s2|crtp)<=0.0f)|((s2|crbp)<=0.0f)|((s2|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s3|cltp)<=0.0f)|((s3|crtp)<=0.0f)|((s3|crbp)<=0.0f)|((s3|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s4|cltp)<=0.0f)|((s4|crtp)<=0.0f)|((s4|crbp)<=0.0f)|((s4|clbp)<=0.0f)  )) return CULL_EXCLUSION;
		if (!(  ((s5|cltp)<=0.0f)|((s5|crtp)<=0.0f)|((s5|crbp)<=0.0f)|((s5|clbp)<=0.0f)  )) return CULL_EXCLUSION;
	}

	return CULL_OVERLAP; //AABB is patially visible
}














////////////////////////////////////////////////////////////////		
//! convert from degrees to radians and adjust the coordinate system
////////////////////////////////////////////////////////////////		
inline Ang3 ConvertToRad( const Ang3& v ){
	Ang3 angles;
	angles.x=DEG2RAD( v.z+180.0f);
	angles.y=DEG2RAD(-v.x+90.0f);
	angles.z=DEG2RAD( v.y);
	return angles; 
}


////////////////////////////////////////////////////////////////		
//! convert a view angle from degrees to a normalized view-vector
////////////////////////////////////////////////////////////////		
inline Vec3 ConvertToRadAngles( const Ang3& v )	{	

	Vec3 vec=ConvertToRad(v);	

	Vec3 dir;
	dir.x=-sin_tpl(vec.y)*sin_tpl(vec.x);
	dir.y= sin_tpl(vec.y)*cos_tpl(vec.x);
	dir.z=-cos_tpl(vec.y);			
	return dir;
}	

inline Matrix44	ViewMatrix(const Ang3 &angle)	{
	Matrix33 ViewMatZ=Matrix33::CreateRotationZ(-angle.x);
	Matrix33 ViewMatX=Matrix33::CreateRotationX(-angle.y);
	Matrix33 ViewMatY=Matrix33::CreateRotationY(+angle.z);
	return GetTransposed44( ViewMatX*ViewMatY*ViewMatZ);
}

//ZXY
inline Matrix33	CryViewMatrix(const Ang3 &angle)	{
	Matrix33 ViewMatZ=Matrix33::CreateRotationZ(-angle.x);
	Matrix33 ViewMatX=Matrix33::CreateRotationX(-angle.y);
	Matrix33 ViewMatY=Matrix33::CreateRotationY(+angle.z);
	return  Matrix33::CreateRotationX(gf_PI*0.5f)*ViewMatX*ViewMatY*ViewMatZ;
}


/*! convert a lenght unit vector to camera's angles:
x camera axis is looking up/down
y is roll
z is left/right	
*/
inline Vec3 ConvertUnitVectorToCameraAngles(const Vec3& vec)	{

	Vec3 v=vec;

	float	fForward;
	float	fYaw,fPitch;

	if (v.y==0 && v.x==0) 
	{
		//looking up/down
		fYaw=0;
		if (v.z>0) 			
			fPitch=90;			
		else 			
			fPitch=270;	
	} 
	else 
	{
		if (v.x!=0) 
		{
			fYaw=(float)(cry_atan2f((float)(v.y),(float)(v.x))*180.0f/gf_PI);
		}
		else 
			//lokking left/right	
			if (v.y>0) 							
				fYaw=90;			
			else 			
				fYaw=270;			

			if (fYaw<0) 			
				fYaw+=360;			

			fForward=(float)cry_sqrtf(v.x*v.x+v.y*v.y);
			fPitch=(float)(cry_atan2f(v.z,fForward)*180.0f/gf_PI);
			if (fPitch<0) 
				fPitch+=360;			
	}

	//y = -fPitch;
	//x = fYaw;
	//z = 0;
	v.x=-fPitch;
	v.y=0; 
	v.z=fYaw+90;

	//clamp again
	if (v.x>360)	v.x-=360;else
		if (v.x<-360) v.x+=360;
	if (v.z>360)	v.z-=360;else
		if (v.z<-360) v.z+=360;

	return v;

}

/*! convert a vector to camera's angles:
x camera axis is looking up/down
y is roll
z is left/right	
*/
inline Vec3 ConvertVectorToCameraAngles(const Vec3& v) {
	return ConvertUnitVectorToCameraAngles( GetNormalized(v) );
}

/*! convert a vector to camera's angles:
x camera axis is looking up/down
y is roll
z is left/right	
*/
inline Ang3 ConvertVectorToCameraAnglesSnap180(const Vec3& vec)
{
	Ang3 ang=ConvertUnitVectorToCameraAngles( GetNormalized(vec) );
	ang.Snap180();
	return ang;
}






#endif

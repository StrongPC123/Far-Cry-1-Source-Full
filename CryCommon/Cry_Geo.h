//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//	
//	File: Cry_Geo.h
//	Description: Common structures for geometry computations
//
//	History:
//	-March 15,2003: Created by Ivo Herzeg
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYGEOSTRUCTS_H
#define CRYGEOSTRUCTS_H

#include "Cry_Math.h" 

#if _MSC_VER > 1000
# pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
// Forward declarations                                                      //
///////////////////////////////////////////////////////////////////////////////

struct Line;
struct Ray;
struct Lineseg;
template <typename F> struct Triangle_tpl;

struct AABB;
template <typename F> struct OBB_tpl;

struct Sphere;
struct AAEllipsoid;
struct Ellipsoid;

//-----------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// Definitions                                                               //
///////////////////////////////////////////////////////////////////////////////

#define FINDMINMAX(x0,x1,x2,min,max) \
	min = max = x0;   \
	if(x1<min) min=x1;\
	if(x1>max) max=x1;\
	if(x2<min) min=x2;\
	if(x2>max) max=x2;



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Line
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct Line {

	Vec3 pointonline;
	Vec3 direction; //caution: the direction is important for any intersection test

	//default Line constructor (without initialisation)
	inline Line( void ) {}
	inline Line( const Vec3 &o, const Vec3 &d ) {  pointonline=o; direction=d; }
	inline void operator () (  const Vec3 &o, const Vec3 &d  ) {  pointonline=o; direction=d; }

	~Line( void ) {};
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Ray
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct Ray {

	Vec3 origin;
	Vec3 direction;

	//default Ray constructor (without initialisation)
	inline Ray( void ) {}
	inline Ray( const Vec3 &o, const Vec3 &d ) {  origin=o; direction=d; }
	inline void operator () (  const Vec3 &o, const Vec3 &d  ) {  origin=o; direction=d; }

	~Ray( void ) {};
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Lineseg
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct Lineseg {

	Vec3 start;
	Vec3 end;

	//default Lineseg constructor (without initialisation)
	inline Lineseg( void ) {}
	inline Lineseg( const Vec3 &s, const Vec3 &e ) {  start=s; end=e; }
	inline void operator () (  const Vec3 &s, const Vec3 &e  ) {  start=s; end=e; }

	~Lineseg( void ) {};
};




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Triangle
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <typename F> struct Triangle_tpl {

	Vec3_tpl<F> v0,v1,v2;

	//default Lineseg constructor (without initialisation)
	inline Triangle_tpl( void ) {}
	inline Triangle_tpl( const Vec3_tpl<F>& a, const Vec3_tpl<F>& b, const Vec3_tpl<F>& c ) {  v0=a; v1=b; v2=c; }
	inline void operator () (  const Vec3_tpl<F>& a, const Vec3_tpl<F>& b, const Vec3_tpl<F>& c ) { v0=a; v1=b; v2=c; }

	~Triangle_tpl( void ) {};
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct AABB
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct AABB {

	Vec3 min;
	Vec3 max;

	//default AABB constructor (without initialisation)
	inline AABB( void ) {}
	inline AABB( const Vec3 &vmin, const Vec3 &vmax ) {  min=vmin; max=vmax; }
	inline void operator () (  const Vec3 &vmin, const Vec3 &vmax  ) {  min=vmin; max=vmax; }

	~AABB( void ) {};

	//! Reset Bounding box before calculating bounds.
	void Reset()	{	min = Vec3(MAX);	max = Vec3(MIN);	}

	//! Check if bounding box is empty (Zero volume).
	bool IsEmpty() const { return min == max; }

	void Add( const Vec3 &v )	{
		min.x=__min( min.x,v.x );	min.y=__min( min.y,v.y );	min.z=__min( min.z,v.z );
		max.x=__max( max.x,v.x );	max.y=__max( max.y,v.y );	max.z=__max( max.z,v.z );
	}

	//! Check if this bounding box overlap with bounding box of sphere.
	bool IsOverlapSphereBounds( const Vec3 &pos,float radius ) const
	{
		if (pos.x > min.x && pos.x < max.x &&	pos.y > min.y && pos.y < max.y &&	pos.z > min.z && pos.z < max.z) 
			return true;

		if (pos.x+radius < min.x) return false;
		if (pos.y+radius < min.y) return false;
		if (pos.z+radius < min.z) return false;
		if (pos.x-radius > max.x) return false;
		if (pos.y-radius > max.y) return false;
		if (pos.z-radius > max.z) return false;
		return true;
	}

	//! Check if this bounding box contain sphere within itself.
	bool IsContainSphere( const Vec3 &pos,float radius ) const
	{
		if (pos.x-radius < min.x) return false;
		if (pos.y-radius < min.y) return false;
		if (pos.z-radius < min.z) return false;
		if (pos.x+radius > max.x) return false;
		if (pos.y+radius > max.y) return false;
		if (pos.z+radius > max.z) return false;
		return true;
	}


	// Check two bounding boxes for intersection.
	inline bool	IsIntersectBox( const AABB &b ) const	{
		// Check for intersection on X axis.
		if ((min.x > b.max.x)||(b.min.x > max.x)) return false;
		// Check for intersection on Y axis.
		if ((min.y > b.max.y)||(b.min.y > max.y)) return false;
		// Check for intersection on Z axis.
		if ((min.z > b.max.z)||(b.min.z > max.z)) return false;
		// Boxes overlap in all 3 axises.
		return true;
	}

	//! Transforms AABB with specified matrix.
	void Transform( const Matrix44 &tm )	{
		Vec3 m = tm.TransformPointOLD( min );
		Vec3 vx = Vec3(tm(0,0),tm(0,1),tm(0,2))*(max.x-min.x);
		Vec3 vy = Vec3(tm(1,0),tm(1,1),tm(1,2))*(max.y-min.y);
		Vec3 vz = Vec3(tm(2,0),tm(2,1),tm(2,2))*(max.z-min.z);
		min = m;
		max = m;
		if (vx.x < 0) min.x += vx.x; else max.x += vx.x;
		if (vx.y < 0) min.y += vx.y; else max.y += vx.y;
		if (vx.z < 0) min.z += vx.z; else max.z += vx.z;

		if (vy.x < 0) min.x += vy.x; else max.x += vy.x;
		if (vy.y < 0) min.y += vy.y; else max.y += vy.y;
		if (vy.z < 0) min.z += vy.z; else max.z += vy.z;

		if (vz.x < 0) min.x += vz.x; else max.x += vz.x;
		if (vz.y < 0) min.y += vz.y; else max.y += vz.y;
		if (vz.z < 0) min.z += vz.z; else max.z += vz.z;
	}



	/*!
	* calculate the new bounds of a transformed AABB 
	*
	* Example:
	* AABB aabb = AABB::CreateAABBfromOBB(m34,aabb);
	*
	* return values:
	*  expanded AABB in world-space
	*/
	ILINE void SetTransformedAABB( const Matrix34& m34, const AABB& aabb ) {
		Vec3 sz		=	Matrix33(m34).GetFabs()*((aabb.max-aabb.min)*0.5f);
		Vec3 pos	= m34*((aabb.max+aabb.min)*0.5f);
		min=pos-sz;	max=pos+sz;
	}
	ILINE static AABB CreateTransformedAABB( const Matrix34& m34, const AABB& aabb ) { AABB taabb; taabb.SetTransformedAABB(m34,aabb); return taabb; 	}



	//create an AABB using just the extensions of the OBB and ignore the orientation. 
	template<typename F>
	ILINE void SetAABBfromOBB( const OBB_tpl<F>& obb ) { min=obb.c-obb.h; max=obb.c+obb.h;	}
	template<typename F>
	ILINE static AABB CreateAABBfromOBB( const OBB_tpl<F>& obb ) {	return AABB(obb.c-obb.h,obb.c+obb.h);	}

	/*!
	* converts an OBB into an tight fitting AABB 
	*
	* Example:
	* AABB aabb = AABB::CreateAABBfromOBB(wposition,obb,1.0f);
	*
	* return values:
	*  expanded AABB in world-space
	*/
	template<typename F>
	ILINE void SetAABBfromOBB( const Vec3& wpos, const OBB_tpl<F>& obb, f32 scaling=1.0f ) {
		Vec3 pos	= obb.m33*obb.c*scaling + wpos;
		Vec3 sz		=	obb.m33.GetFabs()*obb.h*scaling;
		min=pos-sz; max=pos+sz;
	}
	template<typename F>
	ILINE static AABB CreateAABBfromOBB( const Vec3& wpos, const OBB_tpl<F>& obb, f32 scaling=1.0f) { AABB taabb; taabb.SetAABBfromOBB(wpos,obb,scaling); return taabb; 	}

};






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct OBB
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <typename F> struct OBB_tpl {

	Matrix33 m33; //orientation vectors
	Vec3 h;				//half-length-vector
	Vec3 c;				//center of obb 

	//default OBB constructor (without initialisation)
	inline OBB_tpl() {}

	ILINE void SetOBB( const Matrix33& m33, const Vec3& hlv, const Vec3& center  ) {  m33=m33; h=hlv; c=center;	}
	ILINE static OBB_tpl<F> CreateOBB( const Matrix33& m33, const Vec3& hlv, const Vec3& center  ) {	OBB_tpl<f32> obb; obb.m33=m33; obb.h=hlv; obb.c=center; return obb;	}

	ILINE void SetOBBfromAABB( const Matrix33& mat33, const AABB& aabb ) {
		m33	=	mat33;
		h		=	(aabb.max-aabb.min)*0.5f;	//calculate the half-length-vectors
		c		=	(aabb.max+aabb.min)*0.5f;	//the center is relative to the PIVOT
	}
	ILINE static OBB_tpl<F> CreateOBBfromAABB( const Matrix33& m33, const AABB& aabb ) { OBB_tpl<f32> obb; obb.SetOBBfromAABB(m33,aabb); return obb;	}

	~OBB_tpl( void ) {};
};




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Sphere
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct Sphere {

	Vec3 center;
	float radius;

	//default Sphere constructor (without initialisation)
	inline Sphere( void ) {}
	inline Sphere( const Vec3 &c, const float &r ) {  center=c; radius=r; }
	inline void operator () (  const Vec3 &c, const float &r  ) {  center=c; radius=r; }

	~Sphere( void ) {};
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct AAEllipsoid
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct AAEllipsoid {

	Vec3 center;
	Vec3 radius_vec;

	//default AAEllipsoid constructor (without initialisation)
	inline AAEllipsoid( void ) {}
	inline AAEllipsoid( const Vec3 &c, const Vec3 &rv  ) {  radius_vec=rv; center=c; }
	inline void operator () ( const Vec3 &c, const Vec3 &rv ) {  radius_vec=rv; center=c; }

	~AAEllipsoid( void ) {};
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Ellipsoid
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct Ellipsoid {

	Matrix34 ExtensionPos;

	//default Ellipsoid constructor (without initialisation)
	inline Ellipsoid( void ) {}
	inline Ellipsoid( const Matrix34 &ep ) {  ExtensionPos=ep; }
	inline void operator () (  const Matrix34 &ep  ) {  ExtensionPos=ep; }

	~Ellipsoid( void ) {};
};






typedef Triangle_tpl<f32>	Triangle;
typedef Triangle_tpl<f64>	Triangle_f64;
typedef OBB_tpl<f32>	OBB;





#include "Cry_GeoDistance.h" 
#include "Cry_GeoOverlap.h" 
#include "Cry_GeoIntersect.h" 










/////////////////////////////////////////////////////////////////////////
//this is some special engine stuff, should be moved to a better location
/////////////////////////////////////////////////////////////////////////

// for bbox's checks and calculations
#define MAX_BB	+99999.0f
#define MIN_BB	-99999.0f

//! checks if this has been set to minBB
inline bool IsMinBB( const Vec3& v ) {
	if (v.x<=MIN_BB) return (true);
	if (v.y<=MIN_BB) return (true);
	if (v.z<=MIN_BB) return (true);
	return (false);
}

//! checks if this has been set to maxBB
inline bool IsMaxBB( const Vec3& v ) {
	if (v.x>=MAX_BB) return (true);
	if (v.y>=MAX_BB) return (true);
	if (v.z>=MAX_BB) return (true);
	return (false);
}

inline Vec3	SetMaxBB( void ) { return Vec3(MAX_BB,MAX_BB,MAX_BB); }
inline Vec3	SetMinBB( void ) { return Vec3(MIN_BB,MIN_BB,MIN_BB); }

inline void AddToBounds (const Vec3& v, Vec3& mins, Vec3& maxs) {
	if (v.x < mins.x)	mins.x = v.x;
	if (v.x > maxs.x)	maxs.x = v.x;
	if (v.y < mins.y)	mins.y = v.y;
	if (v.y > maxs.y)	maxs.y = v.y;
	if (v.z < mins.z)	mins.z = v.z;
	if (v.z > maxs.z)	maxs.z = v.z;
}


////////////////////////////////////////////////////////////////		
//! calc the area of a polygon giving a list of vertices and normal
inline float CalcArea(const Vec3 *vertices,int numvertices,const Vec3 &normal)
{	
	Vec3 csum(0,0,0);

	int n=numvertices;
	for (int i = 0, j = 1; i <= n-2; i++, j++)
	{
		csum.x += vertices[i].y*vertices[j].z-vertices[i].z*vertices[j].y;
		csum.y += vertices[i].z*vertices[j].x-vertices[i].x*vertices[j].z;
		csum.z += vertices[i].x*vertices[j].y-vertices[i].y*vertices[j].x;
	}

	csum.x += vertices[n-1].y*vertices[0].z-vertices[n-1].z*vertices[0].y;
	csum.y += vertices[n-1].z*vertices[0].x-vertices[n-1].x*vertices[0].z;
	csum.z += vertices[n-1].x*vertices[0].y-vertices[n-1].y*vertices[0].x;

	float area=0.5f*(float)fabs(normal*csum);
	return (area);
}



#endif //geostructs

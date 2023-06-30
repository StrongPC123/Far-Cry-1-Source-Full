////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   bbox.h
//  Version:     v1.00
//  Created:     2/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __bbox_h__
#define __bbox_h__

#if _MSC_VER > 1000
#pragma once
#endif

/*!
 *  BBox is a bounding box structure.
 */
struct BBox
{
	Vec3 min;
	Vec3 max;

	BBox() {}
	BBox( const Vec3 &vMin,const Vec3 &vMax ) { min = vMin; max = vMax; }
	void Reset()
	{
		min = Vec3d( 100000,100000,100000 );
		max = Vec3d( -100000,-100000,-100000 );
	}
	void Add( const Vec3 &v )
	{
		min.x = __min( min.x,v.x );
		min.y = __min( min.y,v.y );
		min.z = __min( min.z,v.z );
		
		max.x = __max( max.x,v.x );
		max.y = __max( max.y,v.y );
		max.z = __max( max.z,v.z );
	}

	bool IsOverlapSphereBounds( const Vec3 &pos,float radius ) const
	{
		if (pos.x > min.x && pos.x < max.x &&
				pos.y > min.y && pos.y < max.y &&
				pos.z > min.z && pos.z < max.z) 
			return true;

		if (pos.x+radius < min.x) return false;
		if (pos.y+radius < min.y) return false;
		if (pos.z+radius < min.z) return false;
		if (pos.x-radius > max.x) return false;
		if (pos.y-radius > max.y) return false;
		if (pos.z-radius > max.z) return false;
		return true;
	}
	
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

	bool IsEmpty() const { return IsEquivalent(min,max,VEC_EPSILON); }

	// Check two ortogonal bounding boxes for intersection.
	inline bool	IsIntersectBox( const BBox &b ) const
	{
		// Check for intersection on X axis.
		if ((min.x > b.max.x)||(b.min.x > max.x)) return false;
		// Check for intersection on Y axis.
		if ((min.y > b.max.y)||(b.min.y > max.y)) return false;
		// Check for intersection on Z axis.
		if ((min.z > b.max.z)||(b.min.z > max.z)) return false;
		
		// Boxes intersect in all 3 axises.
		return true;
	}

	void Transform( const Matrix44 &tm )
	{
		Vec3 m = tm.TransformPointOLD( min );
		Vec3 vx = Vec3(tm[0][0],tm[0][1],tm[0][2])*(max.x-min.x);
		Vec3 vy = Vec3(tm[1][0],tm[1][1],tm[1][2])*(max.y-min.y);
		Vec3 vz = Vec3(tm[2][0],tm[2][1],tm[2][2])*(max.z-min.z);
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

	bool IsIntersectRay( const Vec3 &origin,const Vec3 &dir,Vec3 &pntContact );

	//! Check if ray intersect edge of bounding box.
	//! @param epsilonDist if distance between ray and egde is less then this epsilon then edge was intersected.
	//! @param dist Distance between ray and edge.
	//! @param intPnt intersection point.
	bool RayEdgeIntersection( const Vec3 &raySrc,const Vec3 &rayDir,float epsilonDist,float &dist,Vec3 &intPnt );
};

#endif // __bbox_h__

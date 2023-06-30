////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   bbox.cpp
//  Version:     v1.00
//  Created:     2/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BBox.h"

typedef unsigned int udword;

// Integer representation of a floating-Vec3 value.
#define IR(x)	((udword&)x)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A method to compute a ray-AABB intersection.
 *	Original code by Andrew Woo, from "Graphics Gems", Academic Press, 1990
 *	Optimized code by Pierre Terdiman, 2000 (~20-30% faster on my Celeron 500)
 *	Epsilon value added by Klaus Hartmann. (discarding it saves a few cycles only)
 *
 *	Hence this version is faster as well as more robust than the original one.
 *
 *	Should work provided:
 *	1) the integer representation of 0.0f is 0x00000000
 *	2) the sign bit of the float is the most significant one
 *
 *	Report bugs: p.terdiman@codercorner.com
 *
 *	\param		aabb		[in] the axis-aligned bounding box
 *	\param		origin		[in] ray origin
 *	\param		dir			[in] ray direction
 *	\param		coord		[out] impact coordinates
 *	\return		true if ray intersects AABB
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define RAYAABB_EPSILON 0.00001f
bool BBox::IsIntersectRay( const Vec3& vOrigin, const Vec3& vDir, Vec3& pntContact )
{
	BOOL Inside = TRUE;
	float MinB[3] = { min.x,min.y,min.z };
	float MaxB[3] = { max.x,max.y,max.z };
	float MaxT[3];
	MaxT[0]=MaxT[1]=MaxT[2]=-1.0f;
	float coord[3] = { 0,0,0 };

	float origin[3] = { vOrigin.x,vOrigin.y,vOrigin.z };
	float dir[3] = { vDir.x,vDir.y,vDir.z };

	// Find candidate planes.
	for(udword i=0;i<3;i++)
	{
		if(origin[i] < MinB[i])
		{
			coord[i]	= MinB[i];
			Inside		= FALSE;

			// Calculate T distances to candidate planes
			if(IR(dir[i]))	MaxT[i] = (MinB[i] - origin[i]) / dir[i];
		}
		else if(origin[i] > MaxB[i])
		{
			coord[i]	= MaxB[i];
			Inside		= FALSE;

			// Calculate T distances to candidate planes
			if(IR(dir[i]))	MaxT[i] = (MaxB[i] - origin[i]) / dir[i];
		}
	}

	// Ray origin inside bounding box
	if(Inside)
	{
		//////////////////////////////////////////////////////////////////////////
		// Timur: for editor this need to be treated as intersection.
		//////////////////////////////////////////////////////////////////////////
		pntContact(origin[0],origin[1],origin[2]);
		return true;
	}

	// Get largest of the maxT's for final choice of intersection
	udword WhichPlane = 0;
	if(MaxT[1] > MaxT[WhichPlane])	WhichPlane = 1;
	if(MaxT[2] > MaxT[WhichPlane])	WhichPlane = 2;

	// Check final candidate actually inside box
	if(IR(MaxT[WhichPlane])&0x80000000) return false;

	for(i=0;i<3;i++)
	{
		if(i!=WhichPlane)
		{
			coord[i] = origin[i] + MaxT[WhichPlane] * dir[i];
#ifdef RAYAABB_EPSILON
			if(coord[i] < MinB[i] - RAYAABB_EPSILON || coord[i] > MaxB[i] + RAYAABB_EPSILON)	return false;
#else
			if(coord[i] < MinB[i] || coord[i] > MaxB[i])	return false;
#endif
		}
	}

	pntContact(coord[0],coord[1],coord[2]);
	return true;	// ray hits box
}

//////////////////////////////////////////////////////////////////////////
bool BBox::RayEdgeIntersection( const Vec3 &raySrc,const Vec3 &rayDir,float epsilonDist,float &dist,Vec3 &intPnt )
{
	// Check 6 group lines.
	Vec3 rayTrg = raySrc + rayDir*10000.0f;
	Vec3 pnt[12];

	float d[12];

	// Near
	d[0] = RayToLineDistance( raySrc,rayTrg,Vec3(min.x,min.y,max.z),Vec3(max.x,min.y,max.z),pnt[0] );
	d[1] = RayToLineDistance( raySrc,rayTrg,Vec3(min.x,max.y,max.z),Vec3(max.x,max.y,max.z),pnt[1] );
	d[2] = RayToLineDistance( raySrc,rayTrg,Vec3(min.x,min.y,max.z),Vec3(min.x,max.y,max.z),pnt[2] );
	d[3] = RayToLineDistance( raySrc,rayTrg,Vec3(max.x,min.y,max.z),Vec3(max.x,max.y,max.z),pnt[3] );

	// Far
	d[4] = RayToLineDistance( raySrc,rayTrg,Vec3(min.x,min.y,min.z),Vec3(max.x,min.y,min.z),pnt[4] );
	d[5] = RayToLineDistance( raySrc,rayTrg,Vec3(min.x,max.y,min.z),Vec3(max.x,max.y,min.z),pnt[5] );
	d[6] = RayToLineDistance( raySrc,rayTrg,Vec3(min.x,min.y,min.z),Vec3(min.x,max.y,min.z),pnt[6] );
	d[7] = RayToLineDistance( raySrc,rayTrg,Vec3(max.x,min.y,min.z),Vec3(max.x,max.y,min.z),pnt[7] );

	// Sides.
	d[8] = RayToLineDistance( raySrc,rayTrg,Vec3(min.x,min.y,min.z),Vec3(min.x,min.y,max.z),pnt[8] );
	d[9] = RayToLineDistance( raySrc,rayTrg,Vec3(max.x,min.y,min.z),Vec3(max.x,min.y,max.z),pnt[9] );
	d[10] = RayToLineDistance( raySrc,rayTrg,Vec3(min.x,max.y,min.z),Vec3(min.x,max.y,max.z),pnt[10] );
	d[11] = RayToLineDistance( raySrc,rayTrg,Vec3(max.x,max.y,min.z),Vec3(max.x,max.y,max.z),pnt[11] );

	dist = FLT_MAX;
	for (int i = 0; i < 12; i++)
	{
		if (d[i] < dist)
		{
			dist = d[i];
			intPnt = pnt[i];
		}
	}
	if (dist < epsilonDist)
	{
		return true;
	}
	return false;
}
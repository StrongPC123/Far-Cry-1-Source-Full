////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   math.h
//  Version:     v1.00
//  Created:     2/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Various math and geometry related functions.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __math_h__
#define __math_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BBox.h"

//! Half PI
#define PI_HALF (3.1415926535897932384626433832795f / 2.0f)

//! Epsilon for vector comparasions.
#define FLOAT_EPSILON 0.0001f

//////////////////////////////////////////////////////////////////////////
/** Compare two vectors if they are equal.
*/
inline bool IsVectorsEqual( const Vec3 &v1,const Vec3 &v2 )
{
	if (fabs(v2.x-v1.x) < FLOAT_EPSILON && fabs(v2.y-v1.y) < FLOAT_EPSILON && fabs(v2.z-v1.z) < FLOAT_EPSILON)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
/** Compare two matrices if they are equal.
*/
inline bool IsMatrixEqual( const Matrix44 &tm1,const Matrix44 &tm2 )
{
	for (int i = 0; i < 4; i++)
	{
		if (fabs(tm1[i][0] - tm2[i][0]) > FLOAT_EPSILON ||
				fabs(tm1[i][1] - tm2[i][1]) > FLOAT_EPSILON ||
				fabs(tm1[i][2] - tm2[i][2]) > FLOAT_EPSILON ||
				fabs(tm1[i][3] - tm2[i][3]) > FLOAT_EPSILON)
			return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Math utilities.
//////////////////////////////////////////////////////////////////////////
inline float PointToLineDistance2D( const Vec3 &p1,const Vec3 &p2,const Vec3 &p3 )
{
	float dx = p2.x-p1.x;
	float dy = p2.y-p1.y;
	float u = ((p3.x-p1.x)*dx + (p3.y-p1.y)*dy) / (dx*dx + dy*dy);
	if (u < 0)
		return (float)sqrt( (p3.x-p1.x)*(p3.x-p1.x) + (p3.y-p1.y)*(p3.y-p1.y) );
	else if (u > 1)
		return (float)sqrt( (p3.x-p2.x)*(p3.x-p2.x) + (p3.y-p2.y)*(p3.y-p2.y) );
	else
	{
		float x = p1.x + u*dx;
		float y = p1.y + u*dy;
		return (float)sqrt( (p3.x-x)*(p3.x-x) + (p3.y-y)*(p3.y-y) );
	}
}

inline float PointToLineDistance( const Vec3 &p1,const Vec3 &p2,const Vec3 &p3 )
{
	Vec3 d = p2 - p1;
	float u = d.Dot(p3-p1) / GetLengthSquared(d);
	if (u < 0)
		return (p3 - p1).Length();
	else if (u > 1)
		return (p3 - p2).Length();
	else
	{
		Vec3 p = p1 + u*d;
		return (p3 - p).Length();
	}
}

/** Calculate distance between point and line.
	@param p1 Source line point.
	@param p2 Target line point.
	@param p3 Point to find intersecion with.
	@param intersectPoint Intersection point on the line.
	@return Distance between point and line.
*/
inline float PointToLineDistance( const Vec3 &p1,const Vec3 &p2,const Vec3 &p3,Vec3 &intersectPoint )
{
	Vec3 d = p2 - p1;
	float u = d.Dot(p3-p1) / GetLengthSquared(d);
	if (u < 0)
	{
		intersectPoint = p1;
		return (p3 - p1).Length();
	}
	else if (u > 1)
	{
		intersectPoint = p2;
		return (p3 - p2).Length();
	}
	else
	{
		Vec3 p = p1 + u*d;
		intersectPoint = p;
		return (p3 - p).Length();
	}
}

/**
   Calculate the line segment PaPb that is the shortest route between
   two lines P1P2 and P3P4. Calculate also the values of mua and mub where
      Pa = P1 + mua (P2 - P1)
      Pb = P3 + mub (P4 - P3)
	
	@param p1 Source point of first line.
	@param p2 Target point of first line.
	@param p3 Source point of second line.
	@param p4 Target point of second line.
	@return FALSE if no solution exists.
*/
inline bool LineLineIntersect( const Vec3 &p1,const Vec3 &p2,const Vec3 &p3,const Vec3 &p4,
															Vec3 &pa,Vec3 &pb,float &mua,float &mub )
{
	Vec3 p13,p43,p21;
	float d1343,d4321,d1321,d4343,d2121;
	float numer,denom;

	p13.x = p1.x - p3.x;
	p13.y = p1.y - p3.y;
	p13.z = p1.z - p3.z;
	p43.x = p4.x - p3.x;
	p43.y = p4.y - p3.y;
	p43.z = p4.z - p3.z;
	if (fabs(p43.x)  < LINE_EPS && fabs(p43.y)  < LINE_EPS && fabs(p43.z)  < LINE_EPS)
		return false;
	p21.x = p2.x - p1.x;
	p21.y = p2.y - p1.y;
	p21.z = p2.z - p1.z;
	if (fabs(p21.x)  < LINE_EPS && fabs(p21.y)  < LINE_EPS && fabs(p21.z)  < LINE_EPS)
		return false;

	d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
	d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
	d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
	d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
	d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

	denom = d2121 * d4343 - d4321 * d4321;
	if (fabs(denom) < LINE_EPS)
		return(FALSE);
	numer = d1343 * d4321 - d1321 * d4343;

	mua = numer / denom;
	mub = (d1343 + d4321 * (mua)) / d4343;

	pa.x = p1.x + mua * p21.x;
	pa.y = p1.y + mua * p21.y;
	pa.z = p1.z + mua * p21.z;
	pb.x = p3.x + mub * p43.x;
	pb.y = p3.y + mub * p43.y;
	pb.z = p3.z + mub * p43.z;

	return true;
}

/*!
		Calculates shortest distance between ray and a arbitary line segment.
		@param raySrc Source point of ray.
		@param rayTrg Target point of ray.
		@param p1 First point of line segment.
		@param p2 Second point of line segment.
		@param intersectPoint This parameter returns nearest point on line segment to ray.
		@return distance fro ray to line segment.
*/
inline float RayToLineDistance( const Vec3 &raySrc,const Vec3 &rayTrg,const Vec3 &p1,const Vec3 &p2,Vec3 &nearestPoint )
{
	Vec3 intPnt;
	Vec3 rayLineP1 = raySrc;
	Vec3 rayLineP2 = rayTrg;
	Vec3 pa,pb;
	float ua,ub;

	if (!LineLineIntersect( p1,p2, rayLineP1,rayLineP2, pa,pb,ua,ub ))
		return FLT_MAX;
		
	float d = 0;
	if (ua < 0)
		d = PointToLineDistance( rayLineP1,rayLineP2,p1,intPnt );
	else if (ua > 1)
		d = PointToLineDistance( rayLineP1,rayLineP2,p2,intPnt );
	else
	{
		intPnt = rayLineP1 + ub*(rayLineP2-rayLineP1);
		d = (pb-pa).Length();
	}
	nearestPoint = intPnt;
	return d;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
inline Matrix44 MatrixFromVector( const Vec3d &dir )
{
	Matrix44 M;
	// LookAt transform.
	Vec3d xAxis,yAxis,zAxis;
	Vec3d upVector;

	yAxis = GetNormalized(dir);

	if (yAxis.x == 0.0 && yAxis.y == 0)
		upVector( -yAxis.z,0,0 );
	else 
		upVector( 0,0,1.0f );

	xAxis = GetNormalized((upVector.Cross(yAxis)));
	zAxis = GetNormalized( -(xAxis.Cross(yAxis)) );

	/*
	// D3D kind of matrix.
	M[0][0] = xAxis.x;
	M[0][1] = yAxis.x;
	M[0][2] = zAxis.x;
	M[0][3] = 0;

	M[1][0] = xAxis.y;
	M[1][1] = yAxis.y;
	M[1][2] = zAxis.y;
	M[1][3] = 0;

	M[2][0] = xAxis.z;
	M[2][1] = yAxis.z;
	M[2][2] = zAxis.z;
	M[2][3] = 0;

	M[3][0] = 0;
	M[3][1] = 0;
	M[3][2] = 0;
	M[3][3] = 1;
	*/

	// OpenGL kind of matrix.
	M[0][0] = xAxis.x;
	M[1][0] = yAxis.x;
	M[2][0] = zAxis.x;
	M[3][0] = 0;

	M[0][1] = xAxis.y;
	M[1][1] = yAxis.y;
	M[2][1] = zAxis.y;
	M[3][1] = 0;

	M[0][2] = xAxis.z;
	M[1][2] = yAxis.z;
	M[2][2] = zAxis.z;
	M[3][2] = 0;

	M[0][3] = 0;
	M[1][3] = 0;
	M[2][3] = 0;
	M[3][3] = 1;

	return M;
}

//////////////////////////////////////////////////////////////////////////
inline Matrix44 MatrixFromVector( const Vec3d &dir,const Vec3d &up,float rollAngle=0 )
{
	Matrix44 M;
	// LookAt transform.
	Vec3d xAxis,yAxis,zAxis;
	Vec3d upVector = up;

	yAxis = GetNormalized(-dir);

	//if (zAxis.x == 0.0 && zAxis.z == 0)	up.Set( -zAxis.y,0,0 );	else up.Set( 0,1.0f,0 );

	xAxis = GetNormalized((upVector.Cross(yAxis)));
	zAxis = GetNormalized(xAxis.Cross(yAxis));

	// OpenGL kind of matrix.
	M[0][0] = xAxis.x;
	M[1][0] = yAxis.x;
	M[2][0] = zAxis.x;
	M[3][0] = 0;

	M[0][1] = xAxis.y;
	M[1][1] = yAxis.y;
	M[2][1] = zAxis.y;
	M[3][1] = 0;

	M[0][2] = xAxis.z;
	M[1][2] = yAxis.z;
	M[2][2] = zAxis.z;
	M[3][2] = 0;

	M[0][3] = 0;
	M[1][3] = 0;
	M[2][3] = 0;
	M[3][3] = 1;

	if (rollAngle != 0)
	{
		Matrix44 RollMtx;
		RollMtx.SetIdentity();

		float s = sinf(rollAngle);
		float c = cosf(rollAngle);

		RollMtx[0][0] = c; RollMtx[2][0] = -s;;
		RollMtx[0][2] = s; RollMtx[2][2] = c;

		// Matrix multiply.
		M = RollMtx * M;
	}

	return M;
}

#endif // __math_h__

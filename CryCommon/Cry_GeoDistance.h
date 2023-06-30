//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//	
//	File:Cry_GeoDistance.h
//	Description: Common distance-computations
//
//	History:
//	-March 15,2003: Created by Ivo Herzeg
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYDISTANCE_H
#define CRYDISTANCE_H

#if _MSC_VER > 1000
# pragma once
#endif

#include <Cry_Geo.h>

namespace Distance {


 /*!
	* Distance: Origin_Triangle2D
	* Calculate the closest distance of a triangle in XY-plane to the coordinate origin.
	* it is assumed that the z-values of the triangle are all in the same plane. 
	* The function returns the 3d-position of the closest point on the triangle.
	*
	* Example:
	*  Vec3 result = Distance::Origin_Triangle2D( triangle );
	*/
	template<typename F>
		ILINE Vec3_tpl<F> Origin_Triangle2D( const Triangle_tpl<F>& t ) {
			Vec3_tpl<F> a=t.v0;	
			Vec3_tpl<F> b=t.v1;
			Vec3_tpl<F> c=t.v2;
			//check if (0,0,0) is inside or in fron of any triangle sides.
			u32 flag = ((a.x*(a.y-b.y)-a.y*(a.x-b.x))<0) | (((b.x*(b.y-c.y)-b.y*(b.x-c.x))<0)<<1) | (((c.x*(c.y-a.y)-c.y*(c.x-a.x))<0)<<2);
			switch (flag) {
			case 0:	return Vec3_tpl<F>(0,0,a.z); //center is inside of triangle
			case 1:	if ((a|(b-a))>0.0f) flag=5;	else if ((b|(a-b))>0.0f) flag=3;	break;
			case 2:	if ((b|(c-b))>0.0f) flag=3; else if ((c|(b-c))>0.0f) flag=6;	break;
			case 3:	return b; //vertex B is closed
			case 4:	if ((c|(a-c))>0.0f) flag=6; else if ((a|(c-a))>0.0f) flag=5;	break;
			case 5:	return a; //vertex A is closed 
			case 6:	return c; //vertex C is closed
			}
			//check again using expanded area
			switch (flag) {
			case 1: { Vec3_tpl<F> n=(b-a).GetNormalized(); return n*(-a|n)+a; }
			case 2:	{	Vec3_tpl<F> n=(c-b).GetNormalized(); return n*(-b|n)+b; }
			case 3:	return b;
			case 4:	{ Vec3_tpl<F> n=(a-c).GetNormalized(); return n*(-c|n)+c; }
			case 5:	return a;
			case 6:	return c;
			}
			return Vec3_tpl<F>(0,0,0);
		}


	/*!
	* Distance: Point_Triangle
	* Calculate the closest distance of a point to a triangle in 3d-space.
	* The function returns the squared distance. 
	*
	* Example:
	*  float result = Distance::Point_Triangle( pos, triangle );
	*/
	template<typename F>
	 ILINE F Point_Triangle( const Vec3_tpl<F>& p, const Triangle_tpl<F> &t ) {
		//translate triangle into origin
			Vec3_tpl<F> a=t.v0-p;	
			Vec3_tpl<F> b=t.v1-p;	
			Vec3_tpl<F> c=t.v2-p;
			//transform triangle into XY-plane to simplify the test
			Matrix33_tpl<F,3,1> r33=Matrix33_tpl<F,3,1>::CreateRotationV0( ((b-a)%(a-c)).GetNormalized() );
			Vec3_tpl<F> h = Origin_Triangle2D(Triangle_tpl<F>(r33*a,r33*b,r33*c));
			return (h|h); //return squared distance
		}


 /*!
	* Distance: Point_Triangle
	* Calculate the closest distance of a point to a triangle in 3d-space.
	* The function returns the squared distance and the 3d-position of the 
	* closest point on the triangle.
	*
	* Example:
	*  float result = Distance::Point_Triangle( pos, triangle, output );
	*/
	template<typename F>
		ILINE F Point_Triangle( const Vec3_tpl<F>& p, const Triangle_tpl<F> &t, Vec3_tpl<F>& output ) {
			//translate triangle into origin
			Vec3_tpl<F> a=t.v0-p;	
			Vec3_tpl<F> b=t.v1-p;	
			Vec3_tpl<F> c=t.v2-p;
			//transform triangle into XY-plane to simplify the test
			Matrix33_tpl<F,3,1> r33=Matrix33_tpl<F,3,1>::CreateRotationV0( ((b-a)%(a-c)).GetNormalized() );
			Vec3_tpl<F> h = Origin_Triangle2D(Triangle_tpl<F>(r33*a,r33*b,r33*c));
			output=h*r33+p;
			return (h|h); //return squared distance
	}



	//----------------------------------------------------------------------------------
	// Distance: Sphere_Triangle
	//----------------------------------------------------------------------------------
	// Calculate the closest distance of a sphere to a triangle in 3d-space.
	// The function returns the squared distance. If sphere and triangle overlaps, 
	// the returned distance is 0
	//
	// Example:
	//  float result = Distance::Point_Triangle( pos, triangle );
	//----------------------------------------------------------------------------------
	template<typename F>
		ILINE F Sphere_Triangle( const Sphere &s, const Triangle_tpl<F> &t ) {
			F sqdistance =  Distance::Point_Triangle(s.center,t) - (s.radius*s.radius);
			if (sqdistance<0) sqdistance=0; 
			return sqdistance;
	}

	template<typename F>
		ILINE F Sphere_Triangle( const Sphere &s, const Triangle_tpl<F> &t, Vec3_tpl<F>& output ) {
			F sqdistance =  Distance::Point_Triangle(s.center,t,output) - (s.radius*s.radius);
			if (sqdistance<0) sqdistance=0; 
			return sqdistance;
	}


} //namespace Distance


#endif

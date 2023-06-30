//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//	
//	File:Cry_GeoOverlap.h
//	Description: Common overlap-tests
//
//	History:
//	-March 15,2003: Created by Ivo Herzeg
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYOVERLAP_H
#define CRYOVERLAP_H

#if _MSC_VER > 1000
# pragma once
#endif


#include <Cry_Geo.h>




namespace Overlap {

	////////////////////////////////////////////////////////////////		
	//! check if the point is inside an AABB
	inline bool	Point_AABB(const Vec3 &p, const Vec3 &mins,const Vec3 &maxs)
	{
		if ((p.x>=mins.x && p.x<=maxs.x) && (p.y>=mins.y && p.y<=maxs.y) && (p.z>=mins.z && p.z<=maxs.z))	return (true);
		return (false);
	}

	/*!
	* check if a point is inside an OBB.
	*
	* Example:
	*  bool result=Overlap::Point_OBB( point, obb );
	*
	*/
	ILINE bool	Point_OBB(const Vec3& p, const Vec3& wpos, const OBB& obb) {
		AABB aabb=AABB(obb.c-obb.h,obb.c+obb.h);
		Vec3 t=(p-wpos)*obb.m33;
		return ((t.x>=aabb.min.x && t.x<=aabb.max.x) && (t.y>=aabb.min.y && t.y<=aabb.max.y) && (t.z>=aabb.min.z && t.z<=aabb.max.z));
	}

	//-----------------------------------------------------------------------------------------


	//! check if a Lineseg and a Sphere overlap
	inline bool	Lineseg_Sphere(const Lineseg& ls,const Sphere& s)
	{

		float radius2=s.radius*s.radius;

		//check if one of the two edpoints of the line is inside the sphere  
		Vec3 diff = ls.end-s.center;
		if (diff.x*diff.x+diff.y*diff.y+diff.z*diff.z <= radius2)	return true;

		Vec3 AC = s.center-ls.start;
		if (AC.x*AC.x+AC.y*AC.y+AC.z*AC.z <= radius2)	return true;

		//check distance from the sphere to the line
		Vec3 AB = ls.end-ls.start;

		float r = (AC.x*AB.x+AC.y*AB.y+AC.z*AB.z) / (AB.x*AB.x+AB.y*AB.y+AB.z*AB.z);

		//projection falls outside the line
		if (r<0 || r>1)	return false;

		//check if the distance from the line to the center of the sphere is less than radius
		Vec3 point = ls.start + r*AB;
		if ((point.x-s.center.x)*(point.x-s.center.x) + (point.y-s.center.y)*(point.y-s.center.y) + (point.z-s.center.z)*(point.z-s.center.z) > radius2)	return false;

		return true;
	}


	/*!
	* we use the SEPARATING AXIS TEST to check if a Linesegment overlap an AABB.
	*
	* Example:
	*  bool result=Overlap::Lineseg_AABB( ls, pos,aabb );
	*
	*/
	inline bool Lineseg_AABB ( const Lineseg &ls, const Vec3 &pos, const AABB &aabb ) {
		//calculate the half-length-vectors of the AABB
		Vec3 h	=	(aabb.max-aabb.min)*0.5f;
		//"t" is the transfer-vector from one center to the other
		Vec3 t		= ((ls.start+ls.end)*0.5f - ((aabb.max+aabb.min)*0.5f+pos));
		//calculate line-direction
		Vec3 ld  =  (ls.end-ls.start)*0.5f;
		if( fabsf(t.x) > (h.x + fabsf(ld.x)) ) return 0;
		if( fabsf(t.y) > (h.y + fabsf(ld.y)) ) return 0;
		if( fabsf(t.z) > (h.z + fabsf(ld.z)) ) return 0;
		if( fabsf(t.z*ld.y-t.y*ld.z) > (fabsf(h.y*ld.z) + fabsf(h.z*ld.y)) )	return 0;
		if( fabsf(t.x*ld.z-t.z*ld.x) > (fabsf(h.x*ld.z) + fabsf(h.z*ld.x)) )	return 0;
		if( fabsf(t.y*ld.x-t.x*ld.y) > (fabsf(h.x*ld.y) + fabsf(h.y*ld.x)) )	return 0;
		return 1; //no separating axis found, objects overlap
	}



	/*!
	* we use the SEPARATING AXIS TEST to check if two OBB's overlap.
	*
	* Example:
	*  bool result=Overlap::Lineseg_OBB( lineseg, pos,obb );
	*
	*/
	inline bool Lineseg_OBB ( const Lineseg &ls, const Vec3 &pos, const OBB &obb ) {
		//the new center-position of Lineseg and OBB in world-space
		Vec3 wposobb	=	obb.m33*obb.c + pos;
		Vec3 wposls	=	(ls.start+ls.end)*0.5f;
		//"t" is the transfer-vector from one center to the other
		Vec3 t		= (wposls - wposobb)*obb.m33;
		//calculate line-direction in local obb-space
		Vec3 ld  =  ( (ls.end-ls.start)*obb.m33 )*0.5f;
		if( fabsf(t.x) > (obb.h.x + fabsf(ld.x)) ) return 0;
		if( fabsf(t.y) > (obb.h.y + fabsf(ld.y)) ) return 0;
		if( fabsf(t.z) > (obb.h.z + fabsf(ld.z)) ) return 0;
		if( fabsf(t.z*ld.y-t.y*ld.z) > (fabsf(obb.h.y*ld.z) + fabsf(obb.h.z*ld.y)) ) return 0;
		if( fabsf(t.x*ld.z-t.z*ld.x) > (fabsf(obb.h.x*ld.z) + fabsf(obb.h.z*ld.x)) ) return 0;
		if( fabsf(t.y*ld.x-t.x*ld.y) > (fabsf(obb.h.x*ld.y) + fabsf(obb.h.y*ld.x)) ) return 0;
		return 1; //no separating axis found, objects overlap
	}




	/*!
	*
	* overlap-test between a line and a triangle.
	* IMPORTANT: this is a single-sided test. That means its not enough 
	* that the triangle and line overlap, its also important that the triangle 
	* is "visible" when you are looking along the line-direction.  
	* 
	* If you need a double-sided test, you'll have to call this function twice with 
	* reversed order of triangle vertices.   
	* 
	* return values 
	* return "true" if line and triangle overlap.
	* 
	*/
	inline bool Line_Triangle( const Line &line, const Vec3 &v0, const Vec3 &v1, const Vec3 &v2 ) {
		//find vectors for two edges sharing v0
		Vec3 edge_1 = v1-v0;
		Vec3 edge_2 = v2-v0;
		//begin calculating determinant - also used to calculate U parameter
		Vec3 pvec  =  line.direction % edge_1; 
		//if determinat is near zero, line lies in plane of triangel 
		float det = edge_2 | pvec;
		if (det<=0) return 0;
		//calculate distance from v0 to line origin
		Vec3 tvec=line.pointonline-v0;
		float u=tvec | pvec;
		if ( (u<0.0f) || (u>det)) return 0;
		//prepare to test V parameter
		Vec3 qvec=tvec % edge_2;
		float v= (line.direction | qvec);
		if ( (v<0.0f) || ((u+v)>det)) return 0;
		return 1;
	}


	/*!
	*
	* overlap-test between a ray and a triangle.
	* IMPORTANT: this is a single-sided test. That means its not sufficient 
	* that the triangle and ray overlap, its also important that the triangle 
	* is "visible" when you are looking from the origin along the ray-direction.  
	* 
	* If you need a double-sided test, you'll have to call this function twice with 
	* reversed order of triangle vertices.   
	* 
	* return values 
	* return "true" if ray and triangle overlap.
	*/
	inline bool Ray_Triangle( const Ray &ray, const Vec3 &v0, const Vec3 &v1, const Vec3 &v2 ) {
		//find vectors for two edges sharing v0
		Vec3 edge_1 = v1-v0;
		Vec3 edge_2 = v2-v0;
		//begin calculating determinant - also used to calculate U parameter
		Vec3 pvec  =  ray.direction % edge_1; 
		//if determinat is near zero, ray lies in plane of triangle 
		float det = edge_2 | pvec;
		if (det<=0) return 0;
		//calculate distance from v0 to ray origin
		Vec3 tvec=ray.origin-v0;
		//calculate U parameter and test bounds
		float u=tvec | pvec;
		if ( u<0.0f || u>det) return 0;
		//prepare to test V parameter
		Vec3 qvec=tvec % edge_2;
		//calculate V parameter and test bounds
		float v= (ray.direction | qvec);
		if ( v<0.0f || (u+v)>det) return 0;
		//------------------------------------------------------
		//We have an intersection and now we can calculate t
		float t = (edge_1 | qvec) / det;
		//we use t als a scale parameter, to get the 3D-intersection point
		Vec3 output = (ray.direction*t)+ray.origin;
		//skip, if cutting-point is "behind" ray.origin
		if (((output-ray.origin)|ray.direction)<0) return 0;
		return 1;
	}

	/*!
	*
	* overlap-test between line-segment and a triangle.
	* IMPORTANT: this is a single-sided test. That means its not sufficient 
	* that the triangle and line-segment overlap, its also important that the triangle 
	* is "visible" when you are looking along the linesegment from "start" to "end".  
	* 
	* If you need a double-sided test, you'll have to call this function twice with 
	* reversed order of triangle vertices.   
	* 
	* return values 
	* return "true" if linesegment and triangle overlap.
	*/
	inline bool Lineseg_Triangle( const Lineseg &lineseg, const Vec3 &v0, const Vec3 &v1, const Vec3 &v2 ) {
		//find vectors for two edges sharing v0
		Vec3 edge_1 = v1-v0;
		Vec3 edge_2 = v2-v0;
		//direction of lineseg. normalizing is not necessary
		Vec3 direction=(lineseg.end-lineseg.start);
		//begin calculating determinant - also used to calculate U parameter
		Vec3 pvec  =  direction % edge_1; 
		//if determinat is near zero, ray lies in plane of triangle 
		float det = edge_2 | pvec;
		if (det<=0) return 0;
		//calculate distance from v0 to ray origin
		Vec3 tvec=lineseg.start-v0;
		//calculate U parameter and test bounds
		float u=tvec | pvec;
		if (u<0.0f || u>det) return 0;
		//prepare to test V parameter
		Vec3 qvec=tvec % edge_2;
		//calculate V parameter and test bounds
		float v= (direction | qvec);
		if ( v<0.0f || (u+v)>det) return 0;
		//------------------------------------------------------
		//If we ignore the limitations of linesegment, then we have an intersection and we can calculate t
		float t = (edge_1 | qvec) / det;
		//skip, if lineseg and triangle are not overlapping
		Vec3 output = (direction*t)+lineseg.start;
		if (((output-lineseg.start)|direction)<0) return 0;
		if (((output-lineseg.end)|direction)>0) return 0;
		return 1;
	}





	/*----------------------------------------------------------------------------------
	* Sphere_AABB
	*	Sphere and AABB are assumed to be in the same space
	*
	* Example:
	*  bool result=Overlap::Sphere_AABB_Inside( sphere, aabb );
	*
	* 0 = no overlap             
	* 1 = overlap                
	*----------------------------------------------------------------------------------*/
	ILINE bool Sphere_AABB( const Sphere &s, const AABB &aabb ) {
		//we are using Arvo's method, to check if the objects are overlaping 
		float quatradius = s.radius * s.radius;
		Vec3 quat(0,0,0);
		if(s.center.x < aabb.min.x) { quat.x = s.center.x - aabb.min.x;	} 
		else if(s.center.x > aabb.max.x) {	quat.x = s.center.x - aabb.max.x;}
		if(s.center.y < aabb.min.y) { quat.y = s.center.y - aabb.min.y;	}	
		else if(s.center.y > aabb.max.y) {	quat.y = s.center.y - aabb.max.y;	}
		if(s.center.z < aabb.min.z) { quat.z=s.center.z-aabb.min.z;	} 
		else if(s.center.z > aabb.max.z) {	quat.z = s.center.z - aabb.max.z; }
		return( (quat|quat) < quatradius);
	}

	/*!
	*
	* conventional method to check if a Sphere and an AABB overlap, 
	* or if the Sphere is completely inside the AABB.
	*	Sphere and AABB are assumed to be in the same space
	*
	* Example:
	*  bool result=Overlap::Sphere_AABB_Inside( sphere, aabb );
	*
	* return values:
	* 0x00 = no overlap         
	* 0x01 = Sphere and AABB overlap
	* 0x02 = Sphere in inside AABB 
	*/
	ILINE char Sphere_AABB_Inside(  const Sphere &s, const AABB& aabb ) {
		if ( Sphere_AABB(s,aabb) ) {
			Vec3 amin=aabb.min-s.center;
			Vec3 amax=aabb.max-s.center;
			if  (amin.x>=(-s.radius)) return 1;
			if  (amin.y>=(-s.radius)) return 1; 
			if 	(amin.z>=(-s.radius)) return 1; 
			if  (amax.x<=(+s.radius)) return 1;
			if  (amax.y<=(+s.radius)) return 1;
			if  (amax.z<=(+s.radius)) return 1;
			//yes, its inside 
			return 2;
		}
		return 0;
	}

	//----------------------------------------------------------------------------------
	//  Sphere_OBB
	//	VERY IMPORTANT: Sphere is assumed to be in the space of the OBB, otherwise it won't work 
	//
	//--- 0 = no overlap                                     ---------------------------
	//--- 1 = overlap                                                  -----------------
	//----------------------------------------------------------------------------------
	inline bool Sphere_OBB( const Sphere &s, const OBB &obb ) {
		//first we transform the sphere-center into the AABB-space of the OBB
		Vec3 SphereInOBBSpace	=	s.center*obb.m33;
		//the rest ist the same as the "Overlap::Sphere_AABB" calculation
		float quatradius = s.radius * s.radius;
		Vec3 quat(0,0,0);
		AABB aabb=AABB(obb.c-obb.h,obb.c+obb.h);
		if(SphereInOBBSpace.x < aabb.min.x) { quat.x = SphereInOBBSpace.x - aabb.min.x;	} 
		else if(SphereInOBBSpace.x > aabb.max.x) {	quat.x = SphereInOBBSpace.x - aabb.max.x;}
		if(SphereInOBBSpace.y < aabb.min.y) { quat.y = SphereInOBBSpace.y - aabb.min.y;	}	
		else if(SphereInOBBSpace.y > aabb.max.y) {	quat.y = SphereInOBBSpace.y - aabb.max.y;	}
		if(SphereInOBBSpace.z < aabb.min.z) { quat.z=SphereInOBBSpace.z-aabb.min.z;	} 
		else if(SphereInOBBSpace.z > aabb.max.z) {	quat.z = SphereInOBBSpace.z - aabb.max.z; }
		return((quat|quat) < quatradius);
	}


	//----------------------------------------------------------------------------------
	//  Sphere_Sphere overlap test
	//
	//--- 0 = no overlap                                     ---------------------------
	//--- 1 = overlap                                                  -----------------
	//----------------------------------------------------------------------------------
	inline bool Sphere_Sphere( const Sphere &s1, const Sphere &s2 ) {
		Vec3 distc	=	s1.center-s2.center;
		f32 sqrad		=	(s1.radius+s2.radius) * (s1.radius+s2.radius);
		return ( sqrad>(distc|distc) );				
	}

	//----------------------------------------------------------------------------------
	//  Sphere_Triangle overlap test
	//
	//--- 0 = no overlap                                     ---------------------------
	//--- 1 = overlap                                                  -----------------
	//----------------------------------------------------------------------------------
	template<typename F>
	ILINE bool Sphere_Triangle( const Sphere &s, const Triangle_tpl<F> &t ) {
		//create a "bouding sphere" around triangle for fast rejection test
		Vec3_tpl<F> middle=(t.v0+t.v1+t.v2)*(1/3.0f);
		Vec3_tpl<F> ov0=t.v0-middle;
		Vec3_tpl<F> ov1=t.v1-middle;
		Vec3_tpl<F> ov2=t.v2-middle;
		F SqRad1=(ov0|ov0);
		if (SqRad1<(ov1|ov1)) SqRad1=(ov1|ov1);
		if (SqRad1<(ov2|ov2)) SqRad1=(ov2|ov2);
		//first simple rejection-test...
		if ( Sphere_Sphere(s,Sphere(middle,sqrt_tpl(SqRad1)))==0 ) return 0; //overlap not possible
		//...and now the hardcore-test!
		if ( (s.radius*s.radius)<Distance::Point_Triangle(s.center,t))	return 0;
		return 1; //sphere and triangle are overlapping
	}


	/*!
	*
	* we use the SEPARATING-AXIS-TEST for OBB/Plane overlap.
	*
	* Example:
	*  bool result=Overlap::OBB_Plane( pos,obb, plane );
	*
	*/
	inline bool OBB_Plane( const Vec3 &pos, const OBB &obb, const Plane &plane ) {
		//the new center-position in world-space
		Vec3 p	=	obb.m33*obb.c + pos;
		//extract the orientation-vectors from the columns of the 3x3 matrix
		//and scale them by the half-lengths
		Vec3 ax	= Vec3(obb.m33(0,0), obb.m33(1,0), obb.m33(2,0))*obb.h.x;
		Vec3 ay	= Vec3(obb.m33(0,1), obb.m33(1,1), obb.m33(2,1))*obb.h.y;
		Vec3 az	= Vec3(obb.m33(0,2), obb.m33(1,2), obb.m33(2,2))*obb.h.z;
		//check OBB against Plane, using the plane-normal as separating axis
		return	fabsf(plane|p) < (fabsf(plane.n|ax) + fabsf(plane.n|ay) + fabsf(plane.n|az));
	}


	/*!
	*
	* we use the SEPARATING AXIS TEST to check if a triangle and AABB overlap.
	*
	* Example:
	*  bool result=Overlap::AABB_Triangle( pos,aabb, tv0,tv1,tv2 );
	*
	*/
	inline bool AABB_Triangle ( const AABB &aabb,  const Vec3 &tv0, const Vec3 &tv1, const Vec3 &tv2 ) {

		//------ convert AABB into half-length AABB -----------
		Vec3 h	=	(aabb.max-aabb.min)*0.5f;	//calculate the half-length-vectors
		Vec3 c	=	(aabb.max+aabb.min)*0.5f;	//the center is relative to the PIVOT

		//move everything so that the boxcenter is in (0,0,0)
		Vec3 v0	=	tv0-c;
		Vec3 v1	=	tv1-c;
		Vec3 v2	=	tv2-c;

		//compute triangle edges
		Vec3 e0	=	v1-v0;
		Vec3 e1	=	v2-v1;
		Vec3 e2	=	v0-v2;

		//--------------------------------------------------------------------------------------
		//    use SEPARATING AXIS THEOREM to test overlap  between AABB and triangle
		//    cross-product(edge from triangle, {x,y,z}-direction),  this are 3x3=9 tests
		//--------------------------------------------------------------------------------------
		float min,max,p0,p1,p2,rad,fex,fey,fez;  
		fex = fabsf(e0.x);
		fey = fabsf(e0.y);
		fez = fabsf(e0.z);

		//AXISTEST_X01(e0.z, e0.y, fez, fey);
		p0 = e0.z*v0.y - e0.y*v0.z;  
		p2 = e0.z*v2.y - e0.y*v2.z;   
		if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} 
		rad = fez * h.y + fey * h.z; 
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Y02(e0.z, e0.x, fez, fex);
		p0 = -e0.z*v0.x + e0.x*v0.z;
		p2 = -e0.z*v2.x + e0.x*v2.z;
		if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}
		rad = fez * h.x + fex * h.z;
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Z12(e0.y, e0.x, fey, fex);
		p1 = e0.y*v1.x - e0.x*v1.y;
		p2 = e0.y*v2.x - e0.x*v2.y;
		if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}
		rad = fey * h.x + fex * h.y;
		if(min>rad || max<-rad) return 0;

		//-----------------------------------------------

		fex = fabsf(e1.x);
		fey = fabsf(e1.y);
		fez = fabsf(e1.z);
		//AXISTEST_X01(e1.z, e1.y, fez, fey);
		p0 = e1.z*v0.y - e1.y*v0.z;
		p2 = e1.z*v2.y - e1.y*v2.z;
		if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}
		rad = fez * h.y + fey * h.z;
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Y02(e1.z, e1.x, fez, fex);
		p0 = -e1.z*v0.x + e1.x*v0.z;
		p2 = -e1.z*v2.x + e1.x*v2.z;
		if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}
		rad = fez * h.x + fex * h.z;
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Z0(e1.y, e1.x, fey, fex);
		p0 = e1.y*v0.x - e1.x*v0.y;
		p1 = e1.y*v1.x - e1.x*v1.y;
		if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}
		rad = fey * h.x + fex * h.y;
		if(min>rad || max<-rad) return 0;

		//-----------------------------------------------

		fex = fabsf(e2.x);
		fey = fabsf(e2.y);
		fez = fabsf(e2.z);
		//AXISTEST_X2(e2.z, e2.y, fez, fey);
		p0 = e2.z*v0.y - e2.y*v0.z;
		p1 = e2.z*v1.y - e2.y*v1.z;
		if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}
		rad = fez * h.y + fey * h.z;
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Y1(e2.z, e2.x, fez, fex);
		p0 = -e2.z*v0.x + e2.x*v0.z;
		p1 = -e2.z*v1.x + e2.x*v1.z;
		if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}
		rad = fez * h.x + fex * h.z;
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Z12(e2.y, e2.x, fey, fex);
		p1 = e2.y*v1.x - e2.x*v1.y;
		p2 = e2.y*v2.x - e2.x*v2.y;
		if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}
		rad = fey * h.x + fex * h.y;
		if(min>rad || max<-rad) return 0;

		//the {x,y,z}-directions (actually, since we use the AABB of the triangle we don't even need to test these) 
		//first test overlap in the {x,y,z}-directions
		//find min, max of the triangle each direction, and test for overlap in that direction -- 
		//this is equivalent to testing a minimal AABB around the triangle against the AABB
		AABB taabb;
		FINDMINMAX(v0.x, v1.x, v2.x, taabb.min.x,taabb.max.x);
		FINDMINMAX(v0.y, v1.y, v2.y, taabb.min.y,taabb.max.y);
		FINDMINMAX(v0.z, v1.z, v2.z, taabb.min.z,taabb.max.z);

		//test in X-direction
		FINDMINMAX(v0.x, v1.x, v2.x, taabb.min.x,taabb.max.x);
		if(taabb.min.x>h.x || taabb.max.x<-h.x) return 0;

		//test in Y-direction
		FINDMINMAX(v0.y, v1.y, v2.y, taabb.min.y,taabb.max.y);
		if(taabb.min.y>h.y || taabb.max.y<-h.y) return 0;

		//test in Z-direction
		FINDMINMAX(v0.z, v1.z, v2.z, taabb.min.z,taabb.max.z);
		if(taabb.min.z>h.z || taabb.max.z<-h.z) return 0;

		//test if the box intersects the plane of the triangle
		//compute plane equation of triangle: normal*x+d=0
		Plane plane=GetPlane( (e0%e1), v0);

		Vec3 vmin,vmax;
		if(plane.n.x>0.0f) {  vmin.x=-h.x; vmax.x=+h.x; } 
		else { vmin.x=+h.x; vmax.x=-h.x; }
		if(plane.n.y>0.0f) {  vmin.y=-h.y; vmax.y=+h.y; } 
		else { vmin.y=+h.y; vmax.y=-h.y; }
		if(plane.n.z>0.0f) {	vmin.z=-h.z; vmax.z=+h.z; } 
		else {	vmin.z=+h.z;	vmax.z=-h.z;	}
		if( (plane|vmin) > 0.0f) return 0;
		if( (plane|vmax) < 0.0f) return 0;
		return 1;
	}



	/*!
	*
	* we use the SEPARATING AXIS TEST to check if a triangle and an OBB overlaps.
	*
	* Example:
	*  bool result=Overlap::OBB_Trinagle( pos1,obb1, tv0,tv1,tv2 );
	*
	*/
	inline bool OBB_Triangle( const Vec3 &pos, const OBB &obb,  const Vec3 &tv0, const Vec3 &tv1, const Vec3 &tv2 ) {

		Vec3 p	=	obb.m33*obb.c + pos;									//the new center-position in world-space

		//move everything so that the boxcenter is in (0,0,0)
		Vec3 v0	=	(tv0-p)*obb.m33; //pre-transform
		Vec3 v1	=	(tv1-p)*obb.m33; //pre-transform
		Vec3 v2	=	(tv2-p)*obb.m33; //pre-transform


		//compute triangle edges
		Vec3 e0	=	v1-v0;
		Vec3 e1	=	v2-v1;
		Vec3 e2	=	v0-v2;

		//--------------------------------------------------------------------------------------
		//    use SEPARATING AXIS THEOREM to test intersection between AABB and triangle
		//    cross-product(edge from triangle, {x,y,z}-direction),  this are 3x3=9 tests
		//--------------------------------------------------------------------------------------
		float min,max,p0,p1,p2,rad,fex,fey,fez;  
		fex = fabsf(e0.x);
		fey = fabsf(e0.y);
		fez = fabsf(e0.z);

		//AXISTEST_X01(e0.z, e0.y, fez, fey);
		p0 = e0.z*v0.y - e0.y*v0.z;  
		p2 = e0.z*v2.y - e0.y*v2.z;   
		if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} 
		rad = fez * obb.h.y + fey * obb.h.z; 
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Y02(e0.z, e0.x, fez, fex);
		p0 = -e0.z*v0.x + e0.x*v0.z;
		p2 = -e0.z*v2.x + e0.x*v2.z;
		if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}
		rad = fez * obb.h.x + fex * obb.h.z;
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Z12(e0.y, e0.x, fey, fex);
		p1 = e0.y*v1.x - e0.x*v1.y;
		p2 = e0.y*v2.x - e0.x*v2.y;
		if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}
		rad = fey * obb.h.x + fex * obb.h.y;
		if(min>rad || max<-rad) return 0;

		//-----------------------------------------------

		fex = fabsf(e1.x);
		fey = fabsf(e1.y);
		fez = fabsf(e1.z);
		//AXISTEST_X01(e1.z, e1.y, fez, fey);
		p0 = e1.z*v0.y - e1.y*v0.z;
		p2 = e1.z*v2.y - e1.y*v2.z;
		if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}
		rad = fez * obb.h.y + fey * obb.h.z;
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Y02(e1.z, e1.x, fez, fex);
		p0 = -e1.z*v0.x + e1.x*v0.z;
		p2 = -e1.z*v2.x + e1.x*v2.z;
		if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}
		rad = fez * obb.h.x + fex * obb.h.z;
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Z0(e1.y, e1.x, fey, fex);
		p0 = e1.y*v0.x - e1.x*v0.y;
		p1 = e1.y*v1.x - e1.x*v1.y;
		if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}
		rad = fey * obb.h.x + fex * obb.h.y;
		if(min>rad || max<-rad) return 0;

		//-----------------------------------------------

		fex = fabsf(e2.x);
		fey = fabsf(e2.y);
		fez = fabsf(e2.z);
		//AXISTEST_X2(e2.z, e2.y, fez, fey);
		p0 = e2.z*v0.y - e2.y*v0.z;
		p1 = e2.z*v1.y - e2.y*v1.z;
		if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}
		rad = fez * obb.h.y + fey * obb.h.z;
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Y1(e2.z, e2.x, fez, fex);
		p0 = -e2.z*v0.x + e2.x*v0.z;
		p1 = -e2.z*v1.x + e2.x*v1.z;
		if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}
		rad = fez * obb.h.x + fex * obb.h.z;
		if(min>rad || max<-rad) return 0;

		//AXISTEST_Z12(e2.y, e2.x, fey, fex);
		p1 = e2.y*v1.x - e2.x*v1.y;
		p2 = e2.y*v2.x - e2.x*v2.y;
		if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}
		rad = fey * obb.h.x + fex * obb.h.y;
		if(min>rad || max<-rad) return 0;

		//the {x,y,z}-directions (actually, since we use the AABB of the triangle we don't even need to test these) 
		//first test overlap in the {x,y,z}-directions
		//find min, max of the triangle each direction, and test for overlap in that direction -- 
		//this is equivalent to testing a minimal AABB around the triangle against the AABB
		AABB taabb;
		FINDMINMAX(v0.x, v1.x, v2.x, taabb.min.x,taabb.max.x);
		FINDMINMAX(v0.y, v1.y, v2.y, taabb.min.y,taabb.max.y);
		FINDMINMAX(v0.z, v1.z, v2.z, taabb.min.z,taabb.max.z);

		// test in X-direction
		FINDMINMAX(v0.x, v1.x, v2.x, taabb.min.x,taabb.max.x);
		if(taabb.min.x>obb.h.x || taabb.max.x<-obb.h.x) return 0;

		// test in Y-direction
		FINDMINMAX(v0.y, v1.y, v2.y, taabb.min.y,taabb.max.y);
		if(taabb.min.y>obb.h.y || taabb.max.y<-obb.h.y) return 0;

		// test in Z-direction
		FINDMINMAX(v0.z, v1.z, v2.z, taabb.min.z,taabb.max.z);
		if(taabb.min.z>obb.h.z || taabb.max.z<-obb.h.z) return 0;

		//test if the box overlaps the plane of the triangle
		//compute plane equation of triangle: normal*x+d=0
		Plane plane=GetPlane( (e0%e1), v0);

		Vec3 vmin,vmax;
		if(plane.n.x>0.0f) {  vmin.x=-obb.h.x; vmax.x=+obb.h.x; } 
		else { vmin.x=+obb.h.x; vmax.x=-obb.h.x; }
		if(plane.n.y>0.0f) {  vmin.y=-obb.h.y; vmax.y=+obb.h.y; } 
		else { vmin.y=+obb.h.y; vmax.y=-obb.h.y; }
		if(plane.n.z>0.0f) {	vmin.z=-obb.h.z; vmax.z=+obb.h.z; } 
		else {	vmin.z=+obb.h.z;	vmax.z=-obb.h.z;	}
		if( (plane|vmin) > 0.0f) return 0;
		if( (plane|vmax) < 0.0f) return 0;
		return 1;
	}









	/*!
	*
	* conventional method to check if two AABB's overlap.
	* both AABBs are assumed to be in the same space
	*
	* Example:
	*  bool result=Overlap::AABB_AABB( aabb1, aabb2 );
	*
	*/
	ILINE bool AABB_AABB( const AABB &aabb1, const AABB &aabb2 ) {
		if  (aabb1.min.x>=aabb2.max.x) return 0;
		if  (aabb1.min.y>=aabb2.max.y) return 0; 
		if 	(aabb1.min.z>=aabb2.max.z) return 0; 
		if  (aabb1.max.x<=aabb2.min.x) return 0;
		if  (aabb1.max.y<=aabb2.min.y) return 0;
		if  (aabb1.max.z<=aabb2.min.z) return 0; 
		return 1; //the aabb's overlap
	}

	/*!
	*
	* Conventional method to check if two AABB's overlap.
	* Both AABBs are in local object space. Used the position-vector 
	* to translate them into world-space
	*
	* Example:
	*  bool result=Overlap::AABB_AABB( pos1,aabb1, pos2,aabb2 );
	*
	*/
	ILINE bool AABB_AABB( const Vec3 &pos1,const AABB &aabb1,  const Vec3 &pos2,const AABB &aabb2 ) {
		AABB waabb1(aabb1.min+pos1,aabb1.max+pos1);
		AABB waabb2(aabb2.min+pos2,aabb2.max+pos2);
		return AABB_AABB( waabb1, waabb2 );
	}


	/*!
	*
	* conventional method to check if two AABB's overlap 
	* or if AABB1 is comletely inside AABB2.
	* both AABBs are assumed to be in the same space
	*
	* Example:
	*  bool result=Overlap::AABB_AABB_Inside( aabb1, aabb2 );
	*
	* return values:
	* 0x00 = no overlap         
	* 0x01 = both AABBs one overlap
	* 0x02 = AABB1 in inside AABB2 
	*/
	ILINE char AABB_AABB_Inside( const AABB& aabb1, const AABB& aabb2 ) {
		if ( AABB_AABB(aabb1,aabb2) ) {
			if  (aabb1.min.x<=aabb2.min.x) return 1;
			if  (aabb1.min.y<=aabb2.min.y) return 1; 
			if 	(aabb1.min.z<=aabb2.min.z) return 1; 
			if  (aabb1.max.x>=aabb2.max.x) return 1;
			if  (aabb1.max.y>=aabb2.max.y) return 1;
			if  (aabb1.max.z>=aabb2.max.z) return 1;
			//yes, its inside 
			return 2;
		}
		return 0;
	}


	/*!
	*
	* we use the SEPARATING AXIS TEST to check if two OBB's overlap.
	*
	* Example:
	*  bool result=Overlap::OBB_OBB( pos1,obb1, pos2,obb2 );
	*
	*/
	inline bool OBB_OBB ( const Vec3 &pos1,const OBB &obb1,  const Vec3 &pos2,const OBB &obb2 ) {

		//tranform obb2 in local space of obb1
		Matrix33 M=obb1.m33.T()*obb2.m33;

		//the new center-position in world-space
		Vec3 p1	=	obb1.m33*obb1.c + pos1;
		Vec3 p2	=	obb2.m33*obb2.c + pos2;

		//"t" is the transfer-vector from one center to the other
		Vec3 t		= (p2-p1)*obb1.m33;

		float ra,rb;

		//--------------------------------------------------------------------------
		//--  we use the vectors "1,0,0","0,1,0" and "0,0,1" as separating axis
		//--------------------------------------------------------------------------
		rb	=	fabsf(M(0,0)*obb2.h.x) + fabsf(M(0,1)*obb2.h.y) + fabsf(M(0,2)*obb2.h.z);
		if(   fabsf(t.x) > (fabsf(obb1.h.x)+rb) ) return 0;
		rb	=	fabsf(M(1,0)*obb2.h.x) + fabsf(M(1,1)*obb2.h.y) + fabsf(M(1,2)*obb2.h.z);
		if(   fabsf(t.y) > (fabsf(obb1.h.y)+rb) ) return 0;
		rb	=	fabsf(M(2,0)*obb2.h.x) + fabsf(M(2,1)*obb2.h.y) + fabsf(M(2,2)*obb2.h.z);
		if(   fabsf(t.z) > (fabsf(obb1.h.z)+rb) ) return 0;

		//--------------------------------------------------------------------------
		//--  we use the orientation-vectors "Mx","My" and "Mz" as separating axis
		//--------------------------------------------------------------------------
		ra	= fabsf(M(0,0)*obb1.h.x) + fabsf(M(1,0)*obb1.h.y) + fabsf(M(2,0)*obb1.h.z);
		if( fabsf(t|Vec3(M(0,0),M(1,0),M(2,0))) > (ra+obb2.h.x) ) return 0;
		ra	= fabsf(M(0,1)*obb1.h.x) + fabsf(M(1,1)*obb1.h.y) + fabsf(M(2,1)*obb1.h.z);
		if( fabsf(t|Vec3(M(0,1),M(1,1),M(2,1))) > (ra+obb2.h.y) ) return 0;
		ra	= fabsf(M(0,2)*obb1.h.x) + fabsf(M(1,2)*obb1.h.y) + fabsf(M(2,2)*obb1.h.z);
		if( fabsf(t|Vec3(M(0,2),M(1,2),M(2,2))) > (ra+obb2.h.z) ) return 0;

		//---------------------------------------------------------------------
		//----  using 9 cross products we generate new separating axis
		//---------------------------------------------------------------------
		ra = obb1.h.y*fabsf(M(2,0)) + obb1.h.z*fabsf(M(1,0));
		rb = obb2.h.y*fabsf(M(0,2)) + obb2.h.z*fabsf(M(0,1));
		if( fabsf(t.z*M(1,0)-t.y*M(2,0)) > (ra+rb) ) return 0;
		ra = obb1.h.y*fabsf(M(2,1)) + obb1.h.z*fabsf(M(1,1));
		rb = obb2.h.x*fabsf(M(0,2)) + obb2.h.z*fabsf(M(0,0));
		if( fabsf(t.z*M(1,1)-t.y*M(2,1)) > (ra+rb) ) return 0;
		ra = obb1.h.y*fabsf(M(2,2)) + obb1.h.z*fabsf(M(1,2));
		rb = obb2.h.x*fabsf(M(0,1)) + obb2.h.y*fabsf(M(0,0));
		if( fabsf(t.z*M(1,2)-t.y*M(2,2)) > (ra+rb) ) return 0;


		ra = obb1.h.x*fabsf(M(2,0)) + obb1.h.z*fabsf(M(0,0));
		rb = obb2.h.y*fabsf(M(1,2)) + obb2.h.z*fabsf(M(1,1));
		if( fabsf(t.x*M(2,0)-t.z*M(0,0)) > (ra+rb) ) return 0;
		ra = obb1.h.x*fabsf(M(2,1)) + obb1.h.z*fabsf(M(0,1));
		rb = obb2.h.x*fabsf(M(1,2)) + obb2.h.z*fabsf(M(1,0));
		if( fabsf(t.x*M(2,1)-t.z*M(0,1)) > (ra+rb) ) return 0;
		ra = obb1.h.x*fabsf(M(2,2)) + obb1.h.z*fabsf(M(0,2));
		rb = obb2.h.x*fabsf(M(1,1)) + obb2.h.y*fabsf(M(1,0));
		if( fabsf(t.x*M(2,2)-t.z*M(0,2)) > (ra+rb) ) return 0;


		ra = obb1.h.x*fabsf(M(1,0)) + obb1.h.y*fabsf(M(0,0));
		rb = obb2.h.y*fabsf(M(2,2)) + obb2.h.z*fabsf(M(2,1));
		if( fabsf(t.y*M(0,0)-t.x*M(1,0)) > (ra+rb) ) return 0;
		ra = obb1.h.x*fabsf(M(1,1)) + obb1.h.y*fabsf(M(0,1));
		rb = obb2.h.x*fabsf(M(2,2)) + obb2.h.z*fabsf(M(2,0));
		if( fabsf(t.y*M(0,1)-t.x*M(1,1)) > (ra+rb) ) return 0;
		ra = obb1.h.x*fabsf(M(1,2)) + obb1.h.y*fabsf(M(0,2));
		rb = obb2.h.x*fabsf(M(2,1)) + obb2.h.y*fabsf(M(2,0));
		if( fabsf(t.y*M(0,2)-t.x*M(1,2)) > (ra+rb) ) return 0;
		
		return 1; //no separating axis found, we have an overlap
	}






#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2
#define	PLANE_NON_AXIAL 3

	//! check if the point is inside a triangle
	inline bool	PointInTriangle(const Vec3& point, const Vec3& v0,const Vec3& v1,const Vec3& v2,const Vec3& normal)
	{

		float xt,yt;
		Vec3 nn;

		int p1,p2;

		nn = normal;
		nn.x = (float)fabs(nn.x);
		nn.y = (float)fabs(nn.y);
		nn.z = (float)fabs(nn.z);

		if ((nn.x>=nn.y) && (nn.x>=nn.z)) 
		{
			xt=point.y; yt=point.z;
			p1=PLANE_Y;p2=PLANE_Z;
		}
		else
			if ((nn.y>=nn.x) && (nn.y>=nn.z))
			{
				xt=point.x;yt=point.z;
				p1=PLANE_X;p2=PLANE_Z;
			}
			else
			{
				xt=point.x;yt=point.y;
				p1=PLANE_X;p2=PLANE_Y;
			}

			float Ax,Ay,Bx,By;
			float s;

			bool front=false;
			bool back=false;


			Ax=(v0)[p1];Bx=(v1)[p1];
			Ay=(v0)[p2];By=(v1)[p2];

			s=((Ay-yt)*(Bx-Ax)-(Ax-xt)*(By-Ay));

			if (s>=0) 
			{ 
				if (back) 
					return (false); 
				front=true;
			}
			else 
			{ 
				if (front) 
					return (false); 
				back=true; 
			}

			Ax=(v1)[p1];Bx=(v2)[p1];
			Ay=(v1)[p2];By=(v2)[p2];

			s=((Ay-yt)*(Bx-Ax)-(Ax-xt)*(By-Ay));

			if (s>=0) 
			{ 
				if (back) 
					return (false); 
				front=true;
			}
			else 
			{ 
				if (front) return (false); 
				back=true; 
			}

			Ax=(v2)[p1];Bx=(v0)[p1];
			Ay=(v2)[p2];By=(v0)[p2];

			s=((Ay-yt)*(Bx-Ax)-(Ax-xt)*(By-Ay));

			if (s>=0) 
			{ 
				if (back) 
					return (false); 
				front=true;
			}
			else 
			{ 
				if (front) 
					return (false); 
				back=true; 
			}

			return (true);
	}





}



#endif //overlap

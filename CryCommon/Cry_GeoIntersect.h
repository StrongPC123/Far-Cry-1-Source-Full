//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//	
//	File:Cry_GeoIntersect.h
//	Description: Common intersection-tests
//
//	History:
//	-March 15,2003: Created by Ivo Herzeg
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYINTERSECTION_H
#define CRYINTERSECTION_H


#if _MSC_VER > 1000
# pragma once
#endif

#include <Cry_Geo.h>

namespace Intersect {



inline bool Ray_Plane(const Ray &ray, const Plane &plane, Vec3 &output ) {
	float numer		= plane|ray.origin;
	float cosine	=	plane.n|ray.direction;
	//REJECTION 1: if "line-direction" is perpendicular to "plane-normal", an intersection is not possible!
	//REJECTION 2: we deal with single-sided planes. 
	//             if "line-direction" is pointing in the same direction as "the plane-normal", 
	//             an intersection is not possible!
	if (cosine > 0) return 0;					//normal is orthogonal to vector, cant intersect
	output				=	ray.origin+(ray.direction*(-numer/cosine));
	//skip, if cutting-point is "behind" ray.origin
	if (((output-ray.origin)|ray.direction)<0) return 0;
	return 1;														//intersection occured	
}

inline bool Line_Plane(const Line &line, const Plane &plane, Vec3 &output ) {
	double perpdist	= plane|line.pointonline;
	double cosine		=	plane.n|line.direction;
	//REJECTION 1: if "line-direction" is perpendicular to "plane-normal", an intersection is not possible!
	//REJECTION 2: we deal with single-sided planes. 
	//             if "line-direction" is pointing in the same direction as "the plane-normal", 
	//             an intersection is not possible!
	if (cosine > 0) return 0;
	//an intersection is possible: calculate the exact point!
	float pd_c			=	(float)(-perpdist/cosine);
	output					=	line.pointonline+(line.direction*pd_c);
	return 1;														//intersection occured	
}






/*
* calculates intersection between a line and a triangle.
* IMPORTANT: this is a single-sided intersection test. That means its not enough 
* that the triangle and line overlap, its also important that the triangle 
* is "visible" when you are looking along the line-direction.  
* 
* If you need a double-sided test, you'll have to call this function twice with 
* reversed order of triangle vertices.   
* 
* return values 
* if there is an intertection the functions return "true" and stores the  
* 3d-intersection point in "output". if the function returns "false" the value in 
* "output" is undefined  
* 
*/
inline bool Line_Triangle( const Line &line, const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, Vec3 &output ) {
	//find vectors for two edges sharing v0
	Vec3 edge_1 = v1-v0;
	Vec3 edge_2 = v2-v0;
	//begin calculating determinant - also used to calculate U parameter
	Vec3 pvec  =  line.direction % edge_1; 
	//if determinat is near zero, ray lies in plane of triangel 
	float det = edge_2 | pvec;
	if (det<=0) return 0;
	//calculate distance from v0 to ray origin
	Vec3 tvec=line.pointonline-v0;
	//calculate U parameter and test bounds
	float u=tvec | pvec;
	if (u<0.0f || u>det) return 0;
	//prepare to test V parameter
	Vec3 qvec=tvec % edge_2;
	//calculate V parameter and test bounds
	float v= (line.direction | qvec);
	if ( v<0.0f || (u+v)>det) return 0;
	//------------------------------------------------------
	//we have an intersection and now we can calculate t
	float t = (edge_1 | qvec) / det;
	//we use t als a scale parameter, to get the 3D-intersection point
	output = (line.direction*t)+line.pointonline;
	return 1;
}







/*
* calculates intersection between a ray and a triangle.
* IMPORTANT: this is a single-sided intersection test. That means its not sufficient 
* that the triangle and rayt overlap, its also important that the triangle 
* is "visible" when you from the origin along the ray-direction.  
* 
* If you need a double-sided test, you'll have to call this function twice with 
* reversed order of triangle vertices.   
* 
* return values 
* if there is an intertection the functions return "true" and stores the  
* 3d-intersection point in "output". if the function returns "false" the value in 
* "output" is undefined  
*/
inline bool Ray_Triangle( const Ray &ray, const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, Vec3 &output ) {
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
	if (u<0.0f || u>det) return 0;
	//prepare to test V parameter
	Vec3 qvec=tvec % edge_2;
	//calculate V parameter and test bounds
	float v= (ray.direction | qvec);
	if ( v<0.0f || (u+v)>det) return 0;
	//------------------------------------------------------
	//We have an intersection and we can calculate t
	float t = (edge_1 | qvec) / det;
	//we use t als a scale parameter, to get the 3D-intersection point
	output = (ray.direction*t)+ray.origin;
	//skip, if cutting-point is "behind" ray.origin
	if (((output-ray.origin)|ray.direction)<0) return 0;
	return 1;
}









/*!
*
* calculates intersection between a line-segment and a triangle.
* IMPORTANT: this is a single-sided intersection test. That means its not sufficient 
* that the triangle and line-segment overlap, its also important that the triangle 
* is "visible" when you are looking along the linesegment from "start" to "end".  
* 
* If you need a double-sided test, you'll have to call this function twice with 
* reversed order of triangle vertices.   
* 
* return values 
* if there is an intertection the the functions return "true" and stores the  
* 3d-intersection point in "output". if the function returns "false" the value in 
* "output" is undefined  
* 
*/
inline bool Lineseg_Triangle( const Lineseg &lineseg, const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, Vec3 &output ) {
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
	//If we ignore the limitations of linesegment, then we have an intersection and we calcultate t
	float t = (edge_1 | qvec) / det;
	//we use t als a scale parameter, to get the 3D-intersection point
	output = (direction*t)+lineseg.start;
	//skip, if lineseg and triangle are not overlapping
	if (((output-lineseg.start)|direction)<0) return 0;
	if (((output-lineseg.end)|direction)>0) return 0;
	return 1;
}





//----------------------------------------------------------------------------------
//--- 0x00 = no intersection                               --------------------------
//--- 0x01 = not possible   --
//--- 0x02 = not possible  --
//--- 0x03 = two intersection, lineseg has ENTRY and EXIT point  --
//----------------------------------------------------------------------------------

inline unsigned char Line_Sphere( const Line &line, const Sphere &s, Vec3 &i0, Vec3 &i1 ) {

	Vec3 end=line.pointonline+line.direction;

	float a = line.direction|line.direction;
	float b = (line.direction|(line.pointonline-s.center))*2.0f;
	float c = ((line.pointonline-s.center)|(line.pointonline-s.center)) - (s.radius*s.radius); 

	float desc = (b*b) - (4 * a *c); 

	unsigned char intersection=0;
	if (desc >= 0.0f)
	{
		float lamba0 = (-b - cry_sqrtf(desc)) / (2.0f * a); 
		//_stprintf(d3dApp.token,"lamba0: %20.12f",lamba0);
		//d3dApp.m_pFont->DrawText( 2, d3dApp.PrintY, D3DCOLOR_ARGB(255,255,255,0), d3dApp.token );	d3dApp.PrintY+=20;
		i0 = line.pointonline + ((end-line.pointonline)*lamba0); 
		intersection=1;

		float lamba1 = (-b + cry_sqrtf(desc))/(2.0f*a); 
		//_stprintf(d3dApp.token,"lamba1: %20.12f",lamba1);
		//d3dApp.m_pFont->DrawText( 2, d3dApp.PrintY, D3DCOLOR_ARGB(255,255,255,0), d3dApp.token );	d3dApp.PrintY+=20;
		i1 = line.pointonline + ((end-line.pointonline)*lamba1); 
		intersection|=2;
	}

	return intersection; 
}



//----------------------------------------------------------------------------------
//--- 0x00 = no intersection                               --------------------------
//--- 0x01 = not possible   --
//--- 0x02 = one intersection, lineseg has just an EXIT point but no ENTRY point (ls.start is inside the sphere)  --
//--- 0x03 = two intersection, lineseg has ENTRY and EXIT point  --
//----------------------------------------------------------------------------------

inline unsigned char Ray_Sphere( const Ray &ray, const Sphere &s, Vec3 &i0, Vec3 &i1 ) {
		Vec3 end=ray.origin+ray.direction;
		float a = ray.direction|ray.direction;
		float b = (ray.direction|(ray.origin-s.center))*2.0f;
		float c = ((ray.origin-s.center)|(ray.origin-s.center)) - (s.radius*s.radius); 

		float desc = (b*b) - (4 * a *c); 
				
		unsigned char intersection=0;
		if (desc >= 0.0f)
		{
			float lamba0 = (-b - cry_sqrtf(desc)) / (2.0f * a); 
			//	_stprintf(d3dApp.token,"lamba0: %20.12f",lamba0);
			//	d3dApp.m_pFont->DrawText( 2, d3dApp.PrintY, D3DCOLOR_ARGB(255,255,255,0), d3dApp.token );	d3dApp.PrintY+=20;
			if (lamba0>0.0f)	{
				i0 = ray.origin + ((end-ray.origin)*lamba0); 
				intersection=1;
			}

			float lamba1 = (-b + cry_sqrtf(desc))/(2.0f*a); 
			//	_stprintf(d3dApp.token,"lamba1: %20.12f",lamba1);
			//	d3dApp.m_pFont->DrawText( 2, d3dApp.PrintY, D3DCOLOR_ARGB(255,255,255,0), d3dApp.token );	d3dApp.PrintY+=20;
			if (lamba1>0.0f)	{
				i1 = ray.origin + ((end-ray.origin)*lamba1); 
				intersection|=2;
			}
	}
	return intersection; 
}

inline bool Ray_SphereFirst( const Ray &ray, const Sphere &s, Vec3 &intPoint )
{
	Vec3 p2;
	unsigned char res = Ray_Sphere( ray,s,intPoint,p2 );
	if (res == 2) {	intPoint = p2; }
	if (res > 1)	return true;
	return false;
}



//----------------------------------------------------------------------------------
//--- 0x00 = no intersection                               --------------------------
//--- 0x01 = one intersection, lineseg has just an ENTRY point but no EXIT point (ls.end is inside the sphere)  --
//--- 0x02 = one intersection, lineseg has just an EXIT point but no ENTRY point (ls.start is inside the sphere)  --
//--- 0x03 = two intersection, lineseg has ENTRY and EXIT point  --
//----------------------------------------------------------------------------------
inline unsigned char Lineseg_Sphere( const Lineseg &ls, const Sphere &s, Vec3 &i0, Vec3 &i1 ) {

		Vec3 dir = (ls.end - ls.start);

		float a = dir|dir;
		float b = (dir|(ls.start-s.center))*2.0f;
		float c = ((ls.start-s.center)|(ls.start-s.center)) - (s.radius*s.radius); 
		float desc = (b*b) - (4 * a *c); 
				
		unsigned char intersection=0;
		if (desc >= 0.0f)	{

		float lamba0 = (-b - cry_sqrtf(desc)) / (2.0f * a); 
		//_stprintf(d3dApp.token,"lamba0: %20.12f",lamba0);
		//d3dApp.m_pFont->DrawText( 2, d3dApp.PrintY, D3DCOLOR_ARGB(255,255,255,0), d3dApp.token );	d3dApp.PrintY+=20;
		if (lamba0 >0.0f)	{
			i0 = ls.start + ((ls.end-ls.start)*lamba0); 
			//skip, if 1st cutting-point is "in front" of ls.end
			if (((i0-ls.end)|dir)>0) return 0;
			intersection=0x01;
		}
					
		float lamba1 = (-b + cry_sqrtf(desc)) / (2.0f * a); 
		//_stprintf(d3dApp.token,"lamba1: %20.12f",lamba1);
		//d3dApp.m_pFont->DrawText( 2, d3dApp.PrintY, D3DCOLOR_ARGB(255,255,255,0), d3dApp.token );	d3dApp.PrintY+=20;
		if ( lamba1 > 0.0f)	{
			i1 = ls.start + ((ls.end-ls.start)*lamba1); 
			//skip, if 2nd cutting-point is "in front" of ls.end (=ls.end is inside sphere)
			if (((i1-ls.end)|dir)>0) return intersection;
			intersection|=0x02;
		}
	}
	return intersection; 
}



// line/sphere-intersection
// p1: 1st point of line
// p2: 2nd point of line
// p3: center of sphere
// r: radius of sphere
// i1: 1st intersection point
// i2: 2nd intersection point
// return number of intersections
inline int Lineseg_Sphere(Vec3 p1, Vec3 p2, Vec3 p3, float r, Vec3 &i1, Vec3 i2)
{
	float dx=p2.x-p1.x;
	float dy=p2.y-p1.y;
	float dz=p2.z-p1.z;
	float a=dx*dx+dy*dy+dz*dz;
	float b=2.0f*(dx*(p1.x-p3.x)+dy*(p1.y-p3.y)+dz*(p1.z-p3.z));
	float c=p3.Dot(p3)+p2.Dot(p2)-2.0f*(p3.x*p1.x+p3.y*p1.y+p3.z-p1.z)-r*r;
	float d=b*b-4.0f*a*c;
	if (d<0.0f)
		return 0;
	float u;
	u=(-b+(float)sqrt((double)d))/a*a;
	i1=p1+((p2-p1)*u);
	if (d==0.0)
		return 1;
	u=(-b-(float)sqrt((double)d))/a*a;
	i2=p1+((p2-p1)*u);
	return 2;
}


}; //CIntersect














#endif //vector

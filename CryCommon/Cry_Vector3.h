//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//	
//	File: Cry_Vector3.h
//	Description: Common vector class
//
//	History:
//	-Feb 27,2003: Created by Ivo Herzeg
//
//////////////////////////////////////////////////////////////////////

#ifndef VECTOR_H
#define VECTOR_H

#if _MSC_VER > 1000
# pragma once
#endif

#include "platform.h"
#include <math.h>
#include "Cry_Matrix.h" 

enum type_zero { zero };
enum type_min { MIN };
enum type_max { MAX };

extern const float gf_PI;

#define	VEC_EPSILON	( 0.01f )
#define DEG2RAD( a ) ( (a) * (gf_PI/180.0f) )
#define RAD2DEG( a ) ( (a) * (180.0f/gf_PI) )


#if defined(LINUX)
template <class F> struct Vec3_tpl;
template<class F> 
F GetLengthSquared( const Vec3_tpl<F> &v );
template<class F> 
F GetLength( const Vec3_tpl<F>& v );
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// class Vec3_tpl
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class F> struct Vec3_tpl
{
public:
	
	F x,y,z;

	Vec3_tpl() {};
	Vec3_tpl(type_zero) { x=y=z=0; }
	Vec3_tpl(type_min) { x=y=z=-100; }
	Vec3_tpl(type_max) { x=y=z=100; }

	Vec3_tpl( const F vx, const F vy, const F vz ) { x=vx; y=vy; z=vz; };

	//bracket operator
	void operator () ( const F vx,const F vy,const F vz ) { x=vx; y=vy; z=vz; };

	//f32* fptr=vec;
	operator F* ()					{ return (F*)this; }

	//f32 farray[3]={1,2,3};
	//Vec3 v=Vec3(farray);
	//
	//f32* fptr;
	//Vec3 newv=Vec3(fptr);
	//
	//Matrix44 tm=GetTranslationMat(Vec3(44,55,66));
	//Vec3 tvec=Vec3(tm[3]); //really dangerous! Should be replaced by "GetTranslation(tm)"
	template <class T>
	explicit Vec3_tpl(const T *src) { x=src[0]; y=src[1]; z=src[2]; }

	//CONSTRUCTOR: implement the copy/casting/assignement constructor:	
	template <class T>
	ILINE Vec3_tpl( const Vec3_tpl<T>& v ) {	x=(F)v.x;	y=(F)v.y;	z=(F)v.z; }

	//overload = operator to copy f64=doube or f32=f32
	//Vec3_tpl<f32>=Vec3_tpl<f32>
	//Vec3_tpl<f64>=Vec3_tpl<f64>
	template <class T>
	ILINE Vec3_tpl& operator=(const Vec3_tpl<T> &v) { x=v.x; y=v.y; z=v.z; return *this; }


	ILINE F &operator [] (int index)		  { assert(index>=0 && index<=2); return ((F*)this)[index]; }
	ILINE F operator [] (int index) const { assert(index>=0 && index<=2); return ((F*)this)[index]; }



	////////////////////////////////////////////////////////////////		
	//overloaded arithmetic operators
	////////////////////////////////////////////////////////////////		

	//three methods for a "dot-product" operation
	ILINE F Dot (const Vec3_tpl<F> &vec2)	const	{ return x*vec2.x + y*vec2.y + z*vec2.z; }
	//two methods for a "cross-product" operation
	ILINE Vec3_tpl<F> Cross (const Vec3_tpl<F> &vec2) const	{	return Vec3_tpl<F>( y*vec2.z  -  z*vec2.y,     z*vec2.x -    x*vec2.z,   x*vec2.y  -  y*vec2.x); 	}	


	ILINE Vec3_tpl<F> operator*(F k) const { return Vec3_tpl<F>(x*k,y*k,z*k); }
	ILINE Vec3_tpl<F> operator/(F k) const { k=(F)1.0/k; return Vec3_tpl<F>(x*k,y*k,z*k); }
	ILINE friend Vec3_tpl operator * (f32 f, const Vec3_tpl &vec)	{ return Vec3_tpl(f*vec.x, f*vec.y, f*vec.z); }


	ILINE Vec3_tpl<F>& operator *= (F k) { x*=k;y*=k;z*=k; return *this; }
	ILINE Vec3_tpl<F>& operator /= (F k) { k=(F)1.0/k; x*=k;y*=k;z*=k; return *this; }



	//flip vector
	ILINE Vec3_tpl<F> operator - ( void ) const { return Vec3_tpl<F>(-x,-y,-z); }
//	ILINE Vec3_tpl&	Negate() { x=-x; y=-y; z=-z; return *this; }	
  ILINE Vec3_tpl& Flip()   { x=-x; y=-y; z=-z; return *this; }
	ILINE Vec3_tpl& flip()   { x=-x; y=-y; z=-z; return *this; }



	ILINE friend bool operator ==(const Vec3_tpl<F> &v0, const Vec3_tpl<F> &v1)	{
		return ((v0.x==v1.x) && (v0.y==v1.y) && (v0.z==v1.z));
	}
	ILINE friend bool operator !=(const Vec3_tpl<F> &v0, const Vec3_tpl<F> &v1)	{ 
		return !(v0==v1); 
	}
	ILINE friend bool IsEquivalent(const Vec3_tpl<F>& v0, const Vec3_tpl<F>& v1, f32 epsilon ) {
		return  ((fabs_tpl(v0.x-v1.x) <= epsilon) &&	(fabs_tpl(v0.y-v1.y) <= epsilon)&&	(fabs_tpl(v0.z-v1.z) <= epsilon));	
	}
	ILINE friend bool IsEquivalent(const Vec3_tpl<F>& v0, const Vec3_tpl<F>& v1 ) {
		return( IsEquivalent( v0, v1, VEC_EPSILON ) );
	}

	ILINE bool IsZero() const {	return  x == 0 && y == 0 && z == 0;	}

	////////////////////////////////////////////////////////////////
	//common methods
	////////////////////////////////////////////////////////////////
	ILINE Vec3_tpl& zero() { x=y=z=0; return *this; }
	ILINE Vec3_tpl<F>& Set(const F xval,const F yval, const F zval) { x=xval; y=yval; z=zval; return *this; }

	//! calcultae the length of the vector
	ILINE F	Length() const { return sqrt_tpl(x*x+y*y+z*z); }		
	ILINE F	GetLength() const { return sqrt_tpl(x*x+y*y+z*z); }		
	ILINE F len() const { return sqrt_tpl(x*x+y*y+z*z); }

	//! calcultae the squared length of the vector
	ILINE F GetLengthSquared() const { return x*x+y*y+z*z; }
	ILINE F len2() const { return x*x +y*y + z*z; }



	//! normalize the vector and return the inverted length if successfull
	ILINE f32	Normalize() { 
		f32 fLen = (f32)GetLength();
		//assert(fLen>0.0f);
		if (fLen<0.00001f) return(0);  //no good idea! not everybody will check this
		f32 fInvLen=1.0f/fLen; 
		x*=fInvLen; y*=fInvLen; z*=fInvLen; 
		return fInvLen; 
	}

	//! return a normalized vector
	ILINE friend Vec3_tpl<F> GetNormalized( const Vec3_tpl<F> &v ) {
		F vlength = ::GetLength(v);	
		assert(vlength>0.0f);
		F ivlength=1.0f/vlength;
		return (v*ivlength);
	}


	ILINE Vec3_tpl& normalize() { 
		F rlen=sqrt_tpl(x*x+y*y+z*z); 
		//assert(rlen>0.00001f);
		if (rlen>0) { rlen=(F)1.0/rlen; x*=rlen;y*=rlen;z*=rlen; } 
		else Set(0,0,1); return *this; 
	}
	ILINE Vec3_tpl normalized() const { 
		F rlen=sqrt_tpl(x*x+y*y+z*z);
		//assert(rlen>0.00001f);
		if (rlen>0) { rlen=(F)1.0/rlen; return Vec3_tpl(x*rlen,y*rlen,z*rlen); } 
		else return Vec3_tpl(0,0,1);
	}

  ILINE Vec3_tpl GetNormalized() const { return normalized(); } // vlad: all get functions should begin from Get...

	ILINE f32	NormalizeFast() {
		f32 fLen = x*x + y*y + z*z;
		//assert(fLen>0.00001f);
		unsigned int *n1 = (unsigned int *)&fLen;
		unsigned int n = 0x5f3759df - (*n1 >> 1);
		f32 *n2 = (f32 *)&n;
		fLen = (1.5f - (fLen * 0.5f) * *n2 * *n2) * *n2;
		x*=fLen; y*=fLen; z*=fLen;
		return fLen;
	}



	ILINE friend F	GetSquaredDistance(const Vec3_tpl<F> &vec1, const Vec3_tpl<F> &vec2)	{		
		return (vec2.x-vec1.x)*(vec2.x-vec1.x)+(vec2.y-vec1.y)*(vec2.y-vec1.y)+(vec2.z-vec1.z)*(vec2.z-vec1.z);
	}
	ILINE friend F GetDistance(const Vec3_tpl<F> &vec1, const Vec3_tpl<F> &vec2) { 
		return  sqrt_tpl((vec2.x-vec1.x)*(vec2.x-vec1.x)+(vec2.y-vec1.y)*(vec2.y-vec1.y)+(vec2.z-vec1.z)*(vec2.z-vec1.z)); 
	}	

	ILINE F GetDistance(const Vec3_tpl<F> vec1) const {
		return  sqrt_tpl((x-vec1.x)*(x-vec1.x)+(y-vec1.y)*(y-vec1.y)+(z-vec1.z)*(z-vec1.z)); 
	}

	//! force vector length by normalizing it
	//! 08/26/2002 optimized a little by M.M.
	ILINE void  SetLen(const f32 fLen)	{ 
		f32 fLenMe = GetLengthSquared();
		if(fLenMe<0.00001f*0.00001f)return;
		fLenMe=fLen/(f32)sqrt((f64)fLenMe);
		x*=fLenMe; y*=fLenMe; z*=fLenMe;
	}


	// permutate coordinates so that z goes to new_z slot
	ILINE Vec3_tpl permutated(int new_z) const { return Vec3_tpl(*(&x+inc_mod3[new_z]), *(&x+dec_mod3[new_z]), *(&x+new_z)); }

	// returns volume of a box with this vector as diagonal 
	ILINE F volume() const { return x*y*z; }

	// returns a vector orthogonal to this one
	ILINE Vec3_tpl orthogonal() const {
		int i = isneg(square((F)0.9)*GetLengthSquared()-x*x);
		Vec3_tpl<F> res;
		res[i]=0; res[incm3(i)]=(*this)[decm3(i)]; res[decm3(i)]=-(*this)[incm3(i)];
		return res;
	}

	// returns a vector orthogonal to this one
	ILINE void SetOrthogonal( const Vec3_tpl<F>& v ) {
		int i = isneg(square((F)0.9)*v.GetLengthSquared()-v.x*v.x);
		(*this)[i]=0; (*this)[incm3(i)]= v[decm3(i)];	(*this)[decm3(i)]=-v[incm3(i)];
	}

	// returns a vector that consists of absolute values of this one's coordinates
	ILINE Vec3_tpl abs() const { return Vec3_tpl(fabs_tpl(x),fabs_tpl(y),fabs_tpl(z)); }


	//! check for min bounds
	ILINE void	CheckMin(const Vec3_tpl &other)	{ 
		if (other.x<x) x=other.x;
		if (other.y<y) y=other.y;
		if (other.z<z) z=other.z;
	}			

	//! check for max bounds
	ILINE void	CheckMax(const Vec3_tpl &other)	{
		if (other.x>x) x=other.x;
		if (other.y>y) y=other.y;
		if (other.z>z) z=other.z;
	}



	//this is a special case for CAngleAxis
	ILINE Vec3_tpl rotated(const Vec3_tpl &axis, F angle) const { 
		return rotated(axis,cos_tpl(angle),sin_tpl(angle)); 
	}
	ILINE Vec3_tpl rotated(const Vec3_tpl &axis, F cosa,F sina) const {
		Vec3_tpl zax = axis*(*this*axis); 
		Vec3_tpl xax = *this-zax; 
		Vec3_tpl yax = axis^xax;
		return xax*cosa + yax*sina + zax;
	}

	ILINE Vec3_tpl rotated(const Vec3_tpl &center,const Vec3_tpl &axis, F cosa,F sina) const { 
		return center+(*this-center).rotated(axis,cosa,sina); 
	}
	ILINE Vec3_tpl rotated(const Vec3_tpl &center,const Vec3_tpl &axis, F angle) const { 
		return center+(*this-center).rotated(axis,angle); 
	}

	//! set random normalized vector (=random position on unit sphere)
	void SetRandomDirection( void )
	{ 
		F Length2;
		do{
			x=( (F)rand() )/RAND_MAX*2-1;
			y=( (F)rand() )/RAND_MAX*2-1;
			z=( (F)rand() )/RAND_MAX*2-1;
			Length2=len2();
		} while(Length2>1.0f || Length2<0.0001f);

		F InvScale=1/sqrt_tpl(Length2);				// dividion by 0 is prevented

		x*=InvScale;y*=InvScale;z*=InvScale;
	}	


	/*!
	* Project a point/vector on a (virtual) plane 
	* Consider we have a plane going through the origin. 
	* Because d=0 we need just the normal. The vector n is assumed to be a unit-vector.
	* 
	* Example:
	*  Vec3 result=Vec3::CreateProjection( incident, normal );
	*/
	ILINE static Vec3_tpl CreateProjection( const Vec3_tpl& i, const Vec3_tpl& n ) { return i - n*(n|i); }

	/*!
	* Calculate a reflection vector. Vec3 n is assumed to be a unit-vector.
	* 
	* Example:
	*  Vec3 result=Vec3::CreateReflection( incident, normal );
	*/
	ILINE static Vec3_tpl CreateReflection( const Vec3_tpl& i, const Vec3_tpl &n ) {	return (n*(i|n)*2)-i; }

};

///////////////////////////////////////////////////////////////////////////////
// Typedefs                                                                  //
///////////////////////////////////////////////////////////////////////////////
typedef Vec3_tpl<f32>		Vec3;   //we will use only this throughout the project
typedef Vec3_tpl<real>	Vec3_f64;


typedef Vec3 Vec3d;  //obsolete! please use just Vec3
typedef Vec3 vectorf;
typedef Vec3_f64 vectorr;
typedef Vec3_tpl<int>		vectori;


inline Vec3_tpl<f32>::Vec3_tpl(type_min) { x=y=z=-3.3E38f; }
inline Vec3_tpl<f32>::Vec3_tpl(type_max) { x=y=z=3.3E38f; }
inline Vec3_tpl<f64>::Vec3_tpl(type_min) { x=y=z=-1.7E308; }
inline Vec3_tpl<f64>::Vec3_tpl(type_max) { x=y=z=1.7E308; }

template<class F> 
ILINE F GetLengthSquared( const Vec3_tpl<F> &v ) { return v.x*v.x + v.y*v.y + v.z*v.z; }
template<class F> 
ILINE F GetLength( const Vec3_tpl<F>& v ) { return sqrt_tpl(v.x*v.x + v.y*v.y + v.z*v.z); }


// dot product (2 versions)
template<class F1,class F2> 
ILINE F1 operator * (const Vec3_tpl<F1> &v0, const Vec3_tpl<F2> &v1) { 
	return (F1)(v0.x*v1.x+v0.y*v1.y+v0.z*v1.z); 
} 
template<class F1,class F2> 
ILINE F1 operator | (const Vec3_tpl<F1> &v0, const Vec3_tpl<F2> &v1) { 
	return v0.x*v1.x+v0.y*v1.y+v0.z*v1.z; 
} 

// cross product
template<class F1,class F2> 
ILINE Vec3_tpl<F1> operator ^ (const Vec3_tpl<F1> &v0, const Vec3_tpl<F2> &v1) {
	return Vec3_tpl<F1>(v0.y*v1.z-v0.z*v1.y, v0.z*v1.x-v0.x*v1.z, v0.x*v1.y-v0.y*v1.x); 
} 
template<class F1,class F2> 
ILINE Vec3_tpl<F1> operator % (const Vec3_tpl<F1> &v0, const Vec3_tpl<F2> &v1) {
	return Vec3_tpl<F1>(v0.y*v1.z-v0.z*v1.y, v0.z*v1.x-v0.x*v1.z, v0.x*v1.y-v0.y*v1.x); 
} 


//---------------------------------------------------------------------------

//vector addition
template<class F1,class F2>
ILINE Vec3_tpl<F1> operator + (const Vec3_tpl<F1> &v0, const Vec3_tpl<F2> &v1) {
	return Vec3_tpl<F1>(v0.x+v1.x, v0.y+v1.y, v0.z+v1.z);
}
//vector subtraction
template<class F1,class F2>
ILINE Vec3_tpl<F1> operator - (const Vec3_tpl<F1> &v0, const Vec3_tpl<F2> &v1) {
	return Vec3_tpl<F1>(v0.x-v1.x, v0.y-v1.y, v0.z-v1.z);
}


//---------------------------------------------------------------------------


//vector self-addition
template<class F1,class F2>
ILINE Vec3_tpl<F1>& operator += (Vec3_tpl<F1> &v0, const Vec3_tpl<F2> &v1) {
	v0.x+=v1.x; v0.y+=v1.y; v0.z+=v1.z; return v0;
}
//vector self-subtraction
template<class F1,class F2>
ILINE Vec3_tpl<F1>& operator -= (Vec3_tpl<F1> &v0, const Vec3_tpl<F2> &v1) {
	v0.x-=v1.x; v0.y-=v1.y; v0.z-=v1.z; return v0;
}

//! calculate the angle between two vectors in 3d (by M.M.)
//! /param invA direction of vector a (don't have to be normalized)
//! /param invB direction of vector b (don't have to be normalized)
//! /return angle in the range 0 to p radians. 
ILINE f32 CalcAngleBetween( const Vec3 &invA, const Vec3 &invB )	{
	f64 LengthQ=GetLength(invA)*GetLength(invB);
	if(LengthQ<0.01)LengthQ=0.01;
	return((f32)(acos((invA*invB)/LengthQ)));
}

// returns a vector orthogonal to this one
template<class F>
ILINE  Vec3_tpl<F> GetOrthogonal( const Vec3_tpl<F>& v ) {
	int i = isneg(square((F)0.9)*GetLengthSquared(v)-v.x*v.x);
	Vec3_tpl<F> res;
	res[i]=0;  res[incm3(i)]= v[decm3(i)]; res[decm3(i)]=-v[incm3(i)];
	return res;
}







///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Ang3_tpl
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



template <class F> 
struct Ang3_tpl : public Vec3_tpl<F>
{
	Ang3_tpl() {}

	Ang3_tpl(const Vec3 &v) { this->x=v.x;	this->y=v.y; this->z=v.z;	}  
	Ang3_tpl	&operator = (const Vec3 &v)  { this->x=v.x; this->y=v.y; this->z=v.z; return *this; 	}
	ILINE Ang3_tpl( const F vx, const F vy, const F vz )	{	this->x=vx; this->y=vy; this->z=vz;	}  
	//! normalize the vector ANGLE to -180, 180 range 
	ILINE void	Snap180()	
	{
		this->x = Snap_s180(this->x);
		this->y = Snap_s180(this->y);
		this->z = Snap_s180(this->z);
	}

	//! normalize the vector ANGLE to 0-360 range 
	ILINE void	Snap360()	
	{
		this->x = Snap_s360(this->x);
		this->y = Snap_s360(this->y);
		this->z = Snap_s360(this->z);
	}

	ILINE void  Rad2Deg()	{		this->x=RAD2DEG(this->x);		this->y=RAD2DEG(this->y);		this->z=RAD2DEG(this->z);	}
	//! convert from degrees to radians
	//ILINE void  Deg2Rad() {   this->x=DEG2RAD(this->x);   this->y=DEG2RAD(this->y); 	this->z=DEG2RAD(this->z);	}

	template<int SI,int SJ>
	ILINE void SetAnglesXYZ( const Matrix33_tpl<F,SI,SJ>& m ) 
	{
		//check if we have an orthonormal-base (assuming we are using a right-handed coordinate system)
		assert( IsEquivalent(m.GetOrtX(),m.GetOrtY()%m.GetOrtZ(),0.1f) );
		assert( IsEquivalent(m.GetOrtY(),m.GetOrtZ()%m.GetOrtX(),0.1f) );
		assert( IsEquivalent(m.GetOrtZ(),m.GetOrtX()%m.GetOrtY(),0.1f) );
		this->y = asin_tpl(MAX((F)-1.0,MIN((F)1.0,-m.data[SI*2+SJ*0])));
		if (fabs_tpl(fabs_tpl(this->y)-(F)(pi*0.5))<(F)0.01)	
		{
			this->x = 0;
			this->z = atan2_tpl(-m.data[SI*0+SJ*1],m.data[SI*1+SJ*1]);
		} else 	
		{
			this->x = atan2_tpl(m.data[SI*2+SJ*1], m.data[SI*2+SJ*2]);
			this->z = atan2_tpl(m.data[SI*1+SJ*0], m.data[SI*0+SJ*0]);
		}
	}
	template<int SI,int SJ>
	ILINE static Ang3_tpl<F> GetAnglesXYZ( const Matrix33_tpl<F,SI,SJ>& m ) {	Ang3_tpl<F> a; a.SetAnglesXYZ(m);	return a;	}
};

typedef Ang3_tpl<f32>		Ang3;

//ILINE Ang3 Rad2Deg(const Ang3& a)	{	return Ang3(RAD2DEG(a.x),RAD2DEG(a.y),RAD2DEG(a.z));	}
ILINE Ang3 Deg2Rad(const Ang3& a) {	return Ang3(DEG2RAD(a.x),DEG2RAD(a.y),DEG2RAD(a.z));	}

//! normalize the val to 0-360 range 
ILINE f32 Snap_s360( f32 val ) {
	if( val < 0.0f )
		val =f32( 360.0f + cry_fmod(val,360.0f));
	else
		if(val >= 360.0f)
			val =f32(cry_fmod(val,360.0f));
	return val;
}

//! normalize the val to -180, 180 range 
ILINE f32 Snap_s180( f32 val ) {
	if( val > -180.0f && val < 180.0f)
		return val;
	val = Snap_s360( val );
	if( val>180.0f )
		return -(360.0f - val);
	return val;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct CAngleAxis
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class F> struct AngleAxis_tpl {

	//! storage for the Angle&Axis coordinates.
	F angle; Vec3_tpl<F> axis;

	// default quaternion constructor
	AngleAxis_tpl( void ) { };
	AngleAxis_tpl( const f32 a, const f32 ax, const f32 ay, const f32 az ) {  angle=a; axis.x=ax; axis.y=ay; axis.z=az; }
	AngleAxis_tpl( const f32 a, const Vec3_tpl<F> &n ) { angle=a; axis=n; }
	void operator () ( const f32 a, const Vec3_tpl<F> &n ) {  angle=a; axis=n; }


	//CONSTRUCTOR: implement the copy/casting/assignement constructor:	
	AngleAxis_tpl( const AngleAxis_tpl<F>& aa ); //CAngleAxis aa=angleaxis
	//explicit AngleAxis_tpl( const Matrix33_tpl<F>& m );		//CAngleAxis aa=m33
	explicit AngleAxis_tpl( const Quaternion_tpl<F>& q );

	const Vec3_tpl<F> operator * ( const Vec3_tpl<F>& v ) const;

};

typedef AngleAxis_tpl<f32> AngleAxis;
typedef AngleAxis_tpl<f64> AngleAxis_f64;



///////////////////////////////////////////////////////////////////////////////
// CAngleAxis: Function & Operator declarations                                                     //
///////////////////////////////////////////////////////////////////////////////

/*!
*
* CAngleAxis copy constructor: CAngleAxis aa=mat33
* We take only the rotation of the 3x3 part 
*
*	Use with caution: this method suffers from some instability problems! 
*	If the rotation-radiant is near 0 or near 2pi we get an inaccurate result,
* because we have to normalize a very small vector! 
* If "angle=~0" then any axis will do, since there is no rotation!
* 
* Example:
*  Matrix33 mat33 = rotationXYZ33(0.5f, 2.5f, 1.5f);
*  CAngleAxis aa=mat33;
*/
/*template<class F> 
ILINE AngleAxis_tpl<F>::AngleAxis_tpl( const Matrix33_tpl<F> &m ) {

	angle		= acos_tpl((m(0,0) + m(1,1) + m(2,2)-1.0f)*0.5f);
	axis		= GetNormalized( Vec3_tpl<F>(m(2,1)-m(1,2), m(0,2)-m(2,0), m(1,0)-m(0,1)) );

	//if "angle=~0" any axis will do, since there is no rotation
	if (angle<0.00001f) { axis(1,0,0); return; }	
	//if "angle~=pi" we need a special approach to get the axis
	if (fabsf(angle-gf_PI)<0.00001f) {	
		//check if "m00" is the biggest element
		if	( (m(0,0)>m(1,1)) && (m(0,0)>m(2,2)) ) {
			axis.x = sqrtf( m(0,0) - m(1,1) - m(2,2) + 1.0f )/2;
			axis.y	= ( m(0,1) ) / (2*axis.x);
			axis.z	= ( m(0,2) ) / (2*axis.x);
			return;
		}
		//check if "m11" is the biggest element
		if	( (m(1,1)>m(0,0)) && (m(1,1)>m(2,2)) ) {
			axis.x = sqrtf( m(1,1) - m(0,0) - m(2,2) + 1.0f )/2;
			axis.y	= ( m(0,1) ) / (2*axis.x);
			axis.z	= ( m(1,2) ) / (2*axis.x);
			return;
		}
		//check if "m22" is the biggest element
		if	( (m(2,2)>m(0,0)) && (m(2,2)>m(1,1)) ) {
			axis.x = sqrtf( m(2,2) - m(0,0) - m(1,1) + 1.0f )/2;
			axis.y	= ( m(0,2) ) / (2*axis.x);
			axis.z	= ( m(1,2) ) / (2*axis.x);
			return;
		}
	}
}*/



/*!
*
* CAngleAxisn copy constructor; CAngleAxis aa=q
*
*	Use with caution: this method suffers from some instability problems! 
*	If the rotation-radiant is near 0 or near 2pi we get an inaccurate result,
* because we have to normalize a very small vector! 
* 
* Example:
*  CQuaternion q;
*  q.rotationXYZ(0.5f, 2.5f, 1.5f);
*  CAngleAxis aa=mat33;
*/
template<class F> 
ILINE AngleAxis_tpl<F>::AngleAxis_tpl( const Quaternion_tpl<F> &q ) {
	// quaternion is assumed to be a unit-quaternion.
	angle		= acos_tpl(q.w)*2;
	axis 		= q.v;
	F sine = sin_tpl(angle*0.5f);
	if ( sine ) { axis=q.v/sine; }
	// if  vector=0 then we assume this quaternion to be an identity-quaternion.
	// an identity quaternion is a neutral element, so any normalized axis will do.
	// we choose angle=0 and axis(1,0,0). Any arbitrary axis with zero-angle performs no rotation
	if(  (axis.x==0) && (axis.y==0) && (axis.z==0) )	{	angle=0; axis.x=1; axis.y=0; axis.z=0; }
}


/*!
*
* rotation of a vector by axis & angle
* 
* Example:
*  CAngleAxis aa(3.142f,0,1,0);
*  Vec3 v(33,44,55);
*	 Vec3 result = aa*v;
*/
template<class F> 
ILINE const Vec3_tpl<F> AngleAxis_tpl<F>::operator * ( const Vec3_tpl<F> &v ) const {
	Vec3_tpl<F> origin 	= axis*(axis|v);
	return 	origin +  (v-origin)*cos_tpl(angle)  +  (axis % v)*sin_tpl(angle);
}

















//////////////////////////////////////////////////////////////////////
struct Plane
{

	//plane-equation: n.x*x+n.y*y+n.z*z+d>0 is in front of the plane 
	Vec3	n;	//!< normal
	f32	d;	//!< distance

	//----------------------------------------------------------------	 

	Plane()	{	};

	ILINE Plane( const Plane &p ) {	n=p.n; d=p.d; }
	ILINE Plane( const Vec3 &normal, const f32 &distance ) {  n=normal; d=distance; }

	//! set normal and dist for this plane and  then calculate plane type
	ILINE void	Set(const Vec3 &vNormal,const f32 fDist)	{	
		n = vNormal; 
		d = fDist;
	}

	ILINE void SetPlane( const Vec3 &normal, const Vec3 &point ) { 
		n=normal; 
		d=-(point | normal); 
	}

	ILINE friend Plane GetPlane(  const Vec3 &normal, const Vec3 &point ) {  
		return Plane( normal,-(point|normal)  );
	}

	/*!
	*
	* Constructs the plane by tree given Vec3s (=triangle) 
	*
	* Example 1:
	* \code
	*  Vec3 v0(1,2,3),v1(4,5,6),v2(6,5,6);
	*  Plane  plane;
	*  plane.CalculatePlane(v0,v1,v2);
	* \endcode
	*
	* Example 2:
	* \code
	*  Vec3 v0(1,2,3),v1(4,5,6),v2(6,5,6);
	*  Plane  plane=CalculatePlane(v0,v1,v2);
	* \endcode
	*
	*/
	ILINE void SetPlane( const Vec3 &v0, const Vec3 &v1, const Vec3 &v2 ) {  
		n = GetNormalized((v1-v0)%(v0-v2));	//vector cross-product
		d	=	-(n | v0);				//calculate d-value
	}
	ILINE friend Plane GetPlane( const Vec3 &v0, const Vec3 &v1, const Vec3 &v2 ) {  
		Plane p;
		p.n = GetNormalized((v1-v0)%(v0-v2)); //vector cross-product
		p.d	=	-(p.n | v0);			 //calculate d-value
		return p;
	}

	/*!
	*
	* Computes signed distance from point to plane.
	* This is the standart plane-equation: d=Ax*By*Cz+D.
	* The normal-vector is assumed to be normalized.
	* 
	* Example:
	*  Vec3 v(1,2,3);
	*  Plane  plane=CalculatePlane(v0,v1,v2);
	*  f32 distance = plane|v;
	*
	*/
	ILINE f32 operator | ( const Vec3 &point ) const { return (n | point) + d; }


	//! check for equality between two planes
	ILINE  friend	bool operator ==(const Plane &p1, const Plane &p2) {
		if (fabsf(p1.n.x-p2.n.x)>0.0001f) return (false);
		if (fabsf(p1.n.y-p2.n.y)>0.0001f) return (false);
		if (fabsf(p1.n.z-p2.n.z)>0.0001f) return (false);
		if (fabsf(p1.d-p2.d)<0.01f) return(true);
		return (false);
	}


//-------------------------------------------------------------------------------------
//---------old stuff-------------------------------------------------------------------
//---------dont use, many bugs, danger danger -----------------------------------------
//-------------------------------------------------------------------------------------

	//! calc the plane giving 3 points
  //DANGER, DANGER! calculation of d-value is wrong 
	void	CalcPlane(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2) {
		n = (p0-p1)^(p2-p1);
		n.Normalize();
		d = (p0*n);
	}



	//! Makes the plane by 3 given points
	void Init(const Vec3 & v1, const Vec3 & v2, const Vec3 & v3)
	{
		Vec3 u, v, t;
		u = v2 - v1;
		v = v2 - v3;
		t = u ^ v;
		t.Normalize();
		n = t;
		d = t*v1;
	}

	//! same as above for const members
	ILINE f32	DistFromPlane(const Vec3 &vPoint) const	{
		return (n*vPoint-d);
	}

	Vec3 MirrorVector(Vec3& i)  {
		return i - n*((n|i)*2);
	}

	Vec3 MirrorPosition(Vec3& i) {
		return i - n * (2.f * ((n|i) - d));
	}

};



//! calculate 2 vector that form a orthogonal base with a given input vector (by M.M.)
//! /param invDirection input direction (has to be normalized)
//! /param outvA first output vector that is perpendicular to the input direction
//! /param outvB second output vector that is perpendicular the input vector and the first output vector
ILINE void GetOtherBaseVec( const Vec3 &invDirection, Vec3 &outvA, Vec3 &outvB )
{
	if(invDirection.z<-0.5f || invDirection.z>0.5f)
	{
		outvA.x=invDirection.z;
		outvA.y=invDirection.y;
		outvA.z=-invDirection.x;
	}
	else
	{
		outvA.x=invDirection.y;
		outvA.y=-invDirection.x;
		outvA.z=invDirection.z;
	}
	outvB = invDirection.Cross(outvA);
	outvB.Normalize();
	outvA = invDirection.Cross(outvB);

	// without this normalization the results are not good enouth (Cross product introduce a errors)
	outvA.Normalize();
}



#endif //vector

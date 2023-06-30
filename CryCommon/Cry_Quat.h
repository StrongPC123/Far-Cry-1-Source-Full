//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//	
//	File:Cry_Quat.h
//	Description: Common quaternion class
//
//	History:
//	-Feb 27,2003: Created by Ivo Herzeg
//
//////////////////////////////////////////////////////////////////////

#ifndef _CRYQUAT_H
#define _CRYQUAT_H

#include "platform.h"
#include "Cry_Vector2.h"
#include "Cry_Vector3.h"
#include "Cry_Matrix.h"


#if defined(LINUX)
#undef assert
#define assert(exp) (void)( (exp) || (printf("Assert: ' %s ' has failed\n", #exp), 0) )
#endif


///////////////////////////////////////////////////////////////////////////////
// Typedefs                                                                  // 
///////////////////////////////////////////////////////////////////////////////
typedef Quaternion_tpl<f32>	CryQuat;
#ifndef MAX_API_NUM
typedef Quaternion_tpl<f32>	Quat;
typedef Quaternion_tpl<real>	Quat_f64;
#endif
typedef Quaternion_tpl<f32> quaternionf;
typedef Quaternion_tpl<real>	quaternion;




///////////////////////////////////////////////////////////////////////////////
// Definitions                                                               //
///////////////////////////////////////////////////////////////////////////////

#define M00 data[SI*0+SJ*0]
#define M10 data[SI*1+SJ*0]
#define M20 data[SI*2+SJ*0]
#define M30 data[SI*3+SJ*0]
#define M01 data[SI*0+SJ*1]
#define M11 data[SI*1+SJ*1]
#define M21 data[SI*2+SJ*1]
#define M31 data[SI*3+SJ*1]
#define M02 data[SI*0+SJ*2]
#define M12 data[SI*1+SJ*2]
#define M22 data[SI*2+SJ*2]
#define M32 data[SI*3+SJ*2]
#define M03 data[SI*0+SJ*3]
#define M13 data[SI*1+SJ*3]
#define M23 data[SI*2+SJ*3]
#define M33 data[SI*3+SJ*3]



#define m_00 m.data[SI1*0+SJ1*0]
#define m_10 m.data[SI1*1+SJ1*0]
#define m_20 m.data[SI1*2+SJ1*0]
#define m_30 m.data[SI1*3+SJ1*0]
#define m_01 m.data[SI1*0+SJ1*1]
#define m_11 m.data[SI1*1+SJ1*1]
#define m_21 m.data[SI1*2+SJ1*1]
#define m_31 m.data[SI1*3+SJ1*1]
#define m_02 m.data[SI1*0+SJ1*2]
#define m_12 m.data[SI1*1+SJ1*2]
#define m_22 m.data[SI1*2+SJ1*2]
#define m_32 m.data[SI1*3+SJ1*2]
#define m_03 m.data[SI1*0+SJ1*3]
#define m_13 m.data[SI1*1+SJ1*3]
#define m_23 m.data[SI1*2+SJ1*3]
#define m_33 m.data[SI1*3+SJ1*3]


#if defined(LINUX)
template<class F,int SI,int SJ> 
Quaternion_tpl<F> GetQuatFromMat33(const Matrix33_tpl<F,SI,SJ>& m);
#endif


//----------------------------------------------------------------------
// Quaternion
//----------------------------------------------------------------------
template <class F> struct Quaternion_tpl
{

	Vec3_tpl<F> v;	F w;

//-------------------------------

	//constructors
	Quaternion_tpl()	{	w=1;	v(0,0,0);	}
	Quaternion_tpl( F W,F X,F Y,F Z )	{ w = W; v.x=X; v.y=Y; v.z=Z; }
	Quaternion_tpl( const F angle, const Vec3_tpl<F> &axis);


	template<class F1,int SI,int SJ> 
	explicit ILINE Quaternion_tpl(const Matrix33_tpl<F1,SI,SJ>& m)	{	*this=GetQuatFromMat33(m); }
	template<class T>	
	explicit ILINE Quaternion_tpl(const Matrix34_tpl<T>& m)	{	*this=GetQuatFromMat33(Matrix33(m));	} 
	template<class F1,int SI,int SJ>	
	explicit ILINE Quaternion_tpl(const Matrix44_tpl<F1,SI,SJ>& m) { *this=GetQuatFromMat33(Matrix33_tpl<f32,3,1>(m));	} 


	//CONSTRUCTOR: implement the copy/casting/assignement constructor:	
	template <class T>
	ILINE Quaternion_tpl( const Quaternion_tpl<T>& q ) {	w=q.w;  v.x=q.v.x; v.y=q.v.y;	v.z=q.v.z; }


	//assignement of equal types
	ILINE Quaternion_tpl& operator=(const Quaternion_tpl<F>& q)	{	
		w=q.w; v=q.v; return *this; 	
	}
	//double=f32
	template<typename U> ILINE Quaternion_tpl& operator=(const U& q)	{ 	
		w=(F)q.w; v.x=(F)q.v.x; v.y=(F)q.v.y;  v.z=(F)q.v.z; return *this; 	
	}

/*	declared as static but never used or implemented as static member functions, Linux gcc don't like it too
#ifndef WIN64
	//dot-product
	template<typename F1,typename F2>	static f32 operator | (const Quaternion_tpl<F1>& q, const Quaternion_tpl<F2>& p);

	//multiplication  operator
	template<typename F1,typename F2>	static Quaternion_tpl<F1> operator * (const Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p); 
	template<typename F1,typename F2>	static void operator*=(Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p); 

	//division operator
	template<typename F1,typename F2> static Quaternion_tpl<F1> operator / (const Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p); 
	template<typename F1,typename F2> static void operator /= (Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p); 

	//addition operator
	template<typename F1,typename F2>	static Quaternion_tpl<F1> operator+(const Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p); 
	template<typename F1,typename F2>	static void operator+=(Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p); 

	//subtraction operator
	template<typename F1,typename F2>	static Quaternion_tpl<F1> operator - (const Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p);
	template<typename F1,typename F2>	static void operator-=(Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p);

	//rotate a vector by a quaternion
	template<typename F1,typename F2>	static Vec3_tpl<F2> operator*(const Quaternion_tpl<F1> &q, const Vec3_tpl<F2> &v);
	template<typename F1,typename F2>	static Vec3_tpl<F1> operator*(const Vec3_tpl<F1> &v, const Quaternion_tpl<F2> &q);
#endif
*/
	//multiplication by a scalar
	//friend Quaternion_tpl<F> operator * ( const Quaternion_tpl<F> &q, f32 t );
	//friend Quaternion_tpl<F> operator * ( f32 t, const Quaternion_tpl<F> &q );
	void operator *= (F op) {	w*=op; v*=op;	}

	//negate quaternion. dont confuse this with quaternion-inversion.
	Quaternion_tpl<F>	operator - () const { return Quaternion_tpl<F>( -w,-v ); };

	Quaternion_tpl<F> operator ! () const;
	




	void	SetIdentity(void);
	static Quaternion_tpl<F> GetIdentity(void);

	void SetRotationAA(F rad, const Vec3_tpl<F> &axis);
  static Quaternion_tpl<F> GetRotationAA(F rad, const Vec3_tpl<F> &axis);

	void SetRotationAA(F cosha, F sinha, const Vec3_tpl<F> &axis);
	static Quaternion_tpl<F> GetRotationAA(F cosha, F sinha, const Vec3_tpl<F> &axis);

	void SetRotationV0V1(const Vec3_tpl<F>& v0, const Vec3_tpl<F>& v1);
	static Quaternion_tpl<F> GetRotationV0V1(const Vec3_tpl<F>& v0, const Vec3_tpl<F>& v1);

	void SetRotationXYZ(const Ang3 &a);
	static Quaternion_tpl<F> GetRotationXYZ(const Ang3 &a);


	void Invert( void );
	static Quaternion_tpl<F> GetInverted(Quaternion_tpl<F>& q);

	void	Normalize(void);
	static Quaternion_tpl<F> GetNormalized( const Quaternion_tpl<F> &q );

	static F GetLength( const Quaternion_tpl<F> &q );

};




//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------










	/*!
	* Constructor using a scalar and a vector 
	*
	* Example:
	*   Vec3 vec;
	*   Quaternion quat = quaternionf( 0.5f, vec );
	*/
	template<class F>	
	ILINE Quaternion_tpl<F>::Quaternion_tpl( const F angle, const Vec3_tpl<F> &axis) {	w=angle; v=axis; 	}




/*!
*
* The "inner product" or "dot product" operation.
*
* calculate the "inner product" between two Quaternion.
* If both Quaternion are unit-quaternions, the result is the cosine: p*q=cos(angle)
* 
* Example:
*  Quaternion p(1,0,0,0),q(1,0,0,0);
*  f32 cosine = ( p | q );
*
*/
template<typename F1,typename F2>	
ILINE f32 operator | (const Quaternion_tpl<F1>& q, const Quaternion_tpl<F2>& p) { 
	return (q.v.x*p.v.x + q.v.y*p.v.y + q.v.z*p.v.z + q.w*p.w); 
} 


/*!
*  multiplication operator
*
*  Example 1:
*   Quaternion p(1,0,0,0),q(1,0,0,0);
*   Quaternion result=p*q;
*
*  Example 2: (self-multiplication)
*   Quaternion p(1,0,0,0),q(1,0,0,0);
*   Quaternion p*=q;
*
*/
template<class F1,class F2> 
ILINE Quaternion_tpl<F1> operator * (const Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p) { 
	return Quaternion_tpl<F1>(q.w*p.w-(q.v|p.v),  q.v*p.w + q.w*p.v + (q.v%p.v) ); 
}
template<class F1,class F2>
ILINE void operator *= (Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p) { 
	F1 s0=q.w; q.w=q.w*p.w-(q.v|p.v); q.v=p.v*s0+q.v*p.w+(q.v%p.v); 
}

/*!
*  division operator
*
*  Example 1:
*   Quaternion p(1,0,0,0),q(1,0,0,0);
*   Quaternion result=p/q;
*
*  Example 2: (self-division)
*   Quaternion p(1,0,0,0),q(1,0,0,0);
*   Quaternion p/=q;
*
*/
template<class F1,class F2> 
ILINE Quaternion_tpl<F1> operator / (const Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p) { 
	return (!p*q); 
}
template<class F1,class F2>
ILINE void operator /= (Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p) { 
	q=(!p*q);
}


/*!
*  addition operator
*
*  Example:
*   Quaternion p(1,0,0,0),q(1,0,0,0);
*   Quaternion result=p+q;
*
*  Example:(self-addition operator)
*   Quaternion p(1,0,0,0),q(1,0,0,0);
*   Quaternion p-=q;
*
*/
template<class F1,class F2> 
ILINE Quaternion_tpl<F1> operator+(const Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p) { 
	return Quaternion_tpl<F1>( q.w+p.w, q.v+p.v ); 
}
template<class F1,class F2> 
ILINE void operator+=(Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p) { 
	q.w+=p.w; q.v+=p.v; 
}



/*!
*  subtraction operator
*
*  Example:
*   Quaternion p(1,0,0,0),q(1,0,0,0);
*   Quaternion result=p-q;
*
*  Example: (self-subtraction operator)
*   Quaternion p(1,0,0,0),q(1,0,0,0);
*   Quaternion p-=q;
*
*/
template<class F1,class F2> 
ILINE Quaternion_tpl<F1> operator-(const Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p) { 
	return Quaternion_tpl<F1>( q.w-p.w, q.v-p.v); 
}
template<class F1,class F2> 
ILINE void operator-=(Quaternion_tpl<F1> &q, const Quaternion_tpl<F2> &p) { 
	q.w-=p.w; q.v-=p.v; 
}


//! Scale quaternion free function.
template <typename F1>
ILINE Quaternion_tpl<F1>	operator * ( const Quaternion_tpl<F1> &q, f32 t ) {
	return Quaternion_tpl<F1>( q.w*t, q.v*t );
};
//! Scale quaternion free function.
template <typename F1>
ILINE Quaternion_tpl<F1>	operator *( f32 t, const Quaternion_tpl<F1> &q ) {
	return Quaternion_tpl<F1>( t*q.w, t*q.v );
};



/*!
*
* multiplication operator with a Vec3 (3D rotations with quaternions)
* 
* Example:
*  Quaternion q(1,0,0,0);
*  Vec3 v(33,44,55);
*	 Vec3 result = q*v;
*/
template<class F1,class F2>
ILINE Vec3_tpl<F2> operator*(const Quaternion_tpl<F1> &q, const Vec3_tpl<F2> &v) {
#if !defined(LINUX)
	assert((fabs_tpl(1-(q|q)))<0.05); //check if unit-quaternion
#endif
	F1 vxvx=q.v.x*q.v.x;
	F1 vzvz=q.v.z*q.v.z;
	F1 vyvy=q.v.y*q.v.y; 
	F1 vxvy=q.v.x*q.v.y;
	F1 vxvz=q.v.x*q.v.z;
	F1 vyvz=q.v.y*q.v.z; 
	F1 svx=q.w*q.v.x,svy=q.w*q.v.y,svz=q.w*q.v.z;
	Vec3_tpl<F2> res;
	res.x = v.x*(1-(vyvy+vzvz)*2) + v.y*(vxvy-svz)*2      + v.z*(vxvz+svy)*2;
	res.y = v.x*(vxvy+svz)*2      + v.y*(1-(vxvx+vzvz)*2) + v.z*(vyvz-svx)*2;
	res.z = v.x*(vxvz-svy)*2      + v.y*(vyvz+svx)*2      + v.z*(1-(vxvx+vyvy)*2);
	return res;
}


/*!
* multiplication operator with a Vec3 (3D rotations with quaternions)
* 
* Example:
*  Quaternion q(1,0,0,0);
*  Vec3 v(33,44,55);
*	 Vec3 result = v*q;
*/
template<class F1,class F2>
ILINE Vec3_tpl<F1> operator*(const Vec3_tpl<F1> &v, const Quaternion_tpl<F2> &q) {
#if !defined(LINUX)
	assert((fabs_tpl(1-(q|q)))<0.001); //check if unit-quaternion
#endif
	F2 vxvx=q.v.x*q.v.x;
	F2 vzvz=q.v.z*q.v.z;
	F2 vyvy=q.v.y*q.v.y; 
	F2 vxvy=q.v.x*q.v.y;
	F2 vxvz=q.v.x*q.v.z;
	F2 vyvz=q.v.y*q.v.z; 
	F2 svx=q.w*q.v.x,svy=q.w*q.v.y,svz=q.w*q.v.z;
	Vec3_tpl<F1> res;
	res.x = v.x*(1-(vyvy+vzvz)*2) + v.y*(vxvy+svz)*2      + v.z*(vxvz-svy)*2;
	res.y = v.x*(vxvy-svz)*2      + v.y*(1-(vxvx+vzvz)*2) + v.z*(vyvz+svx)*2;
	res.z = v.x*(vxvz+svy)*2      + v.y*(vyvz-svx)*2      + v.z*(1-(vxvx+vyvy)*2);
	return res;
}








	/*!
	*
	* invert quaternion.
	* 
	* Example 1:
	*  Quaternion q=GetRotationXYZ<f32>(Ang3(1,2,3));
	*  Quaternion result = !q;
	* 
	*  Quaternion result = GetInverted(q);
	* 
	*  q.Invert();
	* 
	*/
	template<class F> ILINE Quaternion_tpl<F> Quaternion_tpl<F>::operator ! () const { 
#if !defined(LINUX)
		assert((fabs_tpl(1-(*this|*this)))<0.001); //check if unit-quaternion
#endif
		return Quaternion_tpl(w,-v); 
	}
	template<class F>	ILINE void Quaternion_tpl<F>::Invert( void ) { *this=!*this;	}
	template<class F> ILINE Quaternion_tpl<F> Quaternion_tpl<F>::GetInverted(Quaternion_tpl<F>& q) { return !q; } 
	template<class F>	ILINE Quaternion_tpl<F> GetInverted(Quaternion_tpl<F>& q) { return !q; } 



/*!
 *
 * Create rotation-quaternion around an arbitrary axis (=CVector).
 * This is an implemetation of "Eulers Theorem". the axis is assumed to be nomalised.
 *
 * Example:
 *		Quaternion_tpl<f32> q=GetRotationAA( 2.14255f, GetNormalized(Vec3(1,2,3)) );
 *		q.SetRotationAA( 2.14255f, GetNormalized(Vec3(1,2,3)) );
 */
template<class F> 
ILINE void Quaternion_tpl<F>::SetRotationAA(F rad, const Vec3_tpl<F> &axis) { F cs[2]; sincos_tpl( rad*(F)0.5, cs);	SetRotationAA(cs[0],cs[1], axis); }
template<class F> 
ILINE Quaternion_tpl<F> Quaternion_tpl<F>::GetRotationAA(F rad, const Vec3_tpl<F> &axis) { 	Quaternion_tpl<F> q;	q.SetRotationAA(rad,axis); 	return q;	}
template<class F> 
ILINE Quaternion_tpl<F> GetRotationAA(F rad, const Vec3_tpl<F> &axis) {	Quaternion_tpl<F> q;	q.SetRotationAA(rad,axis); 	return q;	}


template<class F>ILINE void Quaternion_tpl<F>::SetRotationAA(F cosha, F sinha, const Vec3_tpl<F> &axis)	{ 
#if !defined(LINUX)
	assert((fabs_tpl(1-(axis|axis)))<0.001); //check if unit-vector
#endif
	w=cosha; v=axis*sinha; 
}
template<class F> 
ILINE Quaternion_tpl<F> Quaternion_tpl<F>::GetRotationAA(F cosha, F sinha, const Vec3_tpl<F> &axis) { Quaternion_tpl<F> q;	q.SetRotationAA(cosha,sinha,axis); 	return q;	}
template<class F> 
ILINE Quaternion_tpl<F> GetRotationAA(F cosha, F sinha, const Vec3_tpl<F> &axis) { Quaternion_tpl<F> q;	q.SetRotationAA(cosha,sinha,axis); 	return q;	}


/*!
 *
 * Create rotation-quaternion that rotates from one vector to another.
 * Both vectors are assumed to be nomalised.
 *
 * Example:
 *		Quaternion_tpl<f32> q=GetRotationV0V1( GetNormalized(Vec3(3,1,7)), GetNormalized(Vec3(1,2,3)) );
 *		q.SetRotationV0V1( GetNormalized(Vec3(3,1,7)), GetNormalized(Vec3(1,2,3)) );
 */
template<class F> 
void Quaternion_tpl<F>::SetRotationV0V1(const Vec3_tpl<F>& v0, const Vec3_tpl<F>& v1)	
{ 
#if !defined(LINUX)
	assert((fabs_tpl(1-(v0|v0)))<0.001); //check if unit-vector
	assert((fabs_tpl(1-(v1|v1)))<0.001); //check if unit-vector
#endif
	w=sqrt_tpl(max((F)0.0,(1+v0*v1)*(F)0.5)); 
	if (w>0.001) {
		v=v0^v1; v*=(F)0.5/w; 
	} else {
		//v=v0.orthogonal().normalized(); w=0;
		v=GetOrthogonal(v0).normalized(); w=0;
	}
}
template<class F> 
ILINE Quaternion_tpl<F> Quaternion_tpl<F>::GetRotationV0V1(const Vec3_tpl<F>& v0, const Vec3_tpl<F>& v1) {	Quaternion_tpl<F> q;	q.SetRotationV0V1(v0,v1); 	return q;	}
template<class F> 
ILINE Quaternion_tpl<F> GetRotationV0V1(const Vec3_tpl<F>& v0, const Vec3_tpl<F>& v1) {	Quaternion_tpl<F> q;	q.SetRotationV0V1(v0,v1); 	return q;	}



/*!
 *
 * Create rotation-quaternion that around the fixed coordinate axes.
 *
 * Example:
 *		Quaternion_tpl<f32> q=GetRotationXYZ( Ang3(1,2,3) );
 *		q.SetRotationXYZ( Ang3(1,2,3) );
 */
template<class F> 
ILINE void Quaternion_tpl<F>::SetRotationXYZ(const Ang3 &a)	{ 
	F csx[2]; sincos_tpl( a.x*(F)0.5, csx);
	F csy[2]; sincos_tpl( a.y*(F)0.5, csy);
	F csz[2]; sincos_tpl( a.z*(F)0.5, csz);
	w		= cx*cy*cz + sx*sy*sz;
	v.x = cz*cy*sx - sz*sy*cx;
	v.y = cz*sy*cx + sz*cy*sx;
	v.z = sz*cy*cx - cz*sy*sx;
}
template<class F> 
ILINE Quaternion_tpl<F> Quaternion_tpl<F>::GetRotationXYZ(const Ang3 &a) {	Quaternion_tpl<F> q;	q.SetRotationXYZ(a); 	return q;	}
template<class F> 
ILINE Quaternion_tpl<F> GetRotationXYZ(const Ang3 &a) {	Quaternion_tpl<F> q;	q.SetRotationXYZ(a); 	return q;	}



/*!
 * set identity quaternion
 * 
 * Example:
 *		Quaternion q;
 *		q.Identity();
 */
template <class F>
ILINE void	Quaternion_tpl<F>::SetIdentity(void) { w=1; v.x=0, v.y=0; v.z=0; }
template <class F>
ILINE Quaternion_tpl<F> Quaternion_tpl<F>::GetIdentity(void)	{ return Quaternion_tpl(1,0,0,0); }
template <class F>
ILINE Quaternion_tpl<F> GetIdentity(void)	{ return Quaternion_tpl<F>(1,0,0,0); }


	/*!
	* normalize quaternion.
	* 
	* Example 1:
	*  Quaternion q;
	*  q.Normalize();
	*
	* Example 2:
	*  Quaternion q;
	*  Quaternion qn=GetNormalized( q );
	*
	*/
	template <class F>
	ILINE void	Quaternion_tpl<F>::Normalize(void)	{
#if !defined(LINUX)
		assert((*this|*this)>0.00001f); 
#endif
		F d = GetLength(*this);
		d=(F)1/d;		w*=d;	v*=d;
	}
	template <class F> 
	ILINE Quaternion_tpl<F> Quaternion_tpl<F>::GetNormalized( const Quaternion_tpl<F> &q )	{   Quaternion_tpl<F> t=q; t.Normalize(); return t;	}
	template <class F> 
	ILINE Quaternion_tpl<F> GetNormalized( const Quaternion_tpl<F> &q )	{   Quaternion_tpl<F> t=q; t.Normalize(); return t;	}


	/*!
	* get length of quaternion.
	* 
	* Example 1:
	*  f32 l=GetLength(q);
	*
	*/
	template <class F>
	ILINE F Quaternion_tpl<F>::GetLength( const Quaternion_tpl<F>& q ) { return sqrt_tpl(q|q); }
	template <class F>
	ILINE F GetLength( const Quaternion_tpl<F>& q ) { return sqrt_tpl(q|q); }





	/*!
	*
	* linear-interpolation between quaternions (lerp)
	* 
	* Example:
	*  CQuaternion result,p,q;
	*  result=qlerp( p, q, 0.5f );
	*
	*/
	template<class F> 
		ILINE Quaternion_tpl<F> qlerp( const Quaternion_tpl<F> &p, const Quaternion_tpl<F> &q, f32 t ) {
#if !defined(LINUX)
			assert((fabs_tpl(1-(p|p)))<0.001); //check if unit-quaternion
			assert((fabs_tpl(1-(q|q)))<0.001); //check if unit-quaternion
#endif
			return p*(1.0f-t) + q*t;
		}



		/*!
		*
		* spherical-interpolation between quaternions (geometrical slerp)
		* 
		* Example:
		*  Quaternion_tpl<f32> result,p,q;
		*  result=qslerp_g( p, q, 0.5f );
		*
		*/
	template<class F> 
	ILINE Quaternion_tpl<F> Slerp( const Quaternion_tpl<F> &p, const Quaternion_tpl<F> &tq, f32 t ) {
#if !defined(LINUX)
				assert((fabs_tpl(1-(p|p)))<0.001); //check if unit-quaternion
				assert((fabs_tpl(1-(tq|tq)))<0.001); //check if unit-quaternion
#endif
				Quaternion_tpl<F> q=tq;

				// calculate cosine using the "inner product" between two quaternions: p*q=cos(radiant)
				f32 cosine = (p|q);

				// Problem 1:  given any two quaternions, there exist two possible arcs, along which one can move. 
				// One of them goes around the long way and this is the one that we wish to avoid. 
				if( cosine < 0.0 ) { cosine=-cosine;		q=-q;	} 

				//Problem 2:  we explore the special cases where the both quaternions are very close together, 
				//in which case we approximate using the more economical LERP and avoid "divisions by zero" since sin(Angle) = 0  as   Angle=0
				if(cosine>=0.999f) {	
					return qlerp(p,q,t);
				}	else {
					//perform qslerp_g: because of the "qlerp"-check above, a "division by zero" is not possible
					f32 angle     = cry_acosf(cosine);
					f32 isine  		= 1.0f/sin_tpl(angle);
					f32 scale_0   = sin_tpl((1.0f - t) * angle);
					f32 scale_1   = sin_tpl(t * angle);
					return  (p*scale_0 + q*scale_1) * isine;
				}
			}



//! Spherical linear interpolation of quaternions.
//! @param a Source quaternion.
//! @param b Target quaternion.
//! @param t Interpolation time.

//! Spherical linear interpolation of quaternions.
/*template <class F>
ILINE Quaternion_tpl<F> Slerp(const Quaternion_tpl<F>& a, const Quaternion_tpl<F>& b, f32 t )
{
	assert((fabs_tpl(1-(a|a)))<0.001); //check if unit-quaternion
	assert((fabs_tpl(1-(b|b)))<0.001); //check if unit-quaternion

	Quaternion_tpl<F> q;

	f32 f1,f2;					// interpolation coefficions.
	f32 angle;					// angle between A and B
	f32 oosin_a, cos_a;	// sine, cosine of angle

	Quaternion_tpl<F> to;

	cos_a = a|b;
	if (cos_a < 0) {
		cos_a = -cos_a; //????????
		to = -b;
	} else {
		to = b;
	}

	if (1.0f - fabs(cos_a) < 0.001f) {
		f1 = 1.0f - t;
		f2 = t;
	} else {				// normal case
		angle = (f32)acos(cos_a);
		oosin_a = 1.0f / (f32)sin(angle);
		f1 = (f32)sin( (1.0f - t)*angle ) * oosin_a;
		f2 = (f32)sin( t*angle ) * oosin_a;
	}

	q.w	= f1*a.w + f2*to.w;
	q.v = f1*a.v + f2*to.v;

	return q;
}*/



//! squad(p,a,b,q,t) = slerp( slerp(p,q,t),slerp(a,b,t), 2(1-t)t).
template <class F>
ILINE Quaternion_tpl<F>	Squad( const Quaternion_tpl<F> p,const Quaternion_tpl<F> a,const Quaternion_tpl<F> b,const Quaternion_tpl<F> q,f32 t )
{
	f32 k = 2.0f * (1.0f - t)*t;
	//return long_slerp( long_slerp(p,q,t),long_slerp(a,b,t), k );
	return Slerp( Slerp(p,q,t),Slerp(a,b,t), k );
}

//! Quaternion interpolation for angles > 2PI.
template <class F>
ILINE Quaternion_tpl<F>	SquadRev(	f32 angle,				// angle of rotation 
													const Vec3& axis,	// the axis of rotation 
													const Quaternion_tpl<F>& p,		// start quaternion 
													const Quaternion_tpl<F>& a, 	// start tangent quaternion 
													const Quaternion_tpl<F>& b, 	// end tangent quaternion 
													const Quaternion_tpl<F>& q,		// end quaternion 
													f32 t	)						// Time parameter, in range [0,1]
{
	f32 s,v;
	f32 omega = 0.5f*angle;
	f32 nrevs = 0;
	Quaternion_tpl<F> r,pp,qq;
		
	if (omega < (gf_PI - 0.00001f)) { 
		return Squad( p,a,b,q,t );
	}
		
	while (omega > (gf_PI - 0.00001f)) { 
		omega -= gf_PI;
		nrevs += 1.0f;
	}
	if (omega < 0) omega = 0;
	s = t*angle/gf_PI;		// 2t(omega+Npi)/pi
		
	if (s < 1.0f) {
		pp = p*Quaternion_tpl<f32>(0.0f,axis);//pp = p.Orthog( axis );
		r = Squad(p,a,pp,pp,s);	// in first 90 degrees.
	}	else {
		v = s + 1.0f - 2.0f*(nrevs+(omega/gf_PI));
		if (v <= 0.0f)  {
			// middle part, on great circle(p,q).
			while (s >= 2.0f) s -= 2.0f;
			pp = p*Quaternion_tpl<f32>(0.0f,axis);//pp = p.Orthog(axis);
			r = Slerp(p,pp,s);
		}	else {
			// in last 90 degrees.
			qq = -q*Quaternion_tpl<f32>(0.0f,axis);
			r = Squad(qq,qq,b,q,v);
		}
	}
	return r;
}



// Exponent of Quaternion.
template <class F>
ILINE Quaternion_tpl<F> exp(const Quaternion_tpl<F>& q) {
	double d = sqrt( double(q.v.x*q.v.x + q.v.y*q.v.y + q.v.z*q.v.z) );
	if (d > 1e-4)	{
		double m = sin(d)/d;
		return Quaternion_tpl<F>( (F)cos(d),F(q.v.x*m),F(q.v.y*m),F(q.v.z*m));
	}	else	return Quaternion_tpl<F> (F(1-d*d), F(q.v.x), F(q.v.y), F(q.v.z) );		// cos(d) ~= 1-d^2 when d->0
}

// logarithm of a quaternion, imaginary part (the real part of the logarithm is always 0)
template <class F>
ILINE Quaternion_tpl<F> log (const Quaternion_tpl<F>& q) {
#if !defined(LINUX)
	assert((fabs_tpl(1-(q|q)))<0.01); //check if unit-quaternion
#endif
	double d = sqrt( double(q.v.x*q.v.x + q.v.y*q.v.y + q.v.z*q.v.z ));
	if (d > 1e-7)	{
		d = atan2 ( d, (double)q.w ) / d;
		return Quaternion_tpl<F>(0, F(q.v.x*d),F(q.v.y*d), F(q.v.z*d) );
	}	else	return Quaternion_tpl<F>(0,0,0,0);
}





//////////////////////////////////////////////////////////////////////////
//! Logarithm of Quaternion difference.
template <class F>
ILINE Quaternion_tpl<F> LnDif( const Quaternion_tpl<F> &q1,const Quaternion_tpl<F> &q2 ){
#if !defined(LINUX)
	assert((fabs_tpl(1-(q1|q1)))<0.001); //check if unit-quaternion
	assert((fabs_tpl(1-(q2|q2)))<0.001); //check if unit-quaternion
#endif
	return log(q2/q1);
	//Quaternion_tpl<F> r = q2/q1;
	//return log(r);
}

/*
//! Compute a, the term used in Boehm-type interpolation.
//! a[n] =    q[n]* qexp(-(1/4)*( ln(qinv(q[n])*q[n+1]) +ln( qinv(q[n])*q[n-1] )))
template <class F>
ILINE Quaternion_tpl<F> CompQuatA( const Quaternion_tpl<F>& qprev,const Quaternion_tpl<F>& q,const Quaternion_tpl<F>& qnext ) {
	Quaternion_tpl<F> qm,qp,r;
	qm = LnDif( q,qprev );
	qp = LnDif( q,qnext );
	r = (-0.25f)*(Quaternion_tpl<F>(qm.v.x+qp.v.x,qm.v.y+qp.v.y,qm.v.z+qp.v.z,qm.w+qp.w));
	return q*r.Exp();
}*/







/*!
*
* spherical-interpolation between quaternions (algebraic slerp_a)
* I have included this function just for the sake of completeness, because 
* its the only useful application to check if exp & log really work. 
* Both slerp-functions are returning the same result.
*	
* Example:
*  Quaternion result,p,q;
*  result=qslerp_a( p, q, 0.5f );
*
*/
template<class F> 
ILINE Quaternion_tpl<F> qslerp_a( const Quaternion_tpl<F> &p, const Quaternion_tpl<F> &tq, F t ) {
#if !defined(LINUX)
	assert((fabs_tpl(1-(p|p)))<0.001); //check if unit-quaternion
	assert((fabs_tpl(1-(tq|tq)))<0.001); //check if unit-quaternion
#endif
	Quaternion_tpl<F> q=tq;

	// calculate cosine using the "inner product" between two quaternions: p*q=cos(radiant)
	f32 cosine = (p|q);

	// Problem: given any two quaternions, there exist two possible arcs, along which one can move. 
	// One of them goes around the long way and this is the one that we wish to avoid. 
	if( cosine < 0.0 ) { cosine=-cosine; q=-q;	} 

	//perform qslerp_a: 
	return exp( (log (p* !q)) * (1-t)) * q;		//algebraic slerp (1)

	//...and some more slerp-functions producing all the same result
	//return p * exp( (log (invert(p)*q)) * t);			//algebraic slerp (2)
	//return exp( (log (q*invert(p))) * t) * p;			//algebraic slerp (3)
	//return q * exp( (log (invert(q)*p)) * (1-t));	//algebraic slerp (4)
}


/*!
*
* quaternion copy constructor; Quaternion q=mat33
* We take only the rotation of the 3x3 part 
* 
* Example 1:
* \code
*  Matrix33 mat33;
*  mat33.rotationXYZ33(0.5f, 2.5f, 1.5f);
*  Quaternion q=mat33;
* \endcode
* 
* Example 2:
* \code
*  CMatrix34 mat34;
*  mat34.rotationXYZ34(0.5f, 2.5f, 1.5f);
*  Quaternion q=Matrix33(mat34);
* \endcode
*/
template<class F,int SI,int SJ> 
inline Quaternion_tpl<F> GetQuatFromMat33(const Matrix33_tpl<F,SI,SJ>& m)	{ 
	Quaternion_tpl<f32> q;
	//check if we have an orthonormal-base (assuming we are using a right-handed coordinate system)
	//assert removed by ivo: it was impossible to load a level bacause of this asser!
	//but be warned, if you convert a non-uniform-scaled matrix into a quaternion 
	//you get a worthless bullshit-quaternion!
	//vlad: still impossible to load
	//assert( IsEquivalent(m.GetOrtX(),m.GetOrtY()%m.GetOrtZ(),0.1f) );
	//assert( IsEquivalent(m.GetOrtY(),m.GetOrtZ()%m.GetOrtX(),0.1f) );
	//assert( IsEquivalent(m.GetOrtZ(),m.GetOrtX()%m.GetOrtY(),0.1f) );

	F tr,s,p;
	tr = m.M00 + m.M11 + m.M22;

	//check the diagonal
	if(tr > 0.0) {
		s=sqrt_tpl(tr+1.0f); p=0.5f/s;
		q.w=s*0.5f; q.v((m.M21-m.M12)*p,(m.M02-m.M20)*p,(m.M10-m.M01)*p); return q;
	}
	//diagonal is negativ. now we have to find the biggest element on the diagonal
	//check if "m00" is the biggest element
	if	( (m.M00>=m.M11) && (m.M00>=m.M22) ) {
		s=p=sqrt_tpl(m.M00-m.M11-m.M22+1.0f);	if (s) { p=0.5f/s; }
		q.w=(m.M21-m.M12)*p; q.v(s*0.5f,(m.M10+m.M01)*p,(m.M20+m.M02)*p); return q;
	}
	//check if "m11" is the biggest element
	if	( (m.M11>=m.M00) && (m.M11>=m.M22) ) {
		s=p=sqrt_tpl(m.M11-m.M22-m.M00+1.0f); if (s) { p=0.5f/s; }
		q.w=(m.M02-m.M20)*p; q.v((m.M01+m.M10)*p,s*0.5f,(m.M21+m.M12)*p); return q;
	}
	//check if "m22" is the biggest element
	if	( (m.M22>=m.M00) && (m.M22>=m.M11) ) {
		s=p=sqrt_tpl(m.M22-m.M00-m.M11+1.0f);		if (s) { p=0.5f/s; }
		q.w=(m.M10-m.M01)*p; q.v((m.M02+m.M20)*p,(m.M12+m.M21)*p,s*0.5f); return q;
	}
#if !defined(LINUX)
	assert(0); 
#endif
	return q;//if it ends here, then we have no valid rotation matrix
}


#endif // _Quaternion_tpl_H

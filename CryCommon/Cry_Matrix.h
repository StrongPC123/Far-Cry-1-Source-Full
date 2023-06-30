//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//	
//	File:Cry_Matrix.h
//	Description: Common matrix class
//
//	History:
//	-Feb 27,2003: Created by Ivo Herzeg
//                
//
//////////////////////////////////////////////////////////////////////


#ifndef MATRIX_H
#define MATRIX_H 

#if _MSC_VER > 1000
# pragma once
#endif

#include "Cry_Quat.h"
#include "Cry_Vector2.h"
#include "Cry_Vector3.h"


#if defined(LINUX)
#undef assert
#define assert(exp) (void)( (exp) || (printf("Assert: ' %s ' has failed\n", #exp), 0) )
#endif


#define l_00 l.data[SI1*0+SJ1*0]
#define l_10 l.data[SI1*1+SJ1*0]
#define l_20 l.data[SI1*2+SJ1*0]
#define l_30 l.data[SI1*3+SJ1*0]
#define l_01 l.data[SI1*0+SJ1*1]
#define l_11 l.data[SI1*1+SJ1*1]
#define l_21 l.data[SI1*2+SJ1*1]
#define l_31 l.data[SI1*3+SJ1*1]
#define l_02 l.data[SI1*0+SJ1*2]
#define l_12 l.data[SI1*1+SJ1*2]
#define l_22 l.data[SI1*2+SJ1*2]
#define l_32 l.data[SI1*3+SJ1*2]
#define l_03 l.data[SI1*0+SJ1*3]
#define l_13 l.data[SI1*1+SJ1*3]
#define l_23 l.data[SI1*2+SJ1*3]
#define l_33 l.data[SI1*3+SJ1*3]





#define r_00 r.data[SI2*0+SJ2*0]
#define r_10 r.data[SI2*1+SJ2*0]
#define r_20 r.data[SI2*2+SJ2*0]
#define r_30 r.data[SI2*3+SJ2*0]
#define r_01 r.data[SI2*0+SJ2*1]
#define r_11 r.data[SI2*1+SJ2*1]
#define r_21 r.data[SI2*2+SJ2*1]
#define r_31 r.data[SI2*3+SJ2*1]
#define r_02 r.data[SI2*0+SJ2*2]
#define r_12 r.data[SI2*1+SJ2*2]
#define r_22 r.data[SI2*2+SJ2*2]
#define r_32 r.data[SI2*3+SJ2*2]
#define r_03 r.data[SI2*0+SJ2*3]
#define r_13 r.data[SI2*1+SJ2*3]
#define r_23 r.data[SI2*2+SJ2*3]
#define r_33 r.data[SI2*3+SJ2*3]






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Matrix33diag_tpl
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename F> struct Matrix33diag_tpl {

	F x,y,z;

	Matrix33diag_tpl() {}

	Matrix33diag_tpl(F dx,F dy,F dz) { x=dx; y=dy; z=dz; }

	Matrix33diag_tpl(const Vec3_tpl<F> &src) { x=src.x; y=src.y; z=src.z; } 

	Matrix33diag_tpl& operator=(const Matrix33diag_tpl<F> &src) { 
		x=src.x; y=src.y; z=src.z; return *this; 
	}

	template<class F1> Matrix33diag_tpl& operator=(const Matrix33diag_tpl<F1> &src) { 
		x=src.x; y=src.y; z=src.z; return *this; 
	}

	template<class F1> const Matrix33diag_tpl& operator=(const Vec3_tpl<F1> &src) {
		x=src.x; y=src.y; z = src.z; return *this; 
	}

	const Matrix33diag_tpl& identity() {	x=y=z=1; return *this;	}

	const Matrix33diag_tpl& zero() {	x=y=z=0; return *this; }

	Matrix33diag_tpl& fabs() {	x=fabs_tpl(x); y=fabs_tpl(y); z=fabs_tpl(z); return *this;	}

	Matrix33diag_tpl& invert() { // in-place inversion
		F det = determinant();
		if (det==0) return *this;	
		det = (F)1.0/det;
		F oldata[3]; oldata[0]=x; oldata[1]=y; oldata[2]=z;
		x = oldata[1]*oldata[2]*det; 
		y = oldata[0]*oldata[2]*det; 
		z = oldata[0]*oldata[1]*det;
		return *this;
	}

	F determinant() const {	return x*y*z; }
};

///////////////////////////////////////////////////////////////////////////////
// Typedefs                                                                  //
///////////////////////////////////////////////////////////////////////////////

typedef Matrix33diag_tpl<f32> Matrix33diag;
typedef Matrix33diag_tpl<real> Matrix33diag_f64;


template<class F1, class F2> 
Matrix33diag_tpl<F1> operator*(const Matrix33diag_tpl<F1> &l, const Matrix33diag_tpl<F2> &r) {
	return Matrix33diag_tpl<F1>(	l.x*r.x, l.y*r.y,	l.z*r.z	);
}

template<class F1, class F2,int SI,int SJ> 
Matrix33_tpl<F2,SI,SJ> operator*(const Matrix33diag_tpl<F1> &l, const Matrix33_tpl<F2,SI,SJ> &r) {
	Matrix33_tpl<F2,SI,SJ> res;
	res.M00 = r.M00*l.x;	res.M01 = r.M01*l.x;		res.M02 = r.M02*l.x;
	res.M10 = r.M10*l.y;	res.M11 = r.M11*l.y;		res.M12 = r.M12*l.y;
	res.M20 = r.M20*l.z;	res.M21 = r.M21*l.z;		res.M22 = r.M22*l.z;
	return res;
}

template<class F1,int SI,int SJ, class F2> 
Matrix33_tpl<F1,SI,SJ> operator*(const Matrix33_tpl<F1,SI,SJ> &l, const Matrix33diag_tpl<F2> &r) {
	Matrix33_tpl<F1,SI,SJ> res;
	res.M00 = l.M00*r.x;		res.M01 = l.M01*r.y;		res.M02 = l.M02*r.z;
	res.M10 = l.M10*r.x;		res.M11 = l.M11*r.y;		res.M12 = l.M12*r.z;
	res.M20 = l.M20*r.x;		res.M21 = l.M21*r.y;		res.M22 = l.M22*r.z;
	return res;
}

template<class F1, class F2> 
Matrix34_tpl<F1> operator*(const Matrix34_tpl<F1> &l, const Matrix33diag_tpl<F2> &r) {
	Matrix34_tpl<F1> m;
	m.m00=l.m00*r.x;	m.m01=l.m01*r.y;	m.m02=l.m02*r.z;	m.m03=l.m03;
	m.m10=l.m10*r.x;	m.m11=l.m11*r.y;	m.m12=l.m12*r.z;	m.m13=l.m13;
	m.m20=l.m20*r.x;	m.m21=l.m21*r.y;	m.m22=l.m22*r.z;	m.m23=l.m23;
	return m;
}

template<class F1, class F2> 
Matrix34_tpl<F2> operator*(const Matrix33diag_tpl<F1> &l, const Matrix34_tpl<F2> &r) {
	Matrix34_tpl<F2> m;
	m.m00=l.x*r.m00;	m.m01=l.x*r.m01;	m.m02=l.x*r.m02;	m.m03=l.x*r.m03;
	m.m10=l.y*r.m10;	m.m11=l.y*r.m11;	m.m12=l.y*r.m12;	m.m13=l.y*r.m13;
	m.m20=l.z*r.m20;	m.m21=l.z*r.m21;	m.m22=l.z*r.m22;	m.m23=l.z*r.m23;
	return m;
}

template<class F1,int SI,int SJ, class F2> 
const Matrix33_tpl<F1,SI,SJ>& operator *= (const Matrix33_tpl<F1,SI,SJ> &l, const Matrix33diag_tpl<F2> &r) {
	l.M00*=r.x;	l.M01*=r.y;	l.M02*=r.z; 
	l.M10*=r.x;	l.M11*=r.y;	l.M12*=r.z;
	l.M20*=r.x;	l.M21*=r.y;	l.M22*=r.z;
	return l;
}

template<class F1,class F2>
Vec3_tpl<F2> operator *(const Matrix33diag_tpl<F1> &mtx, const Vec3_tpl<F2> &vec) {
	return Vec3_tpl<F2>(mtx.x*vec.x, mtx.y*vec.y, mtx.z*vec.z);
}

template<class F1,class F2>
Vec3_tpl<F1> operator *(const Vec3_tpl<F1> &vec, const Matrix33diag_tpl<F2> &mtx) {
	return Vec3_tpl<F1>(mtx.x*vec.x, mtx.y*vec.y, mtx.z*vec.z);
}








///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Matrix33_tpl
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class F,int SI,int SJ> struct Matrix33_tpl {

	F data[(SI-(SI-SJ & (SI-SJ)>>31))*3];

	//---------------------------------------------------------------------------------

	Matrix33_tpl() {}

	ILINE Matrix33_tpl(const Matrix33_tpl& m) { 
		assert ( (void*)this!=(void*)&m );
		M00=m.M00;	M01=m.M01;	M02=m.M02; 
		M10=m.M10;	M11=m.M11;	M12=m.M12;
		M20=m.M20;	M21=m.M21;	M22=m.M22; 
	}

	template<class F1,int SI1,int SJ1> 
	ILINE Matrix33_tpl(const Matrix33_tpl<F1,SI1,SJ1>& m) {
		assert (  (SI==SI1) | ((void*)this!=(void*)&m) );
		M00=m_00;	M01=m_01;	M02=m_02; 
		M10=m_10;	M11=m_11;	M12=m_12;
		M20=m_20;	M21=m_21;	M22=m_22; 
	}

	explicit ILINE Matrix33_tpl(const Matrix34_tpl<F>& m )	{
		M00=m.m00;		M01=m.m01;		M02=m.m02;	
		M10=m.m10;		M11=m.m11;		M12=m.m12;
		M20=m.m20;		M21=m.m21;		M22=m.m22;
	}

	template<class F1,int SI1,int SJ1> 
	explicit ILINE Matrix33_tpl(const Matrix44_tpl<F1,SI1,SJ1>& m ) {
		M00=m_00;	M01=m_01;	M02=m_02; 
		M10=m_10;	M11=m_11;	M12=m_12;
		M20=m_20;	M21=m_21;	M22=m_22; 
	}

	// non-templated version is needed since default operator= has precedence over templated operator=
	ILINE Matrix33_tpl& operator=(const Matrix33_tpl<F,SI,SJ> &m) { 
		M00=m.M00;	M01=m.M01;	M02=m.M02; 
		M10=m.M10;	M11=m.M11;	M12=m.M12;
		M20=m.M20;	M21=m.M21;	M22=m.M22; 
		return *this; 
	}
	template<class F1,int SI1,int SJ1> 
	ILINE Matrix33_tpl& operator = (const Matrix33_tpl<F1,SI1,SJ1>& m) { 
		assert (  (SI==SI1) | ((void*)this!=(void*)&m) );
		M00=m_00;	M01=m_01;		M02=m_02; 
		M10=m_10;	M11=m_11;		M12=m_12;
		M20=m_20;	M21=m_21;		M22=m_22; 
		return *this; 
	}




	template<class F1> 
	Matrix33_tpl& operator = (const Vec3_tpl<F1> &v) {
		M00=v.x;		M01=0;			M02=0; 
		M10=0;			M11=v.y;		M12=0;
		M20=0;			M21=0;			M22=v.z; 
		return *this;
	}
	template<class F1> 
	Matrix33_tpl& operator=(const Matrix33diag_tpl<F1> &src) {
		M00=src.x; M01=0;			M02=0; 
		M10=0;		 M11=src.y;	M12=0;
		M20=0;		 M21=0;			M22=src.z; 
		return *this;
	}

	//Convert unit quaternion to matrix.
	explicit ILINE Matrix33_tpl( const Quaternion_tpl<F>& q ){
		assert((fabs_tpl(1-(q|q)))<0.1); //check if unit-quaternion
		F vxvx=q.v.x*q.v.x;		F	vzvz=q.v.z*q.v.z;		F	vyvy=q.v.y*q.v.y; 
		F	vxvy=q.v.x*q.v.y;		F	vxvz=q.v.x*q.v.z;		F	vyvz=q.v.y*q.v.z; 
		F	svx=q.w*q.v.x;			F	svy=q.w*q.v.y;			F	svz=q.w*q.v.z;
		M00=1-(vyvy+vzvz)*2;	M01=(vxvy-svz)*2;			M02=(vxvz+svy)*2;
		M10=(vxvy+svz)*2;			M11=1-(vxvx+vzvz)*2;	M12=(vyvz-svx)*2;
		M20=(vxvz-svy)*2;			M21=(vyvz+svx)*2;			M22=1-(vxvx+vyvy)*2;
	}

//---------------------------------------------------------------------------------------
	//obsolete
	ILINE void SetIdentity33(void) {
		M00=1;	M01=0;	M02=0; 
		M10=0;	M11=1;	M12=0;
		M20=0;	M21=0;	M22=1; 
	}

	ILINE void SetIdentity(void) {
		M00=1;	M01=0;	M02=0; 
		M10=0;	M11=1;	M12=0;
		M20=0;	M21=0;	M22=1; 
	}
	ILINE void SetZero() { 
		M00=0;	M01=0;	M02=0; 
		M10=0;	M11=0;	M12=0;
		M20=0;	M21=0;	M22=0; 
	}


	ILINE void SetRotationAA(F angle, Vec3_tpl<F> axis) {	F cs[2]; sincos_tpl( angle, cs);	SetRotationAA(cs[0],cs[1], axis);	}
	ILINE void SetRotationAA(F c, F s, Vec3_tpl<F> axis) { 
		assert((fabs_tpl(1-(axis|axis)))<0.001); //check if unit-vector
		F	mc	=	(F)1.0-c;
		M00=mc*axis.x*axis.x + c;					M01=mc*axis.x*axis.y - axis.z*s;	M02=mc*axis.x*axis.z + axis.y*s;	
		M10=mc*axis.y*axis.x + axis.z*s;	M11=mc*axis.y*axis.y + c;					M12=mc*axis.y*axis.z - axis.x*s;	
		M20=mc*axis.z*axis.x - axis.y*s;	M21=mc*axis.z*axis.y + axis.x*s;	M22=mc*axis.z*axis.z + c;					
	}
	ILINE static Matrix33_tpl<F,SI,SJ> CreateRotationAA( const f32 rad, const Vec3_tpl<F>& axis ) {	
		Matrix33_tpl<F,SI,SJ> m33;	m33.SetRotationAA(rad,axis); return m33;	
	}
		


	/*!
	*
	* Create rotation-matrix about X axis using an angle.
	* The angle is assumed to be in radians. 
	*
	*  Example:
	*		Matrix m33;
	*		m33.SetRotationX(0.5f);
	*/
	ILINE void SetRotationX(const f32 rad )	{
		F cs[2]; sincos_tpl(rad,cs);
		M00=1.0f;		M01=0.0f;		M02=	0.0f;		
		M10=0.0f;		M11=cs[0];	M12=-cs[1];
		M20=0.0f;		M21=cs[1];	M22= cs[0];
	}
	ILINE static Matrix33_tpl<F,SI,SJ> CreateRotationX(const f32 rad ) {	Matrix33_tpl<F,SI,SJ> m33; m33.SetRotationX(rad); return m33;	}


	ILINE void SetRotationY(const f32 rad ) {
		F cs[2]; sincos_tpl(rad,cs);
		M00	=	cs[0];	M01	=	0.0f;		M02	=	cs[1];
		M10	=	0.0f;		M11	=	1.0f;		M12	=	0.0f;			
		M20	=-cs[1];	M21	=	0.0f;		M22	= cs[0];	
	}
	ILINE static Matrix33_tpl<F,SI,SJ> CreateRotationY(const f32 rad ) { Matrix33_tpl<F,SI,SJ> m33; m33.SetRotationY(rad);	return m33;	}


	ILINE void SetRotationZ( const f32 rad ) {
		F cs[2]; sincos_tpl(rad,cs);
		M00	=	cs[0];	M01	=-cs[1];	M02	=	0.0f;	
		M10	=	cs[1];	M11	=	cs[0];	M12	=	0.0f;	
		M20	=	0.0f;		M21	=	0.0f;		M22	= 1.0f;
	}
	ILINE static Matrix33_tpl<F,SI,SJ> CreateRotationZ(const f32 rad) {	Matrix33_tpl<F,SI,SJ> m33;	m33.SetRotationZ(rad);	return m33;	}



	ILINE void SetRotationXYZ( const Ang3_tpl<F>& rad ) {
		F csx[2]; sincos_tpl(rad.x, csx);
		F csy[2]; sincos_tpl(rad.y, csy);
		F csz[2]; sincos_tpl(rad.z, csz);
		F sycz  =(sy*cz), sysz  =(sy*sz);
		M00=cy*cz;	M01=sycz*sx-cx*sz;	M02=sycz*cx+sx*sz;
		M10=cy*sz;	M11=sysz*sx+cx*cz;	M12=sysz*cx-sx*cz;
		M20=-sy;	M21=cy*sx;			M22=cy*cx;				
	}
	ILINE static Matrix33_tpl<F,SI,SJ> CreateRotationXYZ( const Ang3_tpl<F>& rad ) {	Matrix33_tpl<F,SI,SJ> m33;	m33.SetRotationXYZ(rad); return m33;	}


	
 /*!
  * Creates a rotation matrix that rotates the vector "v0" into "v1". 
  *	USE WITH CAUTION: this method suffers from some instability problems! 
  *	a) If both vectors are almost parallel or diametrical we have to normalize 
	*    a very small vector and the result is inaccurate.
  *	b) If both vectors are exactly parallel it returns an identity-matrix 
  *	c) If both vectors are exactly diametrical it returns a matrix that rotates 
	     pi-radians about a "random" axis.  
	* It is recommended to use this function with 64-bit precision. 
	*
	*/
	ILINE void SetRotationV0V1( const Vec3_tpl<F>& v0, const Vec3_tpl<F>& v1 ) {
		assert((fabs_tpl(1-(v0|v0)))<0.001); //check if unit-vector
		assert((fabs_tpl(1-(v1|v1)))<0.001); //check if unit-vector
		F e = v0|v1;
		M00=e;	M01=0;	M02=0;
		M10=0;	M11=1;	M12=0;
    M20=0;	M21=0;	M22=e;
		if ( fabs_tpl(e) < (F)0.99999)	{
			Vec3_tpl<F> v = v0%v1;	F h = 1/(1 + e); 
			M00= e+h *v.x*v.x;		M01=h*v.x*v.y-v.z;		M02=h*v.x*v.z+v.y;
			M10=h*v.x*v.y+v.z;		M11= e+h *v.y*v.y;		M12=h*v.y*v.z-v.x;
			M20=h*v.x*v.z-v.y;		M21=h*v.y*v.z+v.x;		M22= e+h *v.z*v.z;
		}
	}
	ILINE static Matrix33_tpl<F,SI,SJ> CreateRotationV0V1( const Vec3_tpl<F>& v0, const Vec3_tpl<F>& v1 ) {	Matrix33_tpl<F,SI,SJ> m33;	m33.SetRotationV0V1(v0,v1); return m33;	}

 /*!
  *  Creates a rotation matrix that rotates a vector called "n" into the vector (0,0,1). 
	*  This is an optimized version of SetRotationV0V1();
	*/
	ILINE void SetRotationV0( const Vec3_tpl<F>& n ) {
		assert((fabs_tpl(1-(n|n)))<0.001); //check if unit-vector
		F div=(n.x*n.x + n.y*n.y);
		M00=n.z;	M01= 0;	M02= 0;
		M10= 0;		M11=+1;	M12= 0;
		M20= 0;		M21= 0;	M22=n.z;
		if (div>0) {
			F		h=(1-n.z) / div;
			M00= h*n.y*n.y+n.z;	M01=-h*n.y*n.x;			M02=-n.x;
			M10=-h*n.y*n.x;			M11=+h*n.x*n.x+n.z;	M12=-n.y;
			M20= n.x;						M21= n.y;						M22= n.z;
		}
	}
	ILINE static Matrix33_tpl<F,SI,SJ> CreateRotationV0( const Vec3_tpl<F>& n ) {	Matrix33_tpl<F,SI,SJ> m33;	m33.SetRotationV0(n); return m33;	}



	ILINE void SetScale( const Vec3_tpl<F> &s ) {
		M00=s.x;		M01=0;			M02=0;
		M10=0;			M11=s.y;		M12=0;
		M20=0;			M21=0;			M22=s.z;
	}
	ILINE static Matrix33_tpl<F,SI,SJ> CreateScale( const Vec3_tpl<F> &s  ) { Matrix33_tpl<F,SI,SJ> m; m.SetScale(s);	return m;	}


	//NOTE: all vectors are stored in columns
	ILINE void SetMatFromVectors(const Vec3_tpl<F>& vx, const Vec3_tpl<F>& vy, const Vec3_tpl<F>& vz)	{
		M00=vx.x;		M01=vy.x;		M02=vz.x;
		M10=vx.y;		M11=vy.y;		M12=vz.y;
		M20=vx.z;		M21=vy.z;		M22=vz.z;
	}
	ILINE static Matrix33_tpl<F,SI,SJ> CreateMatFromVectors( const Vec3_tpl<F>& vx, const Vec3_tpl<F>& vy, const Vec3_tpl<F>& vz ) {	Matrix33_tpl<F,SI,SJ> dst; dst.SetMatFromVectors(vx,vy,vz); return dst;	}




	ILINE void Transpose() { // in-place transposition
		F t; 
		t=M01; M01=M10; M10=t;
		t=M02; M02=M20; M20=t;
		t=M12; M12=M21; M21=t;
	}
	ILINE Matrix33_tpl<F,SI,SJ> GetTransposed() const {
		Matrix33_tpl<F,SI,SJ> dst;
		dst.M00=M00;			dst.M01=M10;			dst.M02=M20;
		dst.M10=M01;			dst.M11=M11;			dst.M12=M21;
		dst.M20=M02;			dst.M21=M12;			dst.M22=M22;
		return dst;
	}

	ILINE Matrix33_tpl& Fabs() {
		M00=fabs_tpl(M00); M01=fabs_tpl(M01);	M02=fabs_tpl(M02); 
		M10=fabs_tpl(M10); M11=fabs_tpl(M11); M12=fabs_tpl(M12);
		M20=fabs_tpl(M20); M21=fabs_tpl(M21);	M22=fabs_tpl(M22); 
		return *this;
	}
	ILINE Matrix33_tpl<F,SI,SJ> GetFabs() const {	Matrix33_tpl<F,SI,SJ> m=*this; m.Fabs();	return m;	}

	ILINE void Adjoint( void ) {  
		//rescue members
		Matrix33_tpl<F,SI,SJ> m=*this;
		//calculate the adjoint-matrix
		M00=m.M11*m.M22-m.M12*m.M21;	M01=m.M12*m.M20-m.M10*m.M22;	M02=m.M10*m.M21-m.M11*m.M20;
		M10=m.M21*m.M02-m.M22*m.M01;	M11=m.M22*m.M00-m.M20*m.M02;	M12=m.M20*m.M01-m.M21*m.M00;
		M20=m.M01*m.M12-m.M02*m.M11;	M21=m.M02*m.M10-m.M00*m.M12;	M22=m.M00*m.M11-m.M01*m.M10;
	}
	ILINE Matrix33_tpl<F,SI,SJ> GetAdjoint() const {	Matrix33_tpl<F,SI,SJ> dst=*this; dst.Adjoint(); return dst;	}


	
	/*!
	*
	* calculate a real inversion of a Matrix33.
	* an inverse-matrix is an UnDo-matrix for all kind of transformations 
	* NOTE: if the return value of Invert33() is zero, then the inversion failed! 
	* 
	*  Example 1:
	*		Matrix33 im33;
	*		bool st=i33.Invert();
	*   assert(st);  
  *
	*  Example 2:
	*   matrix33 im33=Matrix33::GetInverted(m33);
	*/
	ILINE bool Invert( void ) {  
		//rescue members
		Matrix33_tpl<F,SI,SJ>	m=*this;
		//calculate the cofactor-matrix (=transposed adjoint-matrix)
		M00=m.M22*m.M11-m.M12*m.M21;	M01=m.M02*m.M21-m.M22*m.M01;	M02=m.M12*m.M01-m.M02*m.M11;
		M10=m.M12*m.M20-m.M22*m.M10;	M11=m.M22*m.M00-m.M02*m.M20;	M12=m.M02*m.M10-m.M12*m.M00;
		M20=m.M10*m.M21-m.M20*m.M11;	M21=m.M20*m.M01-m.M00*m.M21;	M22=m.M00*m.M11-m.M10*m.M01;
		// calculate determinant
		F det=(m.M00*M00 + m.M10*M01 + m.M20*M02);
		if (fabs_tpl(det)<1E-20f) 
			return 0;	
		//devide the cofactor-matrix by the determinat
		F idet=(F)1.0/det;
		M00*=idet; M01*=idet;	M02*=idet;
		M10*=idet; M11*=idet;	M12*=idet;
		M20*=idet; M21*=idet;	M22*=idet;
		return 1;
	}
	ILINE Matrix33_tpl<F,SI,SJ> GetInverted() const {	Matrix33_tpl<F,SI,SJ> dst=*this; dst.Invert(); return dst;	}



//--------------------------------------------------------------------------------
//----                  helper functions to access matrix-members     ------------
//--------------------------------------------------------------------------------

	ILINE F *GetData() { return data; }
	ILINE const F *GetData() const { return data; }

	ILINE const Matrix33_tpl<F,SJ,SI>& T() const { return (const Matrix33_tpl<F,SJ,SI>&)*(&data[0]); }
	ILINE Matrix33_tpl<F,SJ,SI>& T() { return (Matrix33_tpl<F,SJ,SI>&)*(&data[0]); }

	ILINE F operator () (unsigned i, unsigned j) const { assert ((i<3) && (j<3));	return data[i*SI+j*SJ];	}
	ILINE F& operator () (unsigned i, unsigned j)	{	assert ((i<3) && (j<3));	return data[i*SI+j*SJ];	}

	ILINE Vec3_tpl<F> GetRow(int iRow) const { return Vec3_tpl<F>(	data[iRow*SI+0*SJ],data[iRow*SI+1*SJ],data[iRow*SI+2*SJ]); 	}
	ILINE void SetRow(int iRow, const Vec3_tpl<F> &row)	{	data[iRow*SI+0*SJ]=row.x; data[iRow*SI+1*SJ]=row.y;		data[iRow*SI+2*SJ]=row.z;	}

	ILINE Vec3_tpl<F> GetColumn(int iCol) const	{	return Vec3_tpl<F>(	data[0*SI+iCol*SJ],	data[1*SI+iCol*SJ], data[2*SI+iCol*SJ] );	}
	ILINE void SetColumn(int iCol, const Vec3_tpl<F> &col)	{	data[0*SI+iCol*SJ]=col.x;	data[1*SI+iCol*SJ]=col.y;	data[2*SI+iCol*SJ]=col.z;	}

	ILINE Vec3_tpl<F> GetOrtX() const	{	return Vec3_tpl<F>( M00, M10, M20 );	}
	ILINE Vec3_tpl<F> GetOrtY() const	{	return Vec3_tpl<F>( M01, M11, M21 );	}
	ILINE Vec3_tpl<F> GetOrtZ() const	{	return Vec3_tpl<F>( M02, M12, M22 );	}


	ILINE void operator *= (F op) {
		M00*=op; M01*=op;	M02*=op;
		M10*=op; M11*=op;	M12*=op;
		M20*=op; M21*=op;	M22*=op;
	}

	ILINE void operator /= (F op) {
		F iop=(F)1.0/op;
		M00*=iop; M01*=iop;	M02*=iop;
		M10*=iop; M11*=iop;	M12*=iop;
		M20*=iop; M21*=iop;	M22*=iop;
	}

	ILINE int IsIdentity() const	{
		return 0 == (fabs_tpl((F)1-M00) + fabs_tpl(M01) + fabs_tpl(M02)+ fabs_tpl(M10) + fabs_tpl((F)1-M11) + fabs_tpl(M12)+ fabs_tpl(M20) + fabs_tpl(M21) + fabs_tpl((F)1-M22));
	}
	ILINE int IsZero() const {
		return 0 == (fabs_tpl(M00)+fabs_tpl(M01)+fabs_tpl(M02)+fabs_tpl(M10)+fabs_tpl(M11)+fabs_tpl(M12)+fabs_tpl(M20)+fabs_tpl(M21)+fabs_tpl(M22));
	}

	template<class F1> 
	ILINE void extract_from4x4T(const Matrix44_tpl<F1,4,1>& m, Vec3_tpl<F>& offset, F& scale) {
		*this=Matrix33(m).T();
		offset	= m.GetTranslationOLD();
		scale		= m.GetOrtX().len();
		*this /= scale;
	}

};

///////////////////////////////////////////////////////////////////////////////
// Typedefs                                                                  //
///////////////////////////////////////////////////////////////////////////////

typedef Matrix33_tpl<f32,3,1> Matrix33;
typedef Matrix33_tpl<real,3,1> Matrix33_f64;
typedef Matrix33_tpl<f32,1,3> Matrix33T;
typedef Matrix33_tpl<real,1,3> Matrix33T_f64;



// matrix3x3 operations with another matrix3x3
template<class F1,int SI1,int SJ1, class F2,int SI2,int SJ2> 
ILINE Matrix33_tpl<F1,SI1,SJ1> operator*(const Matrix33_tpl<F1,SI1,SJ1> &l, const Matrix33_tpl<F2,SI2,SJ2> &r) {
	Matrix33_tpl<F1,SI1,SJ1> m;
	m_00 = l_00*r_00 +	l_01*r_10 +	l_02*r_20;
	m_01 = l_00*r_01 +	l_01*r_11 +	l_02*r_21;
	m_02 = l_00*r_02 +	l_01*r_12 +	l_02*r_22;
	m_10 = l_10*r_00 +	l_11*r_10 +	l_12*r_20;
	m_11 = l_10*r_01 +	l_11*r_11 +	l_12*r_21;
	m_12 = l_10*r_02 +	l_11*r_12 +	l_12*r_22;
	m_20 = l_20*r_00 +	l_21*r_10 + l_22*r_20;
	m_21 = l_20*r_01 +	l_21*r_11 +	l_22*r_21;
	m_22 = l_20*r_02 +	l_21*r_12 +	l_22*r_22;
	return m;
}


/*!
*
*  Implements the multiplication operator: Matrix34=Matrix33*Matrix34
*
*  Matrix33 and Matrix34 are specified in collumn order for a right-handed coordinate-system.        
*  AxB = operation B followed by operation A.  
*  A multiplication takes 36 muls and 24 adds. 
*
*  Example:
*   Matrix33 m33=Matrix33::CreateRotationX(1.94192f);;
*   Matrix34 m34=Matrix34::CreateRotationZ(3.14192f);
*	  Matrix34 result=m33*m34;
*
*/
template<class F,int SI,int SJ> 
ILINE Matrix34_tpl<F> operator * (const Matrix33_tpl<F,SI,SJ>& l, const Matrix34_tpl<F>& r) {
	Matrix34_tpl<F> m;
	m.m00 = l.M00*r.m00 + l.M01*r.m10 + l.M02*r.m20;
	m.m10 = l.M10*r.m00 + l.M11*r.m10 + l.M12*r.m20;
	m.m20 = l.M20*r.m00 + l.M21*r.m10 + l.M22*r.m20;
	m.m01 = l.M00*r.m01 + l.M01*r.m11 + l.M02*r.m21;
	m.m11 = l.M10*r.m01 + l.M11*r.m11 + l.M12*r.m21;
	m.m21 = l.M20*r.m01 + l.M21*r.m11 + l.M22*r.m21;
	m.m02 = l.M00*r.m02 + l.M01*r.m12 + l.M02*r.m22;
	m.m12 = l.M10*r.m02 + l.M11*r.m12 + l.M12*r.m22;
	m.m22 = l.M20*r.m02 + l.M21*r.m12 + l.M22*r.m22;
	m.m03 = l.M00*r.m03 + l.M01*r.m13 + l.M02*r.m23;
	m.m13 = l.M10*r.m03 + l.M11*r.m13 + l.M12*r.m23;
	m.m23 = l.M20*r.m03 + l.M21*r.m13 + l.M22*r.m23;
	return m;
}


/*!
*
*  Implements the multiplication operator: Matrix44=Matrix33*Matrix44
*
*  Matrix33 and Matrix44 are specified in collumn order for a right-handed coordinate-system.        
*  AxB = operation B followed by operation A.  
*  A multiplication takes 36 muls and 24 adds. 
*
*  Example:
*   Matrix33 m33=Matrix33::CreateRotationX(1.94192f);;
*   Matrix44 m44=Matrix33::CreateRotationZ(3.14192f);
*	  Matrix44 result=m33*m44;
*
*/
template<class F1,int SI1,int SJ1, class F2,int SI2,int SJ2> 
ILINE Matrix44_tpl<F2,SI2,SJ2> operator * (const Matrix33_tpl<F1,SI1,SJ1>& l, const Matrix44_tpl<F2,SI2,SJ2>& r) {
	Matrix44_tpl<F2,SI2,SJ2> m;
	m(0,0) = l_00*r_00 + l_01*r_10 + l_02*r_20;
	m(1,0) = l_10*r_00 + l_11*r_10 + l_12*r_20;
	m(2,0) = l_20*r_00 + l_21*r_10 + l_22*r_20;
	m(3,0) = r_30;
	m(0,1) = l_00*r_01 + l_01*r_11 + l_02*r_21;
	m(1,1) = l_10*r_01 + l_11*r_11 + l_12*r_21;
	m(2,1) = l_20*r_01 + l_21*r_11 + l_22*r_21;
	m(3,1) = r_31;
	m(0,2) = l_00*r_02 + l_01*r_12 + l_02*r_22;
	m(1,2) = l_10*r_02 + l_11*r_12 + l_12*r_22;
	m(2,2) = l_20*r_02 + l_21*r_12 + l_22*r_22;
	m(3,2) = r_32;
	m(0,3) = l_00*r_03 + l_01*r_13 + l_02*r_23;
	m(1,3) = l_10*r_03 + l_11*r_13 + l_12*r_23;
	m(2,3) = l_20*r_03 + l_21*r_13 + l_22*r_23;
	m(3,3) = r_33;
	return m;
}





template<class F1,int SI1,int SJ1, class F2,int SI2,int SJ2> 
ILINE Matrix33_tpl<F1,SI1,SJ1>& operator *= (Matrix33_tpl<F1,SI1,SJ1> &l,	const Matrix33_tpl<F2,SI2,SJ2> &r) 
{
	F1 oldata[9]; 
	oldata[0]=l_00; oldata[1]=l_01; oldata[2]=l_02;
	oldata[3]=l_10; oldata[4]=l_11; oldata[5]=l_12; 
	oldata[6]=l_20; oldata[7]=l_21; oldata[8]=l_22; 
	l_00 = oldata[0]*r_00 + oldata[1]*r_10 +		oldata[2]*r_20;
	l_01 = oldata[0]*r_01 + oldata[1]*r_11 + 	oldata[2]*r_21;
	l_02 = oldata[0]*r_02 +	oldata[1]*r_12 + 	oldata[2]*r_22;
	l_10 = oldata[3]*r_00 +	oldata[4]*r_10 + 	oldata[5]*r_20;
	l_11 = oldata[3]*r_01 + oldata[4]*r_11 + 	oldata[5]*r_21;
	l_12 = oldata[3]*r_02 + oldata[4]*r_12 + 	oldata[5]*r_22;
	l_20 = oldata[6]*r_00 +	oldata[7]*r_10 + 	oldata[8]*r_20;
	l_21 = oldata[6]*r_01 +	oldata[7]*r_11 + 	oldata[8]*r_21;
	l_22 = oldata[6]*r_02 + oldata[7]*r_12 + 	oldata[8]*r_22;
	return l;
}






template<class F1,int SI1,int SJ1, class F2,int SI2,int SJ2> 
ILINE Matrix33_tpl<F1,SI1,SJ1> operator+(const Matrix33_tpl<F1,SI1,SJ1> &l, const Matrix33_tpl<F2,SI2,SJ2> &r) {
	Matrix33_tpl<F1,SI1,SJ1> res; 
	res.data[SI1*0+SJ1*0] = l_00+r_00; res.data[SI1*0+SJ1*1] = l_01+r_01;	res.data[SI1*0+SJ1*2] = l_02+r_02; 
	res.data[SI1*1+SJ1*0] = l_10+r_10; res.data[SI1*1+SJ1*1] = l_11+r_11; res.data[SI1*1+SJ1*2] = l_12+r_12;
	res.data[SI1*2+SJ1*0] = l_20+r_20; res.data[SI1*2+SJ1*1] = l_21+r_21; res.data[SI1*2+SJ1*2] = l_22+r_22; 
	return res;
}
template<class F1,int SI1,int SJ1, class F2,int SI2,int SJ2> 
ILINE Matrix33_tpl<F1,SI1,SJ1>& operator+=(Matrix33_tpl<F1,SI1,SJ1> &l,	const Matrix33_tpl<F2,SI2,SJ2> &r) {
	l_00+=r_00; l_01+=r_01;	l_02+=r_02; 
	l_10+=r_10;	l_11+=r_11; l_12+=r_12; 
	l_20+=r_20; l_21+=r_21;	l_22+=r_22; 
	return l;
}




template<class F1,int SI1,int SJ1, class F2,int SI2,int SJ2> 
ILINE Matrix33_tpl<F1,SI1,SJ1> operator - (const Matrix33_tpl<F1,SI1,SJ1> &l, const Matrix33_tpl<F2,SI2,SJ2> &r) {
	Matrix33_tpl<F1,SI1,SJ1> res;
	res.M1_00 = l_00-r_00;	res.M1_01 = l_01-r_01;	res.M1_02 = l_02-r_02; 
	res.M1_10 = l_10-r_10; 	res.M1_11 = l_11-r_11; 	res.M1_12 = l_12-r_12;
	res.M1_20 = l_20-r_20; 	res.M1_21 = l_21-r_21; 	res.M1_22 = l_22-r_22; 
	return res;
}
template<class F1,int SI1,int SJ1, class F2,int SI2,int SJ2> 
ILINE Matrix33_tpl<F1,SI1,SJ1>& operator-=(Matrix33_tpl<F1,SI1,SJ1> &l, const Matrix33_tpl<F2,SI2,SJ2> &r) 
{
	l_00-=r_00; l_01-=r_01;	l_02-=r_02;	
	l_10-=r_10;	l_11-=r_11; l_12-=r_12; 
	l_20-=r_20; l_21-=r_21;	l_22-=r_22; 
	return l;
}




template<class F,int SI,int SJ>
ILINE Matrix33_tpl<F,SI,SJ> operator*(const Matrix33_tpl<F,SI,SJ> &m, F op) {
	Matrix33_tpl<F,SI,SJ> res;
	res.M00=m.M00*op; res.M01=m.M01*op;	res.M02=m.M02*op; 
	res.M10=m.M10*op; res.M11=m.M11*op; res.M12=m.M12*op;
	res.M20=m.M20*op; res.M21=m.M21*op;	res.M22=m.M22*op;
	return res;
}
template<class F,int SI,int SJ>
ILINE Matrix33_tpl<F,SI,SJ> operator/(const Matrix33_tpl<F,SI,SJ> &src, F op) { return src*((F)1.0/op); }







//post-multiply
template<class F1, class F2,int SI,int SJ>
ILINE Vec3_tpl<F1> operator*(const Matrix33_tpl<F2,SI,SJ> &m, const Vec3_tpl<F1> &v) {
	return Vec3_tpl<F1>(v.x*m.M00 + v.y*m.M01 + v.z*m.M02,
		                  v.x*m.M10 + v.y*m.M11 + v.z*m.M12,
		                  v.x*m.M20 + v.y*m.M21 + v.z*m.M22);
}

//pre-multiply
template<class F1, class F2,int SI,int SJ>
ILINE Vec3_tpl<F1> operator*(const Vec3_tpl<F1> &v, const Matrix33_tpl<F2,SI,SJ> &m) {
	return Vec3_tpl<F1>(v.x*m.M00 + v.y*m.M10 + v.z*m.M20,
		                  v.x*m.M01 + v.y*m.M11 + v.z*m.M21,
		                  v.x*m.M02 + v.y*m.M12 + v.z*m.M22);
}

template<class F1,int SI,int SJ> 
ILINE Matrix33_tpl<F1,SI,SJ>& crossproduct_matrix(const Vec3_tpl<F1> &v, Matrix33_tpl<F1,SI,SJ> &m) {
	m.M00=0;			m.M01=-v.z;		m.M02=v.y;
	m.M10=v.z;    m.M11=0;			m.M12=-v.x;
	m.M20=-v.y;	  m.M21=v.x;		m.M22=0;
	return m;
}

template<class F1,int SI,int SJ> 
ILINE Matrix33_tpl<F1,SI,SJ>& dotproduct_matrix(const Vec3_tpl<F1> &v, const Vec3_tpl<F1> &op, Matrix33_tpl<F1,SI,SJ> &m) {
	m.M00=v.x*op.x;		m.M10=v.y*op.x;		m.M20=v.z*op.x;
	m.M01=v.x*op.y;		m.M11=v.y*op.y;		m.M21=v.z*op.y;
	m.M02=v.x*op.z;		m.M12=v.y*op.z;		m.M22=v.z*op.z;
	return m;
}















///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Matrix34_tpl
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <typename F> struct Matrix34_tpl {

#ifdef SIMD_CODE
	//specialized format for PS2 and Intel-Pentium  (maybe 3d_NOW)
	F m00,m10,m20, pad30;
	F m01,m11,m21, pad31;
	F m02,m12,m22, pad32;
	F m03,m13,m23, pad33;
#else
	//! generic code GAMECUBE / PC
	F m00,m01,m02,m03;
	F m10,m11,m12,m13;
	F m20,m21,m22,m23;
#endif



	//default constructor
	Matrix34_tpl() {}


	//overload = operator to copy double=doube or f32=f32
	ILINE Matrix34_tpl& operator=(const Matrix34_tpl<F> &m) {
		m00=m.m00;		m01=m.m01;		m02=m.m02; m03=m.m03;	
		m10=m.m10;		m11=m.m11;		m12=m.m12; m13=m.m13;
		m20=m.m20;		m21=m.m21;		m22=m.m22; m23=m.m23;
		return *this;
	}


	explicit ILINE Matrix34_tpl( const Quaternion_tpl<F>& q ){	*this=Matrix33(q);	}

	template<int SI,int SJ> 
	ILINE  Matrix34_tpl(const Matrix33_tpl<F,SI,SJ>& m) {
			m00=m.M00;		m01=m.M01;		m02=m.M02;		m03=0;	
			m10=m.M10;		m11=m.M11;		m12=m.M12;		m13=0;
			m20=m.M20;		m21=m.M21;		m22=m.M22;		m23=0;
		}

	ILINE Matrix34_tpl( const Matrix34_tpl<F>& m ) {
			m00=m.m00;		m01=m.m01;		m02=m.m02;	m03=m.m03;	
			m10=m.m10;		m11=m.m11;		m12=m.m12;	m13=m.m13;
			m20=m.m20;		m21=m.m21;		m22=m.m22;	m23=m.m23;
		}

	template<int SI,int SJ> 
	ILINE explicit Matrix34_tpl(const Matrix44_tpl<F,SI,SJ>& m ) {
			m00=m.M00;		m01=m.M01;		m02=m.M02;		m03=m.M03;	
			m10=m.M10;		m11=m.M11;		m12=m.M12;		m13=m.M13;
			m20=m.M20;		m21=m.M21;		m22=m.M22;		m23=m.M23;
		}





		void SetIdentity( void );
		static Matrix34_tpl<F> CreateIdentity( void );


		void SetTranslation( const Vec3_tpl<F>& t );
		Vec3_tpl<F> GetTranslation() const;

		void SetRotationAA(const F rad, const Vec3_tpl<F>& axis, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) );
		static Matrix34_tpl<F> CreateRotationAA( const F rad, const Vec3_tpl<F>& axis, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) );


		void SetRotationX(const f32 rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) );
		static Matrix34_tpl<F> CreateRotationX( const f32 rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f)   );
		void SetRotationY(const f32 rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) );
		static Matrix34_tpl<F> CreateRotationY( const f32 rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f)   );
		void SetRotationZ(const f32 rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) );
		static Matrix34_tpl<F> CreateRotationZ( const f32 rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f)   );


		void SetRotationXYZ( const Ang3_tpl<F>& rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) );
		static Matrix34_tpl<F> CreateRotationXYZ( const Ang3_tpl<F>& rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f)  );

		void SetScale( const Vec3_tpl<F> &s, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) );
		static Matrix34_tpl<F> CreateScale(  const Vec3_tpl<F> &s, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f)  );


		ILINE void SetTranslationMat(  const Vec3_tpl<F>& v  ) {
			m00=1.0f;	m01=0.0f;	m02=0.0f;	m03=v.x;
			m10=0.0f;	m11=1.0f;	m12=0.0f;	m13=v.y;
			m20=0.0f;	m21=0.0f;	m22=1.0f;	m23=v.z;
		}
		ILINE static Matrix34_tpl<F> CreateTranslationMat(  const Vec3_tpl<F>& v  ) {	Matrix34_tpl<F> m34; m34.SetTranslationMat(v); return m34; 	}
	

		//NOTE: all vectors are stored in columns
		ILINE void SetMatFromVectors(const Vec3& vx, const Vec3& vy, const Vec3& vz, const Vec3& pos)	{
			m00=vx.x;		m01=vy.x;		m02=vz.x;		m03 = pos.x;
			m10=vx.y;		m11=vy.y;		m12=vz.y;		m13 = pos.y;
			m20=vx.z;		m21=vy.z;		m22=vz.z;		m23 = pos.z;
		}
		ILINE static Matrix34_tpl<F> CreateMatFromVectors(const Vec3_tpl<F>& vx, const Vec3_tpl<F>& vy, const Vec3_tpl<F>& vz, const Vec3_tpl<F>& pos) {
			Matrix34_tpl<F> m; m.SetMatFromVectors34(vx,vy,vz,pos); return m;
		}

		void Invert( void );
		Matrix34_tpl<F> GetInverted();

		ILINE F operator () (unsigned i, unsigned j) const {
					assert ((i<3) && (j<4));
					F* p_data=(F*)(&m00);
					return p_data[i*4+j];
		}
		ILINE F& operator () (unsigned i, unsigned j)	{
				assert ((i<3) && (j<4));
				F* p_data=(F*)(&m00);
				return p_data[i*4+j];
		}



    //NOTE: vector p is multiplied by the rows of the matrix
    //this it not the same as in M44
		ILINE Vec3 TransformVector(const Vec3 &p) const  {
			Vec3 v;
			v.x = m00*p.x + m01*p.y + m02*p.z;
			v.y = m10*p.x + m11*p.y + m12*p.z;
			v.z = m20*p.x + m21*p.y + m22*p.z;
			return v;
		}

		//! transform a point ans add translation vector
		//NOTE: vector p is multiplied by the rows of the matrix
		//this it not the same as in M44
		ILINE Vec3 TransformPoint(const Vec3 &p) const  {
			Vec3 v;
			v.x = m00*p.x + m01*p.y + m02*p.z + m03;
			v.y = m10*p.x + m11*p.y + m12*p.z + m13;
			v.z = m20*p.x + m21*p.y + m22*p.z + m23;
			return v;
		}


};

///////////////////////////////////////////////////////////////////////////////
// Typedefs                                                                  //
///////////////////////////////////////////////////////////////////////////////

typedef Matrix34_tpl<f32> Matrix34;
typedef Matrix34_tpl<real> Matrix34_f64;


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//-------------       implementation of Matrix34      ------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

/*!
*
*  multiplication of Matrix34 by a (column) Vec3.
*  This function transforms the given input Vec3
*  into the coordinate system defined by this matrix.
*
*  Example:
*   Matrix34 m34;
*   Vec3 vector(44,55,66);
*   Vec3 result = m34*vector;
*
*/
template<class F> 
ILINE Vec3_tpl<F> operator * (const Matrix34_tpl<F>& m, const Vec3_tpl<F> &p) {
	Vec3_tpl<F> tp;
	tp.x	=	m.m00*p.x + m.m01*p.y + m.m02*p.z + m.m03;
	tp.y	=	m.m10*p.x + m.m11*p.y + m.m12*p.z + m.m13;
	tp.z	=	m.m20*p.x + m.m21*p.y + m.m22*p.z + m.m23;
	return	tp;
}


/*!
*
*  Implements the multiplication operator: Matrix34=Matrix34*Matrix33
*
*  Matrix33 and Matrix44 are specified in collumn order for a right-handed coordinate-system.        
*  AxB = operation B followed by operation A.  
*  A multiplication takes 27 muls and 24 adds. 
*
*  Example:
*   Matrix34 m34=Matrix33::CreateRotationX(1.94192f);;
*   Matrix33 m33=Matrix34::CreateRotationZ(3.14192f);
*	  Matrix34 result=m34*m33;
*
*/

template<class F,int SI,int SJ> 
ILINE Matrix34_tpl<F> operator * (const Matrix34_tpl<F>& l, const Matrix33_tpl<F,SI,SJ>& r) {
	Matrix34_tpl<F> m;
	m.m00 = l.m00*r.M00 + l.m01*r.M10 + l.m02*r.M20;
	m.m10 = l.m10*r.M00 + l.m11*r.M10 + l.m12*r.M20;
	m.m20 = l.m20*r.M00 + l.m21*r.M10 + l.m22*r.M20;
	m.m01 = l.m00*r.M01 + l.m01*r.M11 + l.m02*r.M21;
	m.m11 = l.m10*r.M01 + l.m11*r.M11 + l.m12*r.M21;
	m.m21 = l.m20*r.M01 + l.m21*r.M11 + l.m22*r.M21;
	m.m02 = l.m00*r.M02 + l.m01*r.M12 + l.m02*r.M22;
	m.m12 = l.m10*r.M02 + l.m11*r.M12 + l.m12*r.M22;
	m.m22 = l.m20*r.M02 + l.m21*r.M12 + l.m22*r.M22;
	m.m03 = l.m03;
	m.m13 = l.m13;
	m.m23 = l.m23;
	return m;
}



/*!
*
*  Implements the multiplication operator: Matrix34=Matrix34*Matrix34
*
*  Matrix34 is specified in collumn order.        
*  AxB = rotation B followed by rotation A.  
*  This operation takes 36 mults and 27 adds. 
*
*  Example:
*   Matrix34 m34=Matrix34::CreateRotationX(1.94192f, Vec3(11,22,33));;
*   Matrix34 m34=Matrix33::CreateRotationZ(3.14192f);
*	  Matrix34 result=m34*m34;
*
*/
template<class F> 
ILINE Matrix34_tpl<F> operator * (const Matrix34_tpl<F>& l, const Matrix34_tpl<F>& r) {
	Matrix34_tpl<F> m;
	m.m00 = l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20;
	m.m10 = l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20;
	m.m20 = l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20;
	m.m01 = l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21;
	m.m11 = l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21;
	m.m21 = l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21;
	m.m02 = l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22;
	m.m12 = l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22;
	m.m22 = l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22;
	m.m03 = l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23 + l.m03;
	m.m13 = l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23 + l.m13;
	m.m23 = l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23 + l.m23;
	return m;
}


/*!
*
*  Implements the multiplication operator: Matrix44=Matrix34*Matrix44
*
*  Matrix44 and Matrix34 are specified in collumn order.         
*	 AxB = rotation B followed by rotation A.  
*  This operation takes 48 mults and 36 adds.  
*
*  Example:
*   Matrix34 m34=Matrix33::CreateRotationX(1.94192f);;
*   Matrix44 m44=Matrix33::CreateRotationZ(3.14192f);
*	  Matrix44 result=m34*m44;
*
*/

template<class F,int SI,int SJ> 
ILINE Matrix44_tpl<F,SI,SJ> operator * (const Matrix34_tpl<F>& l, const Matrix44_tpl<F,SI,SJ>& r) {
	Matrix44_tpl<F,SI,SJ> m;
	m.M00 = l.m00*r.M00 + l.m01*r.M10 + l.m02*r.M20 + l.m03*r.M30;
	m.M10 = l.m10*r.M00 + l.m11*r.M10 + l.m12*r.M20 + l.m13*r.M30;
	m.M20 = l.m20*r.M00 + l.m21*r.M10 + l.m22*r.M20 + l.m23*r.M30;
	m.M30 = r.M30;
	m.M01 = l.m00*r.M01 + l.m01*r.M11 + l.m02*r.M21 + l.m03*r.M31;
	m.M11 = l.m10*r.M01 + l.m11*r.M11 + l.m12*r.M21 + l.m13*r.M31;
	m.M21 = l.m20*r.M01 + l.m21*r.M11 + l.m22*r.M21 + l.m23*r.M31;
	m.M31 = r.M31;
	m.M02 = l.m00*r.M02 + l.m01*r.M12 + l.m02*r.M22 + l.m03*r.M32;
	m.M12 = l.m10*r.M02 + l.m11*r.M12 + l.m12*r.M22 + l.m13*r.M32;
	m.M22 = l.m20*r.M02 + l.m21*r.M12 + l.m22*r.M22 + l.m23*r.M32;
	m.M32 = r.M32;
	m.M03 = l.m00*r.M03 + l.m01*r.M13 + l.m02*r.M23 + l.m03*r.M33;
	m.M13 = l.m10*r.M03 + l.m11*r.M13 + l.m12*r.M23 + l.m13*r.M33;
	m.M23 = l.m20*r.M03 + l.m21*r.M13 + l.m22*r.M23 + l.m23*r.M33;
	m.M33 = r.M33;
	return m;
}





/*!
*
*  Initializes the Matrix34 with the identity.
*
*/
template<class F> 
ILINE void Matrix34_tpl<F>::SetIdentity( void ) {
	m00=1.0f;	m01=0.0f;	m02=0.0f;	m03=0.0f;
	m10=0.0f;	m11=1.0f;	m12=0.0f;	m13=0.0f;
	m20=0.0f;	m21=0.0f; m22=1.0f;	m23=0.0f;
}
template<class F> 
ILINE Matrix34_tpl<F> Matrix34_tpl<F>::CreateIdentity( void ) {	Matrix34_tpl<F> m34; m34.SetIdentity();	return m34;	}


/*!
* set translation vector in Matrix34 .
*
*  Example:
*		Matrix34 m34;
*		m34.SetTranslation( Vec3 (0.5f, 1.0f, 2.0f) );
*/
template<class F> 
ILINE void Matrix34_tpl<F>::SetTranslation( const Vec3_tpl<F>& t ) { m03=t.x;	m13=t.y; m23=t.z;	}

/*!
*
* get translation vector from Matrix34 .
*
*  Example 1:
*		Matrix m34;
*		Vec3 v=m34.GetTranslation();
*
*/
template<class F> 
ILINE Vec3_tpl<F> Matrix34_tpl<F>::GetTranslation() const { return Vec3_tpl<F>(m03,m13,m23); }


/*!
*
*  Create a rotation matrix around an arbitrary axis (Eulers Theorem).  
*  The axis is specified as an normalized Vec3. The angle is assumed to be in radians.  
*  This function also assumes a translation-vector and stores it in the right column.  
*
*  Example:
*		Matrix34 m34;
*		Vec3 axis=GetNormalized( Vec3(-1.0f,-0.3f,0.0f) );
*		m34.SetRotationAA( 3.14314f, axis, Vec3(5,5,5) );
*/
template<class F> 
ILINE void Matrix34_tpl<F>::SetRotationAA(const F rad, const Vec3_tpl<F>& axis, const Vec3_tpl<F>& t )	{
	assert((fabs_tpl(1-(axis|axis)))<0.001); //check if unit-vector
	*this=Matrix33::CreateRotationAA(rad,axis); this->SetTranslation(t);
}
template<class F> 
ILINE Matrix34_tpl<F> Matrix34_tpl<F>::CreateRotationAA( const F rad, const Vec3_tpl<F>& axis, const Vec3_tpl<F>& t )	{	Matrix34_tpl<F> m34;  m34.SetRotationAA(rad,axis,t);	return m34;	}


/*!
* Create rotation-matrix about X axis using an angle.
* The angle is assumed to be in radians. 
* The translation-vector is set to zero.  
*
*  Example:
*		Matrix34 m34;
*		m34.SetRotationX(0.5f);
*/
template<class F> 
ILINE void Matrix34_tpl<F>::SetRotationX(const f32 rad, const Vec3_tpl<F>& t )	{
	*this=Matrix33::CreateRotationX(rad); this->SetTranslation(t);
}
template<class F> 
ILINE Matrix34_tpl<F> Matrix34_tpl<F>::CreateRotationX( const f32 rad, const Vec3_tpl<F>& t   )	{		Matrix34_tpl<F> m34;  m34.SetRotationX(rad,t);	return m34;	}

/*!
* Create rotation-matrix about Y axis using an angle.
* The angle is assumed to be in radians. 
* The translation-vector is set to zero.  
*
*  Example:
*		Matrix34 m34;
*		m34.SetRotationY(0.5f);
*/
template<class F> 
ILINE void Matrix34_tpl<F>::SetRotationY(const f32 rad, const Vec3_tpl<F>& t )	{
	*this=Matrix33::CreateRotationY(rad);	this->SetTranslation(t);
}
template<class F> 
ILINE Matrix34_tpl<F> Matrix34_tpl<F>::CreateRotationY( const f32 rad, const Vec3_tpl<F>& t   )	{	Matrix34_tpl<F> m34;  m34.SetRotationY(rad,t);	return m34;	}

/*!
* Create rotation-matrix about Z axis using an angle.
* The angle is assumed to be in radians. 
* The translation-vector is set to zero.  
*
*  Example:
*		Matrix34 m34;
*		m34.SetRotationZ(0.5f);
*/
template<class F> 
ILINE void Matrix34_tpl<F>::SetRotationZ(const f32 rad, const Vec3_tpl<F>& t )	{
	*this=Matrix33::CreateRotationZ(rad);  this->SetTranslation(t);
}
template<class F> 
ILINE Matrix34_tpl<F> Matrix34_tpl<F>::CreateRotationZ( const f32 rad, const Vec3_tpl<F>& t   )	{	Matrix34_tpl<F> m34;  m34.SetRotationZ34(rad,t);	return m34;	}


/*!
*
* Convert three Euler angle to mat33 (rotation order:XYZ)
* The Euler angles are assumed to be in radians. 
* The translation-vector is set to zero.  
*
*  Example 1:
*		Matrix34 m34;
*		m34.SetRotationXYZ( Ang3(0.5f,0.2f,0.9f), translation );
*
*  Example 2:
*		Matrix34 m34=Matrix34::CreateRotationXYZ( Ang3(0.5f,0.2f,0.9f), translation );
*/
template<class F> 
ILINE void Matrix34_tpl<F>::SetRotationXYZ( const Ang3_tpl<F>& rad, const Vec3_tpl<F>& t ) {
	*this=Matrix33::CreateRotationXYZ(rad); this->SetTranslation(t);
}
template<class F> 
ILINE Matrix34_tpl<F> Matrix34_tpl<F>::CreateRotationXYZ( const Ang3_tpl<F>& rad, const Vec3_tpl<F>& t  )	{	Matrix34_tpl<F> m34;  m34.SetRotationXYZ(rad,t);	return m34;	}


/*!
* Create scaling-matrix.
* The translation-vector is set to zero.  
*
*  Example 1:
*		Matrix m34;
*		m34.SetScale( Vec3(0.5f, 1.0f, 2.0f) );
*
*  Example 2:
*		Matrix34 m34 = Matrix34::CreateScale( Vec3(0.5f, 1.0f, 2.0f) );
*
*/
/*template<class F> 
ILINE void Matrix34_tpl<F>::SetScale( const Vec3_tpl<F> &s, const Vec3_tpl<F>& t ) {
	*this=Matrix33::GetScale(s); this->SetTranslation(t);
}
*/
template<class F> 
ILINE Matrix34_tpl<F> Matrix34_tpl<F>::CreateScale(  const Vec3_tpl<F> &s, const Vec3_tpl<F>& t  )	{ 	Matrix34_tpl<F> m34;  m34.SetScale(s,t);	return m34;	}


/*!
* calculate a real inversion of a Matrix34
* an inverse-matrix is an UnDo-matrix for all kind of transformations 
* 
*  Example 1:
*		Matrix34 im34;
*		im34.Invert();
*
*  Example 2:
*   Matrix34 im34=m34.GetInverted();
*/
template<class F> 
ILINE void Matrix34_tpl<F>::Invert( void ) {
	//rescue members	
	Matrix34_tpl<F>	m=*this;
	// calculate 12 cofactors
	m00= m.m22*m.m11-m.m12*m.m21;
	m10= m.m12*m.m20-m.m22*m.m10;
	m20= m.m10*m.m21-m.m20*m.m11;
	m01= m.m02*m.m21-m.m22*m.m01;
	m11= m.m22*m.m00-m.m02*m.m20;
	m21= m.m20*m.m01-m.m00*m.m21;
	m02= m.m12*m.m01-m.m02*m.m11;
	m12= m.m02*m.m10-m.m12*m.m00;
	m22= m.m00*m.m11-m.m10*m.m01;
	m03= (m.m22*m.m13*m.m01 + m.m02*m.m23*m.m11 + m.m12*m.m03*m.m21) - (m.m12*m.m23*m.m01 + m.m22*m.m03*m.m11 + m.m02*m.m13*m.m21);
	m13= (m.m12*m.m23*m.m00 + m.m22*m.m03*m.m10 + m.m02*m.m13*m.m20) - (m.m22*m.m13*m.m00 + m.m02*m.m23*m.m10 + m.m12*m.m03*m.m20);
	m23= (m.m20*m.m11*m.m03 + m.m00*m.m21*m.m13 + m.m10*m.m01*m.m23) - (m.m10*m.m21*m.m03 + m.m20*m.m01*m.m13 + m.m00*m.m11*m.m23);
	// calculate determinant
	F det=1.0f/(m.m00*m00+m.m10*m01+m.m20*m02);
	assert(det>0.0001);
	// calculate matrix inverse/
	m00*=det; m01*=det; m02*=det; m03*=det;
	m10*=det; m11*=det; m12*=det; m13*=det;
	m20*=det; m21*=det; m22*=det; m23*=det;
}

template<class F> 
ILINE Matrix34_tpl<F> Matrix34_tpl<F>::GetInverted() {	Matrix34_tpl<F> dst=*this; dst.Invert(); return dst; }


/*!
*
*  Name:             ReflectMat34
*
*  Description:      reflect a rotation matrix with respect to a plane.
*
*  Example:
*
* \code
*		Vec3 normal( 0.0f,-1.0f, 0.0f);
*		Vec3 pos(0,1000,0);
*		Matrix34 m34=CreateReflectionMat( pos, normal );
* \endcode
*/

ILINE Matrix34 CreateReflectionMat ( const Vec3& p, const Vec3& n )
{
	Matrix34 m;	
	f32 vxy   = -2.0f * n.x * n.y;
	f32 vxz   = -2.0f * n.x * n.z;
	f32 vyz   = -2.0f * n.y * n.z;
	f32 pdotn = 2.0f * (p|n);

	m.m00=1.0f-2.0f*n.x*n.x;	m.m01=vxy;    						m.m02=vxz;    						m.m03=pdotn*n.x;
	m.m10=vxy;  							m.m11=1.0f-2.0f*n.y*n.y; 	m.m12=vyz;    						m.m13=pdotn*n.y;
	m.m20=vxz;  							m.m21=vyz;   							m.m22=1.0f-2.0f*n.z*n.z; 	m.m23=pdotn*n.z;

	return m;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// struct Matrix44_tpl
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<class F,int SI,int SJ> struct Matrix44_tpl;
///////////////////////////////////////////////////////////////////////////////
// Typedefs                                                                  //
///////////////////////////////////////////////////////////////////////////////

typedef Matrix44_tpl<f32,4,1> Matrix44;
typedef Matrix44_tpl<real,4,1> Matrix44_f64;


template<class F,int SI,int SJ> struct Matrix44_tpl {

	F data[(SI-(SI-SJ & (SI-SJ)>>31))*4];

	//---------------------------------------------------------------------------------

	Matrix44_tpl()  {}   

	template<class F1,int SI1,int SJ1> 
	ILINE Matrix44_tpl(const Matrix33_tpl<F1,SI1,SJ1>& m ) {
			M00=m_00;		M01=m_01;		M02=m_02;		M03=0;	
			M10=m_10;		M11=m_11;		M12=m_12;		M13=0;
			M20=m_20;		M21=m_21;		M22=m_22;		M23=0;
			M30=0;			M31=0;			M32=0;			M33=1;
	}
	ILINE Matrix44_tpl(const Matrix34_tpl<F>& m ) {
			M00=m.m00;		M01=m.m01;		M02=m.m02;		M03=m.m03;	
			M10=m.m10;		M11=m.m11;		M12=m.m12;		M13=m.m13;
			M20=m.m20;		M21=m.m21;		M22=m.m22;		M23=m.m23;
			M30=0;				M31=0;				M32=0;				M33=1;
		}
	template<int SI1,int SJ1> 
	ILINE Matrix44_tpl(const Matrix44_tpl<F,SI1,SJ1>& m ) {
			assert (  (SI==SI1) | ((void*)this!=(void*)&m) );
			M00=m.M00;		M01=m.M01;		M02=m.M02;	M03=m.M03;	
			M10=m.M10;		M11=m.M11;		M12=m.M12;	M13=m.M13;
			M20=m.M20;		M21=m.M21;		M22=m.M22;	M23=m.M23;
			M30=m.M30; 	  M31=m.M31;	  M32=m.M32;	M33=m.M33;
	}
	explicit ILINE Matrix44_tpl( const Quaternion_tpl<F>& q ){	*this=Matrix33(q);	}


	// non-templated version is needed since default operator= has precedence over templated operator=
	template<class F1,int SI1,int SJ1> 
	Matrix44_tpl& operator=(const Matrix33_tpl<F1,SI1,SJ1> &m) { 
			M00=m_00;		M01=m_01;		M02=m_02;		M03=0;	
			M10=m_10;		M11=m_11;		M12=m_12;		M13=0;
			M20=m_20;		M21=m_21;		M22=m_22;		M23=0;
			M30=0;			M31=0;			M32=0;			M33=1;
			return *this; 
		}
	// non-templated version is needed since default operator= has precedence over templated operator=
	Matrix44_tpl& operator=(const Matrix34_tpl<F> &m) { 
			M00=m.m00;		M01=m.m01;		M02=m.m02;		M03=m.m03;	
			M10=m.m10;		M11=m.m11;		M12=m.m12;		M13=m.m13;
			M20=m.m20;		M21=m.m21;		M22=m.m22;		M23=m.m23;
			M30=0;				M31=0;				M32=0;				M33=1;
			return *this; 
	}

	// non-templated version is needed since default operator= has precedence over templated operator=
	ILINE Matrix44_tpl& operator=(const Matrix44_tpl<F,SI,SJ> &m) { 
			M00=m.M00;	M01=m.M01;	M02=m.M02; 	M03=m.M03; 
			M10=m.M10;	M11=m.M11;	M12=m.M12; 	M13=m.M13;
			M20=m.M20;	M21=m.M21;	M22=m.M22; 	M23=m.M23;
			M30=m.M30;	M31=m.M31;	M32=m.M32; 	M33=m.M33;
			return *this; 
	}
	template<class F1,int SI1,int SJ1> 
	ILINE  Matrix44_tpl& operator = (const Matrix44_tpl<F1,SI1,SJ1>& m) { 
			assert (  (SI==SI1) | ((void*)this!=(void*)&m) );
			M00=m_00;	M01=m_01;		M02=m_02;		M03=m_03; 
			M10=m_10;	M11=m_11;		M12=m_12;		M13=m_13;
			M20=m_20;	M21=m_21;		M22=m_22; 	M23=m_23;
			M30=m_30;	M31=m_31;		M32=m_32; 	M33=m_33;
			return *this; 
	}


		//---------------------------------------------------------------------------------

		//! build a matrix from 3 vectors
/*		ILINE Matrix44_tpl(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3)	{ 
			M00=v1.x; M01=v2.x; M02=v3.x;	M03=0;
			M10=v1.y; M11=v2.y; M12=v3.y;	M13=0;
			M20=v1.z; M21=v2.z; M22=v3.z;	M23=0;
			M30=0;		M31=0;		M32=0;		M33=1;
		}*/

/*		//! build a matrix from 4 vectors
		ILINE Matrix44_tpl(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Vec3 &v4)  { 
			M00=v1.x; M01=v2.x; M02=v3.x;	M03=v4.x;
			M10=v1.y; M11=v2.y; M12=v3.y;	M13=v4.y;
			M20=v1.z; M21=v2.z; M22=v3.z;	M23=v4.z;
			M30=0;		M31=0;		M32=0;		M33=1;
		}*/


		//! build a matrix from 16 f32s
		ILINE Matrix44_tpl(f32 v00, f32 v01, f32 v02, f32 v03,
			                 f32 v10, f32 v11, f32 v12, f32 v13,
			                 f32 v20, f32 v21, f32 v22, f32 v23,
			                 f32 v30, f32 v31, f32 v32, f32 v33)
		{ 
			M00=v00; M01=v01; M02=v02; M03=v03; 
			M10=v10; M11=v11; M12=v12; M13=v13; 
			M20=v20; M21=v21; M22=v22; M23=v23; 
			M30=v30; M31=v31; M32=v32; M33=v33; 
		}

		//! build a matrix from a pointer to a f32 array
		Matrix44_tpl(const F *p) {
			M00 = p[ 0]; M01 = p[ 1]; M02 = p[ 2]; M03 = p[ 3];
			M10 = p[ 4]; M11 = p[ 5]; M12 = p[ 6]; M13 = p[ 7];
			M20 = p[ 8]; M21 = p[ 9]; M22 = p[10]; M23 = p[11];
			M30 = p[12]; M31 = p[13]; M32 = p[14]; M33 = p[15];
		}

		//!post-transform: matrix*vector
		ILINE friend	Vec3 operator * (const Matrix44 &m, const Vec3 &v)	{
			return Vec3(		m.M00*v.x + m.M01*v.y + m.M02*v.z,
				              m.M10*v.x + m.M11*v.y + m.M12*v.z,
				              m.M20*v.x + m.M21*v.y + m.M22*v.z );
		}


		//! multiply the matrix by another matrix (Anton has an optimized version for this)
		ILINE  Matrix44_tpl<F,SI,SJ>	&operator *= (const Matrix44_tpl<F,SI,SJ> &m) { operator = (*this * m); return *this;}    

		//---------------------------------------------------------------------
		
		//! multiply all m1 matrix's values by f and return the matrix
		ILINE friend	Matrix44_tpl<F,SI,SJ> operator * (const Matrix44_tpl<F,SI,SJ>& m, const f32 f)	{ 
			Matrix44_tpl<F,SI,SJ> r;
			r.M00=m.M00*f;	r.M01=m.M01*f;	r.M02=m.M02*f;	r.M03=m.M03*f; 
			r.M10=m.M10*f;	r.M11=m.M11*f;	r.M12=m.M12*f;	r.M13=m.M13*f;
			r.M20=m.M20*f;	r.M21=m.M21*f;	r.M22=m.M22*f;	r.M23=m.M23*f;
			r.M30=m.M30*f;	r.M31=m.M31*f;	r.M32=m.M32*f;	r.M33=m.M33*f;
			return r;
		}	

		//! multiply all m1 matrix's values by f and return the matrix
		ILINE void operator *= (const f32 f)	{ 
			M00*=f;	M01*=f;	M02*=f;	M03*=f; 
			M10*=f;	M11*=f;	M12*=f;	M13*=f;
			M20*=f;	M21*=f;	M22*=f;	M23*=f;
			M30*=f;	M31*=f;	M32*=f;	M33*=f;
		}	


		ILINE void SetIdentity()	{
			M00=1;		M01=0;		M02=0;	M03=0;
			M10=0;		M11=1;		M12=0;	M13=0;
			M20=0;		M21=0;		M22=1;	M23=0;
			M30=0;		M31=0;		M32=0;	M33=1;
		}


		ILINE f32 Determinant() const		{
			//determinant is ambiguous: only the upper-left-submatrix's determinant is calculated
			return (M00*M11*M22) + (M01*M12*M20) + (M02*M10*M21) - (M02*M11*M20) - (M00*M12*M21) - (M01*M10*M22);
		}

		//all vectors are stored in columns, if created with this mathlib
		Vec3 GetOrtX() const	{	return Vec3( M00, M10, M20 );	}
		Vec3 GetOrtY() const	{	return Vec3( M01, M11, M21 );	}
		Vec3 GetOrtZ() const	{	return Vec3( M02, M12, M22 );	}


		ILINE Vec3_tpl<F> GetRow(int iRow) const {	return Vec3_tpl<F>(	data[iRow*SI+0*SJ],	data[iRow*SI+1*SJ],	data[iRow*SI+2*SJ] );	}
		ILINE void SetRow(int iRow, const Vec3_tpl<F> &row)	{	data[iRow*SI+0*SJ]=row.x;	data[iRow*SI+1*SJ]=row.y;		data[iRow*SI+2*SJ]=row.z;	}

		Vec3_tpl<F> GetColumn(int iCol) const	{	return Vec3_tpl<F>(	data[0*SI+iCol*SJ],	data[1*SI+iCol*SJ],		data[2*SI+iCol*SJ]	);	}
		void SetColumn(int iCol, const Vec3_tpl<F> &col)	{	data[0*SI+iCol*SJ]=col.x;		data[1*SI+iCol*SJ]=col.y;		data[2*SI+iCol*SJ]=col.z;	}

		F *GetData() { return data; }
		const F *GetData() const { return data; }

		F operator () (unsigned i, unsigned j) const {	assert ((i<4) && (j<4));	return data[i*SI+j*SJ];		}
		F& operator () (unsigned i, unsigned j)	{		assert ((i<4) && (j<4)); return data[i*SI+j*SJ];	}

		ILINE	F* operator [] (int index)				{ return &data[index*SI]; }	
		ILINE	const F* operator [] (int index) const	{ return &data[index*SI]; }	



		/*!
		*
		* transpose a Matrix
		* a transpose is an UnDo-matrix for orthogonal rotations 
		*
		*	|xx xy xz xw|    |xx yx zx wx|
		*	|yx yy yz yw| -> |xy yy zy wy|
		*	|zx zy zz zw|    |xz yz zz wz|
		*	|zx zy zz ww|    |xw yw zw ww|
		*
		*  Example 1:
		*  Matrix m44;
		*	 m44.Transpose();
		*
		*  Example 2:
		*	 Matrix44 t44=GetTransposed44(m44);
		*/
		void Transpose()	{
			Matrix44 m;
			m.M00=M00;		m.M01=M10;		m.M02=M20; 	m.M03=M30;
			m.M10=M01;		m.M11=M11;		m.M12=M21; 	m.M13=M31;
			m.M20=M02;		m.M21=M12;		m.M22=M22; 	m.M23=M32;
			m.M30=M03;		m.M31=M13;		m.M32=M23; 	m.M33=M33;
			M00=m.M00;		M01=m.M01;		M02=m.M02;	M03=m.M03;
			M10=m.M10;		M11=m.M11;		M12=m.M12;	M13=m.M13;
			M20=m.M20;		M21=m.M21;		M22=m.M22;	M23=m.M23;
			M30=m.M30;		M31=m.M31;		M32=m.M32;	M33=m.M33;
		}
		ILINE friend Matrix44_tpl<F,SI,SJ> GetTransposed44( const Matrix44_tpl<F,SI,SJ>& m ) {
			Matrix44_tpl<F,SI,SJ> dst;
			dst.M00=m.M00;	dst.M01=m.M10;	dst.M02=m.M20;	dst.M03=m.M30;
			dst.M10=m.M01;	dst.M11=m.M11;	dst.M12=m.M21;	dst.M13=m.M31;
			dst.M20=m.M02;	dst.M21=m.M12;	dst.M22=m.M22;	dst.M23=m.M32;
			dst.M30=m.M03;	dst.M31=m.M13;	dst.M32=m.M23;	dst.M33=m.M33;
			return dst;
		}




		/*!
		*  Create a rotation matrix around an arbitrary axis (Eulers Theorem).  
		*  The axis is specified as an normalized CVectoa. The angle is assumed to be in radians.  
		*
		*  Example 1:
		*		Vec3 axis=GetNormalized(Vec3(-1.0f, -0.3f, 0.0f));
		*		Matrix m44;
		*   m44.SetRotationAA44( 3.14314f, axis );
		*
		*  Example 2:
		*		Matrix44 m44=CreateRotationAA44<f32>( 3.14314f, axis );
		*/
		//member function
	/*	ILINE void SetRotationAA44(f32 rad, const Vec3_tpl<F>& axis, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) ) {
			assert((fabs_tpl(1-(axis|axis)))<0.001); //check if unit-vector
			F	c		= cos_tpl(rad);
			F	s		= sin_tpl(rad);
			F	mc	=	(F)1.0-c;
			M00=mc*axis.x*axis.x + c;					M01=mc*axis.x*axis.y - axis.z*s;	M02=mc*axis.x*axis.z + axis.y*s;	M03=t.x;
			M10=mc*axis.y*axis.x + axis.z*s;	M11=mc*axis.y*axis.y + c;					M12=mc*axis.y*axis.z - axis.x*s;	M13=t.y;
			M20=mc*axis.z*axis.x - axis.y*s;	M21=mc*axis.z*axis.y + axis.x*s;	M22=mc*axis.z*axis.z + c;					M23=t.z;
			M30=0.0f;													M31=0.0f;													M32=0.0f;													M33=1.0f;
		}
		ILINE friend Matrix44_tpl<F,4,1> CreateRotationAA44(f32 rad, const Vec3_tpl<F>& axis, const Vec3_tpl<F>& t  ) {
			Matrix44_tpl<F,4,1> m44; m44.SetRotationAA44(rad,axis,t); return m44;
		}
		ILINE friend Matrix44_tpl<F,4,1> CreateRotationAA44(f32 rad, const Vec3_tpl<F>& axis ) {
			Matrix44_tpl<F,4,1> m44; m44.SetRotationAA44(rad,axis); return m44;
		}*/



		/*!
		*
		* Create rotation-matrix about X axis using an angle.
		* The angle is assumed to be in radians. 
		*
		*  Example 1:
		*		Matrix m44;
		*		m44.SetRotationX44(0.5f);
		*
		*  Example 2:
		*		Matrix m44=CreateRotationX44<f32>(0.5f);
		*/
	/*	ILINE void SetRotationX44(f32 rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f)  ) {
			M00=1.0f;		M01=0.0f;					M02=0.0f;						M03=t.x;		
			M10=0.0f;		M11=cos_tpl(rad);	M12=-sin_tpl(rad);	M13=t.y;
			M20=0.0f;		M21=sin_tpl(rad);	M22=cos_tpl(rad);		M23=t.z;
			M30=0.0f;		M31=0.0f;					M32=0.0f;						M33=1.0f;
		}*/

		/*!
		* Create rotation-matrix about Y axis using an angle.
		* The angle is assumed to be in radians. 
		*
		*  Example 1:
		*		Matrix m44;
		*		m44.SetRotationY44(0.5f);
		*
		*  Example 2:
		*		Matrix m44=CreateRotationY44<f32>(0.5f);
		*/
/*		ILINE void SetRotationY44(f32 rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) ) {
			M00=cos_tpl(rad);	M01=0.0f;		M02=sin_tpl(rad);		M03=t.x;
			M10=0.0f;					M11=1.0f;		M12=0.0f;						M13=t.y;
			M20=-sin_tpl(rad);	M21=0.0f;		M22=cos_tpl(rad);	M23=t.z;
			M30=0.0f;					M31=0.0f;		M32=0.0f;						M33=1.0f;
		}*/



		/*!
		*
		* Create rotation-matrix about Z axis using an angle.
		* The angle is assumed to be in radians. 
		*
		*  Example:
		*		Matrix m44;
		*		m44.SetRotationZ44(0.5f);
		*
		*  Example 2:
		*		Matrix44 m44=CreateRotationZ44<f32>( 0.5f );
		*
		*/
/*		ILINE void SetRotationZ44(const f32 rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) ) {
			M00=cos_tpl(rad);	M01=-sin_tpl(rad);	M02=0.0f;	M03=t.x;
			M10=sin_tpl(rad);	M11=cos_tpl(rad);	  M12=0.0f;	M13=t.y;
			M20=0.0f;					M21=0.0f;					  M22=1.0f;	M23=t.z;
			M30=0.0f;					M31=0.0f;					  M32=0.0f;	M33=1.0f;
		}*/



		/*!
		*
		* Convert three Euler angle to mat33 (rotation order:XYZ)
		* The Euler angles are assumed to be in radians. 
		*
		*  Example 1:
		*		Matrix44 m44;
		*		m44.SetRotationXYZ( Ang3(0.5f, 0.2f, 0.9f) );
		*
		*  Example 2:
		*		Matrix44 m44=CreateRotationXYZ33<f32r>( Ang3(0.5f, 0.2f, 0.9f) );
		*/
		ILINE void SetRotationXYZ( const Ang3_tpl<F>& rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) )	{
			F csx[2]; sincos_tpl(rad.x, csx);
			F csy[2]; sincos_tpl(rad.y, csy);
			F csz[2]; sincos_tpl(rad.z, csz);
			f32 sycz  =(sy*cz);		f32 sysz  =(sy*sz);
			M00=cy*cz;	M01=sycz*sx-cx*sz;	M02=sycz*cx+sx*sz;	M03=t.x;
			M10=cy*sz;	M11=sysz*sx+cx*cz;	M12=sysz*cx-sx*cz;	M13=t.y;
			M20=-sy;		M21=cy*sx;					M22=cy*cx;					M23=t.z;
			M30=0.0f;		M31=0.0f;						M32=0.0f;						M33=1.0f;
		}
	/*	ILINE static Matrix44_tpl<F,SI,SJ> CreateRotationXYZ44( const Ang3_tpl<F>& rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) )	{
			Matrix44_tpl<F,SI,SJ> m; m.SetRotationXYZ44(rad,t); return m;
		}
		ILINE friend Matrix44_tpl<F,4,1> CreateRotationXYZ44( const Ang3_tpl<F>& rad, const Vec3_tpl<F>& t )	{	Matrix44_tpl<F,4,1> m; m.SetRotationXYZ44(rad,t); return m;	}
		ILINE friend Matrix44_tpl<F,4,1> CreateRotationXYZ44( const Ang3_tpl<F>& rad )	{	Matrix44_tpl<F,4,1> m; m.SetRotationXYZ44(rad); return m;	}
*/


		//create a pre-concatenarted matrix:
		//order: Scaling-Rotation-Translation
		/*ILINE void SetMatrixSRT( const Vec3_tpl<F>& s, const Ang3_tpl<F>& rad,  const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f) )	{
			f32 cx=cos_tpl(rad.x);	f32 sx=sin_tpl(rad.x);
			f32 cy=cos_tpl(rad.y);	f32 sy=sin_tpl(rad.y);
			f32 cz=cos_tpl(rad.z);	f32 sz=sin_tpl(rad.z);
			f32 sycz  =(sy*cz);			f32 sysz  =(sy*sz);
			M00=(cy*cz)*s.x;	M01=(sycz*sx-cx*sz)*s.y;	M02=(sycz*cx+sx*sz)*s.z;	M03=t.x;
			M10=(cy*sz)*s.x;	M11=(sysz*sx+cx*cz)*s.y;	M12=(sysz*cx-sx*cz)*s.z;	M13=t.y;
			M20=(-sy)*s.x;		M21=(cy*sx)*s.y;					M22=(cy*cx)*s.z;					M23=t.z;
			M30=0.0f;					M31=0.0f;									M32=0.0f;									M33=1.0f;
		}*/



		/*!
		*
		* Convert three Euler angle to mat33 (rotation order:ZYX)
		* The Euler angles are assumed to be in radians. 
		*
		*  Example 1:
		*		Matrix44 m44;
		*		m44.SetRotationZYX33( Ang3(0.5f, 0.2f, 0.9f) );
		*
		*  Example 2:
		*		Matrix44 m44=CreateRotationZYX33<f32r>( Ang3(0.5f, 0.2f, 0.9f) );
		*/
		ILINE void SetRotationZYX( const Ang3_tpl<F>& rad, const Vec3_tpl<F>& t=Vec3(0.0f,0.0f,0.0f)  )	{
			F csx[2]; sincos_tpl(rad.x, csx);
			F csy[2]; sincos_tpl(rad.y, csy);
			F csz[2]; sincos_tpl(rad.z, csz);
			F sxsy=sx*sy;
			F cxsy=cx*sy;
			M00=   cy*cz;					M01=  -cy*sz;					M02=  sy;			M03=t.x;
			M10=cz*sxsy + cx*sz;	M11=cx*cz - sxsy*sz;	M12=-cy*sx;		M13=t.z;
			M20=sx*sz - cxsy*cz;	M21=cz*sx + cxsy*sz;	M22= cx*cy;		M23=t.y;				
			M30=0.0f;							M31=0.0f;							M32=0.0f;			M33=1.0f;
		}
		ILINE static Matrix44_tpl<F,4,1> CreateRotationZYX( const Vec3_tpl<F>& rad, const Vec3_tpl<F>& t )	{
			Matrix44_tpl<F,4,1> m44; m44.SetRotationZYX(rad,t);	return m44;
		}
		ILINE static Matrix44_tpl<F,4,1> CreateRotationZYX( const Vec3_tpl<F>& rad )	{
			Matrix44_tpl<F,4,1> m44; m44.SetRotationZYX(rad);	return m44;
		}

		//NOTE: all vectors are stored in columns
		/*ILINE void SetMatFromVectors44(const Vec3& vx, const Vec3& vy, const Vec3& vz, const Vec3& pos)	{
			M00=vx.x;		M01=vy.x;		M02=vz.x;		M03 = pos.x;
			M10=vx.y;		M11=vy.y;		M11=vz.y;		M13 = pos.y;
			M20=vx.z;		M21=vy.z;		M22=vz.z;		M23 = pos.z;
			M30=0.0f;		M31=0.0f;		M32=0.0f;		M33 = 1.0f;
		}
		ILINE friend Matrix44_tpl<F,4,1> GetMatFromVectors44(const Vec3_tpl<F>& vx, const Vec3_tpl<F>& vy, const Vec3_tpl<F>& vz, const Vec3_tpl<F>& pos) {
			Matrix44_tpl<F,4,1> m; m.SetMatFromVectors44(vx,vy,vz,pos); return m;
		}*/

		//! Remove scale from matrix.
		ILINE void NoScale()	{
			Vec3 x( M00,M01,M02 );
			Vec3 y( M10,M11,M12 );
			Vec3 z;
			x = GetNormalized( x );
			z = GetNormalized( x.Cross(y) );
			y = GetNormalized( z.Cross(x) );
			M00=x.x;		M10=y.x;		M20=z.x;
			M01=x.y;		M11=y.y;		M21=z.y;
			M02=x.z;		M12=y.z;		M22=z.z;
		}

		/*!
		*
		* Create translation-matrix.
		*
		*  Example 1:
		*		Matrix44 m44;
		*		m44.SetTransMat44( Vec3(0.5f, 1.0f, 2.0f) );
		*
		*  Example 2:
		*		Matrix44 m44 = GetTransMat44( Vec3(0.5f, 1.0f, 2.0f) );
		*
		*/
/*		ILINE void SetTransMat44(  const Vec3_tpl<F>& v  ) {
			M00=1.0f;	M01=0.0f;	M02=0.0f;	M03=v.x;
			M10=0.0f;	M11=1.0f;	M12=0.0f;	M13=v.y;
			M20=0.0f;	M21=0.0f;	M22=1.0f;	M23=v.z;
			M30=0.0f;	M31=0.0f;	M32=0.0f;	M33=1.0f;
		}
		ILINE static Matrix44_tpl<F,4,1> GetTransMat44( const Vec3_tpl<F>& v  ) {
			Matrix44_tpl<F,4,1> m; m.SetTransMat44(v); return m;
		}*/



		/*!
		*
		* calculate a real inversion of a Matrix44.
		* an inverse-matrix is an UnDo-matrix for all kind of transformations 
		* 
		*  Example 1:
		*		Matrix44 im44;
		*		im44.Invert44();
		*
		*  Example 2:
		*   Matrix44 im44=Invert44(m44);
		*/
		void Invert44( void ) {
			F	tmp[12];
			Matrix44_tpl<F,SI,SJ>	m=*this;

			/* calculate pairs for first 8 elements (cofactors) */
			tmp[0] = m.M22 * m.M33;
			tmp[1] = m.M32 * m.M23;
			tmp[2] = m.M12 * m.M33;
			tmp[3] = m.M32 * m.M13;
			tmp[4] = m.M12 * m.M23;
			tmp[5] = m.M22 * m.M13;
			tmp[6] = m.M02 * m.M33;
			tmp[7] = m.M32 * m.M03;
			tmp[8] = m.M02 * m.M23;
			tmp[9] = m.M22 * m.M03;
			tmp[10]= m.M02 * m.M13;
			tmp[11]= m.M12 * m.M03;

			/* calculate first 8 elements (cofactors) */
			M00 = tmp[0]*m.M11 + tmp[3]*m.M21 + tmp[ 4]*m.M31;
			M00-= tmp[1]*m.M11 + tmp[2]*m.M21 + tmp[ 5]*m.M31;
			M01 = tmp[1]*m.M01 + tmp[6]*m.M21 + tmp[ 9]*m.M31;
			M01-= tmp[0]*m.M01 + tmp[7]*m.M21 + tmp[ 8]*m.M31;
			M02 = tmp[2]*m.M01 + tmp[7]*m.M11 + tmp[10]*m.M31;
			M02-= tmp[3]*m.M01 + tmp[6]*m.M11 + tmp[11]*m.M31;
			M03 = tmp[5]*m.M01 + tmp[8]*m.M11 + tmp[11]*m.M21;
			M03-= tmp[4]*m.M01 + tmp[9]*m.M11 + tmp[10]*m.M21;
			M10 = tmp[1]*m.M10 + tmp[2]*m.M20 + tmp[ 5]*m.M30;
			M10-= tmp[0]*m.M10 + tmp[3]*m.M20 + tmp[ 4]*m.M30;
			M11 = tmp[0]*m.M00 + tmp[7]*m.M20 + tmp[ 8]*m.M30;
			M11-= tmp[1]*m.M00 + tmp[6]*m.M20 + tmp[ 9]*m.M30;
			M12 = tmp[3]*m.M00 + tmp[6]*m.M10 + tmp[11]*m.M30;
			M12-= tmp[2]*m.M00 + tmp[7]*m.M10 + tmp[10]*m.M30;
			M13 = tmp[4]*m.M00 + tmp[9]*m.M10 + tmp[10]*m.M20;
			M13-= tmp[5]*m.M00 + tmp[8]*m.M10 + tmp[11]*m.M20;

			/* calculate pairs for second 8 elements (cofactors) */
			tmp[ 0] = m.M20*m.M31;
			tmp[ 1] = m.M30*m.M21;
			tmp[ 2] = m.M10*m.M31;
			tmp[ 3] = m.M30*m.M11;
			tmp[ 4] = m.M10*m.M21;
			tmp[ 5] = m.M20*m.M11;
			tmp[ 6] = m.M00*m.M31;
			tmp[ 7] = m.M30*m.M01;
			tmp[ 8] = m.M00*m.M21;
			tmp[ 9] = m.M20*m.M01;
			tmp[10] = m.M00*m.M11;
			tmp[11] = m.M10*m.M01;

			/* calculate second 8 elements (cofactors) */
			M20 = tmp[ 0]*m.M13 + tmp[ 3]*m.M23 + tmp[ 4]*m.M33;
			M20-= tmp[ 1]*m.M13 + tmp[ 2]*m.M23 + tmp[ 5]*m.M33;
			M21 = tmp[ 1]*m.M03 + tmp[ 6]*m.M23 + tmp[ 9]*m.M33;
			M21-= tmp[ 0]*m.M03 + tmp[ 7]*m.M23 + tmp[ 8]*m.M33;
			M22 = tmp[ 2]*m.M03 + tmp[ 7]*m.M13 + tmp[10]*m.M33;
			M22-= tmp[ 3]*m.M03 + tmp[ 6]*m.M13 + tmp[11]*m.M33;
			M23 = tmp[ 5]*m.M03 + tmp[ 8]*m.M13 + tmp[11]*m.M23;
			M23-= tmp[ 4]*m.M03 + tmp[ 9]*m.M13 + tmp[10]*m.M23;
			M30 = tmp[ 2]*m.M22 + tmp[ 5]*m.M32 + tmp[ 1]*m.M12;
			M30-= tmp[ 4]*m.M32 + tmp[ 0]*m.M12 + tmp[ 3]*m.M22;
			M31 = tmp[ 8]*m.M32 + tmp[ 0]*m.M02 + tmp[ 7]*m.M22;
			M31-= tmp[ 6]*m.M22 + tmp[ 9]*m.M32 + tmp[ 1]*m.M02;
			M32 = tmp[ 6]*m.M12 + tmp[11]*m.M32 + tmp[ 3]*m.M02;
			M32-= tmp[10]*m.M32 + tmp[ 2]*m.M02 + tmp[ 7]*m.M12;
			M33 = tmp[10]*m.M22 + tmp[ 4]*m.M02 + tmp[ 9]*m.M12;
			M33-= tmp[ 8]*m.M12 + tmp[11]*m.M22 + tmp[ 5]*m.M02;

			/* calculate determinant */
			F det=(m.M00*M00+m.M10*M01+m.M20*M02+m.M30*M03);
			if (fabs_tpl(det)<0.0001f) assert(0);	

			//devide the cofactor-matrix by the determinat
			F idet=(F)1.0/det;
			M00*=idet; M01*=idet; M02*=idet; M03*=idet;
			M10*=idet; M11*=idet; M12*=idet; M13*=idet;
			M20*=idet; M21*=idet; M22*=idet; M23*=idet;
			M30*=idet; M31*=idet; M32*=idet; M33*=idet;
		}
		ILINE static Matrix44_tpl<F,SI,SJ> GetInverted44( const Matrix44_tpl<F,SI,SJ> &m ) {	Matrix44_tpl<F,SI,SJ> dst=m; dst.Invert44(); return dst;	}
		ILINE friend Matrix44_tpl<F,4,1> GetInverted44( const Matrix44_tpl<F,4,1> &m ) { return Matrix44::GetInverted44(m); }





//-------------------------------------------------------------------------------------
//-------------                obsolete functions       -------------------------------
//-------    just need them to keep functionality with old game-logic       -----------
//-------------------------------------------------------------------------------------
//----- in many parts of this engine we are still storing the 4x4-matrices in the -----
//----- in the row-format (all 4 vectors are in rows and not in the collums). ---------
//-------------------------------------------------------------------------------------
//----- And that why we still need these obsolete functions to access e.i the     -----
//----	translation vector!                                                       -----
//-------------------------------------------------------------------------------------

		//NOTE: old version-> all vectors are stored in rows
		ILINE void BuildFromVectors(const Vec3& vx, const Vec3& vy, const Vec3& vz, const Vec3& pos)
		{
			M00=vx[0];	M01=vx[1];	M02=vx[2];	M03 = 0;
			M10=vy[0];	M11=vy[1];	M12=vy[2];	M13 = 0;
			M20=vz[0];	M21=vz[1];	M22=vz[2];	M23 = 0;
			M30=pos[0];	M31=pos[1];	M32=pos[2];	M33 = 1;
		}

		//apply scaling to matrix.
		//NOTE: this function expects the matrix to be in transposed format in memory => othonormal base in rows.
		ILINE void ScaleMatRow( const Vec3_tpl<F>& s)	
		{
			M00*=s.x;		M01*=s.x;		M02*=s.x;
			M10*=s.y;		M11*=s.y;		M12*=s.y;
			M20*=s.z;		M21*=s.z;		M22*=s.z;
		}

		//! transform a point by a "transposed" matrix
		//this function expects the matrix to be in transposed format in memory => othonormal base in rows.
		ILINE Vec3 TransformVectorOLD(const Vec3 &b) const  
		{
			Vec3 v;
			v.x = M00*b.x + M10*b.y + M20*b.z;
			v.y = M01*b.x + M11*b.y + M21*b.z;
			v.z = M02*b.x + M12*b.y + M22*b.z;
			return v;
		}
		//! transform a point by a "transposed" matrix
		//this function expects the matrix to be in transposed format in memory => othonormal base in rows.
		ILINE Vec3 TransformPointOLD(const Vec3 &b) const  
		{
			Vec3 v;
			v.x = M00*b.x + M10*b.y + M20* b.z + M30;
			v.y = M01*b.x + M11*b.y + M21* b.z + M31;
			v.z = M02*b.x + M12*b.y + M22* b.z + M32;
			return v;
		}

		//if we use thse function then we assume the translation is in the last row
		ILINE Vec3 GetTranslationOLD() const	        {	return Vec3( M30, M31, M32 );	}
		ILINE void SetTranslationOLD( const Vec3& t )	{	M30 =t.x;	M31 =t.y;	M32 =t.z;	}
		ILINE void AddTranslationOLD( const Vec3& t )	{	M30+=t.x;	M31+=t.y;	M32+=t.z;	}
		ILINE void ScaleTranslationOLD (f32 s)			{	M30*=s;		M31*=s;		M32*=s;		}

		/*!
		*
		* Create translation-matrix.
		*
		*  Example 1:
		*		Matrix m34;
		*		m34.translationXYZ34( Vec3(0.5f, 1.0f, 2.0f) );
		*
		*  Example 2:
		*		Matrix34 m34 = translationXYZ34( Vec3(0.5f, 1.0f, 2.0f) );
		*
		*/
	ILINE void SetTranslationMat(  const Vec3_tpl<F>& v  ) {
		M00=1.0f;	M01=0.0f;	M02=0.0f;	M03=0.0f;
		M10=0.0f;	M11=1.0f;	M12=0.0f;	M13=0.0f;
		M20=0.0f;	M21=0.0f;	M22=1.0f;	M23=0.0f;
		M30=v.x;	M31=v.y;	M32=v.z;	M33=1.0f;
	}
	ILINE friend Matrix44_tpl<F,4,1> GetTranslationMat( const Vec3_tpl<F>& v  ) {
		Matrix44_tpl<F,4,1> m; m.SetTranslationMat(v); return m;
	}

};


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//-------------       implementation of Matrix44      ------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------



/*!
*
*  Implements the multiplication operator: Matrix44=Matrix44*Matrix33
*
*  Matrix44 and Matrix33 are specified in collumn order.         
*  AxB = rotation B followed by rotation A.  
*  This operation takes 48 mults and 24 adds. 
*
*  Example:
*   Matrix33 m34=CreateRotationX33(1.94192f);;
*   Matrix44 m44=CreateRotationZ33(3.14192f);
*	  Matrix44 result=m44*m33;
*
*/
template<class F1,int SI1,int SJ1,  class F2,int SI2,int SJ2> 
ILINE Matrix44_tpl<F1,SI1,SJ1> operator * (const Matrix44_tpl<F1,SI1,SJ1>& l, const Matrix33_tpl<F2,SI2,SJ2>& r) {
	Matrix44_tpl<F1,SI1,SJ1> m;
	m_00 =l_00*r_00 + l_01*r_10 + l_02*r_20;
	m_10 =l_10*r_00 + l_11*r_10 + l_12*r_20;
	m_20 =l_20*r_00 + l_21*r_10 + l_22*r_20;
	m_30 =l_30*r_00 + l_31*r_10 + l_32*r_20;
	m_01 =l_00*r_01 + l_01*r_11 + l_02*r_21;
	m_11 =l_10*r_01 + l_11*r_11 + l_12*r_21;
	m_21 =l_20*r_01 + l_21*r_11 + l_22*r_21;
	m_31 =l_30*r_01 + l_31*r_11 + l_32*r_21;
	m_02 =l_00*r_02 + l_01*r_12 + l_02*r_22;
	m_12 =l_10*r_02 + l_11*r_12 + l_12*r_22;
	m_22 =l_20*r_02 + l_21*r_12 + l_22*r_22;
	m_32 =l_30*r_02 + l_31*r_12 + l_32*r_22;
	m_03 =l_03;
	m_13 =l_13;
	m_23 =l_23;
	m_33 =l_33;
	return m;
}


/*!
*
*  Implements the multiplication operator: Matrix44=Matrix44*Matrix34
*
*  Matrix44 and Matrix34 are specified in collumn order.         
*  AxB = rotation B followed by rotation A.  
*  This operation takes 48 mults and 36 adds. 
*
*  Example:
*   Matrix34 m34=CreateRotationX33(1.94192f);;
*   Matrix44 m44=CreateRotationZ33(3.14192f);
*	  Matrix44 result=m44*m34;
*
*/
template<class F,int SI,int SJ> 
ILINE Matrix44_tpl<F,SI,SJ> operator * (const Matrix44_tpl<F,SI,SJ>& l, const Matrix34_tpl<F>& r) {
	Matrix44_tpl<F,SI,SJ> m;
	m.M00 = l.M00*r.m00 + l.M01*r.m10 + l.M02*r.m20;
	m.M10 = l.M10*r.m00 + l.M11*r.m10 + l.M12*r.m20;
	m.M20 = l.M20*r.m00 + l.M21*r.m10 + l.M22*r.m20;
	m.M30 = l.M30*r.m00 + l.M31*r.m10 + l.M32*r.m20;
	m.M01 = l.M00*r.m01 + l.M01*r.m11 + l.M02*r.m21;
	m.M11 = l.M10*r.m01 + l.M11*r.m11 + l.M12*r.m21;
	m.M21 = l.M20*r.m01 + l.M21*r.m11 + l.M22*r.m21;
	m.M31 = l.M30*r.m01 + l.M31*r.m11 + l.M32*r.m21;
	m.M02 = l.M00*r.m02 + l.M01*r.m12 + l.M02*r.m22;
	m.M12 = l.M10*r.m02 + l.M11*r.m12 + l.M12*r.m22;
	m.M22 = l.M20*r.m02 + l.M21*r.m12 + l.M22*r.m22;
	m.M32 = l.M30*r.m02 + l.M31*r.m12 + l.M32*r.m22;
	m.M03 = l.M00*r.m03 + l.M01*r.m13 + l.M02*r.m23 + l.M03;
	m.M13 = l.M10*r.m03 + l.M11*r.m13 + l.M12*r.m23 + l.M13;
	m.M23 = l.M20*r.m03 + l.M21*r.m13 + l.M22*r.m23 + l.M23;
	m.M33 = l.M30*r.m03 + l.M31*r.m13 + l.M32*r.m23 + l.M33;
	return m;
}	




/*!
*
*  Implements the multiplication operator: Matrix44=Matrix44*Matrix44
*
*  Matrix44 and Matrix34 are specified in collumn order.         
*	 AxB = rotation B followed by rotation A.  
*  This operation takes 48 mults and 36 adds.  
*
*  Example:
*   Matrix44 m44=CreateRotationX33(1.94192f);;
*   Matrix44 m44=CreateRotationZ33(3.14192f);
*	  Matrix44 result=m44*m44;
*
*/
template<class F1,int SI1,int SJ1, class F2,int SI2,int SJ2 > 
ILINE Matrix44_tpl<F1,SI1,SJ1> operator * (const Matrix44_tpl<F1,SI1,SJ1>& l, const Matrix44_tpl<F2,SI2,SJ2>& r)
{
	Matrix44_tpl<F1,SI1,SJ1> m;
	m_00 = l_00*r_00 + l_01*r_10 + l_02*r_20 + l_03*r_30;
	m_10 = l_10*r_00 + l_11*r_10 + l_12*r_20 + l_13*r_30;
	m_20 = l_20*r_00 + l_21*r_10 + l_22*r_20 + l_23*r_30;
	m_30 = l_30*r_00 + l_31*r_10 + l_32*r_20 + l_33*r_30;

	m_01 = l_00*r_01 + l_01*r_11 + l_02*r_21 + l_03*r_31;
	m_11 = l_10*r_01 + l_11*r_11 + l_12*r_21 + l_13*r_31;
	m_21 = l_20*r_01 + l_21*r_11 + l_22*r_21 + l_23*r_31;
	m_31 = l_30*r_01 + l_31*r_11 + l_32*r_21 + l_33*r_31;

	m_02 = l_00*r_02 + l_01*r_12 + l_02*r_22 + l_03*r_32;
	m_12 = l_10*r_02 + l_11*r_12 + l_12*r_22 + l_13*r_32;
	m_22 = l_20*r_02 + l_21*r_12 + l_22*r_22 + l_23*r_32;
	m_32 = l_30*r_02 + l_31*r_12 + l_32*r_22 + l_33*r_32;

	m_03 = l_00*r_03 + l_01*r_13 + l_02*r_23 + l_03*r_33;
	m_13 = l_10*r_03 + l_11*r_13 + l_12*r_23 + l_13*r_33;
	m_23 = l_20*r_03 + l_21*r_13 + l_22*r_23 + l_23*r_33;
	m_33 = l_30*r_03 + l_31*r_13 + l_32*r_23 + l_33*r_33;

	return m;
}	



ILINE Vec3 transform_vector(const Matrix44 &mtx, const Vec3 vec) {
	Vec3 res = mtx*vec;
	res.x += mtx(0,3);
	res.y += mtx(1,3);
	res.z += mtx(2,3);
	return res;
}

ILINE Vec3 UntransformVector (const Matrix44& m, const Vec3& v) {
	return Vec3 (
		m(0,0)*v.x+m(0,1)*v.y+m(0,2)*v.z, 
		m(1,0)*v.x+m(1,1)*v.y+m(1,2)*v.z, 
		m(2,0)*v.x+m(2,1)*v.y+m(2,2)*v.z
		);
}









//-------------------------------------------------------------------------------------
//--- some typedefs for Antons physics code                    ------------------------
//--- we already have typdefs for this classes at the beginning of this file, ---------
//--- thus we will rename them sooner or later                   ----------------------
//-------------------------------------------------------------------------------------

		typedef Matrix33diag_tpl<f32> matrix3x3diagf;
		typedef Matrix33diag_tpl<real> matrix3x3diag;

		typedef Matrix33_tpl<f32,3,1> matrix3x3f;
		typedef Matrix33_tpl<f32,1,3> matrix3x3Tf;
		typedef Matrix33_tpl<real, 3,1> matrix3x3;
		typedef Matrix33_tpl<real, 1,3> matrix3x3T;

		typedef Matrix33_tpl<f32,3,1> matrix3x3RMf;
		typedef Matrix33_tpl<f32,1,3> matrix3x3CMf;
		typedef Matrix33_tpl<real, 3,1> matrix3x3RM;
		typedef Matrix33_tpl<real, 1,3> matrix3x3CM;

		typedef Matrix33_tpl<f32,3,1> matrix3x3bf;
		typedef Matrix33_tpl<f32,1,3> matrix3x3Tbf;
		typedef Matrix33_tpl<real ,3,1> matrix3x3b;
		typedef Matrix33_tpl<real ,1,3> matrix3x3Tb;

		typedef Matrix33_tpl<f32,4,1> matrix3x3in4x4f;
		typedef Matrix33_tpl<f32,1,4> matrix3x3in4x4Tf;
		typedef Matrix33_tpl<real, 4,1> matrix3x3in4x4;
		typedef Matrix33_tpl<real, 1,4> matrix3x3in4x4T;

		typedef Matrix33_tpl<f32,6,1> matrix3x3in6x6f;




#endif //MATRIX_H

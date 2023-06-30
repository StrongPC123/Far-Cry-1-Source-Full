//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//
//	File:Cry_Math.h
//	Description: Common math class
//
//	History:
//	-Feb 27,2003: Created by Ivo Herzeg
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYMATH_H
#define CRYMATH_H

#if _MSC_VER > 1000
# pragma once
#endif

//========================================================================================

#include <math.h>

#include "platform.h"

#if !defined(LINUX)
#include <assert.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// Forward declarations                                                      //
///////////////////////////////////////////////////////////////////////////////
template <class F> struct Vec3_tpl;
template <class F> struct Ang3_tpl;
template <class F> struct AngleAxis_tpl;
template <class F> struct Quaternion_tpl;

template <class F> struct Matrix33diag_tpl;
template <class F,int SI,int SJ> struct Matrix33_tpl;
template <class F> struct Matrix34_tpl;
template <class F,int SI,int SJ> struct Matrix44_tpl;






///////////////////////////////////////////////////////////////////////////////
// Definitions                                                               //
///////////////////////////////////////////////////////////////////////////////
const float gf_PI       =  3.14159265358979323846264338327950288419716939937510f; // pi
const float gf_PI_MUL_2 =  3.14159265358979323846264338327950288419716939937510f*2; // 2*pi
const float gf_PI_DIV_2 =  3.14159265358979323846264338327950288419716939937510f*0.5f; // pi/2
const float gf_DEGTORAD =  0.01745329251994329547f; // Degrees to Radians
const float gf_RADTODEG = 57.29577951308232286465f; // Radians to Degrees

// the check for compatibility with Max SDK: Max gfx.h header defines its own pi
#if !defined(_GFX_H_)
const real pi			= (real)3.1415926535897932384626433832795;
#endif
const real sqrt2	= (real)1.4142135623730950488016887242097;
const real sqrt3	= (real)1.7320508075688772935274463415059;

#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))

//-----------------------------------------------------------------------







#if defined(LINUX)
#undef assert
#define assert(exp) (void)( (exp) || (printf("Assert: ' %s ' has failed\n", #exp), 0) )

#endif



//-------------------------------------------
//-- the portability functions for AMD64
//-------------------------------------------
#if defined(WIN64) &&  defined(_CPU_AMD64) && !defined(LINUX)
#define ILINE __forceinline

extern "C" void fastsincosf(float x, float * sincosfx);
extern "C" float fastsinf(float x);
extern "C" float fastcosf(float x);

ILINE void cry_sincosf (float angle, float* pCosSin) {	fastsincosf(angle,pCosSin);	}
ILINE void cry_sincos  (double angle, double* pCosSin) {	pCosSin[0] = cos(angle);	pCosSin[1] = sin(angle); }
ILINE float cry_sinf(float x) {return fastsinf(x); }
ILINE float cry_cosf(float x) {return fastcosf(x); }

ILINE float cry_fmod(float x, float y) {return (float)fmod((double)x,(double)y);}

ILINE float cry_asinf(float x) {return (float)asin((double)x);}
ILINE float cry_acosf(float x) {return (float)acos((double)x);}
ILINE float cry_atanf(float x) {return (float)atan((double)x);}
ILINE float cry_atan2f(float x, float y) {return (float)atan2((double)x,(double)y);}

ILINE float cry_tanhf(float x) {	double expz = exp(double(x)), exp_z = exp(-double(x));	return (float)((expz-exp_z)/(expz+exp_z)); }
ILINE float cry_tanf(float x) {return (float)tan((double)x);}

ILINE float cry_sqrtf(float x) {return (float)sqrt((double)x);}
ILINE float cry_fabsf(float x) {return (float)fabs((double)x);}
ILINE float cry_expf(float x) {return (float)exp((double)x);}
ILINE float cry_logf(float x) {return (float)log((double)x);}
ILINE float cry_powf(float x, float y) {return (float) pow((double)x,(double)y);}

ILINE float cry_ceilf(float x) {return (float)ceil((double)x);}
ILINE float cry_floorf(float x) {return (float)floor((double)x);}

ILINE double cry_sinh(double z) {return (exp (z) - exp (-z)) * 0.5;}
ILINE double cry_cosh(double z) {return (exp (z) + exp (-z)) * 0.5;}
#endif



//-------------------------------------------
//-- the portability functions for CPU_X86
//-------------------------------------------
#if defined(_CPU_X86) && defined(_MSC_VER) && !defined(LINUX)
#define ILINE __forceinline

// calculates the cosine and sine of the given angle in radians 
ILINE void cry_sincosf (float angle, float* pCosSin) {
	__asm {
			FLD         DWORD PTR       angle
			FSINCOS
			MOV         EAX,pCosSin
			FSTP        DWORD PTR [EAX]		//put cosine into pCosSin[0]
			FSTP        DWORD PTR [EAX+4]	//put sine into cossin[1]
	}
}
// calculates the cosine and sine of the given angle in radians 
ILINE void cry_sincos (double angle, double* pCosSin) {
	__asm {
		FLD         QWORD PTR       angle
			FSINCOS
			MOV         EAX,pCosSin
			FSTP        QWORD PTR [EAX]			//put cosine into pCosSin[0]
			FSTP        QWORD PTR [EAX+8]		//put sine into cossin[1]
	}
}
ILINE float cry_sinf(float x) {return sinf(x);}
ILINE float cry_cosf(float x) {return cosf(x);}
ILINE float cry_fmod(float x, float y) {return (float)fmodf(x,y);}
ILINE float cry_ceilf(float x) {return ceilf(x);}
ILINE float cry_asinf(float x) {return asinf(x);}
ILINE float cry_acosf(float x) {return acosf(x);}
ILINE float cry_atanf(float x) {return atanf(x);}
ILINE float cry_atan2f(float x, float y) {return atan2f(x,y);}
ILINE float cry_sqrtf(float x) {return sqrtf(x);}
ILINE float cry_tanhf(float x) {return tanhf(x);}
ILINE float cry_fabsf(float x) {return fabsf(x);}
ILINE float cry_expf(float x) {return expf(x);}
ILINE float cry_logf(float x) {return logf(x);}
ILINE float cry_floorf(float x) {return floorf(x);}
ILINE float cry_tanf(float x) {return tanf(x);}
ILINE float cry_powf(float x, float y) {return powf(x,y);}
#endif


//-------------------------------------------
//-- the portability functions for LINUX
//-------------------------------------------
#if defined(LINUX)
#define ILINE inline

ILINE void cry_sincosf (float angle, float* pCosSin) { 	pCosSin[0] = (float)cos(angle);	pCosSin[1] = (float)sin(angle); 	}
ILINE void cry_sincos (double angle, double* pCosSin) {	pCosSin[0] = cos(angle);	pCosSin[1] = sin(angle); 	}
ILINE float cry_sinf(float x) {return sinf(x);}
ILINE float cry_cosf(float x) {return cosf(x);}
ILINE float cry_fmod(float x, float y) {return (float)fmodf(x,y);}
ILINE float cry_ceilf(float x) {return ceilf(x);}
ILINE float cry_asinf(float x) {return asinf(x);}
ILINE float cry_acosf(float x) {return acosf(x);}
ILINE float cry_atanf(float x) {return atanf(x);}
ILINE float cry_atan2f(float x, float y) {return atan2f(x,y);}
ILINE float cry_sqrtf(float x) {return sqrtf(x);}
ILINE float cry_tanhf(float x) {return tanhf(x);}
ILINE float cry_fabsf(float x) {return fabsf(x);}
ILINE float cry_expf(float x) {return expf(x);}
ILINE float cry_logf(float x) {return logf(x);}
ILINE float cry_floorf(float x) {return floorf(x);}
ILINE float cry_tanf(float x) {return tanf(x);}
ILINE float cry_powf(float x, float y) {return powf(x,y);}
#endif


//-----------------------------------------------------------------------

ILINE void sincos_tpl(double angle, double* pCosSin) { cry_sincos(angle,pCosSin); }
ILINE void sincos_tpl(float angle, float* pCosSin) { cry_sincosf(angle,pCosSin); }

ILINE double cos_tpl(double op) { return cos(op); }
ILINE float  cos_tpl(float op) { return cry_cosf(op); }

ILINE double sin_tpl(double op) { return sin(op); }
ILINE float  sin_tpl(float op) { return cry_sinf(op); }

ILINE double acos_tpl(double op) { return acos(op); }
ILINE float  acos_tpl(float op) { return cry_acosf(op); }

ILINE double asin_tpl(double op) { return asin(op); }
ILINE float  asin_tpl(float op) { return cry_asinf(op); }

ILINE double atan_tpl(double op) { return atan(op); }
ILINE float  atan_tpl(float op) { return cry_atanf(op); }

ILINE double atan2_tpl(double op1,double op2) { return atan2(op1,op2); }
ILINE float  atan2_tpl(float op1,float op2) { return cry_atan2f(op1,op2); }

ILINE double exp_tpl(double op) { return exp(op); }
ILINE float  exp_tpl(float op) { return cry_expf(op); }

ILINE double log_tpl(double op) { return log(op); }
ILINE float  log_tpl(float op) { return cry_logf(op); }

ILINE double sqrt_tpl(double op) { return sqrt(op); }
ILINE float  sqrt_tpl(float op) { return cry_sqrtf(op); }

ILINE double fabs_tpl(double op) { return fabs(op); }
ILINE float  fabs_tpl(float op) { return cry_fabsf(op); }
ILINE int    fabs_tpl(int op) { int mask=op>>31; return op+mask^mask; }

ILINE int    floor_tpl(int op) {return op;}
ILINE float  floor_tpl(float op) {return cry_floorf(op);}
ILINE double floor_tpl(double op) {return floor(op);}

ILINE int    ceil_tpl(int op) {return op;}
ILINE float  ceil_tpl(float op) {return cry_ceilf(op);}
ILINE double ceil_tpl(double op) {return ceil(op);}

ILINE float  tan_tpl(float op) {return cry_tanf(op);}
ILINE double tan_tpl(double op) {return tan(op);}



static int inc_mod3[]={1,2,0}, dec_mod3[]={2,0,1};
#ifdef PHYSICS_EXPORTS
#define incm3(i) inc_mod3[i]
#define decm3(i) dec_mod3[i]
#else
inline int incm3(int i) { return i+1 & (i-2)>>31; }
inline int decm3(int i) { return i-1 + ((i-1)>>31&3); }
#endif

template <class T> T clamp_tpl( T X, T Min, T Max ) {	return X<Min ? Min : X<Max ? X : Max; }

template<class F> inline F square(F fOp) { return(fOp*fOp); }

//this can easily be confused with square-root?
template<class F> inline F sqr(const F &op) { return op*op; }

template<class F> inline F cube(const F &op) { return op*op*op; }
template<class F> inline F sqr_signed(const F &op) { return op*fabs_tpl(op); }

#define cx csx[0]
#define sx csx[1]
#define cy csy[0]
#define sy csy[1]
#define cz csz[0]
#define sz csz[1]

//#include "Cry_Vector2.h"
//#include "Cry_Vector3.h"
#include "Cry_Matrix.h"
//#include "Cry_Quat.h"

#undef cx
#undef sx
#undef cy
#undef sy
#undef cz
#undef sz



#if (defined(WIN32) || defined (_XBOX))
#include "Cry_XOptimise.h"
#endif

inline float sqr(vectorf op) { return op*op; }
inline real sqr(vectorr op) { return op*op; }

inline int sgnnz(double x) {
	union { float f; int i; } u;
	u.f=(float)x; return ((u.i>>31)<<1)+1;
}

inline int sgnnz(float x) {
	union { float f; int i; } u;
	u.f=x; return ((u.i>>31)<<1)+1;
}
inline int sgnnz(int x) {
	return ((x>>31)<<1)+1;
}

inline int sgn(double x) {
	union { float f; int i; } u;
	u.f=(float)x; return (u.i>>31)+((u.i-1)>>31)+1;
}

inline int sgn(float x) {
	union { float f; int i; } u;
	u.f=x; return (u.i>>31)+((u.i-1)>>31)+1;
}
inline int sgn(int x) {
	return (x>>31)+((x-1)>>31)+1;
}

inline int isneg(double x) {
	union { float f; unsigned int i; } u;
	u.f=(float)x; return (int)(u.i>>31);
}

inline int isneg(float x) {
	union { float f; unsigned int i; } u;
	u.f=x; return (int)(u.i>>31);
}

inline int isneg(int x) {
	return (int)((unsigned int)x>>31);
}

inline int isnonneg(double x) {
	union { float f; unsigned int i; } u;
	u.f=(float)x; return (int)(u.i>>31^1);
}

inline int isnonneg(float x) {
	union { float f; unsigned int i; } u;
	u.f=x; return (int)(u.i>>31^1);
}
inline int isnonneg(int x) {
	return (int)((unsigned int)x>>31^1);
}

inline int iszero(double x) {
	union { float f; int i; } u;
	u.f=(float)x;
	u.i&=0x7FFFFFFF;
	return -((u.i>>31)^(u.i-1)>>31);
}

inline int iszero(float x) {
	union { float f; int i; } u;
	u.f=x; u.i&=0x7FFFFFFF; return -(u.i>>31^(u.i-1)>>31);
}

inline int iszero(int x) {
	return -(x>>31^(x-1)>>31);
}

#if defined(WIN64) || defined(LINUX64)
// AMD64 port: TODO: optimize
inline int64 iszero(__int64 x) 
{
	return -(x>>63^(x-1)>>63);
}
#endif

#if defined(LINUX64)
inline int64 iszero(intptr_t x) 
{
	return (sizeof(x) == 8)?iszero((__int64)x) : iszero((int)x);
}
#endif

template<class F> int inrange(F x, F end1, F end2) {
	return isneg(fabs_tpl(end1+end2-x*(F)2) - fabs_tpl(end1-end2));
}

template<class F> F cond_select(int bFirst, F op1,F op2) {
	F arg[2] = { op1,op2 };
	return arg[bFirst^1];
}

template<class F> int idxmax3(F *pdata) {
	int imax = isneg(pdata[0]-pdata[1]);
	imax |= isneg(pdata[imax]-pdata[2])<<1;
	return imax & (2|(imax>>1^1));
}

template<class F> int idxmin3(F *pdata) {
	int imin = isneg(pdata[1]-pdata[0]);
	imin |= isneg(pdata[2]-pdata[imin])<<1;
	return imin & (2|(imin>>1^1));
}






inline int getexp(float x) { return (int)(*(unsigned int*)&x>>23&0x0FF)-127; }
inline int getexp(double x) { return (int)(*((unsigned int*)&x+1)>>20&0x7FF)-1023; }

inline float &setexp(float &x,int iexp) { (*(unsigned int*)&x &= ~(0x0FF<<23)) |= (iexp+127)<<23; return x; }
inline double &setexp(double &x,int iexp) { (*((unsigned int*)&x+1) &= ~(0x7FF<<20)) |= (iexp+1023)<<20; return x; }





class unused_marker {
public:
	unused_marker() {}
	unused_marker& operator,(float &x) { *(int*)&x = 0xFFBFFFFF; return *this; }
	unused_marker& operator,(double &x) { *((int*)&x+1) = 0xFFF7FFFF; return *this; }
	unused_marker& operator,(int &x) { x=1<<31; return *this; }
	unused_marker& operator,(unsigned int &x) { x=1u<<31; return *this; }
	template<class ref> unused_marker& operator,(ref *&x) { x=(ref*)-1; return *this; }
	template<class F> unused_marker& operator,(Vec3_tpl<F> &x) { return *this,x.x; }
	template<class F> unused_marker& operator,(Quaternion_tpl<F> &x) { return *this,x.w; }
};
inline bool is_unused(const float &x) { return (*(int*)&x & 0xFFA00000) == 0xFFA00000; }
inline bool is_unused(int x) { return x==1<<31; }
inline bool is_unused(unsigned int x) { return x==1u<<31; }
template<class ref> bool is_unused(ref *x) { return x==(ref*)-1; }
template<class F> bool is_unused(const Vec3_tpl<F> &x) { return is_unused(x.x); }
template<class F> bool is_unused(const Quaternion_tpl<F> &x) { return is_unused(x.w); }
inline bool is_unused(const double &x) { return (*((int*)&x+1) & 0xFFF40000) == 0xFFF40000; }
#define MARK_UNUSED unused_marker(),


template<class dtype> class strided_pointer {
public:
	strided_pointer() { data=0; iStride=sizeof(dtype); }
	strided_pointer(dtype *pdata,int stride=sizeof(dtype)) { data=pdata; iStride=stride; }
	strided_pointer(const strided_pointer &src) { data=src.data; iStride=src.iStride; }
	template<class dtype1> strided_pointer(const strided_pointer<dtype1> &src) { data=src.data; iStride=src.iStride; }

	strided_pointer& operator=(dtype *pdata) { data=pdata; return *this; }
	strided_pointer& operator=(const strided_pointer<dtype> &src) { data=src.data; iStride=src.iStride; return *this; }
	template<class dtype1> strided_pointer& operator=(const strided_pointer<dtype1> &src) { data=src.data; iStride=src.iStride; return *this; }

	dtype& operator[](int idx) { return *(dtype*)((char*)data+idx*iStride); }
	const dtype& operator[](int idx) const { return *(const dtype*)((const char*)data+idx*iStride); }
	strided_pointer<dtype> operator+(int idx) const { return strided_pointer<dtype>((dtype*)((char*)data+idx*iStride),iStride); }
	strided_pointer<dtype> operator-(int idx) const { return strided_pointer<dtype>((dtype*)((char*)data-idx*iStride),iStride); }
	dtype& operator*() const { return *data; }
	operator void*() const { return (void*)data; }

	dtype *data;
	int iStride;
};

template<class F> F _condmax(F, int masknot) {
	return 1<<(sizeof(int)*8-2) & ~masknot;
}
inline float _condmax(float, int masknot) {
	return 1E30f*(masknot+1);
}

template<class F> int unite_lists(F *pSrc0,INT_PTR nSrc0, F *pSrc1,INT_PTR nSrc1, F *pDst,int szdst)	//AMD Port
{
	int i0,i1,n;
	INT_PTR inrange0=-nSrc0>>((sizeof(size_t)*8)-1),inrange1=-nSrc1>>((sizeof(size_t)*8)-1);			//AMD Port
	F a0,a1,ares,dummy=0;
	INT_PTR pDummy( (INT_PTR) &dummy );
	pSrc0 = (F*)((INT_PTR)pSrc0&inrange0 | pDummy&~inrange0); // make pSrc point to valid data even if nSrc is zero	//AMD Port
	pSrc1 = (F*)((INT_PTR)pSrc1&inrange1 | pDummy&~inrange1);									//AMD Port
	for(n=i0=i1=0; (inrange0 | inrange1) & n-szdst>>31; inrange0=(i0+=isneg(a0-ares-1))-nSrc0>>31, inrange1=(i1+=isneg(a1-ares-1))-nSrc1>>31)
	{
		a0 = pSrc0[i0&inrange0] + _condmax(pSrc0[0],inrange0); //(1<<(sizeof(index)*8-2)&~inrange0);
		a1 = pSrc1[i1&inrange1] + _condmax(pSrc1[0],inrange1); //(1<<(sizeof(index)*8-2)&~inrange1);
		pDst[n++] = ares = min(a0,a1);
	}
	return n;
}

template<class F> int intersect_lists(F *pSrc0,int nSrc0, F *pSrc1,int nSrc1, F *pDst)
{
	int i0,i1,n; F ares;
	for(i0=i1=n=0; isneg(i0-nSrc0) & isneg(i1-nSrc1); i0+=isneg(pSrc0[i0]-ares-1),i1+=isneg(pSrc1[i1]-ares-1)) {
		pDst[n] = ares = min(pSrc0[i0],pSrc1[i1]); n += iszero(pSrc0[i0]-pSrc1[i1]);
	}
	return n;
}





#if defined(PHYSICS_EXPORTS) || (defined(WIN64) && defined(CRYENTITYDLL_EXPORTS))  || (defined(LINUX))
#ifdef max
 #undef max
#endif
#ifdef min
 #undef min
#endif
inline double min(double op1,double op2) { return (op1+op2-fabs(op1-op2))*0.5; }
inline double max(double op1,double op2) { return (op1+op2+fabs(op1-op2))*0.5; }
inline float max(float op1,float op2) { return (op1+op2+fabsf(op1-op2))*0.5f; }
inline float min(float op1,float op2) { return (op1+op2-fabsf(op1-op2))*0.5f; }
inline int max(int op1,int op2) { return op1 - (op1-op2 & (op1-op2)>>31); }
inline int min(int op1,int op2) { return op2 + (op1-op2 & (op1-op2)>>31); }
inline double minmax(double op1,double op2,int bMax) { return (op1+op2+fabs(op1-op2)*(bMax*2-1))*0.5; }
inline float minmax(float op1,float op2,int bMax) { return (op1+op2+fabsf(op1-op2)*(bMax*2-1))*0.5f; }
inline int minmax(int op1,int op2,int bMax) {	return (op1&-bMax | op2&~-bMax) + ((op1-op2 & (op1-op2)>>31)^-bMax)+bMax; }
#endif

template<class F> inline F max_safe(F op1,F op2) { return op1>op2 ? op1:op2; }//int mask=isneg(op2-op1); return op1*mask+op2*(mask^1); }
template<class F> inline F min_safe(F op1,F op2) { return op1<op2 ? op1:op2; }//{ int mask=isneg(op1-op2); return op1*mask+op2*(mask^1); }


#ifndef PHYSICS_EXPORTS
#define VALIDATOR_LOG(pLog,str)
#define VALIDATORS_START
#define VALIDATOR(member)
#define VALIDATOR_NORM(member)
#define VALIDATOR_NORM_MSG(member,msg,member1)
#define VALIDATOR_RANGE(member,minval,maxval)
#define VALIDATOR_RANGE2(member,minval,maxval)
#define VALIDATORS_END
#endif


typedef struct VALUE16 {
	union {
		struct { unsigned char a,b; } c;
		unsigned short ab;
	};
} VALUE16;

inline unsigned short SWAP16(unsigned short l) {
	VALUE16 l16;
	unsigned char a,b;
	l16.ab=l;
 	a=l16.c.a;  b=l16.c.b;
 	l16.c.a=b; 	l16.c.b=a;
	return l16.ab;
}

//--------------------------------------------

typedef struct VALUE32 {
	union {
		struct { unsigned char a,b,c,d; } c;
		float FLOAT;
		unsigned long abcd;
		const void* ptr;
	};
} VALUE32;

inline unsigned long SWAP32(unsigned long l) {
	VALUE32 l32;
	unsigned char a,b,c,d;
	l32.abcd=l;
 	a=l32.c.a;  b=l32.c.b;  c=l32.c.c;  d=l32.c.d;
 	l32.c.a=d;	l32.c.b=c; 	l32.c.c=b; 	l32.c.d=a;
	return l32.abcd;
}

inline const void* SWAP32(const void* l) {
	VALUE32 l32;
	unsigned char a,b,c,d;
	l32.ptr=l;
 	a=l32.c.a;  b=l32.c.b;  c=l32.c.c;  d=l32.c.d;
 	l32.c.a=d;	l32.c.b=c; 	l32.c.c=b; 	l32.c.d=a;
	return l32.ptr;
}


inline float FSWAP32(float f) {
	VALUE32 l32;
	unsigned char a,b,c,d;
	l32.FLOAT=f;
	a=l32.c.a;  b=l32.c.b;  c=l32.c.c;  d=l32.c.d;
	l32.c.a=d;	l32.c.b=c; 	l32.c.c=b; 	l32.c.d=a;
	return l32.FLOAT;
}



#endif //math

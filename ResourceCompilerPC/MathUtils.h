#ifndef _CRY_ANIMATION_MATH_UTILS_HDR_
#define _CRY_ANIMATION_MATH_UTILS_HDR_

#include "IBindable.h"
#include "Cry_Geo.h"

// aligned on 16-byte boundary vector
class Vec3dA16
{
public:
	Vec3 v;
	unsigned m_Pad;

	Vec3dA16 () {}
	Vec3dA16 (float x, float y, float z):
	v(x,y,z)
	{}
};


// for the given forward vector and position, builds up the right vector and up vector
// and builds the corresponding matrix
// Builds the right and up vector, given the forward vector, and initializes the matOut
void BuildMatrixFromFwd (const Vec3& ptNormal, const Vec3& ptPosition, Matrix44& matOut);

// for the given forward vector and position, builds up the right vector and up vector
// and builds the corresponding matrix
// Builds the right and up vector, given the forward vector, and initializes the matOut
void BuildMatrixFromFwdZRot (const Vec3& ptNormal, const Vec3& ptPosition, float fZRotate, Matrix44& matOut);

inline bool IsOrthoUniform (const Matrix44& b, float fTolerance = 0.01f)
{
	for (int i = 0; i < 3; ++i)
	{
		if (fabs(b.GetRow(i).GetLengthSquared()-1) > fTolerance)
			return false;
		if (fabs(b.GetColumn(i).GetLengthSquared()-1) > fTolerance)
			return false;
	}
	return true;
}

inline void OrthoNormalize(Matrix44& m)
{
	m.NoScale();
}

// given the bone matrix, returns its inverted.
// the bone matrices are orthounitary
inline Matrix44 OrthoUniformGetInverted (const Matrix44& b)
{
#ifdef _DEBUG
	for (int i = 0; i < 3; ++i)
	{
		assert (fabs(b.GetRow(i).len2()-1) < 1e-2);
		assert (fabs(b.GetColumn(i).len2()-1) < 1e-2);
	}
#endif
	return GetInverted44(b);
	Matrix44 m;
	m(0,0) = b(0,0); m(0,1) = b(1,0); m(0,2)=  b(2,0); m(0,3) = 0;
	m(1,0) = b(0,1); m(1,1) = b(1,1); m(1,2)=  b(2,1); m(1,3) = 0;
	m(2,0) = b(0,2); m(2,1) = b(1,2); m(2,2)=  b(2,2); m(2,3) = 0;
	m.SetTranslationOLD(b.TransformVectorOLD(b.GetTranslationOLD())); m(3,3) = 1;

#ifdef _DEBUG
	Matrix44 e = b * m;
	assert (e.GetTranslationOLD().GetLengthSquared() < 0.01f);
#endif
	return m;
}

// rotates the matrix by the "Angles" used by the FarCry game
void Rotate (Matrix44& matInOut, const Vec3& vAngles);

// rotates the matrix by the "Angles" used by the FarCry game,
// but in inverse order and in opposite direction (effectively constructing the inverse matrix)
void RotateInv (Matrix44& matInOut, const Vec3& vAngles);

// Smoothes linear blending into cubic (b-spline) with 0-derivatives
// near 0 and 1
extern float SmoothBlendValue (float fBlend);

template <typename T>
T min2 (T a, T b)
{
	return a < b ? a : b;
}
template <typename T>
T max2 (T a, T b)
{
	return a > b ? a : b;
}

template <typename T>
T max3(T val1, T val2, T val3)
{
	return max2(max2(val1,val2),val3);
}

template <typename T>
T min3(T val1, T val2, T val3)
{
	return min2(min2(val1,val2),val3);
}

template <typename T>
T min4 (T a, T b, T c, T d)
{
	return min2(min2(a,b),min2(c,d));
}



#include "QuaternionExponentX87.h"

// checks whether the quaternions are almost equal
inline bool isEqual (const CryQuat& q, const CryQuat& p)
{
	const float fEpsilon = 0.05f;
	if (fabs(q.w-p.w) > fEpsilon)
		return false;
	if (fabs(q.v.x-p.v.x) > fEpsilon)
		return false;
	if (fabs(q.v.y-p.v.y) > fEpsilon)
		return false;
	if (fabs(q.v.z-p.v.z) > fEpsilon)
		return false;
	return true;
}

__inline void quaternionExponentOptimized(const Vec3& rSrcVector, CryQuat& rDstQuat)
{
#if DO_ASM && defined (_CPU_X86)
	float fResultWXYZ[4];
	quaternionExponent_x87 (&rSrcVector.x, fResultWXYZ);
	rDstQuat.w   = fResultWXYZ[0];
	rDstQuat.v.x = fResultWXYZ[1];
	rDstQuat.v.y = fResultWXYZ[2];
	rDstQuat.v.z = fResultWXYZ[3];

	assert (isEqual(rDstQuat,exp(quaternionf(0,rSrcVector))));
#else
	CryQuat tmp;
	tmp.v = rSrcVector;
	tmp.w = 0;
	rDstQuat = exp(tmp);
#endif
}






struct CryAABB
{

	Vec3 vMin;
	Vec3 vMax;

	CryAABB() {}

/*	CryAABB(struct IBindable* pObj)	{
		pObj->GetBBox(vMin, vMax);
	}*/

	CryAABB(const Vec3& a, const Vec3& b)	{
		vMin = a;
		vMax = b;
	}

	CryAABB(const CryAABB& right)	{
		vMin = right.vMin;
		vMax = right.vMax;
	}

	Vec3 getSize() const {return vMax-vMin;}
	Vec3 getCenter() const {return (vMin+vMax)*0.5f;}

	bool empty() const {return vMin.x >= vMax.x || vMin.y >= vMax.y || vMin.z >= vMax.z;}

	void include(const Vec3& v) { AddToBounds(v, vMin, vMax);} 
	void include (const CryAABB& right)	{
		include (right.vMin);
		include (right.vMax);
	}


//	static Vec3& toVec3d(Vec3&v) { return v; }
//	static const Vec3& toVec3d(const Vec3&v) {return v;}

//	static Vec3& toVec3d(Vec3dA16&v){return v.v;}
//	static const Vec3& toVec3d(const Vec3dA16&v){return v.v;}
//  CryAABB& operator = (const Vec3& v) {vMin=vMax = v; return *this;}
/*
	CryAABB(const Vec3& a)	{
		toVec3d(vMin) = a;
		toVec3d(vMax) = a;
	}*/


};






template <typename TVec>
class CryBBox_tpl
{
public:
	CryBBox_tpl(){}
	CryBBox_tpl(const Vec3& a)
		{
			toVec3d(vMin) = a;
			toVec3d(vMax) = a;
		}
	CryBBox_tpl(struct IBindable* pObj)
	{
		pObj->GetBBox(toVec3d(vMin), toVec3d(vMax));
	}
	CryBBox_tpl(const Vec3& a, const Vec3& b)
		{
			toVec3d(vMin) = a;
			toVec3d(vMax) = b;
		}
	template <typename T2>
	CryBBox_tpl(const CryBBox_tpl<T2>& right)
	{
		toVec3d(vMin) = toVec3d(right.vMin);
		toVec3d(vMax) = toVec3d(right.vMax);
	}

	TVec vMin;
	TVec vMax;

	bool empty()const {return vMin.x >= vMax.x || vMin.y >= vMax.y || vMin.z >= vMax.z;}

	static Vec3& toVec3d(Vec3&v){return v;}
	static const Vec3& toVec3d(const Vec3&v){return v;}
	static Vec3& toVec3d(Vec3dA16&v){return v.v;}
	static const Vec3& toVec3d(const Vec3dA16&v){return v.v;}

	Vec3 getSize() const {return toVec3d(vMax)-toVec3d(vMin);}
	Vec3 getCenter() const {return (toVec3d(vMin)+toVec3d(vMax))*0.5f;}
	CryBBox_tpl<TVec>& operator = (const Vec3& v) {toVec3d(vMin) = toVec3d(vMax) = v; return *this;}
	void include(const Vec3& v) { AddToBounds(v, toVec3d(vMin), toVec3d(vMax));} 
	template <typename T3>
		void include (const CryBBox_tpl<T3>&right)
	{
		include (toVec3d(right.vMin));
		include (toVec3d(right.vMax));
	}
};

//typedef CryBBox_tpl<Vec3> CryBBox;
typedef CryBBox_tpl<Vec3dA16> CryBBoxA16;


//extern Vec3 UntransformVector (const Matrix& m, const Vec3& v);
inline bool isUnit (const Vec3& v, float fTolerance)
{
	float fLen = v.GetLengthSquared();
	return fLen >= 1-fTolerance && fLen < 1+fTolerance;
}

inline bool isSane(float x)
{
	return x > -1e9 && x < 1e9;
}

inline bool isSane (const Vec3& v)
{
	return isSane(v.x) && isSane(v.y) && isSane(v.z);
}

inline bool isUnit (const TangData& td, float fTolerance)
{
	return isUnit(td.binormal, fTolerance) && isUnit(td.tangent, fTolerance) && isUnit(td.tnormal, fTolerance);
}

#endif
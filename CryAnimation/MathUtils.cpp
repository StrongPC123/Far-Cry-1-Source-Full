#include "stdafx.h"
#include "MathUtils.h"

#include "IStatObj.h"

// RightUpOut = two vectors: right and up, the output
void BuildRightUpFromFwd (const Vec3& ptNormal, const Vec3& ptPosition, Vec3* pRightUpOut)
{
	Vec3& ptRight=pRightUpOut[0];
	Vec3& ptUp=pRightUpOut[1];

	int nNormal[3] = {0,1,2};
	for (int i = 0; i < 2; ++i)
		for (int j = i + 1; j < 3; ++j)
			if (fabs(ptNormal[nNormal[i]]) < fabs(ptNormal[nNormal[j]]))
				std::swap (nNormal[i],nNormal[j]);

	ptRight[nNormal[2]] = 0;
	ptRight[nNormal[1]] = -ptNormal[nNormal[0]];
	ptRight[nNormal[0]] = ptNormal[nNormal[1]];

	ptUp = ptRight ^ ptNormal;
}

// for the given forward vector and position, builds up the right vector and up vector
// and builds the corresponding matrix
void BuildMatrixFromFwd (const Vec3& ptNormal, const Vec3& ptPosition, Matrix44& matOut)
{
	Vec3 ptRightUp[2];
	BuildRightUpFromFwd(ptNormal, ptPosition, ptRightUp);
	matOut.BuildFromVectors(ptRightUp[0], ptRightUp[1], ptNormal, ptPosition);
}

// for the given forward vector and position, builds up the right vector and up vector
// and builds the corresponding matrix
void BuildMatrixFromFwdZRot (const Vec3& ptNormal, const Vec3& ptPosition, float fZRotate, Matrix44& matOut)
{
	Vec3 ptRightUp[2];
	BuildRightUpFromFwd(ptNormal, ptPosition, ptRightUp);

	float fCosSin[2];
	cry_sincosf(fZRotate, fCosSin);
	matOut.BuildFromVectors(ptRightUp[0]*fCosSin[0]+ptRightUp[1]*fCosSin[1], ptRightUp[1]*fCosSin[0]-ptRightUp[0]*fCosSin[1], ptNormal, ptPosition);
}

// rotates the matrix by the "Angles" used by the FarCry game
void Rotate (Matrix44& matInOut, const Vec3& vAngles)
{
	matInOut=matInOut*Matrix33::CreateRotationX( DEG2RAD(-vAngles.x) );
	matInOut=matInOut*Matrix33::CreateRotationY( DEG2RAD(+vAngles.y) ); //IMPORTANT: radian-angle must be negated 
	matInOut=matInOut*Matrix33::CreateRotationZ( DEG2RAD(-vAngles.z) ); 

}

// rotates the matrix by the "Angles" used by the FarCry game
// but in inverse order and in opposite direction (effectively constructing the inverse matrix)
void RotateInv (Matrix44& matInOut, const Vec3& vAngles)
{
	matInOut=matInOut*Matrix33::CreateRotationZ( DEG2RAD(vAngles.z) ); 
	matInOut=matInOut*Matrix33::CreateRotationY( DEG2RAD(-vAngles.y) ); //IMPORTANT: radian-angle must be negated 
	matInOut=matInOut*Matrix33::CreateRotationX( DEG2RAD(vAngles.x) );
}




// Smoothes linear blending into cubic (b-spline) with 0-derivatives
// near 0 and 1
float SmoothBlendValue (float fBlend)
{
	if (fBlend <= 0) return 0;
	if (fBlend >= 1) return 1;
	fBlend -= 0.5f;
	return 0.5f - 2.0f * (fBlend*fBlend*fBlend) + 1.5f * fBlend;
}


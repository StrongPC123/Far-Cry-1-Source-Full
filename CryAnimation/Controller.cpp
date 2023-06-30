// Controller.cpp: implementation of the utilities declared with the
// IController interface
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CryBone.h"
#include "CryModEffector.h"
#include "CryModEffIKSolver.h"
#include "CryModelState.h"
#include "Controller.h"
#include "ControllerManager.h"
#include "CryKeyInterpolation.h"
#include "MathUtils.h"

// returns the rotation quaternion
CryQuat IController::PQLog::getOrientation()const
{
	return exp( quaternionf(0,vRotLog) );
}

// resets the state to zero
void IController::PQLog::reset ()
{
	vPos(0,0,0);
	vRotLog(0,0,0);
}

// blends the pqSource[0] and pqSource[1] with weights 1-fBlend and fBlend into pqResult
void IController::PQLog::blendPQ (const PQLog* pqSource, float fBlend)
{
	blendPQ (pqSource[0], pqSource[1], fBlend);
}

// returns the equivalent rotation in logarithmic space (the quaternion of which is negative the original)
Vec3 IController::PQLog::getComplementaryRotLog() const
{
	// assuming |vRotLog| < gPi
	Vec3 vResult = vRotLog * float(1 - gPi/vRotLog.Length());
#ifdef _DEBUG
	//CryQuat qOrig = exp(vRotLog);
	//CryQuat qNew = exp(vResult);
	//assert (qOrig.Dot(qNew) < -0.99);
#endif
	return vResult;
}

// blends the pqFrom and pqTo with weights 1-fBlend and fBlend into pqResult
void IController::PQLog::blendPQ (const PQLog& pqFrom, const PQLog& pqTo, float fBlend)
{
	assert (fBlend >= 0 && fBlend <=1);
	vPos = pqFrom.vPos * (1-fBlend) + pqTo.vPos * fBlend;
	vRotLog = pqFrom.vRotLog * (1-fBlend) + pqTo.vRotLog * fBlend;
}

// builds the matrix out of the position and orientation stored in this PQLog
void IController::PQLog::buildMatrix(Matrix44& matDest)const
{
	CryQuat qRot;
	quaternionExponentOptimized(vRotLog, qRot);
	//BuildMatrixFromQP(matDest, qRot, vPos);
	matDest=Matrix44(qRot);
	matDest.SetTranslationOLD(vPos);
}

void IController::PQLog::assignFromMatrix (const Matrix44& mat)
{
	this->vPos = mat.GetTranslationOLD();

	Matrix33 m(mat);
	m.SetRow (0, mat.GetRow(0).normalized());
	m.SetRow (1, mat.GetRow(1).normalized());
	m.SetRow (2, mat.GetRow(2).normalized());

	// check for parity
	if ((m.GetRow(0)^m.GetRow(1))*m.GetRow(2) < 0)
		m.SetRow (2, -m.GetRow(2));

	this->vRotLog = log(CryQuat(m)).v;

#ifdef _DEBUG
	Matrix44 t;
	buildMatrix(t);
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			assert (fabs(m(i,j)-t(i,j)) < 0.5);
#endif
}

// a special version of the buildMatrix that adds a rotation to the rotation of this PQLog
void IController::PQLog::buildMatrixPlusRot(Matrix44& matDest, const CryQuat& qRotPlus)const
{
	CryQuat qRot;
	quaternionExponentOptimized(vRotLog, qRot);
	//BuildMatrixFromQP(matDest, qRot*qRotPlus, vPos);
	matDest=Matrix44(qRot*qRotPlus);	matDest.SetTranslationOLD(vPos);
}

Vec3& operator *= (Vec3& v, double d)
{
	v.x = float(v.x*d);
	v.y = float(v.y*d);
	v.z = float(v.z*d);
	return v;
}

// adjusts the rotation of these PQs: if necessary, flips them or one of them (effectively NOT changing the whole rotation,but
// changing the rotation angle to Pi-X and flipping the rotation axis simultaneously)
// this is needed for blending between animations represented by quaternions rotated by ~PI in quaternion space
// (and thus ~2*PI in real space)
void AdjustLogRotations (Vec3& vRotLog1, Vec3& vRotLog2)
{
	double dLen1 = DLength(vRotLog1);
	if (dLen1 > gPi/2)
	{
		vRotLog1 *= 1 - gPi/dLen1; 
		// now the length of the first vector is actually gPi - dLen1,
		// and it's closer to the origin than its complementary

		// but we won't need the dLen1 any more
	}

	double dLen2 = DLength(vRotLog2);
	// if the flipped 2nd rotation vector is closer to the first rotation vector,
	// then flip the second vector
	if (vRotLog1 * vRotLog2 < (dLen2 - gPi/2) * dLen2)
	{
		// flip the second rotation also
		vRotLog2 *= 1 - gPi / dLen2;
	}
}

void AdjustLogRotationTo (const Vec3& vRotLog1, Vec3& vRotLog2)
{
	double dLen2 = DLength(vRotLog2);

	if (dLen2 > gPi/2)
	{
		if (dLen2 > gPi)
		{
			double dReminder = fmod (dLen2, gPi);
			if (dReminder <= gPi/2)
			{
				// we don't need to flip the axis
				vRotLog2 *= dReminder / dLen2;
				dLen2 = dReminder;
			}
			else
			{
				// we need to flip the axis
				vRotLog2 *= (dReminder - gPi)/dLen2;
				dLen2 = fabs(dReminder - gPi);
			}
		}
		else
		{
			vRotLog2 *= 1 - gPi/dLen2; // (dLen2-gPi)/dLen2
			dLen2 = fabs(dLen2 - gPi);
		}
		// now the length of the first vector is actually gPi - dLen1,
		// and it's closer to the origin than its complementary

		// but we won't need the dLen1 any more
	}

	// if the flipped 2nd rotation vector is closer to the first rotation vector,
	// then flip the second vector
	if (vRotLog1 * vRotLog2 < (dLen2 - gPi/2) * dLen2)
	{
		// flip the second rotation also
		vRotLog2 *= 1 - gPi / dLen2;
	}
}


void AdjustLogRotation(Vec3& vRotLog)
{
	double dLen = DLength(vRotLog);
	if (dLen > gPi/2)
	{
		if (dLen > gPi)
		{
			double dReminder = fmod (dLen, gPi);
			if (dReminder <= gPi/2)
			{
				// we don't need to flip the axis
				vRotLog *= dReminder / dLen;
			}
			else
			{
				// we need to flip the axis
				vRotLog *= (dReminder - gPi)/dLen;
			}
		}
		else
			vRotLog *= 1 - gPi/dLen; // (dLen-gPi)/dLen
		// now the length of the first vector is actually gPi - dLen1,
		// and it's closer to the origin than its complementary

		// but we won't need the dLen1 any more
	}

}
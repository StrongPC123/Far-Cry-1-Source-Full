/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//  Content:
//  Single bone implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <float.h>
#include <StlUtils.h>
#include "CryModel.h"
#include "CryBoneInfo.h"
#include "CryModelState.h"
#include "ControllerManager.h"
#include "ChunkFileReader.h"
#include "STringUtils.h"
#include "CVars.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CryBone::CryBone():
	m_bUseMatPlus (false),
	m_bUseReadyRelativeToParentMatrix (false),
	m_pParent(NULL)
{
  //m_MatPlus.Identity();
  m_matRelativeToParent.SetIdentity();; // current result of animation

	// zero orientation and rotation
  m_pqTransform.reset();
}

void CryBone::setParent (CryModelState*pParent)
{
	m_pParent = pParent;
}

static inline bool isone (float x)
{
	return x >= 0.98f && x < 1.02f;
}

//////////////////////////////////////////////////////////////////////////
// builds the relative to parent matrix
void CryBone::BuildRelToParentFromQP (const IController::PQLog& pqNew)
{
	if (m_bUseMatPlus)
		pqNew.buildMatrixPlusRot (m_matRelativeToParent, m_qRotPlus);
	else
		pqNew.buildMatrix (m_matRelativeToParent);

	if (_isnan(m_matRelativeToParent[0][0]))
		g_GetLog()->LogWarning ("\001CryBone::BuildRelToParentFromQP: Invalid QP(q=%g,%g,%g;p=%g,%g,%g)", pqNew.vRotLog.x,pqNew.vRotLog.y,pqNew.vRotLog.z,pqNew.vPos.x,pqNew.vPos.y,pqNew.vPos.z);
#ifdef _DEBUG
	if (g_GetCVars()->ca_Debug())
	{
		// we don't have translations that are millions of kilometers away
		assert (GetLengthSquared(m_matRelativeToParent.GetTranslationOLD()) < 1e22);
		// we should have the normal orthogonal and unitary matrix
		assert (isone(GetLengthSquared(m_matRelativeToParent.GetRow(0))));
		assert (isone(GetLengthSquared(m_matRelativeToParent.GetRow(1))));
		assert (isone(GetLengthSquared(m_matRelativeToParent.GetRow(2))));
	}
#endif
}

// builds the m_pqTransform from the internal relative to parent matrix
void CryBone::BuildQPFromRelToParent ()
{
	m_pqTransform.assignFromMatrix(m_matRelativeToParent);
}

// adds an offset to the bone relative to parent matrix
void CryBone::AddOffsetRelToParent (const Vec3d& vOffset)
{
	m_matRelativeToParent.AddTranslationOLD(vOffset);
}

void CryBone::ScaleRelToParent (const Vec3d& vScale)
{
  //m_matRelativeToParent *= fScale;
	Vec3d vPos = m_matRelativeToParent.GetRow(3);
	for (int i = 0; i < 3; ++i)
	{
		m_matRelativeToParent.SetRow(i, vScale[i]*m_matRelativeToParent.GetRow(i));
		vPos[i] = vScale[i]*vPos[i];
	}
	m_matRelativeToParent.SetRow(3,vPos);
}



Vec3d CryBone::GetBonePosition()
{
  return *(Vec3d*) (getMatrixGlobal()[3]);
}


Vec3d CryBone::GetBoneAxis(char cAxis)
{
	assert (cAxis == 'x' || cAxis == 'y' || cAxis == 'z');
  switch(cAxis)
  {
    case 'x':
		case 'y':
		case 'z':
      return Vec3d(getMatrixGlobal()[cAxis - 'x']);
			break;
  }

  return Vec3d(0,0,0);
}

// checks for possible memory corruptions in this object and its children
void CryBone::SelfValidate ()const
{
#ifdef _DEBUG
#endif
}


CryQuat quatAxis (int nAxis, float fDegrees)
{
	struct  {
		double cos;
		double sin;
	} x;
	cry_sincos( DEG2RAD(fDegrees)/2, &x.cos);
	CryQuat q;
	q.w = float(x.cos);
	switch (nAxis)
	{ 
	case 0: //x
		q.v.x = float(x.sin); q.v.y = q.v.z = 0;
		break;
	case 1: //y
		q.v.y = float(x.sin); q.v.x = q.v.z = 0;
		break;
	case 2: //z
		q.v.z = float(x.sin); q.v.x = q.v.y = 0;
		break;
	}
	return q;
}


//////////////////////////////////////////////////////////////////////////
// sets the plus-matrix rotation components.
// the plus-matrix is used to rotate the upper body in response
// to small rotation of the head (in order to avoid to frequent
// rotation of the whole body) and is set by the game code
//
void CryBone::SetPlusRotation(float dX, float dY, float dZ)
{
	if (g_GetCVars()->ca_NoMatPlus() || (dX == 0 && dY == 0 && dZ == 0))
	{
		m_bUseMatPlus = false;
		return;
	}
	
	if (!isSane(dX) || !isSane(dY) || !isSane(dZ))
	{
		m_bUseMatPlus = false;
		g_GetLog()->LogError ("\003CryBone::SetPlusRotation: insane value passed as an angle(%g,%g,%g); ignoring.",dX, dY,dZ);
		return;
	}

	m_bUseMatPlus = true;
	m_qRotPlus = 
		quatAxis (0, -dX) * quatAxis(1, -dY) * quatAxis(2, -dZ);

	if (g_GetCVars()->ca_MatPlusDebug())
	{
		if (fabs(dX) > 5 || fabs(dY) > 5 || fabs(dZ) > 5)
			g_GetLog()->LogToFile ("\005 MatPlus(%s, %s)(%.1f,%.1f,%.1f)", getBoneInfo()->getNameCStr(), m_bUseReadyRelativeToParentMatrix?"TM<-outside":"normal TM", dX,dY,dZ);
	}
}

//////////////////////////////////////////////////////////////////////////
// sets the plus-matrix rotation components.
// the plus-matrix is used to rotate the upper body in response
// to small rotation of the head (in order to avoid to frequent
// rotation of the whole body) and is set by the game code
//
void CryBone::SetPlusRotation(const CryQuat& qRot)
{
	if (g_GetCVars()->ca_NoMatPlus() || (qRot.v.x == 0 && qRot.v.y == 0 && qRot.v.z == 0))
	{
		m_bUseMatPlus = false;
		return;
	}
	
	if (!isSane(qRot.v) || !isSane(qRot.w))
	{
		g_GetLog()->LogError ("\001CryBone::SetPlusRotation: insane quaternion{%g,%g,%g,%g}",qRot.w,qRot.v.x,qRot.v.y,qRot.v.z);
		return;
	}

	float fUnit = qRot|qRot;
	if (fUnit < 0.9f || fUnit > 1.1f)
	{
		m_bUseMatPlus = false;
		g_GetLog()->LogError ("\003CryBone::SetPlusRotation(CryQuat): non-normalized quaternion {%g,%g,%g,%g} passed; ignoring.",qRot.w,qRot.v.x,qRot.v.y,qRot.v.z);
		return;
	}

	m_bUseMatPlus = true;
	m_qRotPlus = qRot;
}

// returns the parent world coordinate system rotation as a quaternion
CryQuat CryBone::GetParentWQuat ()
{
	if (!getBoneInfo()->hasParent())
		return CryQuat (1,0,0,0);

	const Matrix44& matParent = getParent()->getMatrixGlobal();
  //M2Q_CHANGED_BY_IVO
	//CryQuat qResult = CovertMatToQuat<float>(matParent);
	CryQuat qResult = Quat(matParent);

#ifdef _DEBUG
	if (g_GetCVars()->ca_DebugGetParentWQuat())
	{
		Matrix44 matTest;

		//Q2M_CHANGED_BY_IVO
		//qResult.GetMatrix(matTest);
		matTest=GetTransposed44(Matrix33(qResult));

		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; ++j)
				assert (fabs(matTest(i,j) - matParent(i,j)) < 0.02f);
	}
#endif

	return qResult;
}


//////////////////////////////////////////////////////////////////////////
// resets the plus-matrix to identity, so that the body is not rotated
// additionally to the currently played animation.
void CryBone::ResetPlusRotation() 
{
	m_bUseMatPlus = false;
	//m_MatPlus.Identity();
	m_qRotPlus = CryQuat(1, 0,0,0);
}

Matrix44& CryBone::getMatrixGlobal ()
{
	return m_pParent->getBoneMatrixGlobal (this);
}

const Matrix44& CryBone::getMatrixGlobal ()const
{
	return m_pParent->getBoneMatrixGlobal (this);
}

CryBoneInfo* CryBone::getBoneInfo()
{
	return m_pParent->getBoneInfo(this);
}
const CryBoneInfo* CryBone::getBoneInfo()const
{
	return m_pParent->getBoneInfo(this);
}

// fixes the bone matrix to the given position in world coordinates,
// assuming the character position and orientation are given by the vCharPos and vCharAngles
// vCharAngles are the same as in the entity and in the Draw call to ICryCharInstance
void CryBone::FixBoneOriginInWorld (const Vec3d& vCharPos, const Vec3d& vCharAngles, const Vec3d& vTargetOrigin)
{
	using namespace CryStringUtils;
	m_bUseReadyRelativeToParentMatrix = false; // assume the safety checks won't pass
	if (!isSane(vCharPos) || !isSane(vCharAngles) || !isSane(vTargetOrigin))
	{
		g_GetLog()->LogError ("\001CryBone::FixBoneOriginInWorld: insane input vCharPos = %s, vCharAngles = %s, vTargetOrigin = %s", toString(vCharPos).c_str(), toString(vCharAngles).c_str(), toString(vTargetOrigin).c_str());
		return;
	}


	// make tran&rot matrix		
	//Matrix44 matChar;
	//matChar.Identity();
	//matChar = GetTranslationMat(vCharPos)*matChar;
	//matChar = GetRotationZYX44(-gf_DEGTORAD*vCharAngles )*matChar; //NOTE: angles in radians and negated 

	//OPTIMIZED_BY_IVO
	Matrix44 matChar=Matrix34::CreateRotationXYZ( Deg2Rad(vCharAngles),vCharPos );
	matChar	=	GetTransposed44(matChar); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

	Matrix44 matParentBone;
	matParentBone.SetIdentity();
	for (CryBone* pParent = getParent(); pParent; pParent = pParent->getParent())
		matParentBone = matParentBone * pParent->m_matRelativeToParent;

	Matrix44 matInvParentBone = OrthoUniformGetInverted(matParentBone/*getParent()->getMatrixGlobal()*/*matChar);
	m_matRelativeToParent.SetTranslationOLD(matInvParentBone.TransformPointOLD(vTargetOrigin));
	m_bUseReadyRelativeToParentMatrix = true;
}

// Sets the bone matrix to the given position in world coordinates, only for this frame
// assuming the character position and orientation are given by the vCharPos and vCharAngles
// vCharAngles are the same as in the entity and in the Draw call to ICryCharInstance
void CryBone::SetBoneOriginInWorld (const Vec3d& vCharPos, const Vec3d& vCharAngles, const Vec3d& vTargetOrigin)
{
	using namespace CryStringUtils;
	if (!isSane(vCharPos) || !isSane(vCharAngles) || !isSane(vTargetOrigin))
	{
		g_GetLog()->LogError ("\001CryBone::SetBoneOriginInWorld: insane input vCharPos = %s, vCharAngles = %s, vTargetOrigin = %s", toString(vCharPos).c_str(), toString(vCharAngles).c_str(), toString(vTargetOrigin).c_str());
		return;
	}
// make tran&rot matrix		
//	Matrix44 matChar;
//	matChar.Identity();
//	matChar = GetTranslationMat(vCharPos)*matChar;
//	matChar = GetRotationZYX44(-gf_DEGTORAD*vCharAngles )*matChar; //NOTE: angles in radians and negated 

	//OPTIMIZED_BY_IVO
	Matrix44 matChar=Matrix34::CreateRotationXYZ( Deg2Rad(vCharAngles),vCharPos );
	matChar	=	GetTransposed44(matChar); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

	Matrix44 matInvChar = OrthoUniformGetInverted(matChar);
	getMatrixGlobal().SetTranslationOLD(matInvChar.TransformPointOLD(vTargetOrigin));
}


void CryBone::FixBoneMatrix (const Matrix44& mtxBone)
{
	using namespace CryStringUtils;
	// suppose we won't accept this matrix
	m_bUseReadyRelativeToParentMatrix = false;
	if (!(mtxBone(0,3)==0 && mtxBone(1,3)==0 && mtxBone(2,3)==0 && mtxBone(3,3)==1))
	{
		g_GetLog()->LogError ("\001CryBone::FixBoneMatrix: invalid matrix last column {%g,%g,%g,%g}; expected{0,0,0,1}",mtxBone(0,3),mtxBone(1,3),mtxBone(2,3),mtxBone(3,3));
		return;
	}

	Vec3d vTrans = mtxBone.GetRow(3);
	if (!isSane(vTrans))
	{
		g_GetLog()->LogError ("\001CryBone::FixBoneMatrix: insane translation vector %s",toString(vTrans).c_str());
		return;
	}

	for (int i = 0; i < 3; ++i)
	{
		Vec3d v = mtxBone.GetRow(i);
		if (!isUnit(v,0.5f))
		{
			g_GetLog()->LogError ("\001CryBone::FixBoneMatrix: matrix is not orthonormal, row %d :%s", i, toString(v).c_str());
			return;
		}
	}

	m_matRelativeToParent = mtxBone;
	m_bUseReadyRelativeToParentMatrix = true;
}

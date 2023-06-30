/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Oscar Blasco
//	Taken over by Vladimir Kajalin, Andrey Honich
//  Taken over by Sergiy Migdalskiy <sergiy@crytek.de>
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _BONE_H
#define _BONE_H

#include "CryHeaders.h"
#include "Controller.h"
#include "AnimationLayerInfo.h"
#include "CryBoneInfo.h"

class CryModel; 

//////////////////////////////////////////////////////////////////////////
// The bone class contains bone matrices and pointers to it parent and child’s.
// Also every bone has pointer to array of controllers for this bone.
// Bone uses controller to get position and orientation of bone for current animation frame.
class CryBone : public ICryBone
{
friend class CryModel;
public:
	CryBone();

	void setParent (CryModelState*pParent);

	// builds the relative to parent matrix
	void BuildRelToParentFromQP (const IController::PQLog& pqNew);

	// builds the m_pqTransform from the internal relative to parent matrix
	void BuildQPFromRelToParent ();

	// adds an offset to the bone relative to parent matrix
	void AddOffsetRelToParent (const Vec3& vOffset);
	void ScaleRelToParent (const Vec3& fScale);

	unsigned numChildren ()const {return getBoneInfo()->numChildren();}
	CryBone* getChild (unsigned i) {assert(i < numChildren()); return this + getBoneInfo()->m_nOffsetChildren + i;}
	const CryBone* getChild (unsigned i) const {assert(i < numChildren()); return this + getBoneInfo()->m_nOffsetChildren + i;}
	CryBone* getParent () {return getBoneInfo()->m_nOffsetParent ? this + getBoneInfo()->m_nOffsetParent : NULL;}
	int getParentIndexOffset()const {return getBoneInfo()->m_nOffsetParent;}
	const CryBone* getParent () const {return getBoneInfo()->m_nOffsetParent ? this + getBoneInfo()->m_nOffsetParent : NULL;}

	ICryBone* GetParent() {return getParent();}
	// returns the matrix relative to parent
	virtual const Matrix44& GetRelativeMatrix() {return m_matRelativeToParent;}
	// returns the matrix in object coordinates
	virtual const Matrix44& GetAbsoluteMatrix() {return getMatrixGlobal();}
	// fixes the bone matrix to the given position in world coordinates,
	// assuming the character position and orientation are given by the vCharPos and vCharAngles
	// vCharAngles are the same as in the entity and in the Draw call to ICryCharInstance
	virtual void FixBoneOriginInWorld (const Vec3& vCharPos, const Vec3& vCharAngles, const Vec3& vTargetOrigin);
	virtual void SetBoneOriginInWorld (const Vec3& vCharPos, const Vec3& vCharAngles, const Vec3& vTargetOrigin);
private:
	// only for debugging/logging purposes
	CryBoneInfo* getBoneInfo();
	const CryBoneInfo* getBoneInfo()const;

	CryModelState* m_pParent;
public:
	// ICryBone imlpementation
	const Matrix44& GetGlobalMatrix() const {return getMatrixGlobal();}

	// sets the plus-matrix rotation components.
	// the plus-matrix is used to rotate the upper body in response to small rotation of the head
	// (in order to avoid to frequent rotation of the whole body) and is set by the game code
	void SetPlusRotation(float x, float y, float z);
	void SetPlusRotation(const CryQuat& qRotation);

	// resets the plus-matrix to identity, so that the body is not rotated additionally to the currently played animation.
	void ResetPlusRotation() ;

	virtual Vec3 GetBonePosition();
	virtual Vec3 GetBoneAxis(char cAxis);
	// returns the parent world coordinate system rotation as a quaternion
	virtual CryQuat GetParentWQuat ();

	// todo: clear this up
	virtual void DoNotCalculateBoneRelativeMatrix(bool bDoNotCalculate) 
	{ m_bUseReadyRelativeToParentMatrix = bDoNotCalculate; };

	// remembers the given matrix as the bone's matrix (relative to parent) and from now on
	// doesn't change it. To reset this effect, call DoNotCalculateBoneRelativeMatrix(false)
	void FixBoneMatrix (const Matrix44& mtxBone);

	bool
		// if this is true, then the relative to parent matrix will be set from outside (by the lipsync at the moment)
		m_bUseReadyRelativeToParentMatrix
		// if this is false, we don't have to use the matPlus which is set from the outside, and can assume it's identity
		,m_bUseMatPlus
	;

	// relative to parent position and orientation of the bone
	IController::PQLog m_pqTransform;

  Matrix44 m_matRelativeToParent; // current result of animation
	//Matrix	m_matGlobal; // used by physics
	Matrix44& getMatrixGlobal ();
	const Matrix44& getMatrixGlobal ()const;

	CryQuat m_qRotPlus; // additional rotation set by the game; this is the logarithm of the quaternion representing the rotation
	
	// checks for possible memory corruptions in this object and its children
	void SelfValidate ()const;
};


#endif // _BONE_H
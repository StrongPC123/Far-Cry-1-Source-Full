
#ifndef _EFFECTOR_IKSOLVER_H
#define _EFFECTOR_IKSOLVER_H

class CryModelState;

class CCryModEffIKSolver
{
private:
	vectorf m_ptGoal,m_GoalNormal;
	float m_additLen,m_GoalHeight;
	int m_flags;
	// the model state owning this object - used to retrieve the bone info
	CryModelState* m_pParent;
	int m_nPrimaryBone;
	bool m_bActive;
	float m_fDeltaTime,m_fFeetAngle;

public:
	CCryModEffIKSolver(CryModelState* pParent);

	void SetPrimaryBone(int nBone){ m_nPrimaryBone = nBone;}		// Could be rootbone or a bone which will be affected.
	void Tick(float deltatime);
	void ApplyToBone(int nLayer);
	void Reset();

	void SetGoal(vectorf ptgoal,int flags,float addlen=0,vectorf goal_normal=vectorf(0,0,0),float goal_height=0);
	bool AffectsBone(int nBone);
	void UpdateBoneChildren(int iBone);
};


#endif
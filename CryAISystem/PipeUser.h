#ifndef _PIPE_USER_
#define _PIPE_USER_

// created by Petar
#include "GoalPipe.h"
#include "AIObject.h"
#include "Graph.h"


#include <vector>


class CGoalPipe;

typedef std::vector<QGoal> VectorOGoals;

class CPipeUser :public CAIObject , public IPipeUser
{

protected:

	AgentParameters		m_Parameters;

	VectorOGoals	m_vActiveGoals;
	bool				m_bBlocked;

	bool				m_bStartTiming;
	float				m_fEngageTime;

	CGoalPipe		*m_pCurrentGoalPipe;

	Vec3d				m_vLastHidePoint;
	
	

public:
	CPipeUser(void);
	virtual ~CPipeUser(void);

	void GetStateFromActiveGoals(SOBJECTSTATE &state);
	CGoalPipe *GetGoalPipe(const char *name);
	void RemoveActiveGoal(int nOrder);
	void SetAttentionTarget(CAIObject *pObject);
	void RestoreAttentionTarget( );
	void SetLastOpResult(CAIObject * pObject);

	virtual void Steer(const Vec3d & vTargetPos, GraphNode * pNode) {}
	virtual Vec3d FindHidePoint(float fSearchDistance, int nMethod, bool bIndoor = false, bool bSameOk=false) {return m_vPosition;}
	virtual void RequestPathTo(const Vec3d &pos);
	virtual void Devalue(CAIObject *pObject,bool bDevaluePuppets) {}
	virtual void Forget(CAIObject *pDummyObject) {}
	virtual void Navigate(CAIObject *pTarget) {}
	virtual void CreateFormation(const char * szName) {}

	CGoalPipe *GetCurrentGoalPipe() { return m_pCurrentGoalPipe;}
	void ResetCurrentPipe();
	void RegisterAttack(const char *name);
	void RegisterRetreat(const char *name);
	void RegisterWander(const char *name);
	void RegisterIdle(const char *name);
	bool SelectPipe(int id,const char *name, IAIObject *pArgument);
	bool InsertSubPipe(int id, const char * name, IAIObject * pArgument);

	IAIObject *GetAttentionTarget(void) {	return m_pAttentionTarget; }


	AgentParameters &GetParameters() { return m_Parameters;}

	// DEBUG MEMBERS
	string m_sDEBUG_GOAL;
	Vec3d m_vDEBUG_VECTOR;
	bool m_bDEBUG_Unstuck;
	//-----------------------------------

	bool m_bMovementSupressed;

	CAIObject *m_pAttentionTarget;
	CAIObject *m_pPrevAttentionTarget;
	CAIObject *m_pLastOpResult;		
	CAIObject *m_pReservedNavPoint;
	
	ListPositions m_lstPath;
	bool m_bHaveLiveTarget;
	bool m_AvoidingCrowd;

	float				m_fTimePassed; //! how much time passed since last full update

	bool m_bHiding;

	bool m_bAllowedToFire;
	bool m_bSmartFire;
	bool m_bDirectionalNavigation;	// true if the enemy should look where he is going

	bool m_bLooseAttention;		// true when we have don't have to look exactly at our target all the time
	CAIObject *m_pLooseAttentionTarget; // optional
	bool m_bUpdateInternal;

	bool		m_bLastHideResult;
	Vec3d		m_vLastHidePos;

	int m_nPathDecision;
	

	virtual void Save(CStream & stm);
	virtual void Load(CStream & stm);
};

#endif
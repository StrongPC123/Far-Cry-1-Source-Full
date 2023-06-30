// GoalPipe.h: interface for the CGoalPipe class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GOALPIPE_H__12BD0344_3B3F_4B55_8500_25581ECF7ACC__INCLUDED_)
#define AFX_GOALPIPE_H__12BD0344_3B3F_4B55_8500_25581ECF7ACC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IAgent.h"
#include <vector>
#include <string>
#include "pointer_container.h"


class CGoalOp;

class CAISystem;

typedef pointer_container<CGoalOp> GoalPointer;

typedef struct QGoal
{
	string name;
	GoalPointer pGoalOp;
	bool bBlocking;
	GoalParameters params;

	QGoal()
	{
		pGoalOp = 0;
		bBlocking = false;
	}

} QGoal;

typedef std::vector<QGoal> GoalQueue;

/*! This class defines a logical set of actions that an agent performs in succession.
*/
class CGoalPipe : public IGoalPipe
{

	CAISystem			*m_pAISystem;

	GoalQueue			m_qGoalPipe;

	
	unsigned int		m_nPosition;	// position in pipe
	CGoalPipe			*m_pSubPipe;
	
public:
	void Reset();
	CGoalPipe * Clone();
	CGoalPipe(const string &name, CAISystem *pAISystem);
	virtual ~CGoalPipe();

	// IGoalPipe
	void PushGoal(const string &pName, bool bBlocking,GoalParameters &params);

	GoalPointer PopGoal(bool &blocking, string &name, GoalParameters &params,CPipeUser *pOperand);

	string		m_sName;		// name of this pipe
	CAIObject			*m_pArgument;
	// Makes the IP of this pipe jump to the desired position
	void Jump(int position);
	bool IsInSubpipe(void);
	CGoalPipe * GetSubpipe(void);
	void SetSubpipe(CGoalPipe * pPipe);
	size_t MemStats();
	int GetPosition() { return m_nPosition;}
	void SetPosition(int iNewPos) { if ((iNewPos>0) && (iNewPos<(m_qGoalPipe.size()))) m_nPosition=iNewPos;}
};

#endif // !defined(AFX_GOALPIPE_H__12BD0344_3B3F_4B55_8500_25581ECF7ACC__INCLUDED_)

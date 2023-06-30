#include "stdafx.h"
#include "CAISystem.h"
#include <ISystem.h>
#include <IConsole.h>
#include <ITimer.h>
#include "GoalOp.h"
#include "pipeuser.h"
#include <stream.h>


CPipeUser::CPipeUser(void)
{
	m_bMovementSupressed = false;
	m_bDirectionalNavigation = false;
	m_pReservedNavPoint = 0;
	m_AvoidingCrowd = false;
	m_bHiding = false;
	m_bStartTiming = false;
	m_fEngageTime = 0;
}

CPipeUser::~CPipeUser(void)
{
}

void CPipeUser::GetStateFromActiveGoals(SOBJECTSTATE &state)
{
	FUNCTION_PROFILER(GetAISystem()->m_pSystem,PROFILE_AI);
	bool bSkipAdd = false;
	if (m_pCurrentGoalPipe)
	{
		if (!m_bBlocked)	// if goal queue not blocked
		{
			QGoal Goal;

			while (Goal.pGoalOp = m_pCurrentGoalPipe->PopGoal(Goal.bBlocking,Goal.name, Goal.params,this))
			{
							
				if (Goal.name == AIOP_LOOP)
				{
					bSkipAdd = true;
		
					if ( (m_vActiveGoals.empty()) && (m_lstPath.empty())  )
					{
						if (Goal.params.nValue == 1) 
						{
							if (!m_bLastHideResult)
								break;
						}
						else
							break;
					}

					m_pCurrentGoalPipe->Jump((int)Goal.params.fValue);
					break;
				}

				if (Goal.name == AIOP_CLEAR)
				{
					m_vActiveGoals.clear();
					GetAISystem()->FreeFormationPoint(m_Parameters.m_nGroup,this);
					SetAttentionTarget(0);
					m_bBlocked = false;
					m_bUpdateInternal = true;
					return;
				}


				if (!Goal.pGoalOp->Execute(this))
					break;
			}

			if (!Goal.pGoalOp)
				m_pCurrentGoalPipe->Reset();
			else
			{
				if (!bSkipAdd)
				{
					m_vActiveGoals.push_back(Goal);
					m_bBlocked = Goal.bBlocking;
				}
			}

		}
	}

	if (!m_vActiveGoals.empty())
	{
		//ListOGoals::iterator gi;

		//for (gi=m_lstActiveGoals.begin();gi!=m_lstActiveGoals.end();)
		for (size_t i = 0; i < m_vActiveGoals.size(); i++)
		{
			//QGoal Goal = (*gi);
			QGoal Goal = m_vActiveGoals[i];

			m_sDEBUG_GOAL = Goal.name;



			ITimer *pTimer = GetAISystem()->m_pSystem->GetITimer();
			int val = GetAISystem()->m_cvProfileGoals->GetIVal();

			if (val)
				pTimer->MeasureTime("");
			bool exec = Goal.pGoalOp->Execute(this);
			if (val)
			{
				float f = pTimer->MeasureTime("");
				TimingMap::iterator ti;
				ti = GetAISystem()->m_mapDEBUGTimingGOALS.find(Goal.name);
				if (ti == GetAISystem()->m_mapDEBUGTimingGOALS.end())
					GetAISystem()->m_mapDEBUGTimingGOALS.insert(TimingMap::iterator::value_type(Goal.name,f));
				else
				{
					if (f > ti->second)
						ti->second = f;
				}
			}

			if (exec)
			{
				RemoveActiveGoal(i);
				if (!m_vActiveGoals.empty())
					i--;

				if (Goal.bBlocking) 
					m_bBlocked = false;
			}
		}
	}
}

void CPipeUser::SetLastOpResult(CAIObject * pObject)
{
	if (m_pLastOpResult)
		if (m_pLastOpResult->GetType()==AIOBJECT_HIDEPOINT)
			GetAISystem()->RemoveObject(m_pLastOpResult);

	m_pLastOpResult = pObject;

}


void CPipeUser::SetAttentionTarget(CAIObject *pTarget)
{
	if (pTarget==0)
	{
		m_bHaveLiveTarget = false;
		if (m_pAttentionTarget && m_bCanReceiveSignals) // if I had a target previously I want to reevaluate
			m_State.bReevaluate = true;
	}
	else if (m_pAttentionTarget!=pTarget)
		m_State.bReevaluate = true;

	if(m_pAttentionTarget!=0 && m_pAttentionTarget->GetType()!=AIOBJECT_DUMMY 
		&& m_pAttentionTarget->GetType()!=200)	//FIXME  not to remember grenades - not good, needs change
		m_pPrevAttentionTarget = m_pAttentionTarget;
	m_pAttentionTarget = pTarget;
}


void CPipeUser::RestoreAttentionTarget( )
{
//fixMe	-	need to do something 
return;

	SetAttentionTarget( m_pPrevAttentionTarget );
//	m_pAttentionTarget = m_pPrevAttentionTarget;
//	if (m_pAttentionTarget==0)
//		m_bHaveLiveTarget = false;

}




void CPipeUser::RequestPathTo(const Vec3d &pos)
{
	m_nPathDecision=PATHFINDER_STILLTRACING;
	Vec3d myPos = m_vPosition;
	if (m_nObjectType == AIOBJECT_PUPPET)
		myPos.z-=m_fEyeHeight;
	GetAISystem()->TracePath(myPos,pos,this);
}

CGoalPipe *CPipeUser::GetGoalPipe(const char *name)
{
	CGoalPipe *pPipe = (CGoalPipe*) GetAISystem()->OpenGoalPipe(name);

	if (pPipe)
		return pPipe;
	else
		return 0;
}

void CPipeUser::RemoveActiveGoal(int nOrder)
{
	if (m_vActiveGoals.empty())
		return;
	int size = (int)m_vActiveGoals.size();

	if (size == 1)
	{
		m_vActiveGoals.front().pGoalOp->Reset(this);
		m_vActiveGoals.clear();
		return;
	}

	if (nOrder != (size-1))
		m_vActiveGoals[nOrder] = m_vActiveGoals[size-1];

	if (m_vActiveGoals.back().name == AIOP_TRACE)
		m_pReservedNavPoint = 0;

	m_vActiveGoals.back().pGoalOp->Reset(this);
	m_vActiveGoals.pop_back();
}

bool CPipeUser::SelectPipe(int id, const char *name, IAIObject *pArgument)
{


	if (pArgument)
		SetLastOpResult((CAIObject *) pArgument);


	if (m_pCurrentGoalPipe)
	{
		if (m_pCurrentGoalPipe->m_sName == string(name))
		{
			if (pArgument)
				m_pCurrentGoalPipe->m_pArgument = (CAIObject*)pArgument;
			return true;
		}
	}

	
	CGoalPipe *pPipe = 0;
	if (pPipe=GetAISystem()->IsGoalPipe(name))
	{
		pPipe->m_pArgument = (CAIObject*) pArgument;
		ResetCurrentPipe();
		m_pCurrentGoalPipe = pPipe;		// this might be too slow, in which case we will go back to registration
		m_pCurrentGoalPipe->Reset();
	}
	else 
		return false;
	
	m_pReservedNavPoint = 0;
	m_bDirectionalNavigation = false;

/*	if (m_pMyObstacle) 
	{
		m_pMyObstacle->bOccupied = false;
		m_pMyObstacle = 0;
	}
	*/
	return true;
}


void CPipeUser::RegisterAttack(const char *name)
{
	/*
	CGoalPipe *pPipe = GetGoalPipe(name);

	if ((pPipe) && (m_mapAttacks.find(name)==m_mapAttacks.end()))
	{
		// clone this pipe first.. each puppet must use its own copy
		CGoalPipe *pClone = pPipe->Clone();
		m_mapAttacks.insert(GoalMap::iterator::value_type(name,pClone));
	}
	*/
}

void CPipeUser::RegisterRetreat(const char *name)
{
	/*
	CGoalPipe *pPipe = GetGoalPipe(name);

	if ((pPipe) && (m_mapRetreats.find(name)==m_mapRetreats.end()))
	{
		// clone this pipe first.. each puppet must use its own copy
		CGoalPipe *pClone = pPipe->Clone();
		m_mapRetreats.insert(GoalMap::iterator::value_type(name,pClone));
	}
	*/
}


void CPipeUser::RegisterIdle(const char *name)
{
	/*
	CGoalPipe *pPipe = GetGoalPipe(name);

	if ((pPipe) && (m_mapIdles.find(name)==m_mapIdles.end()))
	{
		// clone this pipe first.. each puppet must use its own copy
		CGoalPipe *pClone = pPipe->Clone();
		m_mapIdles.insert(GoalMap::iterator::value_type(name,pClone));
	}
	*/
}



void CPipeUser::RegisterWander(const char *name)
{
	/*
	CGoalPipe *pPipe = GetGoalPipe(name);

	if ((pPipe) && (m_mapWanders.find(name)==m_mapWanders.end()))
	{
		// clone this pipe first.. each puppet must use its own copy
		CGoalPipe *pClone = pPipe->Clone();
		m_mapWanders.insert(GoalMap::iterator::value_type(name,pClone));
	}
	*/
}

bool CPipeUser::InsertSubPipe(int id, const char * name, IAIObject * pArgument)
{

	if (!m_pCurrentGoalPipe)
	{
		return false;
	}

	if (m_pCurrentGoalPipe->m_sName == name)
		return false;

	// first lets find the goalpipe
	CGoalPipe *pPipe = 0;
	if (pPipe=GetAISystem()->IsGoalPipe(name))
	{
		// now find the innermost pipe
		CGoalPipe *pExecutingPipe = m_pCurrentGoalPipe;
		while (pExecutingPipe->IsInSubpipe())
		{
			pExecutingPipe = pExecutingPipe->GetSubpipe();
			if (pExecutingPipe->m_sName == name)
			{
				delete pPipe;
				return false;
			}

		}

		//if (pExecutingPipe->m_sName != name)
		//{
				
			if (!m_vActiveGoals.empty() && m_bBlocked)
			{
				// pop the last executing goal
				RemoveActiveGoal(m_vActiveGoals.size()-1);
				// but make sure we end up executing it again
				pExecutingPipe->Jump(-1);

			}


			pExecutingPipe->SetSubpipe(pPipe);
			// unblock current pipe
			m_bBlocked = false;


			pPipe->m_pArgument = (CAIObject*) pArgument;
//		}
//		else
//		{
//			delete pPipe;
//			return false;
//		}
	}
	else 
		return false;

	

	if (pArgument)
		SetLastOpResult((CAIObject *) pArgument);

	m_bDirectionalNavigation = false;

	return true;
}

void CPipeUser::ResetCurrentPipe()
{
	if (!m_vActiveGoals.empty())
	{
		VectorOGoals::iterator li;
		for (li=m_vActiveGoals.begin();li!=m_vActiveGoals.end();li++)
		{
			QGoal goal = (*li);
			goal.pGoalOp->Reset(this);
		}

		m_vActiveGoals.clear();
	}


	if (m_pCurrentGoalPipe)
	{
		delete m_pCurrentGoalPipe;
		m_pCurrentGoalPipe = 0;
	}

	m_bBlocked = false;
	m_bUpdateInternal = true;
	m_bLooseAttention = false;
	if (m_pLooseAttentionTarget) 
		m_pAISystem->RemoveDummyObject(m_pLooseAttentionTarget);
	m_pLooseAttentionTarget  = 0;
	m_State.left = m_State.right = false;
}

void CPipeUser::Save(CStream & stm)
{

}

void CPipeUser::Load(CStream & stm)
{
	
}

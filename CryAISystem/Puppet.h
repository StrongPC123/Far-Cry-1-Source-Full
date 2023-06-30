// Puppet.h: interface for the CPuppet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PUPPET_H__539B7168_3AA0_47B1_9D72_723B52A869E2__INCLUDED_)
#define AFX_PUPPET_H__539B7168_3AA0_47B1_9D72_723B52A869E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "CAISystem.h"
#include "PipeUser.h"
#include "AgentParams.h"
#include "IAgent.h"
#include "GoalPipe.h"
#include "Graph.h"
#include <list>
#include <map>
#include <vector>

#define PUPPETSTATE_NONE				0
#define PUPPETSTATE_ATTACKING		1
#define PUPPETSTATE_RETREATING	2
#define PUPPETSTATE_IDLING			3
#define PUPPETSTATE_WANDERING		4


class CGoalOp;

//! vision sensory data
typedef struct VisionSD
{
	bool	bFrameTag;			// set to true if this object was seen this frame
	float fExposureTime;
	float fThreatIndex;					// how interesting is this sensory data
	float fInterestIndex;					// how interesting is this sensory data
	float fExpirationTime;

	VisionSD()
	{
		bFrameTag = false;
		fExposureTime = 0;
		fThreatIndex =0;
		fInterestIndex = 0;
		fExpirationTime = 0;
	}
} VisionSD;

typedef struct SoundSD
{
	float fThreatIndex;
	float fInterestIndex;
	Vec3d vPosition;
	float fTimeout;
	CAIObject *pDummyRepresentation;
	CAIObject *pOwner;
} SoundSD;

typedef struct MemoryRecord
{
	CAIObject *pDummyRepresentation;
	float fIntensity;
	float fThreatIndex;
	Vec3d vLastKnownPosition;
} MemoryRecord;

typedef std::vector<QGoal> VectorOGoals;
typedef std::map<CAIObject*,VisionSD> VisibilityMap;
typedef std::map<int,SoundSD> AudibilityMap;
typedef std::map<CAIObject*,MemoryRecord> MemoryMap;
typedef std::map<CAIObject*,float> DevaluedMap;
typedef std::map<CAIObject*,CAIObject*> ObjectObjectMap;


class CPuppet : public CPipeUser, IPuppet
{
//VehicleChange
protected:	
	//AgentParameters		m_Parameters;
	
	
	float				m_fDEBUG_MaxHealth;
	float				m_fMotionAddition;
	
	float				m_fSuppressFiring;	// in seconds

	float				m_fAIMTime;
	bool				m_bInFire;
	Vec3d				m_vLastTargetVector;
	int					m_nSampleFrequency;
	bool				m_bTargetDodge;
	bool				m_bOnceWithinAttackRange;	//<<FIXME>> Remove this 

	// to prevent puppet firing gun while still not looking straight at the target
	bool				m_bAccurateDirectionFire;

	float				m_fMaxThreat;
	float				m_fRespawnTime;

	GoalMap			m_mapAttacks;
	GoalMap			m_mapRetreats;
	GoalMap			m_mapWanders;
	GoalMap			m_mapIdles;

//	Vec3d				m_vLastHidePoint;
//	bool				m_bLastHideResult;


//	VectorOGoals	m_vActiveGoals;
//	bool				m_bBlocked;

	
	float m_fAccuracySupressor;

	
	bool m_bRunning;
	bool m_bLeftStrafe;
	bool m_bRightStrafe;


	
	ObstacleData *m_pMyObstacle;		// used to track when this puppet occupies a hiding place
	
	
public:
	
//	void SetAttentionTarget(CAIObject *pObject);
	bool PointAudible(const Vec3d &pos, float fRadius);
	void Forget(CAIObject *pDummyObject);
	void OnObjectRemoved(CAIObject *pObject);
	bool m_bDryUpdate;
	void RequestPathTo(const Vec3d &pos);
	void Navigate(CAIObject *pTarget);
	bool PointVisible(const Vec3d &pos);
	void Event(unsigned short eType, SAIEVENT *pEvent);
	void UpdatePuppetInternalState();
	bool CanBeConvertedTo(unsigned short type, void **pConverted);
	void AddToVisibleList(CAIObject *pAIObject, bool bForce = false, float fAdditionalMultiplier=1.f);
	void QuickVisibility();
	CPuppet();
	virtual ~CPuppet();

	//void RegisterAttack(const char *name);
	//void RegisterRetreat(const char *name);
	//void RegisterWander(const char *name);
	//void RegisterIdle(const char *name);
	//bool SelectPipe(int id,const char *name, IAIObject *pArgument);

	AgentParameters GetPuppetParameters() { return GetParameters();}
	void SetPuppetParameters(AgentParameters &pParams) { SetParameters(pParams);}

	void ParseParameters(const AIObjectParameters &params);
	void Update();

	void Devalue(CAIObject *pObject, bool bDevaluePuppets);

	//AgentParameters &GetParameters() { return m_Parameters;}

	bool m_bMeasureAll;
	//bool m_bHaveLiveTarget;
	
	CFormation *m_pFormation;
	IPuppetProxy	*m_pProxy;
	float m_fUrgency;
	bool m_bVisible;
//	bool m_bDEBUG_Unstuck;
//	bool m_bUpdateInternal;
	//bool m_bLooseAttention;		// true when we have don't have to look exactly at our target all the time
//	CGoalPipe		*m_pCurrentGoalPipe;
	float m_fCos;
	float m_fBound;
	
//	Vec3d m_vDEBUG_VECTOR;
	//string m_sDEBUG_GOAL;

	// move these to private after debug stage
	VisibilityMap m_mapVisibleAgents;
	MemoryMap m_mapMemory;
	DevaluedMap m_mapDevaluedPoints;
	DevaluedMap m_mapPotentialTargets;
	ObjectObjectMap m_mapInterestingDummies;
	AudibilityMap m_mapSoundEvents;

//	CAIObject *m_pAttentionTarget;

//	CAIObject *m_pLastOpResult;		// temporary here while real system in development

//	int m_nPathDecision;
//	ListPositions m_lstPath;
//	bool m_bAllowedToFire;
//	bool m_bSmartFire;
	int m_nBodyPos;
	
	
	float m_DEBUG_LASTUPDATETIME;
	CAIObject *m_pDEBUGLastHideSpot;

	void GetAgentParams(AgentParameters &params) { params = m_Parameters;	}
	void GetCurrentGoalName(string &name) {  if (m_pCurrentGoalPipe) name = m_pCurrentGoalPipe->m_sName;
																								else name = "";}


protected:
	void HandleSoundEvent(SAIEVENT *pEvent);
	void HandlePathDecision(SAIEVENT *pEvent);
	//void ResetCurrentPipe();
	void AssessThreat(CAIObject *pObject, VisionSD &data);
	void Remember(CAIObject *pObject, VisionSD &data);
	void HandleVisualStimulus(SAIEVENT *pEvent);
//	void GetStateFromActiveGoals(SOBJECTSTATE &state);
//	CGoalPipe *GetGoalPipe(const char *name);
//VehicleChange
//private:
	float m_fHorizontalFOVrad;
protected:
	// calculates threat based on input parameters and this puppet's parameters
	float CalculateThreat(const AgentParameters & params);
	// calculates interest value of the target with the given parameters
	float CalculateInterest(const AgentParameters & params);
public:
	// Steers the puppet outdoors and makes it avoid the immediate obstacles
//VehicleChange
	virtual void Steer(const Vec3d & vTargetPos, GraphNode * pNode);
	// debug function to unstuck the puppet if it is stuck
	void CreateFormation(const char * szName);
	void Reset(void);

	void ReleaseFormation(void);
	// removes a goal from the active goals and reshuffles the active goals
	//void RemoveActiveGoal(int nOrder);
	Vec3d GetIndoorHidePoint(int nMethod, float fSearchDistance, bool bSameOK);
protected:
	// decides whether to fire or not
	void FireCommand(void);
public:
	void AddToMemory(CAIObject * pObject);
	// finds hide point in graph based on specified search method
	Vec3d FindHidePoint(float fSearchDistance, int nMethod, bool bIndoor = false, bool bSameOk = false);
	// Evaluates whether the chosen navigation point will expose us too much to the target
	bool Compromising(const ObstacleData &od,bool bIndoor);
	
	// returns true if puppet visible
	bool Sees(CPuppet * pObject);
	void SetParameters(AgentParameters & sParams);
	//void SetLastOpResult(CAIObject * pObject);
	///bool InsertSubPipe(int id, const char * name, IAIObject * pArgument);
	Vec3d GetOutdoorHidePoint(int nMethod, float fSearchDistance, bool bSameOk);
	virtual IUnknownProxy* GetProxy() { return m_pProxy; };

	void CheckTargetLateralMovement();

	size_t MemStats();

	bool	m_bCloseContact;
	float	m_fLastUpdateTime;
	

	void CrowdControl(void);
	void CheckPlayerTargeting(void);
	void RemoveFromGoalPipe(CAIObject* pObject);
	CAIObject * GetMemoryOwner(CAIObject * pMemoryRepresentation);
	
	void Save(CStream & stm);
	void Load(CStream & stm);
	void Load_PATCH_1(CStream & stm);
};

#endif // !defined(AFX_PUPPET_H__539B7168_3AA0_47B1_9D72_723B52A869E2__INCLUDED_)

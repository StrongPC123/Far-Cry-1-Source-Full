#ifndef _IAGENT_H_
#define _IAGENT_H_

#include "Cry_Math.h"
#include "AgentParams.h"
#include <string>
#include <vector>
#include <algorithm>

#ifdef LINUX
#	include "platform.h"
#endif

struct IUnknownProxy;

//! Defines for AIObject types

#define	AIOBJECT_NONE 			200
#define AIOBJECT_DUMMY				0
#define	AIOBJECT_PUPPET				1
#define AIOBJECT_AWARE				2
#define AIOBJECT_ATTRIBUTE			3
#define AIOBJECT_WAYPOINT			6
#define AIOBJECT_HIDEPOINT		7
#define AIOBJECT_SNDSUPRESSOR	8
#define AIOBJECT_VEHICLE			30
#define AIOBJECT_HELICOPTER		40
#define AIOBJECT_CAR					50
#define AIOBJECT_BOAT					60
#define AIOBJECT_AIRPLANE			70
#define AIOBJECT_PIPEUSER			10
#define AIOBJECT_CPIPEUSER		11
#define AIOBJECT_MOUNTEDWEAPON	90
#define	AIOBJECT_PLAYER 		100

#define AIOBJECT_CPUPPET		20
#define AIOBJECT_CVEHICLE		21

//	defines for vehicles types
#define AIVEHICLE_BOAT				20
#define AIVEHICLE_CAR				30
#define AIVEHICLE_HELICOPTER		40
#define AIVEHICLE_AIRPLANE			50


//! Event types
#define AIEVENT_ONBODYSENSOR			1
#define AIEVENT_ONVISUALSTIMULUS		2
#define AIEVENT_ONPATHDECISION			3
#define AIEVENT_ONSOUNDEVENT			4
#define AIEVENT_AGENTDIED				5
#define AIEVENT_SLEEP					6
#define AIEVENT_WAKEUP					7
#define AIEVENT_ENABLE					8
#define AIEVENT_DISABLE					9
#define AIEVENT_REJECT					10
#define AIEVENT_PATHFINDON				11
#define AIEVENT_PATHFINDOFF				12
#define AIEVENT_CLOAK					13
#define AIEVENT_UNCLOAK					14
#define AIEVENT_CLEAR					15
#define AIEVENT_MOVEMENT_CONTROL		16		// based on parameters lets object know if he is allowed to move.
#define AIEVENT_DROPBEACON				17

#define AIREADIBILITY_INTERESTING		5
#define AIREADIBILITY_SEEN				10
#define AIREADIBILITY_LOST				20
#define AIREADIBILITY_NORMAL			30
#define AIREADIBILITY_NOPRIORITY		1


#define SIGNALFILTER_LASTOP					1
#define SIGNALFILTER_GROUPONLY			2
#define SIGNALFILTER_SPECIESONLY		3
#define SIGNALFILTER_ANYONEINCOMM		4
#define SIGNALFILTER_TARGET					5
#define SIGNALFILTER_SUPERGROUP			6
#define SIGNALFILTER_SUPERSPECIES		7
#define SIGNALFILTER_SUPERTARGET		8
#define SIGNALFILTER_NEARESTGROUP		9
#define SIGNALFILTER_NEARESTSPECIES	10
#define SIGNALFILTER_NEARESTINCOMM	11
#define SIGNALFILTER_HALFOFGROUP		12
#define SIGNALFILTER_READIBILITY		100


#define HM_NEAREST											0
#define HM_FARTHEST_FROM_TARGET					1
#define HM_NEAREST_TO_TARGET						2
#define HM_FARTHEST_FROM_GROUP					3
#define HM_NEAREST_TO_GROUP							4							
#define HM_LEFTMOST_FROM_TARGET					5	
#define HM_RIGHTMOST_FROM_TARGET				6	
#define HM_RANDOM												7	
#define HM_NEAREST_TO_FORMATION					10						
#define HM_FARTHEST_FROM_FORMATION			11					
#define HM_NEAREST_TO_LASTOPRESULT			20					
#define HM_FARTHEST_FROM_LASTOPRESULT		21					
#define HM_NEAREST_TO_LASTOPRESULT_NOSAME 22					
#define HM_FRONTLEFTMOST_FROM_TARGET		8	
#define HM_FRONTRIGHTMOST_FROM_TARGET		9	


#define BODYPOS_STAND		0
#define BODYPOS_CROUCH		1
#define BODYPOS_PRONE		2
#define BODYPOS_RELAX		3
#define BODYPOS_STEALTH		5



typedef struct AIObjectParameters
{
	AgentParameters	 m_sParamStruct;
	IUnknownProxy *pProxy;
	float	fEyeHeight;
	bool	bUsePathfindOutdoors;

} AIObjectParameters;

struct IAIObject;

typedef struct GoalParameters
{
	Vec3 m_vPosition;
	IAIObject *m_pTarget; 
	float fValue;
	float fValueAux;
	int nValue;
	bool bValue;
	string szString;
	//const char *szString;
    
	GoalParameters()
	{
		fValue = 0;
		fValueAux = 0;
		nValue = 0;
		//szString = 0;
		m_pTarget = 0;
	}

#if (defined(WIN64) || defined(LINUX64)) && !defined(_DLL) 
	// FIX: refcounted STL with static libs (e.g. on AMD64 compiler) will crash without these
	// TODO: AMD64 port: make a robust solution
	inline GoalParameters (const GoalParameters& params)
	{
		m_vPosition = params.m_vPosition;
		m_pTarget   = params.m_pTarget; 
		fValue      = params.fValue;
		fValueAux   = params.fValueAux;
		nValue      = params.nValue;
		bValue      = params.bValue;
		szString    = params.szString.c_str();
	}

	inline GoalParameters& operator = (const GoalParameters& params)
	{
		m_vPosition = params.m_vPosition;
		m_pTarget   = params.m_pTarget; 
		fValue      = params.fValue;
		fValueAux   = params.fValueAux;
		nValue      = params.nValue;
		bValue      = params.bValue;
		szString    = params.szString.c_str();
		return *this;
	}
#endif

} GoalParameters;


typedef struct IGoalPipe
{
	virtual void PushGoal(const string &name, bool bBlocking, GoalParameters &params) =0;
} IGoalPipe;


struct IAIObject;

typedef struct SAIEVENT
{
	bool bFuzzySight;
	int nDeltaHealth;
	float fThreat;
	float fInterest;
	IAIObject *pSeen;
	bool bPathFound;
	Vec3 vPosition;
} SAIEVENT;

struct SOBJECTSTATE;
class CStream;

typedef struct IAIObject
{
	virtual void SetRadius(float fRadius) = 0;
	virtual float GetRadius() = 0;
	virtual void SetPos(const Vec3 &pos,bool bKeepEyeHeight = true) = 0;
	virtual const Vec3 &GetPos() = 0;
	virtual void SetAngles(const Vec3 &angles) = 0;
	virtual const Vec3 &GetAngles() = 0;
	virtual unsigned short GetType() = 0;
	virtual void *GetAssociation() = 0;
	virtual void Release()=0;
	virtual void ParseParameters(const AIObjectParameters &params)=0;
	virtual bool CanBeConvertedTo(unsigned short type, void **pConverted) = 0;
	virtual void SetName(const char *pName)= 0;
	virtual char *GetName()= 0;
	virtual void IsEnabled(bool enabled)= 0;
	virtual void Event(unsigned short, SAIEVENT *pEvent) = 0;
	virtual void SetEyeHeight(float fHeight) = 0;
	virtual SOBJECTSTATE * GetState() = 0;
	virtual void Bind(IAIObject* bind) = 0;
	virtual void Unbind( ) = 0;
//	virtual IAIObject* GetBound( ) = 0;
	virtual	void CreateBoundObject( unsigned short type, const Vec3& vBindPos, const Vec3& vBindAngl)=0;
	virtual void EDITOR_DrawRanges(bool bEnable = true) = 0;
	virtual IUnknownProxy* GetProxy() = 0;
	virtual void SetSignal(int nSignalID, const char * szText, void *pSender=0) = 0;
	virtual bool IsMoving() = 0;
	virtual void Save(CStream & stm)=0;
	virtual void Load(CStream & stm)=0;
	virtual void Load_PATCH_1(CStream & stm)=0;
	virtual void NeedsPathOutdoor( bool bNeeds, bool bForce=false ) = 0;
	virtual bool IfNeedsPathOutdoor( ) = 0;
} IAIObject;

typedef struct IPupeUser
{
	virtual void RegisterAttack(const char *name)=0;
	virtual void RegisterRetreat(const char *name)=0;
	virtual void RegisterWander(const char *name)=0;
	virtual void RegisterIdle(const char *name)=0;
	virtual bool SelectPipe(int id, const char *name, IAIObject *pArgument = 0)=0;
	virtual bool InsertSubPipe(int id, const char *name, IAIObject *pArgument = 0)=0;
	virtual IAIObject *GetAttentionTarget()=0;
} IPipeUser;

typedef struct IPuppet 
{
	virtual AgentParameters GetPuppetParameters()=0;
	virtual void SetPuppetParameters(AgentParameters &pParams)=0;
} IPuppet;

typedef struct IVehicle 
{
//	virtual void SetGunner(IAIObject *pGunner)=0;
//	virtual AgentParameters &GetPuppetParameters()=0;
//	virtual void SetPuppetParameters(AgentParameters &pParams)=0;
} IVehicle;



// test state
typedef struct SAISTATE
{
	bool	forward;
	bool	back;
	bool  turning;
	bool  jump;
	bool left;
	bool right;
	float fValue;

} SAISTATE;


typedef struct AISIGNAL
{
	int						nSignal;
	///const char *			strText;
	char 			  strText[1024];
	void *					pSender;
} AISIGNAL;


typedef struct SOBJECTSTATE
{

//fixme	- remove it when dom\ne with tweaking of vehicles movement
	char	DEBUG_controlDirection;

	
	bool	forward;
	bool	back;
	bool  turnleft;
	bool	turnright;
	bool  turnup;
	bool	turndown;
	
	bool	left;
	bool	run;
	bool	right;
	bool	dodge;		// for vehicles - when avoiding rockets/grenades
//	bool	bLost;		// proxy would like to regenerate path

	enum	PathUsage
	{
		PU_NewPathWanted,	// 1 proxy would like to regenerate path
		PU_PathOK,			// 0 proxy don't need new path
		PU_NoPathfind,		// 2 proxy don't want to use pathfinder - just approach
	};

	PathUsage	pathUsage;
	
	char	mutantJump;
	bool	bCloseContact;

	float	fDesiredSpeed;	// 1 - max speed 0 - no speed
	float	fStickDist;
	float fValue;
	float fValueAux;
	bool fire;
	bool aimLook;
	int bodystate;
	Vec3 vFireDir;
	Vec3 vTargetPos;
	Vec3 vMoveDir;

	// jump related
	bool  jump;
	Vec3 vJumpDirection;
	float fJumpDuration;
	float nJumpDirection;

	bool bReevaluate;
	bool bTakingDamage;
	bool bHaveTarget;
	bool bMemory;
	bool bSound;
	bool bTargetEnabled;
	float fThreat;
	float fInterest;
	float fDistanceFromTarget;
	float fHealth;
	int nTargetType;
	std::vector<AISIGNAL> vSignals;
	int nAuxSignal;
	string szAuxSignalText;
	
	SOBJECTSTATE():
	vFireDir(1.0f,0.0f,0.0f),
	vTargetPos(1.0f,0.0f,0.0f),
	vMoveDir(1.0f,0.0f,0.0f)
	{
		Reset();
		mutantJump = 0;
		run = left = right = false;
		bodystate = 0;
		bMemory = false;
		bSound = false;
		bTakingDamage = false;
		bCloseContact = false;
		vSignals.clear();
		//pSignalSender.clear();
		fValueAux = 0;
		fStickDist = -1;
		pathUsage = PU_NewPathWanted;
		fDistanceFromTarget = 0;
		
	}

	void Reset()
	{

		DEBUG_controlDirection = 0;

		forward = back = turnleft = turnright = turnup = turndown = jump =  fire = false;
//		mutantJump = 0;
		bCloseContact = false;
		bHaveTarget = false;
		fThreat = 0;
		fInterest = 0;
		//left = right = false;
		//bReevaluate = false;
		fValueAux = 0;
	//	bTakingDamage = false;
		bMemory = false;
		bSound = false;
		vMoveDir(0,0,0);
		vTargetPos(0,0,0);
		bTargetEnabled = false;
		nAuxSignal = 0;
		fDesiredSpeed = 1.0f;
		
	}

	bool operator==(SOBJECTSTATE &other)
	{
		if (forward == other.forward)
			if (back == other.back)
				if (turnleft == other.turnleft)
					if (turnright == other.turnright)
					if (run == other.run)
					if (jump == other.jump)
						if (left == other.left)
							if (right == other.right)
								if (fire == other.fire)
									if (bodystate == other.bodystate)
											return true;
		return false;
	}


} SOBJECTSTATE;

typedef struct SMOTORSTATE
{
	bool crouched;
	bool proned;

	SMOTORSTATE()
	{
		crouched = proned = false;
	}
	
} SMOTORSTATE;

struct IAISystem;

// PROXY TYPES
#define	AIPROXY_PUPPET	1
#define	AIPROXY_OBJECT	2
#define	AIPROXY_VEHICLE 3

// PROXY STATUS TYPES to check
#define	AIPROXYSTATUS_INVEHICLE	1


class IPhysicalEntity;

typedef struct IUnknownProxy
{
	virtual bool QueryProxy(unsigned char type, void **pProxy) = 0;
	virtual int Update(SOBJECTSTATE *) = 0;
	virtual void Release()=0;
	virtual bool CustomUpdate(Vec3& pos, Vec3 &angles) = 0;
	virtual IPhysicalEntity* GetPhysics() = 0;
	virtual void DebugDraw(struct IRenderer *pRenderer) = 0;
	virtual bool CheckStatus(unsigned char status) = 0;
	virtual void ApplyHealth(float fHealth) = 0;
	virtual void Load(CStream &str)=0;
	virtual void Load_PATCH_1(CStream &str)=0;
	virtual void Save(CStream &str)=0;
//	virtual bool IsBound() = 0;
} IUnknownProxy;


typedef struct IPuppetProxy : public IUnknownProxy
{
	virtual void GetDimensions(int bodypos, float &eye_height, float &height) = 0;
	virtual void Reset() = 0;
	virtual void MovementControl(bool bEnableMovement, float fDuration) = 0;
} IPuppetProxy;

typedef struct IVehicleProxy : public IUnknownProxy
{
	virtual void GetDimensions(int bodypos, float &eye_height, float &height) = 0;
	virtual void Reset() = 0;
	virtual Vec3	UpdateThreat( void* threat ) = 0;
	virtual Vec3	HeliAttackAdvance( SOBJECTSTATE &state ) = 0;
	virtual void SetSpeeds(float fwd, float bkw=-1) = 0;
} IVehicleProxy;


typedef struct GameNodeData
{
	Vec3	m_pos;
	float fSlope;										// the incline of the point  1 means very inclined (45 deg) 0 is flat;
	bool  bWater;										// true, point in water, false point in dry land

	void Reset()
	{
		m_pos(0,0,0);
		fSlope = 0;
		bWater = false;
	}
} GameNodeData;

typedef struct ObstacleData
{
	Vec3 vPos;
	Vec3 vDir;

	bool operator==(const ObstacleData &other)
	{
		if (IsEquivalent(other.vPos,vPos,VEC_EPSILON)) 
			if (IsEquivalent(vDir,other.vDir,VEC_EPSILON))
				return true;

		return false;
	}

	ObstacleData()
	{
		vPos.Set(0,0,0);
		vDir.Set(0,0,0);
	}

} ObstacleData;

typedef std::vector<ObstacleData>	Obstacles;
typedef std::vector<int>			ObstacleIndexVector;

struct GraphNode;

typedef struct GraphLink
{
	GraphNode *pLink;											// next triangle this way
	unsigned int nStartIndex, nEndIndex;						// indices of the edge vertices of this edge
	float	fMaxRadius;											// maximum size sphere that can pass trough this edge
	Vec3	vEdgeCenter;
	Vec3	vWayOut;											// shows the out direction from this edge

	GraphLink()
	{
		pLink = 0;
		nStartIndex = nEndIndex = 0;
		fMaxRadius = 0;
		vWayOut(0,0,0);
		vEdgeCenter(0,0,0);
	}

	bool IsNewLink()
	{
		if( fMaxRadius<0.0f && fMaxRadius>-5.0f )
			return true;
		return false;
	}

} GraphLink;


typedef std::vector<GraphLink>	VectorOfLinks;
struct IVisArea;

typedef struct GraphNode
{
	VectorOfLinks link;
	ObstacleIndexVector vertex;
	bool tag;
	bool mark;
	bool bCreated;		// is true if designer created node
	float fHeuristic;
	float fDistance;
	int nRefCount;
	IVisArea *pArea;
	int nBuildingID;
	
	GameNodeData data;

	GraphNode()
	{
		bCreated = true;
		link.reserve(5);	
		tag = false;
		data.Reset();
		mark = false;
		nRefCount = 0;
		fHeuristic = -9999.f;
		pArea = NULL;	// this is outside
		nBuildingID = -1; // outside
		fDistance = 0;
	}

	~GraphNode()
	{
		link.clear();
		vertex.clear();
	}

	bool Release()
	{
		nRefCount--;

		assert(nRefCount>=0);

		if (nRefCount == 0)
			return true;
		return false;
	}

	void AddRef()
	{
		nRefCount++;
	}

	float				GetDegeneracyValue();
	void				MakeAntiClockwise();
	bool				IsAntiClockwise();
	float				GetCross( const Vec3 &vCutStart, const Vec3 &vDir, const GraphLink &theLink );
	GraphLink*			FindNewLink();

/*	void AddHidePoint(const Vec3 &pos, const Vec3 &dir)
	{
			ObstacleData od;
			od.vPos = pos;
			od.vDir = dir;
			if (std::find(vertex.begin(),vertex.end(),od) == vertex.end())
						vertex.push_back(od);
	}
	*/
	
} GraphNode;

typedef struct IGraph {
	virtual GraphNode * CreateNewNode(bool bFromTriangulation = false)=0;
	virtual void DeleteNode(GraphNode * pNode)=0;
	virtual void WriteToFile(const char *pname)=0;
	virtual bool ReadFromFile(const char *pname)=0;
	virtual bool RemoveEntrance(int nBuildingID, GraphNode * pNode)=0;
	virtual void RemoveIndoorNodes(void)=0;
	virtual void AddIndoorEntrance(int nBuildingID, GraphNode* pNode, bool bExitOnly = false) = 0;
	virtual void Connect(GraphNode *one, GraphNode *two) = 0;
	virtual void Disconnect(GraphNode * pDisconnected,bool bDelete = true	) =0;
	virtual GraphNode *GetEnclosing(const Vec3 &pos, GraphNode *pStart = 0 ,bool bOutsideOnly = false) = 0;
	virtual void AddHidePoint(GraphNode *pOwner, const Vec3 &pos, const Vec3 &dir) = 0;
	virtual void RemoveHidePoint(GraphNode *pOwner, const Vec3 &pos, const Vec3 &dir) = 0;
	virtual void DisableInSphere(const Vec3 &pos,float fRadius) = 0;
	virtual void EnableInSphere(const Vec3 &pos,float fRadius) = 0;
} IGraph;


#define AIHEURISTIC_DEFAULT		0
#define AIHEURISTIC_STANDARD	1
#define AIHEURISTIC_VEHICLE		2
// define additional heuristics here

// ATOMIC AI OPERATIONS

#define AIOP_ACQUIRETARGET	"acqtarget"			// ok
#define AIOP_LOOKAROUND			"lookaround"		// ok
#define AIOP_APPROACH				"approach"			// ok
#define AIOP_BACKOFF				"backoff"				// ok
#define AIOP_FIRECMD				"firecmd"				// ok
#define AIOP_STRAFE					"strafe"				// ok
#define AIOP_BODYPOS				"bodypos"				// ok
#define AIOP_RUN						"run"						// ok
#define AIOP_JUMP						"jump"					// ok
#define AIOP_TIMEOUT				"timeout"				// ok
#define AIOP_PATHFIND				"pathfind"			// ok
#define AIOP_LOCATE					"locate"				// ok
#define AIOP_TRACE					"trace"				
#define AIOP_SIGNAL					"signal"
#define AIOP_IGNOREALL			"ignoreall"
#define AIOP_DEVALUE				"devalue"				// ok
#define AIOP_FORGET					"forget"				// ok
#define AIOP_HIDE					  "hide"					// ok
///#define AIOP_MUTANTHIDE			"mutanthide"		// new
#define AIOP_FORM						"form"					// ok
#define AIOP_STICK					"stick"					// ok
#define AIOP_CLEAR					"clear"					// ok
#define AIOP_LOOP 					"branch"				// ok
#define AIOP_LOOKAT 				"lookat"				// ok
#define AIOP_HELIADV 				"heliadv"				// new





#endif //_IAGENT_H_

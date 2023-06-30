
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <IAgent.h>

#include "aihandler.h"

class CVehicle;


class CXVehicleProxy :
	public IVehicleProxy	//IPuppetProxy
{
public:
	CXVehicleProxy(IEntity *pEntity,IScriptSystem *pScriptSystem, CXGame *pGame);
	~CXVehicleProxy(void);

//	bool	IsBound() { return m_pEntity->IsBound(); }
	int Update(SOBJECTSTATE *state);
	void Release() { delete this; }

	bool QueryProxy(unsigned char type, void **pProxy);
	void GetDimensions(int bodypos, float &eye_height, float &height) { return; }
	bool CustomUpdate(Vec3d& pos, Vec3d& angle) {return false;}
	void Reset() { return; }
	void ApplyHealth( float fHealth ) {}
	IPhysicalEntity* GetPhysics() 
	{ 
		return m_pEntity->GetPhysics();
	}

	bool CheckStatus(unsigned char status) { return false; }

	void SetSignalFunc(HSCRIPTFUNCTION pFunc);
	void SetBehaviourFunc(HSCRIPTFUNCTION pFunc);
	void SetMotorFunc(HSCRIPTFUNCTION pFunc);

	void SetSpeeds(float fwd, float bkw=-1);
	void SetType( unsigned short	type ) { m_Type = type; }

	Vec3d	UpdateThreat( void* threat );
	void	SetMinAltitude( float minAlt ) { m_MinAltitude = minAlt; }

protected:
	void SendSignal(int signalID, const char * szText,IEntity *pSender);
	void SendAuxSignal(int signalID, const char * szText);
	void UpdateMind(SOBJECTSTATE *state);
	int UpdateMotor(SOBJECTSTATE *state);
	//------------------------------------------------------------------car movement stuff
	void MoveLikeACar(SOBJECTSTATE * state);
	Vec3d	UpdateThreatCar( void* threat );
	Vec3d	CarAvoidCollision( float distance );
	//------------------------------------------------------------------boat movement stuff
	void MoveLikeABoat(SOBJECTSTATE * state);
	bool BoatPointInWater( const Vec3d& pos );
	Vec3d	BoatFindAttackPoint( const Vec3d& targetPos );
	Vec3d	BoatAvoidCollision( float distance ); 
	//------------------------------------------------------------------helicopter movement stuff
	void MoveLikeAHelicopter(SOBJECTSTATE * state);
	void UpdateHover( Vec3d& posOffset, Vec3d& angOffset,	const float t );
	float UpdateAltitude( const Vec3d& moveDir, float desiredAltitude, float absMinAlt, bool useObjects=true );
	float GetObjectsBelowHeight( const Vec3d& pos, const float depth, const float scale=1.0f );
	float HelyEvaluatePosition( const Vec3d& pos );
	float HelyGetTerrainElevation( const Vec3d& pos, const Vec3d& fwdPos, float boxSize );
	bool  CanGoDown( const float deltaHeight );
	void UpdateDeviation( const Vec3d& targetPos, const float tScale );
	void DemperVect( Vec3d& current, const Vec3d& target, const float tScale );
	Vec3d	UpdateThreatHeli( void* threat );

	Vec3d	HelyAvoidCollision( float distance=35.0f );
	Vec3d	HeliAttackAdvance( SOBJECTSTATE &state );
	bool	IsTargetVisible( const Vec3d& selfPos, const Vec3d& targetPos );

//	void UpdateMovenent( const Vec3d& movement, const float tScale );
	//------------------------------------------------------------------
	void MoveLikeAnAirplane(SOBJECTSTATE * state);

private:	

	float	m_MinAltitude;

	IEntity		*m_pEntity;
	CVehicle	*m_pVehicle;


	unsigned short	m_Type;

	_HScriptFunction	m_hBehaviourFunc;
	_HScriptFunction	m_hMotorFunc;
	_HScriptFunction	m_hSignalFunc;

	IScriptSystem *m_pScriptSystem;

	float m_fForwardSpeed;
	float m_fBackwardSpeed;

	SOBJECTSTATE	m_LastObjectState;
	CXGame * m_pGame;

	_SmartScriptObject pSignalTable;

	float m_fThrust;
	float m_fTilt;
	float m_fAngle;
	Vec3d	m_curMoveDir;
	Vec3d	m_prevTagretPos;

	float	m_VertThrust;

	Vec3d	m_Movement;
	Vec3d	m_Direction;
	Vec3d	m_Deviation;
	float	m_DeviationTime;

	bool	m_bTargetInWater;

	//----------------------------------------------------
	//--	for car movemets - to avoid stacking
	float	m_NotMovingTime;
	float	m_ReverseTime;
	Vec3d	m_LastPos;

	CAIHandler	m_AIHandler;

public:
	void DebugDraw(struct IRenderer * pRenderer);
	void Save(CStream &str);
	void Load(CStream &str);
	void Load_PATCH_1(CStream &str){
		Load(str);
	}
};
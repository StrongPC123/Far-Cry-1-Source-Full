
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// ScriptObjectVehicle.h: interface for the CScriptObjectVehicle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTVEHICLE_H__2DADD57E_2A94_4A77_9BF3_D502BC12DA1D__INCLUDED_)
#define AFX_SCRIPTOBJECTVEHICLE_H__2DADD57E_2A94_4A77_9BF3_D502BC12DA1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>
#include "ScriptObjectVector.h"
class CVehicle;

/*! This class implements all vehicle-related script-functions.

	REMARKS:
	After initialization of the script-object it will be accessable to vehicle-entities through scripts using the extension "cnt".
	
	Example:
		local wheelstats = VehicleEntity.cnt.GetWheelStatus( 0 );
*/
class CScriptObjectVehicle : 
	public IScriptObjectSink, 
	public _ScriptableEx<CScriptObjectVehicle>  
{
public:
	CVehicle *GetVehicle();
	void SetVehicle(CVehicle *pPlayer);
	
	CScriptObjectVehicle();
	virtual ~CScriptObjectVehicle();
	bool Create(IScriptSystem *pScriptSystem, IEntitySystem *);


	int SetUser(IFunctionHandler *pH);
	int ReleaseUser(IFunctionHandler *pH);

	//int SetDriver(IFunctionHandler *pH);
	//int ReleaseDriver(IFunctionHandler *pH);

	int GetWheelStatus(IFunctionHandler *pH);
	int GetVehicleVelocity(IFunctionHandler *pH);
	int GetVehicleStatus(IFunctionHandler *pH);
	int HasAccelerated(IFunctionHandler *pH);
	int IsBreaking(IFunctionHandler *pH);
	int HandBreak(IFunctionHandler *pH);
	int	WakeUp(IFunctionHandler *pH);
	int SetDrivingParameters(IFunctionHandler *pH);
	int SetCameraParameters(IFunctionHandler *pH);
	int SetWaterVehicleParameters(IFunctionHandler *pH);
	int SetVehicleEngineHealth(IFunctionHandler *pH);
	int	SetGravity(IFunctionHandler *pH);
	//int Explode(IFunctionHandler *pH);

	int GetVertDeviation(IFunctionHandler *pH);
	int InitLights(IFunctionHandler *pH);
	int	EnableLights(IFunctionHandler *pH);	// enable AI's to use lights

	int SetWeaponName(IFunctionHandler *pH);
	int SetWeaponLimits(IFunctionHandler *pH);
//	int ShakePassengers(IFunctionHandler *pH);
	int AnimateUsers(IFunctionHandler *pH);

	int AnimateMountedWeapon(IFunctionHandler *pH); //!< animate the mounted weapon

//	int SetDamage(IFunctionHandler *pH);

	//IScriptObjectSink
	void OnRelease()
	{
		m_pScriptThis=NULL;
		delete this;
	}
	static void InitializeTemplate(IScriptSystem *pSS);
	static void ReleaseTemplate( );
private:
	CScriptObjectVector m_soSlipVelVec;
	CScriptObjectVector m_soContactPtVec;
	CVehicle *m_pVehicle;
	IEntitySystem *m_pEntitySystem;

	_SmartScriptObject m_GetVehicleStatus;
	_SmartScriptObject m_GetWheelStatus;
};

#endif // !defined(AFX_SCRIPTOBJECTVEHICLE_H__2DADD57E_2A94_4A77_9BF3_D502BC12DA1D__INCLUDED_)

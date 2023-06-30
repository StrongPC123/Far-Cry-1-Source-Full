////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   scriptobjectboids.h
//  Version:     v1.00
//  Created:     17/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __scriptobjectboids_h__
#define __scriptobjectboids_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include <IScriptSystem.h>
#include <_ScriptableEx.h>

// forward declarations.
class CFlockManager;
struct SBoidsCreateContext;

/** Script object providing LUA acess to FlockManager.
*/
class CScriptObjectBoids : public _ScriptableEx<CScriptObjectBoids>
{
public:
	CScriptObjectBoids(void);
	virtual ~CScriptObjectBoids(void);

	void Init(IScriptSystem *pScriptSystem, ISystem *pSystem, CFlockManager *flockMgr);
	
	/** Create new birds flock.
		params: name,pos,params_table
		return: flock_handle
	*/
	int CreateBirdsFlock(IFunctionHandler *pH);

	/** Create new fishes flock.
		params: name,pos,params_table
		return: flock_handle
	*/
	int CreateFishFlock(IFunctionHandler *pH);

	/** Create new flock of bugs.
	params: name,pos,params_table
	return: flock_handle
	*/
	int CreateBugsFlock(IFunctionHandler *pH);

	/** Move flock to a new position.
		params: flock_handle,pos
		return: void
	*/
	int SetFlockPos(IFunctionHandler *pH);

	/** Assign to flock a new name.
		params: flock_handle,name
		return: void
	*/
	int SetFlockName(IFunctionHandler *pH);

	/** Change parameters of flock.
		params: flock_handle,params_table
		return: void
	*/
	int SetFlockParams(IFunctionHandler *pH);

	/** Remove flock.
		params: flock_handle
		return: void
	*/
	int RemoveFlock(IFunctionHandler *pH);

	/** Enables/Disables flock.
	params: flock_handle,bEnable
	return: void
	*/
	int EnableFlock(IFunctionHandler *pH);

	/** Set flock percentage of visibility.
	params: flock_handle,percent (0-100)
	return: void
	*/
	int SetFlockPercentEnabled(IFunctionHandler *pH);

	static void InitializeTemplate(IScriptSystem *pSS);
private:
	bool ReadParamsTable( IScriptObject *pTable, struct SBoidContext &bc,SBoidsCreateContext &ctx );	

	int CommonCreateFlock( IFunctionHandler *pH,int type );

	ISystem *m_pSystem;
	IScriptSystem *m_pScriptSystem;
	CFlockManager *m_flockMgr;
};

#endif // __scriptobjectboids_h__
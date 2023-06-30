////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   scriptobjectboids.cpp
//  Version:     v1.00
//  Created:     17/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "scriptobjectboids.h"

#include "ScriptObjectVector.h"

#include "Flock.h"

_DECLARE_SCRIPTABLEEX(CScriptObjectBoids)

//////////////////////////////////////////////////////////////////////////
CScriptObjectBoids::CScriptObjectBoids(void)
{
}

//////////////////////////////////////////////////////////////////////////
CScriptObjectBoids::~CScriptObjectBoids(void)
{
}

//////////////////////////////////////////////////////////////////////////
void CScriptObjectBoids::Init(IScriptSystem *pScriptSystem, ISystem *pSystem, CFlockManager *flockMgr)
{
	m_pSystem = pSystem;
	m_pScriptSystem = pScriptSystem;
	m_flockMgr = flockMgr;

	InitGlobal(pScriptSystem,"Boids",this);
	
}

void CScriptObjectBoids::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectBoids>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectBoids,CreateBirdsFlock);
	REG_FUNC(CScriptObjectBoids,CreateFishFlock);
	REG_FUNC(CScriptObjectBoids,CreateBugsFlock);
	REG_FUNC(CScriptObjectBoids,SetFlockPos);
	REG_FUNC(CScriptObjectBoids,SetFlockName);
	REG_FUNC(CScriptObjectBoids,SetFlockParams);
	REG_FUNC(CScriptObjectBoids,RemoveFlock );
	REG_FUNC(CScriptObjectBoids,EnableFlock );
	REG_FUNC(CScriptObjectBoids,SetFlockPercentEnabled );
}

int CScriptObjectBoids::CommonCreateFlock( IFunctionHandler *pH,int type )
{
	CHECK_PARAMETERS(4);

	CScriptObjectVector oVec(m_pScriptSystem,true);
	_SmartScriptObject  pParams(m_pScriptSystem,true);

	IEntity *pEntity = 0;
	int nEntityId = 0;

	const char *str;
	int flock_handle = 0;
	int count = 0;
	Vec3d pos;
	string model;

	SBoidContext bc;

	CFlock *flock = m_flockMgr->CreateFlock( (EFlockType)type );
	if (!flock)
	{
		return pH->EndFunction(0);
	}
	flock->GetBoidSettings( bc );
	flock_handle = flock->GetId();

	//////////////////////////////////////////////////////////////////////////
	// 1st param name.
	//////////////////////////////////////////////////////////////////////////
	if(pH->GetParam(1,str))
	{
		flock->SetName( str );
	}
	else
		m_pScriptSystem->RaiseError( "<CreateFlock> parameter 1(name) not specified or nil" );

	//////////////////////////////////////////////////////////////////////////
	// 2nd param position.
	//////////////////////////////////////////////////////////////////////////
	if(pH->GetParam(2,oVec))
	{
		flock->SetPos( oVec.Get() );
	}
	else
		m_pScriptSystem->RaiseError( "<CreateFlock> parameter 2(pos) not specified or nil" );

	//////////////////////////////////////////////////////////////////////////
	// 3rd param params, entity id.
	//////////////////////////////////////////////////////////////////////////
	if(pH->GetParam(3,nEntityId))
	{
		pEntity = m_pSystem->GetIEntitySystem()->GetEntity(nEntityId);
		flock->SetEntity( pEntity );
	}

	//////////////////////////////////////////////////////////////////////////
	// 4rd param params.
	//////////////////////////////////////////////////////////////////////////
	if(pH->GetParam(4,pParams))
	{
		SBoidsCreateContext ctx;
		if (ReadParamsTable( pParams,bc,ctx ))
		{
			bc.entity = pEntity;
			flock->SetBoidSettings( bc );
			flock->CreateBoids( ctx );
		}
	}
	else
		m_pScriptSystem->RaiseError( "<CreateFlock> parameter 3(params table) not specified or nil" );

	// return.
	return pH->EndFunction(flock_handle);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectBoids::CreateBirdsFlock(IFunctionHandler *pH)
{
	return CommonCreateFlock( pH,EFLOCK_BIRDS );
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectBoids::CreateFishFlock(IFunctionHandler *pH)
{
	return CommonCreateFlock( pH,EFLOCK_FISH );
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectBoids::CreateBugsFlock(IFunctionHandler *pH)
{
	return CommonCreateFlock( pH,EFLOCK_BUGS );
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectBoids::SetFlockPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	CScriptObjectVector oVec(m_pScriptSystem,true);

	int flock_handle;
	CFlock *flock;

	//////////////////////////////////////////////////////////////////////////
	// 1st param name.
	//////////////////////////////////////////////////////////////////////////
	if(!pH->GetParam(1,flock_handle))
		m_pScriptSystem->RaiseError( "<SetFlockPos> parameter 1(flock_handle) not specified or nil" );

	flock = m_flockMgr->GetFlock(flock_handle);
	if (!flock)
		return pH->EndFunction();

	//////////////////////////////////////////////////////////////////////////
	// 2nd param position.
	//////////////////////////////////////////////////////////////////////////
	if(pH->GetParam(2,oVec))
	{
		flock->SetPos( oVec.Get() );
	}
	else
		m_pScriptSystem->RaiseError( "<SetFlockPos> parameter 2(pos) not specified or nil" );

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectBoids::SetFlockName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2)

		int flock_handle;
	const char *name;
	CFlock *flock;

	//////////////////////////////////////////////////////////////////////////
	// 1st param name.
	//////////////////////////////////////////////////////////////////////////
	if(!pH->GetParam(1,flock_handle))
		m_pScriptSystem->RaiseError( "<SetFlockName> parameter 1(flock_handle) not specified or nil" );

	flock = m_flockMgr->GetFlock(flock_handle);
	if (!flock)
		return pH->EndFunction();

	//////////////////////////////////////////////////////////////////////////
	// 2nd param position.
	//////////////////////////////////////////////////////////////////////////
	if(pH->GetParam(2,name))
	{
		flock->SetName( name );
	}
	else
		m_pScriptSystem->RaiseError( "<SetFlockName> parameter 2(name) not specified or nil" );

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectBoids::SetFlockParams(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	_SmartScriptObject  pParams(m_pScriptSystem,true);

	int flock_handle;
	CFlock *flock;

	//////////////////////////////////////////////////////////////////////////
	// 1st param name.
	//////////////////////////////////////////////////////////////////////////
	if(!pH->GetParam(1,flock_handle))
		m_pScriptSystem->RaiseError( "<SetFlockParams> parameter 1(flock_handle) not specified or nil" );

	flock = m_flockMgr->GetFlock(flock_handle);
	if (!flock)
		return pH->EndFunction();

	string currModel = flock->GetModelName();
	int currCount = flock->GetBoidsCount();
	SBoidContext bc;
	flock->GetBoidSettings(bc);

	int count = 0;
	string model;

	//////////////////////////////////////////////////////////////////////////
	// 2nd param position.
	//////////////////////////////////////////////////////////////////////////
	if (pH->GetParam(2,pParams))
	{
		SBoidsCreateContext ctx;
		if (ReadParamsTable( pParams,bc,ctx ))
		{
			flock->SetBoidSettings( bc );
			string model = "";
			if (!ctx.models.empty())
				model  = ctx.models[0];
			if ((!model.empty()  && stricmp(model.c_str(),currModel.c_str()) == 0) ||
					(ctx.boidsCount > 0 && ctx.boidsCount != currCount))
			{
				flock->CreateBoids( ctx );
			}
		}
	}
	else
		m_pScriptSystem->RaiseError( "<SetFlockParams> parameter 2(params table) not specified or nil" );

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectBoids::RemoveFlock(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	int flock_handle;
	CFlock *flock;

	//////////////////////////////////////////////////////////////////////////
	// 1st param name.
	//////////////////////////////////////////////////////////////////////////
	if(!pH->GetParam(1,flock_handle))
		m_pScriptSystem->RaiseError( "<RemoveFlock> parameter 1(flock_handle) not specified or nil" );

	flock = m_flockMgr->GetFlock(flock_handle);
	if (!flock)
		return pH->EndFunction();

	m_flockMgr->RemoveFlock( flock );

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectBoids::EnableFlock(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	int flock_handle;
	bool bEnable = true;
	CFlock *flock;

	//////////////////////////////////////////////////////////////////////////
	// 1st param name.
	//////////////////////////////////////////////////////////////////////////
	if(!pH->GetParam(1,flock_handle))
		m_pScriptSystem->RaiseError( "<EnableFlock> parameter 1(flock_handle) not specified or nil" );

	flock = m_flockMgr->GetFlock(flock_handle);
	if (!flock)
		return pH->EndFunction();

	if(!pH->GetParam(2,bEnable))
		m_pScriptSystem->RaiseError( "<EnableFlock> parameter 2 (bEnable) not specified or nil" );

	flock->SetEnabled( bEnable );

	return pH->EndFunction();
}

int CScriptObjectBoids::SetFlockPercentEnabled(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	int flock_handle;
	int percent = 100;
	CFlock *flock;

	//////////////////////////////////////////////////////////////////////////
	// 1st param name.
	//////////////////////////////////////////////////////////////////////////
	if(!pH->GetParam(1,flock_handle))
		m_pScriptSystem->RaiseError( "<SetFlockPercentEnabled> parameter 1(flock_handle) not specified or nil" );

	flock = m_flockMgr->GetFlock(flock_handle);
	if (!flock)
		return pH->EndFunction();

	if(!pH->GetParam(2,percent))
		m_pScriptSystem->RaiseError( "<SetFlockPercentEnabled> parameter 2 (percent) not specified or nil" );

	flock->SetPercentEnabled( percent );

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
bool CScriptObjectBoids::ReadParamsTable(IScriptObject *pTable, struct SBoidContext &bc,SBoidsCreateContext &ctx )
{
	CScriptObjectVector oVec(m_pScriptSystem,true);

	pTable->BeginSetGetChain();
	float fval;
	const char *str;

	/*
	CXmlTemplate::AddParam( m_paramsTemplate,"BirdSize",bc.boidScale );

	CXmlTemplate::AddParam( m_paramsTemplate,"MinHeight",bc.MinHeight );
	CXmlTemplate::AddParam( m_paramsTemplate,"MaxHeight",bc.MaxHeight );
	CXmlTemplate::AddParam( m_paramsTemplate,"MinAttractDist",bc.MinAttractDistance );
	CXmlTemplate::AddParam( m_paramsTemplate,"MaxAttractDist",bc.MaxAttractDistance );

	CXmlTemplate::AddParam( m_paramsTemplate,"MinSpeed",bc.MinSpeed );
	CXmlTemplate::AddParam( m_paramsTemplate,"MaxSpeed",bc.MaxSpeed );

	CXmlTemplate::AddParam( m_paramsTemplate,"FactorAlign",bc.factorAlignment );
	CXmlTemplate::AddParam( m_paramsTemplate,"FactorCohesion",bc.factorCohesion );
	CXmlTemplate::AddParam( m_paramsTemplate,"FactorSeparation",bc.factorSeparation );
	CXmlTemplate::AddParam( m_paramsTemplate,"FactorOrigin",bc.factorAttractToOrigin );
	CXmlTemplate::AddParam( m_paramsTemplate,"FactorHeight",bc.factorKeepHeight );
	CXmlTemplate::AddParam( m_paramsTemplate,"FactorAvoidLand",bc.factorAvoidLand );

	CXmlTemplate::AddParam( m_paramsTemplate,"FovAngle",(float)acos(bc.cosFovAngle)/PI*180.0f );
	CXmlTemplate::AddParam( m_paramsTemplate,"MaxAnimSpeed",bc.MaxAnimationSpeed );

	CXmlTemplate::AddParam( m_paramsTemplate,"FollowPlayer",bc.followPlayer );
	CXmlTemplate::AddParam( m_paramsTemplate,"NoLanding",bc.noLanding );

	CXmlTemplate::AddParam( m_paramsTemplate,"AvoidObstacles",bc.avoidObstacles );
	CXmlTemplate::AddParam( m_paramsTemplate,"MaxViewDistance",bc.maxVisibleDistance );
	*/
	
	ctx.models.clear();
	ctx.boidsCount = 0;
	pTable->GetValueChain( "count",ctx.boidsCount );
	if (pTable->GetValueChain( "model",str ))
	{
		ctx.models.push_back(str);
	}
	if (pTable->GetValueChain( "model1",str ))
	{
		if (strlen(str) > 0)
			ctx.models.push_back(str);
	}
	if (pTable->GetValueChain( "model2",str ))
	{
		if (strlen(str) > 0)
			ctx.models.push_back(str);
	}
	if (pTable->GetValueChain( "model3",str ))
	{
		if (strlen(str) > 0)
			ctx.models.push_back(str);
	}
	if (pTable->GetValueChain( "model4",str ))
	{
		if (strlen(str) > 0)
			ctx.models.push_back(str);
	}
	if (pTable->GetValueChain( "character",str ))
	{
		ctx.characterModel = str;
	}
	if (pTable->GetValueChain( "animation",str ))
	{
		ctx.animation = str;
	}

	pTable->GetValueChain( "behavior",bc.behavior );

	pTable->GetValueChain( "boid_mass",bc.fBoidMass);

	pTable->GetValueChain( "boid_size",bc.boidScale );
	pTable->GetValueChain( "min_height",bc.MinHeight );
	pTable->GetValueChain( "max_height",bc.MaxHeight );
	pTable->GetValueChain( "min_attract_distance",bc.MinAttractDistance );
	pTable->GetValueChain( "max_attract_distance",bc.MaxAttractDistance );
	pTable->GetValueChain( "min_speed",bc.MinSpeed );
	pTable->GetValueChain( "max_speed",bc.MaxSpeed );

	pTable->GetValueChain( "factor_align",bc.factorAlignment );
	pTable->GetValueChain( "factor_cohesion",bc.factorCohesion );
	pTable->GetValueChain( "factor_separation",bc.factorSeparation );
	pTable->GetValueChain( "factor_origin",bc.factorAttractToOrigin );
	pTable->GetValueChain( "factor_keep_height",bc.factorKeepHeight );
	pTable->GetValueChain( "factor_avoid_land",bc.factorAvoidLand );

	pTable->GetValueChain( "max_anim_speed",bc.MaxAnimationSpeed );
	pTable->GetValueChain( "follow_player",bc.followPlayer );
	pTable->GetValueChain( "no_landing",bc.noLanding );
	pTable->GetValueChain( "avoid_obstacles",bc.avoidObstacles );
	pTable->GetValueChain( "max_view_distance",bc.maxVisibleDistance );

	pTable->GetValueChain( "spawn_radius",bc.fSpawnRadius);
	//pTable->GetValueChain( "boid_radius",bc.fBoidRadius);
	pTable->GetValueChain( "gravity_at_death",bc.fGravity);
	pTable->GetValueChain( "boid_mass",bc.fBoidMass);

	if (pTable->GetValueChain( "fov_angle",fval ))
		bc.cosFovAngle = cry_cosf(fval*gf_PI/180.0f);

	pTable->EndSetGetChain();

	return true;
}
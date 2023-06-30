////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   ScriptVarNode.cpp
//  Version:     v1.00
//  Created:     11/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ScriptVarNode.h"
#include "AnimTrack.h"

#include <ISystem.h>
#include <IScriptSystem.h>

//////////////////////////////////////////////////////////////////////////
CAnimScriptVarNode::CAnimScriptVarNode( IMovieSystem *sys )
: CAnimNode(sys)
{
	SetFlags( GetFlags()|ANODE_FLAG_CAN_CHANGE_NAME );
	m_dwSupportedTracks = PARAM_BIT(APARAM_FLOAT_1);
	m_value = -1e-20f;
}

//////////////////////////////////////////////////////////////////////////
void CAnimScriptVarNode::CreateDefaultTracks()
{
	CreateTrack(APARAM_FLOAT_1);
};

//////////////////////////////////////////////////////////////////////////
int CAnimScriptVarNode::GetParamCount() const
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimScriptVarNode::GetParamInfo( int nIndex, SParamInfo &info ) const
{
	if (nIndex == 0)
	{
		info.flags = 0;
		info.name = "Value";
		info.paramId = APARAM_FLOAT_1;
		info.valueType = AVALUE_FLOAT;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimScriptVarNode::GetParamInfoFromId( int paramId, SParamInfo &info ) const
{
	if (paramId == APARAM_FLOAT_1)
	{
		GetParamInfo( 0,info );
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
void CAnimScriptVarNode::Animate( SAnimContext &ec )
{
	IAnimBlock *anim = GetAnimBlock();
	if (!anim)
		return;
	
	float value = m_value;

	IAnimTrack *pValueTrack = anim->GetTrack(APARAM_FLOAT_1);
	if (pValueTrack)
	{
		pValueTrack->GetValue(ec.time, value);
	}

	if (value != m_value)
	{
		m_value = value;
		// Change console var value.
		SetScriptValue();
	}
}

void CAnimScriptVarNode::SetScriptValue()
{
	const char *sVarName = GetName();
	const char *sPnt = strchr(sVarName,'.');
	if (sPnt == 0)
	{
		// Global variable.
		IScriptSystem *pScriptSystem = m_pMovieSystem->GetSystem()->GetIScriptSystem();
		pScriptSystem->SetGlobalValue( sVarName,m_value );
	}
	else
	{
		char sTable[256];
		char sName[256];
		strcpy( sTable,sVarName );
		sTable[sPnt-sVarName] = 0;
		strcpy( sName,sPnt+1 );

		// In Table value.
		IScriptSystem *pScriptSystem = m_pMovieSystem->GetSystem()->GetIScriptSystem();
		_SmartScriptObject pTable(pScriptSystem,true);
		if (pScriptSystem->GetGlobalValue( sTable,pTable ))
		{
			// Set float value inside table.
			pTable->SetValue( sName,m_value );
		}
	}
}

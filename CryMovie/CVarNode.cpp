////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   CVarNode.cpp
//  Version:     v1.00
//  Created:     10/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CVarNode.h"
#include "AnimTrack.h"

#include <ISystem.h>
#include <IConsole.h>

//////////////////////////////////////////////////////////////////////////
CAnimCVarNode::CAnimCVarNode( IMovieSystem *sys )
: CAnimNode(sys)
{
	SetFlags( GetFlags()|ANODE_FLAG_CAN_CHANGE_NAME );
	m_dwSupportedTracks = PARAM_BIT(APARAM_FLOAT_1);
	m_value = 0;
}

void CAnimCVarNode::CreateDefaultTracks()
{
	CreateTrack(APARAM_FLOAT_1);
}

//////////////////////////////////////////////////////////////////////////
int CAnimCVarNode::GetParamCount() const
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimCVarNode::GetParamInfo( int nIndex, SParamInfo &info ) const
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
bool CAnimCVarNode::GetParamInfoFromId( int paramId, SParamInfo &info ) const
{
	if (paramId == APARAM_FLOAT_1)
	{
		GetParamInfo( 0,info );
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CAnimCVarNode::SetName( const char *name )
{
	// Name of node is used as a name of console var.
	CAnimNode::SetName(name);
	ICVar *pVar = m_pMovieSystem->GetSystem()->GetIConsole()->GetCVar( GetName(),false );
	if (pVar)
	{
		m_value = pVar->GetFVal();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimCVarNode::Animate( SAnimContext &ec )
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
		ICVar *pVar = m_pMovieSystem->GetSystem()->GetIConsole()->GetCVar( GetName(),false );
		if (pVar)
		{
			pVar->Set( m_value );
		}
	}
}
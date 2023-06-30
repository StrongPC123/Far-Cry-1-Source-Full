////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   ScriptVarNode.h
//  Version:     v1.00
//  Created:     11/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ScriptVarNode_h__
#define __ScriptVarNode_h__
#pragma once

#include "AnimNode.h"

class CAnimScriptVarNode : public CAnimNode
{
public:
	CAnimScriptVarNode( IMovieSystem *sys );

	virtual EAnimNodeType GetType() const { return ANODE_SCRIPTVAR; }

	//////////////////////////////////////////////////////////////////////////
	// Overrides from CAnimNode
	//////////////////////////////////////////////////////////////////////////
	void Animate( SAnimContext &ec );
	void CreateDefaultTracks();

	//////////////////////////////////////////////////////////////////////////
	virtual int GetParamCount() const;
	virtual bool GetParamInfo( int nIndex, SParamInfo &info ) const;
	virtual bool GetParamInfoFromId( int paramId, SParamInfo &info ) const;

private:
	void SetScriptValue();
	float m_value;
};

#endif // __ScriptVarNode_h__
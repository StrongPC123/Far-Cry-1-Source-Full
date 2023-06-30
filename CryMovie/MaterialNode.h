////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   MaterialNode.h
//  Version:     v1.00
//  Created:     11/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MaterialNode_h__
#define __MaterialNode_h__
#pragma once

#include "AnimNode.h"

class CAnimMaterialNode : public CAnimNode
{
public:
	CAnimMaterialNode( IMovieSystem *sys );

	virtual EAnimNodeType GetType() const { return ANODE_MATERIAL; }

	//////////////////////////////////////////////////////////////////////////
	// Overrides from CAnimNode
	//////////////////////////////////////////////////////////////////////////
	void Animate( SAnimContext &ec );

	//////////////////////////////////////////////////////////////////////////
	// Supported tracks description.
	//////////////////////////////////////////////////////////////////////////
	virtual int GetParamCount() const;
	virtual bool GetParamInfo( int nIndex, SParamInfo &info ) const;
	virtual bool GetParamInfoFromId( int paramId, SParamInfo &info ) const;

private:
	void SetScriptValue();
};

#endif // __MaterialNode_h__
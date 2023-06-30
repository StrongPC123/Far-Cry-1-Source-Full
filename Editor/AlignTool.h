////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   aligntool.h
//  Version:     v1.00
//  Created:     13/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __aligntool_h__
#define __aligntool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"
#include "Objects\ObjectManager.h"

//////////////////////////////////////////////////////////////////////////
class CAlignPickCallback : public IPickObjectCallback
{
public:
	CAlignPickCallback() { m_bActive = true; };
	//! Called when object picked.
	virtual void OnPick( CBaseObject *picked );
	//! Called when pick mode cancelled.
	virtual void OnCancelPick();
	//! Return true if specified object is pickable.
	virtual bool OnPickFilter( CBaseObject *filterObject );

	static bool IsActive() { return m_bActive; }
private:
	static bool m_bActive;
};


#endif // __aligntool_h__

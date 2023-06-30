////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   gizmomanager.h
//  Version:     v1.00
//  Created:     2/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __gizmomanager_h__
#define __gizmomanager_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IGizmoManager.h"
#include "Gizmo.h"

/** GizmoManager manages set of currently active Gizmo objects.
*/
class CGizmoManager : public IGizmoManager
{
public:
	void AddGizmo( CGizmo *gizmo );
	void RemoveGizmo( CGizmo *gizmo );

	void Display( DisplayContext &dc );
	bool HitTest( HitContext &hc );

private:
	typedef std::set<CGizmoPtr> Gizmos;
	Gizmos m_gizmos;
};

#endif // __gizmomanager_h__

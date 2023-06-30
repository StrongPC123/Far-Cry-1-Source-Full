////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   IGizmoManager.h
//  Version:     v1.00
//  Created:     4/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IGizmoManager_h__
#define __IGizmoManager_h__
#pragma once

class CGizmo;
struct DisplayContext;
struct HitContext;

/** GizmoManager manages set of currently active Gizmo objects.
*/
struct IGizmoManager
{
	virtual void AddGizmo( CGizmo *gizmo ) = 0;
	virtual void RemoveGizmo( CGizmo *gizmo ) = 0;

	virtual void Display( DisplayContext &dc ) = 0;
	virtual bool HitTest( HitContext &hc ) = 0;
};

#endif // __IGizmoManager_h__

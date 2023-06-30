////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   gizmomanager.cpp
//  Version:     v1.00
//  Created:     2/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GizmoManager.h"
#include "DisplayContext.h"
#include "ObjectManager.h"

//////////////////////////////////////////////////////////////////////////
void CGizmoManager::Display( DisplayContext &dc )
{
	FUNCTION_PROFILER( GetIEditor()->GetSystem(),PROFILE_EDITOR );

	BBox bbox;
	std::vector<CGizmo*> todelete;
	for (Gizmos::iterator it = m_gizmos.begin(); it != m_gizmos.end(); ++it)
	{
		CGizmo *gizmo = *it;
		if (gizmo->GetFlags() & EGIZMO_HIDDEN)
			continue;

		gizmo->GetWorldBounds( bbox );
		if (dc.IsVisible(bbox))
		{
			gizmo->Display( dc );
		}

		if (gizmo->IsDelete())
			todelete.push_back(gizmo);
	}

	// Delete gizmos that needs deletion.
	for (int i = 0; i < todelete.size(); i++)
	{
		RemoveGizmo(todelete[i]);
	}
}

//////////////////////////////////////////////////////////////////////////
void CGizmoManager::AddGizmo( CGizmo *gizmo )
{
	m_gizmos.insert( gizmo );
}

//////////////////////////////////////////////////////////////////////////
void CGizmoManager::RemoveGizmo( CGizmo *gizmo )
{
	m_gizmos.erase( gizmo );
}

//////////////////////////////////////////////////////////////////////////
bool CGizmoManager::HitTest( HitContext &hc )
{
	float mindist = FLT_MAX;

	HitContext ghc = hc;

	BBox bbox;
	for (Gizmos::iterator it = m_gizmos.begin(); it != m_gizmos.end(); ++it)
	{
		CGizmo *gizmo = *it;

		if (gizmo->GetFlags() & EGIZMO_SELECTABLE)
		{
			if (gizmo->HitTest( ghc ))
			{
				if (ghc.dist < mindist)
				{
					mindist = ghc.dist;
					hc = ghc;
				}
			}
		}
	}
	return hc.object != 0;
};
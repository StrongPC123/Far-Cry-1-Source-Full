////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   PickObjectTool.cpp
//  Version:     v1.00
//  Created:     18/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PickObjectTool.h"
#include "Viewport.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CPickObjectTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CPickObjectTool::CPickObjectTool( IPickObjectCallback *callback,CRuntimeClass *targetClass )
{
	assert( callback != 0 );
	m_callback = callback;
	m_targetClass = targetClass;
	m_bMultiPick = false;
}

//////////////////////////////////////////////////////////////////////////
CPickObjectTool::~CPickObjectTool()
{
	GetIEditor()->GetObjectManager()->SetSelectCallback( 0 );
	//m_prevSelectCallback = 0;
	if (m_callback)
		m_callback->OnCancelPick();
}

//////////////////////////////////////////////////////////////////////////
void CPickObjectTool::BeginEditParams( IEditor *ie,int flags )
{
	CString str = "Pick object";
	if (m_targetClass)
	{
		str.Format( "Pick %s object",m_targetClass->m_lpszClassName );
	}
	SetStatusText( str );

	//m_prevSelectCallback = 
	GetIEditor()->GetObjectManager()->SetSelectCallback( this );
}

//////////////////////////////////////////////////////////////////////////
bool CPickObjectTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (event == eMouseLDown)
	{
		Vec3d src,dir;
		view->ViewToWorldRay( point,src,dir );
		dir = GetNormalized(dir);
		ObjectHitInfo hitInfo(view,point);
		GetIEditor()->GetObjectManager()->HitTest( src,dir,0,hitInfo );
		CBaseObject *obj = hitInfo.object;
		if (obj)
		{
			if (IsRelevant(obj))
			{
				if (m_callback)
				{
					// Can pick this one.
					m_callback->OnPick( obj );
				}
				if (!m_bMultiPick)
				{
					m_callback = 0;
					GetIEditor()->SetEditTool(0);
				}
			}
		}
	}
	else if (event == eMouseMove)
	{
		Vec3d src,dir;
		view->ViewToWorldRay( point,src,dir );
		dir = GetNormalized(dir);
		ObjectHitInfo hitInfo(view,point);
		GetIEditor()->GetObjectManager()->HitTest( src,dir,0,hitInfo );
		CBaseObject *obj = hitInfo.object;
		if (obj)
		{
			if (IsRelevant(obj))
			{
				// Set Cursors.
				view->SetObjectCursor(obj);
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CPickObjectTool::OnSelectObject( CBaseObject *obj )
{
	if (IsRelevant(obj))
	{
		// Can pick this one.
		if (m_callback)
		{
			m_callback->OnPick( obj );
			m_callback = 0;
		}
		if (!m_bMultiPick)
		{
			GetIEditor()->SetEditTool(0);
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CPickObjectTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{ 
	if (nChar == VK_ESCAPE)
	{
		// Cancel selection.
		GetIEditor()->SetEditTool(0);
	}
	return false; 
}

//////////////////////////////////////////////////////////////////////////
bool CPickObjectTool::IsRelevant( CBaseObject *obj )
{
	assert( obj != 0 );
	if (!m_callback)
		return false;

	if (!m_targetClass)
	{
		return m_callback->OnPickFilter(obj);
	}
	else
	{
		if (obj->GetRuntimeClass() == m_targetClass || obj->GetRuntimeClass()->IsDerivedFrom(m_targetClass))
		{
			return m_callback->OnPickFilter(obj);
		}
	}
	return false;
}
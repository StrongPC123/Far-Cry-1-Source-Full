////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   aligntool.cpp
//  Version:     v1.00
//  Created:     13/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////


#include "StdAfx.h"
#include "AlignTool.h"

#include "Objects\BaseObject.h"

//////////////////////////////////////////////////////////////////////////
bool CAlignPickCallback::m_bActive = false;

//////////////////////////////////////////////////////////////////////////
//! Called when object picked.
void CAlignPickCallback::OnPick( CBaseObject *picked )
{
	Matrix44 tm;
	tm = picked->GetWorldTM();

	{
		bool bUndo = !CUndo::IsRecording();
		if (bUndo)
			GetIEditor()->BeginUndo();
		
		CSelectionGroup *selGroup = GetIEditor()->GetSelection();
		selGroup->FilterParents();

		for (int i = 0; i < selGroup->GetFilteredCount(); i++)
		{
			selGroup->GetFilteredObject(i)->SetWorldTM(tm);
		}
		m_bActive = false;
		if (bUndo)
			GetIEditor()->AcceptUndo( "Align To Object");
	}
	delete this;
}

//! Called when pick mode cancelled.
void CAlignPickCallback::OnCancelPick()
{
	m_bActive = false;
	delete this;
}

	//! Return true if specified object is pickable.
bool CAlignPickCallback::OnPickFilter( CBaseObject *filterObject )
{
	return true;
};
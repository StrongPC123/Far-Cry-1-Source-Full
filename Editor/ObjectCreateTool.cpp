////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ObjectCreateTool.cpp
//  Version:     v1.00
//  Created:     18/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ObjectCreateTool.h"
#include "ObjectTypeBrowser.h"
#include "PanelTreeBrowser.h"
#include "Viewport.h"
#include "DisplaySettings.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CObjectCreateTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CObjectCreateTool::CObjectCreateTool( CreateCallback createCallback )
{
	SetStatusText( "Drag&Drop item to create an object" );

	m_hCreateCursor = AfxGetApp()->LoadCursor( IDC_POINTER_OBJHIT );

	m_createCallback = createCallback;
	m_object = 0;
	m_objectBrowserPanelId = 0;
	m_fileBrowserPanelId = 0;
	
	if (!m_createCallback)
		GetIEditor()->ClearSelection();
}

//////////////////////////////////////////////////////////////////////////
CObjectCreateTool::~CObjectCreateTool()
{
	if (GetIEditor()->IsUndoRecording())
		GetIEditor()->CancelUndo();
}

//////////////////////////////////////////////////////////////////////////
void CObjectCreateTool::CloseFileBrowser()
{
	if (m_fileBrowserPanelId)
	{
		GetIEditor()->RemoveRollUpPage(ROLLUP_OBJECTS,m_fileBrowserPanelId);
		m_fileBrowserPanelId = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectCreateTool::SelectCategory( const CString &category )
{
	// Check if this category have more then one subtype.
	std::vector<CString> types;
	GetIEditor()->GetObjectManager()->GetClassTypes( category,types );
	if (types.size() == 1)
	{
		// If only one or less sub types in this category, assume it type itsel, and start creation.
		StartCreation( types[0] );
		return;
	}

	// Check if this category have more then one subtype.
	ObjectTypeBrowser* panel = new ObjectTypeBrowser( AfxGetMainWnd() );
	m_objectBrowserPanelId = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,"Object Type",panel );
	panel->SetCategory( this,category );
	AfxGetMainWnd()->SetFocus();
}

//////////////////////////////////////////////////////////////////////////
void CObjectCreateTool::StartCreation( const CString &type,const CString &param )
{
	// Delete object currently in creation.
	if (GetIEditor()->IsUndoRecording())
		GetIEditor()->CancelUndo();

	m_objectType = type;

	CObjectClassDesc *clsDesc = GetIEditor()->GetObjectManager()->FindClass( type );
	if (!clsDesc)
	{
		MessageBox( NULL,"Object creation failed, unknown object type.", "Warning", MB_ICONEXCLAMATION|MB_OK );
		return;
	}
	if (param.IsEmpty())
	{
		CString fileSpec = clsDesc->GetFileSpec();
		if (!fileSpec.IsEmpty())
		{
			//! Check if file spec contain wildcards.
			if (fileSpec.Find("*") >= 0)
			{
				// Create file broswer panel.
				// When file is selected OnSelectFile callback will be called and creation process will be finalized.
				CPanelTreeBrowser *br = new CPanelTreeBrowser;
				br->Create( functor(*this,&CObjectCreateTool::OnSelectFile),clsDesc->GetFileSpec(),AfxGetMainWnd() );
				m_fileBrowserPanelId = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,"Browser",br );
				br->AddPreviewPanel();
				AfxGetMainWnd()->SetFocus();
				return;
			}
		}
		OnSelectFile( fileSpec );
	}
	else
	{
		OnSelectFile( param );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectCreateTool::OnSelectFile( CString file )
{
	if (m_objectType.IsEmpty())
	{
		CancelCreation();
		return;
	}
	CWaitCursor wait;
	GetIEditor()->BeginUndo();
	m_object = GetIEditor()->NewObject( m_objectType,file );
	if (m_object)
	{
		// Close file browser if was open, not needed anymore.
		CloseFileBrowser();

		// if this object type was hidden by category, re-display it.
		int hideMask = GetIEditor()->GetDisplaySettings()->GetObjectHideMask();
		hideMask = hideMask & ~(m_object->GetType());
		GetIEditor()->GetDisplaySettings()->SetObjectHideMask( hideMask );

		// Enable display of current layer.
		CObjectLayer *pLayer = GetIEditor()->GetObjectManager()->GetLayersManager()->GetCurrentLayer();
		pLayer->SetFrozen(false);
		pLayer->SetVisible(true);

		if (!m_createCallback)
			GetIEditor()->GetObjectManager()->BeginEditParams( m_object,OBJECT_CREATE );
		// Document modified.
		GetIEditor()->SetModifiedFlag();
	}

	if (m_createCallback)
		m_createCallback( this,m_object );
}

//////////////////////////////////////////////////////////////////////////
void CObjectCreateTool::CancelCreation()
{
	// Make sure created object is unselected.
	if (m_object)
	{
		// Destroy ourself.
		GetIEditor()->SetEditTool(0);
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectCreateTool::AcceptCreation()
{
	// Make sure created object is unselected.
	if (m_object)
	{
		if (GetIEditor()->IsUndoRecording())
			GetIEditor()->AcceptUndo( CString("New ")+m_object->GetTypeName() );
		GetIEditor()->SetEditTool(0);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CObjectCreateTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (m_object)
	{
		GetIEditor()->SuspendUndo();
		int res = m_object->MouseCreateCallback( view,event,point,flags );
		GetIEditor()->ResumeUndo();
		if (res == MOUSECREATE_ABORT)
		{
			// Cancel object creation.
			CancelCreation();
		}
		else if (res == MOUSECREATE_OK)
		{
			// Accept this object, abort edit tool.
			AcceptCreation();
			/*
			// Accept this object, create a new clone object for editing.
			m_object = GetIEditor()->GetObjectManager()->NewObject( m_object->GetClassDesc(),m_object );
			if (m_object)
				GetIEditor()->GetObjectManager()->BeginEditParams( m_object,OBJECT_CREATE );
				*/
		}
	}
	//GetIEditor()->SetStatusText( "Drag item from rollup panel to create object." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CObjectCreateTool::BeginEditParams( IEditor *ie,int flags )
{
}

//////////////////////////////////////////////////////////////////////////
void CObjectCreateTool::EndEditParams()
{
	if (m_objectBrowserPanelId)
	{
		GetIEditor()->RemoveRollUpPage(ROLLUP_OBJECTS,m_objectBrowserPanelId);
		m_objectBrowserPanelId = 0;
	}
	CloseFileBrowser();
}

//////////////////////////////////////////////////////////////////////////
bool CObjectCreateTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{ 
	if (nChar == VK_ESCAPE || nChar == VK_DELETE)
	{
		CancelCreation();
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CObjectCreateTool::OnSetCursor( CViewport *vp )
{
	SetCursor( m_hCreateCursor );
	return true;
}
// ViewManager.cpp: implementation of the CViewManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ViewManager.h"
#include "MainFrm.h"
#include "LayoutWnd.h"
#include "Viewport.h"

#include "2DViewport.h"
#include "TopRendererWnd.h"
#include "RenderViewport.h"
#include "ModelViewport.h"
#include "ZViewport.h"

#include "Settings.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CViewManager::CViewManager()
{
	gSettings.pGrid = &m_grid;

	m_zoomFactor = 1;
	
	m_origin2D(0,0,0);
	m_zoom2D = 1.0f;

	m_cameraObjectId = GUID_NULL;

	m_pPerspectiveViewport = 0;

	m_updateRegion.min = Vec3(-100000,-100000,-100000);
	m_updateRegion.max = Vec3(100000,100000,100000);

	// Initialize viewport descriptions.
	RegisterViewportDesc( new CViewportDesc(ET_ViewportXY,_T("Top"),RUNTIME_CLASS(C2DViewport)) );
	RegisterViewportDesc( new CViewportDesc(ET_ViewportXZ,_T("Front"),RUNTIME_CLASS(C2DViewport)) );
	RegisterViewportDesc( new CViewportDesc(ET_ViewportYZ,_T("Left"),RUNTIME_CLASS(C2DViewport)) );
	RegisterViewportDesc( new CViewportDesc(ET_ViewportCamera,_T("Perspective"),RUNTIME_CLASS(CRenderViewport)) );
	RegisterViewportDesc( new CViewportDesc(ET_ViewportMap,_T("Map"),RUNTIME_CLASS(CTopRendererWnd)) );
	RegisterViewportDesc( new CViewportDesc(ET_ViewportModel,_T("ModelPreview"),RUNTIME_CLASS(CModelViewport)) );
	RegisterViewportDesc( new CViewportDesc(ET_ViewportZ,_T("Z View"),RUNTIME_CLASS(CZViewport)) );
}

//////////////////////////////////////////////////////////////////////////
CViewManager::~CViewManager()
{
	// Delete all registered views.
	for (int i = 0; i < m_viewports.size(); i++)
	{
		delete m_viewports[i];
	}
	m_viewports.clear();
	m_viewportDesc.clear();
}

//////////////////////////////////////////////////////////////////////////
void CViewManager::RegisterViewportDesc( CViewportDesc *desc )
{
	m_viewportDesc.push_back(desc);
}

//////////////////////////////////////////////////////////////////////////
CViewport* CViewManager::CreateView( EViewportType type,CWnd *pParentWnd )
{
	if (type == ET_ViewportCamera)
	{
		if (m_pPerspectiveViewport)
			return m_pPerspectiveViewport;
	}
	CViewportDesc *vd = 0;
	for (int i = 0; i < m_viewportDesc.size(); i++)
	{
		if (m_viewportDesc[i]->type == type)
		{
			vd = m_viewportDesc[i];
		}
	}

	if (!vd)
		return 0;

	// Create CViewport derived class.
	CViewport *pViewport = (CViewport*)vd->rtClass->CreateObject();
	if (!pViewport)
		return 0;

	pViewport->SetViewManager(this);
	pViewport->SetType( type );
	pViewport->Create( pParentWnd,type,vd->name );

	m_viewports.push_back( pViewport );

	if (type == ET_ViewportCamera)
		m_pPerspectiveViewport = pViewport;

	return pViewport;
}

//////////////////////////////////////////////////////////////////////////
void CViewManager::ReleaseView( CViewport *pViewport )
{
	// Only 1 perspective view is allowed.
	if (pViewport != m_pPerspectiveViewport)
	{
		stl::find_and_erase( m_viewports,pViewport );
		pViewport->DestroyWindow();
		delete pViewport;
	}
}

//////////////////////////////////////////////////////////////////////////
CViewport* CViewManager::GetViewport( EViewportType type ) const
{
	////////////////////////////////////////////////////////////////////////
	// Returns the first view which has a render window of a specific
	// type attached
	////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < m_viewports.size(); i++)
	{
		if (m_viewports[i]->GetType() == type)
			return m_viewports[i];
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CViewport* CViewManager::GetViewport( const CString &name ) const
{
	for (int i = 0; i < m_viewports.size(); i++)
	{
		if (stricmp(m_viewports[i]->GetName(),name) == 0)
			return m_viewports[i];
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CViewport* CViewManager::GetActiveViewport() const
{
	for (int i = 0; i < m_viewports.size(); i++)
	{
		if (m_viewports[i]->IsActive())
			return m_viewports[i];
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CViewManager::SetZoomFactor( float zoom )
{
	m_zoomFactor = zoom;
}

//////////////////////////////////////////////////////////////////////////
void CViewManager::UpdateViews( int flags )
{
	// Update each attached view,
	for (int i = 0; i < m_viewports.size(); i++)
	{
		m_viewports[i]->UpdateContent( flags );
	}
}

//////////////////////////////////////////////////////////////////////////
void CViewManager::ResetViews()
{
	// Reset each attached view,
	for (int i = 0; i < m_viewports.size(); i++)
	{
		m_viewports[i]->ResetContent();
	}
}

//////////////////////////////////////////////////////////////////////////
void CViewManager::Update()
{
	// Update each attached view,
	for (int i = 0; i < m_viewports.size(); i++)
	{
		m_viewports[i]->Update();
	}
}

//////////////////////////////////////////////////////////////////////////
void	CViewManager::SetAxisConstrain( int axis )
{
	for (int i = 0; i < m_viewports.size(); i++)
	{
		m_viewports[i]->SetAxisConstrain(axis);
	}
}

//////////////////////////////////////////////////////////////////////////
CLayoutWnd* CViewManager::GetLayout() const
{
	CMainFrame *pMainFrame = ((CMainFrame*)AfxGetMainWnd());
	if (pMainFrame)
    return pMainFrame->GetLayout();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CViewManager::GetViewportDescriptions( std::vector<CViewportDesc*> &descriptions )
{
	descriptions.clear();
	for (int i = 0; i < m_viewportDesc.size(); i++)
	{
		descriptions.push_back( m_viewportDesc[i] );
	}
}

//////////////////////////////////////////////////////////////////////////
void CViewManager::Cycle2DViewport()
{
	GetLayout()->Cycle2DViewport();
}

//////////////////////////////////////////////////////////////////////////
CViewport* CViewManager::GetViewportAtPoint( CPoint point ) const
{
	CWnd *wnd = CWnd::WindowFromPoint( point );

	for (int i = 0; i < m_viewports.size(); i++)
	{
		if (m_viewports[i] == wnd)
		{
			return m_viewports[i];
		}
	}
	return 0;
}
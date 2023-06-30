// PropertiesPanel.cpp : implementation file
//

#include "stdafx.h"
#include "PropertiesPanel.h"

#include "Objects\\BaseObject.h"
#include "Objects\\SelectionGroup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define HEIGHT_OFFSET 4
#define HEIGHT_ADD 4

/////////////////////////////////////////////////////////////////////////////
// CPropertiesPanel dialog


CPropertiesPanel::CPropertiesPanel( CWnd* pParent /*=NULL*/)
	: CDialog(CPropertiesPanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPropertiesPanel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_multiSelect = false;

	Create( IDD,pParent );
}


void CPropertiesPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertiesPanel)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertiesPanel, CDialog)
	//{{AFX_MSG_MAP(CPropertiesPanel)
	ON_WM_DESTROY()
	ON_WM_KILLFOCUS()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertiesPanel message handlers

BOOL CPropertiesPanel::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect rc;
	GetClientRect(rc);
	m_wndProps.Create( WS_CHILD|WS_VISIBLE,rc,this );
	m_wndProps.ModifyStyleEx( 0,WS_EX_CLIENTEDGE );
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPropertiesPanel::SetMultiSelect( bool bEnable )
{
	m_multiSelect = bEnable;
	if (m_wndProps.m_hWnd)
	{
		m_wndProps.SetDisplayOnlyModified( bEnable );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPropertiesPanel::DeleteVars()
{
	if (!m_wndProps.m_hWnd)
		return;
	m_wndProps.DeleteAllItems();
	m_updateCallbacks.clear();
	m_varBlock = 0;
}

//////////////////////////////////////////////////////////////////////////
void CPropertiesPanel::AddVars( CVarBlock *vb,const UpdateCallback &updCallback )
{
	assert(vb);

	if (!m_wndProps.m_hWnd)
		return;
	//m_wndProps.DeleteAllItems();

	bool bNewBlock = false;
	// Make a clone of properties.
	if (!m_varBlock)
	{
		// Must first clear any selection in properties window.
		m_wndProps.ClearSelection();
		m_wndProps.DeleteAllItems();
		m_varBlock = vb->Clone(true);
		m_wndProps.AddVarBlock( m_varBlock );
		bNewBlock = true;
	}
	m_varBlock->Wire( vb );
	//CVarBlock *propVB = m_varBlock->Clone(true);
	//propVB->Wire( m_varBlock );
	//m_wndProps.AddVarBlock( propVB );

	if (bNewBlock)
	{
		m_wndProps.SetUpdateCallback( functor(*this,&CPropertiesPanel::OnPropertyChanged) );
		m_wndProps.ExpandAll();

		// Resize to fit properties.
		CRect rc;
		GetClientRect( rc );
		int h = m_wndProps.GetVisibleHeight() + HEIGHT_ADD;
		if (h > 400)
			h = 400;
		SetWindowPos( NULL,0,0,rc.right,h+HEIGHT_OFFSET*2+4,SWP_NOMOVE );

		m_multiSelect = false;
		m_wndProps.SetDisplayOnlyModified( false );

		// When new object set all previous callbacks freed.
		m_updateCallbacks.clear();
	}
	else
	{
		m_multiSelect = true;
		m_wndProps.SetDisplayOnlyModified( true );
	}

	if (updCallback)
		stl::push_back_unique( m_updateCallbacks,updCallback );
}

void CPropertiesPanel::ReloadValues()
{
	if (m_wndProps.m_hWnd)
	{
		m_wndProps.ReloadValues();
	}
}

void CPropertiesPanel::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CPropertiesPanel::OnKillFocus(CWnd* pNewWnd) 
{
	CDialog::OnKillFocus(pNewWnd);	
}

void CPropertiesPanel::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if (m_wndProps.m_hWnd)
	{
		int h = m_wndProps.GetVisibleHeight() + HEIGHT_ADD;
		CRect rc( 2,HEIGHT_OFFSET,cx-2,cy - HEIGHT_OFFSET*2 + 4 );
		m_wndProps.MoveWindow( rc,TRUE );
	}
}


//////////////////////////////////////////////////////////////////////////
void CPropertiesPanel::OnPropertyChanged( XmlNodeRef node )
{
	std::list<UpdateCallback>::iterator iter;
	for (iter = m_updateCallbacks.begin(); iter != m_updateCallbacks.end(); ++iter)
	{
		(*iter)();
	}
}

void CPropertiesPanel::OnLButtonDown(UINT nFlags, CPoint point)
{
	//ReloadValues();

	CDialog::OnLButtonDown(nFlags, point);
}
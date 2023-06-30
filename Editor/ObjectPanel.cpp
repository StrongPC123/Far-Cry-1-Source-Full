// ObjectPanel.cpp : implementation file
//

#include "stdafx.h"
#include "ObjectPanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "Objects\ObjectManager.h"
#include "Objects\BaseObject.h"
#include "LayersSelectDialog.h"
#include "Material\Material.h"

/////////////////////////////////////////////////////////////////////////////
// CObjectPanel dialog


CObjectPanel::CObjectPanel(CWnd* pParent )
	: CXTResizeDialog(CObjectPanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CObjectPanel)
	m_name = _T("");
	//m_flatten = FALSE;
	//m_bShared = FALSE;
	//}}AFX_DATA_INIT
	m_obj = 0;
	m_multiSelect = false;
}

CObjectPanel::~CObjectPanel()
{
}


void CObjectPanel::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectPanel)
	DDX_Control(pDX, IDC_OBJECT_COLOR, m_colorCtrl);
	DDX_Text(pDX, IDC_OBJECT_NAME, m_name);
	//DDX_Check(pDX, IDC_FLATTEN, m_flatten);
	//DDX_Check(pDX, IDC_SHARED, m_bShared);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_OBJECT_NAME, m_nameCtrl);
	//DDX_Control(pDX, IDC_FLATTEN, m_flattenCtrl);
	//DDX_Control(pDX, IDC_SHARED, m_sharedCtrl);
	DDX_Control(pDX, IDC_LAYER, m_layerBtn);
	DDX_Control(pDX, IDC_LAYER_NAME, m_layerName);
	DDX_Control(pDX, IDC_MATERIAL, m_mtlBtn);
}


BEGIN_MESSAGE_MAP(CObjectPanel, CXTResizeDialog)
	//{{AFX_MSG_MAP(CObjectPanel)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_OBJECT_COLOR, OnObjectColor)
	ON_WM_KEYDOWN()
	//ON_EN_UPDATE(IDC_OBJECT_NAME,OnUpdate)
	ON_EN_UPDATE(IDC_OBJECT_AREA,OnUpdate)
	ON_EN_UPDATE(IDC_OBJECT_HELPER,OnUpdate)
	//ON_BN_CLICKED(IDC_SHARED, OnShared)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LAYER, OnBnClickedLayer)
	ON_EN_KILLFOCUS(IDC_OBJECT_NAME, OnChangeName)
	ON_BN_CLICKED(IDC_MATERIAL, OnBnClickedMaterial)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectPanel message handlers

//////////////////////////////////////////////////////////////////////////
void CObjectPanel::SetMultiSelect( bool bEnable )
{
	m_multiSelect = bEnable;

	if (bEnable)
	{
		//m_sharedCtrl.SetButtonStyle(BS_AUTO3STATE);
		//m_flattenCtrl.SetButtonStyle(BS_AUTO3STATE);
		m_nameCtrl.EnableWindow(FALSE);
		//m_sharedCtrl.SetCheck(2);
		//m_flattenCtrl.SetCheck(2);
		m_area.EnableWindow(FALSE);
		m_layerName.SetWindowText( "" );
		m_helperSize.EnableWindow(FALSE);

		m_mtlBtn.EnableWindow( FALSE );
		m_mtlBtn.SetWindowText( "" );
	}else
	{
		m_nameCtrl.EnableWindow(TRUE);
		m_helperSize.EnableWindow(TRUE);
		//m_sharedCtrl.SetButtonStyle(BS_AUTOCHECKBOX);
		//m_flattenCtrl.SetButtonStyle(BS_AUTOCHECKBOX);
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectPanel::SetParams( CBaseObject *obj, const SParams &params )
{
	if (obj != m_obj && m_obj)
	{
		UpdateData(TRUE);
		OnUpdate();
	}
	m_obj = obj;

	COLORREF col = m_color;

	m_name = params.name;
	m_color = params.color;
	m_area.SetValue( params.area );
	m_helperSize.SetValue( params.helperScale );
	//m_flatten = params.flatten;
	//m_bShared = params.shared;

	UpdateData(FALSE);

	if (m_obj && m_obj->GetMaterial())
	{
		m_mtlBtn.EnableWindow( TRUE );
		m_mtlBtn.SetWindowText( m_obj->GetMaterial()->GetName() );
	}
	else
	{
		m_mtlBtn.EnableWindow( FALSE );
		if (m_obj)
			m_mtlBtn.SetWindowText( "No Material" );
		else
			m_mtlBtn.SetWindowText( "" );
	}

	m_currentLayer = params.layer;
	// Find layer from id.
	CObjectLayer *layer = GetIEditor()->GetObjectManager()->GetLayersManager()->FindLayerByName(params.layer);
	if (layer)
		m_layerName.SetWindowText( layer->GetName() );
	else
		m_layerName.SetWindowText( "" );

	if (col != m_color)
	{
		GetDlgItem(IDC_OBJECT_COLOR)->RedrawWindow();
		//RedrawWindow();
	}
}

void CObjectPanel::GetParams( SParams &params )
{
	UpdateData(TRUE);
	params.name = m_name;
	params.color = m_color;
	params.area = m_area.GetValue();
	params.helperScale = m_helperSize.GetValue();
	//params.flatten = m_flatten;
	//params.shared = m_bShared;
	params.layer = m_currentLayer;
}

void CObjectPanel::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDCtl == IDC_OBJECT_COLOR)
	{
		CDC dc;
		dc.Attach( lpDrawItemStruct->hDC );
		CBrush brush( m_color );
		CPen pen( PS_SOLID,1,RGB(1,1,1) );
		CBrush *prevBrush = dc.SelectObject( &brush );
		CPen *prevPen = dc.SelectObject( &pen );
		dc.Rectangle( &lpDrawItemStruct->rcItem );
		dc.SelectObject( prevBrush );
		dc.SelectObject( prevPen );
		dc.Detach();
		return;
	}
	
	CXTResizeDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CObjectPanel::OnObjectColor() 
{
	// TODO: Add your control notification handler code here
	COLORREF col = m_color;
	if (GetIEditor()->SelectColor(col,this))
	{
		m_color = col;
		m_colorCtrl.RedrawWindow();
		OnUpdate();

		if (m_multiSelect)
		{
			CUndo undo("Set Color");
			// Update shared flags in current selction group.
			CSelectionGroup *selection = GetIEditor()->GetSelection();
			for (int i = 0; i < selection->GetCount(); i++)
			{
				selection->GetObject(i)->SetColor(m_color);
			}
		}
	}
}

void CObjectPanel::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CXTResizeDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CObjectPanel::OnInitDialog() 
{
	CXTResizeDialog::OnInitDialog();
	
	m_area.Create( this,IDC_OBJECT_AREA );
	m_layerBtn.SetIcon( MAKEINTRESOURCE(IDI_LAYERS) );

	m_helperSize.Create( this,IDC_OBJECT_HELPER );
	m_helperSize.SetRange( 0.01f,1000 );

	SetResize( IDC_OBJECT_NAME,SZ_RESIZE(1) );
	SetResize( IDC_LAYER_NAME,SZ_RESIZE(1) );
	SetResize( IDC_MATERIAL,SZ_RESIZE(1) );
	SetResize( IDC_OBJECT_COLOR,SZ_REPOS(1) );

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CObjectPanel::OnUpdate()
{
	if (m_obj)
		m_obj->OnUIUpdate();
}

/*
void CObjectPanel::OnShared() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	OnUpdate();

	if (m_multiSelect)
	{
		CUndo undo("Set Shared");
		// Update shared flags in current selction group.
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		for (int i = 0; i < selection->GetCount(); i++)
		{
			selection->GetObject(i)->SetShared(m_bShared);
		}
	}
}
*/

//////////////////////////////////////////////////////////////////////////
void CObjectPanel::OnUpdateName()
{
	//UpdateData(TRUE);
	//OnUpdate();

	/*
	if (m_multiSelect)
	{
		CUndo undo("Set Name");
		// Update shared flags in current selction group.
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		for (int i = 0; i < selection->GetCount(); i++)
		{
			selection->GetObject(i)->SetName(m_name);
		}
	}
	*/
}

/*
//////////////////////////////////////////////////////////////////////////
void CObjectPanel::OnUpdateFlatten()
{
	UpdateData(TRUE);
	OnUpdate();

	if (m_multiSelect)
	{
		CUndo undo("Set Flatten");
		// Update shared flags in current selction group.
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		for (int i = 0; i < selection->GetCount(); i++)
		{
			//OBJFLAG_FLATTEN
			if (m_flatten)
				selection->GetObject(i)->SetFlags( OBJFLAG_FLATTEN );
			else
				selection->GetObject(i)->ClearFlags( OBJFLAG_FLATTEN );
		}
	}
}
*/
//////////////////////////////////////////////////////////////////////////
void CObjectPanel::OnUpdateArea()
{
	UpdateData(TRUE);
	OnUpdate();

	if (m_multiSelect)
	{
		CUndo undo("Set Area");
		// Update shared flags in current selction group.
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		for (int i = 0; i < selection->GetCount(); i++)
		{
			selection->GetObject(i)->SetArea(m_area.GetValue());
		}
	}
}
void CObjectPanel::OnBnClickedLayer()
{
	// Open Layer selection dialog.
	CRect rc;
	m_layerBtn.GetWindowRect(rc);
	CLayersSelectDialog dlg( CPoint(rc.left-40,rc.bottom) );

	CString selLayer;
	m_layerName.GetWindowText(selLayer);
	dlg.SetSelectedLayer( selLayer );
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.GetSelectedLayer() != selLayer)
			selLayer = dlg.GetSelectedLayer();
		if (!selLayer.IsEmpty())
		{
			CObjectLayer *pLayer = GetIEditor()->GetObjectManager()->GetLayersManager()->FindLayerByName(selLayer);
			if (!pLayer)
				return;
			
			m_layerName.SetWindowText(selLayer);
			m_currentLayer = pLayer->GetName();

			CUndo undo("Set Object Layer");
			if (m_multiSelect)
			{
				// Update shared flags in current selction group.
				CSelectionGroup *selection = GetIEditor()->GetSelection();
				for (int i = 0; i < selection->GetCount(); i++)
				{
					selection->GetObject(i)->SetLayer(pLayer);
				}
			}
			else
			{
				OnUpdate();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectPanel::OnChangeName()
{	
	CUndo undo("Set Name");
	if (m_multiSelect)
	{
		CString name;
		m_nameCtrl.GetWindowText(name);
		// Update shared flags in current selction group.
		CSelectionGroup *selection = GetIEditor()->GetSelection();
		for (int i = 0; i < selection->GetCount(); i++)
		{
			GetIEditor()->GetObjectManager()->ChangeObjectName( selection->GetObject(i),name );
		}
	}
	else
	{
		OnUpdate();
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectPanel::OnOK()
{
	// Name change.
	OnChangeName();
};

//////////////////////////////////////////////////////////////////////////
void CObjectPanel::OnBnClickedMaterial()
{
	// Select current material
	if (m_obj)
	{
		if (m_obj->GetMaterial())
		{
			GetIEditor()->OpenDataBaseLibrary( EDB_MATERIAL_LIBRARY,m_obj->GetMaterial() );
		}
	}
}

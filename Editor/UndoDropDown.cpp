// UndoDropDown.cpp : implementation file
//

#include "stdafx.h"
#include "UndoDropDown.h"
#include "Undo\Undo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUndoDropDown dialog


CUndoDropDown::CUndoDropDown( const CPoint &pos,bool bUndo,CWnd* pParent /* = NULL */)
	: CDialog(CUndoDropDown::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUndoDropDown)
	//}}AFX_DATA_INIT

	m_bUndo = bUndo;
	m_pos = pos;
}


void CUndoDropDown::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUndoDropDown)
	DDX_Control(pDX, IDC_UNDO_CLEAR, m_undoClear);
	DDX_Control(pDX, IDC_UNDO_BUTTON, m_undoButton);
	DDX_Control(pDX, IDC_UNDO, m_undo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUndoDropDown, CDialog)
	//{{AFX_MSG_MAP(CUndoDropDown)
	ON_LBN_SELCHANGE(IDC_UNDO, OnSelchangeUndo)
	ON_BN_CLICKED(IDC_UNDO_BUTTON, OnUndoButton)
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_UNDO_CLEAR, OnUndoClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUndoDropDown message handlers

BOOL CUndoDropDown::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CUndoManager *undoMgr = GetIEditor()->GetUndoManager();

	CString str;
	std::vector<CString> names;
	if (m_bUndo)
	{
		undoMgr->GetUndoStackNames( names );
		str.Format( "Undo 1 action(s)",names.size() );
	}
	else
	{
		undoMgr->GetRedoStackNames( names );
		str.Format( "Redo 1 action(s)",names.size() );
	}

	m_undo.ResetContent();
	for (int i = names.size()-1; i >= 0; i--)
	{
		m_undo.AddString( names[i] );
		if (i == 0)
			m_undo.SetSel(i);
	}
	m_undoButton.SetWindowText( str );

	SetWindowPos( NULL,m_pos.x,m_pos.y,0,0,SWP_NOSIZE );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUndoDropDown::OnSelchangeUndo() 
{
	//bool 
	bool bSelected = false;
	for (int i = m_undo.GetCount()-1; i >= 0; i--)
	{
		if (bSelected)
			m_undo.SetSel(i);
		else
		{
			if (m_undo.GetSel(i))
				bSelected = true;
		}
	}

	int numSelected = m_undo.GetSelCount();
	CString str;
	if (m_bUndo)
	{
		str.Format( "Undo %d action(s)",numSelected );
	}
	else
	{
		str.Format( "Redo %d action(s)",numSelected );
	}
	m_undoButton.SetWindowText( str );
}


void CUndoDropDown::OnUndoButton() 
{
	int numSelected = m_undo.GetSelCount();
	if (m_bUndo)
	{
		GetIEditor()->GetUndoManager()->Undo( numSelected );
	}
	else
	{
		GetIEditor()->GetUndoManager()->Redo( numSelected );
	}
	EndDialog(IDOK);
}

void CUndoDropDown::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CDialog::OnRButtonDown(nFlags, point);
	
	EndDialog(IDCANCEL);
}

void CUndoDropDown::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CDialog::OnLButtonDown(nFlags, point);

	CRect rc;
	GetClientRect( rc );
	if (!rc.PtInRect(point))
	{
		EndDialog(IDCANCEL);
	}
}

LRESULT CUndoDropDown::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT res = CDialog::WindowProc(message, wParam, lParam);

	if (message == WM_NCACTIVATE)
	{
		if (wParam == FALSE)
		{
			PostMessage( WM_COMMAND,MAKEWPARAM(IDCANCEL,0), 0 );
		}
	}
	
	return res;
}

void CUndoDropDown::OnUndoClear() 
{
	//int numSelected = m_undo.GetSelCount();
	if (m_bUndo)
	{
		GetIEditor()->GetUndoManager()->ClearUndoStack();
	}
	else
	{
		GetIEditor()->GetUndoManager()->ClearRedoStack();
	}
	EndDialog(IDOK);
}

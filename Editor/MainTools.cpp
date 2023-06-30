// MainTools.cpp : implementation file
//

#include "stdafx.h"
#include "MainTools.h"
#include "ObjectCreateTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainTools dialog


CMainTools::CMainTools(CWnd* pParent /*=NULL*/)
	: CDialog(CMainTools::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMainTools)
	//}}AFX_DATA_INIT

	m_lastPressed = -1;
	m_nTimer = 0;
}

CMainTools::~CMainTools()
{
	ReleaseButtons();
}


void CMainTools::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainTools)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMainTools, CDialog)
	//{{AFX_MSG_MAP(CMainTools)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainTools message handlers

BOOL CMainTools::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// Route the commands to the view
	if (nID == ID_INSERT_ENTITY && AfxGetMainWnd())
	{
		AfxGetMainWnd()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	}
	//((CFrameWnd *) (AfxGetMainWnd()))->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	
	return CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainTools::UncheckAll()
{
	m_lastPressed = -1;
	for (int i = 0; i < m_buttons.size(); i++)
	{
		m_buttons[i]->SetCheck( 0 );
	}
}

BOOL CMainTools::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (HIWORD(wParam) == BN_CLICKED)
	{
		int ctrlId = LOWORD(wParam);

		OnButtonPressed( ctrlId );
		return TRUE;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

BOOL CMainTools::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CreateButtons();
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CMainTools::CreateButtons()
{
	int xOffset = 5;
	int yOffset = 4;

	int m_buttonHeight = 18;

	int row = 0;
	int col = 0;

	CRect rc;
	GetClientRect( rc );

	int buttonWidth = ((rc.right - rc.left) - 3*xOffset)/2;

	std::vector<CString> categories;
	GetIEditor()->GetObjectManager()->GetClassCategories( categories );

	int newHeight = 0;

	// Get class categories.
	for (int i = 0; i < categories.size(); i++)
	{
		CString category = categories[i];

		//CColorButton *button = new CColorButton;
		//CColorButton *button = new CColorButton;

		CColorCheckBox *button = new CColorCheckBox;
		button->SetPushedBkColor( RGB(255,255,0) );
		//button->SetBevel(1);
		//button->SetBkColor( RGB(255,255,0) );
		CRect brc;

		brc.left = xOffset + col*(buttonWidth + xOffset);
		brc.right = brc.left + buttonWidth;

		brc.top = yOffset + row*(m_buttonHeight + 4);
		brc.bottom = brc.top + m_buttonHeight;

		newHeight = brc.bottom + 4;

		//button->Create( category,WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTORADIOBUTTON|BS_PUSHLIKE,brc,this,i );
		//button->Create( category,WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX,brc,this,i );
		button->Create( category,WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,brc,this,i );
		button->SetFont( CFont::FromHandle( (HFONT)::GetStockObject(DEFAULT_GUI_FONT)) );
		//button->ModifyStyleEx( 0,WS_EX_STATICEDGE,SWP_FRAMECHANGED );
		button->SetCheck(0);
		m_buttons.push_back( button );

		col++;
		brc.left = xOffset + col*(buttonWidth + xOffset);
		brc.right = brc.left + buttonWidth;
		if (brc.right > rc.right-4)
		{
			col = 0;
			row++;
		}
	}

	SetWindowPos( NULL,0,0,rc.right-rc.left,newHeight+4,SWP_NOMOVE );
}

//////////////////////////////////////////////////////////////////////////
void CMainTools::ReleaseButtons()
{
	for (int i = 0; i < m_buttons.size(); i++)
	{
		delete m_buttons[i];
	}
	m_buttons.clear();
}

void CMainTools::OnButtonPressed( int i )
{
	ASSERT( i >= 0 && i < m_buttons.size() );

	if (i == m_lastPressed)
	{
		UncheckAll();
		if (GetIEditor()->GetEditTool())
			GetIEditor()->SetEditTool(0);
		return;
	}
	
	UncheckAll();

	m_lastPressed = i;
	m_buttons[i]->SetCheck(1);

	CString category;
	m_buttons[i]->GetWindowText( category );
	
	// Create browse mode for this category.
	CObjectCreateTool *tool = new CObjectCreateTool;
	GetIEditor()->SetEditTool( tool );
	tool->SelectCategory( category );

	// Start monitoring button state.
	StartTimer();
}

void CMainTools::OnTimer(UINT_PTR nIDEvent) 
{
	CDialog::OnTimer(nIDEvent);

	CEditTool *tool = GetIEditor()->GetEditTool();
	if (!tool || !(tool->GetRuntimeClass() == RUNTIME_CLASS(CObjectCreateTool)))
	{
		UncheckAll();
		StopTimer();
	}
}

void CMainTools::StartTimer()
{
	StopTimer();
	m_nTimer = SetTimer(1,500,NULL);
}
	
void CMainTools::StopTimer()
{
	if (m_nTimer != 0)
		KillTimer(m_nTimer);
	m_nTimer = 0;
}

void CMainTools::OnDestroy() 
{
	StopTimer();
	CDialog::OnDestroy();
}

void CMainTools::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	/*
	static bIgnoreSize = false;
	if (m_buttons.size() > 0 && !bIgnoreSize)
	{
		bIgnoreSize = true;
		ReleaseButtons();
		CreateButtons();
		bIgnoreSize = false;
	}
	*/
}

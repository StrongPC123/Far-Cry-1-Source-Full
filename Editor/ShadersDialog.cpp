// ShadersDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ShadersDialog.h"
#include "ShaderEnum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShadersDialog dialog


CShadersDialog::CShadersDialog(const CString &selection, CWnd* pParent /*=NULL*/)
	: CXTResizeDialog(CShadersDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShadersDialog)
	m_selection = _T("");
	//}}AFX_DATA_INIT

	m_sel = selection;
	m_brush.CreateSolidBrush( RGB(0xE0,0xE0,0xE0) );
}


void CShadersDialog::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShadersDialog)
	DDX_Control(pDX, IDC_SHADERS, m_shaders);
	DDX_Control(pDX, IDC_TEXT, m_shaderText);
	DDX_Control(pDX, IDC_SAVE, m_saveButton);
	DDX_LBString(pDX, IDC_SHADERS, m_selection);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShadersDialog, CXTResizeDialog)
	//{{AFX_MSG_MAP(CShadersDialog)
	ON_LBN_SELCHANGE(IDC_SHADERS, OnSelchangeShaders)
	ON_LBN_DBLCLK(IDC_SHADERS, OnDblclkShaders)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_EDIT, OnBnClickedEdit)
	ON_BN_CLICKED(IDC_SAVE, OnBnClickedSave)
	ON_EN_CHANGE(IDC_TEXT, OnEnChangeText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShadersDialog message handlers

void CShadersDialog::OnSelchangeShaders() 
{
	// When shader changes.
	// Edit shader file.
	int sel = m_shaders.GetCurSel();
	if (sel != LB_ERR)
	{
		CString file = GetIEditor()->GetShaderEnum()->GetShaderFile( sel );
		file.Replace( '/','\\' );
		m_shaderText.LoadFile( file );
		// Just loaded file.. Not savable.
		m_saveButton.EnableWindow(FALSE);

		CString shaderName;
		m_shaders.GetText( sel,shaderName );
		shaderName = CString("\'") + shaderName + "\'";

		m_shaderText.SetSel(0,-1);
		CString text = m_shaderText.GetSelText();
		int pos = text.Find( shaderName );

		if (pos >= 0)
		{
			m_shaderText.SetSel( pos,pos+strlen(shaderName) );
		}
		else
		{
			m_shaderText.SetSel(0,0);
		}
	}
}

BOOL CShadersDialog::OnInitDialog() 
{
	CXTResizeDialog::OnInitDialog();

	CWaitCursor wait;
	
	m_shaders.ResetContent();
	// Fill with shaders.
	CShaderEnum *shaderEnum = GetIEditor()->GetShaderEnum();
	int numShaders = shaderEnum->EnumShaders();
	for (int i = 0; i < numShaders; i++)
	{
		m_shaders.AddString( shaderEnum->GetShader(i) );
	}
	/*
	if (numShaders > 0)
	{
		int i = m_shaders.FindString(m_sel);
		if (i != LB_ERR)
			m_shaders.SetCurSel( i );
	}
	*/
	m_selection = m_sel;

	UpdateData(FALSE);

	SetResize( IDOK,SZ_VERREPOS(1) );
	SetResize( IDCANCEL,SZ_VERREPOS(1) );
	SetResize( IDC_LINE,CXTResizeRect(0,1,1,1)  );
	SetResize( IDC_SHADERS,SZ_VERRESIZE(1) );

	SetResize( IDC_TEXT,SZ_RESIZE(1) );
	SetResize( IDC_SAVE,SZ_REPOS(1) );
	SetResize( IDC_EDIT,SZ_REPOS(1) );
	SetMinSize( CSize(260,200) );

	AutoLoadPlacement( "Dialogs\\Shaders" );

	OnSelchangeShaders();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CShadersDialog::OnDblclkShaders() 
{
	// Same as IDOK.
	UpdateData(TRUE);
	EndDialog(IDOK);
}

HBRUSH CShadersDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CXTResizeDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (nCtlColor == CTLCOLOR_LISTBOX && pWnd == &m_shaders)
	{
		
		// Set the background mode for text to transparent 
		// so background will show thru.
		pDC->SetBkMode(TRANSPARENT);

		hbr = m_brush;
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CShadersDialog::OnBnClickedEdit()
{
	// Edit shader file.
	int sel = m_shaders.GetCurSel();
	if (sel != LB_ERR)
	{
		CShaderEnum *shaderEnum = GetIEditor()->GetShaderEnum();
		CString file = shaderEnum->GetShaderFile( sel );

		CFileUtil::EditTextFile( file,CFileUtil::FILE_TYPE_SHADER );
	}
}

//////////////////////////////////////////////////////////////////////////
void CShadersDialog::OnBnClickedSave()
{
	if (m_shaderText.IsModified())
	{
		m_shaderText.SaveFile( m_shaderText.GetFilename() );
		m_shaderText.Parse();
		if (m_shaderText.IsModified())
		{
			m_saveButton.EnableWindow(TRUE);
		}
		else
		{
			m_saveButton.EnableWindow(FALSE);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CShadersDialog::OnEnChangeText()
{
	m_shaderText.OnChange();
	// File can be saved.
	m_saveButton.EnableWindow(TRUE);
}

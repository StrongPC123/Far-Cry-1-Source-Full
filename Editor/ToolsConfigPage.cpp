////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   toolsconfigpage.cpp
//  Version:     v1.00
//  Created:     27/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolsConfigPage.h"
#include "ExternalTools.h"

// CToolsConfigPage dialog

IMPLEMENT_DYNAMIC(CToolsConfigPage, CXTResizePropertyPage)
CToolsConfigPage::CToolsConfigPage()
	: CXTResizePropertyPage(CToolsConfigPage::IDD)
{

		// Call CXTEditListBox::SetListEditStyle to set the type of edit list. You can
	// pass in LBS_XT_NOTOOLBAR if you don't want the toolbar displayed.
	m_editList.SetListEditStyle(_T(" &Tools:"),
		LBS_XT_DEFAULT);
}

CToolsConfigPage::~CToolsConfigPage()
{
	// Free tools.
	for (int i = 0; i < m_tools.size(); i++)
	{
		delete m_tools[i];
	}
	m_tools.clear();
}

void CToolsConfigPage::DoDataExchange(CDataExchange* pDX)
{
	CXTResizePropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TXT_EDIT1, m_txtEdit1);
	DDX_Control(pDX, IDC_EDIT1, m_edit1);
	DDX_Control(pDX, IDC_TOGGLEVARONOFF, m_toggleVar);
	DDX_Control(pDX, IDC_EDIT_LIST, m_editList);
}


BEGIN_MESSAGE_MAP(CToolsConfigPage, CXTResizePropertyPage)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_BN_CLICKED( IDC_TOGGLEVARONOFF,OnToggleVar )
	ON_LBN_SELCHANGE(IDC_EDIT_LIST, OnSelchangeEditList)
	ON_LBN_XT_NEWITEM(IDC_EDIT_LIST, OnNewItem)
END_MESSAGE_MAP()


// CToolsConfigPage message handlers
BOOL CToolsConfigPage::OnInitDialog() 
{
	CXTResizePropertyPage::OnInitDialog();
	
	m_editList.SetCurSel(0);
//	OnSelchangeEditList();

	// Set control resizing.
	SetResize(IDC_EDIT_LIST, SZ_TOP_LEFT, SZ_BOTTOM_RIGHT);
	SetResize(IDC_TXT_EDIT1, SZ_BOTTOM_LEFT, SZ_BOTTOM_LEFT);
	SetResize(IDC_EDIT1, SZ_BOTTOM_LEFT, SZ_BOTTOM_RIGHT);

	CExternalToolsManager *pTools = GetIEditor()->GetExternalToolsManager();
	for (int i = 0; i < pTools->GetToolsCount(); i++)
	{
		Tool *tool = new Tool;
		CExternalTool* extTool = pTools->GetTool(i);
		tool->bToggle = extTool->m_variableToggle;
		tool->command = extTool->m_command;

		int id = m_editList.AddString(extTool->m_title);
		m_editList.SetItemData( id,(DWORD_PTR)tool );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CToolsConfigPage::OnOK()
{
	CExternalToolsManager *pTools = GetIEditor()->GetExternalToolsManager();
	pTools->ClearTools();
	CString title;
	for (int i = 0; i < m_editList.GetCount(); i++)
	{
		m_editList.GetText(i,title);
		if (title.IsEmpty())
			continue;

		Tool *tool = (Tool*)m_editList.GetItemData(i);
		if (!tool)
			continue;
		CExternalTool *extTool = new CExternalTool;

		extTool->m_title = title;
		extTool->m_command = tool->command;
		extTool->m_variableToggle = tool->bToggle;
		pTools->AddTool(extTool);
	}
}

//////////////////////////////////////////////////////////////////////////
void CToolsConfigPage::OnSelchangeEditList()
{
	int iIndex = m_editList.GetCurSel();

	Tool* pTool = (Tool*)m_editList.GetItemData(iIndex);
	if (pTool != NULL && iIndex != LB_ERR)
	{
		CString command = pTool->command;
		m_edit1.SetWindowText( command );
		m_toggleVar.SetCheck( (pTool->bToggle)?BST_CHECKED:BST_UNCHECKED );
	}
	else
	{
		m_edit1.SetWindowText(_T(""));
		m_toggleVar.SetCheck(0);
	}
}

//////////////////////////////////////////////////////////////////////////
void CToolsConfigPage::ReadFromControls( int ctrl )
{
	int iIndex = m_editList.GetCurSel();

	if (iIndex != LB_ERR)
	{
		Tool* pTool = (Tool*)m_editList.GetItemData(iIndex);
		if (pTool != NULL)
		{
			switch (ctrl)
			{
			case CTRL_COMMAND:
				m_edit1.GetWindowText( pTool->command );
				break;
			case CTRL_TOGGLE:
				pTool->bToggle = m_toggleVar.GetCheck() != 0;
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CToolsConfigPage::OnChangeEdit1()
{
	ReadFromControls(CTRL_COMMAND);
}

//////////////////////////////////////////////////////////////////////////
void CToolsConfigPage::OnToggleVar()
{
	ReadFromControls(CTRL_TOGGLE);
}

//////////////////////////////////////////////////////////////////////////
void CToolsConfigPage::OnNewItem()
{
	int iItem = m_editList.GetCurrentIndex();
	if (iItem != -1)
	{
		Tool* pTool = new Tool;
		m_tools.push_back(pTool);

		pTool->command = "";
		pTool->bToggle = false;
		
		m_editList.SetItemData(iItem, (DWORD_PTR)pTool);
		m_editList.SetCurSel(iItem);

		m_edit1.SetWindowText(_T(""));
		m_toggleVar.SetCheck(0);
	}
}
// ToolButton.cpp : implementation file
//

#include "stdafx.h"
#include "ToolButton.h"
#include "EditTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TOOLBUTTON_TIMERID 1001

/////////////////////////////////////////////////////////////////////////////
// CToolButton
IMPLEMENT_DYNAMIC(CToolButton,CColorCheckBox)

//////////////////////////////////////////////////////////////////////////
CToolButton::CToolButton()
{
	m_toolClass = 0;
	m_userData = 0;
	m_nTimer = 0;
}

CToolButton::~CToolButton()
{
	StopTimer();
}

void CToolButton::SetToolClass( CRuntimeClass *toolClass,void *userData )
{
	m_toolClass = toolClass;
	m_userData = userData;
}

BEGIN_MESSAGE_MAP(CToolButton, CButton)
	//{{AFX_MSG_MAP(CToolButton)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolButton message handlers
void CToolButton::OnTimer(UINT_PTR nIDEvent) 
{
	// Check tool state.
	CEditTool *tool = GetIEditor()->GetEditTool();
	CRuntimeClass *toolClass = 0;
	if (tool)
		toolClass = tool->GetRuntimeClass();
		
	int c = GetCheck();
		
	if (toolClass != m_toolClass)
	{
		if (GetCheck() == 1)
			SetCheck(0);
		StopTimer();
	}
		
	CButton::OnTimer(nIDEvent);
}

void CToolButton::OnDestroy() 
{
	StopTimer();
	CButton::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////
void CToolButton::OnPaint()
{
	CColorCheckBox::OnPaint();

	// Check if tool is current tool.
	CEditTool *tool = GetIEditor()->GetEditTool();
	if (tool && tool->GetRuntimeClass() == m_toolClass)
	{
		if (GetCheck() == 0)
		{
			SetCheck(1);
			StartTimer();
		}
	}
	else
	{
		if (GetCheck() == 1)
		{
			SetCheck(0);
			StopTimer();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CToolButton::OnClicked()
{
	if (!m_toolClass)
		return;

	CEditTool *tool = GetIEditor()->GetEditTool();
	if (tool && tool->GetRuntimeClass() == m_toolClass)
	{
		GetIEditor()->SetEditTool(0);
		SetCheck(0);

		StopTimer();
	}
	else
	{
		CEditTool *tool = (CEditTool*)m_toolClass->CreateObject();
		if (!tool)
			return;
		
		SetCheck(1);
		StartTimer();
		
		if (m_userData)
			tool->SetUserData( m_userData );

		// Must be last function, can delete this.
		GetIEditor()->SetEditTool( tool );
	}
}

void CToolButton::StartTimer()
{
	StopTimer();
	m_nTimer = SetTimer(TOOLBUTTON_TIMERID,200,NULL);
}
	
void CToolButton::StopTimer()
{
	if (m_nTimer != 0)
		KillTimer(m_nTimer);
	m_nTimer = 0;
}

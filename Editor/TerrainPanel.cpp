// TerrainPanel.cpp : implementation file
//

#include "stdafx.h"
#include "TerrainPanel.h"
#include "TerrainModifyTool.h"
#include "TerrainHoleTool.h"
#include "VegetationTool.h"
#include "EnvironmentTool.h"
#include "ShaderTemplateTool.h"
#include "TerrainTexturePainter.h"
#include "TerrainMoveTool.h"
#include "TextureTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTerrainPanel dialog


CTerrainPanel::CTerrainPanel(CWnd* pParent /*=NULL*/)
: CDialog(CTerrainPanel::IDD, pParent)
{
	Create(MAKEINTRESOURCE(IDD),pParent);
	//{{AFX_DATA_INIT(CTerrainPanel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTerrainPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTerrainPanel)
	DDX_Control(pDX, IDC_TEXTURE, m_textureBtn);
	DDX_Control(pDX, IDC_ENVIRONMENT, m_environmentBtn);
	DDX_Control(pDX, IDC_VEGETATION, m_vegetationBtn);
	DDX_Control(pDX, IDC_MODIFY, m_modifyBtn);
	DDX_Control(pDX, IDC_HOLE, m_holeBtn);
	//DDX_Control(pDX, IDC_SHADER, m_shaderBtn);
	DDX_Control(pDX, IDC_TERRAIN_MOVE, m_moveBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTerrainPanel, CDialog)
	//{{AFX_MSG_MAP(CTerrainPanel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CTerrainPanel::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// Route the commands to the MainFrame
	//AfxGetMainWnd()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	
	return CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CTerrainPanel::OnIdleUpdate()
{
	/*
	CEditTool *tool = GetIEditor()->GetEditTool();
	CRuntimeClass *toolClass = 0;
	if (tool)
		toolClass = tool->GetRuntimeClass();

	if (toolClass != RUNTIME_CLASS(CTerrainModifyTool))
	{
		if (IsDlgButtonChecked(IDC_MODIFY))
			CheckDlgButton( IDC_MODIFY,BST_UNCHECKED );
	}
	*/
}

BOOL CTerrainPanel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_modifyBtn.SetToolClass( RUNTIME_CLASS(CTerrainModifyTool) );
	m_holeBtn.SetToolClass( RUNTIME_CLASS(CTerrainHoleTool) );
	m_vegetationBtn.SetToolClass( RUNTIME_CLASS(CVegetationTool) );
	m_environmentBtn.SetToolClass( RUNTIME_CLASS(CEnvironmentTool) );
	//m_shaderBtn.SetToolClass( RUNTIME_CLASS(CShaderTemplateTool) );

	//m_textureBtn.SetToolClass( RUNTIME_CLASS(CTextureTool) );
	m_textureBtn.SetToolClass( RUNTIME_CLASS(CTerrainTexturePainter) );
	m_moveBtn.SetToolClass( RUNTIME_CLASS(CTerrainMoveTool) );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

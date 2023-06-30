#if !defined(AFX_TERRAINPANEL_H__246D04AD_4C47_4362_910A_6FD65C1A07E8__INCLUDED_)
#define AFX_TERRAINPANEL_H__246D04AD_4C47_4362_910A_6FD65C1A07E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TerrainPanel.h : header file
//

#include "Controls\ToolButton.h"

/////////////////////////////////////////////////////////////////////////////
// CTerrainPanel dialog

class CTerrainPanel : public CDialog
{
// Construction
public:
	CTerrainPanel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTerrainPanel)
	enum { IDD = IDD_TERRAIN_PANEL };
	CToolButton	m_textureBtn;
	CToolButton	m_shaderBtn;
	CToolButton	m_environmentBtn;
	CToolButton	m_vegetationBtn;
	CToolButton	m_modifyBtn;
	CToolButton	m_holeBtn;
	CToolButton	m_moveBtn;
	//}}AFX_DATA

	void OnIdleUpdate();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerrainPanel)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTerrainPanel)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAINPANEL_H__246D04AD_4C47_4362_910A_6FD65C1A07E8__INCLUDED_)

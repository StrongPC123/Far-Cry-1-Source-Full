////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ShaderTemplatePanel.h
//  Version:     v0.01
//  Created:     13/2/2002 by Martin Mittring
//  Compilers:   Visual C++ 6.0
//  Description: Shader template parameter tweaker panel
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ShaderTemplatePanel_h__
#define __ShaderTemplatePanel_h__
#include "afxwin.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertiesPanel.h : header file
//

#include "Controls\PropertyCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CShaderTemplatePanel dialog

class CShaderTemplateTool;

class CShaderTemplatePanel : public CDialog
{
// Construction
public:
	CShaderTemplatePanel( CShaderTemplateTool *obj, XmlNodeRef &node,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CShaderTemplatePanel)
	enum { IDD = IDD_PANEL_SHADERTEMPLATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void SetProperties( XmlNodeRef &props );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShaderTemplatePanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	void OnPropertyChanged( XmlNodeRef node );

	// Generated message map functions
	//{{AFX_MSG(CShaderTemplatePanel)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CPropertyCtrl					m_propWnd;
	XmlNodeRef						m_node;
	CShaderTemplateTool *	m_object;
public:
	CComboBox		m_TemplateCombo;
	CComboBox		m_Material;
	CString			m_sFilename;
	CString			m_sParameterSetName;

	afx_msg void OnCbnSelchangeTemplatecombo();
	afx_msg void OnBnClickedShadertemplCopy();
	afx_msg void OnBnClickedShadertemplSetdefault();
	afx_msg void OnBnClickedShadertemplPaste();
	afx_msg void OnBnClickedShadertemplLoad();
	afx_msg void OnBnClickedShadertemplSave();
	afx_msg void OnBnClickedShadertemplGetfromsel();
	afx_msg void OnBnClickedShadertemplSelectbystp();
	afx_msg void OnBnClickedShadertemplTolib();
	afx_msg void OnBnClickedShadertemplFromlib();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __ShaderTemplatePanel_h__

////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PrefabPanel.h
//  Version:     v1.00
//  Created:     14/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PrefabPanel_h__
#define __PrefabPanel_h__
#pragma once

#include "XTToolkit.h"

class CBaseObject;
class CPrefabObject;
// CPrefabPanel dialog

class CPrefabPanel : public CXTResizeDialog
{
	DECLARE_DYNCREATE(CPrefabPanel)

public:
	CPrefabPanel(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPrefabPanel();

	void SetObject( CPrefabObject *object );

	// Dialog Data
	enum { IDD = IDD_PANEL_PREFAB };

protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnSelChangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedExtractSelected();
	afx_msg void OnBnClickedExtractAll();
	afx_msg void OnBnClickedPrefab();

	DECLARE_MESSAGE_MAP()

	void ReloadObjects();
	
	CPrefabObject* m_object;

	CTreeCtrl m_tree;
	CCustomButton m_extractSelectedBtn;
	CCustomButton m_extractAllBtn;
	CCustomButton m_prefabNameBtn;
	CStatic m_objectsText;
	int m_type;
};

#endif // __PrefabPanel_h__


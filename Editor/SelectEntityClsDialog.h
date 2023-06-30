////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   selectentityclsdialog.h
//  Version:     v1.00
//  Created:     23/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __selectentityclsdialog_h__
#define __selectentityclsdialog_h__
#pragma once

#include "XTToolkit.h"

// CSelectEntityClsDialog dialog

class CSelectEntityClsDialog : public CXTResizeDialog
{
	DECLARE_DYNAMIC(CSelectEntityClsDialog)

public:
	CSelectEntityClsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectEntityClsDialog();

	CString GetEntityClass() { return m_entityClass; };

// Dialog Data
	enum { IDD = IDD_SELECT_ENTITY_CLASS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnDoubleClick(NMHDR *pNMHDR, LRESULT *pResult);
	BOOL OnInitDialog();
	
	void ReloadEntities();

	DECLARE_MESSAGE_MAP()

	CTreeCtrl m_tree;
	CImageList m_imageList;
	CString m_entityClass;
	std::map<HTREEITEM,CString> m_itemsMap;
};

#endif // __selectentityclsdialog_h__
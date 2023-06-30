////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   toolsconfigpage.h
//  Version:     v1.00
//  Created:     27/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __toolsconfigpage_h__
#define __toolsconfigpage_h__
#pragma once

#include <XTToolkit.h>

/** Tools configuration property page.
*/
class CToolsConfigPage : public CXTResizePropertyPage
{
	DECLARE_DYNAMIC(CToolsConfigPage)

public:
	CToolsConfigPage();
	virtual ~CToolsConfigPage();

// Dialog Data
	enum { IDD = IDD_TOOLSCONFIG };

protected:
	enum Ctrls {
		CTRL_COMMAND,
		CTRL_TOGGLE
	};
	void ReadFromControls( int ctrl );

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	
	afx_msg void OnSelchangeEditList();
	afx_msg void OnChangeEdit1();
	afx_msg void OnToggleVar();
	afx_msg void OnNewItem();

	DECLARE_MESSAGE_MAP()

	struct Tool
	{
		CString command;
		bool bToggle;
	};

	//////////////////////////////////////////////////////////////////////////
	// Vars.
	//////////////////////////////////////////////////////////////////////////
	CStatic	m_txtEdit1;
	CEdit	m_edit1;
	CButton m_toggleVar;
	CXTEditListBox	m_editList;

	std::vector<Tool*> m_tools;
};

#endif // __toolsconfigpage_h__
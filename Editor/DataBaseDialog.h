////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   databasedialog.h
//  Version:     v1.00
//  Created:     21/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __databasedialog_h__
#define __databasedialog_h__
#pragma once

#include "XTToolkit.h"
#include "ToolbarDialog.h"

class CEntityProtLibDialog;
class CBaseLibraryDialog;

/** Main dialog window of DataBase window.
*/
class CDataBaseDialog : public CToolbarDialog
{
	DECLARE_DYNAMIC(CDataBaseDialog)
public:
	CDataBaseDialog( CWnd *pParent = NULL );
	virtual ~CDataBaseDialog();

	enum { IDD = IDD_DATABASE };

	//! Select Object/Terrain
	void Select( int num );
	CBaseLibraryDialog* GetPage( int num );
	int GetSelection() { return m_selectedCtrl; };

	void AddTab( const char *szTitle,CBaseLibraryDialog *wnd );
	CBaseLibraryDialog* GetCurrent();

	//! Called every frame.
	void Update();

protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	void DoDataExchange(CDataExchange* pDX);
	BOOL OnInitDialog();
	BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	//! Activates/Deactivates dialog window.
	void Activate( CBaseLibraryDialog *dlg,bool bActive );

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTabSelect(NMHDR* pNMHDR, LRESULT* pResult);

  DECLARE_MESSAGE_MAP()

	CTabCtrl m_tabCtrl;
	CImageList m_tabImageList;
	std::vector<CBaseLibraryDialog*> m_windows;
	int m_selectedCtrl;

	//////////////////////////////////////////////////////////////////////////
	// Database dialogs.
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Menu.
	CXTMenuBar m_menubar;
};

#endif // __databasedialog_h__

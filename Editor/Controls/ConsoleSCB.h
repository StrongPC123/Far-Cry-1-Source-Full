// ConsoleSCB.h: interface for the CConsoleSCB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONSOLESCB_H__F8855885_1C08_4504_BDA3_7BFB98D381EA__INCLUDED_)
#define AFX_CONSOLESCB_H__F8855885_1C08_4504_BDA3_7BFB98D381EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "sizecbar.h"
#include "scbarg.h"
#include "ConsoleDialog.h"

/** Edit box used for input in Console.
*/
class CConsoleEdit : public CEdit
{
public:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg UINT OnGetDlgCode();

private:
	std::vector<CString> m_history;
};

/** Console class.
*/
class CConsoleSCB : public CWnd
{
public:
	CConsoleSCB();
	virtual ~CConsoleSCB();

	void SetInputFocus();

	// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMyBar)
    //}}AFX_VIRTUAL

protected:
	
	CListBox m_cListBox;
	CEdit m_edit;
	CConsoleEdit m_input;
	CConsoleDialog m_dialog;

	//{{AFX_MSG(CMyBar)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void CConsoleSCB::OnDestroy();
		afx_msg void OnSetFocus( CWnd* pOldWnd );
		afx_msg void OnEditSetFocus();
		afx_msg void OnEditKillFocus();
		virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_CONSOLESCB_H__F8855885_1C08_4504_BDA3_7BFB98D381EA__INCLUDED_)

#pragma once

#include "ToolbarDialog.h"
#include "Controls\PropertyCtrl.h"

class CEAXPresetMgr;

// CSoundPresetsDlg dialog

class CEAXPresetsDlg : public CToolbarDialog
{
	DECLARE_DYNAMIC(CEAXPresetsDlg)

protected:
	CEAXPresetMgr *m_pEAXPresetMgr;
	CDlgToolBar m_cDlgToolBar;
	CListBox m_wndPresets;
	CPropertyCtrl m_wndParams;

protected:
	void InitToolbar();
	void Update();

protected:
	virtual BOOL OnInitDialog();

public:
	CEAXPresetsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEAXPresetsDlg();
	void OnParamsChanged(XmlNodeRef pNode);

// Dialog Data
	enum { IDD = IDD_EAXPRESETS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnUpdateProperties(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLbnSelchangePresets();
	afx_msg void OnSavePreset();
	afx_msg void OnAddPreset();
	afx_msg void OnDelPreset();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
};

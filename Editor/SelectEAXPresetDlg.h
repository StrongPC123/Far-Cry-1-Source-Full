#pragma once

class CEAXPresetMgr;

// CSelectEAXPresetDlg dialog

class CSelectEAXPresetDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelectEAXPresetDlg)

protected:
	CEAXPresetMgr *m_pEAXPresetMgr;
	CListBox m_wndPresets;
	CString m_sCurrPreset;

public:
	CSelectEAXPresetDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectEAXPresetDlg();

// Dialog Data
	enum { IDD = IDD_SELECTPRESET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void SetCurrPreset(CString sPreset) { m_sCurrPreset=sPreset; }
	CString GetCurrPreset() { return m_sCurrPreset; }
	afx_msg void OnLbnSelchangePresets();
};

#pragma once

class CSoundPresetMgr;

// CSelectSoundPresetDlg dialog

class CSelectSoundPresetDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelectSoundPresetDlg)

protected:
	CSoundPresetMgr *m_pSoundPresetMgr;
	CListBox m_wndPresets;
	CString m_sCurrPreset;

public:
	CSelectSoundPresetDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectSoundPresetDlg();

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

#pragma once

#include "ToolbarDialog.h"
#include "Controls\PropertyCtrl.h"

// CSoundPresetsDlg dialog

class CSoundPresetsDlg : public CToolbarDialog
{
	DECLARE_DYNAMIC(CSoundPresetsDlg)

protected:
	CSoundPresetMgr *m_pSoundPresetMgr;
	CDlgToolBar m_cDlgToolBar;
	CListBox m_wndPresets;
	CPropertyCtrl m_wndSounds;

protected:
	void InitToolbar();
	void Update();

protected:
	virtual BOOL OnInitDialog();

public:
	CSoundPresetsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSoundPresetsDlg();
	void OnSoundsChanged(XmlNodeRef pNode);
	void OnSoundsSelChanged(XmlNodeRef pNode);

// Dialog Data
	enum { IDD = IDD_SOUNDPRESETS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnUpdateProperties(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLbnSelchangePresets();
	afx_msg void OnSavePreset();
	afx_msg void OnAddPreset();
	afx_msg void OnDelPreset();
	afx_msg void OnAddSound();
	afx_msg void OnDelSound();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
};

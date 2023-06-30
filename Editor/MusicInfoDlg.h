#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CMusicInfoDlg dialog

class CMusicInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CMusicInfoDlg)

protected:
	CRect m_rcClient;
	CRect m_rcPlayingFrame;
	CRect m_rcPlaying;
	CRect m_rcPlayingClient;
	UINT_PTR m_hTimer;

public:
	CMusicInfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMusicInfoDlg();
	void Resize();

// Dialog Data
	enum { IDD = IDD_MUSICINFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CListCtrl m_wndPlaying;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_wndStreaming;
	CStatic m_wndTheme;
	CStatic m_wndMood;
	CStatic m_wndPlayingFrame;
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

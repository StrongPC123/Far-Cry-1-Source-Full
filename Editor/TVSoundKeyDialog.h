#pragma once

#include "IKeyDlg.h"
#include "afxwin.h"

// CTVSoundKeyDialog dialog

class CTVSoundKeyDialog : public IKeyDlg
{
	DECLARE_DYNAMIC(CTVSoundKeyDialog)
private:
	CCustomButton m_browse;
	CNumberCtrl m_Volume;
	CNumberCtrl m_Pan;
	CNumberCtrl m_InRadius;
	CNumberCtrl m_OutRadius;
	CEdit m_filenameCtrl;
	IAnimTrack* m_track;
	IAnimNode* m_node;
	CButton m_streamBtn;
	CButton m_loopBtn;
	int m_soundType;
	int m_key;
	static CString m_sLastPath;
public:
	CTVSoundKeyDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTVSoundKeyDialog();
	void SetKey( IAnimNode *node,IAnimTrack *track,int nkey );

// Dialog Data
	enum { IDD = IDD_TV_SOUNDKEY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateValue();
	afx_msg void OnBnClickedStereo();
	afx_msg void OnBnClicked3d();
	afx_msg void OnBnClickedStream();
	afx_msg void OnBnClickedLoop();
	afx_msg void OnBnClickedBrowse();

	DECLARE_MESSAGE_MAP()
};

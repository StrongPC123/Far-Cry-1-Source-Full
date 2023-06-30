#pragma once

#include "IKeyDlg.h"

// CTVMusicKeyDialog dialog

class CTVMusicKeyDialog : public IKeyDlg
{
	DECLARE_DYNAMIC(CTVMusicKeyDialog)
private:
	CEdit m_Mood;
	CNumberCtrl m_Time;
	IAnimTrack* m_track;
	IAnimNode* m_node;
	int m_key;
public:
	CTVMusicKeyDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTVMusicKeyDialog();

// Dialog Data
	enum { IDD = IDD_TV_MUSICKEY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void SetKey( IAnimNode *node,IAnimTrack *track,int nkey );
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateValue();
};

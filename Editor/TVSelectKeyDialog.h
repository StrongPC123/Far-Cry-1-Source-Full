#pragma once

#include "IKeyDlg.h"

// CTVSelectKeyDialog dialog

class CTVSelectKeyDialog : public IKeyDlg
{
	DECLARE_DYNAMIC(CTVSelectKeyDialog)

private:
	CComboBox m_name;
	CEdit m_id;
	IAnimTrack* m_track;
	IAnimNode* m_node;
	int m_key;

public:
	CTVSelectKeyDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTVSelectKeyDialog();

// Dialog Data
	enum { IDD = IDD_TV_SELKEY };

protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void SetKey( IAnimNode *node,IAnimTrack *track,int nkey );
	afx_msg void OnUpdateValue();
	afx_msg void OnCbnSelchangeName();
};

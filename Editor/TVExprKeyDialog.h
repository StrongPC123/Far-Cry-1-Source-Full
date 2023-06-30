#pragma once

#include "IKeyDlg.h"

// CTVExprKeyDialog dialog

class CTVExprKeyDialog : public IKeyDlg
{
	DECLARE_DYNAMIC(CTVExprKeyDialog)
private:
	CComboBox m_Name;
	CNumberCtrl m_Amp;
	CNumberCtrl m_BlendIn;
	CNumberCtrl m_Hold;
	CNumberCtrl m_BlendOut;
	IAnimTrack* m_track;
	IAnimNode* m_node;
	int m_key;
	static CString m_sLastPath;
public:
	CTVExprKeyDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTVExprKeyDialog();

// Dialog Data
	enum { IDD = IDD_TV_EXPRKEY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void SetKey( IAnimNode *node,IAnimTrack *track,int nkey );
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateValue();
};

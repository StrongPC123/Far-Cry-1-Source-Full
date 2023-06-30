#pragma once


// CAIAnchorsDialog dialog

class CAIAnchorsDialog : public CDialog
{
	DECLARE_DYNAMIC(CAIAnchorsDialog)

private:
	CString m_sAnchor;
	CListBox m_wndAnchorsList;

public:
	CAIAnchorsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAIAnchorsDialog();

	void SetAIAnchor(const CString &sAnchor) { m_sAnchor=sAnchor; }
	CString GetAIAnchor() { return m_sAnchor; };

// Dialog Data
	enum { IDD = IDD_AIANCHORS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg void OnLbnSelchangeAnchors();
public:
	virtual BOOL OnInitDialog();
};

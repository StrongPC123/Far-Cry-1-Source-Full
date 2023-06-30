#pragma once

#include "Controls\PickObjectButton.h"

class CAIPoint;
// CAIPointPanel dialog

class CAIPointPanel : public CDialog, public IPickObjectCallback
{
	DECLARE_DYNCREATE(CAIPointPanel)

public:
	CAIPointPanel(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAIPointPanel();

	void SetObject( CAIPoint *object );
	void StartPick();

// Dialog Data
	enum { IDD = IDD_PANEL_AIPOINT };

protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedSelect();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnLbnDblclkLinks();
	afx_msg void OnLbnLinksSelChange();

	DECLARE_MESSAGE_MAP()

	void ReloadLinks();
	// Ovverriden from IPickObjectCallback
	virtual void OnPick( CBaseObject *picked );
	virtual bool OnPickFilter( CBaseObject *picked );
	virtual void OnCancelPick();

	CAIPoint* m_object;
	CColoredListBox m_links;

	CPickObjectButton m_pickBtn;
	CCustomButton m_selectBtn;
	CCustomButton m_removeBtn;
	int m_type;
public:
	afx_msg void OnBnClickedWaypoint();
	afx_msg void OnBnClickedHidepoint();
	afx_msg void OnBnClickedEntrypoint();
	afx_msg void OnBnClickedExitpoint();
};

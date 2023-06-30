////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tventitykeyprops.h
//  Version:     v1.00
//  Created:     28/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __tventitykeyprops_h__
#define __tventitykeyprops_h__
#include "afxwin.h"

#if _MSC_VER > 1000
#pragma once
#endif

struct IAnimNode;
struct IAnimTrack;

#include "ikeydlg.h"

// CTVEntityKeyDialog dialog

class CTVEventKeyDialog : public IKeyDlg
{
	DECLARE_DYNAMIC(CTVEventKeyDialog)

public:
	CTVEventKeyDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTVEventKeyDialog();

// Dialog Data
	enum { IDD = IDD_TV_ENTITY_KEY };

	void SetKey( IAnimNode *node,IAnimTrack *track,int key );

protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	IAnimTrack* m_track;
	IAnimNode* m_node;
	int m_key;
public:
	CComboBox m_animation;
	CComboBox m_event;
	virtual BOOL OnInitDialog();
	///CButton m_hide;
	afx_msg void ControlsToKey();
};

#endif // __tventitykeyprops_h__
////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   shapepanel.h
//  Version:     v1.00
//  Created:     28/2/2002 by Timur.
//  Compilers:   Visual C++.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __shapepanel_h__
#define __shapepanel_h__
#pragma once

#include "Controls\PickObjectButton.h"
#include "Controls\ToolButton.h"

class CShapeObject;

// CShapePanel dialog
class CShapePanel : public CDialog, public IPickObjectCallback
{
	DECLARE_DYNAMIC(CShapePanel)

public:
	CShapePanel( CWnd* pParent = NULL);   // standard constructor
	virtual ~CShapePanel();

// Dialog Data
	enum { IDD = IDD_PANEL_SHAPE };

	void SetShape( CShapeObject *shape );
	CShapeObject* GetShape() const { return m_shape; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSelect();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnLbnDblclkEntities();

	virtual void OnOK() {};
	virtual void OnCancel() {};

	// Ovverriden from IPickObjectCallback
	virtual void OnPick( CBaseObject *picked );
	virtual bool OnPickFilter( CBaseObject *picked );
	virtual void OnCancelPick();

	DECLARE_MESSAGE_MAP()

	void ReloadEntities();

	CShapeObject *m_shape;
	CPickObjectButton m_pickButton;
	CCustomButton m_selectButton;
	CToolButton m_editShapeButton;
	CColoredListBox m_entities;
};

#endif // __shapepanel_h__
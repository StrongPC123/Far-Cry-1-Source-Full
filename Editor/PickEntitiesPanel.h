////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   PickEntitiesPanel.h
//  Version:     v1.00
//  Created:     24/10/2002 by Lennert.
//  Compilers:   Visual C++.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PickEntitiesPanel_h__
#define __PickEntitiesPanel_h__
#pragma once

#include "Controls\PickObjectButton.h"
#include "Controls\ToolButton.h"

class CBaseObject;

struct IPickEntitesOwner
{
	virtual void AddEntity(CBaseObject *pEntity) = 0;
	virtual CBaseObject* GetEntity(int nIdx) = 0;
	virtual int GetEntityCount() = 0;
	virtual void RemoveEntity(int nIdx) = 0;
};

// CShapePanel dialog
class CPickEntitiesPanel : public CDialog, public IPickObjectCallback
{
	DECLARE_DYNAMIC(CPickEntitiesPanel)

public:
	CPickEntitiesPanel( CWnd* pParent = NULL);   // standard constructor
	virtual ~CPickEntitiesPanel();

// Dialog Data
	enum { IDD = IDD_PANEL_PICKENTITIES };

	void SetOwner(IPickEntitesOwner *pOwner);
	IPickEntitesOwner* GetOwner() const { return m_pOwner; }

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

	IPickEntitesOwner *m_pOwner;
	CPickObjectButton m_pickButton;
	CCustomButton m_selectButton;
	CColoredListBox m_entities;
};

#endif // __PickEntitiesPanel_h__
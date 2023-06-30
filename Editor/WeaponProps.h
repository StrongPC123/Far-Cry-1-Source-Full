////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   weaponprops.h
//  Version:     v1.00
//  Created:     12/3/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __weaponprops_h__
#define __weaponprops_h__


#include "Controls\PropertyCtrl.h"

#if _MSC_VER > 1000
#pragma once
#endif

// CWeaponProps dialog

class CWeaponProps : public CPropertyPage
{
	DECLARE_DYNAMIC(CWeaponProps)

public:
	CWeaponProps();
	virtual ~CWeaponProps();

// Dialog Data
	enum { IDD = IDD_WEAPONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CPropertyCtrl	m_propWnd;
	XmlNodeRef m_node;
	CString m_title;

protected:
	CListBox m_availableWeapons;
	CListBox m_usedWeapons;

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual BOOL OnQueryCancel();
	virtual void OnReset();
	virtual void OnCancel();

	void OnPropertyChanged( XmlNodeRef node );

	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedRemove();
};

#endif // __weaponprops_h__

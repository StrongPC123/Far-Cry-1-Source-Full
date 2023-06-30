////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   equippackdialog.h
//  Version:     v1.00
//  Created:     27/06/2002 by Lennert Schneider.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Controls\PropertyCtrl.h"
#include "equippack.h"

struct IVariable;

typedef std::list<SEquipment>	TLstString;
typedef TLstString::iterator	TLstStringIt;

// CEquipPackDialog dialog

class CEquipPackDialog : public CDialog
{
	DECLARE_DYNAMIC(CEquipPackDialog)

private:
	CPropertyCtrl m_AmmoPropWnd;
	CComboBox m_EquipPacksList;
	CListBox m_AvailEquipList;
	CListBox m_EquipList;
	CButton m_OkBtn;
	CButton m_ExportBtn;
	CButton m_AddBtn;
	CButton m_DeleteBtn;
	CButton m_RenameBtn;
	CButton m_InsertBtn;
	CButton m_RemoveBtn;
	TLstString m_lstAvailEquip;
	CString m_sCurrEquipPack;
	bool m_bChanged;

private:
	void UpdateEquipPacksList();
	void UpdateEquipPackParams();
	void AmmoUpdateCallback(IVariable *pVar);
	SEquipment* GetEquipment(CString sDesc);

public:
	void SetCurrEquipPack(CString &sValue) { m_sCurrEquipPack=sValue; }
	CString GetCurrEquipPack() { return m_sCurrEquipPack; }

public:
	CEquipPackDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEquipPackDialog();
// Dialog Data
	enum { IDD = IDD_EQUIPPACKS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeEquippack();
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedRename();
	afx_msg void OnBnClickedInsert();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnLbnSelchangeEquipavaillst();
	afx_msg void OnLbnSelchangeEquipusedlst();
	afx_msg void OnBnClickedImport();
	afx_msg void OnBnClickedExport();
	afx_msg void OnDestroy();
};

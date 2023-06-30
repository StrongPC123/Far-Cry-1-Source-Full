////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   aicharactersdialog.h
//  Version:     v1.00
//  Created:     13/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __aicharactersdialog_h__
#define __aicharactersdialog_h__

#if _MSC_VER > 1000
#pragma once
#endif

// CAICharactersDialog dialog

//////////////////////////////////////////////////////////////////////////
class CAICharactersDialog : public CDialog
{
	DECLARE_DYNAMIC(CAICharactersDialog)

public:
	CAICharactersDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAICharactersDialog();

// Dialog Data
	enum { IDD = IDD_AICHARACTERS };

	void SetAICharacter( const CString &chr );
	CString GetAICharacter() { return m_aiCharacter; };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	virtual BOOL OnInitDialog();
	
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedEdit();
	afx_msg void OnBnClickedReload();
	afx_msg void OnLbnSelchangeBehavior();

	void ReloadCharacters();

	//////////////////////////////////////////////////////////////////////////
	// FIELDS
	//////////////////////////////////////////////////////////////////////////
	CListBox m_list;

	CString m_aiCharacter;

	CCustomButton m_editBtn;
	CCustomButton m_reloadBtn;

	CEdit m_description;
};

#endif // __aicharactersdialog_h__
////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   texteditorctrl.h
//  Version:     v1.00
//  Created:     12/2/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __texteditorctrl_h__
#define __texteditorctrl_h__
#pragma once

// CTextEditorCtrl
#include "SyntaxColorizer.h"

class CTextEditorCtrl : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CTextEditorCtrl)
public:
	CTextEditorCtrl();
	virtual ~CTextEditorCtrl();
	
	void LoadFile( const CString &sFileName );
	void SaveFile( const CString &sFileName );
	CString GetFilename() const { return m_filename; }

	void Parse();
	bool IsModified() const { return m_bModified; }

	//! Must be called after OnChange message.
	void OnChange();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();

	CString m_filename;
	CSyntaxColorizer m_sc;
	EDITSTREAM m_es;
	bool m_bModified;
public:
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#endif // __texteditorctrl_h__
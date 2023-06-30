////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   clipboard.h
//  Version:     v1.00
//  Created:     15/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __clipboard_h__
#define __clipboard_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** Use this class to put and get stuff from windows clipboard.
*/
class CRYEDIT_API CClipboard
{
public:
	//! Put xml node into clipboard
	void Put( XmlNodeRef &node,const CString &title = "" );
	//! Get xml node to clipboard.
	XmlNodeRef Get();

	//! Put string into wibndows clipboard.
	void PutString( const CString &text,const CString &title = "" );
	//! Get string from Windows clipboard.
	CString GetString();

	//! Return name of what is in clipboard now.
	CString GetTitle() const { return m_title; };

	//! Return true if clipboard is empty.
	bool IsEmpty() const;
private:
	static XmlNodeRef m_node;
	static CString m_title;
};


#endif // __clipboard_h__

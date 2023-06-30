////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   clipboard.cpp
//  Version:     v1.00
//  Created:     15/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Clipboard.h"

#include <afxole.h>
#include <afxadv.h>

XmlNodeRef CClipboard::m_node;
CString CClipboard::m_title;

//////////////////////////////////////////////////////////////////////////
// Clipboard implementation.
//////////////////////////////////////////////////////////////////////////
void CClipboard::Put(XmlNodeRef &node,const CString &title )
{
	m_title = title;
	if (m_title.IsEmpty())
	{
		m_title = node->getTag();
	}
	m_node = node;

	PutString( m_node->getXML().c_str(),title );

	/*
	COleDataSource	Source;
	CSharedFile	sf(GMEM_MOVEABLE|GMEM_DDESHARE|GMEM_ZEROINIT);
	CString text = node->getXML();

	sf.Write(text, text.GetLength());

	HGLOBAL hMem = sf.Detach();
	if (!hMem)
		return;
	Source.CacheGlobalData(CF_TEXT, hMem);
	Source.SetClipboard();
	*/
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CClipboard::Get()
{
	/*
	COleDataObject	obj;

	if (obj.AttachClipboard()) {
		if (obj.IsDataAvailable(CF_TEXT)) {
			HGLOBAL hmem = obj.GetGlobalData(CF_TEXT);
			CMemFile sf((BYTE*) ::GlobalLock(hmem), ::GlobalSize(hmem));
			CString buffer;

			LPSTR str = buffer.GetBufferSetLength(::GlobalSize(hmem));
			sf.Read(str, ::GlobalSize(hmem));
			::GlobalUnlock(hmem);

			XmlParser parser;
			XmlNodeRef node = parser.parseBuffer( buffer );
			return node;
		}
	}
	return 0;
	*/
	XmlNodeRef node = m_node;
	//m_node = 0;
	return node;
}

//////////////////////////////////////////////////////////////////////////
void CClipboard::PutString( const CString &text,const CString &title /* = ""  */)
{
	if (!OpenClipboard(NULL))
	{
		AfxMessageBox( "Cannot open the Clipboard" );
		return;
	}
	// Remove the current Clipboard contents
	if( !EmptyClipboard() )
	{
		AfxMessageBox( "Cannot empty the Clipboard" );
		return;
	}

	CSharedFile	sf(GMEM_MOVEABLE|GMEM_DDESHARE|GMEM_ZEROINIT);

	sf.Write(text, text.GetLength());

	HGLOBAL hMem = sf.Detach();
	if (!hMem)
		return;

	// For the appropriate data formats...
	if ( ::SetClipboardData( CF_TEXT,hMem ) == NULL )
	{
		AfxMessageBox( "Unable to set Clipboard data" );
		CloseClipboard();
		return;
	}
	CloseClipboard();

	/*
	COleDataSource	Source;
	Source.CacheGlobalData(CF_TEXT, hMem);
	Source.SetClipboard();
	*/
}

//////////////////////////////////////////////////////////////////////////
CString CClipboard::GetString()
{
	COleDataObject	obj;

	if (obj.AttachClipboard()) {
		if (obj.IsDataAvailable(CF_TEXT)) {
			HGLOBAL hmem = obj.GetGlobalData(CF_TEXT);
			CMemFile sf((BYTE*) ::GlobalLock(hmem), ::GlobalSize(hmem));
			CString buffer;

			LPSTR str = buffer.GetBufferSetLength(::GlobalSize(hmem));
			sf.Read(str, ::GlobalSize(hmem));
			::GlobalUnlock(hmem);

			return buffer;
		}
	}
	return "";
}

//////////////////////////////////////////////////////////////////////////
bool CClipboard::IsEmpty() const
{
	if (m_node)
		return false;
	/*
	COleDataObject	obj;
	if (obj.AttachClipboard())
	{
		if (obj.IsDataAvailable(CF_TEXT)) 
		{
			return true;
		}
	}
	*/

	return true;
}
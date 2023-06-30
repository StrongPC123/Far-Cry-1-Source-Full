
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// ScriptObjectStream.h: interface for the CScriptObjectStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTSTREAM_H__1E5E6A84_26DA_443B_B7D5_2E8896772A9B__INCLUDED_)
#define AFX_SCRIPTOBJECTSTREAM_H__1E5E6A84_26DA_443B_B7D5_2E8896772A9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>

class CStream;

/*! This class implements script-functions for exposing the bit stream functionalities

	REMARKS:
	this object doesn't have a global mapping(is not present as global variable into the script state)
		
	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectStream :
public _ScriptableEx<CScriptObjectStream>
{
public:
	CScriptObjectStream();
	virtual ~CScriptObjectStream();
	bool Create(IScriptSystem *pScriptSystem);
	void Attach(CStream *pStm)
	{
		m_pStm=pStm;
	}
public:
	int WriteInt(IFunctionHandler *pH);
	int WriteShort(IFunctionHandler *pH);
	int WriteByte(IFunctionHandler *pH);
	int WriteFloat(IFunctionHandler *pH);
	int WriteString(IFunctionHandler *pH);
	int WriteBool(IFunctionHandler *pH);
	int WriteNumberInBits(IFunctionHandler *pH);

	int ReadInt(IFunctionHandler *pH);
	int ReadShort(IFunctionHandler *pH);
	int ReadByte(IFunctionHandler *pH);
	int ReadFloat(IFunctionHandler *pH);
	int ReadString(IFunctionHandler *pH);
	int ReadBool(IFunctionHandler *pH);
	int ReadNumberInBits(IFunctionHandler *pH);
	static void InitializeTemplate(IScriptSystem *pSS);
public:
	CStream *m_pStm;
};

#endif // !defined(AFX_SCRIPTOBJECTSTREAM_H__1E5E6A84_26DA_443B_B7D5_2E8896772A9B__INCLUDED_)

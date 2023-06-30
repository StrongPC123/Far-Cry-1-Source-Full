
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	File: ScriptObjectStream.cpp
//
//  Description: 
//		ScriptObjectStream.cpp: implementation of the CScriptObjectStream class.
//
//	History: 
//		- created by Marco C.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptObjectStream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
_DECLARE_SCRIPTABLEEX(CScriptObjectStream)


CScriptObjectStream::CScriptObjectStream()
{

}

CScriptObjectStream::~CScriptObjectStream()
{
	
}

bool CScriptObjectStream::Create(IScriptSystem *pScriptSystem)
{
	m_pStm=NULL;

	Init(pScriptSystem,this);
	return true;
}

void CScriptObjectStream::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectStream>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectStream,WriteInt);
	REG_FUNC(CScriptObjectStream,WriteShort);
	REG_FUNC(CScriptObjectStream,WriteByte);
	REG_FUNC(CScriptObjectStream,WriteFloat);
	REG_FUNC(CScriptObjectStream,WriteString);
	REG_FUNC(CScriptObjectStream,WriteBool);
	REG_FUNC(CScriptObjectStream,WriteNumberInBits);
	REG_FUNC(CScriptObjectStream,ReadInt);
	REG_FUNC(CScriptObjectStream,ReadShort);
	REG_FUNC(CScriptObjectStream,ReadByte);
	REG_FUNC(CScriptObjectStream,ReadFloat);
	REG_FUNC(CScriptObjectStream,ReadString);
	REG_FUNC(CScriptObjectStream,ReadBool);
	REG_FUNC(CScriptObjectStream,ReadNumberInBits);
}

int CScriptObjectStream::WriteInt(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int n;
	pH->GetParam(1,n);
	m_pStm->Write(n);
	return pH->EndFunction();
}

int CScriptObjectStream::WriteShort(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int n;
	short int si;
	pH->GetParam(1,n);
	si=n;
	m_pStm->Write(si);
	return pH->EndFunction();
}

int CScriptObjectStream::WriteByte(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int n;
	unsigned char uc;
	pH->GetParam(1,n);
	uc=n;
	m_pStm->Write(uc);
	return pH->EndFunction();
}

int CScriptObjectStream::WriteFloat(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float f;
	pH->GetParam(1,f);
	m_pStm->Write(f);
	return pH->EndFunction();
}

int CScriptObjectStream::WriteString(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *s;
	pH->GetParam(1,s);
	m_pStm->Write(s);
	return pH->EndFunction();
}

int CScriptObjectStream::WriteBool(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	bool b;
	pH->GetParam(1,b);
	m_pStm->Write(b);
	return pH->EndFunction();
}

int CScriptObjectStream::WriteNumberInBits(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int n;
	int nNumOfBits;
	pH->GetParam(1,n);
	pH->GetParam(2,nNumOfBits);
	m_pStm->WriteNumberInBits(n,nNumOfBits);
	return pH->EndFunction();
}

int CScriptObjectStream::ReadInt(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	int n;

	if(!m_pStm->Read(n))
		return pH->EndFunctionNull();

	return pH->EndFunction(n);
}

int CScriptObjectStream::ReadShort(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	short int n;

	if(!m_pStm->Read(n))
		return pH->EndFunctionNull();

	return pH->EndFunction((int)n);
}

int CScriptObjectStream::ReadByte(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	unsigned char c;

	if(!m_pStm->Read(c))
		return pH->EndFunctionNull();

	return pH->EndFunction((int)c);
}

int CScriptObjectStream::ReadFloat(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	float f;

	if(!m_pStm->Read(f))
		return pH->EndFunctionNull();

	return pH->EndFunction(f);
}

int CScriptObjectStream::ReadString(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	char sTemp[256];

	if(!m_pStm->Read(sTemp,256))
		return pH->EndFunctionNull();

	return pH->EndFunction(sTemp);
}

int CScriptObjectStream::ReadBool(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	bool b;

	if(!m_pStm->Read(b))
		return pH->EndFunctionNull();

	return pH->EndFunction(b);
}

int CScriptObjectStream::ReadNumberInBits(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	unsigned int n;
	int nNumOfBits;
	pH->GetParam(1,nNumOfBits);

	if(nNumOfBits<1 || nNumOfBits>32)
		CryError( "CScriptObjectStream::ReadNumberInBits(%d) failed",nNumOfBits );

	if(!m_pStm->ReadNumberInBits(n,nNumOfBits))
		return pH->EndFunctionNull();

	return pH->EndFunction((int)n);
}
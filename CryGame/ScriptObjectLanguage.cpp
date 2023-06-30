
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	File: ScriptObjectLanguage.cpp
//
//  Description: 
//		ScriptObjectLanguage.cpp: implementation of the CScriptObjectLanguage class.
//
//	History: 
//		- created by Marco C.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptObjectLanguage.h"
#include "StringTableMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define REG_FUNC(_class,_func) _class::RegisterFunction(pSS,#_func,&_class::_func);

_DECLARE_SCRIPTABLEEX(CScriptObjectLanguage)

//////////////////////////////////////////////////////////////////////
CScriptObjectLanguage::CScriptObjectLanguage()
{

}

CScriptObjectLanguage::~CScriptObjectLanguage()
{

}

//////////////////////////////////////////////////////////////////////
void CScriptObjectLanguage::Init(IScriptSystem *pScriptSystem,CStringTableMgr *pMgr)
{
	InitGlobal(pScriptSystem,"Language",this);
	
	m_pMgr=pMgr;
}

void CScriptObjectLanguage::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectLanguage>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectLanguage,LoadStringTable);
	REG_FUNC(CScriptObjectLanguage,GetEnglish);
}

//////////////////////////////////////////////////////////////////////
/*! Add a new string into the string table
		@param s the new string value
		@param nID the numeric id representing the string
*/
void CScriptObjectLanguage::AddString(const char *s,int nID)
{
	m_pScriptThis->SetValue(s,nID);
}

//////////////////////////////////////////////////////////////////////
/*! retrieve the id of a specified string
		@param szKey the string value
		@return the numeric id representing the string
*/
int CScriptObjectLanguage::GetStringID(const char *szKey)
{
	int nRes;
	if (m_pScriptThis->GetValue(szKey,nRes))
		return (nRes);

	return (-1);
}

int CScriptObjectLanguage::GetEnglish(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	char *szKey = 0;

	pH->GetParam(1, szKey);				// don't check for return - szKey still might be 0

	if(!szKey)
		return pH->EndFunctionNull();

	wstring weng;

	m_pMgr->Localize(szKey, weng, 1);

	std::vector<char> tmp;
	tmp.resize(weng.size()+1);

	sprintf (&tmp[0], "%S", weng.c_str());

	return pH->EndFunction(&tmp[0]);
}

//////////////////////////////////////////////////////////////////////
/*! load a string table from disk and add all contents into the global string table
		@param sString the file path of the string table
		@return !=nil(succeded) nil(failed)
*/
int CScriptObjectLanguage::LoadStringTable(IFunctionHandler *pH)
{
	if (pH->GetParamCount()!=1)
	{
		m_pScriptSystem->RaiseError("Language::LoadStringTable wrong number of parameters");
		pH->EndFunctionNull();
	}

	const char *sString;

	pH->GetParam(1, sString);				// don't check for return - sString still might be 0

	if(!sString)
		return pH->EndFunctionNull();

	return pH->EndFunction(m_pMgr->LoadStringTable(sString));
}






#include "stdafx.h"
#include "ScriptObjectSynched2DTable.h"		// CScriptObjectSynched2DTable
#include "Synched2DTable.h"								// CSynched2DTable

_DECLARE_SCRIPTABLEEX(CScriptObjectSynched2DTable)


void CScriptObjectSynched2DTable::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectSynched2DTable>::InitializeTemplate(pSS);

	REG_FUNC(CScriptObjectSynched2DTable,SetEntryXY);
	REG_FUNC(CScriptObjectSynched2DTable,GetEntryXY);
	REG_FUNC(CScriptObjectSynched2DTable,GetEntryFloatXY);
	REG_FUNC(CScriptObjectSynched2DTable,SetEntriesY);
	REG_FUNC(CScriptObjectSynched2DTable,GetLineCount);
	REG_FUNC(CScriptObjectSynched2DTable,GetColumnCount);
}

bool CScriptObjectSynched2DTable::Create(IScriptSystem *pScriptSystem, CSynched2DTable *pSynched2DTable)
{
	m_pSynched2DTable=pSynched2DTable;

	Init(pScriptSystem, this);
	m_pScriptThis->RegisterParent(this);
	// Function-Registration
	return true;
}


int CScriptObjectSynched2DTable::SetEntryXY(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	int iX,iY;
	float fValue;

	if(!pH->GetParam(1,iX))
		return pH->EndFunction();
	if(!pH->GetParam(2,iY))
		return pH->EndFunction();

	if(pH->GetParam(3,fValue))
	{
		m_pSynched2DTable->SetEntryXYFloat(iX,iY,fValue);
		return pH->EndFunction();
	}
	else
	{
		char *szValue;

		if(!pH->GetParam(3,szValue))
			return pH->EndFunction();

		if(szValue)
			m_pSynched2DTable->SetEntryXYString(iX,iY,szValue);
	}

	return pH->EndFunction();
}


int CScriptObjectSynched2DTable::SetEntriesY(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int iY;
	float fValue;

	if(!pH->GetParam(1,iY))
		return pH->EndFunction();

	if(pH->GetParam(2,fValue))
	{
		m_pSynched2DTable->SetEntriesYFloat(iY,fValue);
		return pH->EndFunction();
	}
	else
	{
		char *szValue;

		if(!pH->GetParam(2,szValue))
			return pH->EndFunction();

		if(szValue)
			m_pSynched2DTable->SetEntriesYString(iY,szValue);
	}

	return pH->EndFunction();
}


int CScriptObjectSynched2DTable::GetEntryFloatXY(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	int iX,iY;

	if(!pH->GetParam(1,iX))
		return pH->EndFunction();
	if(!pH->GetParam(2,iY))
		return pH->EndFunction();

	CSynched2DTable::STableEntry ref = m_pSynched2DTable->m_EntryTable.GetXY(iX,iY);

	if(ref.IsFloat())
		return pH->EndFunction(ref.GetFloat());
	 else
		return pH->EndFunction(0.0f);
}


int CScriptObjectSynched2DTable::GetEntryXY(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	
	int iX,iY;
	
	if(!pH->GetParam(1,iX))
		return pH->EndFunction();
	if(!pH->GetParam(2,iY))
		return pH->EndFunction();

	CSynched2DTable::STableEntry ref = m_pSynched2DTable->m_EntryTable.GetXY(iX,iY);

	if(ref.IsFloat())
		return pH->EndFunction(ref.GetFloat());
	else
	{
		static char szOutBuffer[256];

		strcpy(szOutBuffer,ref.GetString().c_str());

		return pH->EndFunction(szOutBuffer);
	}
}


int CScriptObjectSynched2DTable::GetLineCount(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction((int)m_pSynched2DTable->GetLineCount());
}

int CScriptObjectSynched2DTable::GetColumnCount(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction((int)m_pSynched2DTable->GetColumnCount());
}

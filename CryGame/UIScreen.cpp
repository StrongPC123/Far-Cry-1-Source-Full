//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - Manage a list of widgets
//
// History:
//  - [11/7/2003] created the file
//
//-------------------------------------------------------------------------------------------------


#include "StdAfx.h"
#include "UIScreen.h"
#include "UISystem.h"



_DECLARE_SCRIPTABLEEX(CUIScreen);


//------------------------------------------------------------------------------------------------- 
CUIScreen::CUIScreen()
: m_pUISystem(0), m_bActive(0)
{
	m_hOnInit=
	m_hOnUpdate=
	m_hOnRelease=
	m_hOnActivate=
	m_hOnDeactivate=0;
}

//------------------------------------------------------------------------------------------------- 
CUIScreen::~CUIScreen()
{
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::Release()
{
	return m_pUISystem->DestroyScreen(this);
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::GetScriptFunctionPtrs()
{
	GetScriptObject()->GetValue("OnInit", m_hOnInit);
	GetScriptObject()->GetValue("OnUpdate", m_hOnUpdate);
	GetScriptObject()->GetValue("OnRelease", m_hOnRelease);
	GetScriptObject()->GetValue("OnActivate", m_hOnActivate);
	GetScriptObject()->GetValue("OnDeactivate", m_hOnDeactivate);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::ReleaseScriptFunctionPtrs()
{
	m_pScriptSystem->ReleaseFunc(m_hOnInit);
	m_hOnInit = 0;

	m_pScriptSystem->ReleaseFunc(m_hOnUpdate);
	m_hOnUpdate = 0;

	m_pScriptSystem->ReleaseFunc(m_hOnRelease);
	m_hOnRelease = 0;	
	
	m_pScriptSystem->ReleaseFunc(m_hOnActivate);
	m_hOnActivate = 0;

	m_pScriptSystem->ReleaseFunc(m_hOnDeactivate);
	m_hOnDeactivate = 0;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
void CUIScreen::InitializeTemplate(IScriptSystem *pScriptSystem)
{
	_ScriptableEx<CUIScreen>::InitializeTemplate(pScriptSystem);

	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScreen, Release);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScreen, GetName);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScreen, GetWidgetCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScreen, GetWidget);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScreen, AddWidget);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScreen, DelWidget);
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::SetName(const string &szName)
{
	m_szName = szName;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
string &CUIScreen::GetName()
{
	return m_szName;
}

//------------------------------------------------------------------------------------------------- 
string  CUIScreen::GetClassName()
{
	return UICLASSNAME_SCREEN;
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::AddWidget(CUIWidget *pWidget)
{
	CUIWidgetItor pItor = std::find(m_vWidgetList.begin(), m_vWidgetList.end(), pWidget);

	if (pItor != m_vWidgetList.end())
	{
		return 0;
	}

	m_vWidgetList.push_back(pWidget);

	pWidget->m_pScreen = this;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::AddWidget(const string &szName)
{
	CUIWidget *pWidget = m_pUISystem->GetWidget(szName);

	if (pWidget)
	{
		return AddWidget(pWidget);
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::DelWidget(CUIWidget *pWidget)
{
	for (CUIWidgetItor pItor = m_vWidgetList.begin(); pItor != m_vWidgetList.end(); pItor++)
	{
		if (*pItor == pWidget)
		{
			m_vWidgetList.erase(pItor);

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::DelWidget(int iIndex)
{
	return DelWidget(m_vWidgetList[iIndex]);
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::DelWidget(const string &szName)
{
	for (CUIWidgetItor pItor = m_vWidgetList.begin(); pItor != m_vWidgetList.end(); pItor++)
	{
		if ((*pItor)->GetName() == szName)
		{
			(*pItor)->Release();
			m_vWidgetList.erase(pItor);

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
CUIWidgetList *CUIScreen::GetWidgetList()
{
	return &m_vWidgetList;
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUIScreen::GetWidget(int iIndex)
{
	return m_vWidgetList[iIndex];
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUIScreen::GetWidget(const string &szName)
{
	for (CUIWidgetItor pItor = m_vWidgetList.begin(); pItor != m_vWidgetList.end(); ++pItor)
	{
		if ((*pItor)->GetName() == szName)
		{
			return *pItor;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int	CUIScreen::GetWidgetCount()
{
	return m_vWidgetList.size();
}


//------------------------------------------------------------------------------------------------- 
// Script Functions
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
int CUIScreen::Release(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Release, 0);

	Release();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::GetName(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetName, 0);

	return pH->EndFunction(GetName().c_str());
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::GetWidgetCount(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetWidgetCount, 0);

	return pH->EndFunction(GetWidgetCount());
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::GetWidget(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetWidget, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), GetWidget, 1, svtString, svtNumber);


	CUIWidget *pWidget = 0;

	if (pH->GetParamType(1) == svtString)
	{
		char *szValue;

		pH->GetParam(1, szValue);

    pWidget = GetWidget(szValue);
	}
	else
	{
		int iIndex;

		pH->GetParam(1, iIndex);

		pWidget = GetWidget(iIndex);
	}

	if (pWidget)
	{
		pH->EndFunction(m_pUISystem->GetWidgetScriptObject(pWidget));
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::AddWidget(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), AddWidget, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), AddWidget, 1, svtString, svtObject);

	CUIWidget *pWidget = 0;

	if (pH->GetParamType(1) == svtString)
	{
		char *szName;

		pH->GetParam(1, szName);

		pWidget = m_pUISystem->GetWidget(szName);
	}
	else
	{
		IScriptObject *pObject = m_pScriptSystem->CreateEmptyObject();

		pH->GetParam(1, pObject);

		pWidget = (CUIWidget *)pObject->GetNativeData();

		pObject->Release();
	}

	if (pWidget)
	{
		return pH->EndFunction(AddWidget(pWidget));
	}
	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::DelWidget(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), DelWidget, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), DelWidget, 1, svtString, svtObject);

	CUIWidget *pWidget = 0;

	if (pH->GetParamType(1) == svtString)
	{
		char *szName;

		pH->GetParam(1, szName);

		pWidget = m_pUISystem->GetWidget(szName);
	}
	else
	{
		IScriptObject *pObject = m_pScriptSystem->CreateEmptyObject();

		pH->GetParam(1, pObject);

		pWidget = (CUIWidget *)pObject->GetNativeData();

		pObject->Release();
	}

	if (pWidget)
	{
		return pH->EndFunction(DelWidget(pWidget));
	}
	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::OnInit()
{
	GetScriptFunctionPtrs();

	IScriptSystem		*pScriptSystem = m_pUISystem->GetIScriptSystem();
	HSCRIPTFUNCTION pScriptFunction = m_hOnInit;

	if (!pScriptFunction)
	{
		return 1;
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(GetScriptObject());
	pScriptSystem->EndCall(iResult);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::OnUpdate()
{
	IScriptSystem		*pScriptSystem = m_pUISystem->GetIScriptSystem();
	HSCRIPTFUNCTION pScriptFunction = m_hOnUpdate;

	if (!pScriptFunction)
	{
		return 1;
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(GetScriptObject());
	pScriptSystem->EndCall(iResult);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::OnRelease()
{
	IScriptSystem		*pScriptSystem = m_pUISystem->GetIScriptSystem();
	HSCRIPTFUNCTION pScriptFunction = m_hOnRelease;

	if (!pScriptFunction)
	{
		ReleaseScriptFunctionPtrs();

		return 1;
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(GetScriptObject());
	pScriptSystem->EndCall(iResult);

	ReleaseScriptFunctionPtrs();

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::OnActivate()
{
	IScriptSystem		*pScriptSystem = m_pUISystem->GetIScriptSystem();
	HSCRIPTFUNCTION pScriptFunction = m_hOnActivate;

	if (!pScriptFunction)
	{
		return 1;
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(GetScriptObject());
	pScriptSystem->EndCall(iResult);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIScreen::OnDeactivate()
{
	IScriptSystem		*pScriptSystem = m_pUISystem->GetIScriptSystem();
	HSCRIPTFUNCTION pScriptFunction = m_hOnDeactivate;

	if (!pScriptFunction)
	{
		return 1;
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(GetScriptObject());
	pScriptSystem->EndCall(iResult);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
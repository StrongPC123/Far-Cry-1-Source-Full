//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - Make UI functions available from script as UI:Func(param)
//
// History:
//  - [3/7/2003] created the file
//
//-------------------------------------------------------------------------------------------------



#include "StdAfx.h"
#include "ScriptObjectUI.h"
#include "UIStatic.h"
#include "UIButton.h"
#include "UIEditBox.h"
#include "UIScrollBar.h"
#include "UIListView.h"
#include "UICheckBox.h"
#include "UIComboBox.h"
#include "UIVideoPanel.h"

#include "UIScreen.h"



_DECLARE_SCRIPTABLEEX(CScriptObjectUI);

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::Create(CUISystem *pUISystem)
{
	m_pUISystem = pUISystem;
	m_pLog = pUISystem->GetISystem()->GetILog();

	IScriptSystem *pScriptSystem = pUISystem->GetIScriptSystem();
	
	pScriptSystem->SetGlobalToNull("UI");

	InitGlobal(pScriptSystem, "UI", this);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetScriptFunctionPtrs()
{
	m_hCanRenderGame		= m_pScriptSystem->GetFunctionPtr("UI", "CanRenderGame");
	m_hCanSwitch				= m_pScriptSystem->GetFunctionPtr("UI", "CanSwitch");
	m_hOnSwitch					= m_pScriptSystem->GetFunctionPtr("UI", "OnSwitch");
	m_hOnInit						= m_pScriptSystem->GetFunctionPtr("UI", "OnInit");
	m_hOnRelease				= m_pScriptSystem->GetFunctionPtr("UI", "OnRelease");
	m_hOnUpdate					= m_pScriptSystem->GetFunctionPtr("UI", "OnUpdate");
	m_hOnDrawBackground = m_pScriptSystem->GetFunctionPtr("UI", "OnDrawBackground");
	m_hOnDrawMouseCursor= m_pScriptSystem->GetFunctionPtr("UI", "OnDrawMouseCursor");
	m_hOnIdle						= m_pScriptSystem->GetFunctionPtr("UI", "OnIdle");

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ReleaseScriptFunctionPtrs()
{
	m_pScriptSystem->ReleaseFunc(m_hCanRenderGame);
	m_hCanRenderGame = 0;

	m_pScriptSystem->ReleaseFunc(m_hCanSwitch);
	m_hCanSwitch = 0;

	m_pScriptSystem->ReleaseFunc(m_hOnSwitch);
	m_hOnSwitch = 0;

	m_pScriptSystem->ReleaseFunc(m_hOnInit);
	m_hOnInit = 0;

	m_pScriptSystem->ReleaseFunc(m_hOnRelease);
	m_hOnRelease = 0;

	m_pScriptSystem->ReleaseFunc(m_hOnUpdate);
	m_hOnUpdate = 0;

	m_pScriptSystem->ReleaseFunc(m_hOnDrawBackground);
	m_hOnDrawBackground = 0;

	m_pScriptSystem->ReleaseFunc(m_hOnDrawMouseCursor);
	m_hOnDrawMouseCursor = 0;

	m_pScriptSystem->ReleaseFunc(m_hOnIdle);
	m_hOnIdle = 0;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
void CScriptObjectUI::InitializeTemplate(IScriptSystem *pScriptSystem)
{
	_ScriptableEx<CScriptObjectUI>::InitializeTemplate(pScriptSystem);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, Release);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, Reload);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetWidget);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetWidgetCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ShowWidget);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, HideWidget);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, IsWidgetVisible);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, EnableWidget);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, DisableWidget);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, IsWidgetEnabled);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SendMessage);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, BroadcastMessage);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetBackground);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetBackground);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetBackgroundColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetBackgroundColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ShowBackground);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, HideBackground);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, IsBackgroundVisible);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetMouseXY);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetMouseXY);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetMouseCursor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetMouseCursor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetMouseCursorColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetMouseCursorColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetMouseCursorSize);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetMouseCursorWidth);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetMouseCursorHeight);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ShowMouseCursor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, HideMouseCursor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, IsMouseCursorVisible);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetToolTipColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetToolTipColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetToolTipBorderColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetToolTipBorderColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetToolTipBorderSize);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetToolTipBorderSize);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetToolTipBorderStyle);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetToolTipBorderStyle);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetToolTipFontName);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetToolTipFontName);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetToolTipFontEffect);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetToolTipFontEffect);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetToolTipFontColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetToolTipFontColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetToolTipFontSize);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetToolTipFontSize);\

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, CaptureMouse);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ReleaseMouse);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ExtractRed);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ExtractGreen);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ExtractBlue);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ExtractAlpha);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ExtractLeft);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ExtractTop);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ExtractWidth);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ExtractHeight);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetMouseX);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetMouseY);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetTopMostWidget);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetTopMostWidget);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetFocus);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetFocus);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, SetFocusScreen);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetFocusScreen);

	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, FirstTabStop);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, LastTabStop);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, NextTabStop);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, PrevTabStop);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, CreateObjectFromTable);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, CreateScreenFromTable);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetScreenCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetScreen);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ActivateScreen);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, DeactivateScreen);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, IsScreenActive);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, GetActiveScreenCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, DeactivateAllScreens);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, ActivateAllScreens);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, IsScreenActive);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, Disable);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, Enable);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, IsEnabled);

	//------------------------------------------------------------------------------------------------- 
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CScriptObjectUI, StopAllVideo);

	// intialize the script constants
	InitializeConstants(pScriptSystem);
}

//------------------------------------------------------------------------------------------------- 
void CScriptObjectUI::InitializeConstants(IScriptSystem *pScriptSystem)
{
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIFLAG_VISIBLE);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIFLAG_ENABLED);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIFLAG_MOVEABLE);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIFLAG_CANHAVEFOCUS);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIFLAG_CANCHANGEZ);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIFLAG_DEFAULT);

	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTYLE_TRANSPARENT);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTYLE_SHADOWED);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTYLE_MULTILINE);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTYLE_WORDWRAP);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTYLE_MULTISELECTION);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTYLE_PASSWORD);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTYLE_DEFAULT);

	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIALIGN_CENTER);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIALIGN_LEFT);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIALIGN_RIGHT);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIALIGN_MIDDLE);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIALIGN_TOP);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIALIGN_BOTTOM);

	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIBORDERSTYLE_NONE);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIBORDERSTYLE_FLAT);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIBORDERSTYLE_RAISED);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIBORDERSTYLE_SUNKEN);

	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTATE_UP);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTATE_DOWN);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTATE_OVER);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISTATE_CHECKED);

	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISORT_ASCENDING);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISORT_DESCENDING);

	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIBLEND_ADDITIVE);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UIBLEND_OVERLAY);

	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISCROLLBARTYPE_AUTOMATIC);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISCROLLBARTYPE_HORIZONTAL);
	REGISTER_SCRIPT_CONSTANT(pScriptSystem, UISCROLLBARTYPE_VERTICAL);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::Release()
{
	m_pUISystem->Release();
	m_pUISystem =0 ;
	m_pLog = 0;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
// Callback functions
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::OnSwitch(bool bIn)
{
	HSCRIPTFUNCTION pScriptFunction = m_hOnSwitch;

	if (!pScriptFunction)
	{
		return 1;
	}

	int iResult = 1;

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(this->GetScriptObject());
	m_pScriptSystem->PushFuncParam(bIn);
	m_pScriptSystem->EndCall(iResult);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::CanRenderGame()
{
	HSCRIPTFUNCTION pScriptFunction = m_hCanRenderGame;

	if (!pScriptFunction)
	{
		return 1;
	}

	bool bResult = 1;

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(this->GetScriptObject());
	m_pScriptSystem->EndCall(bResult);

	return (bResult ? 1 : 0);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::CanSwitch(bool bIn)
{
	HSCRIPTFUNCTION pScriptFunction = m_hCanSwitch;

	if (!pScriptFunction)
	{
		return 1;
	}

	bool bResult = 1;

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(this->GetScriptObject());
	m_pScriptSystem->PushFuncParam(bIn);
	m_pScriptSystem->EndCall(bResult);

	return (bResult ? 1 : 0);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::OnInit()
{
	GetScriptFunctionPtrs();

	HSCRIPTFUNCTION pScriptFunction = m_hOnInit;

	if (!pScriptFunction)
	{
		return 1;
	}

	int iResult = 1;

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(this->GetScriptObject());
	m_pScriptSystem->EndCall(iResult);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::OnRelease()
{
	HSCRIPTFUNCTION pScriptFunction = m_hOnRelease;

	if (!pScriptFunction)
	{
		ReleaseScriptFunctionPtrs();

		return 1;
	}

	int iResult = 1;

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(this->GetScriptObject());
	m_pScriptSystem->EndCall(iResult);

	ReleaseScriptFunctionPtrs();

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::OnUpdate()
{
	HSCRIPTFUNCTION pScriptFunction = m_hOnUpdate;

	if (!pScriptFunction)
	{
		return 1;
	}

	int iResult = 1;

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(this->GetScriptObject());
	m_pScriptSystem->EndCall(iResult);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::OnDrawBackground()
{
	HSCRIPTFUNCTION pScriptFunction = m_hOnDrawBackground;

	if (!pScriptFunction)
	{
		return 1;
	}

	int iResult = 1;

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(this->GetScriptObject());
	m_pScriptSystem->EndCall(iResult);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::OnDrawMouseCursor()
{
	HSCRIPTFUNCTION pScriptFunction = m_hOnDrawMouseCursor;

	if (!pScriptFunction)
	{
		return 1;
	}

	int iResult = 1;

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(this->GetScriptObject());
	m_pScriptSystem->EndCall(iResult);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::OnIdle(float fIdleTime)
{
	HSCRIPTFUNCTION pScriptFunction = m_hOnIdle;

	if (!pScriptFunction)
	{
		return 1;
	}

	int iResult = 1;

	m_pScriptSystem->BeginCall(pScriptFunction);
	m_pScriptSystem->PushFuncParam(this->GetScriptObject());
	m_pScriptSystem->PushFuncParam(fIdleTime);
	m_pScriptSystem->EndCall(iResult);

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
// Script Functions
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::Release(IFunctionHandler *pH)
{
	Release();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::Reload(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", Reload, 0, 1);

	if (pH->GetParamCount() == 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", Reload, 1, svtNumber);

		int iFrameDelta = 0;

		pH->GetParam(1, iFrameDelta);

		m_pUISystem->Reload(iFrameDelta);
	}
	else
	{
		m_pUISystem->Reload(0);
	}
	
	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetWidget(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", GetWidget, 1, 2);

	char *szWidgetName = 0;
	char *szScreenName = 0;

	CUIScreen *pScreen = 0;
	CUIWidget *pWidget = 0;

	if (pH->GetParamCount() >= 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", GetWidget, 1, svtString);

		pH->GetParam(1, szWidgetName);
	}

	if (pH->GetParamCount() == 2)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", GetWidget, 2, svtString, svtObject);

		if (pH->GetParamType(2) == svtString)
		{
			pH->GetParam(2, szScreenName);
		}
		else
		{
			IScriptObject *pObj = m_pScriptSystem->CreateEmptyObject();

			pH->GetParam(2, pObj);

			pScreen = (CUIScreen *)pObj->GetNativeData();

			pObj->Release();
		}
	}

	if ((szScreenName) && (szWidgetName))
	{
		pWidget = m_pUISystem->GetWidget(szWidgetName, szScreenName);
	}
	else if ((pScreen) && (szWidgetName))
	{
		pWidget = pScreen->GetWidget(szWidgetName);
	}
	else if (szWidgetName)
	{
		pWidget = m_pUISystem->GetWidget(szWidgetName);
	}

	if (pWidget)
	{
		return pH->EndFunction(m_pUISystem->GetWidgetScriptObject(pWidget));
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetWidgetCount(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetWidgetCount, 0);

	return pH->EndFunction(m_pUISystem->GetWidgetCount());
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ShowWidget(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", ShowWidget, 1, 2);

	CUIWidget *pWidget = 0;
	char			*szWidgetName = 0;
	char			*szScreenName = 0;

	if (pH->GetParamCount() == 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", ShowWidget, 1, svtObject, svtString);

		if (pH->GetParamType(1) == svtObject)
		{
			IScriptObject *pObject = m_pScriptSystem->CreateEmptyObject();

			pH->GetParam(1, pObject);

			pWidget = (CUIWidget *)pObject->GetNativeData();

			pObject->Release();
		}
		else
		{
			pH->GetParam(1, szWidgetName);

			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}
	else
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", ShowWidget, 1, svtString);
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", ShowWidget, 2, svtString);

		pH->GetParam(1, szWidgetName);
		pH->GetParam(2, szScreenName);

		if (szScreenName && szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName, szScreenName);
		}
		else if (szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}

	if (pWidget)
	{
		m_pUISystem->ShowWidget(pWidget);
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::HideWidget(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", HideWidget, 1, 2);

	CUIWidget *pWidget = 0;
	char			*szWidgetName = 0;
	char			*szScreenName = 0;

	if (pH->GetParamCount() == 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", HideWidget, 1, svtObject, svtString);

		if (pH->GetParamType(1) == svtObject)
		{
			IScriptObject *pObject = m_pScriptSystem->CreateEmptyObject();

			pH->GetParam(1, pObject);

			pWidget = (CUIWidget *)pObject->GetNativeData();

			pObject->Release();
		}
		else
		{
			pH->GetParam(1, szWidgetName);

			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}
	else
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", HideWidget, 1, svtString);
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", HideWidget, 2, svtString);

		pH->GetParam(1, szWidgetName);
		pH->GetParam(2, szScreenName);

		if (szScreenName && szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName, szScreenName);
		}
		else if (szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}

	if (pWidget)
	{
		m_pUISystem->HideWidget(pWidget);
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::IsWidgetVisible(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", IsWidgetVisible, 1, 2);

	CUIWidget *pWidget = 0;
	char			*szWidgetName = 0;
	char			*szScreenName = 0;

	if (pH->GetParamCount() == 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", IsWidgetVisible, 1, svtObject, svtString);

		if (pH->GetParamType(1) == svtObject)
		{
			IScriptObject *pObject = m_pScriptSystem->CreateEmptyObject();

			pH->GetParam(1, pObject);

			pWidget = (CUIWidget *)pObject->GetNativeData();

			pObject->Release();
		}
		else
		{
			pH->GetParam(1, szWidgetName);

			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}
	else
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", IsWidgetVisible, 1, svtString);
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", IsWidgetVisible, 2, svtString);

		pH->GetParam(1, szWidgetName);
		pH->GetParam(2, szScreenName);

		if (szScreenName && szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName, szScreenName);
		}
		else if (szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}

	if (pWidget)
	{
		return pH->EndFunction(m_pUISystem->IsWidgetVisible(pWidget) != 0);
	}

	return pH->EndFunction(false);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::EnableWidget(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", EnableWidget, 1, 2);

	CUIWidget *pWidget = 0;
	char			*szWidgetName = 0;
	char			*szScreenName = 0;

	if (pH->GetParamCount() == 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", EnableWidget, 1, svtObject, svtString);

		if (pH->GetParamType(1) == svtObject)
		{
			IScriptObject *pObject = m_pScriptSystem->CreateEmptyObject();

			pH->GetParam(1, pObject);

			pWidget = (CUIWidget *)pObject->GetNativeData();

			pObject->Release();
		}
		else
		{
			pH->GetParam(1, szWidgetName);

			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}
	else
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", EnableWidget, 1, svtString);
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", EnableWidget, 2, svtString);

		pH->GetParam(1, szWidgetName);
		pH->GetParam(2, szScreenName);

		if (szScreenName && szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName, szScreenName);
		}
		else if (szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}

	if (pWidget)
	{
		m_pUISystem->EnableWidget(pWidget);
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::DisableWidget(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", DisableWidget, 1, 2);

	CUIWidget *pWidget = 0;
	char			*szWidgetName = 0;
	char			*szScreenName = 0;

	if (pH->GetParamCount() == 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", DisableWidget, 1, svtObject, svtString);

		if (pH->GetParamType(1) == svtObject)
		{
			IScriptObject *pObject = m_pScriptSystem->CreateEmptyObject();

			pH->GetParam(1, pObject);

			pWidget = (CUIWidget *)pObject->GetNativeData();

			pObject->Release();
		}
		else
		{
			pH->GetParam(1, szWidgetName);

			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}
	else
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", DisableWidget, 1, svtString);
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", DisableWidget, 2, svtString);

		pH->GetParam(1, szWidgetName);
		pH->GetParam(2, szScreenName);

		if (szScreenName && szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName, szScreenName);
		}
		else if (szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}

	if (pWidget)
	{
		m_pUISystem->DisableWidget(pWidget);
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::IsWidgetEnabled(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", IsWidgetEnabled, 1, 2);

	CUIWidget *pWidget = 0;
	char			*szWidgetName = 0;
	char			*szScreenName = 0;

	if (pH->GetParamCount() == 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", IsWidgetEnabled, 1, svtObject, svtString);

		if (pH->GetParamType(1) == svtObject)
		{
			IScriptObject *pObject = m_pScriptSystem->CreateEmptyObject();

			pH->GetParam(1, pObject);

			pWidget = (CUIWidget *)pObject->GetNativeData();

			pObject->Release();
		}
		else
		{
			pH->GetParam(1, szWidgetName);

			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}
	else
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", IsWidgetEnabled, 1, svtString);
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", IsWidgetEnabled, 2, svtString);

		pH->GetParam(1, szWidgetName);
		pH->GetParam(2, szScreenName);

		if (szScreenName && szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName, szScreenName);
		}
		else if (szWidgetName)
		{
			pWidget = m_pUISystem->GetWidget(szWidgetName);
		}
	}

	if (pWidget)
	{
		return pH->EndFunction(m_pUISystem->IsWidgetEnabled(pWidget) != 0);
	}

	return pH->EndFunction(false);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SendMessage(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", SendMessage, 4);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", SendMessage, 1, svtString, svtObject);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SendMessage, 2, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SendMessage, 3, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SendMessage, 4, svtNumber);

	CUIWidget *pWidget = 0;

	if (pH->GetParamType(1) == svtString)
	{
		char *szWidget;

		pH->GetParam(1, szWidget);

		pWidget = m_pUISystem->GetWidget(szWidget);
	}
	else 
	{
		IScriptObject *pWidgetObject = 0;

		pH->GetParam(1, pWidgetObject);

		pWidget = (CUIWidget *)pWidgetObject->GetNativeData();
	}

	if (!pWidget)
	{
		return pH->EndFunctionNull();
	}

	int iMessage;
	int wParam;
	int lParam;

	pH->GetParam(2, iMessage);
	pH->GetParam(3, wParam);
	pH->GetParam(4, lParam);

	// AMD64 port note: the below code is only valid as long as we don't return handles or pointers received via SendMessage
	return pH->EndFunction((int)m_pUISystem->SendMessage(pWidget, iMessage, wParam, lParam));
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::BroadcastMessage(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", BroadcastMessage, 3);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", BroadcastMessage, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", BroadcastMessage, 2, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", BroadcastMessage, 3, svtNumber);

	int iMessage;
	int wParam;
	int lParam;

	pH->GetParam(1, iMessage);
	pH->GetParam(2, wParam);
	pH->GetParam(3, lParam);

	return pH->EndFunction((int)m_pUISystem->BroadcastMessage(iMessage, wParam, lParam));
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetBackground(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", SetBackground, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SetBackground, 1, svtUserData);

	int iCookie = 0;
	INT_PTR iTextureID = -1;

	pH->GetParamUDVal(1, iTextureID, iCookie);

	m_pUISystem->SetBackground(iTextureID);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetBackground(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetBackground, 0);

	USER_DATA pUserData = m_pScriptSystem->CreateUserData((int)m_pUISystem->GetBackground(), USER_DATA_TEXTURE);

	return pH->EndFunction(pUserData);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetBackgroundColor(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", SetBackgroundColor, 1, 4);

	if ((pH->GetParamCount() == 1) && (pH->GetParamType(1) == svtString))
	{
		color4f cColor;
		char *szColor;

		pH->GetParam(1, szColor);

		m_pUISystem->RetrieveColor(&cColor, szColor);

		m_pUISystem->SetBackgroundColor(cColor);
	}
	else if ((pH->GetParamCount() == 4) && (pH->GetParamType(1) == svtNumber) && (pH->GetParamType(4) == svtNumber))
	{
		color4f cColor;

		int iColor;

		pH->GetParam(1, iColor);
		cColor.v[0] = iColor * (1.0f / 255.0f);
		pH->GetParam(2, iColor);
		cColor.v[1] = iColor * (1.0f / 255.0f);
		pH->GetParam(3, iColor);
		cColor.v[2] = iColor * (1.0f / 255.0f);
		pH->GetParam(4, iColor);
		cColor.v[3] = iColor * (1.0f / 255.0f);

		m_pUISystem->SetBackgroundColor(cColor);
	}
	else
	{
		m_pScriptSystem->RaiseError("UI:SetBackgroundColor() Wrong type in parameter 1! Expected 'String' or 'Number', but found '%s'!", GET_SCRIPT_TYPE_STRING(pH->GetParamType(1)));
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetBackgroundColor(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetBackgroundColor, 0);

	char szColor[24];
	color4f cColor;

	m_pUISystem->GetBackgroundColor(&cColor);
	m_pUISystem->ConvertToString(szColor, cColor);

	return pH->EndFunction(szColor);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ShowBackground(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ShowBackground, 0);

	m_pUISystem->ShowBackground();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::HideBackground(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ShowBackground, 0);

	m_pUISystem->HideBackground();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::IsBackgroundVisible(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", IsBackgroundVisible, 0);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetMouseXY(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", SetMouseXY, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SetMouseXY, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SetMouseXY, 2, svtNumber);

	float fX = 0.0f;
	float fY = 0.0f;

	pH->GetParam(1, fX);
	pH->GetParam(2, fY);

	m_pUISystem->SetMouseXY(fX, fY);

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetMouseXY(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetMouseXY, 0);

	vector2f vMouseXY = m_pUISystem->GetMouseXY();

	return pH->EndFunction(vMouseXY.x, vMouseXY.y);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetMouseCursor(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", SetMouseCursor, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SetMouseCursor, 1, svtUserData);

	int iCookie = 0;
	INT_PTR iTextureID = -1;

	pH->GetParamUDVal(1, iTextureID, iCookie);

	m_pUISystem->SetMouseCursor((int)iTextureID);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetMouseCursor(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", SetMouseCursor, 0);

	USER_DATA pUserData = m_pScriptSystem->CreateUserData((int)m_pUISystem->GetMouseCursor(), USER_DATA_TEXTURE);

	return pH->EndFunction(pUserData);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetMouseCursorColor(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", SetMouseColorColor, 1, 4);

	if ((pH->GetParamCount() == 1) && (pH->GetParamType(1) == svtString))
	{
		color4f cColor;
		char *szColor;

		pH->GetParam(1, szColor);

		m_pUISystem->RetrieveColor(&cColor, szColor);

		m_pUISystem->SetMouseCursorColor(cColor);
	}
	else if ((pH->GetParamCount() == 4) && (pH->GetParamType(1) == svtNumber) && (pH->GetParamType(4) == svtNumber))
	{
		color4f cColor;

		int iColor;

		pH->GetParam(1, iColor);
		cColor.v[0] = iColor * (1.0f / 255.0f);
		pH->GetParam(2, iColor);
		cColor.v[1] = iColor * (1.0f / 255.0f);
		pH->GetParam(3, iColor);
		cColor.v[2] = iColor * (1.0f / 255.0f);
		pH->GetParam(4, iColor);
		cColor.v[3] = iColor * (1.0f / 255.0f);

		m_pUISystem->SetMouseCursorColor(cColor);
	}
	else
	{
		m_pScriptSystem->RaiseError("UI:SetMouseCursorColor() Wrong type in parameter 1! Expected 'String' or 'Number', but found '%s'!", GET_SCRIPT_TYPE_STRING(pH->GetParamType(1)));

		return pH->EndFunctionNull();
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetMouseCursorColor(IFunctionHandler *pH)
{
	char szColor[64];
	color4f cColor;

	m_pUISystem->GetMouseCursorColor(&cColor);
	m_pUISystem->ConvertToString(szColor, cColor);

	return pH->EndFunction(szColor);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetMouseCursorSize(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", SetMouseCursorSize, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SetMouseCursorSize, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SetMouseCursorSize, 2, svtNumber);

	float fWidth, fHeight;

	pH->GetParam(1, fWidth);
	pH->GetParam(2, fHeight);

	m_pUISystem->SetMouseCursorSize(fWidth, fHeight);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetMouseCursorWidth(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetMouseCursorWidth, 0);

	float fWidth;

	m_pUISystem->GetMouseCursorSize(&fWidth, 0);

	return pH->EndFunction(fWidth);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetMouseCursorHeight(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetMouseCursorHeight, 0);

	float fHeight;

	m_pUISystem->GetMouseCursorSize(&fHeight, 0);

	return pH->EndFunction(fHeight);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ShowMouseCursor(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ShowMouseCursor, 0);

	m_pUISystem->ShowMouseCursor();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::HideMouseCursor(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", HideMouseCursor, 0);

	m_pUISystem->HideMouseCursor();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::IsMouseCursorVisible(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", IsMouseCursorVisible, 0);

	return pH->EndFunction(m_pUISystem->IsMouseCursorVisible());
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetGreyedColor(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", SetGreyedColor, 1, 4);

	if ((pH->GetParamCount() == 1) && (pH->GetParamType(1) == svtString))
	{
		color4f cColor;
		char *szColor;

		pH->GetParam(1, szColor);

		m_pUISystem->RetrieveColor(&cColor, szColor);

		m_pUISystem->SetBackgroundColor(cColor);
	}
	else if ((pH->GetParamCount() == 4) && (pH->GetParamType(1) == svtNumber) && (pH->GetParamType(4) == svtNumber))
	{
		color4f cColor;

		int iColor;

		pH->GetParam(1, iColor);
		cColor.v[0] = iColor * (1.0f / 255.0f);
		pH->GetParam(2, iColor);
		cColor.v[1] = iColor * (1.0f / 255.0f);
		pH->GetParam(3, iColor);
		cColor.v[2] = iColor * (1.0f / 255.0f);
		pH->GetParam(4, iColor);
		cColor.v[3] = iColor * (1.0f / 255.0f);

		m_pUISystem->SetGreyedColor(cColor);
	}
	else
	{
		m_pScriptSystem->RaiseError("UI:SetGreyedColor() Wrong type in parameter 1! Expected 'String' or 'Number', but found '%s'!", GET_SCRIPT_TYPE_STRING(pH->GetParamType(1)));
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetGreyedColor(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetGreyedColor, 0);
	char szColor[24];
	color4f cColor;

	m_pUISystem->GetGreyedColor(&cColor);
	m_pUISystem->ConvertToString(szColor, cColor);

	return pH->EndFunction(szColor);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::CaptureMouse(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ReleaseMouse, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", ReleaseMouse, 1, svtObject, svtString);

	CUIWidget *pWidget = 0;

	if (pH->GetParamType(1) == svtString)
	{
		char *szWidget;

		pH->GetParam(1, szWidget);

		pWidget = m_pUISystem->GetWidget(szWidget);
	}
	else
	{
		IScriptObject *pWidgetObject = 0;

		pH->GetParam(1, pWidgetObject);

		pWidget = (CUIWidget *)pWidgetObject->GetNativeData();
	}

	if (!pWidget)
	{
		return pH->EndFunction(0);
	}

	return pH->EndFunction(m_pUISystem->CaptureMouse(pWidget));
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ReleaseMouse(IFunctionHandler *pH)
{
	m_pUISystem->ReleaseMouse();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ExtractRed(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ExtractRed, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", ExtractRed, 1, svtString);

	char *szColor;
	color4f cColor;

	pH->GetParam(1, szColor);

	m_pUISystem->RetrieveColor(&cColor, szColor);

	return pH->EndFunction((int)(cColor.v[0] * 255));
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ExtractGreen(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ExtractGreen, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", ExtractGreen, 1, svtString);

	char *szColor;
	color4f cColor;

	pH->GetParam(1, szColor);

	m_pUISystem->RetrieveColor(&cColor, szColor);

	return pH->EndFunction((int)(cColor.v[1] * 255));
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ExtractBlue(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ExtractBlue, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", ExtractBlue, 1, svtString);

	char *szColor;
	color4f cColor;

	pH->GetParam(1, szColor);

	m_pUISystem->RetrieveColor(&cColor, szColor);

	return pH->EndFunction((int)(cColor.v[2] * 255));
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ExtractAlpha(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ExtractAlpha, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", ExtractAlpha, 1, svtString);

	char *szColor;
	color4f cColor;

	pH->GetParam(1, szColor);

	m_pUISystem->RetrieveColor(&cColor, szColor);

	return pH->EndFunction((int)(cColor.v[3] * 255));
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ExtractLeft(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ExtractLeft, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", ExtractLeft, 1, svtString);

	char *szRect;
	UIRect pRect;

	pH->GetParam(1, szRect);

	m_pUISystem->RetrieveRect(&pRect, szRect);

	return pH->EndFunction(pRect.fLeft);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ExtractTop(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ExtractTop, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", ExtractTop, 1, svtString);

	char *szRect;
	UIRect pRect;

	pH->GetParam(1, szRect);

	m_pUISystem->RetrieveRect(&pRect, szRect);

	return pH->EndFunction(pRect.fTop);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ExtractWidth(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ExtractWidth, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", ExtractWidth, 1, svtString);

	char *szRect;
	UIRect pRect;

	pH->GetParam(1, szRect);

	m_pUISystem->RetrieveRect(&pRect, szRect);

	return pH->EndFunction(pRect.fWidth);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ExtractHeight(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ExtractHeight, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", ExtractHeight, 1, svtString);

	char *szRect;
	UIRect pRect;

	pH->GetParam(1, szRect);

	m_pUISystem->RetrieveRect(&pRect, szRect);

	return pH->EndFunction(pRect.fHeight);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetMouseX(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetMouseX, 0);

	vector2f vXY = m_pUISystem->GetMouseXY();

	return pH->EndFunction(vXY.x);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetMouseY(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetMouseY, 0);

	vector2f vXY = m_pUISystem->GetMouseXY();

	return pH->EndFunction(vXY.y);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetTopMostWidget(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", SetTopMostWidget, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", SetTopMostWidget, 1, svtString, svtObject);

	CUIWidget *pWidget;

	if (pH->GetParamType(1) == svtString)
	{
		char *szWidgetName;

		pH->GetParam(1, szWidgetName);

		pWidget = m_pUISystem->GetWidget(szWidgetName);
	}
	else if (pH->GetParamType(1) == svtObject)
	{
		IScriptObject *pScriptObject = 0;

		pH->GetParam(1, pScriptObject);

		pWidget = (CUIWidget *)pScriptObject->GetNativeData();
	}

	if (pWidget)
	{
		m_pUISystem->SetTopMostWidget(pWidget);
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetTopMostWidget(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetTopMostWidget, 0);

	return pH->EndFunction(m_pUISystem->GetWidgetScriptObject(m_pUISystem->GetFocus()));
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetFocus(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", SetFocus, 1, 2);

	CUIWidget *pWidget;

	if (pH->GetParamCount() == 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SetFocus, 1, svtObject);

		IScriptObject *pScriptObject = m_pScriptSystem->CreateEmptyObject();

		pH->GetParam(1, pScriptObject);

		pWidget = (CUIWidget *)pScriptObject->GetNativeData();

		pScriptObject->Release();
	}
	else
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", SetFocus, 1, svtString);
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", SetFocus, 2, svtString, svtObject);

		char *szWidgetName;
		char *szScreenName;

		pH->GetParam(1, szWidgetName);

		if (pH->GetParamType(2) == svtObject)
		{
			IScriptObject *pScriptObject = m_pScriptSystem->CreateEmptyObject();

			pH->GetParam(2, pScriptObject);

			CUIScreen *pScreen = (CUIScreen *)pScriptObject->GetNativeData();

			pScriptObject->Release();

			pWidget = pScreen->GetWidget(szWidgetName);
		}
		else
		{
			pH->GetParam(2, szScreenName);

			pWidget = m_pUISystem->GetWidget(szWidgetName, szScreenName);
		}
	}

	m_pUISystem->SetFocus(pWidget);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetFocus(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetFocus, 0);

	CUIWidget *pFocus = m_pUISystem->GetFocus();

	if (!pFocus)
	{
		return pH->EndFunctionNull();
	}

	return pH->EndFunction(m_pUISystem->GetWidgetScriptObject(pFocus));
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetFocusScreen(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", SetFocusScreen, 1, 0);

	if (pH->GetParamCount())
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", SetFocusScreen, 1, svtString, svtObject);
	}
	
	CUIScreen *pScreen = 0;

	if (pH->GetParamCount())
	{
		if (pH->GetParamType(1) == svtString)
		{
			char *szName;

			pH->GetParam(1, szName);

			if (szName && strlen(szName) > 0)
			{
				pScreen = m_pUISystem->GetScreen(szName);

				if (!pScreen)
				{
					m_pLog->LogToConsole("\001$4[Error]:$1 Tried to set focusscreen to screen '%s' which was not found!", szName);
				}
			}
		}
		else
		{
			IScriptObject *pObj = m_pScriptSystem->CreateEmptyObject();

			pH->GetParam(1, pObj);

			pScreen = (CUIScreen *)pObj->GetNativeData();

			if (!pScreen)
			{
				m_pLog->LogToConsole("\001$4[Error]:$1 Tried to set focusscreen to a screen which was not found!");
			}
		}
	}

	m_pUISystem->SetFocusScreen(pScreen);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetFocusScreen(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetFocusScreen, 0);

	return pH->EndFunction(m_pUISystem->GetFocusScreen()->GetScriptObject());
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::FirstTabStop(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", FirstTabStop, 0);

	m_pUISystem->FirstTabStop();

	return pH->EndFunction();
}

int CScriptObjectUI::LastTabStop(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", LastTabStop, 0);

	m_pUISystem->LastTabStop();

	return pH->EndFunction();
}

int CScriptObjectUI::NextTabStop(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", NextTabStop, 0);

	m_pUISystem->NextTabStop();

	return pH->EndFunction();
}

int CScriptObjectUI::PrevTabStop(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", PrevTabStop, 0);

	m_pUISystem->PrevTabStop();

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::CreateObjectFromTable(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, "UI", CreateStatic, 2, 3);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", CreateStatic, 1, svtString);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", CreateStatic, 2, svtObject);

	CUIScreen *pScreen = 0;

	if (pH->GetParamCount() == 3)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", CreateStatic, 2, svtObject, svtString);

		if (pH->GetParamType(3) == svtObject)
		{
			IScriptObject *pObject = m_pScriptSystem->CreateEmptyObject();

			pH->GetParam(3, pObject);

			pScreen = (CUIScreen *)pObject->GetNativeData();

			pObject->Release();
		}
		else
		{
			char *szScreenName = 0;

			pH->GetParam(3, szScreenName);

			if (szScreenName)
			{
				pScreen = m_pUISystem->GetScreen(szScreenName);
			}
		}
	}

	char *szName = 0;
	IScriptObject *pScriptObject = m_pScriptSystem->CreateEmptyObject();

	pH->GetParam(1, szName);
	pH->GetParam(2, pScriptObject);

	CUIWidget *pWidget;
	
	// create the widget
	if (!m_pUISystem->CreateObjectFromTable(&pWidget, 0, pScreen, pScriptObject, szName))
	{
		return pH->EndFunctionNull();
	}

	return pH->EndFunction(m_pUISystem->GetWidgetScriptObject(pWidget));
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::CreateScreenFromTable(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", CreateScreenFromTable, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", CreateScreenFromTable, 1, svtString);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, "UI", CreateScreenFromTable, 2, svtObject);

	char *szName;
	IScriptObject *pObject = m_pScriptSystem->CreateEmptyObject();;

	pH->GetParam(1, szName);
	pH->GetParam(2, pObject);

	CUIScreen *pScreen;

	if (!m_pUISystem->CreateScreenFromTable(&pScreen, szName, pObject))
	{
		pObject->Release();

		m_pLog->LogToConsole("\001$4[Error]:$1 Failed to create screen '%s'...", szName);

		return pH->EndFunctionNull();
	}

	pObject->Release();

	return pH->EndFunction(pScreen->GetScriptObject());
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetScreenCount(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetScreenCount, 0);

	return pH->EndFunction(m_pUISystem->GetScreenCount());
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetScreen(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetScreen, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", GetScreen, 1, svtString, svtNumber);

	CUIScreen *pScreen = 0;

	if (pH->GetParamType(1) == svtString)
	{
		char *szName;

		pH->GetParam(1, szName);

		pScreen = m_pUISystem->GetScreen(szName);

		if (!pScreen)
		{
			m_pLog->LogToConsole("\001$4[Error]:$1 Screen '%s' was not found!", szName);
		}
	}
	else
	{
		int iScreenIndex;

		pH->GetParam(1, iScreenIndex);

		pScreen = m_pUISystem->GetScreen(iScreenIndex);

		if (!pScreen)
		{
			m_pLog->LogToConsole("\001$4[Error]:$1 Screen number '%d' was not found!", iScreenIndex);
		}
	}

	if (pScreen)
	{
		return pH->EndFunction(pScreen->GetScriptObject());
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ActivateScreen(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ActivateScreen, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", ActivateScreen, 1, svtString, svtObject);

	CUIScreen *pScreen = 0;

	if (pH->GetParamType(1) == svtString)
	{
		char *szName;

		pH->GetParam(1, szName);

		pScreen = m_pUISystem->GetScreen(szName);

		if (!pScreen)
		{
			m_pLog->LogToConsole("\001$4[Error]:$1 Tried to activate screen '%s' which was not found!", szName);
		}
	}
	else
	{
		IScriptObject *pObj = m_pScriptSystem->CreateEmptyObject();

		pH->GetParam(1, pObj);

		pScreen = (CUIScreen *)pObj->GetNativeData();

		if (!pScreen)
		{
			m_pLog->LogToConsole("\001$4[Error]:$1 Tried to activate a screen which was not found!");
		}
	}

	if (pScreen)
	{
		m_pUISystem->ActivateScreen(pScreen);
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::DeactivateScreen(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", DeactivateScreen, 1);
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", DeactivateScreen, 1, svtString, svtObject);

	CUIScreen *pScreen = 0;

	if (pH->GetParamType(1) == svtString)
	{
		char *szName;

		pH->GetParam(1, szName);

		pScreen = m_pUISystem->GetScreen(szName);

		if (!pScreen)
		{
			m_pLog->LogToConsole("\001$4[Error]:$1 Tried to Deactivate screen '%s' which was not found!", szName);
		}
	}
	else
	{
		IScriptObject *pObj = m_pScriptSystem->CreateEmptyObject();

		pH->GetParam(1, pObj);

		pScreen = (CUIScreen *)pObj->GetNativeData();

		if (!pScreen)
		{
			m_pLog->LogToConsole("\001$4[Error]:$1 Tried to Deactivate a screen which was not found!");
		}
	}

	if (pScreen)
	{
		m_pUISystem->DeactivateScreen(pScreen);
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::IsScreenActive(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", IsScreenActive, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, "UI", IsScreenActive, 1, svtString, svtObject);

	CUIScreen *pScreen = 0;

	if (pH->GetParamType(1) == svtString)
	{
		char *szName;

		pH->GetParam(1, szName);

		pScreen = m_pUISystem->GetScreen(szName);

		if (!pScreen)
		{
			m_pLog->LogToConsole("\001$4[Error]:$1 Tried to check for screen '%s' which was not found!", szName);
		}
	}
	else
	{
		IScriptObject *pObj = m_pScriptSystem->CreateEmptyObject();

		pH->GetParam(1, pObj);

		pScreen = (CUIScreen *)pObj->GetNativeData();

		if (!pScreen)
		{
			m_pLog->LogToConsole("\001$4[Error]:$1 Tried to check for a screen which was not found!");
		}
	}

	if (pScreen)
	{
		return pH->EndFunction(m_pUISystem->IsScreenActive(pScreen));
	}

	return pH->EndFunction(false);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetActiveScreenCount(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", GetActiveScreenCount, 0);

	return pH->EndFunction(m_pUISystem->GetActiveScreenCount());
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::DeactivateAllScreens(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", DeactivateAllScreens, 0);

	m_pUISystem->DeactivateAllScreens();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::ActivateAllScreens(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, "UI", ActivateAllScreens, 0);

	m_pUISystem->ActivateAllScreens();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::Enable(IFunctionHandler *pH)
{
	m_pUISystem->Enable();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::Disable(IFunctionHandler *pH)
{
	m_pUISystem->Disable();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::IsEnabled(IFunctionHandler *pH)
{
	return pH->EndFunction(m_pUISystem->IsEnabled());
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::StopAllVideo(IFunctionHandler *pH)
{
	m_pUISystem->StopAllVideo();

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetToolTipColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", SetToolTipColor, m_pUISystem->m_cToolTipColor);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetToolTipColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", GetToolTipColor, m_pUISystem->m_cToolTipColor);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetToolTipBorderColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", SetToolTipFontColor, m_pUISystem->m_pToolTipBorder.cColor);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetToolTipBorderColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", GetToolTipBorderColor, m_pUISystem->m_pToolTipBorder.cColor);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetToolTipBorderSize(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", SetToolTipBorderSize, m_pUISystem->m_pToolTipBorder.fSize);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetToolTipBorderSize(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", GetToolTipBorderSize, m_pUISystem->m_pToolTipBorder.fSize);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetToolTipBorderStyle(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", SetToolTipBorderStyle, m_pUISystem->m_pToolTipBorder.iStyle);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetToolTipBorderStyle(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", GetToolTipBorderStyle, m_pUISystem->m_pToolTipBorder.iStyle);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetToolTipFontName(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), "UI", SetToolTipFontName, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pUISystem->GetIScriptSystem(), "UI", SetToolTipFontName, 1, svtString);

	char *szFontName;

	pH->GetParam(1, szFontName);

	m_pUISystem->m_pToolTipFont.szFaceName = szFontName;

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetToolTipFontName(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), "UI", GetToolTipFontName, 0);

	return pH->EndFunction(m_pUISystem->m_pToolTipFont.szFaceName.c_str());
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetToolTipFontEffect(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), "UI", SetToolTipFontEffect, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pUISystem->GetIScriptSystem(), "UI", SetToolTipFontEffect, 1, svtString);

	char *szFontEffect;

	pH->GetParam(1, szFontEffect);

	m_pUISystem->m_pToolTipFont.szEffectName = szFontEffect;

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetToolTipFontEffect(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pUISystem->GetIScriptSystem(), "UI", GetToolTipFontEffect, 0);

	return pH->EndFunction(m_pUISystem->m_pToolTipFont.szEffectName.c_str());
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetToolTipFontColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", SetToolTipFontColor, m_pUISystem->m_pToolTipFont.cColor);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetToolTipFontColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", GetToolTipFontColor, m_pUISystem->m_pToolTipFont.cColor);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::SetToolTipFontSize(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", SetToolTipFontSize, m_pUISystem->m_pToolTipFont.fSize);
}

//------------------------------------------------------------------------------------------------- 
int CScriptObjectUI::GetToolTipFontSize(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pUISystem->GetIScriptSystem(), "UI", GetToolTipFontSize, m_pUISystem->m_pToolTipFont.fSize);
}

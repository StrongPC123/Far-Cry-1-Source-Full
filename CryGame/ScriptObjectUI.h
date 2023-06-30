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
#pragma once



#include "UISystem.h"


class CScriptObjectUI : public _ScriptableEx<CScriptObjectUI>
{
public:

	CScriptObjectUI(): m_pUISystem(0), m_pLog(0)
	{
		m_hCanRenderGame=
		m_hCanSwitch=
		m_hOnSwitch=
		m_hOnInit=
		m_hOnRelease=
		m_hOnUpdate=
		m_hOnDrawBackground=
		m_hOnDrawMouseCursor=
		m_hOnIdle=0;
	};
	~CScriptObjectUI() {};

	int Create(CUISystem *pUISystem);
	int Release();

	int GetScriptFunctionPtrs();
	int ReleaseScriptFunctionPtrs();


	static void InitializeTemplate(IScriptSystem *pScriptSystem);
	static void InitializeConstants(IScriptSystem *pScriptSystem);
  //------------------------------------------------------------------------------------------------- 
	// Callback Functions, called by UISystem
	//------------------------------------------------------------------------------------------------- 

	int CanRenderGame();
	int CanSwitch(bool bIn);
	int OnSwitch(bool bIn);
	int OnInit();
	int OnRelease();
	int OnUpdate();
	int OnDrawBackground();
	int OnDrawMouseCursor();
	int OnIdle(float fIdleTime);

	//------------------------------------------------------------------------------------------------- 
	// Script Functions
	//------------------------------------------------------------------------------------------------- 
	//------------------------------------------------------------------------------------------------- 
	int Release(IFunctionHandler *pH);

	int Reload(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int GetWidget(IFunctionHandler *pH);
	int GetWidgetCount(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int ShowWidget(IFunctionHandler *pH);
	int HideWidget(IFunctionHandler *pH);
	int IsWidgetVisible(IFunctionHandler *pH);
	int EnableWidget(IFunctionHandler *pH);
	int DisableWidget(IFunctionHandler *pH);
	int IsWidgetEnabled(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int SendMessage(IFunctionHandler *pH);
	int BroadcastMessage(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int SetBackground(IFunctionHandler *pH);
	int GetBackground(IFunctionHandler *pH);
	int SetBackgroundColor(IFunctionHandler *pH);
	int GetBackgroundColor(IFunctionHandler *pH);
	int ShowBackground(IFunctionHandler *pH);
	int HideBackground(IFunctionHandler *pH);
	int IsBackgroundVisible(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int SetMouseXY(IFunctionHandler *pH);
	int GetMouseXY(IFunctionHandler *pH);
	int SetMouseCursor(IFunctionHandler *pH);
	int GetMouseCursor(IFunctionHandler *pH);
	int SetMouseCursorColor(IFunctionHandler *pH);
	int GetMouseCursorColor(IFunctionHandler *pH);
	int SetMouseCursorSize(IFunctionHandler *pH);
	int GetMouseCursorWidth(IFunctionHandler *pH);
	int GetMouseCursorHeight(IFunctionHandler *pH);
	int ShowMouseCursor(IFunctionHandler *pH);
	int HideMouseCursor(IFunctionHandler *pH);
	int IsMouseCursorVisible(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int SetGreyedColor(IFunctionHandler *pH);
	int GetGreyedColor(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int CaptureMouse(IFunctionHandler *pH);
	int ReleaseMouse(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int ExtractRed(IFunctionHandler *pH);
	int ExtractGreen(IFunctionHandler *pH);
	int ExtractBlue(IFunctionHandler *pH);
	int ExtractAlpha(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int ExtractLeft(IFunctionHandler *pH);
	int ExtractTop(IFunctionHandler *pH);
	int ExtractWidth(IFunctionHandler *pH);
	int ExtractHeight(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int GetMouseX(IFunctionHandler *pH);
	int GetMouseY(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int SetTopMostWidget(IFunctionHandler *pH);
	int GetTopMostWidget(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int SetFocus(IFunctionHandler *pH);
	int GetFocus(IFunctionHandler *pH);
	int SetFocusScreen(IFunctionHandler *pH);
	int GetFocusScreen(IFunctionHandler *pH);
	int FirstTabStop(IFunctionHandler *pH);
	int LastTabStop(IFunctionHandler *pH);
	int NextTabStop(IFunctionHandler *pH);
	int PrevTabStop(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int CreateObjectFromTable(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int CreateScreenFromTable(IFunctionHandler *pH);
	int GetScreenCount(IFunctionHandler *pH);
	int GetScreen(IFunctionHandler *pH);
	int ActivateScreen(IFunctionHandler *pH);
	int DeactivateScreen(IFunctionHandler *pH);
	int IsScreenActive(IFunctionHandler *pH);
	int GetActiveScreenCount(IFunctionHandler *pH);
	int DeactivateAllScreens(IFunctionHandler *pH);
	int ActivateAllScreens(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int Disable(IFunctionHandler *pH);
	int Enable(IFunctionHandler *pH);
	int IsEnabled(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int StopAllVideo(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int SetToolTipColor(IFunctionHandler *pH);
	int GetToolTipColor(IFunctionHandler *pH);

	int SetToolTipBorderColor(IFunctionHandler *pH);
	int GetToolTipBorderColor(IFunctionHandler *pH);

	int SetToolTipBorderSize(IFunctionHandler *pH);
	int GetToolTipBorderSize(IFunctionHandler *pH);

	int SetToolTipBorderStyle(IFunctionHandler *pH);
	int GetToolTipBorderStyle(IFunctionHandler *pH);

	int SetToolTipFontName(IFunctionHandler *pH);
	int GetToolTipFontName(IFunctionHandler *pH);

	int SetToolTipFontEffect(IFunctionHandler *pH);
	int GetToolTipFontEffect(IFunctionHandler *pH);

	int SetToolTipFontColor(IFunctionHandler *pH);
	int GetToolTipFontColor(IFunctionHandler *pH);

	int SetToolTipFontSize(IFunctionHandler *pH);
	int GetToolTipFontSize(IFunctionHandler *pH);

private:

	ILog			*m_pLog;
	CUISystem	*m_pUISystem;

	HSCRIPTFUNCTION m_hCanRenderGame;
	HSCRIPTFUNCTION m_hCanSwitch;
	HSCRIPTFUNCTION m_hOnSwitch;
	HSCRIPTFUNCTION m_hOnInit;
	HSCRIPTFUNCTION m_hOnRelease;
	HSCRIPTFUNCTION m_hOnUpdate;
	HSCRIPTFUNCTION m_hOnDrawBackground;
	HSCRIPTFUNCTION m_hOnDrawMouseCursor;
	HSCRIPTFUNCTION m_hOnIdle;
};
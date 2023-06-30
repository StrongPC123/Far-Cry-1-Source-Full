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
#pragma once



#define UICLASSNAME_SCREEN	"UIScreen"



#include "UIWidget.h"



class CUIScreen : public _ScriptableEx<CUIScreen>
{
	friend class CUISystem;

public:

	CUIScreen();
	~CUIScreen();

	int Release();
	int GetScriptFunctionPtrs();
	int ReleaseScriptFunctionPtrs();

	static void InitializeTemplate(IScriptSystem *pScriptSystem);

	int SetName(const string &szName);
	string &GetName();

	string GetClassName();

	int AddWidget(CUIWidget *pWidget);
	int AddWidget(const string &szName);
	int DelWidget(CUIWidget *pWidget);
	int DelWidget(int iIndex);
	int DelWidget(const string &szName);

	CUIWidgetList *GetWidgetList();
	CUIWidget *GetWidget(int iIndex);
	CUIWidget *GetWidget(const string &szName);
	int	GetWidgetCount();

	int Activate();
	int Deactivate();

	int OnInit();
	int OnUpdate();
	int OnRelease();
	int OnActivate();
	int OnDeactivate();

	//------------------------------------------------------------------------------------------------- 
	// Script Functions
	//------------------------------------------------------------------------------------------------- 
	int Release(IFunctionHandler *pH);

	int GetName(IFunctionHandler *pH);

	int GetWidgetCount(IFunctionHandler *pH);
	int GetWidget(IFunctionHandler *pH);
	int AddWidget(IFunctionHandler *pH);
	int DelWidget(IFunctionHandler *pH);

private:

	string			m_szName;

	CUISystem				*m_pUISystem;
	CUIWidgetList		m_vWidgetList;
	bool						m_bActive;

	HSCRIPTFUNCTION m_hOnInit;
	HSCRIPTFUNCTION m_hOnUpdate;
	HSCRIPTFUNCTION m_hOnRelease;
	HSCRIPTFUNCTION m_hOnActivate;
	HSCRIPTFUNCTION m_hOnDeactivate;
};
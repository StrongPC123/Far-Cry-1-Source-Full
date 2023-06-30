//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - A Button
//
// History:
//  - [3/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#pragma once


#define UICLASSNAME_BUTTON			"UIButton"



#include "UIWidget.h"


class CUISystem;


class CUIButton : public CUIWidget,
	public _ScriptableEx<CUIButton>
{	
	UI_WIDGET(CUIButton)

public:
	CUIButton();
	~CUIButton();

	string GetClassName();

	LRESULT Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam);	//AMD Port
	int Draw(int iPass);

	int SetText(const wstring &szText);

	static void InitializeTemplate(IScriptSystem *pScriptSystem);

	//------------------------------------------------------------------------------------------------- 
	// Script Functions
	//------------------------------------------------------------------------------------------------- 
	int SetText(IFunctionHandler *pH);
	int GetText(IFunctionHandler *pH);

	int GetState(IFunctionHandler *pH);
	int SetState(IFunctionHandler *pH);

	int SetVAlign(IFunctionHandler *pH);
	int GetVAlign(IFunctionHandler *pH);

	int SetHAlign(IFunctionHandler *pH);
	int GetHAlign(IFunctionHandler *pH);

	int SetTexture(IFunctionHandler *pH);
	int GetTexture(IFunctionHandler *pH);

	int SetDownTexture(IFunctionHandler *pH);
	int GetDownTexture(IFunctionHandler *pH);

	int SetOverTexture(IFunctionHandler *pH);
	int GetOverTexture(IFunctionHandler *pH);

	int SetOverState(IFunctionHandler *pH);

private:

	wstring					m_szText;
	int							m_iHAlignment;
	int							m_iVAlignment;

	int							m_iState;
	bool						m_bKeepOver;

	UISkinTexture		m_pTexture;
};
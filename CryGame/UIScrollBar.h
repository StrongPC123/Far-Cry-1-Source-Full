//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - A ScrollBar
//
// History:
//  - [23/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#pragma once


#include "UIWidget.h"


#define UICLASSNAME_SCROLLBAR			"UIScrollBar"


#define UISCROLLBARSTATE_SLIDER_MOVING	(1 << 0)
#define UISCROLLBARSTATE_SLIDER_OVER	(1 << 1)
#define UISCROLLBARSTATE_MINUS_DOWN		(1 << 2)
#define UISCROLLBARSTATE_MINUS_OVER		(1 << 3)
#define UISCROLLBARSTATE_PLUS_DOWN		(1 << 4)
#define UISCROLLBARSTATE_PLUS_OVER		(1 << 5)
#define UISCROLLBARSTATE_PATH_OVER		(1 << 6)



class CUISystem;


class CUIScrollBar : public CUIWidget,
	public _ScriptableEx<CUIScrollBar>
{
	UI_WIDGET(CUIScrollBar)

public:

	CUIScrollBar();
	~CUIScrollBar();

	string GetClassName();

	LRESULT Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam);	//AMD Port
	int Draw(int iPass);

	static void InitializeTemplate(IScriptSystem *pScriptSystem);

	int SetValue(float fValue);
	float GetValue();

	int SetStep(float fStep);
	float GetStep();

	int GetType();

	//------------------------------------------------------------------------------------------------- 
	// Script Functions
	//------------------------------------------------------------------------------------------------- 
	int GetType(IFunctionHandler *pH);

	int SetValue(IFunctionHandler *pH);
	int GetValue(IFunctionHandler *pH);

	int SetStep(IFunctionHandler *pH);
	int GetStep(IFunctionHandler *pH);

	int SetSliderColor(IFunctionHandler *pH);
	int GetSliderColor(IFunctionHandler *pH);

	int SetMinusColor(IFunctionHandler *pH);
	int GetMinusColor(IFunctionHandler *pH);

	int SetPlusColor(IFunctionHandler *pH);
	int GetPlusColor(IFunctionHandler *pH);

	int SetSliderSize(IFunctionHandler *pH);
	int GetSliderSize(IFunctionHandler *pH);

	int SetButtonSize(IFunctionHandler *pH);
	int GetButtonSize(IFunctionHandler *pH);

	int SetPathTexture(IFunctionHandler *pH);
	int GetPathTexture(IFunctionHandler *pH);

	int SetPathTextureFlip(IFunctionHandler *pH);
	int GetPathTextureFlip(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int SetSliderTexture(IFunctionHandler *pH);
	int GetSliderTexture(IFunctionHandler *pH);

	int SetSliderTextureFlip(IFunctionHandler *pH);
	int GetSliderTextureFlip(IFunctionHandler *pH);

	int SetSliderOverTexture(IFunctionHandler *pH);
	int GetSliderOverTexture(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int SetMinusTexture(IFunctionHandler *pH);
	int GetMinusTexture(IFunctionHandler *pH);

	int SetMinusTextureFlip(IFunctionHandler *pH);
	int GetMinusTextureFlip(IFunctionHandler *pH);

	int SetMinusOverTexture(IFunctionHandler *pH);
	int GetMinusOverTexture(IFunctionHandler *pH);

	int SetMinusDownTexture(IFunctionHandler *pH);
	int GetMinusDownTexture(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int SetPlusTexture(IFunctionHandler *pH);
	int GetPlusTexture(IFunctionHandler *pH);

	int SetPlusTextureFlip(IFunctionHandler *pH);
	int GetPlusTextureFlip(IFunctionHandler *pH);

	int SetPlusOverTexture(IFunctionHandler *pH);
	int GetPlusOverTexture(IFunctionHandler *pH);

	int SetPlusDownTexture(IFunctionHandler *pH);
	int GetPlusDownTexture(IFunctionHandler *pH);

private:

	int MoveSlider(float fDelta);
	int	UpdateRect();

	int						m_iType;
	
	UIRect				m_pPathRect;
	UIRect				m_pMinusRect;
	UIRect				m_pPlusRect;
	float					m_fButtonSize;
	color4f	m_cMinusColor;
	color4f	m_cPlusColor;

	float					m_fSliderClick;
	float					m_fSliderOffset;
	float					m_fSliderSize;
	UIRect				m_pSliderRect;
	color4f	m_cSliderColor;

	float					m_fPathSize;

	UISkinTexture	m_pSliderTexture;
	UISkinTexture	m_pPlusTexture;
	UISkinTexture	m_pMinusTexture;
	UISkinTexture	m_pPathTexture;

	int						m_iState;

	float					m_fStep;
	float					m_fValue;
	float					m_fRepeatTimer;
};
//------------------------------------------------------------------------------------------------- 
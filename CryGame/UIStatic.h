//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - A Static Control
//
// History:
//  - [23/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#pragma once


#define UICLASSNAME_STATIC			"UIStatic"


#include "UIWidget.h"
#include "UISystem.h"



typedef struct UIStaticLine
{
	int						iWrapIndex[UI_DEFAULT_MAX_WRAP_INDICES];
	int						iWrapCount;
	float					fWrapWidth;
	float					fWidth;
	float					fHeight;
	wstring	szText;
		
} UIStaticLine;


class CUISystem;


class CUIStatic : public CUIWidget,
	public _ScriptableEx<CUIStatic>
{
	UI_WIDGET(CUIStatic)

public:
	CUIStatic();
	~CUIStatic();

	string GetClassName();

	int SetStyle(int iStyle);

	LRESULT Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam);	//AMD Port
	int Draw(int iPass);

	int SetText(const wstring &szText);

	int SetVAlign(int iAlign);
	int GetVAlign();

	int SetHAlign(int iAlign);
	int GetHAlign();

	int LoadModel(const string &szModelName);
	int ReleaseModel();
	int StartAnimation(const string &szAnimationName);

	static void InitializeTemplate(IScriptSystem *pScriptSystem);

	//------------------------------------------------------------------------------------------------- 
	// Script Functions
	//------------------------------------------------------------------------------------------------- 
	int SetFontName(IFunctionHandler *pH);
	int SetFontSize(IFunctionHandler *pH);

	int SetText(IFunctionHandler *pH);
	int GetText(IFunctionHandler *pH);

	int SetLineSpacing(IFunctionHandler *pH);
	int GetLineSpacing(IFunctionHandler *pH);

	int Clear(IFunctionHandler *pH);
	int GetLine(IFunctionHandler *pH);
	int SetLine(IFunctionHandler *pH);
	int AddLine(IFunctionHandler *pH);
	
	int GetLineCount(IFunctionHandler *pH);

	int SetVAlign(IFunctionHandler *pH);
	int GetVAlign(IFunctionHandler *pH);

	int SetHAlign(IFunctionHandler *pH);
	int GetHAlign(IFunctionHandler *pH);

	int SetTexture(IFunctionHandler *pH);
	int GetTexture(IFunctionHandler *pH);

	int GetVScrollBar(IFunctionHandler *pH);
	int GetHScrollBar(IFunctionHandler *pH);

	int LoadModel(IFunctionHandler *pH);
	int ReleaseModel(IFunctionHandler *pH);
	int SetView(IFunctionHandler *pH);
	int SetAnimation(IFunctionHandler *pH);
	int SetShaderFloat(IFunctionHandler *pH);
	int SetShader(IFunctionHandler *pH);
	int SetSecondShader(IFunctionHandler *pH);

private:

	int GetLineMetrics(UIStaticLine *pLine, IFFont *pFont);
	int GetLineListMetrics();

	float GetLineHeight(UIStaticLine *pLine);
	float GetLineWidth(UIStaticLine *pLine);
	float GetTextHeight();
	float GetTextWidth();
	UIRect GetTextRect(bool bScrollBars);

	std::vector<UIStaticLine>	m_vLines;
	int												m_iMaxLines;

	float			m_fTotalWidth;
	float			m_fTotalHeight;
	float			m_fLineHeight;

	float			m_fLineSpacing;
	float			m_fLeftSpacing;
	float			m_fRightSpacing;
	
	int				m_iHAlignment;
	int				m_iVAlignment;

	bool			m_bVerticalScrollBar;
	bool			m_bHorizontalScrollBar;

	float			m_fVerticalOffset;
	float			m_fHorizontalOffset;

	CUIScrollBar	*m_pHScroll;
	CUIScrollBar	*m_pVScroll;

	UISkinTexture			m_pTexture;

	ICryCharInstance	*m_pModel;
	string						m_szModelName;
	float							m_fCameraDistance;
	float							m_fCameraFov;
	float							m_fModelRotation;
	float							m_fModelRotationAcc;
	float							m_fMouseMultiplier;
	float							m_fLightDistance;
	float							m_fAngle;
};
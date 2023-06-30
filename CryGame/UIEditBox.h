//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - An EditBox
//
// History:
//  - [20/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#pragma once


#include "UIWidget.h"


#define UICLASSNAME_EDITBOX			"UIEditBox"


class CUISystem;


class CUIEditBox : public CUIWidget,
	public _ScriptableEx<CUIEditBox>
{
	UI_WIDGET(CUIEditBox)
public:

	CUIEditBox();
	~CUIEditBox();

	string GetClassName();

	LRESULT Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam);	//AMD Port
	int Draw(int iPass);

	static void InitializeTemplate(IScriptSystem *pScriptSystem);

	int Cut();
	int CopyToClipboard();
	int CutToClipboard();
	int PasteFromClipboard();

	int GetSelectionStart();
	int GetSelectedCount();

	int SetText(const wstring &szText);
	int GetText(wstring &szText);
	int GetText(string &szText);

	//------------------------------------------------------------------------------------------------- 
	// Script Functions
	//------------------------------------------------------------------------------------------------- 
	int SetText(IFunctionHandler *pH);
	int GetText(IFunctionHandler *pH);

	int Clear(IFunctionHandler *pH);

	int SetMaxLength(IFunctionHandler *pH);
	int GetMaxLength(IFunctionHandler *pH);

	int GetTextLength(IFunctionHandler *pH);

	int SetVAlign(IFunctionHandler *pH);
	int GetVAlign(IFunctionHandler *pH);

	int SetHAlign(IFunctionHandler *pH);
	int GetHAlign(IFunctionHandler *pH);

	int SetSelectionStart(IFunctionHandler *pH);
	int GetSelectionStart(IFunctionHandler *pH);

	int SetSelectionCount(IFunctionHandler *pH);
	int GetSelectionCount(IFunctionHandler *pH);

	int SelectAll(IFunctionHandler *pH);
	int DeselectAll(IFunctionHandler *pH);

	int SetCursorPosition(IFunctionHandler *pH);
	int GetCursorPosition(IFunctionHandler *pH);

	int Cut(IFunctionHandler *pH);
	int CopyToClipboard(IFunctionHandler *pH);
	int CutToClipboard(IFunctionHandler *pH);
	int PasteFromClipboard(IFunctionHandler *pH);

	int SetTexture(IFunctionHandler *pH);
	int GetTexture(IFunctionHandler *pH);
	    
	int SetCursorColor(IFunctionHandler *pH);
	int GetCursorColor(IFunctionHandler *pH);

	int SetNumeric(IFunctionHandler *pH);
	int SetPathSafe(IFunctionHandler *pH);
	int SetNameSafe(IFunctionHandler *pH);
	int SetUbiSafe(IFunctionHandler *pH);
	int SetPassword(IFunctionHandler *pH);
	int SetAllow(IFunctionHandler *pH);
	int SetDisallow(IFunctionHandler *pH);

private:

	int DrawCursor(const UIRect &pTextRect, IFFont *pFont, float fX, float fY, float fHeight);
	int DrawSelection(int iStart, int iCount, IFFont *pFont, const UIRect &pTextRect);
	int SelectLeft();
	int SelectRight();
	int Backspace();
	int Delete();
	int Left();
	int Right();
	int InsertChar(wchar_t cChar);
	int ProcessInput(unsigned int iMessage, int iKeyCode, char *szKeyName);
	int CheckChar(wchar_t cChar);
	
	UIRect GetTextRect();
	int GetCursorCoord(float *fX, float *fY, float *fHeight, const UIRect &pTextRect, IFFont *pFont);
	int GetStringLength(const wchar_t *pString);
	int GetCursorPosition(float fAtX, float fAtY, const UIRect &pTextRect, IFFont *pFont);

	string						m_szAllow;
	string						m_szDisallow;

	int								m_iPathSafe;
	int								m_iNameSafe;
	int								m_iNumeric;
	int								m_iUbiSafe;
	int								m_iMaxLength;

	int								m_iCursorPos;

	int								m_iSelectionStart;
	int								m_iSelectionCount;

	float							m_fLeftSpacing;
	float							m_fRightSpacing;

	color4f			m_cCursorColor;
	color4f			m_cSelectionColor;

	bool							m_bMouseSelecting;
	bool							m_bMouseSelectingAll;
	int								m_iMouseSelectionStart;

	wstring						m_szText;
	int								m_iHAlignment;
	int								m_iVAlignment;

	UISkinTexture			m_pTexture;
};
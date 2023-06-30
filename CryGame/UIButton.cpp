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
#include "StdAfx.h"
#include "UIButton.h"
#include "UISystem.h"



_DECLARE_SCRIPTABLEEX(CUIButton)


//------------------------------------------------------------------------------------------------- 
CUIButton::CUIButton()
: m_iHAlignment(UIALIGN_CENTER),
	m_iVAlignment(UIALIGN_MIDDLE),
	m_iState(UISTATE_UP),
	m_bKeepOver(0)
{
}

//------------------------------------------------------------------------------------------------- 
CUIButton::~CUIButton()
{
}

//------------------------------------------------------------------------------------------------- 
string CUIButton::GetClassName()
{
	return UICLASSNAME_BUTTON;
}

//------------------------------------------------------------------------------------------------- 
LRESULT CUIButton::Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam)	//AMD Port
{
	switch (iMessage)
	{
	case UIM_KEYUP:
		if (lParam != XKEY_RETURN)
		{
			return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
		}
	case UIM_LBUTTONUP:
		{
			m_iState |= UISTATE_UP;
			m_iState &= ~UISTATE_DOWN;

			m_pUISystem->ResetInput();

			OnCommand();

			return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
		}
	case UIM_LBUTTONDOWN:
		{
			m_iState |= UISTATE_DOWN;
			m_iState &= ~UISTATE_UP;

			return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
		}
	case UIM_MOUSEENTER:
		{
			m_iState |= UISTATE_OVER;

			return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
		}
	case UIM_MOUSELEAVE:
		{
			m_iState &= ~UISTATE_OVER;		
			m_iState &= ~UISTATE_DOWN;
			m_iState |= UISTATE_UP;

			return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
		}

	default:
		return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
	}	

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::Draw(int iPass)
{
	if (iPass != 0)
	{
		return 1;
	}

	m_pUISystem->BeginDraw(this);
	
	// get the absolute widget rect
	UIRect pAbsoluteRect(m_pRect);
	
	m_pUISystem->GetAbsoluteXY(&pAbsoluteRect.fLeft, &pAbsoluteRect.fTop, m_pRect.fLeft, m_pRect.fTop, m_pParent);

	// if transparent, draw only the clipped text
	if ((GetStyle() & UISTYLE_TRANSPARENT) == 0)
	{
		// if shadowed, draw the shadow
		if (GetStyle() & UISTYLE_SHADOWED)
		{
			m_pUISystem->DrawShadow(pAbsoluteRect, UI_DEFAULT_SHADOW_COLOR, UI_DEFAULT_SHADOW_BORDER_SIZE, this);
		}
	}

	// if border is large enough to be visible, draw it
	if (m_pBorder.fSize > 0.125f)
	{
		m_pUISystem->DrawBorder(pAbsoluteRect, m_pBorder);
		m_pUISystem->AdjustRect(&pAbsoluteRect, pAbsoluteRect, m_pBorder.fSize);
	}

	// save the client area without the border,
	// to draw a greyed quad later, if disabled
	UIRect pGreyedRect = pAbsoluteRect;

	int iOldState = m_iState;

	if (m_bKeepOver)
	{
		m_iState |= UISTATE_OVER;
	}

	if ((GetStyle() & UISTYLE_TRANSPARENT) == 0)
	{
		// if we are a textured button, draw the correct texture
		if (m_pTexture.iTextureID > -1)
		{
			m_pUISystem->DrawSkin(pAbsoluteRect, m_pTexture, m_cColor, m_iState);
		}
		// if not textured, just draw the emboss border
		else
		{
			if (m_iState & UISTATE_DOWN)
			{
				m_pUISystem->DrawEmboss(pAbsoluteRect, GET_HIGHLIGHT_COLOR(m_cColor), GET_SHADOWED_COLOR(m_cColor), 1, UI_DEFAULT_EMBOSS_BORDER_SIZE);
			}
			else
			{
				m_pUISystem->DrawEmboss(pAbsoluteRect, GET_HIGHLIGHT_COLOR(m_cColor), GET_SHADOWED_COLOR(m_cColor), 0, UI_DEFAULT_EMBOSS_BORDER_SIZE);
			}

			m_pUISystem->AdjustRect(&pAbsoluteRect, pAbsoluteRect, UI_DEFAULT_EMBOSS_BORDER_SIZE);

			m_pUISystem->DrawQuad(pAbsoluteRect, m_cColor);
		}
	}

	m_iState = iOldState;

	if (!m_szText.empty())
	{
		if (m_iState & UISTATE_DOWN)
		{
			pAbsoluteRect.fLeft += UI_DEFAULT_EMBOSS_BORDER_SIZE;
			pAbsoluteRect.fTop += UI_DEFAULT_EMBOSS_BORDER_SIZE;
		}

		UIRect pTextRect;

		m_pUISystem->AdjustRect(&pTextRect, pAbsoluteRect, UI_DEFAULT_TEXT_BORDER_SIZE);
		m_pUISystem->SetScissor(&pTextRect);

		IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

		m_pUISystem->DrawText(pTextRect, m_iHAlignment, m_iVAlignment, pFont, m_szText.c_str());
	}

	// draw a greyed quad ontop, if disabled
	if ((m_iFlags & UIFLAG_ENABLED) == 0)
	{
		m_pUISystem->ResetDraw();
		m_pUISystem->DrawGreyedQuad(pGreyedRect, m_cGreyedColor, m_iGreyedBlend);
	}

	m_pUISystem->EndDraw();

	// draw the children
	if (m_pUISystem->ShouldSortByZ())
	{
		SortChildrenByZ();
	}

	DrawChildren();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::SetText(const wstring &szText)
{
	m_szText = szText;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
void CUIButton::InitializeTemplate(IScriptSystem *pScriptSystem)
{
	_ScriptableEx<CUIButton>::InitializeTemplate(pScriptSystem);

	REGISTER_COMMON_MEMBERS(pScriptSystem, CUIButton);

	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, SetText);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, GetText);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, GetState);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, SetState);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, SetVAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, GetVAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, SetHAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, GetHAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, SetTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, GetTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, SetDownTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, GetDownTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, SetOverTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, GetOverTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIButton, SetOverState);
}


//------------------------------------------------------------------------------------------------- 
// Script Functions
//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
int CUIButton::SetText(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetText, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), SetText, 1, svtString, svtNumber);

	m_szText.clear();

	m_pUISystem->ConvertToWString(m_szText, pH, 1);

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::GetText(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetText, 0);

	char szString[1024] = {0,0};

	size_t iSize = min(m_szText.size(), sizeof(szString)-1);

	size_t i = 0;
	for (; i < iSize; i++)
	{
		szString[i] = (char)m_szText[i];
	}
	szString[i] = 0;

	return pH->EndFunction(szString);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::GetState(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetState, m_iState);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::SetState(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetState, m_iState);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::SetVAlign(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetVerticalTextAlignment, m_iVAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::GetVAlign(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetVerticalTextAlignment, m_iVAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::SetHAlign(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetHorizontalTextAlignment, m_iHAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::GetHAlign(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetHorizontalTextAlignment, m_iHAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::SetTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetTexture, m_pTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::GetTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetTexture, m_pTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::SetDownTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetDownTexture, m_pTexture.iDownTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::GetDownTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetDownTexture, m_pTexture.iDownTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::SetOverTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetOverTexture, m_pTexture.iOverTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::GetOverTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetOverTexture, m_pTexture.iOverTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIButton::SetOverState(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetOverState, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetOverState, 1, svtNumber);

	int iOverState = 0;

	pH->GetParam(1, iOverState);
	m_bKeepOver = (iOverState != 0);

	return pH->EndFunction();
}
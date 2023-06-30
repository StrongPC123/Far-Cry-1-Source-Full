//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - A CheckBox Control
//
// History:
//  - [9/7/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#include "StdAfx.h"
#include "UICheckBox.h"
#include "UISystem.h"



_DECLARE_SCRIPTABLEEX(CUICheckBox)


//------------------------------------------------------------------------------------------------- 
CUICheckBox::CUICheckBox()
: m_iState(0),
	m_fLeftSpacing(0),
	m_fRightSpacing(0),
	m_iVAlignment(UIALIGN_MIDDLE),
	m_iHAlignment(UIALIGN_CENTER),
	m_cCheckColor(0.8f, 0.8f, 0.8f, 0.8f)
{
}

//------------------------------------------------------------------------------------------------- 
CUICheckBox::~CUICheckBox()
{
}

//------------------------------------------------------------------------------------------------- 
string CUICheckBox::GetClassName()
{
	return UICLASSNAME_CHECKBOX;
}

//------------------------------------------------------------------------------------------------- 
LRESULT CUICheckBox::Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam)	//AMD Port
{
	switch (iMessage)
	{
	case UIM_KEYUP:
		if ((lParam != XKEY_RETURN) && (lParam != XKEY_SPACE))
		{
			return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
		}

		if (m_iState & UISTATE_CHECKED)
		{
			m_iState &= ~UISTATE_CHECKED;
		}
		else
		{
			m_iState |= UISTATE_CHECKED;
		}

		OnChanged();

		break;
	case UIM_LBUTTONUP:
		{
			if (m_pUISystem->PointInRect(GetBorderedRect(), UIM_GET_X_FLOAT(wParam), UIM_GET_Y_FLOAT(wParam)))
			{
				if (m_iState & UISTATE_CHECKED)
				{
					m_iState &= ~UISTATE_CHECKED;
				}
				else
				{
					m_iState |= UISTATE_CHECKED;
				}

				OnChanged();
			}
		}
		break;
	case UIM_MOUSEOVER:

		m_iState |= UISTATE_OVER;
		break;
	case UIM_MOUSELEAVE:

		m_iState &= ~UISTATE_OVER;
		break;
	}
	return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
}

//------------------------------------------------------------------------------------------------- 
int CUICheckBox::Draw(int iPass)
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

	if ((GetStyle() & UISTYLE_TRANSPARENT) == 0)
	{
		// if we are textured, draw the correct texture
		if (m_pTexture.iTextureID > -1)
		{
			m_pUISystem->DrawSkin(pAbsoluteRect, m_pTexture, m_cColor, m_iState);
		}
		// if not textured, just draw the back quad
		else
		{
			m_pUISystem->DrawQuad(pAbsoluteRect, m_cColor);

			if (m_iState & UISTATE_CHECKED)
			{
				UIFont pCrossFont = m_pFont;

				pCrossFont.fSize = pAbsoluteRect.fHeight * 1.125f;

				IFFont *pFont = m_pUISystem->GetIFont(pCrossFont);

				m_pUISystem->DrawText(pAbsoluteRect, UIALIGN_CENTER, UIALIGN_MIDDLE, pFont, L"X");
			}
		}
	}

	if ((m_iState & UISTATE_CHECKED) && (m_pTexture.iDownTextureID == -1))
	{
		UIRect pCheckRect;
		
		m_pUISystem->AdjustRect(&pCheckRect, pAbsoluteRect, 2.0f);

		m_pUISystem->DrawQuad(pCheckRect, m_cCheckColor);
	}

	if (!m_szText.empty())
	{
		UIRect pTextRect = UIRect(pAbsoluteRect.fLeft+m_fLeftSpacing, pAbsoluteRect.fTop, pAbsoluteRect.fWidth-m_fLeftSpacing-m_fRightSpacing, pAbsoluteRect.fHeight);

		m_pUISystem->AdjustRect(&pTextRect, pTextRect, UI_DEFAULT_TEXT_BORDER_SIZE);
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
void CUICheckBox::InitializeTemplate(IScriptSystem *pScriptSystem)
{
	_ScriptableEx<CUICheckBox>::InitializeTemplate(pScriptSystem);

	REGISTER_COMMON_MEMBERS(pScriptSystem, CUICheckBox);

	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUICheckBox, SetText);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUICheckBox, GetText);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUICheckBox, SetTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUICheckBox, GetTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUICheckBox, SetChecked);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUICheckBox, GetChecked);
}

//------------------------------------------------------------------------------------------------- 
int CUICheckBox::SetText(const wstring &szwString)
{
	m_szText = szwString;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
UIRect CUICheckBox::GetBorderedRect()
{
	UIRect pRect(0, 0, m_pRect.fWidth, m_pRect.fHeight);

	// if border is large enough to be visible, remove it from the rect
	if (m_pBorder.fSize > 0.125f)
	{
		m_pUISystem->AdjustRect(&pRect, pRect, m_pBorder.fSize);
	}

//	m_pUISystem->GetAbsoluteXY(&pRect.fLeft, &pRect.fTop, pRect.fLeft, pRect.fTop, this);
//	m_pUISystem->GetRelativeXY(&pRect.fLeft, &pRect.fTop, pRect.fLeft, pRect.fTop, this);

	return pRect;
}

//------------------------------------------------------------------------------------------------- 
// Script Functions
//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
int CUICheckBox::SetText(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetText, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), SetText, 1, svtString, svtNumber);

	m_szText.clear();

	m_pUISystem->ConvertToWString(m_szText, pH, 1);

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUICheckBox::GetText(IFunctionHandler *pH)
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
int CUICheckBox::SetTexture(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, GetName().c_str(), SetTexture, 1, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetTexture, 1, svtUserData);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetTexture, 2, svtString);

	int iCookie = 0;

	pH->GetParamUDVal(1, m_pTexture.iTextureID, iCookie);

	if (pH->GetParamCount() == 2)
	{
		char *szTexRect = "";

		pH->GetParam(2, szTexRect);

		m_pUISystem->RetrieveTexRect(m_pTexture.vTexCoord, m_pTexture.iTextureID, szTexRect);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUICheckBox::GetTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetTexture, m_pTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUICheckBox::SetChecked(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetChecked, 1);
//	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetChecked, 1, svtNumber);

	int iValue=0;

	if(pH->GetParamType(1)==svtNumber)
		pH->GetParam(1, iValue);

	if (iValue != 0)
	{
		m_iState |= UISTATE_CHECKED;
	}
	else
	{
		m_iState &= ~UISTATE_CHECKED;
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUICheckBox::GetChecked(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetChecked, 0);

	if (m_iState & UISTATE_CHECKED)
	{
		return pH->EndFunction(1);
	}

	return pH->EndFunction();
}
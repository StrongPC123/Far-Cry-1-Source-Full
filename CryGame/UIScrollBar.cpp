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
#include "StdAfx.h"
#include "UIScrollBar.h"
#include "UISystem.h"



_DECLARE_SCRIPTABLEEX(CUIScrollBar);


//------------------------------------------------------------------------------------------------- 
CUIScrollBar::CUIScrollBar()
: m_fValue(0.0f), m_fStep(0.0275f), m_iState(0), m_fRepeatTimer(0)
{
}

//------------------------------------------------------------------------------------------------- 
CUIScrollBar::~CUIScrollBar()
{
}

//------------------------------------------------------------------------------------------------- 
string CUIScrollBar::GetClassName()
{
	return UICLASSNAME_SCROLLBAR;
}

//------------------------------------------------------------------------------------------------- 
LRESULT CUIScrollBar::Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam)	//AMD Port
{
	m_fValue = CLAMP(m_fValue, 0.0f, 1.0f);

	switch (iMessage)
	{
	case UIM_MOUSEOVER:
		{
			float fX = UIM_GET_X_FLOAT(lParam);
			float fY = UIM_GET_Y_FLOAT(lParam);

			if (m_pUISystem->PointInRect(m_pMinusRect, fX, fY))
			{
				m_iState |= UISCROLLBARSTATE_MINUS_OVER;
			}
			else
			{
				m_iState &= ~UISCROLLBARSTATE_MINUS_OVER;
			}

			if (m_pUISystem->PointInRect(m_pPlusRect, fX, fY))
			{
				m_iState |= UISCROLLBARSTATE_PLUS_OVER;
			}
			else
			{
				m_iState &= ~UISCROLLBARSTATE_PLUS_OVER;
			}

			if (m_pUISystem->PointInRect(m_pSliderRect, fX, fY))
			{
				m_iState |= UISCROLLBARSTATE_SLIDER_OVER;
				m_iState &= ~UISCROLLBARSTATE_PATH_OVER;
			}
			else if (m_pUISystem->PointInRect(m_pPathRect, fX, fY))
			{
				m_iState &= ~UISCROLLBARSTATE_SLIDER_OVER;
				//m_iState |= UISCROLLBARSTATE_PATH_OVER;
			}
			else
			{
				m_iState &= ~UISCROLLBARSTATE_PATH_OVER;
				m_iState &= ~UISCROLLBARSTATE_SLIDER_OVER;
			}
		}
		break;
	case UIM_KEYDOWN:
		{
			if ((GetType() == UISCROLLBARTYPE_HORIZONTAL) && (lParam == XKEY_LEFT))
			{
				m_fValue -= m_fStep;

				OnChanged();
			}
			else if ((GetType() == UISCROLLBARTYPE_HORIZONTAL) && (lParam == XKEY_RIGHT))
			{
				m_fValue += m_fStep;

				OnChanged();
			}
			else if ((GetType() == UISCROLLBARTYPE_VERTICAL) && (lParam == XKEY_UP))
			{
				m_fValue -= m_fStep;

				OnChanged();
			}
			else if ((GetType() == UISCROLLBARTYPE_VERTICAL) && (lParam == XKEY_DOWN))
			{
				m_fValue += m_fStep;

				OnChanged();
			}
		}
		break;
	case UIM_MOUSELEAVE:
		{
			//if (m_iState &= UISCROLLBARSTATE_SLIDER_MOVING)
			//{
			//	m_iState &= ~UISCROLLBARSTATE_SLIDER_MOVING;

			//	m_pUISystem->ReleaseMouse();
			//}			
			m_iState &= UISCROLLBARSTATE_SLIDER_MOVING;
		}
		break;
	case UIM_MOUSEMOVE:
		{
			if (m_iState & UISCROLLBARSTATE_SLIDER_MOVING)
			{
				float fX = UIM_GET_X_FLOAT(lParam);
				float fY = UIM_GET_Y_FLOAT(lParam);

				float fSliderX, fSliderY;

				m_pUISystem->GetAbsoluteXY(&fSliderX, &fSliderY, m_pSliderRect.fLeft, m_pSliderRect.fTop, this);


				float fDelta = 0.0f;

				if (GetType() == UISCROLLBARTYPE_HORIZONTAL)
				{
					fDelta = fX - m_fSliderClick;
				}
				else
				{
					fDelta = fY - m_fSliderClick;
				}

				MoveSlider(fDelta);

				m_fValue += fDelta / m_fPathSize;

				OnChanged();
			}
		}
		break;
	case UIM_MOUSEUP:
		{
			if (m_iState & UISCROLLBARSTATE_SLIDER_MOVING)
			{
				m_iState &= ~UISCROLLBARSTATE_SLIDER_MOVING;

				m_pUISystem->ReleaseMouse();
			}
			m_iState &= ~(UISCROLLBARSTATE_MINUS_DOWN | UISCROLLBARSTATE_PLUS_DOWN);

			m_fRepeatTimer = 0;
		}
		break;
	case UIM_LBUTTONDOWN:
		{
			float fX = UIM_GET_X_FLOAT(wParam);
			float fY = UIM_GET_Y_FLOAT(wParam);

			// check if mouse position is inside the scroll slider
			if (m_pUISystem->PointInRect(m_pSliderRect, fX, fY))
			{
				if ((lParam == XKEY_MOUSE1) && ((m_iState & UISCROLLBARSTATE_SLIDER_MOVING) == 0) && m_pUISystem->CaptureMouse(this))
				{
					m_iState = UISCROLLBARSTATE_SLIDER_MOVING | UISCROLLBARSTATE_SLIDER_OVER;

					float fAbsX, fAbsY;

					m_pUISystem->GetAbsoluteXY(&fAbsX, &fAbsY, fX, fY, this);

					if (GetType() == UISCROLLBARTYPE_HORIZONTAL)
					{
						m_fSliderClick = fAbsX;
						m_fSliderOffset = fX - m_pSliderRect.fLeft;
					}
					else
					{
						m_fSliderClick = fAbsY;
						m_fSliderOffset = fY - m_pSliderRect.fTop;
					}

					m_fRepeatTimer = 0.0f;
				}
			}

			// mouse is not in the scroll slider,
			// check if it is in the arrows

			// left/top arrow
			else if (m_pUISystem->PointInRect(m_pMinusRect, fX, fY) && !(m_iState & UISCROLLBARSTATE_SLIDER_MOVING))
			{
				//m_iState = UISCROLLBARSTATE_MINUS_OVER;

				if (lParam == XKEY_MOUSE1)
				{
					float fTimer = m_pUISystem->GetISystem()->GetITimer()->GetCurrTime();

					if ((m_fRepeatTimer == 0.0f) || (fTimer > m_fRepeatTimer))
					{
						m_iState |= UISCROLLBARSTATE_MINUS_DOWN;
						m_fValue -= m_fStep;

						OnChanged();

						if (m_fRepeatTimer == 0.0f)
						{
							m_fRepeatTimer = fTimer + 0.35f;
						}
						else
						{
							m_fRepeatTimer = fTimer + 0.075f;
						}
					}
				}
			}
			// right/bottom
			else if (m_pUISystem->PointInRect(m_pPlusRect, fX, fY) && !(m_iState & UISCROLLBARSTATE_SLIDER_MOVING))
			{
				//m_iState = UISCROLLBARSTATE_PLUS_OVER;

				if (lParam == XKEY_MOUSE1)
				{
					float fTimer = m_pUISystem->GetISystem()->GetITimer()->GetCurrTime();

					if ((m_fRepeatTimer == 0.0f) || (fTimer > m_fRepeatTimer))
					{
						m_iState |= UISCROLLBARSTATE_PLUS_DOWN;
						m_fValue += m_fStep;

						OnChanged();
						
						if (m_fRepeatTimer == 0.0f)
						{
							m_fRepeatTimer = fTimer + 0.35f;
						}
						else
						{
							m_fRepeatTimer = fTimer + 0.075f;
						}
					}
				}
			}
			// path
			else if (m_pUISystem->PointInRect(m_pPathRect, fX, fY) && !(m_iState & UISCROLLBARSTATE_SLIDER_MOVING))
			{
				//m_iState = UISCROLLBARSTATE_PATH_OVER;

				float fDelta = 0.0f;

				if (GetType() == UISCROLLBARTYPE_HORIZONTAL)
				{
					if (fX > m_pSliderRect.fLeft)
					{
						fX -= m_fSliderSize;
					}			

					fDelta = (fX - m_pSliderRect.fLeft) * 0.05f;
				}
				else
				{
					if (fY > m_pSliderRect.fTop)
					{
						fY -= m_fSliderSize;
					}

					fDelta = (fY - m_pSliderRect.fTop) * 0.05f;
				}

				MoveSlider(fDelta);

				m_fValue += fDelta / m_fPathSize;

				OnChanged();

				m_fRepeatTimer = 0.0f;
			}
		}
		break;
	default:
		break;
	}

	m_fValue = CLAMP(m_fValue, 0.0f, 1.0f);

	UpdateRect();

	return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::Draw(int iPass)
{
	if (iPass != 0)
	{
		return 1;
	}

	// setup to begin drawing
	m_pUISystem->BeginDraw(this);

	// get the absolute widget rect
	UIRect pAbsoluteRect(m_pRect);

	m_pUISystem->GetAbsoluteXY(&pAbsoluteRect.fLeft, &pAbsoluteRect.fTop, m_pRect.fLeft, m_pRect.fTop, m_pParent);

	// if transparent
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
	}

	// save the client area without the border,
	// to draw a greyed quad later, if disabled
	UIRect pGreyedRect = pAbsoluteRect;

	// arrange stuff for different types
	UIRect pNewRect;
	int iState = UISTATE_UP;

	pNewRect = UIRect(pAbsoluteRect.fLeft + m_pPathRect.fLeft, pAbsoluteRect.fTop + m_pPathRect.fTop, m_pPathRect.fWidth, m_pPathRect.fHeight);
	// draw the button path
	// if textured and textureid set, draw the texture
	if (m_pPathTexture.iTextureID > -1)
	{
		if (m_iState & UISCROLLBARSTATE_PATH_OVER)
		{
			iState |= UISTATE_OVER;
		}

		m_pUISystem->DrawSkin(pNewRect, m_pPathTexture, m_cColor, iState);
	}
	// if not textured, or textured but textureid not set, render the defualt button
	else
	{
		m_pUISystem->DrawQuad(pNewRect, m_cColor);
	}

	// draw the slider button
	pNewRect = UIRect(pAbsoluteRect.fLeft + m_pSliderRect.fLeft, pAbsoluteRect.fTop + m_pSliderRect.fTop, m_pSliderRect.fWidth, m_pSliderRect.fHeight);

	// if textured and textureid set, draw the texture
	if (m_pSliderTexture.iTextureID > -1)
	{
		iState = UISTATE_UP;
		// get the correct texture
		if (m_iState & UISCROLLBARSTATE_SLIDER_OVER)
		{
			iState |= UISTATE_OVER;
		}

		m_pUISystem->DrawSkin(pNewRect, m_pSliderTexture, m_cColor, iState);
	}
	// if not textured, or textured but textureid not set, render the defualt button
	else
	{
		m_pUISystem->DrawButton(pNewRect, m_cColor, 1.0f, 0);
	}

	// draw the first button
	pNewRect = UIRect(pAbsoluteRect.fLeft + m_pMinusRect.fLeft, pAbsoluteRect.fTop + m_pMinusRect.fTop, m_pMinusRect.fWidth, m_pMinusRect.fHeight);

	// if textured and textureid set, draw the texture
	if (m_pMinusTexture.iTextureID > -1)
	{
		// get the correct texture
		if (m_iState & UISCROLLBARSTATE_MINUS_DOWN)
		{
			iState = UISTATE_DOWN;
		}
		else
		{
			iState = UISTATE_UP;
		}

		if (m_iState & UISCROLLBARSTATE_MINUS_OVER)
		{
			iState |= UISTATE_OVER;
		}

		m_pUISystem->DrawSkin(pNewRect, m_pMinusTexture, m_cColor, iState);
	}
	else
	{
		IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

		if ((m_iState & UISCROLLBARSTATE_MINUS_DOWN) == 0)
		{
			m_pUISystem->DrawButton(pNewRect, m_cColor, 1.0f, 0);
			m_pUISystem->DrawText(pNewRect, UIALIGN_CENTER, UIALIGN_MIDDLE, pFont, L"-");
		}
		else
		{
			m_pUISystem->DrawButton(pNewRect, m_cColor, 1.0f, 1);
			m_pUISystem->DrawText(UIRect(pNewRect.fLeft + 1.0f, pNewRect.fTop + 1.0f, pNewRect.fHeight, pNewRect.fHeight), UIALIGN_CENTER, UIALIGN_MIDDLE, pFont, L"-");
		}
	}


	// draw second button
	pNewRect = UIRect(pAbsoluteRect.fLeft + m_pPlusRect.fLeft, pAbsoluteRect.fTop + m_pPlusRect.fTop, m_pPlusRect.fWidth, m_pPlusRect.fHeight);

	// if textured and textureid set, draw it
	if (m_pPlusTexture.iTextureID > -1)
	{	
		if (m_iState & UISCROLLBARSTATE_PLUS_DOWN)
		{
			iState = UISTATE_DOWN;
		}
		else
		{
			iState = UISTATE_UP;
		}

		if (m_iState & UISCROLLBARSTATE_PLUS_OVER)
		{
			iState |= UISTATE_OVER;
		}

		m_pUISystem->DrawSkin(pNewRect, m_pPlusTexture, m_cColor, iState);
	}
	// if not textured, or textured but textureid not set, render the defualt button
	else
	{
		IFFont *pFont = m_pUISystem->GetIFont(m_pFont);
	
		if ((m_iState & UISCROLLBARSTATE_PLUS_DOWN) == 0)
		{
			m_pUISystem->DrawButton(pNewRect, m_cColor, 1.0f, 0);
			m_pUISystem->DrawText(pNewRect, UIALIGN_CENTER, UIALIGN_MIDDLE, pFont, L"+");
		}
		else
		{
			m_pUISystem->DrawButton(pNewRect, m_cColor, 1.0f, 1);
			m_pUISystem->DrawText(UIRect(pNewRect.fLeft + 1.0f, pNewRect.fTop + 1.0f, pNewRect.fHeight, pNewRect.fHeight), UIALIGN_CENTER, UIALIGN_MIDDLE, pFont, L"+");
		}
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
void CUIScrollBar::InitializeTemplate(IScriptSystem *pScriptSystem)
{
	_ScriptableEx<CUIScrollBar>::InitializeTemplate(pScriptSystem);

	REGISTER_COMMON_MEMBERS(pScriptSystem, CUIScrollBar);

	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetType);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetValue);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetValue);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetStep);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetStep);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetSliderColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetSliderColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetMinusColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetMinusColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetPlusColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetPlusColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetSliderSize);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetSliderSize);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetButtonSize);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetButtonSize);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetPathTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetPathTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetSliderTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetSliderTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetSliderOverTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetSliderOverTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetMinusTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetMinusTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetMinusOverTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetMinusOverTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetMinusDownTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetMinusDownTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetPlusTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetPlusTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetPlusOverTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetPlusOverTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, SetPlusDownTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIScrollBar, GetPlusDownTexture);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetValue(float fValue)
{
	m_fValue = fValue;

	OnChanged();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
float CUIScrollBar::GetValue()
{
	float fValue = CLAMP(m_fValue, 0.0f, 1.0f);

	return fValue;
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetStep(float fStep)
{
	m_fStep = fStep;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
float CUIScrollBar::GetStep()
{
	return m_fStep;
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetType()
{
	// set the default type, if not set
	if ((m_iType != UISCROLLBARTYPE_HORIZONTAL) &&  (m_iType != UISCROLLBARTYPE_VERTICAL))
	{
		if (m_pRect.fHeight > m_pRect.fWidth)
		{
			return UISCROLLBARTYPE_VERTICAL;
		}
		else
		{
			return UISCROLLBARTYPE_HORIZONTAL;
		}
	}

	return m_iType;
}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::MoveSlider(float fDelta)
{
	UIRect pRect(m_pRect);

	if (m_pBorder.fSize > 0.125f)
	{
		m_pUISystem->AdjustRect(&pRect, pRect, m_pBorder.fSize);
	}

	if (GetType() == UISCROLLBARTYPE_HORIZONTAL)
	{
		m_fButtonSize = m_pUISystem->AdjustWidth(m_fButtonSize);
		m_fSliderSize = m_pUISystem->AdjustWidth(m_fSliderSize);

		m_pSliderRect.fLeft += fDelta;
		m_fSliderClick += fDelta;

		// clamp the position to only within our path
		if (m_pSliderRect.fLeft > pRect.fWidth - m_fButtonSize - m_fSliderSize)
		{
			m_pSliderRect.fLeft = pRect.fWidth - m_fButtonSize - m_fSliderSize;

			float fAbsX, fAbsY;
			m_pUISystem->GetAbsoluteXY(&fAbsX, &fAbsY, m_pSliderRect.fLeft, m_pSliderRect.fTop, this);

			m_fSliderClick = fAbsX + m_fSliderOffset;
		}
		else if (m_pSliderRect.fLeft < m_fButtonSize)
		{
			m_pSliderRect.fLeft = m_fButtonSize;

			float fAbsX, fAbsY;
			m_pUISystem->GetAbsoluteXY(&fAbsX, &fAbsY, m_pSliderRect.fLeft, m_pSliderRect.fTop, this);

			m_fSliderClick = fAbsX + m_fSliderOffset;
		}
	}
	else
	{
		m_fButtonSize = m_pUISystem->AdjustHeight(m_fButtonSize);
		m_fSliderSize = m_pUISystem->AdjustHeight(m_fSliderSize);

		m_pSliderRect.fTop += fDelta;
		m_fSliderClick += fDelta;

		// clamp the position to only within our path
		if (m_pSliderRect.fTop > pRect.fHeight - m_fButtonSize - m_fSliderSize)
		{
			m_pSliderRect.fTop = pRect.fHeight - m_fButtonSize - m_fSliderSize;

			float fAbsX, fAbsY;
			m_pUISystem->GetAbsoluteXY(&fAbsX, &fAbsY, m_pSliderRect.fLeft, m_pSliderRect.fTop, this);

			m_fSliderClick = fAbsY + m_fSliderOffset;
		}
		else if (m_pSliderRect.fTop < m_fButtonSize)
		{
			m_pSliderRect.fLeft = m_fButtonSize;

			float fAbsX, fAbsY;
			m_pUISystem->GetAbsoluteXY(&fAbsX, &fAbsY, m_pSliderRect.fLeft, m_pSliderRect.fTop, this);

			m_fSliderClick = fAbsY + m_fSliderOffset;
		}	
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::UpdateRect()
{
	// make the rect relative to the widget
	UIRect pRect(0, 0, m_pRect.fWidth, m_pRect.fHeight);

//	m_pUISystem->GetRelativeXY(&pRect.fLeft, &pRect.fTop, pRect.fLeft, pRect.fTop, this);

	if (m_pBorder.fSize > 0.125f)
	{
		m_pUISystem->AdjustRect(&pRect, pRect, m_pBorder.fSize);
	}

	switch (GetType())
	{
	case UISCROLLBARTYPE_HORIZONTAL:

		m_fButtonSize = m_pUISystem->AdjustWidth(m_fButtonSize);
		m_fSliderSize = m_pUISystem->AdjustWidth(m_fSliderSize);

		m_pMinusRect = UIRect(pRect.fLeft, pRect.fTop, m_fButtonSize, pRect.fHeight);
		m_pPlusRect = UIRect(pRect.fLeft + pRect.fWidth - m_fButtonSize, pRect.fTop, m_fButtonSize, pRect.fHeight);

		m_fPathSize = m_pPlusRect.fLeft - (m_pMinusRect.fLeft + m_pMinusRect.fWidth + m_fSliderSize);

		m_pSliderRect = UIRect(pRect.fLeft + m_fButtonSize + m_fValue * m_fPathSize, pRect.fTop, m_fSliderSize, pRect.fHeight);
		m_pPathRect = UIRect(pRect.fLeft + m_fButtonSize, pRect.fTop, pRect.fWidth - m_fButtonSize - m_fButtonSize, pRect.fHeight);

		break;

	case UISCROLLBARTYPE_VERTICAL:

		m_fButtonSize = m_pUISystem->AdjustHeight(m_fButtonSize);
		m_fSliderSize = m_pUISystem->AdjustHeight(m_fSliderSize);

		m_pMinusRect = UIRect(pRect.fLeft, pRect.fTop, pRect.fWidth, m_fButtonSize);
		m_pPlusRect = UIRect(pRect.fLeft, pRect.fTop + pRect.fHeight - m_fButtonSize, pRect.fWidth, m_fButtonSize);

		m_fPathSize = m_pPlusRect.fTop - (m_pMinusRect.fTop + m_pMinusRect.fHeight + m_fSliderSize);

		m_pSliderRect = UIRect(pRect.fLeft, pRect.fTop + m_fButtonSize + m_fValue * m_fPathSize, pRect.fWidth, m_fSliderSize);
		m_pPathRect = UIRect(pRect.fLeft, pRect.fTop + m_fButtonSize, pRect.fWidth, pRect.fHeight - m_fButtonSize - m_fButtonSize);

		break;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
// Script Functions
//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetType(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetType, m_iType);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetValue(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetValue, m_fValue);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetValue(IFunctionHandler *pH)
{
	float fValue = GetValue();

	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetValue, fValue);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetStep(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetStep, m_fStep);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetStep(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetStep, m_fStep);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetSliderColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetSliderColor, m_cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetSliderColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetSliderColor, m_cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetMinusColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetMinusColor, m_cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetMinusColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetMinusColor, m_cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetPlusColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetPlusColor, m_cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetPlusColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetPlusColor, m_cColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetSliderSize(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetSliderSize, m_fSliderSize);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetSliderSize(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetSliderSize, m_fSliderSize);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetButtonSize(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetButtonSize, m_fButtonSize);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetButtonSize(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetButtonSize, m_fButtonSize);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetPathTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetPathTexture, m_pPathTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetPathTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetPathTexture, m_pPathTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetSliderTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetSliderTexture, m_pSliderTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetSliderTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetSliderTexture, m_pSliderTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetSliderOverTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetSliderOverTexture, m_pSliderTexture.iOverTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetSliderOverTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetSliderOverTexture, m_pSliderTexture.iOverTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetMinusTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetMinusTexture, m_pMinusTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetMinusTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetMinusTexture, m_pMinusTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetMinusOverTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetMinusOverTexture, m_pMinusTexture.iOverTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetMinusOverTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetMinusOverTexture, m_pMinusTexture.iOverTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetMinusDownTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetMinusDownTexture, m_pMinusTexture.iDownTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetMinusDownTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetMinusDownTexture, m_pMinusTexture.iDownTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetPlusTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetPlusTexture, m_pPlusTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetPlusTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetPlusTexture, m_pPlusTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetPlusOverTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetPlusOverTexture, m_pPlusTexture.iOverTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetPlusOverTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetPlusOverTexture, m_pPlusTexture.iOverTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::SetPlusDownTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetPlusDownTexture, m_pPlusTexture.iDownTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIScrollBar::GetPlusDownTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetPlusDownTexture, m_pPlusTexture.iDownTextureID);
}

//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - A ComboBox
//
// History:
//  - [10/7/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#include "StdAfx.h"
#include <vector>
#include <algorithm>
#include <iostream>

#include "UIComboBox.h"
#include "UIScrollBar.h"
#include "UISystem.h"


_DECLARE_SCRIPTABLEEX(CUIComboBox);



CUIComboBox *CUIComboBox::m_pStaticThis = 0;


//------------------------------------------------------------------------------------------------- 
CUIComboBox::CUIComboBox()
: m_iSelectionIndex(0),
	m_iHAlignment(UIALIGN_LEFT),
	m_iVAlignment(UIALIGN_CENTER),
	m_cButtonColor(color4f(1.0f, 1.0f, 1.0f, 1.0f)),
	m_iState(0),
	m_iOldZ(0),
	m_iNewIndex(1),
	m_bNeedSort(0),
	m_iSortOrder(UISORT_ASCENDING),
	m_fLeftSpacing(0),
	m_iMaxItems(6),
	m_iFirstItem(0),
	m_pVScroll(0),
	m_bVerticalScrollBar(0),
	m_cItemBgColor(0.0f, 0.0f, 0.0f, 0.35f),
	m_iRollUp(0)
{
}

//------------------------------------------------------------------------------------------------- 
CUIComboBox::~CUIComboBox()
{
	Clear();
}

//------------------------------------------------------------------------------------------------- 
string CUIComboBox::GetClassName()
{
	return UICLASSNAME_COMBOBOX;
}

//------------------------------------------------------------------------------------------------- 
LRESULT CUIComboBox::Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam)	//AMD Port
{
	m_fButtonSize = m_pUISystem->AdjustWidth(m_fButtonSize);

	switch (iMessage)
	{
	case UIM_KEYUP:
	case UIM_KEYPRESSED:
		{
			switch(lParam)
			{
			case XKEY_DOWN:
			case XKEY_MWHEEL_DOWN:
				{
					if (m_iState & UICOMBOBOXSTATE_DROPDOWNLIST)
					{
						if (m_pVScroll)
						{
							// scroll 1 item down
							m_pUISystem->SendMessage(m_pVScroll, UIM_KEYDOWN, 0, XKEY_DOWN);
						}
					}
					else if (!m_vItemList.empty())
					{
						if (m_iSelectionIndex)
						{
							for (CUIComboItemItor it = m_vItemList.begin(); it != m_vItemList.end(); ++it)
							{
								if ((*it)->iIndex == m_iSelectionIndex)
								{
									++it;

									if (it != m_vItemList.end())
									{
										m_iSelectionIndex = (*it)->iIndex;

										OnChanged();
									}

									break;
								}
							}
						}
						else
						{
							m_iSelectionIndex = m_vItemList[0]->iIndex;

							OnChanged();
						}
					}
				}
				break;
			case XKEY_UP:
			case XKEY_MWHEEL_UP:
				{
					if (m_iState & UICOMBOBOXSTATE_DROPDOWNLIST)
					{
						if (m_pVScroll)
						{
							// scroll 1 items up
							m_pUISystem->SendMessage(m_pVScroll, UIM_KEYDOWN, 0, XKEY_UP);
						}
					}
					else if (!m_vItemList.empty())
					{
						if (m_iSelectionIndex)
						{
							for (CUIComboItemList::reverse_iterator rit = m_vItemList.rbegin(); rit != m_vItemList.rend(); ++rit)
							{
								if ((*rit)->iIndex == m_iSelectionIndex)
								{
									++rit;

									if (rit != m_vItemList.rend())
									{
										m_iSelectionIndex = (*rit)->iIndex;

										OnChanged();
									}

									break;
								}
							}
						}
						else
						{
							m_iSelectionIndex = 0;

							OnChanged();
						}
					}
				}
				break;
			}
		}
		break;
	case UIM_MOUSEUP:
		{
			float fMouseX = UIM_GET_X_FLOAT(wParam);
			float fMouseY = UIM_GET_Y_FLOAT(wParam);

			m_pUISystem->GetRelativeXY(&fMouseX, &fMouseY, fMouseX, fMouseY, this);

			UIRect pRect(0.0f, 0.0f, m_pRect.fWidth, m_pRect.fHeight);

			if (!m_pUISystem->PointInRect(pRect, fMouseX, fMouseY))
			{
				if ((m_iState & UICOMBOBOXSTATE_JUSTDROPEDLIST) == 0)
				{
					UndropList();
				}
				else
				{
					m_iState &= ~UICOMBOBOXSTATE_JUSTDROPEDLIST;
					m_iState &= ~UICOMBOBOXSTATE_BUTTONDOWN;
				}
			}
			else
			{
				if ((m_iState & UICOMBOBOXSTATE_DROPDOWNLIST) && ((m_iState & UICOMBOBOXSTATE_JUSTDROPEDLIST) == 0))
				{
					UIRect pListRect(GetListRect(0));

					pListRect.fWidth -= m_fButtonSize + (m_pBorder.fSize > 0.125 ? m_pUISystem->AdjustWidth(m_pBorder.fSize) : 0.0f);

					if (m_pUISystem->PointInRect(pListRect, fMouseX, fMouseY))
					{
						float fY = fMouseY - pListRect.fTop;
						int iItem = (int)(fY / m_fItemHeight);;

						if (m_iRollUp)
						{
							iItem = m_iFirstItem + min(m_iMaxItems - 1, (int)m_vItemList.size() - 1) - iItem;
						}
						else
						{
							iItem += m_iFirstItem;
						}

						if (iItem == (int)m_vItemList.size())
						{
							--iItem;
						}

						assert(iItem >= 0 && iItem < (int)m_vItemList.size());

						m_iSelectionIndex = m_vItemList[iItem]->iIndex;

						OnChanged();

						UndropList();
					}
					else if (m_bVerticalScrollBar && m_pVScroll)
					{
						UIRect pRect(GetListRect(0));

						pRect.fLeft += pRect.fWidth - m_fButtonSize;
						pRect.fWidth = m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;

						if (!m_pUISystem->PointInRect(pRect, fMouseX, fMouseY))
						{
							UndropList();
						}
					}
					else
					{
						UndropList();
					}
				}
				else
				{
					m_iState &= ~UICOMBOBOXSTATE_JUSTDROPEDLIST;
				}
			}
			m_iState &= ~UICOMBOBOXSTATE_BUTTONDOWN;
		}
		break;
	case UIM_MOUSELEAVE:
		{
			m_iState &= ~UICOMBOBOXSTATE_BUTTONDOWN;
			m_iState &= ~UICOMBOBOXSTATE_BUTTONOVER;
		}
		break;
	case UIM_LBUTTONDOWN:
		{
			// check if we need to show the list
			if ((m_iState & UICOMBOBOXSTATE_DROPDOWNLIST) == 0)
			{
				DropList();
			}

			UIRect pButtonRect = GetComboRect();

			if (m_fButtonSize > 0.125f)
			{
				pButtonRect.fLeft += pButtonRect.fWidth - m_fButtonSize;
			}

			if (m_pUISystem->PointInRect(pButtonRect, UIM_GET_X_FLOAT(wParam), UIM_GET_Y_FLOAT(wParam)))
			{
				m_iState |= UICOMBOBOXSTATE_BUTTONDOWN;
			}
			
			return 1;
		}
		break;
	case UIM_MOUSEOVER:
		{
			UIRect pButtonRect = GetComboRect();

			if (m_fButtonSize > 0.125f)
			{
				pButtonRect.fLeft += pButtonRect.fWidth - m_fButtonSize;
			}

			if (m_pUISystem->PointInRect(pButtonRect, UIM_GET_X_FLOAT(wParam), UIM_GET_Y_FLOAT(wParam)))
			{
				m_iState |= UICOMBOBOXSTATE_BUTTONOVER;
			}
			else
			{
				m_iState &= ~UICOMBOBOXSTATE_BUTTONOVER;
			}
		}
		break;
	case UIM_LOSTFOCUS:
		{
			if (m_iState & UICOMBOBOXSTATE_DROPDOWNLIST)
			{
				UndropList();
			}
		}
		break;
	}

	if (iMessage == UIM_DRAW)
	{
		m_pVScroll = (CUIScrollBar *)GetChild("vscrollbar");

		// check if we need a scrollbar
		bool bVScroll = ((int)m_vItemList.size() > m_iMaxItems);

		if (bVScroll)
		{
			if (!m_pVScroll)
			{
				m_pUISystem->CreateScrollBar(&m_pVScroll, this, "vscrollbar", UIRect(0, 0, m_fButtonSize, m_iMaxItems * m_fItemHeight), UIFLAG_ENABLED, 0, UISCROLLBARTYPE_VERTICAL);
			}
			assert(m_pVScroll);

			if (!m_bVerticalScrollBar)
			{
				m_bVerticalScrollBar = 1;

				if (m_iRollUp)
				{
					m_pVScroll->SetValue(1.0f);
				}
				else
				{
					m_pVScroll->SetValue(0.0f);
				}
				m_iFirstItem = 0;
			}
		}
		else
		{
			m_iFirstItem = 0;
			m_bVerticalScrollBar = 0;
		}

		if ((m_bVerticalScrollBar) && (m_iState & UICOMBOBOXSTATE_DROPDOWNLIST))
		{
			// set the rect
			UIRect pVScrollRect;

			pVScrollRect.fLeft = GetComboRect().fLeft + GetComboRect().fWidth - m_fButtonSize;
			pVScrollRect.fWidth = m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;
			pVScrollRect.fHeight = m_fItemHeight * m_iMaxItems + (m_pBorder.fSize > 0.125f ? m_pUISystem->AdjustHeight(m_pBorder.fSize) : 0.0f);

			if (m_iRollUp)
			{
				pVScrollRect.fTop = 0.0f;
			}
			else
			{
				pVScrollRect.fTop = m_pComboRect.fHeight;
			}

			m_pVScroll->SetRect(pVScrollRect);
			m_pVScroll->SetFlags(m_pVScroll->GetFlags() | UIFLAG_VISIBLE);
			m_pVScroll->SetStep(m_fItemHeight / ((m_vItemList.size() - m_iMaxItems) * m_fItemHeight));

			if (m_iRollUp)
			{
				m_iFirstItem = (int)floorf((1.0f - m_pVScroll->GetValue()) * (m_vItemList.size() - m_iMaxItems) + 0.5f);
			}
			else
			{
				m_iFirstItem = (int)floorf(m_pVScroll->GetValue() * (m_vItemList.size() - m_iMaxItems) + 0.5f);
			}

			assert(m_iFirstItem < (int)m_vItemList.size() && m_iFirstItem >= 0);
		}
		else if (m_pVScroll)
		{
			m_pVScroll->SetFlags(m_pVScroll->GetFlags() & ~UIFLAG_VISIBLE);
		}
	}

	return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::Draw(int iPass)
{
	if ((iPass != 0) && (iPass != 1))
	{
		return 1;
	}

	m_pUISystem->BeginDraw(this);

	m_fButtonSize = m_pUISystem->AdjustWidth(m_fButtonSize);

	if (iPass == 0)
	{
		// get the absolute widget rect
		UIRect pAbsoluteRect(m_pComboRect);

		m_pUISystem->GetAbsoluteXY(&pAbsoluteRect.fLeft, &pAbsoluteRect.fTop, m_pComboRect.fLeft, m_pComboRect.fTop, m_pParent);

		// if not transparent draw the shadow
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

		IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

		// now draw the dropdown button
		UIRect pRect = UIRect(pAbsoluteRect.fLeft + pAbsoluteRect.fWidth - m_fButtonSize, pAbsoluteRect.fTop, m_fButtonSize, pAbsoluteRect.fHeight);

		if (m_pButtonTexture.iTextureID > -1)
		{
			int iState = UISTATE_UP;

			if (m_iState & UICOMBOBOXSTATE_BUTTONOVER)
			{
				iState = UISTATE_OVER;
			}
			if (m_iState & UICOMBOBOXSTATE_BUTTONDOWN)
			{
				iState = UISTATE_DOWN;
			}

			m_pUISystem->DrawSkin(pRect, m_pButtonTexture, m_cButtonColor, iState);
		}
		else
		{
			m_pUISystem->DrawButton(pRect, m_cButtonColor, UI_DEFAULT_EMBOSS_BORDER_SIZE, (m_iState & UICOMBOBOXSTATE_BUTTONDOWN) != 0);

			if ((m_iState & UICOMBOBOXSTATE_BUTTONDOWN) != 0)
			{
				pRect.fLeft += UI_DEFAULT_EMBOSS_BORDER_SIZE;
				pRect.fTop += UI_DEFAULT_EMBOSS_BORDER_SIZE;
			}

			m_pUISystem->DrawText(pRect, UIALIGN_CENTER, UIALIGN_MIDDLE, pFont, L"+");
		}

		// now just draw the currently selected item
		UIRect pItemRect(pAbsoluteRect.fLeft, pAbsoluteRect.fTop, pAbsoluteRect.fWidth - m_fButtonSize, pAbsoluteRect.fHeight);

		m_pUISystem->DrawSkin(pItemRect, m_pItemBg, m_cItemBgColor, UISTATE_UP);

		if (m_iSelectionIndex)
		{	
			UIComboItem *pItem = GetItem(m_iSelectionIndex);

			assert(pItem);

			if (pItem->pTexture.iTextureID > -1)
			{
				m_pUISystem->DrawSkin(pItemRect, pItem->pTexture, m_cColor, UISTATE_UP);
			}
			else
			{
				m_pUISystem->DrawQuad(pItemRect, pItem->cColor);
			}

			UIRect pTextRect(pItemRect.fLeft + m_fLeftSpacing, pItemRect.fTop, pItemRect.fWidth - m_fLeftSpacing, pItemRect.fHeight);

			m_pUISystem->SetScissor(&pTextRect);

			if (*pItem->szText.c_str() != 0)
			{
				m_pUISystem->DrawText(pTextRect, m_iHAlignment, m_iVAlignment, pFont, pItem->szText.c_str());
			}
		}

		m_pUISystem->SetScissor(&pItemRect);

		if (m_pTexture.iTextureID > -1)
		{
			m_pUISystem->DrawSkin(pItemRect, m_pTexture, m_cColor, UISTATE_UP);
		}

		// draw a greyed quad ontop, if disabled
		if ((m_iFlags & UIFLAG_ENABLED) == 0)
		{
			m_pUISystem->ResetDraw();
			m_pUISystem->DrawGreyedQuad(pGreyedRect, m_cGreyedColor, m_iGreyedBlend);
		}
	}

	//------------------------------------------------------------------------------------------------- 
	// Second Pass
	//------------------------------------------------------------------------------------------------- 

	// check if list is down and if we are drawing second pass
	if ((m_iState & UICOMBOBOXSTATE_DROPDOWNLIST) && (iPass == 1))
	{
		// get the absolute widget rect
		UIRect pAbsoluteRect(m_pComboRect);

		m_pUISystem->GetAbsoluteXY(&pAbsoluteRect.fLeft, &pAbsoluteRect.fTop, m_pComboRect.fLeft, m_pComboRect.fTop, m_pParent);

		// get the font
		IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

		//------------------------------------------------------------------------------------------------- 
		UIRect pItemRect(pAbsoluteRect);
		float fBorderSize = (m_pBorder.fSize > 0.125f ? m_pBorder.fSize : 0.0f);
		int iItemCount = ((int)m_vItemList.size() <= m_iMaxItems ? (int)m_vItemList.size() : m_iMaxItems);

		pItemRect.fLeft += m_pUISystem->AdjustWidth(fBorderSize);
		pItemRect.fWidth -= (m_fButtonSize + 2.0f * m_pUISystem->AdjustWidth(fBorderSize));

		if (m_iRollUp)
		{
			pItemRect.fTop -= m_fItemHeight * iItemCount;
			pItemRect.fHeight = m_fItemHeight * iItemCount;
		}
		else
		{
			pItemRect.fTop += pAbsoluteRect.fHeight;
			pItemRect.fHeight = m_fItemHeight * iItemCount;
		}

		if (m_pBorder.fSize > 0.125f)
		{
			UIRect pBorderRect;

			m_pUISystem->AdjustRect(&pBorderRect, pItemRect, m_pBorder.fSize, 1);
			m_pUISystem->DrawBorder(pBorderRect, m_pBorder);
		}

		if (m_iRollUp)
		{
			pItemRect.fTop += (iItemCount-1) * m_fItemHeight;
		}

		pItemRect.fHeight = m_fItemHeight;

		int iFirstItem = CLAMP(m_iFirstItem, 0, m_vItemList.size());
		int iLastItem = iFirstItem + m_iMaxItems;
		
		for (CUIComboItemItor pItor = m_vItemList.begin() + m_iFirstItem; (pItor != m_vItemList.end()); ++pItor)
		{
			if (m_iFirstItem + m_iMaxItems < m_vItemList.size())
			{
				if (pItor >= m_vItemList.begin() + m_iFirstItem + m_iMaxItems)
				{
					break;
				}
			}
			
			UIComboItem *pItem = *pItor;

			m_pUISystem->SetScissor(&pItemRect);

			m_pUISystem->DrawSkin(pItemRect, m_pItemBg, m_cItemBgColor, UISTATE_UP);

			if (pItem->pTexture.iTextureID > -1)
			{
				m_pUISystem->DrawImage(pItemRect, pItem->pTexture, m_cColor);
			}
			else
			{
				m_pUISystem->DrawQuad(pItemRect, pItem->cColor);
			}

			UIRect pTextRect(pItemRect.fLeft + m_fLeftSpacing, pItemRect.fTop, pItemRect.fWidth - m_fLeftSpacing, pItemRect.fHeight);

			m_pUISystem->SetScissor(&pTextRect);

			m_pUISystem->DrawText(pTextRect, m_iHAlignment, m_iVAlignment, pFont, pItem->szText.c_str());

			if (m_iRollUp)
			{
				pItemRect.fTop -= pItemRect.fHeight;
			}
			else
			{
				pItemRect.fTop += pItemRect.fHeight;
			}
		}
	}

	m_pUISystem->EndDraw();

	// draw children on second pass
	if (iPass == 1)
	{
		// draw the children
		if (m_pUISystem->ShouldSortByZ())
		{
			SortChildrenByZ();
		}

		DrawChildren();
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
void CUIComboBox::InitializeTemplate(IScriptSystem *pScriptSystem)
{
	_ScriptableEx<CUIComboBox>::InitializeTemplate(pScriptSystem);

	REGISTER_COMMON_MEMBERS(pScriptSystem, CUIComboBox);

	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, SetButtonSize);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetButtonSize);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, SetItemHeight);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetItemHeight);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, ClearSelection);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, IsDropDown);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, SelectIndex);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, DeselectIndex);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, IsSelectedIndex);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, Select);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, Deselect);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, IsSelected);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetSelection);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetSelectionIndex);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetItemCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, AddItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, InsertItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, RemoveItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, SetVAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetVAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, SetHAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetHAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, Clear);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, Sort);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, SetTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, SetButtonTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetButtonTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, SetButtonOverTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetButtonOverTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, SetButtonDownTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetButtonDownTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, SetButtonColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIComboBox, GetButtonColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::ClearSelection()
{
	m_iSelectionIndex = 0;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SelectIndex(int iIndex)
{
	if (GetItem(iIndex))
	{
		m_iSelectionIndex = iIndex;
		UndropList();
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::DeselectIndex(int iIndex)
{
	if (iIndex == m_iSelectionIndex)
	{
		m_iSelectionIndex = 0;
	}
	UndropList();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
bool CUIComboBox::IsSelectedIndex(int iIndex)
{
	return (m_iSelectionIndex == iIndex);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::Select(const wstring &szText)
{
	m_iSelectionIndex = 0;

	UIComboItem *pItem = GetItem(szText);

	if (pItem)
	{
		m_iSelectionIndex = pItem->iIndex;
	}

	UndropList();

	if (m_iSelectionIndex)
	{
		return 1;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::Deselect(const wstring &szText)
{
	UIComboItem *pItem = GetItem(szText);

	if ((pItem) && (pItem->iIndex == m_iSelectionIndex))
	{
			m_iSelectionIndex = 0;

			return 1;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
bool CUIComboBox::IsSelected(const wstring &szText)
{
	UIComboItem *pItem = GetItem(szText);

	if (pItem)
	{
		if (pItem->iIndex == m_iSelectionIndex)
		{
			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetItemCount()
{
	return (int)m_vItemList.size();
}

//------------------------------------------------------------------------------------------------- 
UIComboItem *CUIComboBox::GetItem(int iIndex, CUIComboItemItor *pItemItor)
{
	for (CUIComboItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
	{
		if ((*pItor)->iIndex == iIndex)
		{
			if (pItemItor)
			{
				*pItemItor = pItor;
			}

			return (*pItor);
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
UIComboItem *CUIComboBox::GetItem(const wstring &szItemText, CUIComboItemItor *pItemItor)
{
	for (CUIComboItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
	{
		if ((*pItor)->szText == szItemText)
		{
			if (pItemItor)
			{
				*pItemItor = pItor;
			}

			return (*pItor);
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::AddItem(const wstring &szText, UISkinTexture pTexture, const color4f &cColor)
{
	UIComboItem *pItem = new UIComboItem();

	pItem->szText = szText;
	pItem->pTexture = pTexture;
	pItem->iIndex = m_iNewIndex++;
	pItem->cColor = cColor;

	m_vItemList.push_back(pItem);

	m_bNeedSort = 1;

	return pItem->iIndex;
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::InsertItem(int iPosition, const wstring &szText, const UISkinTexture &pTexture, const color4f &cColor)
{
	if (iPosition < 0 || iPosition > (int)m_vItemList.size())
	{
		return 0;
	}

	UIComboItem *pItem = new UIComboItem();

	pItem->szText = szText;
	pItem->pTexture = pTexture;
	pItem->iIndex = m_iNewIndex++;
	pItem->cColor = cColor;

	m_vItemList.insert(m_vItemList.begin() + iPosition, pItem);

	m_bNeedSort = 1;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::RemoveItem(int iItemIndex)
{
	for (CUIComboItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
	{
		if ((*pItor)->iIndex == iItemIndex)
		{
			delete *pItor;

			m_vItemList.erase(pItor);

			m_bNeedSort = 1;

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::Clear()
{
	m_bNeedSort = 0;
	m_iNewIndex = 1;

	for (CUIComboItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
	{
		delete *pItor;
	}

	m_vItemList.clear();
	m_iSelectionIndex = 0;
	UndropList();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::Sort(int iSortOrder)
{
	m_pStaticThis = this;
	m_bNeedSort = 0;

	std::stable_sort(m_vItemList.begin(), m_vItemList.end(), SortCallback);

	return 1;
}


//------------------------------------------------------------------------------------------------- 
UIRect CUIComboBox::GetComboRect()
{
	UIRect pComboRect(m_pComboRect);

	m_pUISystem->GetAbsoluteXY(&pComboRect.fLeft, &pComboRect.fTop, pComboRect.fLeft, pComboRect.fTop, m_pParent);

	if (m_pBorder.fSize > 0.125f)
	{
		m_pUISystem->AdjustRect(&pComboRect, pComboRect, m_pBorder.fSize);
	}

	m_pUISystem->GetRelativeXY(&pComboRect.fLeft, &pComboRect.fTop, pComboRect.fLeft, pComboRect.fTop, this);

	return pComboRect;
}

//------------------------------------------------------------------------------------------------- 
UIRect CUIComboBox::GetListRect(bool bScrollBars)
{
	UIRect pListRect(0, 0, m_pComboRect.fWidth, m_pComboRect.fHeight);

	// if border is large enough to be visible, exclude it
	if (m_pBorder.fSize > 0.125f)
	{
		m_pUISystem->AdjustRect(&pListRect, pListRect, m_pBorder.fSize);
	}

	if (m_iRollUp)
	{
		pListRect.fHeight = m_fItemHeight * min((int)m_vItemList.size(), m_iMaxItems);
	}
	else
	{
		pListRect.fTop += pListRect.fHeight + (m_pBorder.fSize > 0.125f ? m_pBorder.fSize : 0.0f);
		pListRect.fHeight = m_fItemHeight * min((int)m_vItemList.size(), m_iMaxItems);
	}

	if (bScrollBars && m_bVerticalScrollBar)
	{
		if (m_pVScroll)
		{
			pListRect.fWidth -= m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;
		}
	}

	return pListRect;
}

//------------------------------------------------------------------------------------------------- 
void CUIComboBox::DropList()
{
	m_iState |= UICOMBOBOXSTATE_DROPDOWNLIST;
	m_iState |= UICOMBOBOXSTATE_JUSTDROPEDLIST;

	m_pRect = m_pComboRect;

	if (m_iRollUp)
	{
		float fRollUpHeight = m_fItemHeight * min((int)m_vItemList.size(), m_iMaxItems) + (m_pBorder.fSize > 0.125f ? m_pBorder.fSize : 0.0f);

		m_pRect.fHeight += fRollUpHeight;
		m_pRect.fTop -= fRollUpHeight;
	}
	else
	{
		m_pRect.fHeight += m_fItemHeight * min((int)m_vItemList.size(), m_iMaxItems) + (m_pBorder.fSize > 0.125f ? m_pBorder.fSize : 0.0f);
	}

	m_iOldZ = m_iZ; // temporarily change the Z value so that our list is the topmost thing
	m_iZ = 1000000000;

	m_pUISystem->OnZChanged(this);
}

//------------------------------------------------------------------------------------------------- 
void CUIComboBox::UndropList()
{
	m_iState &= ~UICOMBOBOXSTATE_DROPDOWNLIST;
	m_iState &= ~UICOMBOBOXSTATE_JUSTDROPEDLIST;
	m_pRect = m_pComboRect;

	m_iZ = m_iOldZ;

	m_pUISystem->OnZChanged(this);
}

//------------------------------------------------------------------------------------------------- 
// Script Functions
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SetButtonSize(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetButtonSize, m_fButtonSize);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetButtonSize(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetButtonSize, m_fButtonSize);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SetItemHeight(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetItemHeight, m_fItemHeight);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetItemHeight(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetItemHeight, m_fItemHeight);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::ClearSelection(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), ClearSelection, 0);

	m_iSelectionIndex = 0;
	UndropList();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::IsDropDown(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), IsDropDown, 0);

	return pH->EndFunction((bool)(m_iState & UICOMBOBOXSTATE_DROPDOWNLIST));
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SelectIndex(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SelectIndex, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SelectIndex, 1, svtNumber);

	int iItemIndex;

	pH->GetParam(1, iItemIndex);

	return pH->EndFunction(SelectIndex(iItemIndex));
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::DeselectIndex(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), DeselectIndex, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), DeselectIndex, 1, svtNumber);

	int iItemIndex;

	pH->GetParam(1, iItemIndex);

	DeselectIndex(iItemIndex);

	return pH->EndFunctionNull();
}


//------------------------------------------------------------------------------------------------- 
int CUIComboBox::IsSelectedIndex(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), IsSelectedIndex, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), IsSelectedIndex, 1, svtNumber);

	int iItemIndex;

	pH->GetParam(1, iItemIndex);

	return pH->EndFunction(IsSelectedIndex(iItemIndex));
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::Select(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Select, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), Select, 1, svtString, svtNumber);

	wstring szText;

	m_pUISystem->ConvertToWString(szText, pH, 1);

	return pH->EndFunction(Select(szText));
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::Deselect(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Deselect, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), Deselect, 1, svtString, svtNumber);

	wstring szText;

	m_pUISystem->ConvertToWString(szText, pH, 1);

	return pH->EndFunction(Deselect(szText));
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::IsSelected(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), IsSelected, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), IsSelected, 1, svtString, svtNumber);

	wstring szText;

	m_pUISystem->ConvertToWString(szText, pH, 1);

	return pH->EndFunction(IsSelected(szText));
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetSelection(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetSelection, 0);

	if (!m_iSelectionIndex)
	{
		return pH->EndFunctionNull();
	}

	UIComboItem *pItem = GetItem(m_iSelectionIndex);

	if (!pItem)
	{
		return pH->EndFunctionNull();
	}

	char szString[1024];

	m_pUISystem->ConvertToString(szString, pItem->szText, 1023);

	return pH->EndFunction(szString);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetSelectionIndex(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetSelectionIndex, 0);

	if (!m_iSelectionIndex)
	{
		return pH->EndFunctionNull();
	}

	return pH->EndFunction(m_iSelectionIndex);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetItem(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetItem, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), GetItem, 1, svtNumber);

	int iItemIndex;

	pH->GetParam(1, iItemIndex);

	UIComboItem *pItem = GetItem(iItemIndex);

	char szString[1024];

	m_pUISystem->ConvertToString(szString, pItem->szText, 1023);

	return pH->EndFunction(szString);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetItemCount(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetItemCount, 0);

	return pH->EndFunction(GetItemCount());
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::AddItem(IFunctionHandler *pH)
{
	if (pH->GetParamCount() < 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), AddItem, 1);
	}
	else if (pH->GetParamCount() > 4)
	{
		CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), AddItem, 4);
	}

	wstring	szText = L"";
	UISkinTexture pTexture;
	char					*szRect = 0;
	color4f	cColor = color4f(0,0,0,0);

	// get the texture first
	if (pH->GetParamCount() > 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), AddItem, 2, svtUserData, svtNull);

		if (pH->GetParamType(2) == svtUserData)
		{
			int iCookie = 0;

			pH->GetParamUDVal(2, pTexture.iTextureID, iCookie);
		}
	}

	// then get the texrect
	if (pH->GetParamCount() > 2)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), AddItem, 3, svtString, svtNull);

		if (pH->GetParamType(3) == svtString)
		{
			pH->GetParam(3, szRect);

			m_pUISystem->RetrieveTexRect(pTexture.vTexCoord, pTexture.iTextureID, szRect);
		}
	}

	if (pH->GetParamCount() > 3)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddItem, 4, svtString);

		char *szColor = 0;

		if (pH->GetParam(4, szColor))
		{
			m_pUISystem->RetrieveColor(&cColor, szColor);
		}
	}

	if ((pH->GetParamType(1) == svtString) || (pH->GetParamType(1) == svtNumber))
	{
		// get text
		m_pUISystem->ConvertToWString(szText, pH, 1);
	}

	return pH->EndFunction(AddItem(szText, pTexture, cColor));
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::InsertItem(IFunctionHandler *pH)
{
	if (pH->GetParamCount() < 2)
	{
		CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), InsertItem, 2);
	}
	else if (pH->GetParamCount() > 5)
	{
		CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), InsertItem, 5);
	}

	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), InsertItem, 1, svtNumber);

	wstring	szText = L"";
	UISkinTexture pTexture;
	char					*szRect = 0;
	color4f	cColor = color4f(0,0,0,0);
	int						iPosition;

	// get the texture first
	if (pH->GetParamCount() > 2)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), InsertItem, 3, svtUserData, svtNull);

		if (pH->GetParamType(2) == svtUserData)
		{
			int iCookie = 0;

			pH->GetParamUDVal(3, pTexture.iTextureID, iCookie);
		}
	}

	// then get the texrect
	if (pH->GetParamCount() > 3)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), AddItem, 4, svtString, svtNull);

		if (pH->GetParamType(4) == svtString)
		{
			pH->GetParam(4, szRect);

			m_pUISystem->RetrieveTexRect(pTexture.vTexCoord, pTexture.iTextureID, szRect);
		}
	}

	if (pH->GetParamCount() > 4)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddItem, 5, svtString);

		char *szColor = 0;

		if (pH->GetParam(5, szColor))
		{
			m_pUISystem->RetrieveColor(&cColor, szColor);
		}
	}

	if ((pH->GetParamType(2) == svtString) || (pH->GetParamType(2) == svtNumber))
	{
		// get text
		m_pUISystem->ConvertToWString(szText, pH, 2);
	}

	pH->GetParam(1, iPosition);

	return pH->EndFunction(InsertItem(iPosition, szText, pTexture, cColor));
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::RemoveItem(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), RemoveItem, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), RemoveItem, 1, svtNumber);

	int iItemIndex;

	pH->GetParam(1, iItemIndex);

	RemoveItem(iItemIndex);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SetVAlign(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetVAlign, m_iVAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetVAlign(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetVAlign, m_iVAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SetHAlign(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetHAlign, m_iHAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetHAlign(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetHAlign, m_iHAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::Clear(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Sort, 0);

	Clear();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::Sort(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Sort, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), Sort, 1, svtNumber);

	int iSortOrder;

	pH->GetParam(1, iSortOrder);

	Sort(iSortOrder);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SetTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetTexture, m_pTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetTexture, m_pTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SetButtonTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetButtonTexture, m_pButtonTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetButtonTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetButtonTexture, m_pButtonTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SetButtonOverTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetButtonOverTexture, m_pButtonTexture.iOverTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetButtonOverTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetButtonOverTexture, m_pButtonTexture.iOverTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SetButtonDownTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetButtonDownTexture, m_pButtonTexture.iDownTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetButtonDownTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetButtonDownTexture, m_pButtonTexture.iDownTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::SetButtonColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetButtonColor, m_cButtonColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIComboBox::GetButtonColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetButtonColor, m_cButtonColor);
}
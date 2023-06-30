//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - A ListView
//
// History:
//  - [23/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#include "StdAfx.h"
#include "UIListView.h"
#include "UIScrollBar.h"
#include "UISystem.h"


#include <vector>
#include <algorithm>
#include <iostream>



_DECLARE_SCRIPTABLEEX(CUIListView);


CUIListView *CUIListView::m_pStaticThis = 0;


//------------------------------------------------------------------------------------------------- 
CUIListView::CUIListView()
: m_fHeaderHeight(16.0f),
	m_fItemHeight(18.0f),
	m_fVerticalOffset(0),
	m_fHorizontalOffset(0),
	m_bVerticalScrollBar(0),
	m_bHorizontalScrollBar(0),
	m_iSortOrder(UISORT_ASCENDING),
	m_iSortColumn(0),
	m_iNewIndex(1),
	m_bColumnSelect(0),
	m_iSelectedColumn(0),
	m_bNeedSort(0),
	m_pVScroll(0),
	m_pHScroll(0),
	m_fCellSpacing(0.0f),
	m_fCellPadding(0.0f),
	m_fTotalWidth(0.0f),
	m_fTotalHeight(0.0f),
	m_fColumnTextSpacing(0),
	m_cSelectionColor(0.0f, 0.0f, 0.8f, 0.8f),
	m_iNoSort(0),
	m_cSortByColor(0.0f, 0.0f, 0.85f, 1.0f),
	m_cSortByTextColor(1.0f, 1.0f, 1.0f, 1.0f)
{
}

//------------------------------------------------------------------------------------------------- 
CUIListView::~CUIListView()
{
	Clear();
}

//------------------------------------------------------------------------------------------------- 
string CUIListView::GetClassName()
{
	return UICLASSNAME_LISTVIEW;
}

//------------------------------------------------------------------------------------------------- 
LRESULT CUIListView::Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam)	//AMD Port
{
	if (iMessage == UIM_DRAW)
	{
		m_pHScroll = (CUIScrollBar *)GetChild("hscrollbar");
		m_pVScroll = (CUIScrollBar *)GetChild("vscrollbar");

		UIRect pListRect = GetListRect(1);

		bool bVScroll = 0, bHScroll = 0;

		// check if we need an horizontal scrollbar
		m_fTotalWidth = 0.0f;

		for (unsigned int i = 0; i < m_vColumnList.size(); i++)
		{
			m_fTotalWidth += m_vColumnList[i].fWidth + m_fCellSpacing;
		}

		m_fTotalWidth -= m_fCellSpacing;
		m_fTotalHeight = m_vItemList.size() * (m_fItemHeight + m_fCellSpacing);

		bVScroll = (m_fTotalHeight > pListRect.fHeight - GetHeaderRect().fHeight - m_fCellSpacing);
		bHScroll = (m_fTotalWidth > pListRect.fWidth);

		if (bHScroll)
		{
			if (!m_pHScroll)
			{
				m_pUISystem->CreateScrollBar(&m_pHScroll, this, "hscrollbar", UIRect(0, 0, 0, 16.0f), UIFLAG_ENABLED, 0, UISCROLLBARTYPE_HORIZONTAL);
			}
			assert(m_pHScroll);

			if (!m_bHorizontalScrollBar)
			{
				m_bHorizontalScrollBar = 1;
				m_pHScroll->SetFlags(m_pHScroll->GetFlags() | UIFLAG_VISIBLE);
			}
		}
		else if (m_bHorizontalScrollBar)
		{
			m_fHorizontalOffset = 0;
			m_bHorizontalScrollBar = 0;

			m_pHScroll->SetValue(0.0f);
			m_pHScroll->SetFlags(m_pHScroll->GetFlags() & ~UIFLAG_VISIBLE);
		}

		if (bVScroll)
		{
			if (!m_pVScroll)
			{
				m_pUISystem->CreateScrollBar(&m_pVScroll, this, "vscrollbar", UIRect(0, 0, 16.0f, 0.0f), UIFLAG_ENABLED, 0, UISCROLLBARTYPE_VERTICAL);
			}
			assert(m_pVScroll);

			if (!m_bVerticalScrollBar)
			{
				m_bVerticalScrollBar = 1;
				m_pVScroll->SetFlags(m_pVScroll->GetFlags() | UIFLAG_VISIBLE);
			}
		}
		else if (m_bVerticalScrollBar)
		{
			m_bVerticalScrollBar = 0;
			m_fVerticalOffset = 0;

			m_pVScroll->SetValue(0.0f);
		}


		// so we can calculate the scrollbar rects accurately
		pListRect = GetListRect(0);

		if (m_bVerticalScrollBar)
		{
			UIRect pVScrollRect;
			UIBorder pBorder;
			
			m_pVScroll->GetBorder(&pBorder);

			pVScrollRect.fLeft = m_pUISystem->AdjustWidth(pBorder.fSize) + pListRect.fLeft + pListRect.fWidth - m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;
			pVScrollRect.fTop = pListRect.fTop;
			pVScrollRect.fWidth = m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;
			pVScrollRect.fHeight = pListRect.fHeight - (bHScroll ? m_pUISystem->GetWidgetRect(m_pHScroll).fHeight : 0);

			if ((m_fHeaderHeight > 0.125f) && (m_vColumnList.size()))
			{
				pVScrollRect.fTop += m_fHeaderHeight;
				pVScrollRect.fHeight -= m_fHeaderHeight;
			}

			assert(m_pVScroll);

			m_pVScroll->SetRect(pVScrollRect, 1);
			m_pVScroll->SetStep(max(m_fItemHeight / (m_fTotalHeight - pListRect.fHeight), 0.05f));
		}
		else
		{
			if (m_pVScroll)
			{
				m_pVScroll->SetFlags(m_pVScroll->GetFlags() & ~UIFLAG_VISIBLE);
			}
			else
			{
				if (m_pVScroll = (CUIScrollBar *)GetChild("vscrollbar"))
				{
					m_pVScroll->SetFlags(m_pVScroll->GetFlags() & ~UIFLAG_VISIBLE);
				}
			}
		}

		if (m_bHorizontalScrollBar)
		{
			UIRect pHScrollRect;
			UIBorder pBorder;

			m_pHScroll->GetBorder(&pBorder);

			pHScrollRect.fLeft = pListRect.fLeft;
			pHScrollRect.fTop = m_pUISystem->AdjustHeight(pBorder.fSize) + pListRect.fTop + pListRect.fHeight - m_pUISystem->GetWidgetRect(m_pHScroll).fHeight;
			pHScrollRect.fWidth = pListRect.fWidth - (bVScroll ? m_pUISystem->GetWidgetRect(m_pVScroll).fWidth : 0);
			pHScrollRect.fHeight = m_pUISystem->GetWidgetRect(m_pHScroll).fHeight;

			assert(m_pHScroll);

			m_pHScroll->SetRect(pHScrollRect, 1);
			
			m_pHScroll->SetStep(max((pListRect.fWidth * 0.1f) / (m_fTotalWidth - pListRect.fWidth), 0.05f));
		}
		else
		{
			if (m_pHScroll)
			{
				m_pHScroll->SetFlags(m_pHScroll->GetFlags() & ~UIFLAG_VISIBLE);
			}
			else
			{
				if (m_pHScroll = (CUIScrollBar *)GetChild("hscrollbar"))
				{
					m_pHScroll->SetFlags(m_pHScroll->GetFlags() & ~UIFLAG_VISIBLE);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------- 
	//------------------------------------------------------------------------------------------------- 

	// process messages
	switch (iMessage)
	{
	case UIM_KEYUP:
	case UIM_KEYPRESSED:
		{
			switch(lParam)
			{
			case XKEY_PAGE_DOWN:
			case XKEY_MWHEEL_DOWN:
				{
					if (m_pVScroll)
					{
						// scroll 3 items down
						m_pUISystem->SendMessage(m_pVScroll, UIM_KEYDOWN, 0, XKEY_DOWN);
						m_pUISystem->SendMessage(m_pVScroll, UIM_KEYDOWN, 0, XKEY_DOWN);
						m_pUISystem->SendMessage(m_pVScroll, UIM_KEYDOWN, 0, XKEY_DOWN);
					}
				}
				break;
			case XKEY_PAGE_UP:
			case XKEY_MWHEEL_UP:
				{
					if (m_pVScroll)
					{
						// scroll 3 items up
						m_pUISystem->SendMessage(m_pVScroll, UIM_KEYDOWN, 0, XKEY_UP);
						m_pUISystem->SendMessage(m_pVScroll, UIM_KEYDOWN, 0, XKEY_UP);
						m_pUISystem->SendMessage(m_pVScroll, UIM_KEYDOWN, 0, XKEY_UP);
					}
				}
				break;
			case XKEY_SPACE:
			case XKEY_RETURN:
			case XKEY_NUMPADENTER:
				{
					OnCommand();
				}
				break;
			case XKEY_LEFT:
			case XKEY_RIGHT:
				{
					if (!m_bColumnSelect)
					{
						if (m_pHScroll)
						{
							m_pUISystem->SendMessage(m_pHScroll, UIM_KEYDOWN, 0, lParam);
						}
					}
					else
					{
						if (lParam == XKEY_LEFT)
						{
							int it = m_iSelectedColumn;

							do
							{
								--it;
							} while(it && !m_vColumnList[it].bSelectable);

							if ((it >= 0) && m_vColumnList[it].bSelectable)
							{
								m_iSelectedColumn = it;
							}
						}
						else
						{
							int it = m_iSelectedColumn;

							do
							{
								++it;
							} while((it < m_vColumnList.size()) && !m_vColumnList[it].bSelectable);

							if ((it < m_vColumnList.size()) && (m_vColumnList[it].bSelectable))
							{
								m_iSelectedColumn = it;
							}
						}
					}
				}
				break;
			case XKEY_DOWN:
				{
					if (m_vSelectionList.empty())
					{
						for (CUIListItemItor it = m_vItemList.begin(); it != m_vItemList.end(); ++it)
						{
							if ((*it)->bSelectable)
							{
								SelectIndex((*it)->iIndex);
								OnChanged();

								if (!m_vSelectionList.empty())
								{
									ScrollTo(m_vSelectionList[0]);
								}

								break;
							}
						}
					}
					else
					{
						// get first selected
						int iSelected = *(m_vSelectionList.end()-1);
						int iPosition = 0;

						for (CUIListItemItor it = m_vItemList.begin(); it != m_vItemList.end(); ++it)
						{
							if ((*it)->iIndex == iSelected)
							{
								do
								{
									++it;
									++iPosition;
								} while(it != m_vItemList.end() && !(*it)->bSelectable);

								if (it != m_vItemList.end())
								{
									ClearSelection();
									SelectIndex((*it)->iIndex);
									OnChanged();

									if (!m_vSelectionList.empty())
									{
										ScrollTo(m_vSelectionList[0]);
									}
								}

								break;
							}
							++iPosition;
						}
					}
				}
				break;
			case XKEY_UP:
				{
					if (m_vSelectionList.empty())
					{
						for (CUIListItemItor it = m_vItemList.begin(); it != m_vItemList.end(); ++it)
						{
							if ((*it)->bSelectable)
							{
								SelectIndex((*it)->iIndex);
								OnChanged();

								if (!m_vSelectionList.empty())
								{
									ScrollTo(m_vSelectionList[0]);
								}

								break;
							}
						}
					}
					else
					{
						// get first selected
						int iSelected = *(m_vSelectionList.end()-1);
						int iPosition = 0;

						for (CUIListItemList::reverse_iterator rit = m_vItemList.rbegin(); rit != m_vItemList.rend(); ++rit)
						{
							if ((*rit)->iIndex == iSelected)
							{
								do
								{
									++rit;
									++iPosition;
								} while(rit != m_vItemList.rend() && !(*rit)->bSelectable);

								if (rit != m_vItemList.rend())
								{
									ClearSelection();
									SelectIndex((*rit)->iIndex);
									OnChanged();

									if (!m_vSelectionList.empty())
									{
										ScrollTo(m_vSelectionList[0]);
									}
								}

								break;
							}
							++iPosition;
						}
					}
				}
				break;
			}
		}
		break;
	case UIM_LBUTTONUP:
		{
			// change sort method when you  click a column header
			if (m_vColumnList.size())
			{
				float fX = UIM_GET_X_FLOAT(wParam);
				float fY = UIM_GET_Y_FLOAT(wParam);

				UIRect pHeaderRect = GetHeaderRect();

				if (m_pUISystem->PointInRect(pHeaderRect, fX, fY) && !m_iNoSort)
				{
					int iColumn = FindColumnAt(fX, fY);

					if (iColumn > -1)
					{
						if (iColumn == m_iSortColumn)
						{
							if (m_iSortOrder == UISORT_ASCENDING)
							{
								m_iSortOrder = UISORT_DESCENDING;
							}
							else
							{
								m_iSortOrder = UISORT_ASCENDING;
							}
						}
						else
						{
							m_iSortOrder = UISORT_ASCENDING;
							m_iSortColumn = iColumn;
						}

						Sort();
					}
				}
			}
		}
		break;
	case UIM_LBUTTONDOWN:
		{
			float fX = UIM_GET_X_FLOAT(wParam);
			float fY = UIM_GET_Y_FLOAT(wParam);

			UIListItem *pItem = 0;
			CUIListItemItor pItor;

			UIRect pListRect = GetListRect();

			if (!m_vColumnList.empty())
			{
				pListRect.fTop += GetHeaderRect().fHeight + m_fCellSpacing;

				if (!m_pUISystem->PointInRect(pListRect, fX, fY))
				{
					break;
				}
			}

			int iColumn = 0;
			int iIndex = FindItemAt(fX, fY, &pItem, &iColumn, &pItor);

			if (iIndex > -1 && (!m_bColumnSelect || (m_bColumnSelect && m_vColumnList[iColumn].bSelectable)))
			{
				m_iSelectedColumn = iColumn;

				UIListItem *pSelectedItem = GetItem(iIndex);

				assert(pItem);

				if (!pItem->bSelectable)
				{
					ClearSelection();
					OnChanged();

					break;
				}
				else if (GetStyle() & UISTYLE_MULTISELECTION)
				{
					if ((m_pUISystem->GetIInput()->KeyDown(XKEY_LCONTROL)) || (m_pUISystem->GetIInput()->KeyDown(XKEY_RCONTROL)))
					{
						
						SelectIndex(iIndex);

						m_pSelectionStart = pItor;

						OnChanged();
					}
					else if ((m_pUISystem->GetIInput()->KeyDown(XKEY_LSHIFT)) || (m_pUISystem->GetIInput()->KeyDown(XKEY_LSHIFT)))
					{
						if ((GetSelectionCount() > 0) && (pItor != m_pSelectionStart))
						{
							if (m_pSelectionStart > pItor)
							{
								while (m_pSelectionStart != pItor)
								{
									SelectIndex((*pItor)->iIndex);

									pItor++;
								}

								OnChanged();
							}
							else
							{
								while (m_pSelectionStart != pItor)
								{
									SelectIndex((*pItor)->iIndex);

									pItor--;
								}

								OnChanged();
							}
						}
						else
						{
							SelectIndex(iIndex);

							m_pSelectionStart = pItor;

							OnChanged();
						}
					}
				}
				else
				{
					ClearSelection();

					SelectIndex(iIndex);

					m_pSelectionStart = pItor;

					OnChanged();
				}
			}
			else
			{
				ClearSelection();
				OnChanged();
			}
		}
		break;
	case UIM_LBUTTONDBLCLICK:
		{
			float fX = UIM_GET_X_FLOAT(wParam);
			float fY = UIM_GET_Y_FLOAT(wParam);

      UIListItem *pItem = 0;
			CUIListItemItor pItor;

			UIRect pListRect = GetListRect();

			if (!m_vColumnList.empty())
			{
				pListRect.fTop += GetHeaderRect().fHeight + m_fCellSpacing;

				if (!m_pUISystem->PointInRect(pListRect, fX, fY))
				{
					break;
				}
			}

			int iIndex = FindItemAt(fX, fY, &pItem, 0, &pItor);

			if (iIndex > -1)
			{
				OnCommand();
			}
		}
	}

	return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::Draw(int iPass)
{
	if (iPass != 0)
	{
		return 1;
	}

	UIRect pListRect = GetListRect(1);

	m_pUISystem->BeginDraw(this);

	// get the absolute widget rect
	UIRect pAbsoluteRect(m_pRect);

	m_pUISystem->GetAbsoluteXY(&pAbsoluteRect.fLeft, &pAbsoluteRect.fTop, m_pRect.fLeft, m_pRect.fTop, m_pParent);

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

	// draw the background
	if ((GetStyle() & UISTYLE_TRANSPARENT) == 0)
	{
		// if we are a textured listview
		if (m_pTexture.iTextureID > -1)
		{
			m_pUISystem->DrawImage(pAbsoluteRect, m_pTexture, m_cColor);
		}
		else
		{
			m_pUISystem->DrawQuad(pAbsoluteRect, m_cColor);
		}
	}

	// after drawing the "outside" stuff
	// adjust the rect with the scrollbar sizes
	if (m_bHorizontalScrollBar)
	{
		pAbsoluteRect.fHeight -= m_pUISystem->GetWidgetRect(m_pHScroll).fHeight;
	}
	else if (m_bVerticalScrollBar)
	{
		pAbsoluteRect.fWidth -= m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;
	}

	// update the offsets
	if (m_bHorizontalScrollBar)
	{
		m_fHorizontalOffset = -(m_fTotalWidth - pListRect.fWidth) * m_pHScroll->GetValue();
	}
	else
	{
		m_fHorizontalOffset = 0.0f;
	}
	
	if (m_bVerticalScrollBar)
	{
		float fVisibleHeight = pListRect.fHeight - GetHeaderRect().fHeight - m_fCellSpacing;

		m_fVerticalOffset = -(m_fTotalHeight - fVisibleHeight) * m_pVScroll->GetValue();
	}
	else
	{
		m_fVerticalOffset = 0.0f;
	}
	
	// get the font
	IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

	float fX = pAbsoluteRect.fLeft + m_fHorizontalOffset;
	float fY = pAbsoluteRect.fTop + m_fVerticalOffset + GetHeaderRect().fHeight + m_fCellSpacing;
	float fHeight = (pAbsoluteRect.fHeight) + (pAbsoluteRect.fTop - fY);

	UIListColumn *pColumn;

	// if we have columns, draw them
	if (m_vColumnList.size())
	{
		for (unsigned int i = 0; i < m_vColumnList.size(); i++)
		{
			pColumn = &m_vColumnList[i];

			// cull the columns, only drawing the visible ones
			if (fX + pColumn->fWidth <= pAbsoluteRect.fLeft + pListRect.fLeft)
			{
				fX += pColumn->fWidth;

				continue;
			}
			else if (fX >= pAbsoluteRect.fLeft + pListRect.fLeft + pListRect.fWidth)
			{
				break;
			}

			DrawListColumn(i, UIRect(fX, pAbsoluteRect.fTop, pColumn->fWidth, fHeight), pAbsoluteRect, fX, fY, pFont);

			m_pUISystem->ResetDraw();

			fX += pColumn->fWidth + m_fCellSpacing;
		}
	}
	// if we dont have columns, just draw a single list
	else
	{
		DrawListColumn(0, UIRect(fX, pAbsoluteRect.fTop, pAbsoluteRect.fWidth, fHeight), pAbsoluteRect, fX, fY, pFont);
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
void CUIListView::InitializeTemplate(IScriptSystem *pScriptSystem)
{
	_ScriptableEx<CUIListView>::InitializeTemplate(pScriptSystem);

	REGISTER_COMMON_MEMBERS(pScriptSystem, CUIListView);

	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, SetCellSpacing);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetCellSpacing);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, SetCellPadding);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetCellPadding);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, SetItemHeight);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetItemHeight);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, SetHeaderHeight);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetHeaderHeight);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, FindItemAt);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, FindColumnAt);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, ClearSelection);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, SelectIndex);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, DeselectIndex);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, IsSelectedIndex);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetSelectionCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetSelection);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetSelectedColumn);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, Select);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, Deselect);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, IsSelected);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetItemCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetSubItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, GetSubItemCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, AddItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, AddSubItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, InsertItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, RemoveItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, RemoveSubItem);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, AddColumn);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, RemoveColumn);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, ClearColumns);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, Clear);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, Sort);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, SortEx);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, AddImage);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, AddImageList);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, ClearImageList);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, SetItemImage);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIListView, ClearItemImage);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::FindItemAt(float fX, float fY, UIListItem **pItem, int *iSubItem, CUIListItemItor *pItemItor)
{
	UIRect pListRect = GetListRect();
	UIRect pItemRect(pListRect.fLeft, pListRect.fTop + m_fVerticalOffset + GetHeaderRect().fHeight + m_fCellSpacing, pListRect.fWidth, m_fItemHeight);

	pListRect.fTop = GetHeaderRect().fHeight + m_fCellSpacing;
	pListRect.fHeight -= GetHeaderRect().fHeight + m_fCellSpacing;

	for (CUIListItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
	{
		if (pItemRect.fTop + m_fItemHeight < pListRect.fTop)
		{
			pItemRect.fTop += m_fItemHeight + m_fCellSpacing;

			continue;
		}
		else if (pItemRect.fTop > pListRect.fTop + pListRect.fHeight)
		{
			break;
		}

		if (m_pUISystem->PointInRect(pItemRect, fX, fY))
		{
			if (pItem)
			{
				*pItem = (*pItor);
			}

			if (pItemItor)
			{
				*pItemItor = pItor;
			}

			if ((iSubItem) && (!m_vColumnList.empty()))
			{
				*iSubItem = FindColumnAt(fX, fY);
			}

			return (*pItor)->iIndex;
		}

		pItemRect.fTop += m_fItemHeight + m_fCellSpacing;
	}

	return -1;	
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::FindColumnAt(float fX, float fY, UIListColumn **pColumn)
{
	float fListX = m_fHorizontalOffset;

	if (m_vColumnList.size())
	{
		for (unsigned int i = 0; i < m_vColumnList.size(); i++)
		{
			if ((fX >= fListX) && (fX <= fListX + m_vColumnList[i].fWidth))
			{
				if (pColumn)
				{
					*pColumn = &m_vColumnList[i];
				}

				return i;
			}

			fListX += m_vColumnList[i].fWidth + m_fCellSpacing;
		}
	}

	return -1;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::ScrollTo(int iItem)
{
	UIListItem *pItem = GetItem(iItem);

	if (!iItem)
	{
		return 0;
	}

	for(;;)
	{
		UIRect pListRect = GetListRect();
		UIRect pItemRect(pListRect.fLeft, pListRect.fTop + m_fVerticalOffset + GetHeaderRect().fHeight + m_fCellSpacing, pListRect.fWidth, m_fItemHeight);

		pListRect.fTop = GetHeaderRect().fHeight + m_fCellSpacing;
		pListRect.fHeight -= GetHeaderRect().fHeight + m_fCellSpacing;

		for (CUIListItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
		{
			if ((*pItor)->iIndex == pItem->iIndex)
			{
				if (pItemRect.fTop < pListRect.fTop)
				{
					m_fVerticalOffset += abs(pListRect.fTop - pItemRect.fTop);

					if (m_bVerticalScrollBar)
					{
						m_pVScroll->SetValue(m_fVerticalOffset / -(m_fTotalHeight - pListRect.fHeight));
					}

					return 1;
				}
				else if (pItemRect.fTop + m_fItemHeight > pListRect.fTop + pListRect.fHeight)
				{
					m_fVerticalOffset -= abs(pItemRect.fTop + m_fItemHeight - (pListRect.fTop + pListRect.fHeight));

					if (m_bVerticalScrollBar)
					{
						m_pVScroll->SetValue(m_fVerticalOffset / -(m_fTotalHeight - pListRect.fHeight));
					}

					return 1;
				}
				else
				{
					return 1;
				}
			}

			pItemRect.fTop += m_fItemHeight + m_fCellSpacing;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::ClearSelection()
{
	m_vSelectionList.clear();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::SelectIndex(int iItemIndex)
{
	UIListItem *pItem = GetItem(iItemIndex);

	if (pItem && pItem->bSelectable)
	{
		m_vSelectionList.push_back(iItemIndex);

		return 1;
	}
	    
	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::DeselectIndex(int iItemIndex)
{
	for (std::vector<int>::iterator pItor = m_vSelectionList.begin(); pItor != m_vSelectionList.end(); ++pItor)
	{
		if (*pItor == iItemIndex)
		{
			m_vSelectionList.erase(pItor);

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
bool CUIListView::IsSelectedIndex(int iItemIndex)
{
	return std::find(m_vSelectionList.begin(), m_vSelectionList.end(), iItemIndex) != m_vSelectionList.end();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::Select(const wstring &szItemText)
{
	UIListItem *pItem = GetItem(szItemText);

	if (pItem && pItem->bSelectable)
	{
		return SelectIndex(pItem->iIndex);
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::Deselect(const wstring &szItemText)
{
	UIListItem *pItem = GetItem(szItemText);

	if (pItem)
	{
		return DeselectIndex(pItem->iIndex);
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
bool CUIListView::IsSelected(const wstring &szItemText)
{
	UIListItem *pItem = GetItem(szItemText);

	if (pItem)
	{
		return IsSelectedIndex(pItem->iIndex);
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetSelectionCount()
{
	return m_vSelectionList.size();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetItemCount()
{
	return m_vItemList.size();
}

//------------------------------------------------------------------------------------------------- 
UIListItem *CUIListView::GetItem(int iIndex, CUIListItemItor *pItemItor)
{
	for (CUIListItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
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
UIListItem *CUIListView::GetItem(const wstring &szText, CUIListItemItor *pItemItor)
{
	for (CUIListItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
	{
		if ((*pItor)->vSubItem[0].szText == szText)
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
int CUIListView::AddItem(const wstring &szText)
{
	UIListItem *pItem = new UIListItem;

	UIListSubItem	pSubItem;

	if (!szText.empty())
	{
		if ((szText[0] == '$') && (szText.size() > 1) && (szText[szText.size()-1] == '$'))
		{
			pSubItem.szText = wstring(&szText[1], &szText[szText.size()-1]);

			pItem->bSelectable = false;
		}
		else
		{
			pSubItem.szText = wstring(&szText[1], &szText[szText.size()-1]);

			pItem->bSelectable = true;
		}
	}
	else
	{
		pItem->bSelectable = true;
	}

	pItem->vSubItem.push_back(pSubItem);
	pItem->iIndex = m_iNewIndex++;

	m_vItemList.push_back(pItem);

	m_bNeedSort = 1;

	return pItem->iIndex;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::AddItem(const std::vector<wstring> &vszText)
{
	UIListItem *pItem = new UIListItem;

	UIListSubItem pSubItem;

	for (std::vector<wstring>::const_iterator pItor = vszText.begin(); pItor != vszText.end(); ++pItor)
	{
		if (pItor == vszText.begin())
		{
			wstring szAux(*pItor);

			if (!szAux.empty())
			{
				if ((szAux[0] == '$') && (szAux.size() > 1) && (szAux[szAux.size()-1] == '$'))
				{
					pSubItem.szText = wstring(&szAux[1], &szAux[szAux.size()-1]);

					pItem->bSelectable = false;
				}
				else
				{
					pItem->bSelectable = true;

					pSubItem.szText = *pItor;
				}
			}
			else
			{
				pItem->bSelectable = true;
			}
		}
		else
		{
			pSubItem.szText = *pItor;
		}
		pItem->vSubItem.push_back(pSubItem);
	}

	pItem->iIndex = m_iNewIndex++;

	m_vItemList.push_back(pItem);

	m_bNeedSort = 1;

	return pItem->iIndex;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::InsertItem(int iPosition, const wstring &szText)
{
	if (iPosition < 0 || iPosition > (int)m_vItemList.size())
	{
		return 0;
	}

	UIListItem *pItem = new UIListItem;

	UIListSubItem pSubItem;

	if (!szText.empty())
	{
		if ((szText[0] == '$') && (szText.size() > 1) && (szText[szText.size()-1] == '$'))
		{
			pSubItem.szText = wstring(&szText[1], &szText[szText.size()-1]);

			pItem->bSelectable = false;
		}
		else
		{
			pSubItem.szText = szText;

			pItem->bSelectable = true;
		}
	}
	else
	{
		pItem->bSelectable = true;
	}

	pItem->vSubItem.push_back(pSubItem);
	pItem->iIndex = m_iNewIndex++;

	m_vItemList.insert(m_vItemList.begin() + iPosition, pItem);

	m_bNeedSort = 1;

	return pItem->iIndex;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::InsertItem(int iPosition, const std::vector<wstring> &vszText)
{
	if (iPosition < 0 || iPosition > (int)m_vItemList.size())
	{
		return 0;
	}

	UIListItem *pItem = new UIListItem;

	UIListSubItem pSubItem;

	for (std::vector<wstring>::const_iterator pItor = vszText.begin(); pItor != vszText.end(); ++pItor)
	{
		if (pItor == vszText.begin())
		{
			wstring szAux = *pItor;

			if (!szAux.empty())
			{
				if ((szAux[0] == '$') && (szAux.size() > 1) && (szAux[szAux.size()-1] == '$'))
				{
					pSubItem.szText = wstring(&szAux[1], &szAux[szAux.size()-1]);
					pItem->bSelectable = false;
				}
				else
				{
					pSubItem.szText = *pItor;
					pItem->bSelectable = true;
				}
			}
			else
			{
				pItem->bSelectable = true;
			}
		}
		else
		{
			pSubItem.szText = *pItor;
		}

		pItem->vSubItem.push_back(pSubItem);
	}

	pItem->iIndex = m_iNewIndex++;

	m_vItemList.insert(m_vItemList.begin() + iPosition, pItem);

	m_bNeedSort = 1;

	return pItem->iIndex;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::AddSubItem(int iItemIndex, const wstring &szText)
{
	if (UIListItem *pItem = GetItem(iItemIndex))
	{
		UIListSubItem pSubItem;

		pSubItem.szText = szText;
		pItem->vSubItem.push_back(pSubItem);
	}

	m_bNeedSort = 1;
    
	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::RemoveItem(int iItemIndex)
{
	for (CUIListItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
	{
		if ((*pItor)->iIndex == iItemIndex)
		{
			delete *pItor;

			m_vItemList.erase(pItor);

			m_bNeedSort = 1;

			ClearSelection();

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::RemoveSubItem(int iItemIndex, int iSubItemIndex)
{
	UIListItem *pItem = GetItem(iItemIndex);

	if (!pItem)
	{
		return 0;
	}

	int i = 0;
	for (std::vector<UIListSubItem>::iterator pItor = (pItem->vSubItem.begin() + 1); pItor != pItem->vSubItem.end(); ++pItor, i++)
	{
		if (i == iSubItemIndex)
		{
			pItem->vSubItem.erase(pItor);

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::AddColumn(const wstring &szLabel, float fWidth, int iHTextAlignment, const color4f &cHeaderColor, const color4f &cBodyColor, const UISkinTexture &pHeaderTexture, bool bNumeric, bool bSelectable, bool bImageSort)
{
	UIListColumn pColumn;

	pColumn.szLabel = szLabel;
	pColumn.fWidth = fWidth;
	pColumn.iHTextAlignment = iHTextAlignment;
	pColumn.cHeaderColor = cHeaderColor;
	pColumn.cBodyColor = cBodyColor;
	pColumn.pHeaderTexture = pHeaderTexture;
	pColumn.bNumeric = bNumeric;
	pColumn.bSelectable = bSelectable;
	pColumn.bImageSort = bImageSort;

	m_vColumnList.push_back(pColumn);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::RemoveColumn(int iColumnIndex)
{
	int i = 0;
	for (CUIListColumnItor pItor = m_vColumnList.begin(); pItor != m_vColumnList.end(); ++pItor, i++)
	{
		if (i == iColumnIndex)
		{
			m_vColumnList.erase(pItor);

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::ClearColumns()
{
	m_vColumnList.clear();
	ClearSelection();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::Clear()
{
	m_bNeedSort = 0;
	m_iNewIndex = 1;

	for (CUIListItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
	{
		delete *pItor;
	}

	m_vItemList.clear();

	ClearSelection();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::Sort()
{
	m_pStaticThis = this;
	std::stable_sort(m_vItemList.begin(), m_vItemList.end(), SortCallback);
	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::SortEx(int iSortOrder, int iSortColumn)
{
	int iOldSortOrder = m_iSortOrder;
	int iOldSortColumn = m_iSortColumn;
	m_iSortOrder = iSortOrder;
	m_iSortColumn = iSortColumn;

	m_pStaticThis = this;

	std::sort(m_vItemList.begin(), m_vItemList.end(), SortCallback);

	m_iSortOrder = iOldSortOrder;
	m_iSortColumn = iOldSortColumn;

	m_bNeedSort = 0;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::DrawListColumn(int iIndex, const UIRect &pColumnRect, const UIRect pListRect, float fX, float fY, IFFont *pFont)
{
	UIRect pItemRect(fX, fY, pColumnRect.fWidth, m_fItemHeight);
	UIRect pRect = pListRect;
	color4f cItemColor = m_cColor;

	int iAlignment = UIALIGN_LEFT;

	if (!m_vColumnList.empty())
	{
		UIListColumn *pColumn = &m_vColumnList[iIndex];
		assert(pColumn);

		cItemColor = m_cColor;

		// set the boundaries for the
		UIRect pHeaderRect = GetHeaderRect();

		m_pUISystem->GetAbsoluteXY(&pHeaderRect.fLeft, &pHeaderRect.fTop, pHeaderRect.fLeft, pHeaderRect.fTop, this);
		m_pUISystem->SetScissor(&pHeaderRect);

		pHeaderRect = UIRect(fX, pColumnRect.fTop, pColumnRect.fWidth, GetHeaderRect().fHeight);

		color4f cHeaderColor = pColumn->cHeaderColor;

		if ((iIndex == m_iSortColumn) && !m_iNoSort)
		{
			cHeaderColor = m_cSortByColor;
			pFont->SetColor(m_cSortByTextColor);
		}
		
		// draw the header
		if (pColumn->pHeaderTexture.iTextureID > -1)
		{
			m_pUISystem->DrawImage(pHeaderRect, pColumn->pHeaderTexture, cHeaderColor);
		}
		else
		{
			m_pUISystem->DrawQuad(pHeaderRect, cHeaderColor);
		}

		m_pUISystem->AdjustRect(&pHeaderRect, pHeaderRect, m_fCellPadding + m_fColumnTextSpacing);
		m_pUISystem->DrawText(pHeaderRect, pColumn->iHTextAlignment, UIALIGN_CENTER, pFont, pColumn->szLabel.c_str());

		// restore default font color
		pFont->SetColor(m_pFont.cColor);

		// update the rect so it doesnt contain the header
		pRect.fTop += GetHeaderRect().fHeight + m_fCellSpacing;
		pRect.fHeight -= GetHeaderRect().fHeight + m_fCellSpacing;

		iAlignment = pColumn->iHTextAlignment;

		UIRect pBodyRect(pColumnRect.fLeft, pRect.fTop, pColumnRect.fWidth, pRect.fHeight);

		m_pUISystem->SetScissor(&pBodyRect);
		m_pUISystem->DrawQuad(pBodyRect, pColumn->cBodyColor);
	}

	UIListItem	*pItem;

	for (CUIListItemItor pItor = m_vItemList.begin(); pItor != m_vItemList.end(); ++pItor)
	{
		if (pItemRect.fTop + m_fItemHeight < pRect.fTop)
		{
			pItemRect.fTop += m_fItemHeight + m_fCellSpacing;

			continue;
		}
		else if (pItemRect.fTop > pRect.fTop + pRect.fHeight)
		{
			break;
		}

		pItem = (*pItor);

		UIRect pClippedRect;

		m_pUISystem->IntersectRect(&pClippedRect, pItemRect, pRect);

		m_pUISystem->SetScissor(&pClippedRect);

		if (IsSelectedIndex(pItem->iIndex))
		{
			if (!m_bColumnSelect)
			{
				m_pUISystem->DrawQuad(pClippedRect, m_cSelectionColor);
			}
			else if (iIndex == m_iSelectedColumn)
			{
				m_pUISystem->DrawQuad(pClippedRect, m_cSelectionColor);
			}
		}

		if (pItem->vSubItem.size() > (unsigned int)iIndex)
		{
			UIRect pCellRect(pItemRect);

			int iImageIndex = pItem->vSubItem[iIndex].iImageIndex;

			if (iImageIndex >= 1 && iImageIndex <= m_vImageList.size())
			{
				UIRect			pImgRect;
				UIListImage	*pImage = &m_vImageList[iImageIndex-1];

				switch (iAlignment)
				{
				case UIALIGN_LEFT:
					pImgRect.fLeft = pCellRect.fLeft;
					break;
				case UIALIGN_RIGHT:
					pImgRect.fLeft = pCellRect.fLeft + pCellRect.fWidth - pImage->fWidth;
					break;
				default:
					pImgRect.fLeft = pCellRect.fLeft + (pCellRect.fWidth - pImage->fWidth) * 0.5f;
					break;
				}

				pImgRect.fTop = pCellRect.fTop + (pCellRect.fHeight - pImage->fHeight) * 0.5f;
				pImgRect.fWidth = pImage->fWidth;
				pImgRect.fHeight = pImage->fHeight;

				m_pUISystem->DrawImage(pImgRect, pImage->iTextureID, pImage->vTexCoord, color4f(1.0f, 1.0f, 1.0f, 1.0f));
			}

			m_pUISystem->AdjustRect(&pCellRect, pCellRect, m_fCellPadding);
			m_pUISystem->DrawText(pCellRect, iAlignment, UIALIGN_MIDDLE, pFont, pItem->vSubItem[iIndex].szText.c_str());
		}

		pItemRect.fTop += m_fItemHeight + m_fCellSpacing;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
UIRect CUIListView::GetHeaderRect()
{
	UIRect pListRect(GetListRect());

	if (m_vColumnList.empty())
	{
		pListRect.fHeight = 0;
	}
	else
	{
		pListRect.fHeight = m_fHeaderHeight;
	}

	return pListRect;
}

//------------------------------------------------------------------------------------------------- 
UIRect CUIListView::GetListRect(bool bScrollBars)
{
	UIRect pListRect(0, 0, m_pRect.fWidth, m_pRect.fHeight);

	// if border is large enough to be visible, remove it from the rect
	if (m_pBorder.fSize > 0.125f)
	{
		m_pUISystem->AdjustRect(&pListRect, pListRect, m_pBorder.fSize);
	}

	if (bScrollBars)
	{
		// we have the toolbars, remove them from the rect
		if (m_bHorizontalScrollBar)
		{
			pListRect.fHeight -= m_pUISystem->GetWidgetRect(m_pHScroll).fHeight;
		}
		if (m_bVerticalScrollBar)
		{
			pListRect.fWidth -= m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;
		}
	}

	return pListRect;
}

//------------------------------------------------------------------------------------------------- 
UIRect CUIListView::GetColumnRect(int iColumnIndex)
{
	UIRect pColumnRect(GetListRect());

	if ((iColumnIndex < 0) || (m_vColumnList.empty()))
	{
		return pColumnRect;
	}
	else
	{
		float fX = pColumnRect.fLeft + m_fHorizontalOffset;

		UIListColumn *pColumn = &m_vColumnList[0];

		for (int i = 0; i < iColumnIndex; i++)
		{
			pColumn = &m_vColumnList[i];

			fX += pColumn->fWidth + m_fCellSpacing;
		}

		pColumnRect.fLeft = fX;
		pColumnRect.fWidth = pColumn->fWidth;

		return pColumnRect;
	}
}

//------------------------------------------------------------------------------------------------- 
bool CUIListView::SortCallback(const UIListItem *pOne, const UIListItem *pTwo)
{
	wstring szOne;
	wstring szTwo;

	if ((m_pStaticThis->m_iSortColumn < (int)pOne->vSubItem.size()) && pOne->vSubItem[m_pStaticThis->m_iSortColumn].szText.size())
	{
		m_pStaticThis->m_pUISystem->StripControlCodes(szOne, pOne->vSubItem[m_pStaticThis->m_iSortColumn].szText);
	}

	if ((m_pStaticThis->m_iSortColumn < (int)pTwo->vSubItem.size()) && pTwo->vSubItem[m_pStaticThis->m_iSortColumn].szText.size())
	{
		m_pStaticThis->m_pUISystem->StripControlCodes(szTwo, pTwo->vSubItem[m_pStaticThis->m_iSortColumn].szText);
	}

	if (m_pStaticThis->m_vColumnList[m_pStaticThis->m_iSortColumn].bNumeric)
	{
		float fOne = _wtof(szOne.c_str());
		float fTwo = _wtof(szTwo.c_str());

		if (m_pStaticThis->m_iSortOrder == UISORT_ASCENDING)
		{
			return fOne < fTwo;
		}
		else
		{
			return fOne > fTwo;
		}
	}
	else if (m_pStaticThis->m_vColumnList[m_pStaticThis->m_iSortColumn].bImageSort)
	{
		if (m_pStaticThis->m_iSortOrder == UISORT_ASCENDING)
		{
			return pOne->vSubItem[m_pStaticThis->m_iSortColumn].iImageIndex < pTwo->vSubItem[m_pStaticThis->m_iSortColumn].iImageIndex;
		}
		else
		{
			return pOne->vSubItem[m_pStaticThis->m_iSortColumn].iImageIndex > pTwo->vSubItem[m_pStaticThis->m_iSortColumn].iImageIndex;
		}
	}
	else
	{
		if (m_pStaticThis->m_iSortOrder == UISORT_ASCENDING)
		{
			return szOne < szTwo;
		}
		else
		{
			return szOne > szTwo;
		}
	}

	return true;
}


//------------------------------------------------------------------------------------------------- 
// Script Functions
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
int CUIListView::SetCellPadding(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetCellPadding, m_fCellPadding);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetCellPadding(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetCellPadding, m_fCellPadding);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::SetCellSpacing(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetCellSpacing, m_fCellSpacing);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetCellSpacing(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetCellSpacing, m_fCellSpacing);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::SetItemHeight(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetItemHeight, m_fItemHeight);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetItemHeight(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetItemHeight, m_fItemHeight);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::SetHeaderHeight(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetHeaderHeight, m_fHeaderHeight);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetHeaderHeight(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), SetHeaderHeight, m_fHeaderHeight);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::FindItemAt(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), FindItemAt, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), FindItemAt, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), FindItemAt, 2, svtNumber);

	float fX, fY;

	pH->GetParam(1, fX);
	pH->GetParam(2, fY);

	return pH->EndFunction(FindItemAt(fX, fY));
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::FindColumnAt(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), FindColumnAt, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), FindColumnAt, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), FindColumnAt, 2, svtNumber);

	float fX, fY;

	pH->GetParam(1, fX);
	pH->GetParam(2, fY);

	return pH->EndFunction(FindColumnAt(fX, fY));
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::ClearSelection(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), ClearSelection, 0);

	ClearSelection();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::SelectIndex(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SelectIndex, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SelectIndex, 1, svtNumber);

	int iItemIndex;

	pH->GetParam(1, iItemIndex);

	SelectIndex(iItemIndex);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::DeselectIndex(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), DeselectIndex, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), DeselectIndex, 1, svtNumber);

	int iItemIndex;

	pH->GetParam(1, iItemIndex);

	DeselectIndex(iItemIndex);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::IsSelectedIndex(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), IsSelectedIndex, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), IsSelectedIndex, 1, svtNumber);

	int iItemIndex;

	pH->GetParam(1, iItemIndex);

	return pH->EndFunction(IsSelectedIndex(iItemIndex));
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::Select(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Select, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), Select, 1, svtString);

	wstring szItemText;

	m_pUISystem->ConvertToWString(szItemText, pH, 1);

	Select(szItemText);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::Deselect(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Deselect, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), Deselect, 1, svtString);

	wstring szItemText;

	m_pUISystem->ConvertToWString(szItemText, pH, 1);

	Deselect(szItemText);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::IsSelected(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), IsSelected, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), IsSelected, 1, svtString);

	wstring szItemText;

	m_pUISystem->ConvertToWString(szItemText, pH, 1);

	return pH->EndFunction(IsSelected(szItemText));
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetSelectionCount(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetSelectionCount, 0);

	return pH->EndFunction(GetSelectionCount());
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetSelection(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetSelection, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), GetSelection, 1, svtNumber);

	int	iSelectedIndex = 0;

	pH->GetParam(1, iSelectedIndex);

	if ((iSelectedIndex >= 0) && (iSelectedIndex < (int)m_vSelectionList.size()))
	{
		return pH->EndFunction(m_vSelectionList[iSelectedIndex]);
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetSelectedColumn(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetSelectedColumn, 0);

	return pH->EndFunction((int)m_iSelectedColumn);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetItem(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetItem, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), GetItem, 1, svtNumber);

	int iItemIndex;

	pH->GetParam(1, iItemIndex);

	UIListItem *pItem = GetItem(iItemIndex);

	char szString[1024];

	m_pUISystem->ConvertToString(szString, pItem->vSubItem[0].szText, 1023);

	return pH->EndFunction(szString);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetItemCount(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetItemCount, 0);

	return pH->EndFunction(GetItemCount());
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetSubItem(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetSubItem, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), GetSubItem, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), GetSubItem, 1, svtNumber);

	int iItemIndex, iSubItemIndex;

	pH->GetParam(1, iItemIndex);
	pH->GetParam(2, iSubItemIndex);

	UIListItem *pItem = GetItem(iItemIndex);

	if (iSubItemIndex >= (int)pItem->vSubItem.size())
	{
		return pH->EndFunctionNull();
	}

	char szString[1024];
	
	m_pUISystem->ConvertToString(szString, pItem->vSubItem[iSubItemIndex].szText, 1023);

	return pH->EndFunction(szString);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::GetSubItemCount(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetSubItemCount, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), GetSubItemCount, 1, svtNumber);

	int iItemIndex;


	pH->GetParam(1, iItemIndex);

	UIListItem *pItem = GetItem(iItemIndex);

	return pH->EndFunction(int(pItem->vSubItem.size() - 1));
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::AddItem(IFunctionHandler *pH)
{
	if (pH->GetParamCount() < 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), AddItem, 1);
	}

	std::vector<wstring> vszText;

	int iParamCount = pH->GetParamCount();

	wstring szDummy = L"";

	for (int i = 1; i <= iParamCount; i++)
	{
		if ((pH->GetParamType(i) != svtString) && (pH->GetParamType(i) != svtNumber) && (pH->GetParamType(i) != svtNull))
		{
			vszText.push_back(szDummy);

			continue;
		}

		vszText.push_back(szDummy);

		if (pH->GetParamType(i) != svtNull)
		{
			m_pUISystem->ConvertToWString(vszText[i-1], pH, i);
		}
	}

	return pH->EndFunction(AddItem(vszText));
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::AddSubItem(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), AddSubItem, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddSubItem, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), AddSubItem, 2, svtNumber, svtString);
	
	wstring	szText;
	int				iItemIndex;

	pH->GetParam(1, iItemIndex);
	m_pUISystem->ConvertToWString(szText, pH, 2);

	AddSubItem(iItemIndex, szText);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::InsertItem(IFunctionHandler *pH)
{
	if (pH->GetParamCount() < 1)
	{
		CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), AddItem, 2);
	}

	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddItem, 1, svtNumber);

	int iPosition;

	pH->GetParam(1, iPosition);

	std::vector<wstring> vszText;

	int iParamCount = pH->GetParamCount();

	wstring szDummy;

	for (int i = 2; i <= iParamCount; i++)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), AddItem, i, svtString, svtNumber);

		vszText.push_back(szDummy);

		m_pUISystem->ConvertToWString(vszText[i-2], pH, i);
	}

	return pH->EndFunction(InsertItem(iPosition, vszText));
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::RemoveItem(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), RemoveItem, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), RemoveItem, 1, svtNumber);

	int iItemIndex;

	pH->GetParam(1, iItemIndex);

	RemoveItem(iItemIndex);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::RemoveSubItem(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), RemoveSubItem, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), RemoveSubItem, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), RemoveSubItem, 2, svtNumber);
	
	int				iItemIndex;
	int				iSubItemIndex;

	pH->GetParam(1, iItemIndex);
	pH->GetParam(1, iSubItemIndex);

	RemoveSubItem(iItemIndex, iSubItemIndex);
	
	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::AddColumn(IFunctionHandler *pH)
{
	if (pH->GetParamCount() < 2)
	{
		CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), AddColumn, 2);
	}
	else if (pH->GetParamCount() > 10)
	{
		CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), AddColumn, 10);
	}

	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), AddColumn, 1, svtString, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddColumn, 2, svtNumber);

	wstring				szText;
	float					fWidth;
	int						iAlign = UIALIGN_LEFT;
	color4f				cHeaderColor = m_cColor;
	color4f				cBodyColor = m_cColor;
	UISkinTexture	pTexture;
	bool					bNumeric = false;
	bool					bSelectable = true;
	bool					bImageSort = false;
	char					*szValue;
	
	if (pH->GetParamCount() > 9)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddColumn, 10, svtNumber);

		pH->GetParam(10, bImageSort);
	}

	if (pH->GetParamCount() > 8)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddColumn, 9, svtNumber);

		pH->GetParam(9, bSelectable);
	}

	if (pH->GetParamCount() > 7)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddColumn, 8, svtNumber);

		pH->GetParam(8, bNumeric);
	}

	// get texture first
	if (pH->GetParamCount() > 5)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), AddColumn, 6, svtUserData, svtNull);

		if (pH->GetParamType(6) != svtNull)
		{
			int iCookie = 0;

			pH->GetParamUDVal(6, pTexture.iTextureID, iCookie);
		}
	}

	// get texture rect
	if ((pH->GetParamCount() > 6) && (pH->GetParamType(7) != svtNull) && (pH->GetParamType(6) != svtNull))
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), AddColumn, 7, svtString, svtNull);

		pH->GetParam(7, szValue);

		m_pUISystem->RetrieveTexRect(pTexture.vTexCoord, pTexture.iTextureID, szValue);
	}

	if (pH->GetParamCount() > 4)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddColumn, 5, svtString);

		char *szColor;

		pH->GetParam(5, szColor);

		m_pUISystem->RetrieveColor(&cBodyColor, szColor);
	}

	if (pH->GetParamCount() > 3)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddColumn, 4, svtString);

		char *szColor;

		pH->GetParam(4, szColor);

		m_pUISystem->RetrieveColor(&cHeaderColor, szColor);
	}

	if (pH->GetParamCount() > 2)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddColumn, 3, svtNumber);

		pH->GetParam(3, iAlign);
	}

	pH->GetParam(2, fWidth);
	m_pUISystem->ConvertToWString(szText, pH, 1);
	
	AddColumn(szText, fWidth, iAlign, cHeaderColor, cBodyColor, pTexture, bNumeric, bSelectable, bImageSort);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::RemoveColumn(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), RemoveColumn, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), RemoveColumn, 1, svtNumber);

	int iColumnIndex;

	pH->GetParam(1, iColumnIndex);

	RemoveColumn(iColumnIndex);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::ClearColumns(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), ClearColumns, 0);
	
	ClearColumns();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::Clear(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Clear, 0);

	Clear();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::Sort(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Sort, 0);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::SortEx(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SortEx, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SortEx, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SortEx, 2, svtNumber);

	int iSortOrder = UISORT_ASCENDING, iColumnIndex = 0;

	pH->GetParam(1, iSortOrder);
	pH->GetParam(2, iColumnIndex);

	SortEx(iSortOrder, iColumnIndex);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::AddImage(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, GetName().c_str(), AddImage, 3, 4);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), AddImage, 1, svtUserData, svtNull);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddImage, 2, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddImage, 3, svtNumber);

	char				*szTexRect = 0;
	UIListImage	pListImage;

	if (pH->GetParamCount() == 4)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddImage, 4, svtString);

		pH->GetParam(4, szTexRect);
	}

	pH->GetParam(2, pListImage.fWidth);
	pH->GetParam(3, pListImage.fHeight);

	int iCookie = 0;

	pH->GetParamUDVal(1, pListImage.iTextureID, iCookie);

	m_pUISystem->RetrieveTexRect(pListImage.vTexCoord, pListImage.iTextureID, szTexRect);

	m_vImageList.push_back(pListImage);

	return pH->EndFunction(1);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::AddImageList(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), AddImageList, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddImageList, 1, svtObject);

	_SmartScriptObject pImageList(m_pScriptSystem, 1);

	pH->GetParam(1, pImageList);

	for (int i = 1; i <= pImageList->Count(); i++)
	{
		if (pImageList->GetAtType(i) != svtObject)
		{
			m_pUISystem->GetISystem()->GetILog()->Log("$4Warning:$1 Invalid image specification (%d) adding to listview(%s).", i, GetName().c_str());

			continue;
		}

		_SmartScriptObject pImage(m_pScriptSystem, 1);

		pImageList->GetAt(i, pImage);

		UIListImage pListImage;

		int iCookie = 0;

		if (!pImage->GetAtUD(1, (USER_DATA&)pListImage.iTextureID, iCookie) ||
				!pImage->GetAt(2, pListImage.fWidth) ||
				!pImage->GetAt(3, pListImage.fHeight))
		{
			m_pUISystem->GetISystem()->GetILog()->Log("$4Warning:$1 Invalid image specification (%d) adding to listview(%s).", i, GetName().c_str());

			continue;
		}

		char *szTexRect = 0;

		pImage->GetAt(4, szTexRect);

		m_pUISystem->RetrieveTexRect(pListImage.vTexCoord, pListImage.iTextureID, szTexRect);

		m_vImageList.push_back(pListImage);
	}

	return pH->EndFunction(1);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::ClearImageList(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), ClearImageList, 0);

	m_vImageList.clear();

	return pH->EndFunction(1);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::SetItemImage(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, GetName().c_str(), SetItemImage, 2, 3);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetItemImage, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetItemImage, 2, svtNumber);

	int iImageIndex = -1;
	int iItem = 0;
	int iSubItem = 0;

	if (pH->GetParamCount() == 3)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetItemImage, 3, svtNumber);

		pH->GetParam(3, iSubItem);
	}

	pH->GetParam(1, iImageIndex);
	pH->GetParam(2, iItem);

	UIListItem *pItem = GetItem(iItem);

	if (pItem)
	{
		pItem->vSubItem[iSubItem].iImageIndex = iImageIndex;
	}

	return pH->EndFunction(1);
}

//------------------------------------------------------------------------------------------------- 
int CUIListView::ClearItemImage(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, GetName().c_str(), SetItemImage, 1, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetItemImage, 1, svtNumber);

	int iItem = 0;
	int iSubItem = 0;

	if (pH->GetParamCount() == 2)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetItemImage, 2, svtNumber);

		pH->GetParam(2, iSubItem);
	}

	pH->GetParam(1, iItem);

	UIListItem *pItem = GetItem(iItem);

	if (pItem)
	{
		pItem->vSubItem[iSubItem].iImageIndex = -1;
	}

	return pH->EndFunction(1);
}

//------------------------------------------------------------------------------------------------- 
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
#pragma once


#include "UIWidget.h"
#include "UISystem.h"



#define UICLASSNAME_COMBOBOX			"UIComboBox"



#define UICOMBOBOXSTATE_BUTTONDOWN			(1 << 0)
#define UICOMBOBOXSTATE_BUTTONOVER			(1 << 1)
#define UICOMBOBOXSTATE_DROPDOWNLIST		(1 << 2)
#define UICOMBOBOXSTATE_JUSTDROPEDLIST	(1 << 3)



typedef struct UIComboItem
{
	wstring						szText;
	int								iIndex;
	UISkinTexture			pTexture;
	color4f						cColor;

} UIComboItem;

typedef std::vector<UIComboItem *>						CUIComboItemList;
typedef std::vector<UIComboItem *>::iterator	CUIComboItemItor;


class CUIComboBox : public CUIWidget,
	public _ScriptableEx<CUIComboBox>
{
	UI_WIDGET(CUIComboBox)

public:

	CUIComboBox();
	~CUIComboBox();

	string GetClassName();

	LRESULT Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam);	//AMD Port
	int Draw(int iPass);

	static void InitializeTemplate(IScriptSystem *pScriptSystem);

	//------------------------------------------------------------------------------------------------- 
	int ClearSelection();
	int SelectIndex(int iIndex);
	int DeselectIndex(int iIndex);
	bool IsSelectedIndex(int iIndex);
	int Select(const wstring &szItemText);
	int Deselect(const wstring &szItemText);
	bool IsSelected(const wstring &szItemText);

	//------------------------------------------------------------------------------------------------- 
	int GetItemCount();
	UIComboItem *GetItem(int iIndex, CUIComboItemItor *pItemItor = 0);
	UIComboItem *GetItem(const wstring &szwItemText, CUIComboItemItor *pItemItor = 0);

	//------------------------------------------------------------------------------------------------- 
	int AddItem(const wstring &szText, UISkinTexture pTexture = UISkinTexture(), const color4f &cColor = color4f(0,0,0,0));
	int InsertItem(int iPosition, const wstring &szText, const UISkinTexture &pTexture = UISkinTexture(), const color4f &cColor = color4f(0,0,0,0));
	int RemoveItem(int iItemIndex);

	int Clear();
	int Sort(int iSortOrder = UISORT_ASCENDING);

	//------------------------------------------------------------------------------------------------- 
	// Script Functions
	//------------------------------------------------------------------------------------------------- 
	int SetButtonSize(IFunctionHandler *pH);
	int GetButtonSize(IFunctionHandler *pH);

	int SetItemHeight(IFunctionHandler *pH);
	int GetItemHeight(IFunctionHandler *pH);

	int ClearSelection(IFunctionHandler *pH);

	int IsDropDown(IFunctionHandler *pH);
	
	int SelectIndex(IFunctionHandler *pH);
	int DeselectIndex(IFunctionHandler *pH);
	int IsSelectedIndex(IFunctionHandler *pH);

	int Select(IFunctionHandler *pH);
	int Deselect(IFunctionHandler *pH);
	int IsSelected(IFunctionHandler *pH);

	int GetSelection(IFunctionHandler *pH);
	int GetSelectionIndex(IFunctionHandler *pH);
	
	int GetItem(IFunctionHandler *pH);
	int GetItemCount(IFunctionHandler *pH);

	int AddItem(IFunctionHandler *pH);
	int InsertItem(IFunctionHandler *pH);
	int RemoveItem(IFunctionHandler *pH);

	int SetVAlign(IFunctionHandler *pH);
	int GetVAlign(IFunctionHandler *pH);

	int SetHAlign(IFunctionHandler *pH);
	int GetHAlign(IFunctionHandler *pH);

	int Clear(IFunctionHandler *pH);
	int Sort(IFunctionHandler *pH);

	int SetTexture(IFunctionHandler *pH);
	int GetTexture(IFunctionHandler *pH);

	int SetButtonTexture(IFunctionHandler *pH);
	int GetButtonTexture(IFunctionHandler *pH);
	int SetButtonOverTexture(IFunctionHandler *pH);
	int GetButtonOverTexture(IFunctionHandler *pH);
	int SetButtonDownTexture(IFunctionHandler *pH);
	int GetButtonDownTexture(IFunctionHandler *pH);

	int SetButtonColor(IFunctionHandler *pH);
	int GetButtonColor(IFunctionHandler *pH);

private:

	UIRect GetComboRect();
	UIRect GetListRect(bool bScrollBars = 1);
	void DropList();
	void UndropList();

	static bool SortCallback(UIComboItem *pOne, UIComboItem *pTwo)
	{
		if (m_pStaticThis->m_iSortOrder == UISORT_DESCENDING)
		{
			return pOne->szText <= pTwo->szText;
		}
		else
		{
			return pOne->szText >= pTwo->szText;
		}
	}
	static CUIComboBox	*m_pStaticThis;

	UIRect					m_pComboRect;

	int							m_iSelectionIndex;

	int							m_iFirstItem;
	int							m_iMaxItems;

	int							m_iRollUp;
	
	CUIScrollBar		*m_pVScroll;
	bool						m_bVerticalScrollBar;

	float						m_fItemHeight;
	float						m_fLeftSpacing;

	int							m_iHAlignment;
	int							m_iVAlignment;

	float						m_fButtonSize;

	color4f					m_cButtonColor;
	UISkinTexture		m_pButtonTexture;

	UISkinTexture		m_pTexture;
	UISkinTexture		m_pItemBg;
	color4f					m_cItemBgColor;

	int							m_iOldZ;

	int							m_iState;

	int							m_iNewIndex;
	CUIComboItemList	m_vItemList;

	bool						m_bNeedSort;
	int							m_iSortOrder;
};

//------------------------------------------------------------------------------------------------- 
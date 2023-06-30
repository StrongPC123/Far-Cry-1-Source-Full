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
#pragma once


#include "UIWidget.h"
#include "UISystem.h"



#define UICLASSNAME_LISTVIEW			"UIListView"


typedef struct UIListImage
{
	INT_PTR	iTextureID;
	float		fWidth;
	float		fHeight;
	float		vTexCoord[4];
} UIListImage;


typedef struct UIListSubItem
{
	UIListSubItem(): iImageIndex(-1) {};
	wstring		szText;
	int				iImageIndex;

} UIListSubItem;

typedef struct UIListItem
{
	std::vector<UIListSubItem>	vSubItem;
	int													iIndex;
	bool												bSelectable;

} UIListItem;


typedef struct UIListColumn
{
	wstring				szLabel;
	float					fWidth;
	int						iHTextAlignment;
	color4f	cHeaderColor;
	UISkinTexture	pHeaderTexture;
	color4f	cBodyColor;
	bool					bNumeric;
	bool					bImageSort;
	bool					bSelectable;
		
} UIListColumn;



typedef std::vector<UIListItem *>						CUIListItemList;
typedef std::vector<UIListItem *>::iterator	CUIListItemItor;

typedef std::vector<UIListColumn>						CUIListColumnList;
typedef std::vector<UIListColumn>::iterator	CUIListColumnItor;


class CUIListView : public CUIWidget,
	public _ScriptableEx<CUIListView>
{
	UI_WIDGET(CUIListView)
public:

	CUIListView();
	~CUIListView();

	string GetClassName();

	LRESULT Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam);	//AMD Port
	int Draw(int iPass);

	static void InitializeTemplate(IScriptSystem *pScriptSystem);

	//------------------------------------------------------------------------------------------------- 
	int FindItemAt(float fX, float fY, UIListItem **pItem = 0, int *iSubItem = 0, CUIListItemItor *pItemItor = 0);
	int FindColumnAt(float fX, float fY, UIListColumn **pColumn = 0);

	//------------------------------------------------------------------------------------------------- 
	int ScrollTo(int iItem);

	//------------------------------------------------------------------------------------------------- 
	int ClearSelection();
	int SelectIndex(int iItemIndex);
	int DeselectIndex(int iItemIndex);
	bool IsSelectedIndex(int iItemIndex);

	int Select(const wstring &szItemText);
	int Deselect(const wstring &szItemText);
	bool IsSelected(const wstring &szItemText);

	UIListItem *GetSelection(int iIndex);
	int GetSelectionCount();

	//------------------------------------------------------------------------------------------------- 
	int GetItemCount();
	UIListItem *GetItem(int iIndex, CUIListItemItor *pItemItor = 0);
	UIListItem *GetItem(const wstring &szText, CUIListItemItor *pItemItor = 0);

	//------------------------------------------------------------------------------------------------- 
	int AddItem(const wstring &szText);
	int AddItem(const std::vector<wstring> &vszText);
	int InsertItem(int iPosition, const wstring &szText);
	int InsertItem(int iPosition, const std::vector<wstring> &vszText);
	int AddSubItem(int iItemIndex, const wstring &szText);
	int RemoveItem(int iItemIndex);
	int RemoveSubItem(int iItemIndex, int iSubItemIndex);

	int AddColumn(const wstring &szLabel, float fWidth, int iHTextAlignment = UIALIGN_LEFT, const color4f &cHeaderColor = color4f(0.75f, 0.75f, 0.75f, 1.0f), const color4f &cBodyColor = color4f(0.75f, 0.75f, 0.75f, 1.0f), const UISkinTexture &pHeaderTexture = UISkinTexture(), bool bNumeric = 0, bool bSelectable = 1, bool bImageSort = 0);
	int RemoveColumn(int iColumnIndex);
	int ClearColumns();

	int Clear();
	int Sort();
	int SortEx(int iSortOrder, int iSortColumn = -1);

	//------------------------------------------------------------------------------------------------- 
	// Script Functions
	//------------------------------------------------------------------------------------------------- 
	int SetCellPadding(IFunctionHandler *pH);
	int GetCellPadding(IFunctionHandler *pH);

	int SetCellSpacing(IFunctionHandler *pH);
	int GetCellSpacing(IFunctionHandler *pH);

	int SetItemHeight(IFunctionHandler *pH);
	int GetItemHeight(IFunctionHandler *pH);

	int SetHeaderHeight(IFunctionHandler *pH);
	int GetHeaderHeight(IFunctionHandler *pH);

	int FindItemAt(IFunctionHandler *pH);
	int FindColumnAt(IFunctionHandler *pH);

	int ClearSelection(IFunctionHandler *pH);
	int SelectIndex(IFunctionHandler *pH);
	int DeselectIndex(IFunctionHandler *pH);
	int IsSelectedIndex(IFunctionHandler *pH);

	int Select(IFunctionHandler *pH);
	int Deselect(IFunctionHandler *pH);
	int IsSelected(IFunctionHandler *pH);
	

	int GetSelectionCount(IFunctionHandler *pH);
	int GetSelection(IFunctionHandler *pH);
	int GetSelectedColumn(IFunctionHandler *pH);

	int GetItem(IFunctionHandler *pH);
	int GetItemCount(IFunctionHandler *pH);
	int GetSubItem(IFunctionHandler *pH);
	int GetSubItemCount(IFunctionHandler *pH);

	int AddItem(IFunctionHandler *pH);
	int InsertItem(IFunctionHandler *pH);
	int AddSubItem(IFunctionHandler *pH);
	int RemoveItem(IFunctionHandler *pH);
	int RemoveSubItem(IFunctionHandler *pH);

	int AddColumn(IFunctionHandler *pH);
	int RemoveColumn(IFunctionHandler *pH);
	int ClearColumns(IFunctionHandler *pH);

	int Clear(IFunctionHandler *pH);
	int Sort(IFunctionHandler *pH);
	int SortEx(IFunctionHandler *pH);

	int AddImage(IFunctionHandler *pH);
	int AddImageList(IFunctionHandler *pH);
	int ClearImageList(IFunctionHandler *pH);
	int SetItemImage(IFunctionHandler *pH);
	int ClearItemImage(IFunctionHandler *pH);

private:

	int DrawListColumn(int iIndex, const UIRect &pColumnRect, const UIRect pListRect, float fX, float fY, IFFont *pFont);
	
	UIRect GetHeaderRect();
	UIRect GetListRect(bool bScrollBars = 1);
	UIRect GetColumnRect(int iColumnIndex);

	static bool SortCallback(const UIListItem *pOne, const UIListItem *pTwo);
	static CUIListView	*m_pStaticThis;
		
	float							m_fItemHeight;
	float							m_fHeaderHeight;
	float							m_fCellSpacing;
	float							m_fCellPadding;	
	float							m_fColumnTextSpacing;

	float							m_fTotalWidth;
	float							m_fTotalHeight;

	float							m_fVerticalOffset;
	float							m_fHorizontalOffset;

	UISkinTexture			m_pTexture;

	int								m_iNewIndex;
	CUIListItemList		m_vItemList;
	CUIListColumnList	m_vColumnList;

	color4f			m_cSelectionColor;

	bool							m_bVerticalScrollBar;
	bool							m_bHorizontalScrollBar;

	bool							m_bColumnSelect;
	bool							m_bNeedSort;
	int								m_iSelectedColumn;
	int								m_iSortColumn;
	int								m_iSortOrder;
	int								m_iNoSort;
	

	std::vector<int>	m_vSelectionList;
	CUIListItemItor		m_pSelectionStart;
	color4f			m_cSortByColor;
	color4f			m_cSortByTextColor;

	CUIScrollBar			*m_pHScroll;
	CUIScrollBar			*m_pVScroll;

	std::vector<UIListImage> m_vImageList;
};

//------------------------------------------------------------------------------------------------- 
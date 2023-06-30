#pragma once


#define UICLASSNAME_MENU			"UIMenu"



#include "UIWidget.h"


class CUISystem;
class CUIMenuItem;
/*
{
public:
	CUIMenuItem();
	~CUIMenuItem();

	std::string GetClassName();
	
	int Release();

private:
};
*/

typedef std::vector<CUIMenuItem *> CUIMenuItemList;
typedef std::vector<CUIMenuItem *>::iterator CUIMenuItemItor;


class CUIMenu : public CUIWidget
{
public:
	CUIMenu();
	~CUIMenu();

	string GetClassName();

	int Update(unsigned int iMessage, unsigned int wParam, unsigned int lParam);
	int Draw();

	int Release();

	int AddItem(CUIMenuItem *pMenuItem);
	int DelItem(CUIMenuItem *pMenuItem);
	int DelItem(const std::string &szName);
	CUIMenuItem *FindItem(const std::string &szName);


private:

	CUIMenuItemList		m_pItemList;
	int					m_nItemCount;
};
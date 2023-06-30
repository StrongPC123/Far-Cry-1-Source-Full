//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - Base Widget
//
// History:
//  - [03/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#ifndef UIWIDGET_H
#define UIWIDGET_H

#define REGISTER_COMMON_MEMBERS(scriptsystem, classname) \
	REGISTER_SCRIPTOBJECT_MEMBER(scriptsystem, classname, GetName);\
	REGISTER_SCRIPTOBJECT_MEMBER(scriptsystem, classname, GetScreen);\
	REGISTER_SCRIPTOBJECT_MEMBER(scriptsystem, classname, GetChildCount);\
	REGISTER_SCRIPTOBJECT_MEMBER(scriptsystem, classname, GetChild);\
	REGISTER_SCRIPTOBJECT_MEMBER(scriptsystem, classname, Release);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetRect);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetRect);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetBorderColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetBorderColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetBorderSize);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetBorderSize);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetBorderStyle);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetBorderStyle);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetFontName);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetFontName);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetFontEffect);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetFontEffect);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetFontColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetFontColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetFontSize);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetFontSize);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetMouseCursor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetMouseCursor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetToolTip);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetGreyedColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetGreyedColor);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, SetGreyedBlend);\
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, classname, GetGreyedBlend);\


#define UI_WIDGET(classname)\
private:\
	friend class CUISystem;\
public:\
	virtual IScriptObject* GetScriptObject() {	return _ScriptableEx<classname>::GetScriptObject();	}

	
//------------------------------------------------------------------------------------------------- 
// Data Types
//------------------------------------------------------------------------------------------------- 

//! Rectangle Structure
typedef struct UIRect
{ 
	UIRect() { memset(this, 0, sizeof(UIRect)); };
	UIRect(float l, float t, float w, float h)
	{
		fLeft = l;
		fTop = t;
		fWidth = w;
		fHeight = h;
	};

	UIRect(const UIRect &pRect)
	{
		fLeft = pRect.fLeft;
		fTop = pRect.fTop;
		fWidth = pRect.fWidth;
		fHeight = pRect.fHeight;
	};

	float fLeft;
	float fTop;
	float fWidth;
	float fHeight;

} UIRect;


//! Border Structure
typedef struct UIBorder
{
	UIBorder(): iFlags(0xffff), iStyle(1), fSize(0.0f), cColor(color4f(0.35f, 0.35f, 0.35f, 1.0f)) {};
	UIBorder(int iNewStyle, float fNewSize, const color4f &cNewColor, int iNewFlags)
	{
		iStyle = iNewStyle;
		fSize = fNewSize;
		cColor = cNewColor;
		iFlags = iNewFlags;
	};
	UIBorder(const UIBorder &pBorder)
	{
		iStyle = pBorder.iStyle;
		fSize = pBorder.fSize;
		cColor = pBorder.cColor;
		iFlags = pBorder.iFlags;
	};

	int						iStyle;
	float					fSize;
	color4f	cColor;
	int						iFlags;

} UIBorder;


//! Font Structure
typedef struct UIFont
{
	UIFont(): szFaceName("Default"), szEffectName("Default"), fSize(14.0f), cColor(color4f(1.0f, 1.0f, 1.0f, 1.0f)) {};
	UIFont(const string &szNewName, const string &szNewEffect, float fNewSize, const color4f &cNewColor)
	{
		szFaceName = szNewName;
		szEffectName = szNewEffect;
		fSize = fNewSize;
		cColor = cNewColor;
	};
	UIFont(const UIFont &pFont)
	{
		szFaceName = pFont.szFaceName;
		szEffectName = pFont.szEffectName;
		fSize = pFont.fSize;
		cColor = pFont.cColor;
	};

	string		szFaceName;
	string		szEffectName;
	float					fSize;
	color4f	cColor;																																																								

} UIFont;


//! Skin Structure
typedef struct UISkinTexture
{
	UISkinTexture(): iTextureID(-1), iOverTextureID(-1), iDownTextureID(-1)
	{
		vTexCoord[0] = 0;
		vTexCoord[1] = 1;
		vTexCoord[2] = 1;
		vTexCoord[3] = 0;
	};
	UISkinTexture(int iTexID): iTextureID(iTexID), iOverTextureID(iTexID), iDownTextureID(iTexID)
	{
		vTexCoord[0] = 0;
		vTexCoord[1] = 1;
		vTexCoord[2] = 1;
		vTexCoord[3] = 0;
	};
	UISkinTexture(int iTexID, int iOverTexID, int iDownTexID): iTextureID(iTexID), iOverTextureID(iOverTexID), iDownTextureID(iDownTexID)
	{
		vTexCoord[0] = 0;
		vTexCoord[1] = 1;
		vTexCoord[2] = 1;
		vTexCoord[3] = 0;
	};
	UISkinTexture(const UISkinTexture &pSkinTexture)
	{
		iTextureID = pSkinTexture.iTextureID;
		iOverTextureID = pSkinTexture.iOverTextureID;
		iDownTextureID = pSkinTexture.iDownTextureID;

		vTexCoord[0] = pSkinTexture.vTexCoord[0];
		vTexCoord[1] = pSkinTexture.vTexCoord[1];
		vTexCoord[2] = pSkinTexture.vTexCoord[2];
		vTexCoord[3] = pSkinTexture.vTexCoord[3];
	}
	~UISkinTexture() {};

	INT_PTR	iTextureID;
	INT_PTR iOverTextureID;
	INT_PTR iDownTextureID;

	float	vTexCoord[4];
} UISkinTexture;

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 

class CUIWidget;
class CUISystem;


typedef std::vector<CUIWidget *> CUIWidgetList;
typedef std::vector<CUIWidget *>::iterator CUIWidgetItor;


//! UI Widget interface
class CUIWidget
{
	friend class CUISystem;
	friend class CUIScreen;

public:
	//! Constructor
	CUIWidget();
	//! Destructor
	virtual ~CUIWidget() {}

	//! Set the widget name
	virtual int SetName(const string &szName);
	//! Get the widget name
	virtual string &GetName();

	//! Get the classname of the widget
	virtual string GetClassName() = 0;

	//! Get the child list
	virtual CUIWidgetList *GetChildList();
	//! Get a child by index
	virtual CUIWidget *GetChild(int iIndex);
	//! Get a child by name
	virtual CUIWidget *GetChild(const string &szName);
	//! Get the number of children
	virtual int	GetChildCount();

	//! Add a widget as child
	virtual int AddChild(CUIWidget *pWidget);
	//! Remove a widget from child
	virtual int DelChild(CUIWidget *pWidget);
	//! Remove a widget from child
	virtual int DelChild(int iIndex);
	//! Remove a widget from child
	virtual int DelChild(const string &szName);

	//! Get the widget flags
	virtual int GetFlags();
	//! Set the widget flags
	virtual int SetFlags(int iFlags);

	//! Get the widget style flags
	virtual int GetStyle();
	//! Set the widget style flags
	virtual int SetStyle(int iStyle);

	//! Get the widget rect
	virtual int GetRect(UIRect *pRect);
	//! Set the widget rect
	virtual int SetRect(const UIRect &pRect, bool bRelative = true);

	//! Get the border
	virtual int GetBorder(UIBorder *pBorder);

	//! Set the border
	virtual int SetBorder(const UIBorder &pBorder);

	//! Get the widget Z value
	virtual int GetZ();
	//! Set the widget Z value
	virtual int SetZ(int iZ);

	//! Get the widget tabstop
	virtual int GetTabStop();
	//! Set the widget tabstop
	virtual int SetTabStop(int iTabStop);

	//! Get the tooltip of the window
	virtual int GetToolTip(const float fX, const float fY, wstring &szwToolTip);
    
	//! Process messages
	virtual LRESULT Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam) = 0;	//AMD Port
	//! Draw the widget, this is usually called by the widget, when the message UIM_DRAW is sent to it
	virtual int Draw(int iPass) = 0;

	//! Release the widget
	virtual int Release();

	//! Get the widget LUA script object
	virtual IScriptObject *GetScriptObject() = 0;
	//------------------------------------------------------------------------------------------------- 
	// Script Events
	//------------------------------------------------------------------------------------------------- 
	virtual int OnInit();
	virtual int OnRelease();
	virtual int OnKeyDown(int iKeyCode);
	virtual int OnKeyUp(int iKeyCode);
	virtual int OnKeyPressed(int iKeyCode);
	virtual int OnMouseEnter();
	virtual int OnMouseLeave();
	virtual int OnMouseDown(int iKeyCode, float fX, float fY);
	virtual int OnMouseUp(int iKeyCode, float fX, float fY);
	virtual int OnMouseClick(int iKeyCode, float fX, float fY);
	virtual int OnMouseDblClick(int iKeyCode, float fX, float fY);
	virtual int OnMouseOver(float fNewX, float fNewY, float fOldX, float fOldY);
	virtual int OnMouseMove(float fNewX, float fNewY, float fOldX, float fOldY);
	virtual int OnGotFocus();
	virtual int OnLostFocus();
	virtual int OnSized();
	virtual int OnMoved();
	virtual int OnChanged();
	virtual int OnCommand();

	//------------------------------------------------------------------------------------------------- 
	// Common Script Functions
	//------------------------------------------------------------------------------------------------- 
	virtual int GetName(IFunctionHandler *pH);
	virtual int GetScreen(IFunctionHandler *pH);
	virtual int GetChildCount(IFunctionHandler *pH);
	virtual int GetChild(IFunctionHandler *pH);

	virtual int Release(IFunctionHandler *pH);

	virtual int SetRect(IFunctionHandler *pH);
	virtual int GetRect(IFunctionHandler *pH);

	virtual int SetColor(IFunctionHandler *pH);
	virtual int GetColor(IFunctionHandler *pH);

	virtual int SetBorderColor(IFunctionHandler *pH);
	virtual int GetBorderColor(IFunctionHandler *pH);

	virtual int SetBorderSize(IFunctionHandler *pH);
	virtual int GetBorderSize(IFunctionHandler *pH);

	virtual int SetBorderStyle(IFunctionHandler *pH);
	virtual int GetBorderStyle(IFunctionHandler *pH);

	virtual int SetFontName(IFunctionHandler *pH);
	virtual int GetFontName(IFunctionHandler *pH);

	virtual int SetFontEffect(IFunctionHandler *pH);
	virtual int GetFontEffect(IFunctionHandler *pH);

	virtual int SetFontColor(IFunctionHandler *pH);
	virtual int GetFontColor(IFunctionHandler *pH);

	virtual int SetFontSize(IFunctionHandler *pH);
	virtual int GetFontSize(IFunctionHandler *pH);

	virtual int SetMouseCursor(IFunctionHandler *pH);
	virtual int GetMouseCursor(IFunctionHandler *pH);

	virtual int SetToolTip(IFunctionHandler *pH);

	virtual int SetGreyedColor(IFunctionHandler *pH);
	virtual int GetGreyedColor(IFunctionHandler *pH);

	virtual int SetGreyedBlend(IFunctionHandler *pH);
	virtual int GetGreyedBlend(IFunctionHandler *pH);

protected:

	virtual int SortChildrenByZ();
	virtual int DrawChildren();

	static 	bool SortCallback(CUIWidget *pOne, CUIWidget *pTwo)
	{
		return pOne->GetZ() < pTwo->GetZ();
	}

	string		m_szName;
	CUIWidgetList	m_pChildList;

	int					m_iFlags;
	int					m_iStyle;

	bool				m_bMoving;

	color4f	m_cColor;
	color4f	m_cGreyedColor;
	int						m_iGreyedBlend;

	int					m_iZ;

	int					m_iTabStop;

	UIRect			m_pRect;
	UIBorder		m_pBorder;
	UIFont			m_pFont;

	INT_PTR     m_iMouseCursor;

	CUIWidget		*m_pParent;
	CUISystem		*m_pUISystem;
	CUIScreen		*m_pScreen;

	wstring			m_szwToolTip;

	//------------------------------------------------------------------------------------------------- 
	CUIWidgetList m_vVisibleWidgetList;

	//------------------------------------------------------------------------------------------------- 
};

#endif
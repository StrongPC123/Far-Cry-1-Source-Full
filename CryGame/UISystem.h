//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - UI System Window and Input Manager
//
// History:
//  - [3/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#pragma once


#include "UIWidget.h"
#include "UIMessages.h"
#include "UICVars.h"


// Warning very compressed code block ahead :D
#define REGISTER_SCRIPTOBJECT_MEMBER(scriptsystem,classname,functionname){classname::RegisterFunction(scriptsystem,#functionname,&classname::functionname);}
#define REGISTER_SCRIPT_CONSTANT(scriptsystem,constant)(scriptsystem)->SetGlobalValue(#constant,constant)
#define GET_SCRIPT_TYPE_STRING(itype)(itype==svtObject?"Object":(itype==svtString?"String":(itype==svtNumber?"Number":(itype==svtFunction?"Function":(itype==svtUserData?"UserData":(itype==svtNull?"Null":"Unknown"))))))
#define CHECK_SCRIPT_FUNCTION_PARAMCOUNT(scriptsystem,objectname,funcname,nparameters){if(pH->GetParamCount()!=nparameters){scriptsystem->RaiseError("%s:%s() Wrong number of parameters! Expected %d, but found %d!",objectname,#funcname,nparameters, pH->GetParamCount());return pH->EndFunctionNull();}}
#define CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(scriptsystem,objectname,funcname,nparameters,nparameters2){if((pH->GetParamCount()!=nparameters)&&(pH->GetParamCount()!=nparameters2)){scriptsystem->RaiseError("%s:%s() Wrong number of parameters! Expected %d or %d, but found %d!",objectname,#funcname,nparameters, nparameters2, pH->GetParamCount());return pH->EndFunctionNull();}}
#define CHECK_SCRIPT_FUNCTION_PARAMTYPE(scriptsystem,objectname,funcname,idparam,type){if(pH->GetParamType(idparam)!=type){scriptsystem->RaiseError("%s:%s() Wrong type in parameter %d! Expected '%s', but found '%s'!",objectname,#funcname,idparam,GET_SCRIPT_TYPE_STRING(type),GET_SCRIPT_TYPE_STRING(pH->GetParamType(idparam)));return pH->EndFunctionNull();}}

#define CHECK_SCRIPT_FUNCTION_PARAMTYPE2(scriptsystem,objectname,funcname,idparam,type,type2){if((pH->GetParamType(idparam)!=type)&&(pH->GetParamType(idparam)!=type2)){scriptsystem->RaiseError("%s:%s() Wrong type in parameter %d! Expected '%s' or '%s', but found '%s'!",objectname,#funcname,idparam,GET_SCRIPT_TYPE_STRING(type),GET_SCRIPT_TYPE_STRING(type2),GET_SCRIPT_TYPE_STRING(pH->GetParamType(idparam)));return pH->EndFunctionNull();}}

#define CHECK_SCRIPT_FUNCTION_PARAMTYPE3(scriptsystem,objectname,funcname,idparam,type,type2,type3){if((pH->GetParamType(idparam)!=type)&&(pH->GetParamType(idparam)!=type2)&&(pH->GetParamType(idparam)!=type3)){scriptsystem->RaiseError("%s:%s() Wrong type in parameter %d! Expected '%s', '%s' or '%s', but found '%s'!",objectname,#funcname,idparam,GET_SCRIPT_TYPE_STRING(type),GET_SCRIPT_TYPE_STRING(type2),GET_SCRIPT_TYPE_STRING(type3),GET_SCRIPT_TYPE_STRING(pH->GetParamType(idparam)));return pH->EndFunctionNull();}}

//------------------------------------------------------------------------------------------------- 
#define RETURN_TEXTURE_FROM_SCRIPT(scriptsystem, objectname, functionname, varname)	\
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(scriptsystem, objectname, functionname, 1); \
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(scriptsystem, objectname, functionname, 1, svtUserData, svtNull); \
	int iCookie = 0; \
	USER_DATA iTextureID = -1; \
	if(pH->GetParamUDVal(1, iTextureID, iCookie)) \
		varname = iTextureID; \
	else \
		varname = -1; \
	return pH->EndFunctionNull() \

//------------------------------------------------------------------------------------------------- 
#define RETURN_TEXTURE_TO_SCRIPT(scriptsystem, objectname, functionname, varname) \
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(scriptsystem, objectname, functionname, 0); \
	USER_DATA pUserData = scriptsystem->CreateUserData((int)varname, USER_DATA_TEXTURE); \
	return pH->EndFunction(pUserData) \

//------------------------------------------------------------------------------------------------- 
#define RETURN_INT_TO_SCRIPT(scriptsystem, objectname, functionname, varname) \
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(scriptsystem, objectname, functioname, 0); \
	return pH->EndFunction(varname) \

//------------------------------------------------------------------------------------------------- 
#define RETURN_INT_FROM_SCRIPT(scriptsystem, objectname, functionname, varname) \
    CHECK_SCRIPT_FUNCTION_PARAMCOUNT(scriptsystem, objectname, functionname, 1); \
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(scriptsystem, objectname, functionname, 1, svtNumber); \
    pH->GetParam(1, varname); \
	return pH->EndFunction() \

#define RETURN_COLOR_FROM_SCRIPT(scriptsystem, objectname, functionname, varname) \
    CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(scriptsystem, objectname, functionname, 1, 4); \
    if ((pH->GetParamCount() == 1) && (pH->GetParamType(1) == svtString)) \
    { \
		color4f cColor; \
		char *szColor; \
		pH->GetParam(1, szColor); \
		m_pUISystem->RetrieveColor(&cColor, szColor); \
		varname = cColor; \
	} \
	else if ((pH->GetParamCount() == 4) && (pH->GetParamType(1) == svtNumber) && (pH->GetParamType(4) == svtNumber)) \
	{ \
		color4f cColor; \
		int iColor; \
		pH->GetParam(1, iColor); \
		cColor.v[0] = iColor * (1.0f / 255.0f); \
		pH->GetParam(2, iColor); \
		cColor.v[1] = iColor * (1.0f / 255.0f); \
		pH->GetParam(3, iColor); \
		cColor.v[2] = iColor * (1.0f / 255.0f); \
		pH->GetParam(4, iColor); \
		cColor.v[3] = iColor * (1.0f / 255.0f); \
		varname = cColor; \
	} \
	else \
	{ \
		m_pUISystem->GetIScriptSystem()->RaiseError("%s:%s() Wrong type in parameter 1! Expected 'String' or 'Number', but found '%s'!", objectname, #functionname, GET_SCRIPT_TYPE_STRING(pH->GetParamType(1))); \
		return pH->EndFunctionNull(); \
	} \
	return pH->EndFunctionNull() \

#define RETURN_COLOR_TO_SCRIPT(scriptsystem, objectname, functionname, varname) \
    CHECK_SCRIPT_FUNCTION_PARAMCOUNT(scriptsystem, objectname, functionname, 0); \
	char szColor[64]; \
	m_pUISystem->ConvertToString(szColor, varname); \
	return pH->EndFunction(szColor) \


//------------------------------------------------------------------------------------------------- 
// Flags and Styles
//------------------------------------------------------------------------------------------------- 


// widget flags
#define UIFLAG_VISIBLE			(1 << 0)
#define UIFLAG_ENABLED			(1 << 1)
#define UIFLAG_MOVEABLE			(1 << 3)
#define UIFLAG_CANHAVEFOCUS	(1 << 4)
#define UIFLAG_CANCHANGEZ		(1 << 5)

#define UIFLAG_HAVEFOCUS		(1 << 31)

#define UIFLAG_DEFAULT			(UIFLAG_VISIBLE | UIFLAG_ENABLED | UIFLAG_CANHAVEFOCUS)


// widget styles
#define UISTYLE_TRANSPARENT			(1 << 0)
#define UISTYLE_SHADOWED				(1 << 1)
#define UISTYLE_MULTILINE				(1 << 2)
#define UISTYLE_WORDWRAP				(1 << 3)
#define UISTYLE_PASSWORD				(1 << 4)
#define UISTYLE_MULTISELECTION	(1 << 5)

#define UISTYLE_DEFAULT			(0)


// text alignment
#define UIALIGN_CENTER			0
#define UIALIGN_LEFT				1
#define UIALIGN_RIGHT				2
#define UIALIGN_MIDDLE			10
#define UIALIGN_TOP					11
#define UIALIGN_BOTTOM			12


// border style
#define UIBORDERSTYLE_NONE		0
#define UIBORDERSTYLE_FLAT		1
#define UIBORDERSTYLE_RAISED	2
#define UIBORDERSTYLE_SUNKEN	3

// border flags
#define UIBORDERSIDE_LEFT			(1 << 0)
#define UIBORDERSIDE_TOP			(1 << 1)
#define UIBORDERSIDE_RIGHT		(1 << 2)
#define UIBORDERSIDE_BOTTOM		(1 << 3)

// scroll bar types
#define UISCROLLBARTYPE_AUTOMATIC		(0)
#define UISCROLLBARTYPE_HORIZONTAL	(1)
#define UISCROLLBARTYPE_VERTICAL		(2)


// states
#define UISTATE_UP						(1 << 0)
#define UISTATE_DOWN					(1 << 1)
#define UISTATE_OVER					(1 << 2)
#define UISTATE_CHECKED				(1 << 3)

// sort order
#define UISORT_ASCENDING				1
#define UISORT_DESCENDING				2

// greyed blend
#define UIBLEND_OVERLAY					0
#define UIBLEND_ADDITIVE				1

// default ui values
#define UI_DEFAULT_TEXT_BORDER_SIZE		(2.0f)
#define UI_DEFAULT_FOCUS_BORDER_SIZE	(2.0f)
#define UI_DEFAULT_SHADOW_BORDER_SIZE	(4.0f)
#define UI_DEFAULT_SHADOW_COLOR				(color4f(0.0f, 0.0f, 0.0f, 0.4f))
#define UI_DEFAULT_EMBOSS_BORDER_SIZE	(2.0f)
#define UI_DEFAULT_CURSOR_ADVANCE			(2.0f)
#define UI_DEFAULT_CURSOR_BLINK_SPEED	(2.5f)
//#define UI_DEFAULT_SEPARATION			(1.0f)
#define UI_DEFAULT_LINE_WIDTH_ADDITION (2.0f)
#define UI_DEFAULT_MAX_WRAP_INDICES		(8)
#define UI_DEFAULT_WORDWRAP_TRESHOLD	(16)
#define UI_DEFAULT_Z									(0)
#define UI_DEFAULT_PASSES							(2)
#define UI_DEFAULT_IDLETIME_START			(5)

/*
#define UI_DEFAULT_SELECTION_COLOR_0		(color4f(0.0f, 0.0f, 0.25f, 1.0f))
#define UI_DEFAULT_SELECTION_COLOR_1		(color4f(0.25f, 0.25f, 0.25f, 1.0f))
#define UI_DEFAULT_SELECTION_COLOR_2		(color4f(0.75f, 0.75f, 0.75f, 1.0f))
#define UI_DEFAULT_SELECTION_COLOR_3		(color4f(0.25f, 0.25f, 0.5f, 1.0f))
*/

// flip styles
#define UIFLIP_NONE								(0)
#define UIFLIP_HORIZONTAL					(1)
#define UIFLIP_VERTICAL						(2)
#define UIFLIP_ROTATE90						(3)
#define UIFLIP_ROTATE270					(4)


// get emboss colors
#define GET_HIGHLIGHT_COLOR(c)				((c) * 1.35f)
#define GET_SHADOWED_COLOR(c)					(color4f(c.v[0] * 0.45f, c.v[1] * 0.45f, c.v[2] * 0.45f, c.v[3]))


//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 

struct IFFont;
struct IGame;
struct ISystem;
struct IScriptSystem;
struct UIRect;
struct UIFont;
struct UIBorder;

class CScriptObjectUI;

class CUIWidget;
class CUIStatic;
class CUIButton;
class CUIEditBox;
class CUIScrollBar;
class CUIListView;
class CUICheckBox;
class CUIComboBox;
class CUIVideoPanel;

class CUIScreen;


typedef std::vector<CUIScreen *>						CUIScreenList;
typedef std::vector<CUIScreen *>::iterator	CUIScreenItor;


class CUISystem : public IProcess, public IInputEventListener
{
	friend class CScriptObjectUI;
public:
	CUISystem();
	~CUISystem();

	//------------------------------------------------------------------------------------------------- 
	static LRESULT DefaultUpdate(CUIWidget *pWidget, unsigned int iMessage, WPARAM wParam, LPARAM lParam);

	//------------------------------------------------------------------------------------------------- 
	int	Create(IGame *pGame, ISystem *pSystem, IScriptSystem *pScriptSystem, const string &szScriptFileName, bool bRunScriptFile = true);
	bool Init() { return 1; };	
	void Update();
	void Draw();
	void ShutDown(bool bEditorMode);
	void SetFlags(int iFlags) { m_iFlags = iFlags; };
	int	GetFlags() { return 0; };

	int Release();

	//------------------------------------------------------------------------------------------------- 
	int Reset();
	int Reload(int iFrameDelta = 0);

	//------------------------------------------------------------------------------------------------- 
	int Enable();
	int Disable();
	int IsEnabled();

	//------------------------------------------------------------------------------------------------- 
	int CreateCVars();
	int ReleaseCVars();

	//------------------------------------------------------------------------------------------------- 
	CScriptObjectUI *GetScriptObjectUI() { return m_pScriptObjectUI; };

	//------------------------------------------------------------------------------------------------- 
	IGame *GetIGame() { return m_pGame; };
	IScriptSystem *GetIScriptSystem() { return m_pScriptSystem; };
	ISystem	*GetISystem() { return m_pSystem; };
	IRenderer *GetIRenderer() { return m_pRenderer; };
	IInput *GetIInput() { return m_pInput; };

	//------------------------------------------------------------------------------------------------- 
	int ResetKeyRepeat();

	//------------------------------------------------------------------------------------------------- 
	IScriptObject *GetWidgetScriptObject(CUIWidget *pWidget);

	//------------------------------------------------------------------------------------------------- 
	CUIWidgetList *GetWidgetList();
	CUIWidget *GetWidget(int iIndex);
	CUIWidget *GetWidget(const string &szName);
	CUIWidget *GetWidget(const string &szName, const string &szScreenName);
	int	GetWidgetCount();
	int WidgetExist(CUIWidget *pWidget);

	//------------------------------------------------------------------------------------------------- 
	CUIScreenList *GetScreenList();
	CUIScreen *GetScreen(int iIndex);
	CUIScreen *GetScreen(const string &szName);
	int	GetScreenCount();
	int ActivateScreen(CUIScreen *pScreen);
	int DeactivateScreen(CUIScreen *pScreen);
	int IsScreenActive(CUIScreen *pScreen);
	int GetActiveScreenCount();
	int DeactivateAllScreens();
	int ActivateAllScreens();

	//------------------------------------------------------------------------------------------------- 
	CUIWidgetList *GetChildList();
	CUIWidget *GetChild(int iIndex);
	CUIWidget *GetChild(const string &szName);
	int	GetChildCount();
	
	int AddChild(CUIWidget *pWidget);
	int DelChild(CUIWidget *pWidget);
	int DelChild(int iIndex);
	int DelChild(const string &szName);

	//------------------------------------------------------------------------------------------------- 
	int SetBackground(int iBackgroundTexture);
	int GetBackground();
	int SetBackgroundColor(const color4f &cColor);
	int GetBackgroundColor(color4f *pColor);
	int ShowBackground();
	int HideBackground();
	bool IsBackgroundVisible();

	//------------------------------------------------------------------------------------------------- 
	int SetGreyedColor(const color4f &cColor);
	int GetGreyedColor(color4f *cColor);

	//------------------------------------------------------------------------------------------------- 
	int SetMouseCursor(int iTextureID);
	int GetMouseCursor();
	int SetMouseCursorColor(const color4f &cColor);
	int GetMouseCursorColor(color4f *pColor);
	int SetMouseCursorSize(float fWidth, float fHeight);
	int GetMouseCursorSize(float *fWidth, float *fHeight);
	int ShowMouseCursor();
	int HideMouseCursor();
	bool IsMouseCursorVisible();
	int SetMouseXY(float fX, float fY);
	vector2f GetMouseXY();
	int CaptureMouse(CUIWidget *pWidget);
	int ReleaseMouse();

	float GetIdleTime();
	//------------------------------------------------------------------------------------------------- 
	LRESULT SendMessage(string &szName, const string &szScreenName, int iMessage, WPARAM wParam, LPARAM lParam);
	LRESULT SendMessage(string &szName, int iMessage, WPARAM wParam, LPARAM lParam);
	LRESULT SendMessage(CUIWidget *pWidget, int iMessage, WPARAM wParam, LPARAM lParam);
	LRESULT BroadcastMessage(int iMessage, WPARAM wParam, LPARAM lParam);

	//------------------------------------------------------------------------------------------------- 
	CUIWidget *GetWidgetParent(CUIWidget *pWidget);
	wstring GetWidgetText(CUIWidget *pWidget);
	UIRect &GetWidgetRect(CUIWidget *pWidget);

	//------------------------------------------------------------------------------------------------- 
	CUIWidget *GetTabStop(int iTabStop);
	int FirstTabStop();
	int NextTabStop();
	int PrevTabStop();
	int LastTabStop();

	//------------------------------------------------------------------------------------------------- 
	int SetTopMostWidget(const string &szName);
	int SetTopMostWidget(CUIWidget *pWidget);
	CUIWidget *GetTopMostWidget();

	//------------------------------------------------------------------------------------------------- 
	int SetFocus(CUIWidget *pWidget);
	int SetFocus(string &szName);
	int SetFocus(string &szName, string &szScreenName);
	CUIWidget *GetFocus();
	int SetFocusScreen(CUIScreen *pScreen);
	int SetFocusScreen(string &szScreenName);
	CUIScreen *GetFocusScreen();


	//------------------------------------------------------------------------------------------------- 
	color4f GetSelectionColor(const color4f &cBackground, const color4f &cTextcolor);
	IFFont *GetIFont(const UIFont &pFont);
	int	GetAlignedTextXY(float *fNewX, float *fNewY, IFFont *pFont, const UIRect &pTextRect, const wchar_t *szString, int iAlignmentX, int iAlignmentY);
	
	//------------------------------------------------------------------------------------------------- 
	int	GetAbsoluteXY(float *fNewX, float *fNewY, float fRelativeX, float fRelativeY, CUIWidget *pWidget);
	int	GetRelativeXY(float *fNewX, float *fNewY, float fAbsoluteX, float fAbsoluteY, CUIWidget *pWidget);
	int	IntersectRect(UIRect *pNewRect, const UIRect pRect1, const UIRect pRect2);
	int	AdjustRect(UIRect *pNewRect, const UIRect pRect, float fBorderSize, bool bGrow = false);
	bool PointInRect(const UIRect &pRect, float fX, float fY);
	CUIWidget *FindWidgetAt(float fX, float fY);

	//------------------------------------------------------------------------------------------------- 
	int SetScissor(const UIRect *pRect);
	int BeginDraw(CUIWidget *pWidget);
	int ResetDraw();
	int EndDraw();

	float AdjustWidth(float fBorderW);
	float AdjustHeight(float fBorderH);

	//------------------------------------------------------------------------------------------------- 
	int	GetWidgetCanvas(UIRect *pWidgetCanvas, CUIWidget *pWidget);

	//------------------------------------------------------------------------------------------------- 
	int ShowWidget(CUIWidget *pWidget);
	int HideWidget(CUIWidget *pWidget);
	int IsWidgetVisible(CUIWidget *pWidget);
	int EnableWidget(CUIWidget *pWidget);
	int DisableWidget(CUIWidget *pWidget);
	int IsWidgetEnabled(CUIWidget *pWidget);

	//------------------------------------------------------------------------------------------------- 
	int DrawBackground();
	int DrawMouseCursor(float fLeft, float fTop);
	int DrawBorder(const UIRect &pRect, const UIBorder &pBorder);
	int DrawQuad(const UIRect &pRect, const color4f &cColor);
	int DrawGreyedQuad(const UIRect &pRect, const color4f &cColor, int iMode);
	int DrawText(const UIRect &pRect, int iHAlignment, int iVAlignment, IFFont *pFont, const wchar_t *szText, bool bTranslateEscapes = 1);
	int DrawMultiLineText(const UIRect &pRect, int iHAlignment, int iVAlignment, IFFont *pFont, const std::vector<wstring> &vLines, float fHorizontalOffset = 0.0f, float fVerticalOffset = 0.0f);
	int DrawImage(const UIRect &pRect, const UISkinTexture &pTexture, const color4f &cColor);
	int DrawImage(const UIRect &pRect, int iTextureID, const float *vTexCoord, const color4f &cColor);
	int DrawSkin(const UIRect &pRect, const UISkinTexture &pTexture, const color4f &cColor, int iState);
	int DrawEmboss(const UIRect &pRect, const color4f &cHighlightColor, const color4f &cShadowedColor, bool bPressed, float fBorderSize);
	int DrawFocus(const UIRect &pRect, const color4f &cColor, float fBorderSize);
	int DrawShadow(const UIRect &pRect, const color4f &cColor, float fBorderSize, CUIWidget *pWidget = 0);
	int DrawButton(const UIRect &pRect, const color4f &cColor, float fEmbossSize, bool bPressed);
	int DrawToolTip();

	//------------------------------------------------------------------------------------------------- 
	int CreateStatic(CUIStatic **pStatic, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle, const wstring &szText);
	int CreateButton(CUIButton **pButton, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle, const wstring &szText);
	int CreateEditBox(CUIEditBox **pEditBox, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle, const wstring &szText);
	int CreateScrollBar(CUIScrollBar **pScrollBar, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle, int iType = 0);
	int CreateListView(CUIListView **pListView, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle);
	int CreateCheckBox(CUICheckBox **pCheckBox, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle);
	int CreateComboBox(CUIComboBox **pComboBox, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle);
	int CreateVideoPanel(CUIVideoPanel **pVideoPanel, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle);
	int CreateScreen(CUIScreen **pScreen, const string &szName);

	//------------------------------------------------------------------------------------------------- 
	int InheritParentAttributes(CUIWidget *pWidget, CUIWidget *pParent);
	int RetrieveCommonAttribute(IScriptObject *pObject, CUIWidget *pWidget);
	int RetrieveColor(color4f *pColor, char *szString);
	int RetrieveRect(UIRect *pRect, char *szString);
	int RetrieveTexRect(float *pTexCoords, INT_PTR iTextureID, char *szTexRect);
	int RetrieveTextAttribute(CUIWidget *pWidget, IScriptObject *pObject, const string &szTextField);
	int RetrieveTextureAttribute(UISkinTexture *pSkinTexture, IScriptObject *pObject, const char *szTextureField, const char *szTexRectField);

	//------------------------------------------------------------------------------------------------- 
	int CreateObjectFromTable(CUIWidget **pWidget, CUIWidget *pParent, CUIScreen *pScreen, IScriptObject *pObject, const string &szName);
	int CreateStaticFromTable(CUIStatic **pStatic,CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName);
	int CreateButtonFromTable(CUIButton **pButton, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName);
	int CreateEditBoxFromTable(CUIEditBox **pEditBox, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName);
	int CreateScrollBarFromTable(CUIScrollBar **pScrollBar, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName);
	int CreateListViewFromTable(CUIListView **pListView, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName);
	int CreateCheckBoxFromTable(CUICheckBox **pCheckBox, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName);
	int CreateComboBoxFromTable(CUIComboBox **pComboBox, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName);
	int CreateVideoPanelFromTable(CUIVideoPanel **pVideoPanel, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName);
	int CreateScreenFromTable(CUIScreen **pScreen, const string &szName, IScriptObject *pObject);

	//------------------------------------------------------------------------------------------------- 
	int SetupStaticFromTable(CUIStatic *pStatic, IScriptObject *pObject);
	int SetupButtonFromTable(CUIButton *pButton, IScriptObject *pObject);
	int SetupEditBoxFromTable(CUIEditBox *pEditBox, IScriptObject *pObject);
	int SetupScrollBarFromTable(CUIScrollBar *pScrollBar, IScriptObject *pObject);
	int SetupListViewFromTable(CUIListView *pListView, IScriptObject *pObject);
	int SetupCheckBoxFromTable(CUICheckBox *pCheckBox, IScriptObject *pObject);
	int SetupComboBoxFromTable(CUIComboBox *pomboBox, IScriptObject *pObject);
	int SetupVideoPanelFromTable(CUIVideoPanel *pVideoPanel, IScriptObject *pObject);

	//------------------------------------------------------------------------------------------------- 
	//_int LogToConsole(char *szFormat, ...);
	int ConvertToWString(wstring &szWString, IFunctionHandler *pH, int iParam);
	int ConvertToWString(wstring &szWString, const char *szString);
	int ConvertToWString(wstring &szWString, int iStrID);
	int ConvertToString(char *szString, const color4f &pColor);
	int ConvertToString(char *szString, const UIRect &pRect);
	int ConvertToString(char *szString, const wstring &szWString, int iMaxSize = 0);
	int ConvertToString(string &szString, const wstring &szWString);
	int StripControlCodes(wstring &szOutString, const wstring &szWString);
	int StripControlCodes(string &szOutString, const wstring &szWString);
	int StripControlCodes(string &szOutString, const string &szString);

	//------------------------------------------------------------------------------------------------- 
	int DestroyWidget(CUIWidget *pWidget);
	int DestroyScreen(CUIScreen *pScreen);

	//------------------------------------------------------------------------------------------------- 
	int UnloadAllModels();
	int ReloadAllModels();
	int StopAllVideo();
	int ResetInput();

	//------------------------------------------------------------------------------------------------- 
	int OnZChanged(CUIWidget *pWidget);
	int ShouldSortByZ() { return m_bSortZ; };

	//-------------------------------------------------------------------------------------------------
	int OnTabStopChanged(CUIWidget *pWidget);
	int ShouldSortTabStop() { return m_bSortTabStop; };

private:

	int SortChildrenByZ();
	int SortTabStop();

	bool OnInputEvent(const SInputEvent &event);

	int InitializeWidget(CUIWidget *pWidget, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle);
	int InitializeTemplates();
	int ReleaseTemplates();
	void DeleteWidget(CUIWidget *pWidget);

	bool IsReserved(const char *szName);
	bool IsOnFocusScreen(CUIWidget *pWidget);
    
	//------------------------------------------------------------------------------------------------- 
	static bool SortZCallback(CUIWidget *pOne, CUIWidget *pTwo)
	{
		if (pOne->GetZ() > pOne->m_pUISystem->m_iMaxZ)
		{
			pOne->m_pUISystem->m_iMaxZ = pOne->GetZ();
		}
		if (pOne->GetZ() < pOne->m_pUISystem->m_iMinZ)
		{
			pOne->m_pUISystem->m_iMinZ = pOne->GetZ();
		}
		if (pTwo->GetZ() > pOne->m_pUISystem->m_iMaxZ)
		{
			pOne->m_pUISystem->m_iMaxZ = pTwo->GetZ();
		}
		if (pTwo->GetZ() < pOne->m_pUISystem->m_iMinZ)
		{
			pOne->m_pUISystem->m_iMinZ = pTwo->GetZ();
		}
 
		if(pOne->GetZ() == pTwo->GetZ())		// to compensate the instable sort algorithm we use (prevent flickering buttons)
			return pOne>pTwo;

		return (pOne->GetZ() < pTwo->GetZ());
	}

	static bool SortTabStopCallback(CUIWidget *pOne, CUIWidget *pTwo)
	{
		if ((pOne->m_pScreen == pTwo->m_pScreen) && (pOne->GetTabStop() == pTwo->GetTabStop()))
		{
			if (pOne != pTwo)
				pOne->m_pUISystem->m_pLog->Log("Widget '%s' and '%s' have the same TabStop!", pOne->GetName().c_str(), pTwo->GetName().c_str());
		}

		if(pOne->GetTabStop() == pTwo->GetTabStop())		// to compensate the instable sort algorithm we use
			return pOne>pTwo;

		return pOne->GetTabStop() < pTwo->GetTabStop();
	}

	int						m_iMouseCurrentCursor;
	int						m_iMouseCursorTextureID;
	float					m_iMouseCursorTexPixW;
	float					m_iMouseCursorTexPixH;
	float					m_fMouseCursorWidth;
	float					m_fMouseCursorHeight;
	color4f				m_cMouseCursorColor;

	color4f				m_cGreyedColor;

	int						m_iBackgroundTextureID;
	color4f				m_cBackgroundColor;

	int						m_iFlags;

	IGame					*m_pGame;
	ISystem				*m_pSystem;
	IScriptSystem	*m_pScriptSystem;
	IRenderer			*m_pRenderer;
	IInput				*m_pInput;
	ILog					*m_pLog;

	string		m_szScriptFileName;

	//------------------------------------------------------------------------------------------------- 
	CScriptObjectUI	*m_pScriptObjectUI;
	//------------------------------------------------------------------------------------------------- 

	CUIWidgetList	m_pWidgetList;

	UIRect				m_pScissorRect;
	UIRect				m_pCurrentDrawRect;
	IFFont				*m_pCurrentDrawFont;
	bool					m_bDrawing;
	
	CUIWidgetList	m_pChildList;

	int						m_iMaxZ;
	int						m_iMinZ;
	//------------------------------------------------------------------------------------------------- 

	CUIWidget			*m_pMouseCaptured;

	//------------------------------------------------------------------------------------------------- 

	CUIWidgetList	m_pTabStopList;
	int						m_iCurrentTabStop;
	CUIWidget			*m_pFocus;
	CUIScreen			*m_pFocusScreen;
	
	//------------------------------------------------------------------------------------------------- 

	vector2f			m_vMouseXY;
	CUIWidget			*m_pMouseOver;
	bool					m_bLMouseDown;

	//------------------------------------------------------------------------------------------------- 
	int						m_iLastKey;
	char					*m_szLastKeyName;
	float					m_fRepeatTimer;

	//------------------------------------------------------------------------------------------------- 
	float					m_fLastInput;

	//------------------------------------------------------------------------------------------------- 
	color4f				m_cToolTipColor;
	UIFont				m_pToolTipFont;
	UIBorder			m_pToolTipBorder;
	float					m_fToolTipX;
	float					m_fToolTipY;
	wstring				m_szwToolTipText;
	float					m_fToolTipOverStart;
	float					m_fToolTipAlpha;

	double				m_fVirtualToRealX;
	double				m_fVirtualToRealY;
	double				m_fRealToVirtualX;
	double				m_fRealToVirtualY;

	int						m_iReloadFrameID;
	
	//------------------------------------------------------------------------------------------------- 
	CUIScreenList	m_vScreenList;


	//------------------------------------------------------------------------------------------------- 
	// temp containers, defined as members to avoid memory allocations every frame
	CUIScreenList	m_vActiveScreenList;
	CUIWidgetList	m_vVisibleWidgetList;

	bool					m_bSortZ;
	bool					m_bSortTabStop;

	//------------------------------------------------------------------------------------------------- 	
};
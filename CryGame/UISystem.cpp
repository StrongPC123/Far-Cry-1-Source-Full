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
#include "StdAfx.h"
#include "UIWidget.h"
#include "UISystem.h"
#include "UIStatic.h"
#include "UIButton.h"
#include "UIEditBox.h"
#include "UIScrollBar.h"
#include "UIListView.h"
#include "UICheckBox.h"
#include "UIComboBox.h"
#include "UIVideoPanel.h"
#include "UIScreen.h"

#include "ScriptObjectUI.h"

#define UI_MOUSE_VISIBLE				(1 << 0)
#define UI_BACKGROUND_VISIBLE		(1 << 1)
#define UI_ENABLED							(1 << 2)

#define UI_DEFAULTS							(UI_MOUSE_VISIBLE | UI_BACKGROUND_VISIBLE | UI_ENABLED)


//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
CUISystem::CUISystem()
: m_pGame(0), m_pSystem(0), m_pScriptSystem(0), m_pRenderer(0), m_pInput(0)
{
	Reset();
}

//------------------------------------------------------------------------------------------------- 
CUISystem::~CUISystem()
{
}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
// if message processed, return 1, else return 0
LRESULT CUISystem::DefaultUpdate(CUIWidget *pWidget, unsigned int iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case UIM_MOVE:
		{
			pWidget->m_pRect.fLeft = UIM_DWORD_TO_FLOAT(wParam);
			pWidget->m_pRect.fTop = UIM_DWORD_TO_FLOAT(lParam);

			pWidget->m_pUISystem->SendMessage(pWidget, UIM_MOVED, 0, 0);

			return 1;
		}
		break;
	case UIM_SIZE:
		{
			pWidget->m_pRect.fWidth = UIM_DWORD_TO_FLOAT(wParam);
			pWidget->m_pRect.fHeight = UIM_DWORD_TO_FLOAT(lParam);

			pWidget->m_pUISystem->SendMessage(pWidget, UIM_SIZED, 0, 0);

			return 1;
		}
		break;
	case UIM_DRAW:
		{
			if (pWidget->GetFlags() & UIFLAG_VISIBLE)
			{
				int iRet = pWidget->Draw(wParam);
				
				return iRet;
			}
		}
		break;
	case UIM_DESTROY:
		{
			return pWidget->Release();
		}
		break;
	case UIM_MOUSEOVER:
		{
		}
		break;
	case UIM_LBUTTONDOWN:
		{
			if (pWidget->GetFlags() & UIFLAG_MOVEABLE)
			{
				if (pWidget->m_pUISystem->CaptureMouse(pWidget))
				{
					pWidget->m_bMoving = 1;
				}
			}
		}
		break;
	case UIM_MOUSEUP:
		{
			if (pWidget->m_bMoving)
			{
				pWidget->m_bMoving = 0;

				pWidget->m_pUISystem->ReleaseMouse();
			}
		}
		break;
	case UIM_MOUSEMOVE:
		{
			if (pWidget->m_bMoving)
			{
				float fOldX = UIM_GET_X_FLOAT(wParam);
				float fOldY = UIM_GET_Y_FLOAT(wParam);

				float fNewX = UIM_GET_X_FLOAT(lParam);
				float fNewY = UIM_GET_Y_FLOAT(lParam);

				// clamp mouse deltas, so the widget stays inside the parent rect
				if (pWidget->m_pParent)
				{
					// uncomment this to make the widget move only in the parents visible are
//					UIRect pRect;
//					pWidget->m_pUISystem->GetWidgetCanvas(&pRect, pWidget->m_pParent);

					// comment this, if you uncomented the two lines before :)
					UIRect pRect = 	pWidget->m_pParent->m_pRect;
					pWidget->m_pUISystem->GetAbsoluteXY(&pRect.fLeft, &pRect.fTop, pRect.fLeft, pRect.fTop, pWidget->m_pParent->m_pParent);

					// clamp horizontaly
					if (fNewX < pRect.fLeft)
					{
						fNewX = pRect.fLeft;

						if (fOldX < pRect.fLeft)
						{
							fOldX = pRect.fLeft;
						}
					}
					else if (fNewX > pRect.fLeft + pRect.fWidth)
					{
						fNewX = pRect.fLeft + pRect.fWidth;

						if (fOldX > pRect.fLeft + pRect.fWidth)
						{
							fOldX = pRect.fLeft + pRect.fWidth;
						}
					}

					// clamp verticaly
					if (fNewY < pRect.fTop)
					{
						fNewY = pRect.fTop;

						if (fOldY < pRect.fTop)
						{
							fOldY = pRect.fTop;
						}
					}
					else if (fNewY > pRect.fTop + pRect.fHeight)
					{
						fNewY = pRect.fTop + pRect.fHeight;

						if (fOldY > pRect.fTop + pRect.fHeight)
						{
							fOldY = pRect.fTop + pRect.fHeight;
						}
					}
				}

				// we can convert to int because the return is 0 or non-0
				return (int)pWidget->m_pUISystem->SendMessage(pWidget, UIM_MOVE, UIM_FLOAT_TO_DWORD(pWidget->m_pRect.fLeft + (fNewX - fOldX)), UIM_FLOAT_TO_DWORD(pWidget->m_pRect.fTop + (fNewY - fOldY)));
			}
		}
		break;
	default:
		{
		}
		break;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::InitializeTemplates()
{
	CUIStatic::InitializeTemplate(m_pScriptSystem);
	CUIButton::InitializeTemplate(m_pScriptSystem);
	CUIEditBox::InitializeTemplate(m_pScriptSystem);
	CUIScrollBar::InitializeTemplate(m_pScriptSystem);
	CUIListView::InitializeTemplate(m_pScriptSystem);
	CUICheckBox::InitializeTemplate(m_pScriptSystem);
	CUIComboBox::InitializeTemplate(m_pScriptSystem);
	CUIVideoPanel::InitializeTemplate(m_pScriptSystem);
	CUIScreen::InitializeTemplate(m_pScriptSystem);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ReleaseTemplates()
{
	CUIStatic::ReleaseTemplate();
	CUIButton::ReleaseTemplate();
	CUIEditBox::ReleaseTemplate();
	CUIScrollBar::ReleaseTemplate();
	CUIListView::ReleaseTemplate();
	CUICheckBox::ReleaseTemplate();
	CUIComboBox::ReleaseTemplate();
	CUIVideoPanel::ReleaseTemplate();
	CUIScreen::ReleaseTemplate();

	return 1;
}


//------------------------------------------------------------------------------------------------- 
int	CUISystem::Create(IGame *pGame, ISystem *pSystem, IScriptSystem *pScriptSystem, const string &szScriptFileName, bool bRunScriptFile)
{
	m_pGame = pGame;
	m_pSystem = pSystem;
	m_pScriptSystem = pScriptSystem;
	m_pRenderer = pSystem->GetIRenderer();
	m_pInput = m_pSystem->GetIInput();
	m_pLog = m_pSystem->GetILog();

	m_fVirtualToRealX = (double)m_pRenderer->GetWidth() / 800.0;
	m_fVirtualToRealY = (double)m_pRenderer->GetHeight() / 600.0;
	m_fRealToVirtualX = 800.0 / (double)m_pRenderer->GetWidth();
	m_fRealToVirtualY = 600.0 / (double)m_pRenderer->GetHeight();

	TRACE("rtvx: %f rtvy: %f vtrx: %f vtry: %f", m_fRealToVirtualX, m_fRealToVirtualY, m_fVirtualToRealX, m_fVirtualToRealY);

	if (m_pInput)
	{
		m_pInput->AddEventListener(this);
	}

	m_szScriptFileName = szScriptFileName;

	CreateCVars();

	m_pScriptObjectUI = new CScriptObjectUI;

	if (!m_pScriptObjectUI)
	{
		return 0;
	}
	
	m_pScriptObjectUI->Create(this);

	InitializeTemplates();

	if ((m_szScriptFileName.size()) && (bRunScriptFile))
	{
		if (!m_pScriptSystem->ExecuteFile(m_szScriptFileName.c_str(), 1, 1))
		{
			m_pLog->Log("\001$4[Error]$1: Failed to load UISystem.lua! UI System functionality compromissed!");

			return 0;
		}
		else
		{
			if (m_pScriptObjectUI)
			{
				m_pScriptObjectUI->OnInit();
			}
		}
	}

	// reset idle timer
	m_fLastInput = m_pSystem->GetITimer()->GetCurrTime();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ResetKeyRepeat()
{
	m_fRepeatTimer = 0;
	m_iLastKey = XKEY_NULL;
	m_szLastKeyName = "";

	return 1;
}

//------------------------------------------------------------------------------------------------- 
void CUISystem::Update()
{
	// hack to get a background picture if not using any video player
#if defined(NOT_USE_BINK_SDK) && defined(NOT_USE_DIVX_SDK)
	ui_BackGroundVideo->Set(0);
#endif
	
	FUNCTION_PROFILER( m_pSystem, PROFILE_GAME );

	if (m_pRenderer && m_iReloadFrameID > -1 && m_pRenderer->GetFrameID() == m_iReloadFrameID)
	{
		Reload(0);

		return;
	}

	if (m_pRenderer)
	{
		m_fVirtualToRealX = (double)m_pRenderer->GetWidth() / 800.0f;
		m_fVirtualToRealY = (double)m_pRenderer->GetHeight() / 600.0f;
		m_fRealToVirtualX = 800.0f / (double)m_pRenderer->GetWidth();
		m_fRealToVirtualY = 600.0f / (double)m_pRenderer->GetHeight();
	}
	else
	{
		m_fRealToVirtualX = 0.0f;
		m_fRealToVirtualY = 0.0f;
		m_fVirtualToRealX = 0.0f;
		m_fVirtualToRealY = 0.0f;
	}

	const float fTime = m_pSystem->GetITimer()->GetCurrTime();

	// if console is open, don't update anything
	if ((m_pSystem->GetIConsole()->IsOpened()) || !IsEnabled())
	{
		ResetKeyRepeat();

		m_vMouseXY = vector2f(-10.0f, -10.0f);

		// reset idle timer
		m_fLastInput = fTime;

		return;
	}

	//------------------------------------------------------------------------------------------------- 
	if (m_iLastKey != XKEY_NULL)
	{
		float fTime = m_pSystem->GetITimer()->GetAsyncCurTime() * 1000.0f;
		float fNextTimer = (1000.0f / (float)ui_RepeatSpeed->GetIVal()); // repeat speed

		while (fTime - m_fRepeatTimer > fNextTimer)
		{
			if (m_pFocus)
			{
				if (IsOnFocusScreen(m_pFocus))
				{
					SendMessage(m_pFocus, UIM_KEYDOWN, (WPARAM)m_szLastKeyName, m_iLastKey);	//AMD Port
					SendMessage(m_pFocus, UIM_KEYUP, (WPARAM)m_szLastKeyName, m_iLastKey);	//AMD Port
				}
			}
			else
			{
				ResetKeyRepeat();

				break;
			}

			m_fRepeatTimer += fNextTimer;
		}
	}
	//------------------------------------------------------------------------------------------------- 

	// trigger UI:OnUpdate
	if (ui_TriggerUIEvents->GetIVal() != 0)
	{
		if (m_pScriptObjectUI)
		{
			m_pScriptObjectUI->OnUpdate();
		}
	}

	m_vActiveScreenList.resize(0);

	for (CUIScreenItor pItor = m_vScreenList.begin(); pItor != m_vScreenList.end(); ++pItor)
	{
		if ((*pItor)->m_bActive)
		{
			(*pItor)->OnUpdate();

			m_vActiveScreenList.push_back(*pItor);
		}
	}

	// colect some stuff first
	// check if mouse not initialized
	vector2f vMouseXY = GetMouseXY();

	if ((m_vMouseXY.x == -10.0f) && (m_vMouseXY.y == -10.0f))
	{
		m_vMouseXY = vMouseXY;
	}

	IMouse *pMouse = m_pInput->GetIMouse();

	unsigned int	dwPackedMouseXY = UIM_PACK_COORD(vMouseXY.x, vMouseXY.y);
	unsigned int	dwPackedOldMouseXY = UIM_PACK_COORD(m_vMouseXY.x, m_vMouseXY.y);
	CUIWidget		*pMouseOver = FindWidgetAt(vMouseXY.x, vMouseXY.y);
	bool			bMouseMoved = dwPackedMouseXY != dwPackedOldMouseXY;
	bool			bLMouseDown = pMouse->MouseDown(XKEY_MOUSE1);
	bool			bRMouseDown = pMouse->MouseDown(XKEY_MOUSE2);
	bool			bMouseDown = (bLMouseDown || bRMouseDown);
	bool			bLMouseUp = pMouse->MouseReleased(XKEY_MOUSE1);
	bool			bRMouseUp = pMouse->MouseReleased(XKEY_MOUSE2);
	bool			bMouseUp = (bLMouseUp || bRMouseUp);
	bool			bLMouseDblClick = m_pInput->MouseDblClick(XKEY_MOUSE1);
	bool			bRMouseDblClick = m_pInput->MouseDblClick(XKEY_MOUSE2);
	bool			bMouseDblClick = (bLMouseDblClick || bRMouseDblClick);
	CUIWidget	*pMouseCaptured = m_pMouseCaptured;

	if (m_bLMouseDown && (pMouseOver != m_pMouseOver))
	{
		pMouseOver = 0;
	}

	//#define ISACTIVE(scr)	(vActiveScreen.find(vActiveScreen.begin(), vActiveScreen.end(), (scr)) != vActiveScreen.end())
	if (pMouseOver && pMouseOver->m_pScreen)
	{
		if (std::find(m_vActiveScreenList.begin(), m_vActiveScreenList.end(), pMouseOver->m_pScreen) == m_vActiveScreenList.end())
		{
			//assert(0);

			pMouseOver = 0;
		}
	}

	//------------------------------------------------------------------------------------------------- 
	// MOUSE EVENTS
	//------------------------------------------------------------------------------------------------- 

	// update idle timer
	if (bMouseMoved)
	{
		m_fLastInput = fTime;
	}

	if (pMouseCaptured)
	{
		// UIM_MOUSEMOVE
		if (bMouseMoved)
		{
			SendMessage(pMouseCaptured, UIM_MOUSEMOVE, dwPackedOldMouseXY, dwPackedMouseXY);
		}
		// UIM_MOUSEUP
		if (bMouseUp)
		{
			SendMessage(pMouseCaptured, UIM_MOUSEUP, dwPackedOldMouseXY, dwPackedMouseXY);
		}
		// UIM_MOUSEDOWN
		if (bMouseDown)
		{
			SendMessage(pMouseCaptured, UIM_MOUSEDOWN, dwPackedOldMouseXY, dwPackedMouseXY);
		}
	}
	else
	{
		// UIM_MOUSEMOVE
		if (bMouseMoved)
		{
			BroadcastMessage(UIM_MOUSEMOVE, dwPackedOldMouseXY, dwPackedMouseXY);
		}
		// UIM_MOUSEUP
		if (bMouseUp)
		{
			BroadcastMessage(UIM_MOUSEUP, dwPackedOldMouseXY, dwPackedMouseXY);
		}
		// UIM_MOUSEDOWN
		if (bMouseDown)
		{
			BroadcastMessage(UIM_MOUSEDOWN, dwPackedOldMouseXY, dwPackedMouseXY);
		}
	}

	if (pMouseOver && ((pMouseOver == pMouseCaptured) || (!pMouseCaptured)))
	{
		// UIM_MOUSEENTER
		if (pMouseOver != m_pMouseOver)
		{
			SendMessage(pMouseOver, UIM_MOUSEENTER, 0, 0);

			// UIM_MOUSELEAVE
			if (m_pMouseOver)
			{
				SendMessage(m_pMouseOver, UIM_MOUSELEAVE,  0, 0);
			}

			m_pMouseOver = pMouseOver;
			m_fToolTipOverStart = fTime;
			m_fToolTipAlpha = 0.0f;
		}
	}
	else if (m_pMouseOver)
	{
		// UIM_MOUSELEAVE
		SendMessage(m_pMouseOver, UIM_MOUSELEAVE, 0, 0);

		m_pMouseOver = 0;
		m_fToolTipOverStart = fTime;
		m_fToolTipAlpha = 0.0f;
	}
	
	// check if the widget is still "alive"
	// it might be dead because of a call to release, or because of a call to Reload()
	if (!WidgetExist(pMouseOver))
	{
		pMouseOver = 0;
	}

	// UIM_MOUSEOVER
	if ((pMouseOver && (pMouseOver->GetFlags() & UIFLAG_ENABLED)) && ((pMouseCaptured == pMouseOver) || !pMouseCaptured))
	{
		float fX;
		float fY;

		GetRelativeXY(&fX, &fY, vMouseXY.x, vMouseXY.y, pMouseOver);
		unsigned int dwPackedMouseRelativeXY = UIM_PACK_COORD(fX, fY);

		GetRelativeXY(&fX, &fY, m_vMouseXY.x, m_vMouseXY.y, pMouseOver);
		unsigned int dwPackedOldMouseRelativeXY = UIM_PACK_COORD(fX, fY);

		SendMessage(pMouseOver, UIM_MOUSEOVER, dwPackedOldMouseRelativeXY, dwPackedMouseRelativeXY);

		// UIM_MOUSEDOWN
		if (bMouseDown)
		{
			if (bLMouseDown)
			{
				// UIM_LBUTTONDOWN
				SendMessage(pMouseOver, UIM_LBUTTONDOWN, dwPackedMouseRelativeXY, XKEY_MOUSE1);
			}
			if (bRMouseDown)
			{
				// UIM_RBUTTONDOWN
				SendMessage(pMouseOver, UIM_RBUTTONDOWN, dwPackedMouseRelativeXY, XKEY_MOUSE2);
			}
		}

		// UIM_MOUSEUP
		if (bMouseUp)
		{
			if (bLMouseUp)
			{
				// UIM_RBUTTONUP
				SendMessage(pMouseOver, UIM_LBUTTONUP, dwPackedMouseRelativeXY, XKEY_MOUSE1);

				// UIM_LBUTTONCLICK
				SendMessage(pMouseOver, UIM_LBUTTONCLICK, dwPackedMouseRelativeXY, XKEY_MOUSE1);
			}
			if (bRMouseUp)
			{
				// UIM_RBUTTONUP
				SendMessage(pMouseOver, UIM_RBUTTONUP, dwPackedMouseRelativeXY, XKEY_MOUSE2);

				// UIM_RBUTTONCLICK
				SendMessage(pMouseOver, UIM_RBUTTONCLICK, dwPackedMouseRelativeXY, XKEY_MOUSE2);
			}
		}

		if (bMouseDblClick)
		{
			// UIM_LBUTTONDBLCLICK
			if (bLMouseDblClick)
			{
				SendMessage(pMouseOver, UIM_LBUTTONDBLCLICK, dwPackedMouseRelativeXY, XKEY_MOUSE1);
			}
			// UIM_RBUTTONDBLCLICK
			if (bRMouseDblClick)
			{
				SendMessage(pMouseOver, UIM_RBUTTONDBLCLICK, dwPackedMouseRelativeXY, XKEY_MOUSE2);
			}
		}

		// check if the widget is still "alive"
		// it might be dead because of a call to release, or because of a call to Reload()
		if (!WidgetExist(pMouseOver))
		{
			pMouseOver = 0;
		}

		// set the focus to the control if mouse button clicked
		if (bMouseDown)
		{
			// change the focus
			if ((pMouseOver) && (pMouseOver->GetFlags() & UIFLAG_CANHAVEFOCUS) &&	(std::find(m_pTabStopList.begin(), m_pTabStopList.end(), pMouseOver) != m_pTabStopList.end()))
			{
				if (IsOnFocusScreen(pMouseOver))
				{
					SetFocus(pMouseOver);
				}
			}

			// find the top-parent
			CUIWidget *pParent = pMouseOver;

			while (pParent->m_pParent)
			{
				pParent = pParent->m_pParent;
			}

			if (pParent->GetFlags() & UIFLAG_CANCHANGEZ)
			{
				SetTopMostWidget(pParent);
			}
		}
	}

	// check if the widget is still "alive"
	// it might be dead because of a call to release, or because of a call to Reload()
	if (!WidgetExist(pMouseOver))
	{
		pMouseOver = 0;
	}

	// set correct mouse cursor
	if (pMouseOver)
	{
		m_iMouseCurrentCursor = pMouseOver->m_iMouseCursor;
	}
	else
	{
		m_iMouseCurrentCursor = -1;
	}
	
	// save the current mouse postion
	m_vMouseXY = vMouseXY;
	m_bLMouseDown = bLMouseDown;

	float fIdleTime = GetIdleTime();

	if (fIdleTime > 0.0f)
	{
		m_pScriptObjectUI->OnIdle(fIdleTime);
	}

	// update tooltip
	m_szwToolTipText = L"";

	const float fToolTipFadeTime = 450.0f;

	if (ui_ToolTips->GetIVal() != 0)
	{
		if (ui_EasyToolTip->GetIVal() != 0)
		{
			m_fToolTipX = vMouseXY.x;
			m_fToolTipY = vMouseXY.y;

			if (pMouseOver && ((fTime - m_fToolTipOverStart) * 1000.0f >= ui_ToolTipDelay->GetIVal()))
			{
				float fElapsed = (fTime - (m_fToolTipOverStart + ui_ToolTipDelay->GetIVal() * 0.001f)) * 1000.0f;
				m_fToolTipAlpha = max(0.0f, min((fElapsed - fToolTipFadeTime) / fToolTipFadeTime, 1.0f));

				float fRelX, fRelY;
				GetRelativeXY(&fRelX, &fRelY, vMouseXY.x, vMouseXY.y, pMouseOver);

				pMouseOver->GetToolTip(fRelX, fRelY, m_szwToolTipText);
			}
		}
		else if (pMouseOver && ((fTime - m_fLastInput) * 1000.0f >= ui_ToolTipDelay->GetIVal()))
		{
			m_fToolTipX = vMouseXY.x;
			m_fToolTipY = vMouseXY.y;

			float fRelX, fRelY;
			GetRelativeXY(&fRelX, &fRelY, vMouseXY.x, vMouseXY.y, pMouseOver);

			float fElapsed = (fTime - (m_fLastInput + ui_ToolTipDelay->GetIVal() * 0.001f)) * 1000.0f;
			m_fToolTipAlpha = max(0.0f, min((fElapsed - fToolTipFadeTime) / fToolTipFadeTime, 1.0f));

			pMouseOver->GetToolTip(vMouseXY.x, vMouseXY.y, m_szwToolTipText);
		}
	}

	//------------------------------------------------------------------------------------------------- 
	if (ShouldSortTabStop())
	{
		SortTabStop();
	}
}

//------------------------------------------------------------------------------------------------- 
void CUISystem::Draw()
{
	FUNCTION_PROFILER( m_pSystem, PROFILE_GAME );
//	m_pRenderer->ClearDepthBuffer();

	m_pRenderer->Set2DMode(1, m_pRenderer->GetWidth(), m_pRenderer->GetHeight());
  m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);

	int iCurrentFlags = m_iFlags;

	if ((iCurrentFlags & UI_BACKGROUND_VISIBLE) && (!m_pScriptObjectUI || (m_pScriptObjectUI && m_pScriptObjectUI->OnDrawBackground())))
	{
		DrawBackground();
	}

	// sort the children by z
	if (ShouldSortByZ())
	{
		SortChildrenByZ();
	}

	// container to hold the visible widget list
	m_vVisibleWidgetList.resize(0);

	// draw first pass and gater the visible widgets
	for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
	{
		CUIWidget *pWidget = *pItor;

		if (pWidget->GetFlags() & UIFLAG_VISIBLE)
		{
			if ((!pWidget->m_pScreen) || ((pWidget->m_pScreen) && (IsScreenActive(pWidget->m_pScreen))))
			{
				SendMessage(pWidget, UIM_DRAW, 0, 0);

				m_vVisibleWidgetList.push_back(pWidget);
			}
		}
	}

	// draw next passes
	for (int i = 1; i < UI_DEFAULT_PASSES; i++)
	{
		for (CUIWidgetItor pItor = m_vVisibleWidgetList.begin(); pItor != m_vVisibleWidgetList.end(); pItor++)
		{
			CUIWidget *pWidget = *pItor;

			SendMessage(pWidget, UIM_DRAW, i, 0);
		}
	}

	// draw the mouse cursor
	if ((iCurrentFlags & UI_MOUSE_VISIBLE) && (!m_pScriptObjectUI || (m_pScriptObjectUI && m_pScriptObjectUI->OnDrawMouseCursor())))
	{
		DrawMouseCursor(m_pInput->GetIMouse()->GetVScreenX(), m_pInput->GetIMouse()->GetVScreenY());
	}

	if (ui_ToolTips->GetIVal() != 0)
	{
		DrawToolTip();
	}

	m_pRenderer->Set2DMode(0, 0, 0);
  m_pRenderer->SetState(GS_DEPTHWRITE);

	m_bSortZ = 0;
	m_bSortTabStop = 0;
}

//------------------------------------------------------------------------------------------------- 
void CUISystem::ShutDown(bool bEditorMode)
{
	Release();
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::Release()
{
	if(m_pLog)
		m_pLog->Log("\001Releasing UI System...");

	CUIScreenItor pSItor = m_vScreenList.begin();

	while (m_vScreenList.begin() != m_vScreenList.end())
	{
		DestroyScreen(*m_vScreenList.begin());
	}

	while (!m_pWidgetList.empty())
	{
		CUIWidgetItor pWItor = m_pWidgetList.begin();
		DestroyWidget(*pWItor);
	}

	if (m_pScriptObjectUI)
	{
		// trigger UI:OnRelease
		if (ui_TriggerUIEvents->GetIVal())
		{
			if (m_pScriptObjectUI)
			{
				m_pScriptObjectUI->OnRelease();
			}
		}
		
		delete m_pScriptObjectUI;
		m_pScriptObjectUI = 0;
	}

	ReleaseTemplates();
	ReleaseCVars();

	if (m_pInput)
	{
		m_pInput->RemoveEventListener(this);
	}
	
	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::Reset()
{	
	m_pScriptObjectUI = 0;
	m_pMouseCaptured = 0;
	m_pMouseOver = 0;
  m_bDrawing = 0;
	m_iMaxZ = -100000000;
	m_iMinZ = 100000000;
	m_iCurrentTabStop = 0;
	m_pFocus = 0;
	m_pFocusScreen = 0;
	m_vMouseXY = vector2f(-10.0f, -10.0f);
  m_iMouseCurrentCursor = -1;
	m_iMouseCursorTextureID = -1;
	m_fMouseCursorWidth = 16.0f;
	m_fMouseCursorHeight = 16.0f;
	m_iMouseCursorTexPixW = 0.0f;
	m_iMouseCursorTexPixH = 0.0f;
	m_cMouseCursorColor = color4f(1.0f, 1.0f, 1.0f, 1.0f);
	m_cGreyedColor = color4f(0.3f, 0.3f, 0.3f, 0.5f);
  m_iBackgroundTextureID = -1;
	m_cBackgroundColor = color4f(1.0f, 1.0f, 1.0f, 1.0f);
	m_iFlags = UI_DEFAULTS;
	m_bLMouseDown = 0;
	m_cToolTipColor = color4f(0.125f, 0.25f, 0.5f, 1.0f);
	m_fToolTipX = 0.0f;
	m_fToolTipY = 0.0f;
	m_fToolTipAlpha = 0.0f;
	m_fToolTipOverStart = 0;
	m_szwToolTipText = L"";
	m_iReloadFrameID = -1;
	
	m_pToolTipBorder.fSize = 1.0f;
	m_pToolTipBorder.iStyle = UIBORDERSTYLE_FLAT;
	m_pToolTipBorder.cColor = color4f(0.0f, 0.0f, 0.0f, 1.0f);

	m_pScissorRect = UIRect(0.0f, 0.0f, 800.0f, 600.0f);
	m_pCurrentDrawFont = 0;

	m_bSortZ = 1;
	m_bSortTabStop = 1;

	ResetKeyRepeat();

	m_pTabStopList.clear();

	m_vActiveScreenList.reserve(16);
	m_vActiveScreenList.resize(0);
	m_vVisibleWidgetList.reserve(128);
	m_vVisibleWidgetList.resize(0);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::Reload(int iFrameDelta)
{
	if (iFrameDelta > 0)
	{
		m_iReloadFrameID = m_pRenderer->GetFrameID() + iFrameDelta;

		return 1;
	}
	else
	{
		m_iReloadFrameID = -1;
	}

	m_pSystem->GetIConsole()->ShowConsole(1);

	m_pLog->LogToConsole("\001Reloading UI System...");
	m_pLog->LogToConsole("\001  Freeing currently used memory...");

	Release();
	Reset();

	m_fVirtualToRealX = (double)m_pRenderer->GetWidth() / 800.0;
	m_fVirtualToRealY = (double)m_pRenderer->GetHeight() / 600.0;
	m_fRealToVirtualX = 800.0 / (double)m_pRenderer->GetWidth();
	m_fRealToVirtualY = 600.0 / (double)m_pRenderer->GetHeight();

	TRACE("rtvx: %f rtvy: %f vtrx: %f vtry: %f", m_fRealToVirtualX, m_fRealToVirtualY, m_fVirtualToRealX, m_fVirtualToRealY);

	m_pScriptSystem->UnloadScript(m_szScriptFileName.c_str());

	m_pLog->LogToConsole("\001  Readding input listener...");
	if (m_pInput)
	{
		m_pInput->AddEventListener(this);
	}

	m_pLog->LogToConsole("\001  Reloading cvars...");
	CreateCVars();

	m_pLog->LogToConsole("\001  Creating UI script object...");
	m_pScriptObjectUI = new CScriptObjectUI;

	if (!m_pScriptObjectUI)
	{
		Release();

		return 0;
	}

	InitializeTemplates();

	m_pScriptObjectUI->Create(this);

	m_pLog->LogToConsole("\001  Reloading scripts...");
	
	if (!m_pScriptSystem->ExecuteFile(m_szScriptFileName.c_str(), 1, 1))
	{
		Release();

		return 0;
	}

	if (m_pScriptObjectUI)
	{
		if (m_pScriptObjectUI->OnInit())
		{
			m_pLog->LogToConsole("\001New UI system loaded successfuly...");
		}
	}

	// reset my idle timer
	m_fLastInput = m_pSystem->GetITimer()->GetCurrTime();

	m_pSystem->GetIConsole()->ShowConsole(0);

	return 1;
}
//------------------------------------------------------------------------------------------------- 
int CUISystem::Enable()
{
	m_iFlags |= UI_ENABLED;

	// reset the idle timer
	// i don't want idle callbacks to be called after a long time without updating
	m_fLastInput = m_pSystem->GetITimer()->GetCurrTime();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::Disable()
{
	m_iFlags &= ~UI_ENABLED;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::IsEnabled()
{
	return ((m_iFlags & UI_ENABLED) != 0);
}

//------------------------------------------------------------------------------------------------- 
IScriptObject *CUISystem::GetWidgetScriptObject(CUIWidget *pWidget)
{
	return pWidget->GetScriptObject();
}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
CUIWidgetList *CUISystem::GetWidgetList()
{
	return &m_pWidgetList;
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUISystem::GetWidget(int iIndex)
{
	return m_pWidgetList[iIndex];
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUISystem::GetWidget(const string &szName)
{
	for (CUIWidgetItor pItor = m_pWidgetList.begin(); pItor != m_pWidgetList.end(); pItor++)
	{
		if ((*pItor)->m_szName == szName && !(*pItor)->m_pScreen)
		{
			return *pItor;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUISystem::GetWidget(const string &szName, const string &szScreenName)
{
	CUIScreen *pScreen = GetScreen(szScreenName);

	if (!pScreen)
	{
		return 0;
	}

	for (CUIWidgetItor pItor = pScreen->m_vWidgetList.begin(); pItor != pScreen->m_vWidgetList.end(); ++pItor)
	{
		if ((*pItor)->m_szName == szName)
		{
			return *pItor;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int	CUISystem::GetWidgetCount()
{
	return m_pWidgetList.size();
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::WidgetExist(CUIWidget *pWidget)
{
	return (std::find(m_pWidgetList.begin(), m_pWidgetList.end(), pWidget) == m_pWidgetList.end() ? 0 : 1);
}

//------------------------------------------------------------------------------------------------- 
CUIScreenList *CUISystem::GetScreenList()
{
	return &m_vScreenList;
}

//------------------------------------------------------------------------------------------------- 
CUIScreen *CUISystem::GetScreen(int iIndex)
{
	return m_vScreenList[iIndex];
}

//------------------------------------------------------------------------------------------------- 
CUIScreen *CUISystem::GetScreen(const string &szName)
{
	for (CUIScreenItor pItor = m_vScreenList.begin(); pItor != m_vScreenList.end(); pItor++)
	{
		if ((*pItor)->m_szName == szName)
		{
			return *pItor;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int	CUISystem::GetScreenCount()
{
	return m_vScreenList.size();
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ActivateScreen(CUIScreen *pScreen)
{
	CUIScreenItor pItor = std::find(m_vScreenList.begin(), m_vScreenList.end(), pScreen);

	if (pItor != m_vScreenList.end())
	{
		if (!pScreen->m_bActive)
		{
			// remove the focus and mouseover state from all of the widgets
			for (CUIWidgetItor pWItor = pScreen->m_vWidgetList.begin(); pWItor != pScreen->m_vWidgetList.end(); ++pWItor)
			{
				(*pWItor)->OnMouseLeave();
				(*pWItor)->OnLostFocus();
			}

			// activate it, because tabstop needs it to be activated
			pScreen->m_bActive = 1;

			FirstTabStop();

			// now deactivate it again, and call the OnActivate event
			pScreen->m_bActive = 0;

			// must be done, other wise you will get the remaining input events that where created before this call
			ResetKeyRepeat();

			// call the lua eventhandler
			pScreen->OnActivate();

			// finally, really activate it
			pScreen->m_bActive = 1;

			return 1;
		}
	}
  
	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DeactivateScreen(CUIScreen *pScreen)
{
	CUIScreenItor pItor = std::find(m_vScreenList.begin(), m_vScreenList.end(), pScreen);

	if (pItor != m_vScreenList.end())
	{
		if (pScreen->m_bActive)
		{
			pScreen->m_bActive = 0;

			FirstTabStop();

			// must be done, other wise you will get the remaining input events that where created before this call
			ResetKeyRepeat();

			// remove the focus and mouseover state from all of the widgets
			for (CUIWidgetItor pWItor = pScreen->m_vWidgetList.begin(); pWItor != pScreen->m_vWidgetList.end(); ++pWItor)
			{
				(*pWItor)->OnMouseLeave();
				(*pWItor)->OnLostFocus();
			}

			// call the lua eventhandler
			pScreen->OnDeactivate();

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::IsScreenActive(CUIScreen *pScreen)
{
	return pScreen->m_bActive;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetActiveScreenCount()
{
	int iCount = 0;
	for (CUIScreenItor pItor = m_vScreenList.begin(); pItor != m_vScreenList.end(); ++pItor)
	{
		if ((*pItor)->m_bActive)
		{
			++iCount;
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DeactivateAllScreens()
{
	for (CUIScreenItor pItor = m_vScreenList.begin(); pItor != m_vScreenList.end(); ++pItor)
	{
		if ((*pItor)->m_bActive)
		{
			(*pItor)->m_bActive = 0;
			(*pItor)->OnDeactivate();
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ActivateAllScreens()
{
	// i need to call the Activate method, because i want it to call the OnActivate method
	for (CUIScreenItor pItor = m_vScreenList.begin(); pItor != m_vScreenList.end(); ++pItor)
	{
		ActivateScreen(*pItor);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
CUIWidgetList *CUISystem::GetChildList()
{
	return &m_pChildList;
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUISystem::GetChild(int iIndex)
{
	return m_pChildList[iIndex];
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUISystem::GetChild(const string &szName)
{
	for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
	{
		if ((*pItor)->m_szName == szName)
		{
			return *pItor;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetChildCount()
{
	return m_pChildList.size();
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::AddChild(CUIWidget *pWidget)
{
	for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
	{
		if (*pItor == pWidget)
		{
			return 0;
		}
	}

	m_pChildList.push_back(pWidget);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DelChild(CUIWidget *pWidget)
{
	for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
	{
		if (*pItor == pWidget)
		{
			m_pChildList.erase(pItor);

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DelChild(int iIndex)
{
	return DelChild(m_pChildList[iIndex]);
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DelChild(const string &szName)
{
	for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
	{
		if ((*pItor)->m_szName == szName)
		{
			m_pChildList.erase(pItor);

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
int CUISystem::SetBackground(int iBackgroundTexture)
{
	m_iBackgroundTextureID = iBackgroundTexture;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetBackground()
{
	return m_iBackgroundTextureID;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetBackgroundColor(const color4f &cColor)
{
	m_cBackgroundColor = cColor;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetBackgroundColor(color4f *pColor)
{
	*pColor = m_cBackgroundColor;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ShowBackground()
{
	m_iFlags |= UI_BACKGROUND_VISIBLE;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::HideBackground()
{
	m_iFlags &= ~UI_BACKGROUND_VISIBLE;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
bool CUISystem::IsBackgroundVisible()
{
	return ((m_iFlags & UI_BACKGROUND_VISIBLE) != 0);
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetGreyedColor(const color4f &cColor)
{
	m_cGreyedColor = cColor;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetGreyedColor(color4f *cColor)
{
	*cColor = m_cGreyedColor;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetMouseCursor(int iTextureID)
{
	m_iMouseCursorTextureID = iTextureID;

	m_iMouseCursorTexPixW = 0.0f;
	m_iMouseCursorTexPixH = 0.0f;

	if (iTextureID != -1)
	{
		ITexPic *pTexPic = m_pRenderer->EF_GetTextureByID(m_iMouseCursorTextureID);

		if (pTexPic)
		{
			m_iMouseCursorTexPixW = 1.0f / pTexPic->GetWidth();
			m_iMouseCursorTexPixH = 1.0f / pTexPic->GetHeight();
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetMouseCursor()
{
	return m_iMouseCursorTextureID;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetMouseCursorColor(const color4f &cColor)
{
	m_cMouseCursorColor = cColor;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetMouseCursorColor(color4f *pColor)
{
	*pColor = m_cMouseCursorColor;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetMouseCursorSize(float fWidth, float fHeight)
{
	m_fMouseCursorWidth = fWidth;
	m_fMouseCursorHeight = fHeight;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetMouseCursorSize(float *fWidth, float *fHeight)
{
	*fWidth = m_fMouseCursorWidth;
	*fHeight = m_fMouseCursorHeight;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ShowMouseCursor()
{
	m_iFlags |= UI_MOUSE_VISIBLE;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::HideMouseCursor()
{
	m_iFlags &= ~UI_MOUSE_VISIBLE;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
bool CUISystem::IsMouseCursorVisible()
{
	return ((m_iFlags & UI_MOUSE_VISIBLE) != 0);
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetMouseXY(float fX, float fY)
{
	m_pInput->GetIMouse()->SetVScreenX(fX);
	m_pInput->GetIMouse()->SetVScreenY(fY);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
vector2f CUISystem::GetMouseXY()
{
	return vector2f(m_pInput->MouseGetVScreenX(), m_pInput->MouseGetVScreenY());
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CaptureMouse(CUIWidget *pWidget)
{
	if ((m_pMouseCaptured) && (m_pMouseCaptured != pWidget))
	{
		return 0;
	}

	m_pMouseCaptured = pWidget;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ReleaseMouse()
{
	m_pMouseCaptured = 0;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
float CUISystem::GetIdleTime()
{
	return max((m_pSystem->GetITimer()->GetCurrTime() - m_fLastInput) - UI_DEFAULT_IDLETIME_START, 0.0f);
}

//------------------------------------------------------------------------------------------------- 
LRESULT CUISystem::SendMessage(string &szName, const string &szScreenName, int iMessage, WPARAM wParam, LPARAM lParam)
{
	CUIWidget *pWidget = GetWidget(szName, szScreenName);

	if (!pWidget)
	{
		return 0;
	}

	return SendMessage(pWidget, iMessage, wParam, lParam);
}

//------------------------------------------------------------------------------------------------- 
LRESULT CUISystem::SendMessage(CUIWidget *pWidget, int iMessage, WPARAM wParam, LPARAM lParam)
{
	// check if the widget is still alive
	if (!WidgetExist(pWidget))
	{
		return 0;
	}

	// call the script functions
	switch (iMessage)
	{
	case UIM_KEYDOWN:
		if (!pWidget->OnKeyDown(lParam))
		{
			return 0;
		}
		break;
	case UIM_KEYUP:
		if (!pWidget->OnKeyUp(lParam))
		{
			return 0;
		}
		break;
	case UIM_KEYPRESSED:
		if (!pWidget->OnKeyPressed(lParam))
		{
			return 0;
		}
		break;
	case UIM_MOUSEENTER:
		if (!pWidget->OnMouseEnter())
		{
			return 0;
		}
		break;
	case UIM_MOUSELEAVE:
		if (!pWidget->OnMouseLeave())
		{
			return 0;
		}
		break;
	case UIM_MOUSEDOWN:
		if (!pWidget->OnMouseDown(lParam, UIM_GET_X_FLOAT(wParam), UIM_GET_Y_FLOAT(wParam)))
		{
			return 0;
		}
		break;
	case UIM_MOUSEUP:
		if (!pWidget->OnMouseUp(lParam, UIM_GET_X_FLOAT(wParam), UIM_GET_Y_FLOAT(wParam)))
		{
			return 0;
		}
		break;
	case UIM_LBUTTONCLICK:
	case UIM_RBUTTONCLICK:
		if (!pWidget->OnMouseClick(lParam, UIM_GET_X_FLOAT(wParam), UIM_GET_Y_FLOAT(wParam)))
		{
			return 0;
		}
		break;
	case UIM_LBUTTONDBLCLICK:
	case UIM_RBUTTONDBLCLICK:
		if (!pWidget->OnMouseDblClick(lParam, UIM_GET_X_FLOAT(wParam), UIM_GET_Y_FLOAT(wParam)))
		{
			return 0;
		}
		break;
	case UIM_MOUSEOVER:
		if (!pWidget->OnMouseOver(UIM_GET_X_FLOAT(lParam), UIM_GET_Y_FLOAT(lParam), UIM_GET_X_FLOAT(wParam), UIM_GET_X_FLOAT(wParam)))
		{
			return 0;
		}
		break;
	case UIM_MOUSEMOVE:
		if (!pWidget->OnMouseMove(UIM_GET_X_FLOAT(lParam), UIM_GET_Y_FLOAT(lParam), UIM_GET_X_FLOAT(wParam), UIM_GET_X_FLOAT(wParam)))
		{
			return 0;
		}
		break;
	case UIM_GOTFOCUS:
		if (!pWidget->OnGotFocus())
		{
			return 0;
		}
		break;
	case UIM_LOSTFOCUS:
		if (!pWidget->OnLostFocus())
		{
			return 0;
		}
		break;
	case UIM_SIZED:
		if (!pWidget->OnSized())
		{
			return 0;
		}
		break;
	case UIM_MOVED:
		if (!pWidget->OnMoved())
		{
			return 0;
		}
		break;
	case UIM_CHANGED:
		if (!pWidget->OnChanged())
		{
			return 0;
		}
		break;
	case UIM_COMMAND:
		if (!pWidget->OnCommand())
		{
			return 0;
		}
		break;

	}

	return pWidget->Update(iMessage, wParam, lParam);
}

//------------------------------------------------------------------------------------------------- 
LRESULT CUISystem::BroadcastMessage(int iMessage, WPARAM wParam, LPARAM lParam)
{
	CUIWidget *pWidget;

	for (CUIWidgetItor pItor = m_pWidgetList.begin(); pItor != m_pWidgetList.end(); ++pItor)
	{
		pWidget = (*pItor);
		
		if ((!pWidget->m_pScreen) || (IsScreenActive(pWidget->m_pScreen)))
		{
			SendMessage(pWidget, iMessage, wParam, lParam);
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUISystem::GetWidgetParent(CUIWidget *pWidget)
{
	return pWidget->m_pParent;
}

//------------------------------------------------------------------------------------------------- 
wstring CUISystem::GetWidgetText(CUIWidget *pWidget)
{
	if (pWidget->GetClassName() == UICLASSNAME_STATIC)
	{
		return ((CUIStatic *)pWidget)->m_vLines[0].szText;
	}
	else if (pWidget->GetClassName() == UICLASSNAME_BUTTON)
	{
		return ((CUIButton *)pWidget)->m_szText;
	}
	else if (pWidget->GetClassName() == UICLASSNAME_EDITBOX)
	{
		return ((CUIEditBox *)pWidget)->m_szText;
	}
	else
	{
		return L"";
	}
}

//------------------------------------------------------------------------------------------------- 
UIRect &CUISystem::GetWidgetRect(CUIWidget *pWidget)
{
	return pWidget->m_pRect;
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUISystem::GetTabStop(int iTabStop)
{
	assert(iTabStop >= 0 && iTabStop < (int)m_pTabStopList.size());

	return m_pTabStopList[iTabStop];
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::FirstTabStop()
{
	if (!m_pTabStopList.size())
	{
		return 0;
	}

	CUIWidget *pWidget = 0;

	pWidget = m_pTabStopList[m_iCurrentTabStop = 0];

	if ((pWidget->GetFlags() & UIFLAG_ENABLED) && (pWidget->GetFlags() & UIFLAG_VISIBLE) && (pWidget->GetFlags() & UIFLAG_CANHAVEFOCUS))
	{
		if (IsOnFocusScreen(pWidget))
		{
			SetFocus(pWidget);		

			return 1;
		}
	}

	return NextTabStop();
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::NextTabStop()
{
	if (!m_pTabStopList.size())
	{
		return 0;
	}

	int iStart = m_iCurrentTabStop++;
	CUIWidget *pWidget;

	if (m_iCurrentTabStop > (int)m_pTabStopList.size()-1)
	{
		m_iCurrentTabStop = 0;
	}

	while (m_iCurrentTabStop != iStart)
	{	
		pWidget = m_pTabStopList[m_iCurrentTabStop];

		if ((pWidget->GetFlags() & UIFLAG_ENABLED) && (pWidget->GetFlags() & UIFLAG_VISIBLE) && (pWidget->GetFlags() & UIFLAG_CANHAVEFOCUS))
		{
			if (IsOnFocusScreen(pWidget))
			{
				SetFocus(pWidget);

				return 1;
			}
		}

		if (++m_iCurrentTabStop > (int)m_pTabStopList.size()-1)
		{
			m_iCurrentTabStop = 0;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::PrevTabStop()
{
	if (!m_pTabStopList.size())
	{
		return 0;
	}

	int iStart = m_iCurrentTabStop--;
	CUIWidget *pWidget;

	if (m_iCurrentTabStop < 0)
	{
		m_iCurrentTabStop = m_pTabStopList.size()-1;
	}

	while (m_iCurrentTabStop != iStart)
	{	
		pWidget = m_pTabStopList[m_iCurrentTabStop];

		if ((pWidget->GetFlags() & UIFLAG_ENABLED) && (pWidget->GetFlags() & UIFLAG_VISIBLE) && (pWidget->GetFlags() & UIFLAG_CANHAVEFOCUS))
		{
			if (IsOnFocusScreen(pWidget))
			{
				SetFocus(pWidget);

				return 1;
			}
		}

		if (--m_iCurrentTabStop <= -1)
		{
			m_iCurrentTabStop = m_pTabStopList.size()-1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::LastTabStop()
{
	if (!m_pTabStopList.size())
	{
		return 0;
	}

	CUIWidget *pWidget = 0;

	pWidget = m_pTabStopList[m_iCurrentTabStop = m_pTabStopList.size()-1];

	if ((pWidget->GetFlags() & UIFLAG_ENABLED) && (pWidget->GetFlags() & UIFLAG_VISIBLE) && (pWidget->GetFlags() & UIFLAG_CANHAVEFOCUS))
	{
		if (IsOnFocusScreen(pWidget))
		{
			SetFocus(pWidget);

			return 1;
		}
	}

	return PrevTabStop();
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetTopMostWidget(const string &szName)
{
	CUIWidget *pWidget = GetChild(szName);

	if (!pWidget)
	{
		return 0;
	}

	return SetTopMostWidget(pWidget);
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetTopMostWidget(CUIWidget *pWidget)
{
	if (GetChildCount() < 2)
	{
		return 1;
	}

	pWidget->SetZ(++m_iMaxZ);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUISystem::GetTopMostWidget()
{
	return *(m_pChildList.end()-1);
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetFocus(CUIWidget *pWidget)
{
	if (m_pFocus == pWidget)
	{
		return 1;
	}

	CUIWidget *pLostFocus = m_pFocus;
	CUIWidget *pGotFocus = pWidget;

	if (pLostFocus)
	{
		pLostFocus->m_iFlags &= ~UIFLAG_HAVEFOCUS;
	}

	if (pGotFocus)
	{
		pGotFocus->m_iFlags |= UIFLAG_HAVEFOCUS;
	}

	m_pFocus = pGotFocus;

	if (pLostFocus)
	{
		SendMessage(pLostFocus, UIM_LOSTFOCUS, 0, 0);
	}

	if (pGotFocus)
	{
		SendMessage(pGotFocus, UIM_GOTFOCUS, 0, 0);

		m_iCurrentTabStop = 0;

		for (int i = 0; i <  (int)m_pTabStopList.size(); i++)
		{
			if (pGotFocus == m_pTabStopList[i])
			{
				m_iCurrentTabStop = i;

				break;
			}
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetFocus(string &szName)
{
	CUIWidget *pWidget = GetWidget(szName);

	SetFocus(pWidget);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetFocus(string &szName, string &szScreenName)
{
	CUIWidget *pWidget = GetWidget(szName, szScreenName);

	SetFocus(pWidget);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUISystem::GetFocus()
{
	return m_pFocus;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetFocusScreen(CUIScreen *pScreen)
{
	m_pFocusScreen = pScreen;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetFocusScreen(string &szScreenName)
{
	m_pFocusScreen = GetScreen(szScreenName);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
CUIScreen *CUISystem::GetFocusScreen()
{
	return m_pFocusScreen;
}

//------------------------------------------------------------------------------------------------- 
color4f CUISystem::GetSelectionColor(const color4f &cBackground, const color4f &cTextcolor)
{
	color4f cColor = (color4f(1.0f, 1.0f, 1.0f, 1.0f) - 0.5f * cBackground + 0.5f * cTextcolor);

	cColor.v[3] = 0.8f;

	return cColor;
}

//------------------------------------------------------------------------------------------------- 
IFFont *CUISystem::GetIFont(const UIFont &pFont)
{
	IFFont *pIFont = m_pSystem->GetICryFont()->GetFont(pFont.szFaceName.c_str());

	if (!pIFont)
	{
		pIFont = m_pSystem->GetICryFont()->GetFont("Default");
	}

	if (!pIFont)
	{
		return 0;
	}

	pIFont->Reset();

	pIFont->UseRealPixels(0);
	pIFont->SetEffect(pFont.szEffectName.c_str());
	pIFont->SetColor(pFont.cColor);
	pIFont->SetSize(vector2f(pFont.fSize, pFont.fSize));

	return pIFont;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetAlignedTextXY(float *fNewX, float *fNewY, IFFont *pFont, const UIRect &pTextRect, const wchar_t *szString, int iAlignmentX, int iAlignmentY)
{
	vector2f vTextSize = pFont->GetTextSizeW(szString);

	vTextSize.x = (float)(vTextSize.x * m_fRealToVirtualX);
	vTextSize.y = (float)(vTextSize.y * m_fRealToVirtualY);

	// get horizontal alignment
	switch (iAlignmentX)
	{
	case UIALIGN_LEFT:

		*fNewX = pTextRect.fLeft;

		break;

	case UIALIGN_RIGHT:

		*fNewX = pTextRect.fLeft + pTextRect.fWidth - vTextSize.x;

		break;

	default:

		*fNewX = (pTextRect.fLeft + pTextRect.fWidth * 0.5f) - (vTextSize.x * 0.5f);
	}

	//----------------------
	// get vertial alignment
	switch (iAlignmentY)
	{
		case UIALIGN_TOP:

			*fNewY = pTextRect.fTop;

			break;

		case UIALIGN_BOTTOM:

			*fNewY = (pTextRect.fTop + pTextRect.fHeight) - vTextSize.y;

			break;

		default:

			*fNewY = (pTextRect.fTop + pTextRect.fHeight * 0.5f) - (vTextSize.y * 0.5f);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetAbsoluteXY(float *fNewX, float *fNewY, float fRelativeX, float fRelativeY, CUIWidget *pWidget)
{
	*fNewX = fRelativeX;
	*fNewY = fRelativeY;

	if (pWidget)
	{
		CUIWidget *pParent = pWidget;

		while (pParent)
		{
			*fNewX += pParent->m_pRect.fLeft;
			*fNewY += pParent->m_pRect.fTop;

			pParent = pParent->m_pParent;
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetRelativeXY(float *fNewX, float *fNewY, float fAbsoluteX, float fAbsoluteY, CUIWidget *pWidget)
{
	GetAbsoluteXY(fNewX, fNewY, 0, 0, pWidget);

	*fNewX = fAbsoluteX - *fNewX;
	*fNewY = fAbsoluteY - *fNewY;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::IntersectRect(UIRect *pNewRect, const UIRect pRect1, const UIRect pRect2)
{
	// do some checks first
	if ((pRect2.fLeft > pRect1.fLeft + pRect1.fWidth) || (pRect1.fLeft > pRect2.fLeft + pRect2.fWidth) || (pRect2.fTop > pRect1.fTop + pRect1.fHeight) || (pRect1.fTop > pRect2.fTop + pRect2.fHeight))
	{
		pNewRect->fLeft = pRect1.fLeft;
		pNewRect->fTop = pRect1.fTop;
		pNewRect->fWidth = 0;
		pNewRect->fHeight = 0;

		return 0;
	}

	if (pRect2.fLeft > pRect1.fLeft)
	{
		pNewRect->fLeft = pRect2.fLeft;
	}
	else
	{
		pNewRect->fLeft = pRect1.fLeft;
	}

	if (pRect2.fLeft + pRect2.fWidth > pRect1.fLeft + pRect1.fWidth)
	{
		pNewRect->fWidth = pRect1.fLeft + pRect1.fWidth - pNewRect->fLeft;
	}
	else
	{
		pNewRect->fWidth = pRect2.fLeft + pRect2.fWidth - pNewRect->fLeft;
	}

	if (pRect2.fTop > pRect1.fTop)
	{
		pNewRect->fTop = pRect2.fTop;
	}
	else
	{
		pNewRect->fTop = pRect1.fTop;
	}

	if (pRect2.fTop + pRect2.fHeight > pRect1.fTop + pRect1.fHeight)
	{
		pNewRect->fHeight = pRect1.fTop + pRect1.fHeight - pNewRect->fTop;
	}
	else
	{
		pNewRect->fHeight = pRect2.fTop + pRect2.fHeight - pNewRect->fTop;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::AdjustRect(UIRect *pNewRect, const UIRect pRect, float fBorderSize, bool bGrow)
{
	float fSizeX = AdjustWidth(fBorderSize);
	float fSizeY = AdjustHeight(fBorderSize);

	if (bGrow)
	{
		pNewRect->fLeft = pRect.fLeft - fSizeX;
		pNewRect->fTop = pRect.fTop - fSizeY;
		pNewRect->fWidth = pRect.fWidth + fSizeX + fSizeX;
		pNewRect->fHeight = pRect.fHeight + fSizeY + fSizeY;

		return 1;
	}

	pNewRect->fLeft = pRect.fLeft + fSizeX;
	pNewRect->fTop = pRect.fTop + fSizeY;
	pNewRect->fWidth = pRect.fWidth - fSizeX - fSizeX;
	pNewRect->fHeight = pRect.fHeight - fSizeY - fSizeY;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
bool CUISystem::PointInRect(const UIRect &pRect, float fX, float fY)
{
	return (((fX >= pRect.fLeft) && (fX <= pRect.fLeft + pRect.fWidth)) && ((fY >= pRect.fTop) && (fY <= pRect.fTop + pRect.fHeight)));
}

//------------------------------------------------------------------------------------------------- 
CUIWidget *CUISystem::FindWidgetAt(float fX, float fY)
{
	UIRect						pRect;
	CUIWidget					*pWidget;
	CUIWidget					*pTopMost = 0;
	CUIWidget					*pTopMostChild = 0;
	int								iMaxZ = -1000000000;
//	std::vector<CUIWidget *>	pMouseOverList;
	
	// first go through every child
	CUIWidgetItor pItor;

	for (pItor = m_pChildList.begin(); pItor != m_pChildList.end(); ++pItor)
	{
		pWidget = *pItor;

		if (
			((pWidget->m_iFlags & UIFLAG_VISIBLE) == 0) ||
			((pWidget->m_iFlags & UIFLAG_ENABLED) == 0) ||
			(pWidget->m_pScreen && (!IsScreenActive(pWidget->m_pScreen))))
		{
			continue;
		}

		if (PointInRect(pWidget->m_pRect, fX, fY))
		{
			if (pWidget->GetZ() > iMaxZ)
			{
				iMaxZ = pWidget->GetZ();
				pTopMost = pWidget;
			}
		}
	}

	if (!pTopMost)
	{
		return 0;
	}

	iMaxZ = -1;

	while(1)
	{
		for (pItor = pTopMost->m_pChildList.begin(); pItor != pTopMost->m_pChildList.end(); ++pItor)
		{
			pWidget = *pItor;

			if (((pWidget->m_iFlags & UIFLAG_VISIBLE) == 0) ||	((pWidget->m_iFlags & UIFLAG_ENABLED) == 0))
			{
				continue;
			}

			GetWidgetCanvas(&pRect, pWidget);

			if (PointInRect(pRect, fX, fY))
			{
				iMaxZ = pWidget->GetZ();
				pTopMostChild = pWidget;
			}
		}

		if (!pTopMostChild)
		{
			break;
		}

		pTopMost = pTopMostChild;
		pTopMostChild = 0;
	}

	if (pTopMostChild)
	{
		return pTopMostChild;
	}
	else
	{
		return pTopMost;
	}
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::BeginDraw(CUIWidget *pWidget)
{
	if (m_bDrawing)
	{
		return 0;
	}

	if (pWidget)
	{
		GetWidgetCanvas(&m_pCurrentDrawRect, pWidget);

		// set clipping rect
		SetScissor(&m_pCurrentDrawRect);

		// set font clipping
		m_pCurrentDrawFont = GetIFont(pWidget->m_pFont);
	}

	m_bDrawing = 1;

	ResetDraw();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ResetDraw()
{
	if (m_bDrawing)
	{
    m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);

		m_bDrawing = 0;

		SetScissor(&m_pCurrentDrawRect);

		m_bDrawing = 1;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::EndDraw()
{
  m_pRenderer->SetState(GS_DEPTHWRITE);

	if (m_bDrawing)
	{
		m_bDrawing = 0;

		m_pScissorRect = UIRect(0.0f, 0.0f, 800.0f, 600.0f);
		m_pCurrentDrawFont->EnableClipping(0);
		m_pCurrentDrawFont = 0;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
float CUISystem::AdjustWidth(float fBorderW)
{
	if ((fBorderW >= 1.0f) && (m_fVirtualToRealX * fBorderW < 1.0f))
	{
		return (float)(ceil(m_fVirtualToRealX * fBorderW) * m_fRealToVirtualX);
	}
	else
	{
		return (float)(floor(m_fVirtualToRealX * fBorderW) * m_fRealToVirtualX);
	}
}

//------------------------------------------------------------------------------------------------- 
float CUISystem::AdjustHeight(float fBorderH)
{
	if ((fBorderH >= 1.0f) && (m_fVirtualToRealY * fBorderH < 1.0f))
	{
		return (float)(ceil(m_fVirtualToRealY * fBorderH) * m_fRealToVirtualY);
	}
	else
	{
		return (float)(floor(m_fVirtualToRealY * fBorderH) * m_fRealToVirtualY);
	}
};

//------------------------------------------------------------------------------------------------- 
int CUISystem::GetWidgetCanvas(UIRect *pWidgetCanvas, CUIWidget *pWidget)
{
	if (pWidget->m_pParent)
	{
		UIRect pCurrentRect;
		UIRect pChildRect;
		UIRect pParentRect;


		CUIWidget *pParent = pWidget->m_pParent;

		pChildRect = pWidget->m_pRect;

		GetAbsoluteXY(&pChildRect.fLeft, &pChildRect.fTop, pChildRect.fLeft, pChildRect.fTop, pParent);

		while (pParent)
		{
			pParentRect = pParent->m_pRect;

			if (pParent->m_pParent)
			{
				GetAbsoluteXY(&pParentRect.fLeft, &pParentRect.fTop, pParentRect.fLeft, pParentRect.fTop, pParent->m_pParent);
			}

			IntersectRect(&pCurrentRect, pChildRect, pParentRect);

			pChildRect = pCurrentRect;

			pParent = pParent->m_pParent;
		}

		*pWidgetCanvas = pCurrentRect;
	}
	else
	{
		*pWidgetCanvas = pWidget->m_pRect;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ShowWidget(CUIWidget *pWidget)
{
	pWidget->m_iFlags |= UIFLAG_VISIBLE;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::HideWidget(CUIWidget *pWidget)
{
	pWidget->m_iFlags &= ~UIFLAG_VISIBLE;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::IsWidgetVisible(CUIWidget *pWidget)
{
	return (pWidget->m_iFlags & UIFLAG_VISIBLE);
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::EnableWidget(CUIWidget *pWidget)
{
	pWidget->m_iFlags |= UIFLAG_ENABLED;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DisableWidget(CUIWidget *pWidget)
{
	pWidget->m_iFlags &= ~UIFLAG_ENABLED;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::IsWidgetEnabled(CUIWidget *pWidget)
{
	return (pWidget->m_iFlags & UIFLAG_ENABLED);
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetScissor(const UIRect *pRect)
{
	if (!pRect)
	{
		if (m_bDrawing)
		{
			float x = (float)floor(m_fVirtualToRealX * m_pCurrentDrawRect.fLeft);
			float y = (float)floor(m_fVirtualToRealY * m_pCurrentDrawRect.fTop);
			m_pCurrentDrawFont->EnableClipping(1);
			m_pCurrentDrawFont->SetClippingRect(x, y,
				x + (float)floor(m_fVirtualToRealX * m_pCurrentDrawRect.fWidth),
				y + (float)floor(m_fVirtualToRealY * m_pCurrentDrawRect.fHeight)
				);
		}
		else
		{
			m_pCurrentDrawFont->EnableClipping(0);
		}

		return 1;
	}

	UIRect pNewRect(*pRect);

	if (m_bDrawing)
	{
		IntersectRect(&pNewRect, *pRect, m_pCurrentDrawRect);
	}

	m_pScissorRect = pNewRect;

	if (m_pCurrentDrawFont)
	{
		float x = (float)floor(m_fVirtualToRealX * m_pScissorRect.fLeft);
		float y = (float)floor(m_fVirtualToRealY * m_pScissorRect.fTop);

		m_pCurrentDrawFont->EnableClipping(1);
		m_pCurrentDrawFont->SetClippingRect(x, y,
			x + (float)floor(m_fVirtualToRealX * m_pScissorRect.fWidth),
			y + (float)floor(m_fVirtualToRealY * m_pScissorRect.fHeight)
		);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawBackground()
{
	//m_pRenderer->SetScissor();

	DrawImage(UIRect(0, 0, 800, 600), m_iBackgroundTextureID, m_cBackgroundColor);

	return 1;
}

int CUISystem::DrawMouseCursor(float fLeft, float fTop)
{
  m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);

	float vTexCoord[4] = 
	{
		m_iMouseCursorTexPixW * 0.5f,
		1.0f - m_iMouseCursorTexPixH,
		1.0f - m_iMouseCursorTexPixW,
		m_iMouseCursorTexPixH * 0.5f,
	};

	if (m_iMouseCurrentCursor > -1)
	{
		DrawImage(UIRect(fLeft, fTop, m_fMouseCursorWidth, m_fMouseCursorHeight), m_iMouseCurrentCursor, vTexCoord, m_cMouseCursorColor);
	}
	else
	{
		DrawImage(UIRect(fLeft, fTop, m_fMouseCursorWidth, m_fMouseCursorHeight), m_iMouseCursorTextureID, vTexCoord, m_cMouseCursorColor);
	}
	
	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawBorder(const UIRect &pRect, const UIBorder &pBorder)
{
	if (pBorder.fSize < 0.125f)
	{
		return 1;
	}

	if (pBorder.cColor.v[3] < 0.0125f)
	{
		return 1;
	}

	float fSizeX = AdjustWidth(pBorder.fSize);
	float fSizeY = AdjustHeight(pBorder.fSize);

	switch (pBorder.iStyle)
	{
	case UIBORDERSTYLE_RAISED:
	case UIBORDERSTYLE_SUNKEN:
		
		DrawEmboss(pRect, GET_HIGHLIGHT_COLOR(pBorder.cColor), GET_SHADOWED_COLOR(pBorder.cColor), pBorder.iStyle == UIBORDERSTYLE_SUNKEN, pBorder.fSize);
		break;

	case UIBORDERSTYLE_FLAT:
		{
			float fLeft = pRect.fLeft;
			float fWidth = pRect.fWidth;
			float fTop = pRect.fTop;
			float fHeight = pRect.fHeight;

			if ((pBorder.iFlags & UIBORDERSIDE_TOP) == 0)
			{
				fTop += fSizeY;
				fHeight -= fSizeY;
			}

			if ((pBorder.iFlags & UIBORDERSIDE_BOTTOM) == 0)
			{
				fHeight -= fSizeY;
			}

			// draw left
			if (pBorder.iFlags & UIBORDERSIDE_LEFT)
			{
				DrawQuad(UIRect(fLeft, fTop, fSizeX, fHeight), pBorder.cColor);
			}
			// draw right
			if (pBorder.iFlags & UIBORDERSIDE_RIGHT)
			{
				DrawQuad(UIRect(fLeft + fWidth - fSizeX, fTop, fSizeX, fHeight), pBorder.cColor);
			}

			if ((pBorder.iFlags & UIBORDERSIDE_LEFT) == 0)
			{
				fLeft += fSizeX;
				fWidth -= fSizeX;
			}

			if ((pBorder.iFlags & UIBORDERSIDE_RIGHT) == 0)
			{
				fWidth -= fSizeX;
			}

			// draw top
			if (pBorder.iFlags & UIBORDERSIDE_TOP)
			{
				DrawQuad(UIRect(fLeft, fTop, fWidth, fSizeY), pBorder.cColor);
			}
			// draw bottom
			if (pBorder.iFlags & UIBORDERSIDE_BOTTOM)
			{
				DrawQuad(UIRect(fLeft, fTop + fHeight - fSizeY, fWidth, fSizeY), pBorder.cColor);
			}
		}
		break;

	default:
		break;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawQuad(const UIRect &pRect, const color4f &cColor)
{
	color4f cClampColor;

	cClampColor.v[0] = min(max(cColor.v[0], 0.0f), 1.0f);
	cClampColor.v[1] = min(max(cColor.v[1], 0.0f), 1.0f);
	cClampColor.v[2] = min(max(cColor.v[2], 0.0f), 1.0f);
	cClampColor.v[3] = min(max(cColor.v[3], 0.0f), 1.0f);

	DrawImage(pRect, -1, 0, cClampColor);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawGreyedQuad(const UIRect &pRect, const color4f &cColor, int iMode)
{
	if (iMode == UIBLEND_ADDITIVE)
	{
		m_pRenderer->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
	}
	else
	{
		m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
	}
	
	DrawImage(pRect, -1, 0, cColor);

	m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawText(const UIRect &pRect, int iHAlignment, int iVAlignment, IFFont *pFont, const wchar_t *szText, bool bTranslateEscapes)
{
	float fNewX, fNewY;

	GetAlignedTextXY(&fNewX, &fNewY, pFont, pRect, szText, iHAlignment, iVAlignment);

	pFont->DrawStringW(fNewX, fNewY, szText, bTranslateEscapes);

  m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawImage(const UIRect &pRect, const UISkinTexture &pTexture, const color4f &cColor)
{
	DrawImage(pRect, pTexture.iTextureID, pTexture.vTexCoord, cColor);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawImage(const UIRect &pRect, int iTextureID, const float *vTexCoord, const color4f &cColor)
{
	if ((pRect.fWidth <= 0.0f) || (pRect.fHeight <= 0.0f))
	{
		return 0;
	}

	float fX = pRect.fLeft;
	float fY = pRect.fTop;
	float fR = pRect.fLeft + pRect.fWidth;
	float fB = pRect.fTop + pRect.fHeight;

	// clip the image to the scissor rect
	fX = max(m_pScissorRect.fLeft, fX);
	fY = max(m_pScissorRect.fTop, fY);
	fR = min(m_pScissorRect.fLeft + m_pScissorRect.fWidth, fR);
	fB = min(m_pScissorRect.fTop + m_pScissorRect.fHeight, fB);

	// fix texture coordinates
	if (iTextureID > -1)
	{
		float fTexW = 1.0f;
		float fTexH = 1.0f;
		float fRcpWidth = 1.0f / pRect.fWidth;
		float fRcpHeight = 1.0f / pRect.fHeight;

		float vClippedTexCoord[4] = { 0.0f, 1.0f, 1.0f, 0.0f };

		if (vTexCoord)
		{
			fTexW = vTexCoord[2] - vTexCoord[0];
			fTexH = vTexCoord[3] - vTexCoord[1];
		}

		if (vTexCoord)
		{
			// clip horizontal
			vClippedTexCoord[0] = vTexCoord[0] + (fTexW * ((fX - pRect.fLeft) * fRcpWidth));
			vClippedTexCoord[2] = vTexCoord[2] + (fTexW * ((fR - (pRect.fLeft + pRect.fWidth)) * fRcpWidth));

			// clip vertical
			vClippedTexCoord[1] = vTexCoord[1] + (fTexH * ((fY - pRect.fTop) * fRcpHeight));
			vClippedTexCoord[3] = vTexCoord[3] + (fTexH * ((fB - (pRect.fTop + pRect.fHeight)) * fRcpHeight));
		}
		else
		{
			// clip horizontal
			vClippedTexCoord[0] = 0.0f + (fTexW * ((fX - pRect.fLeft) * fRcpWidth));
			vClippedTexCoord[2] = 1.0f + (fTexW * ((fR - (pRect.fLeft + pRect.fWidth)) * fRcpWidth));

			// clip vertical
			vClippedTexCoord[1] = 1.0f + (fTexH * ((fY - pRect.fTop) * fRcpHeight));
			vClippedTexCoord[3] = 0.0f + (fTexH * ((fB - (pRect.fTop + pRect.fHeight)) * fRcpHeight));
		}

		m_pRenderer->Draw2dImage(AdjustWidth(fX), AdjustHeight(fY), AdjustWidth(fR - fX), AdjustHeight(fB - fY),
			iTextureID, vClippedTexCoord[0], vClippedTexCoord[1], vClippedTexCoord[2], vClippedTexCoord[3], 0, cColor.v[0], cColor.v[1], cColor.v[2], cColor.v[3], 0);
	}
	else
	{
		m_pRenderer->Draw2dImage(AdjustWidth(fX), AdjustHeight(fY), AdjustWidth(fR - fX), AdjustHeight(fB - fY),
			-1, 0, 0, 0, 0, 0, cColor.v[0], cColor.v[1], cColor.v[2], cColor.v[3], 0);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawSkin(const UIRect &pRect, const UISkinTexture &pTexture, const color4f &cColor, int iState)
{
	if ((iState & UISTATE_UP) || !((iState & UISTATE_DOWN) || (iState & UISTATE_CHECKED)))
	{
		DrawImage(pRect, pTexture.iTextureID, pTexture.vTexCoord, cColor);
	}
	else if ((iState & UISTATE_DOWN) || (iState & UISTATE_CHECKED))
	{
		DrawImage(pRect, pTexture.iDownTextureID, pTexture.vTexCoord, cColor);
	}

	if (iState & UISTATE_OVER)
	{
    m_pRenderer->SetState(GS_BLSRC_ONE | GS_BLDST_ONE | GS_NODEPTHTEST);
		DrawImage(pRect, pTexture.iOverTextureID, pTexture.vTexCoord, cColor);
    m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawEmboss(const UIRect &pRect, const color4f &cHighlightColor, const color4f &cShadowedColor, bool bPressed, float fBorderSize)
{
	float fTwoBorderSize = fBorderSize + fBorderSize;

	if (bPressed)
	{
		DrawQuad(UIRect(pRect.fLeft, pRect.fTop, pRect.fWidth - fBorderSize, fBorderSize), cShadowedColor);
		DrawQuad(UIRect(pRect.fLeft, pRect.fTop + fBorderSize, fBorderSize, pRect.fHeight - fTwoBorderSize), cShadowedColor);
		DrawQuad(UIRect(pRect.fLeft + fBorderSize, pRect.fTop + pRect.fHeight - fBorderSize, pRect.fWidth - fBorderSize, fBorderSize), cHighlightColor);
		DrawQuad(UIRect(pRect.fLeft + pRect.fWidth - fBorderSize, pRect.fTop + fBorderSize, fBorderSize, pRect.fHeight - fTwoBorderSize), cHighlightColor);
	}
	else
	{
		DrawQuad(UIRect(pRect.fLeft, pRect.fTop, pRect.fWidth - fBorderSize, fBorderSize), cHighlightColor);
		DrawQuad(UIRect(pRect.fLeft, pRect.fTop + fBorderSize, fBorderSize, pRect.fHeight - fTwoBorderSize), cHighlightColor);
		DrawQuad(UIRect(pRect.fLeft + fBorderSize, pRect.fTop + pRect.fHeight - fBorderSize, pRect.fWidth - fBorderSize, fBorderSize), cShadowedColor);
		DrawQuad(UIRect(pRect.fLeft + pRect.fWidth - fBorderSize, pRect.fTop + fBorderSize, fBorderSize, pRect.fHeight - fTwoBorderSize), cShadowedColor);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawFocus(const UIRect &pRect, const color4f &cColor, float fBorderSize)
{
	UIBorder pBorder(UIBORDERSTYLE_FLAT, fBorderSize, cColor, 0xffff);

	DrawBorder(pRect, pBorder);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawShadow(const UIRect &pRect, const color4f &cColor, float fBorderSize, CUIWidget *pWidget)
{
	if (pWidget)
	{
		UIRect pNewDrawRect;

		if (pWidget->m_pParent)
		{
			GetWidgetCanvas(&pNewDrawRect, pWidget->m_pParent);
		}

		if (m_bDrawing)
		{
			m_bDrawing = 0;

			SetScissor(&pNewDrawRect);

			DrawQuad(UIRect(pRect.fLeft + fBorderSize, pRect.fTop + fBorderSize, pRect.fWidth, pRect.fHeight), cColor);

			SetScissor(&m_pCurrentDrawRect);

			m_bDrawing = 1;
		}
		else
		{
			SetScissor(&pNewDrawRect);

			DrawQuad(UIRect(pRect.fLeft + fBorderSize, pRect.fTop + fBorderSize, pRect.fWidth, pRect.fHeight), cColor);

			//m_pRenderer->SetScissor();
		}
	}
	else
	{
		DrawQuad(UIRect(pRect.fLeft + fBorderSize, pRect.fTop + fBorderSize, pRect.fWidth, pRect.fHeight), cColor);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawButton(const UIRect &pRect, const color4f &cColor, float fEmbossSize, bool bPressed)
{
	UIRect pNewRect = pRect;

	DrawEmboss(pRect, GET_HIGHLIGHT_COLOR(cColor), GET_SHADOWED_COLOR(cColor), bPressed, fEmbossSize);
	AdjustRect(&pNewRect, pRect, fEmbossSize, 0);
	DrawQuad(pNewRect, cColor);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DrawToolTip()
{
	if ((m_szwToolTipText.empty()) || (m_fToolTipAlpha < 0.0125f))
	{
		return 0;
	}

	const float		fSpace = 4.0f;
	
	IFFont	*pFont = GetIFont(m_pToolTipFont);

	assert(pFont);

	vector2f vTextSize = pFont->GetTextSizeW(m_szwToolTipText.c_str());
	UIRect pRect(m_fToolTipX, m_fToolTipY, vTextSize.x + 2.0f * fSpace, vTextSize.y + 2.0f * fSpace);

	UIRect pBorderRect;
	AdjustRect(&pBorderRect, pRect, m_pToolTipBorder.fSize, 1);

	if (m_pToolTipBorder.fSize > 0.125f)
	{
		UIBorder pAlphaBorder = m_pToolTipBorder;
		pAlphaBorder.cColor.v[3] *= m_fToolTipAlpha;

		DrawBorder(pBorderRect, pAlphaBorder);
	}

	color4f cColor = m_cToolTipColor;
	cColor.v[3] *= m_fToolTipAlpha;

	DrawQuad(pRect, cColor);
	AdjustRect(&pRect, pRect, fSpace);

	cColor = m_pToolTipFont.cColor;
	cColor.v[3] *= m_fToolTipAlpha;

	pFont->SetColor(cColor);
	DrawText(pRect, UIALIGN_LEFT, UIALIGN_TOP, pFont, m_szwToolTipText.c_str());

	return 1;
}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateStatic(CUIStatic **pStatic, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle, const wstring &szText)
{
	*pStatic = new CUIStatic;

	if (!(*pStatic))
	{
		return 0;
	}

	InitializeWidget(*pStatic, pParent, szName, pRect, iFlags, iStyle);

	if (szText.size())
	{
		(*pStatic)->SetText(szText);
	}

	(*pStatic)->Init(m_pScriptSystem, *pStatic);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateButton(CUIButton **pButton, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle, const wstring &szText)
{
	*pButton = new CUIButton;

	if (!(*pButton))
	{
		return 0;
	}

	InitializeWidget(*pButton, pParent, szName, pRect, iFlags, iStyle);

	(*pButton)->m_szText = szText;
	(*pButton)->Init(m_pScriptSystem, *pButton);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateEditBox(CUIEditBox **pEditBox, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle, const wstring &szText)
{
	*pEditBox = new CUIEditBox;

	if (!(*pEditBox))
	{
		return 0;
	}

	InitializeWidget(*pEditBox, pParent, szName, pRect, iFlags, iStyle);

	(*pEditBox)->SetText(szText);
	(*pEditBox)->SetFlags((*pEditBox)->GetFlags() | UIFLAG_CANHAVEFOCUS);
	(*pEditBox)->Init(m_pScriptSystem, *pEditBox);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateScrollBar(CUIScrollBar **pScrollBar, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle, int iType)
{
	*pScrollBar = new CUIScrollBar;

	if (!(*pScrollBar))
	{
		return 0;
	}

	InitializeWidget(*pScrollBar, pParent, szName, pRect, iFlags, iStyle);

	(*pScrollBar)->m_iType = iType;

	if ((*pScrollBar)->GetType() == UISCROLLBARTYPE_HORIZONTAL)
	{
		(*pScrollBar)->m_fSliderSize = (*pScrollBar)->m_fButtonSize = pRect.fHeight;
	}
	else
	{
		(*pScrollBar)->m_fSliderSize = (*pScrollBar)->m_fButtonSize = pRect.fWidth;
	}
	(*pScrollBar)->Init(m_pScriptSystem, *pScrollBar);

	(*pScrollBar)->UpdateRect();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateListView(CUIListView **pListView, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle)
{
	*pListView = new CUIListView;

	if (!(*pListView))
	{
		return 0;
	}

	InitializeWidget(*pListView, pParent, szName, pRect, iFlags, iStyle);

	(*pListView)->Init(m_pScriptSystem, *pListView);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateCheckBox(CUICheckBox **pCheckBox, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle)
{
	*pCheckBox = new CUICheckBox;

	if (!(*pCheckBox))
	{
		return 0;
	}

	InitializeWidget(*pCheckBox, pParent, szName, pRect, iFlags, iStyle);

	(*pCheckBox)->Init(m_pScriptSystem, *pCheckBox);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateComboBox(CUIComboBox **pComboBox, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle)
{
	*pComboBox = new CUIComboBox;

	if (!(*pComboBox))
	{
		return 0;
	}

	InitializeWidget(*pComboBox, pParent, szName, pRect, iFlags, iStyle);

	(*pComboBox)->m_fButtonSize = pRect.fHeight;
	(*pComboBox)->m_fItemHeight = pRect.fHeight;
	(*pComboBox)->m_pComboRect = pRect;

	(*pComboBox)->Init(m_pScriptSystem, *pComboBox);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateVideoPanel(CUIVideoPanel **pVideoPanel, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle)
{
	*pVideoPanel = new CUIVideoPanel;

	if (!(*pVideoPanel))
	{
		return 0;
	}

	InitializeWidget(*pVideoPanel, pParent, szName, pRect, iFlags, iStyle);

	(*pVideoPanel)->Init(m_pScriptSystem, *pVideoPanel);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateScreen(CUIScreen **pScreen, const string &szName)
{
	*pScreen = new CUIScreen;

	if (!(*pScreen))
	{
		return 0;
	}

	(*pScreen)->m_szName = szName;
	(*pScreen)->m_pUISystem = this;

	(*pScreen)->Init(m_pScriptSystem, *pScreen);

	m_vScreenList.push_back(*pScreen);

	return 1;
}


//------------------------------------------------------------------------------------------------- 
bool CUISystem::OnInputEvent(const SInputEvent &event)
{
	// if console is open, don't update anything
	if ((m_pSystem->GetIConsole()->IsOpened()) || (!IsEnabled()))
	{
		ResetKeyRepeat();

		m_vMouseXY = vector2f(-10.0f, -10.0f);

		return false;
	}

	// refresh idle timer
	m_fLastInput = m_pSystem->GetITimer()->GetCurrTime();

	//------------------------------------------------------------------------------------------------- 
	// Mouse Scroll
	//------------------------------------------------------------------------------------------------- 
	if (IS_MOUSE_KEY(event.key))
	{
		if ((event.key == XKEY_MWHEEL_UP) && (event.type == SInputEvent::KEY_PRESS))
		{
			if (m_pMouseOver)
			{
				SendMessage(m_pMouseOver, UIM_KEYPRESSED, 0, XKEY_MWHEEL_UP);
			}
		}
		else if ((event.key == XKEY_MWHEEL_DOWN) && (event.type == SInputEvent::KEY_PRESS))
		{
			if (m_pMouseOver)
			{
				SendMessage(m_pMouseOver, UIM_KEYPRESSED, 0, XKEY_MWHEEL_DOWN);
			}
		}
	}

	//------------------------------------------------------------------------------------------------- 
	// Keyboard events
	//------------------------------------------------------------------------------------------------- 
	if (IS_KEYBOARD_KEY(event.key))
	{
		if ((event.key == XKEY_TAB) && (event.type == SInputEvent::KEY_PRESS))
		{
			if (m_pInput->KeyDown(XKEY_LSHIFT))
			{
				PrevTabStop();
			}
			else
			{
				NextTabStop();
			}
			
			if (m_pFocus)
			{
				if (IsOnFocusScreen(m_pFocus))
				{			
					// find the top-parent
					CUIWidget *pParent = m_pFocus;

					while (pParent->m_pParent)
					{
						pParent = pParent->m_pParent;
					}

					if (pParent->GetFlags() & UIFLAG_CANCHANGEZ)
					{
						SetTopMostWidget(pParent);
					}
				}
			}
		}

		if (m_pFocus)
		{
			if (IsOnFocusScreen(m_pFocus))
			{
				if (event.type == SInputEvent::KEY_RELEASE)
				{
						ResetKeyRepeat();
				}
				else if (m_iLastKey == XKEY_NULL)
				{
					m_iLastKey = event.key;
					m_szLastKeyName = (char *)m_pInput->GetKeyName(event.key, event.moidifiers, 1);
					m_fRepeatTimer = (m_pSystem->GetITimer()->GetAsyncCurTime() * 1000.0f) + (float)ui_RepeatDelay->GetIVal(); // repeat delay
				}

				if (event.type == SInputEvent::KEY_PRESS)
				{
					SendMessage(m_pFocus, UIM_KEYDOWN, (WPARAM)m_pInput->GetKeyName(event.key, event.moidifiers, 1), event.key);	//AMD Port
				}
				else if (event.type == SInputEvent::KEY_RELEASE)
				{
					SendMessage(m_pFocus, UIM_KEYUP, (WPARAM)m_pInput->GetKeyName(event.key, event.moidifiers, 1), event.key);
				}
			}
			else
			{
				ResetKeyRepeat();
			}
		}
		else
		{
			ResetKeyRepeat();
		}
	}

	//------------------------------------------------------------------------------------------------- 
	// Mouse events
	//------------------------------------------------------------------------------------------------- 
	if (IS_MOUSE_KEY(event.key))
	{
		// mouse messages are processed all at once in Update()
	}

	return false;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::InitializeWidget(CUIWidget *pWidget, CUIWidget *pParent, const string &szName, const UIRect &pRect, int iFlags, int iStyle)
{
	pWidget->m_pRect.fLeft = pRect.fLeft;
	pWidget->m_pRect.fTop = pRect.fTop;
	pWidget->m_pRect.fWidth = pRect.fWidth;
	pWidget->m_pRect.fHeight = pRect.fHeight;

	pWidget->m_szName = szName;

	pWidget->m_pParent = pParent;
	pWidget->m_pUISystem = this;
	pWidget->m_iZ = UI_DEFAULT_Z;
	pWidget->m_iTabStop = -1;
	pWidget->m_cGreyedColor = m_cGreyedColor;

	m_pWidgetList.push_back(pWidget);

	if (pParent)
	{
		pParent->AddChild(pWidget);
	}
	else
	{
		AddChild(pWidget);
	}

	if ((pParent) && (pParent->m_pScreen))
	{
		pParent->m_pScreen->AddWidget(pWidget);
	}

	pWidget->SetFlags(iFlags);
	pWidget->SetStyle(iStyle);

	OnZChanged(pWidget);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DestroyWidget(CUIWidget *pWidget)
{
	pWidget->OnRelease();

	DelChild(pWidget);

	for (CUIWidgetItor pItor = m_pWidgetList.begin(); pItor != m_pWidgetList.end(); pItor++)
	{
		if (*pItor == pWidget)
		{
			DeleteWidget(pWidget);

			m_pWidgetList.erase(pItor);

			return 1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::DestroyScreen(CUIScreen *pScreen)
{
	pScreen->OnRelease();

	CUIWidgetItor pItor = pScreen->m_vWidgetList.begin();

	for (CUIScreenItor pSItor = m_vScreenList.begin(); pSItor != m_vScreenList.end(); pSItor++)
	{
		if (*pSItor == pScreen)
		{
			m_vScreenList.erase(pSItor);

			delete pScreen;

			return 1;
		}
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------
int CUISystem::UnloadAllModels()
{
	// unload all models currently loaded by statics
	// they *must* be reloaded afterwards with CUISystem::ReloadAllModels();
	for (CUIWidgetItor pItor = m_pWidgetList.begin(); pItor != m_pWidgetList.end(); ++pItor)
	{
		if ((*pItor)->GetClassName() == "UIStatic")
		{
			CUIStatic *pStatic = (CUIStatic *)(*pItor);

			if (pStatic->m_pModel)
			{
				m_pSystem->GetIAnimationSystem()->RemoveCharacter(pStatic->m_pModel);

				pStatic->m_pModel = 0;
			}
		}
	}

	return 1;
}

//----------------------------------------------------------------------------------------------------
int CUISystem::ReloadAllModels()
{
	// reload all models previously unloaded by CUISystem::UnloadAllModels()
	for (CUIWidgetItor pItor = m_pWidgetList.begin(); pItor != m_pWidgetList.end(); ++pItor)
	{
		if ((*pItor)->GetClassName() == "UIStatic")
		{
			CUIStatic *pStatic = (CUIStatic *)(*pItor);

			if (!pStatic->m_pModel && !pStatic->m_szModelName.empty())
			{
				pStatic->LoadModel(pStatic->m_szModelName);
			}
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::StopAllVideo()
{
	for (CUIWidgetItor pItor = m_pWidgetList.begin(); pItor != m_pWidgetList.end(); ++pItor)
	{
		if ((*pItor)->GetClassName() == "UIVideoPanel")
		{
			CUIVideoPanel *pVideo = (CUIVideoPanel *)(*pItor);

			pVideo->Stop();
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ResetInput()
{
	m_pInput->GetIKeyboard()->ClearKeyState();

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::OnZChanged(CUIWidget *pWidget)
{
	m_bSortZ = 1;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::OnTabStopChanged(CUIWidget *pWidget)
{
	m_bSortTabStop = 1;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
int CUISystem::SortChildrenByZ()
{
	if (m_pChildList.size() < 2)
	{
		return 1;
	}

	std::sort(m_pChildList.begin(), m_pChildList.end(), SortZCallback);

	if (m_iMinZ > 10000)
	{
		for (CUIWidgetItor pItor = m_pChildList.begin(); pItor != m_pChildList.end(); pItor++)
		{
			(*pItor)->m_iZ -= (m_iMinZ-1);
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SortTabStop()
{
	std::sort(m_pTabStopList.begin(), m_pTabStopList.end(), SortTabStopCallback);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::InheritParentAttributes(CUIWidget *pWidget, CUIWidget *pParent)
{
	if (!pParent || !pWidget)
	{
		return 0;
	}

	pWidget->m_pBorder = pParent->m_pBorder;
	pWidget->m_pFont = pParent->m_pFont;
	pWidget->m_cColor = pParent->m_cColor;
	pWidget->m_iMouseCursor = pParent->m_iMouseCursor;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
#define CHECKATTRIBUTE(name, type)			((strcmp(szAttributeName, name) == 0) && (pObject->GetCurrentType() == (type)))
int CUISystem::RetrieveCommonAttribute(IScriptObject *pObject, CUIWidget *pWidget)
{
	char	*szKeyName;
	char	szAttributeName[128];

	char	*szValue;
	float	fValue;
	int		iValue;

	pObject->GetCurrentKey(szKeyName);

	strcpy(szAttributeName, szKeyName);
	strlwr(szAttributeName);

	pObject->GetCurrent(szValue);
	pObject->GetCurrent(fValue);
	pObject->GetCurrent(iValue);

	// font attributes
	if ((pObject->GetCurrentType() == svtObject) ||
			(CHECKATTRIBUTE("classname", svtString)) ||
			IsReserved(szAttributeName))
	{
		return 1;
	}
	else if (CHECKATTRIBUTE("zorder", svtNumber))
	{
		pWidget->m_iZ = iValue;

		OnZChanged(pWidget);
	}
	else if (CHECKATTRIBUTE("tabstop", svtNumber))
	{
		pWidget->m_iTabStop = iValue;

		m_pTabStopList.push_back(pWidget);

		OnTabStopChanged(pWidget);
	}
	else if (CHECKATTRIBUTE("fontname", svtString))
	{
		pWidget->m_pFont.szFaceName = szValue;
	}
	else if (CHECKATTRIBUTE("fonteffect", svtString))
	{
		pWidget->m_pFont.szEffectName = szValue;
	}
	else if (CHECKATTRIBUTE("fontsize", svtNumber))
	{
		pWidget->m_pFont.fSize = fValue;
	}
	else if (CHECKATTRIBUTE("fontcolor", svtString))
	{
		RetrieveColor(&pWidget->m_pFont.cColor, szValue);
	}

	// border attributes
	else if (CHECKATTRIBUTE("bordersize", svtNumber))
	{
		pWidget->m_pBorder.fSize = floorf(fValue);
	}
	else if (CHECKATTRIBUTE("borderstyle", svtNumber))
	{
		pWidget->m_pBorder.iStyle = iValue;
	}
	else if (CHECKATTRIBUTE("bordercolor", svtString))
	{
		RetrieveColor(&pWidget->m_pBorder.cColor, szValue);
	}
	else if (CHECKATTRIBUTE("bordersides", svtString))
	{
		int iFlags = 0;
		char szFlags[5]={0};

		strncpy(szFlags, szValue, 4);
		strupr(szFlags);
    
		for (int i = 0; i < (int)min(strlen(szFlags), 4); i++)
		{
			if (szFlags[i] == 'L')
			{
				iFlags |= UIBORDERSIDE_LEFT;
			}
			else if (szFlags[i] == 'T')
			{
				iFlags |= UIBORDERSIDE_TOP;
			}
			else if (szFlags[i] == 'R')
			{
				iFlags |= UIBORDERSIDE_RIGHT;
			}
			else if (szFlags[i] == 'B')
			{
				iFlags |= UIBORDERSIDE_BOTTOM;
			}
		}

		pWidget->m_pBorder.iFlags = iFlags;
	}

	// position properties
	else if (CHECKATTRIBUTE("rect", svtString))
	{
		RetrieveRect(&pWidget->m_pRect, szValue);
	}
	else if (CHECKATTRIBUTE("left", svtNumber))
	{
		pWidget->m_pRect.fLeft = (float)(floor(m_fVirtualToRealX * fValue) * m_fRealToVirtualX);
	}
	else if (CHECKATTRIBUTE("top", svtNumber))
	{
		pWidget->m_pRect.fTop = (float)(floor(m_fVirtualToRealY * fValue) * m_fRealToVirtualY);
	}
	else if (CHECKATTRIBUTE("width", svtNumber))
	{
		pWidget->m_pRect.fWidth = (float)(floor(m_fVirtualToRealX * fValue) * m_fRealToVirtualX);
	}
	else if (CHECKATTRIBUTE("height", svtNumber))
	{
		pWidget->m_pRect.fHeight = (float)(floor(m_fVirtualToRealY * fValue) * m_fRealToVirtualY);
	}

	// flags and style
	else if (CHECKATTRIBUTE("flags", svtNumber))
	{
		pWidget->SetFlags(iValue);
	}
	else if (CHECKATTRIBUTE("style", svtNumber))
	{
		pWidget->SetStyle(iValue);
	}

	// color
	else if (CHECKATTRIBUTE("color", svtString))
	{
		RetrieveColor(&pWidget->m_cColor, szValue);
	}
	else if (CHECKATTRIBUTE("greyedcolor", svtString))
	{
		RetrieveColor(&pWidget->m_cGreyedColor, szValue);
	}
	else if (CHECKATTRIBUTE("greyedblend", svtNumber))
	{
		pWidget->m_iGreyedBlend = iValue;
	}

	// tooltip
	else if (CHECKATTRIBUTE("tooltip", svtString))
	{
		ConvertToWString(pWidget->m_szwToolTip, szValue);
	}

	// if not OnBlah event, failure
	else if ((szKeyName[0] != 'O') || (szKeyName[1] != 'n'))
	{
		return 0;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::RetrieveColor(color4f *pColor, char *szString)
{
	int iR, iG, iB, iA = 255;


	if ((!szString) || (*szString == 0))
	{
		pColor->v[0] = 1.0f;
		pColor->v[1] = 1.0f;
		pColor->v[2] = 1.0f;
		pColor->v[3] = 1.0f;

		return 1;
	}

	if ((sscanf(szString, "%d,%d,%d,%d", &iR, &iG, &iB, &iA) == 4) || (sscanf(szString, "%d %d %d %d", &iR, &iG, &iB, &iA) == 4))
	{
		pColor->v[0] = iR * (1.0f / 255.0f);
		pColor->v[1] = iG * (1.0f / 255.0f);
		pColor->v[2] = iB * (1.0f / 255.0f);
		pColor->v[3] = iA * (1.0f / 255.0f);
	}
	else if ((sscanf(szString, "%d,%d,%d", &iR, &iG, &iB) == 3) || (sscanf(szString, "%d %d %d", &iR, &iG, &iB) == 3))
	{
		pColor->v[0] = iR * (1.0f / 255.0f);
		pColor->v[1] = iG * (1.0f / 255.0f);
		pColor->v[2] = iB * (1.0f / 255.0f);
		pColor->v[3] = iA * (1.0f / 255.0f);
	}
	else
	{
		m_pLog->LogToConsole("\001$4[Error]:$1 Failed to retrieve color information from string!");
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::RetrieveRect(UIRect *pRect, char *szString)
{
	float fLeft, fTop, fWidth, fHeight;
	int iLeft, iTop, iWidth, iHeight;

	if ((sscanf(szString, "%f,%f,%f,%f", &fLeft, &fTop, &fWidth, &fHeight) == 4) || (sscanf(szString, "%f %f %f %f", &fLeft, &fTop, &fWidth, &fHeight) == 4))
	{
		pRect->fLeft = fLeft;
		pRect->fTop = fTop;
		pRect->fWidth = fWidth;
		pRect->fHeight = fHeight;
	}
	else if ((sscanf(szString, "%d,%d,%d,%d", &iLeft, &iTop, &iWidth, &iHeight) == 4) || (sscanf(szString, "%d %d %d %d", &iLeft, &iTop, &iWidth, &iHeight) == 4))
	{
		pRect->fLeft = (float)iLeft;
		pRect->fTop = (float)iTop;
		pRect->fWidth = (float)iWidth;
		pRect->fHeight = (float)iHeight;
	}
	else
	{
		m_pLog->LogToConsole("\001$4[Error]:$1 Failed to retrieve rect information from string!");

		return 0;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::RetrieveTexRect(float *pTexCoords, INT_PTR iTextureID, char *szTexRect)
{
	float fLeft, fTop, fWidth, fHeight;
	int iLeft, iTop, iWidth, iHeight;

	if ((!szTexRect) || (*szTexRect == 0))
	{
		pTexCoords[0] = 0.0f;
		pTexCoords[1] = 1.0f;
		pTexCoords[2] = 1.0f;
		pTexCoords[3] = 0.0f;

		return 1;
	}

	if ((sscanf(szTexRect, "%f,%f,%f,%f", &fLeft, &fTop, &fWidth, &fHeight) == 4) || (sscanf(szTexRect, "%f %f %f %f", &fLeft, &fTop, &fWidth, &fHeight) == 4))
	{
		pTexCoords[0] = fLeft;
		pTexCoords[1] = fTop;
		pTexCoords[2] = fLeft + fWidth;
		pTexCoords[3] = (fTop + fHeight);
	}
	else if ((sscanf(szTexRect, "%d,%d,%d,%d", &iLeft, &iTop, &iWidth, &iHeight) == 4) || (sscanf(szTexRect, "%d %d %d %d", &iLeft, &iTop, &iWidth, &iHeight) == 4))
	{
		pTexCoords[0] = (float)iLeft;
		pTexCoords[1] = (float)iTop;
		pTexCoords[2] = (float)(iLeft + iWidth);
		pTexCoords[3] = (float)(iTop + iHeight);
	}
	else
	{
		m_pLog->LogToConsole("\001$4[Error]:$1 Failed to retrieve texture rect information from string!");
	}

	pTexCoords[0] += 0.5f;
	pTexCoords[1] += 0.5f;
	pTexCoords[2] -= 0.5f;
	pTexCoords[3] -= 0.5f;

	if (iTextureID > -1)
	{
		ITexPic *pTex = m_pRenderer->EF_GetTextureByID(iTextureID);

		float fRcpWidth = 1.0f / (float)pTex->GetOriginalWidth();
		float fRcpHeight = 1.0f / (float)pTex->GetOriginalHeight();

		pTexCoords[0] *= fRcpWidth;
		pTexCoords[1] = 1.0f - (pTexCoords[1] * fRcpHeight);
		pTexCoords[2] *= fRcpWidth;
		pTexCoords[3] = 1.0f - (pTexCoords[3] * fRcpHeight);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::RetrieveTextAttribute(CUIWidget *pWidget, IScriptObject *pObject, const string &szTextField)
{
	char	*szKeyName;
	char	szAttributeName[256];
	
	char					*szValue;
	wstring	szWValue;

	pObject->GetCurrentKey(szKeyName);

	strcpy(szAttributeName, szKeyName);
	strlwr(szAttributeName);

	if (szTextField == szAttributeName)
	{
		if ((pObject->GetCurrentType() == svtNumber) || (pObject->GetCurrentType() == svtString))
		{
			if (pObject->GetCurrent(szValue))
			{
				ConvertToWString(szWValue, szValue);

				if (pWidget->GetClassName() == UICLASSNAME_STATIC)
				{
					((CUIStatic *)pWidget)->SetText(szWValue);
				}
				else if (pWidget->GetClassName() == UICLASSNAME_BUTTON)
				{
					((CUIButton *)pWidget)->SetText(szWValue);
				}
				else if (pWidget->GetClassName() == UICLASSNAME_EDITBOX)
				{
					((CUIEditBox *)pWidget)->SetText(szWValue);
				}
				else if (pWidget->GetClassName() == UICLASSNAME_CHECKBOX)
				{
					((CUICheckBox *)pWidget)->SetText(szWValue);
				}
				else
				{
					return 0;
				}

				return 1;
			}
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::RetrieveTextureAttribute(UISkinTexture *pSkinTexture, IScriptObject *pObject, const char *szTextureField, const char *szTexRectField)
{
	char	szOverField[256];
	char	szDownField[256];

	char	*szValue = 0;
	char	*szKey = 0;

	strcpy(szOverField, "over");
	strcat(szOverField, szTextureField);
	strcpy(szDownField, "down");
	strcat(szDownField, szTextureField);

	pObject->GetCurrentKey(szKey);

	if (strcmp(szTextureField, szKey) == 0)
	{
		if (pObject->GetValueType(szTextureField) == svtUserData)
		{
			int iCookie = 0;

			pObject->GetUDValue(szTextureField, pSkinTexture->iTextureID, iCookie);
		}

		if (pObject->GetValueType(szOverField) == svtUserData)
		{
			int iCookie = 0;

			pObject->GetUDValue(szOverField, pSkinTexture->iOverTextureID, iCookie);
		}

		if (pObject->GetValueType(szDownField) == svtUserData)
		{
			int iCookie = 0;

			pObject->GetUDValue(szDownField, pSkinTexture->iDownTextureID, iCookie);
		}

		if (pObject->GetValueType(szTexRectField) == svtString)
		{
			pObject->GetValue(szTexRectField, (const char* &)szValue);

			RetrieveTexRect(pSkinTexture->vTexCoord, pSkinTexture->iTextureID, szValue);
		}

		return 1;
	}
	else if ((strcmp(szTexRectField, szKey) == 0) || (strcmp(szDownField, szKey) == 0) || (strcmp(szOverField, szKey) == 0))
	{
		return 1;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateObjectFromTable(CUIWidget **pWidget, CUIWidget *pParent, CUIScreen *pScreen, IScriptObject *pObject, const string &szName)
{
	char				*szKeyName;
	char				*szAttributeValue;
	string szClassName;
	UIRect			pRect;
	int					iResult = 0;

	IScriptObject *pObj = 0;

	for (int i = 0; i < 2; i++)
	{
		if ((i == 0) && (pObject->GetValueType("skin") != svtObject))
		{
			continue;
		}

		if (i == 0)
		{
			pObj = m_pScriptSystem->CreateEmptyObject();

			pObject->GetValue("skin", pObj);
		}
		else
		{
			pObj = pObject;
		}

		// get classname
		if (pObj->GetValueType("classname") == svtString)
		{
			pObj->GetValue("classname", (const char* &)szAttributeValue);

			// convert to lowercase
			szClassName = szAttributeValue;
			strlwr((char *)szClassName.c_str());
		}

		// get rect
		if (pObj->GetValueType("rect") == svtString)
		{
			pObj->GetValue("rect", (const char* &)szAttributeValue);

			RetrieveRect(&pRect, szAttributeValue);
		}
		else if (
			(pObj->GetValueType("left") == svtNumber) ||
			(pObj->GetValueType("top") == svtNumber) ||
			(pObj->GetValueType("width") == svtNumber) ||
			(pObj->GetValueType("height") == svtNumber))
		{
			pObj->GetValue("left", pRect.fLeft);
			pObj->GetValue("top", pRect.fTop);
			pObj->GetValue("width", pRect.fWidth);
			pObj->GetValue("height", pRect.fHeight);

			
			pRect.fLeft = (float)(floor(m_fVirtualToRealX * pRect.fLeft) * m_fRealToVirtualX);
			pRect.fTop = (float)(floor(m_fVirtualToRealY * pRect.fTop) * m_fRealToVirtualY);
			pRect.fWidth = (float)(floor(m_fVirtualToRealX * pRect.fWidth) * m_fRealToVirtualX);
			pRect.fHeight = (float)(floor(m_fVirtualToRealY * pRect.fHeight) * m_fRealToVirtualY);
		}

		if (i == 0)
		{
			pObj->Release();
		}
	}

	if (!szClassName.size())
	{
		m_pLog->LogToConsole("\001$4[Error]:$1 Need classname for widget '%s'.", szName.c_str());

		return 0;
	}

	// create the apropriate control
	if (szClassName == "static")
	{
		iResult = CreateStaticFromTable((CUIStatic **)pWidget, pParent, pRect, pObject, szName);
	}
	else if (szClassName == "button")
	{
		iResult = CreateButtonFromTable((CUIButton **)pWidget, pParent, pRect, pObject, szName);
	}
	else if (szClassName == "editbox")
	{
		iResult = CreateEditBoxFromTable((CUIEditBox **)pWidget, pParent, pRect, pObject, szName);
	}
	else if (szClassName == "scrollbar")
	{
		iResult = CreateScrollBarFromTable((CUIScrollBar **)pWidget, pParent, pRect, pObject, szName);
	}
	else if (szClassName == "listview")
	{
		iResult = CreateListViewFromTable((CUIListView **)pWidget, pParent, pRect, pObject, szName);
	}
	else if (szClassName == "checkbox")
	{
		iResult = CreateCheckBoxFromTable((CUICheckBox **)pWidget, pParent, pRect, pObject, szName);
	}
	else if (szClassName == "combobox")
	{
		iResult = CreateComboBoxFromTable((CUIComboBox **)pWidget, pParent, pRect, pObject, szName);
	}
	else if (szClassName == "videopanel")
	{
		iResult = CreateVideoPanelFromTable((CUIVideoPanel **)pWidget, pParent, pRect, pObject, szName);
	}
	else
	{
		m_pLog->LogToConsole("\001$4[Error]:$1 Invalid classname for widget '%s': %s", szName.c_str(), szClassName.c_str());
	}

	if (!iResult)
	{
		m_pLog->LogToConsole("\001$4[Error]:$1 Failed to create object '%s'...", szName.c_str());

		return 0;
	}

	if (pScreen)
	{
		pScreen->AddWidget(*pWidget);
	}

	GetWidgetScriptObject(*pWidget)->Clone(pObject);

	// find child objects
	pObject->BeginIteration();

	while (pObject->MoveNext())
	{
		// we only want objects
		if (pObject->GetCurrentType() == svtObject)
		{
			pObject->GetCurrentKey(szKeyName);

			if (IsReserved(szKeyName))
			{
				continue;
			}

			IScriptObject *pNewObject = m_pScriptSystem->CreateEmptyObject();
		
			pObject->GetCurrent(pNewObject);

			if ((pScreen && (pScreen->GetWidget(szKeyName))) || ((!pScreen) && (GetWidget(szKeyName))))
			{
				if ((strcmp(szKeyName, "hscrollbar") != 0) && (strcmp(szKeyName, "vscrollbar") != 0))
				{
					pNewObject->Release();

					m_pLog->LogToConsole("\001$4[Error]:$1 Widget name already exists: '%s'", szKeyName);

					return 0;
				}
			}

			CUIWidget *pChildWidget = 0;

			if (!CreateObjectFromTable(&pChildWidget, *pWidget, pScreen, pNewObject, szKeyName))
			{
				pObject->EndIteration();
				pNewObject->Release();

				return 0;
			}

			pNewObject->Release();

			if (pChildWidget)
			{
				GetWidgetScriptObject(*pWidget)->SetValue(pChildWidget->GetName().c_str(), GetWidgetScriptObject(pChildWidget));
			}
		}
	}

	pObject->EndIteration();

	(*pWidget)->OnInit();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateStaticFromTable(CUIStatic **pStatic, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName)
{
	if (!CreateStatic(pStatic, pParent, szName, pRect, UIFLAG_DEFAULT, 0, L""))
	{
		return 0;
	}

	InheritParentAttributes(*pStatic, pParent);

	IScriptObject *pSkinObject = m_pScriptSystem->CreateEmptyObject();
		
	if (pObject->GetValue("skin", pSkinObject))
	{
		SetupStaticFromTable(*pStatic, pSkinObject);
	}

	pSkinObject->Release();

	SetupStaticFromTable(*pStatic, pObject);
	
	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetupStaticFromTable(CUIStatic *pStatic, IScriptObject *pObject)
{
	// get the specific attributes
	char	*szKeyName;
	char	szAttributeName[128];

	// to hold the values
	char	*szValue;
	float	fValue;
	int		iValue;

	string szModel;
	string szAnimation;

	pObject->BeginIteration();

	while(pObject->MoveNext())
	{
		pObject->GetCurrentKey(szKeyName);

		strcpy(szAttributeName, szKeyName);
		strlwr(szAttributeName);

		pObject->GetCurrent(szValue);
		pObject->GetCurrent(fValue);
		pObject->GetCurrent(iValue);

		if (CHECKATTRIBUTE("halign", svtNumber))
		{
			pStatic->m_iHAlignment = iValue;
		}
		else if (CHECKATTRIBUTE("valign", svtNumber))
		{
			pStatic->m_iVAlignment = iValue;
		}
		else if (CHECKATTRIBUTE("linespacing", svtNumber))
		{
			pStatic->m_fLineSpacing = fValue;
		}
		else if (CHECKATTRIBUTE("leftspacing", svtNumber))
		{
			pStatic->m_fLeftSpacing = fValue;
		}
		else if (CHECKATTRIBUTE("rightspacing", svtNumber))
		{
			pStatic->m_fRightSpacing = fValue;
		}
		else if (CHECKATTRIBUTE("maxlines", svtNumber))
		{
			pStatic->m_iMaxLines = iValue;
		}
		else if (CHECKATTRIBUTE("model", svtString))
		{
			szModel = szValue;
		}
		else if (CHECKATTRIBUTE("animation", svtString))
		{
			szAnimation = szValue;
		}
		else if (CHECKATTRIBUTE("mousemultiplier", svtNumber))
		{
			pStatic->m_fMouseMultiplier = fValue;
		}
		else if (CHECKATTRIBUTE("viewfov", svtNumber))
		{
			pStatic->m_fCameraFov = fValue * gf_PI / 180.0f;
		}
		else if (CHECKATTRIBUTE("viewdistance", svtNumber))
		{
			pStatic->m_fCameraDistance = fValue;
		}
		else if (CHECKATTRIBUTE("lightdistance", svtNumber))
		{
			pStatic->m_fLightDistance = fValue;
		}
		else if (CHECKATTRIBUTE("rotation", svtNumber))
		{
			pStatic->m_fModelRotation = fValue;
		}
		else if (RetrieveTextureAttribute(&pStatic->m_pTexture, pObject, "texture", "texrect"))
		{
		}
		else if (RetrieveTextAttribute(pStatic, pObject, "text"))
		{
		}
		else if (!RetrieveCommonAttribute(pObject, pStatic))
		{
			m_pLog->LogToConsole("\001$5[Warning]:$1 %s unknown attribute/value: '%s'", pStatic->GetName().c_str(), szKeyName);
		}
	}

	pObject->EndIteration();

	if (szModel.size())
	{
		pStatic->LoadModel(szModel);

		if (szAnimation.size())
		{
			pStatic->StartAnimation(szAnimation);
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateButtonFromTable(CUIButton **pButton, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName)
{
	if (!CreateButton(pButton, pParent, szName, pRect, UIFLAG_DEFAULT, 0, L""))
	{
		return 0;
	}

	InheritParentAttributes(*pButton, pParent);

	IScriptObject *pSkinObject = m_pScriptSystem->CreateEmptyObject();

	if (pObject->GetValue("skin", pSkinObject))
	{
		SetupButtonFromTable(*pButton, pSkinObject);
	}

	pSkinObject->Release();

	SetupButtonFromTable(*pButton, pObject);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetupButtonFromTable(CUIButton *pButton, IScriptObject *pObject)
{
	// get the specific attributes
	char	*szKeyName;
	char	szAttributeName[128];

	// to hold the values
	char	*szValue;
	float	fValue;
	int		iValue;

	pObject->BeginIteration();

	while(pObject->MoveNext())
	{
		pObject->GetCurrentKey(szKeyName);

		strcpy(szAttributeName, szKeyName);
		strlwr(szAttributeName);

		pObject->GetCurrent(szValue);
		pObject->GetCurrent(fValue);
		pObject->GetCurrent(iValue);

		//------------------------------------------------------------------------------------------------- 
		if (CHECKATTRIBUTE("halign", svtNumber))
		{
			pButton->m_iHAlignment = iValue;
		}
		else if (CHECKATTRIBUTE("valign", svtNumber))
		{
			pButton->m_iVAlignment = iValue;
		}
		else if (CHECKATTRIBUTE("state", svtNumber))
		{
			pButton->m_iState = iValue;
		}
		else if (RetrieveTextureAttribute(&pButton->m_pTexture, pObject, "texture", "texrect"))
		{
		}
		else if (RetrieveTextAttribute(pButton, pObject, "text"))
		{
		}
		else if (!RetrieveCommonAttribute(pObject, pButton))
		{
			m_pLog->LogToConsole("\001$5[Warning]:$1 %s unknown attribute/value: '%s'", pButton->GetName().c_str(), szKeyName);
		}
		//------------------------------------------------------------------------------------------------- 
	}
	
	pObject->EndIteration();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateEditBoxFromTable(CUIEditBox **pEditBox, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName)
{
	if (!CreateEditBox(pEditBox, pParent, szName, pRect, UIFLAG_DEFAULT, 0, L""))
	{
		return 0;
	}

	InheritParentAttributes(*pEditBox, pParent);

	IScriptObject *pSkinObject = m_pScriptSystem->CreateEmptyObject();

	if (pObject->GetValue("skin", pSkinObject))
	{
		SetupEditBoxFromTable(*pEditBox, pSkinObject);
	}

	pSkinObject->Release();

	SetupEditBoxFromTable(*pEditBox, pObject);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetupEditBoxFromTable(CUIEditBox *pEditBox, IScriptObject *pObject)
{
	// get the specific attributes
	char	*szKeyName;
	char	szAttributeName[128];

	// to hold the values
	char	*szValue;
	float	fValue;
	int		iValue;

	pObject->BeginIteration();

	while(pObject->MoveNext())
	{
		pObject->GetCurrentKey(szKeyName);

		strcpy(szAttributeName, szKeyName);
		strlwr(szAttributeName);

		pObject->GetCurrent(szValue);
		pObject->GetCurrent(fValue);
		pObject->GetCurrent(iValue);

		//------------------------------------------------------------------------------------------------- 
		if (CHECKATTRIBUTE("leftspacing", svtNumber))
		{
			pEditBox->m_fLeftSpacing = fValue;
		}
		else if (CHECKATTRIBUTE("rightspacing", svtNumber))
		{
			pEditBox->m_fRightSpacing = fValue;
		}
		else if (CHECKATTRIBUTE("allow", svtString))
		{
			pEditBox->m_szAllow = szValue;
		}
		else if (CHECKATTRIBUTE("disallow", svtString))
		{
			pEditBox->m_szDisallow = szValue;
		}
		else if (CHECKATTRIBUTE("numeric", svtNumber))
		{
			pEditBox->m_iNumeric = iValue;
		}
		else if (CHECKATTRIBUTE("pathsafe", svtNumber))
		{
			pEditBox->m_iPathSafe = iValue;
		}
		else if (CHECKATTRIBUTE("namesafe", svtNumber))
		{
			pEditBox->m_iNameSafe = iValue;
		}
		else if (CHECKATTRIBUTE("ubisafe", svtNumber))
		{
			pEditBox->m_iUbiSafe = iValue;
		}
		else if (CHECKATTRIBUTE("halign", svtNumber))
		{
			pEditBox->m_iHAlignment = iValue;
		}
		else if (CHECKATTRIBUTE("valign", svtNumber))
		{
			pEditBox->m_iVAlignment = iValue;
		}
		else if (CHECKATTRIBUTE("maxlength", svtNumber))
		{
			pEditBox->m_iMaxLength = iValue;
		}
		else if (CHECKATTRIBUTE("selectionstart", svtNumber))
		{
			pEditBox->m_iSelectionStart = iValue;
		}
		else if (CHECKATTRIBUTE("selectioncount", svtNumber))
		{
			pEditBox->m_iSelectionCount = iValue;
		}
		else if (CHECKATTRIBUTE("selectioncolor", svtString))
		{
			RetrieveColor(&pEditBox->m_cSelectionColor, szValue);
		}
		else if (RetrieveTextAttribute(pEditBox, pObject, "text"))
		{
		}
		else if (CHECKATTRIBUTE("cursorcolor", svtString))
		{
			RetrieveColor(&pEditBox->m_cCursorColor, szValue);
		}
		else if (RetrieveTextureAttribute(&pEditBox->m_pTexture, pObject, "texture", "texrect"))
		{
		}
		else if (!RetrieveCommonAttribute(pObject, pEditBox))
		{
			m_pLog->LogToConsole("\001$5[Warning]:$1 %s unknown attribute/value: '%s'", pEditBox->GetName().c_str(), szKeyName);
		}
		//------------------------------------------------------------------------------------------------- 
	}

	pObject->EndIteration();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateScrollBarFromTable(CUIScrollBar **pScrollBar, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName)
{
	if (!CreateScrollBar(pScrollBar, pParent, szName,pRect, UIFLAG_DEFAULT, 0))
	{
		return 0;
	}

	InheritParentAttributes(*pScrollBar, pParent);

	IScriptObject *pSkinObject = m_pScriptSystem->CreateEmptyObject();

	if (pObject->GetValue("skin", pSkinObject))
	{
		SetupScrollBarFromTable(*pScrollBar, pSkinObject);
	}

	pSkinObject->Release();

	SetupScrollBarFromTable(*pScrollBar, pObject);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetupScrollBarFromTable(CUIScrollBar *pScrollBar, IScriptObject *pObject)
{
	// get the specific attributes
	char	*szKeyName;
	char	szAttributeName[128];

	// to hold the values
	char	*szValue;
	float	fValue;
	int		iValue;

	pObject->BeginIteration();

	while(pObject->MoveNext())
	{
		pObject->GetCurrentKey(szKeyName);

		strcpy(szAttributeName, szKeyName);
		strlwr(szAttributeName);

		pObject->GetCurrent(szValue);
		pObject->GetCurrent(fValue);
		pObject->GetCurrent(iValue);

		//------------------------------------------------------------------------------------------------- 
		if (CHECKATTRIBUTE("value", svtNumber))
		{
			pScrollBar->m_fValue = fValue;
		}
		else if (CHECKATTRIBUTE("step", svtNumber))
		{
			pScrollBar->m_fStep = fValue;
		}
		else if (CHECKATTRIBUTE("slidersize", svtNumber))
		{
			pScrollBar->m_fSliderSize = fValue;
		}
		else if (CHECKATTRIBUTE("buttonsize", svtNumber))
		{
			pScrollBar->m_fButtonSize = fValue;
		}
		else if (RetrieveTextureAttribute(&pScrollBar->m_pPathTexture, pObject, "pathtexture", "pathtexrect"))
		{
		}
		else if (RetrieveTextureAttribute(&pScrollBar->m_pSliderTexture, pObject, "slidertexture", "slidertexrect"))
		{
		}
		else if (RetrieveTextureAttribute(&pScrollBar->m_pPlusTexture, pObject, "plustexture", "plustexrect"))
		{
		}
		else if (RetrieveTextureAttribute(&pScrollBar->m_pMinusTexture, pObject, "minustexture", "minustexrect"))
		{
		}
		else if (!RetrieveCommonAttribute(pObject, pScrollBar))
		{
			m_pLog->LogToConsole("\001$5[Warning]:$1 %s unknown attribute/value: '%s'", pScrollBar->GetName().c_str(), szKeyName);
		}
		//------------------------------------------------------------------------------------------------- 
	}

	pObject->EndIteration();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateListViewFromTable(CUIListView **pListView, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName)
{
	if (!CreateListView(pListView, pParent, szName, pRect, UIFLAG_DEFAULT, 0))
	{
		return 0;
	}

	InheritParentAttributes(*pListView, pParent);

	IScriptObject *pSkinObject = m_pScriptSystem->CreateEmptyObject();

	if (pObject->GetValue("skin", pSkinObject))
	{
		SetupListViewFromTable(*pListView, pSkinObject);
	}

	pSkinObject->Release();

	SetupListViewFromTable(*pListView, pObject);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetupListViewFromTable(CUIListView *pListView, IScriptObject *pObject)
{
	// get the specific attributes
	char	*szKeyName;
	char	szAttributeName[128];

	// to hold the values
	char	*szValue;
	float	fValue;
	int		iValue;

	pObject->BeginIteration();

	while(pObject->MoveNext())
	{
		pObject->GetCurrentKey(szKeyName);

		strcpy(szAttributeName, szKeyName);
		strlwr(szAttributeName);

		pObject->GetCurrent(szValue);
		pObject->GetCurrent(fValue);
		pObject->GetCurrent(iValue);

		//------------------------------------------------------------------------------------------------- 
		if (CHECKATTRIBUTE("itemheight", svtNumber))
		{
			pListView->m_fItemHeight = fValue;
		}
		else if (CHECKATTRIBUTE("columnspacing", svtNumber))
		{
			pListView->m_fItemHeight = fValue;
		}
		else if (CHECKATTRIBUTE("headerheight", svtNumber))
		{
			pListView->m_fHeaderHeight = fValue;
		}
		else if (CHECKATTRIBUTE("cellspacing", svtNumber))
		{
			pListView->m_fCellSpacing = fValue;
		}
		else if (CHECKATTRIBUTE("cellpadding", svtNumber))
		{
			pListView->m_fCellPadding = fValue;
		}
		else if (CHECKATTRIBUTE("columnselect", svtNumber))
		{
			pListView->m_bColumnSelect = (iValue != 0);
		}
		else if (CHECKATTRIBUTE("nosort", svtNumber))
		{
			pListView->m_iNoSort = iValue;
		}
		else if (CHECKATTRIBUTE("selectioncolor", svtString))
		{
			RetrieveColor(&pListView->m_cSelectionColor, szValue);
		}

		else if (CHECKATTRIBUTE("sortcolumncolor", svtString))
		{
			RetrieveColor(&pListView->m_cSortByColor, szValue);
		}
		else if (CHECKATTRIBUTE("sortcolumntextcolor", svtString))
		{
			RetrieveColor(&pListView->m_cSortByTextColor, szValue);
		}
		else if (RetrieveTextureAttribute(&pListView->m_pTexture, pObject, "texture", "texrect"))
		{
		}
		else if (!RetrieveCommonAttribute(pObject, pListView))
		{
			m_pLog->LogToConsole("\001$5[Warning]:$1 %s unknown attribute/value: '%s'", pListView->GetName().c_str(), szKeyName);
		}
		//------------------------------------------------------------------------------------------------- 
	}

	pObject->EndIteration();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateCheckBoxFromTable(CUICheckBox **pCheckBox, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName)
{
	if (!CreateCheckBox(pCheckBox, pParent, szName, pRect, UIFLAG_DEFAULT, 0))
	{
		return 0;
	}

	InheritParentAttributes(*pCheckBox, pParent);

	IScriptObject *pSkinObject = m_pScriptSystem->CreateEmptyObject();

	if (pObject->GetValue("skin", pSkinObject))
	{
		SetupCheckBoxFromTable(*pCheckBox, pSkinObject);
	}

	pSkinObject->Release();

	SetupCheckBoxFromTable(*pCheckBox, pObject);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetupCheckBoxFromTable(CUICheckBox *pCheckBox, IScriptObject *pObject)
{
	// get the specific attributes
	char	*szKeyName;
	char	szAttributeName[128];

	// to hold the values
	char	*szValue;
	float	fValue;
	int		iValue;

	pObject->BeginIteration();

	while(pObject->MoveNext())
	{
		pObject->GetCurrentKey(szKeyName);

		strcpy(szAttributeName, szKeyName);
		strlwr(szAttributeName);

		pObject->GetCurrent(szValue);
		pObject->GetCurrent(fValue);
		pObject->GetCurrent(iValue);

		//------------------------------------------------------------------------------------------------- 
		if (CHECKATTRIBUTE("leftspacing", svtNumber))
		{
			pCheckBox->m_fLeftSpacing = fValue;
		}
		else if (CHECKATTRIBUTE("rightspacing", svtNumber))
		{
			pCheckBox->m_fRightSpacing = fValue;
		}
		else if (CHECKATTRIBUTE("halign", svtNumber))
		{
			pCheckBox->m_iHAlignment = iValue;
		}
		else if (CHECKATTRIBUTE("valign", svtNumber))
		{
			pCheckBox->m_iVAlignment = iValue;
		}
		else if (CHECKATTRIBUTE("checkcolor", svtString))
		{
			RetrieveColor(&pCheckBox->m_cCheckColor, szValue);
		}
		else if (RetrieveTextureAttribute(&pCheckBox->m_pTexture, pObject, "texture", "texrect"))
		{
		}
		else if (RetrieveTextAttribute(pCheckBox, pObject, "text"))
		{
		}
		else if (!RetrieveCommonAttribute(pObject, pCheckBox))
		{
			m_pLog->LogToConsole("\001$5[Warning]:$1 %s unknown attribute/value: '%s'", pCheckBox->GetName().c_str(), szKeyName);
		}
			//------------------------------------------------------------------------------------------------- 
	}

	pObject->EndIteration();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateComboBoxFromTable(CUIComboBox **pComboBox, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName)
{
	if (!CreateComboBox(pComboBox, pParent, szName, pRect, UIFLAG_DEFAULT, 0))
	{
		return 0;
	}

	InheritParentAttributes(*pComboBox, pParent);

	IScriptObject *pSkinObject = m_pScriptSystem->CreateEmptyObject();

	if (pObject->GetValue("skin", pSkinObject))
	{
		SetupComboBoxFromTable(*pComboBox, pSkinObject);
	}

	pSkinObject->Release();

	SetupComboBoxFromTable(*pComboBox, pObject);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetupComboBoxFromTable(CUIComboBox *pComboBox, IScriptObject *pObject)
{
	// get the specific attributes
	char	*szKeyName;
	char	szAttributeName[128];

	// to hold the values
	char	*szValue;
	float	fValue;
	int		iValue;

	pObject->BeginIteration();

	while(pObject->MoveNext())
	{
		pObject->GetCurrentKey(szKeyName);

		strcpy(szAttributeName, szKeyName);
		strlwr(szAttributeName);

		pObject->GetCurrent(szValue);
		pObject->GetCurrent(fValue);
		pObject->GetCurrent(iValue);

		//------------------------------------------------------------------------------------------------- 
		if (CHECKATTRIBUTE("buttonsize", svtNumber))
		{
			pComboBox->m_fButtonSize = fValue;
		}
		else if (CHECKATTRIBUTE("itemheight", svtNumber))
		{
			pComboBox->m_fItemHeight = fValue;
		}
		else if (CHECKATTRIBUTE("maxitems", svtNumber))
		{
			pComboBox->m_iMaxItems = iValue;
		}
		else if (CHECKATTRIBUTE("rollup", svtNumber))
		{
			pComboBox->m_iRollUp = iValue;
		}
		else if (CHECKATTRIBUTE("leftspacing", svtNumber))
		{
			pComboBox->m_fLeftSpacing = fValue;
		}
		else if (CHECKATTRIBUTE("valign", svtNumber))
		{
			pComboBox->m_iVAlignment = iValue;
		}
		else if (CHECKATTRIBUTE("halign", svtNumber))
		{
			pComboBox->m_iHAlignment = iValue;
		}
		else if (CHECKATTRIBUTE("buttoncolor", svtString))
		{
			RetrieveColor(&pComboBox->m_cButtonColor, szValue);
		}
		else if (CHECKATTRIBUTE("itembgcolor", svtString))
		{
			RetrieveColor(&pComboBox->m_cItemBgColor, szValue);
		}
		else if (RetrieveTextureAttribute(&pComboBox->m_pTexture, pObject, "texture", "texrect"))
		{
		}
		else if (RetrieveTextureAttribute(&pComboBox->m_pButtonTexture, pObject, "buttontexture", "buttontexrect"))
		{
		}
		else if (RetrieveTextureAttribute(&pComboBox->m_pItemBg, pObject, "itembg", "itembgtexrect"))
		{
		}
		else if (!RetrieveCommonAttribute(pObject, pComboBox))
		{
			m_pLog->LogToConsole("\001$5[Warning]:$1 %s unknown attribute/value: '%s'", pComboBox->GetName().c_str(), szKeyName);
		}
		//------------------------------------------------------------------------------------------------- 
	}

	pObject->EndIteration();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateVideoPanelFromTable(CUIVideoPanel **pVideoPanel, CUIWidget *pParent, const UIRect &pRect, IScriptObject *pObject, const string &szName)
{
	if (!CreateVideoPanel(pVideoPanel, pParent, szName, pRect, UIFLAG_DEFAULT, 0))
	{
		return 0;
	}

	InheritParentAttributes(*pVideoPanel, pParent);

	IScriptObject *pSkinObject = m_pScriptSystem->CreateEmptyObject();

	if (pObject->GetValue("skin", pSkinObject))
	{
		SetupVideoPanelFromTable(*pVideoPanel, pSkinObject);
	}

	pSkinObject->Release();

	SetupVideoPanelFromTable(*pVideoPanel, pObject);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::SetupVideoPanelFromTable(CUIVideoPanel *pVideoPanel, IScriptObject *pObject)
{
	// get the specific attributes
	char	*szKeyName;
	char	szAttributeName[128];

	// to hold the values
	char	*szValue;
	float	fValue;
	int		iValue;

	float					fVolume = -1.0f;
	float					fPan = 0.0f;
	int						iFrameRate = -1;
	string		szVideo = "";
	int						iPlayNow = 0;
	int						iPlaySound = 0;

	pObject->BeginIteration();

	while(pObject->MoveNext())
	{
		pObject->GetCurrentKey(szKeyName);

		strcpy(szAttributeName, szKeyName);
		strlwr(szAttributeName);

		pObject->GetCurrent(szValue);
		pObject->GetCurrent(fValue);
		pObject->GetCurrent(iValue);

		//------------------------------------------------------------------------------------------------- 
		if (CHECKATTRIBUTE("framerate", svtNumber))
		{
			iFrameRate = iValue;
		}
		else if (CHECKATTRIBUTE("playsound", svtNumber))
		{
			iPlaySound = iValue;
		}
		else if (CHECKATTRIBUTE("playnow", svtNumber))
		{
			iPlayNow = iValue;
		}
		else if (CHECKATTRIBUTE("looping", svtNumber))
		{
			pVideoPanel->m_bLooping = (iValue != 0);
		}
		else if (CHECKATTRIBUTE("keepaspect", svtNumber))
		{
			pVideoPanel->m_bKeepAspect = (iValue != 0);
		}
		else if (CHECKATTRIBUTE("volume", svtNumber))
		{
			fVolume = fValue;
		}
		else if (CHECKATTRIBUTE("video", svtString))
		{
			szVideo = szValue;
		}
		else if (RetrieveTextureAttribute(&pVideoPanel->m_pOverlay, pObject, "overlay", "overlaytexrect"))
		{
		}
		else if (!RetrieveCommonAttribute(pObject, pVideoPanel))
		{
			m_pLog->LogToConsole("\001$5[Warning]:$1 %s unknown attribute/value: '%s'", pVideoPanel->GetName().c_str(), szKeyName);
		}
		//------------------------------------------------------------------------------------------------- 
	}

	pObject->EndIteration();

	if (iFrameRate > 0)
	{
		pVideoPanel->SetFrameRate(iFrameRate);
	}

	if (szVideo.size())
	{
		pVideoPanel->LoadVideo(szVideo, iPlaySound != 0);
	}

	pVideoPanel->SetVolume(1, fVolume);
	pVideoPanel->SetPan(1, fPan);

	if (iPlayNow)
	{
		pVideoPanel->Play();
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::CreateScreenFromTable(CUIScreen **pScreen, const string &szName, IScriptObject *pObject)
{
	if (!CreateScreen(pScreen, szName))
	{
		pObject->Release();

		return 0;
	}

	char *szKeyName;

	pObject->BeginIteration();

	while(pObject->MoveNext())
	{
		pObject->GetCurrentKey(szKeyName);

		if (IsReserved(szKeyName))
		{
			continue;
		}

		switch (pObject->GetCurrentType())
		{
		case svtNull:
			break;
		case svtFunction:
			break;
		case svtNumber:
			break;
		case svtString:
			break;
		case svtUserData:
			break;

			// create a new object
		case svtObject:
			{
				IScriptObject *pNewObject = m_pScriptSystem->CreateEmptyObject();

				pObject->GetCurrent(pNewObject);

				if ((*pScreen)->GetWidget(szKeyName))
				{
					pNewObject->Release();
					pObject->EndIteration();

					m_pLog->LogToConsole("\001$4[Error]:$1 Widget name already exists: '%s'", szKeyName);

					return 0;
				}

				CUIWidget *pWidget = 0;

				if (!CreateObjectFromTable(&pWidget, 0, *pScreen, pNewObject, szKeyName))
				{
					pObject->EndIteration();
					pNewObject->Release();

					return 0;
				}

				pNewObject->Release();

				if (pWidget)
				{
					pObject->SetValue(pWidget->GetName().c_str(), GetWidgetScriptObject(pWidget));
				}
			}		
			break;
		}
	}

	pObject->EndIteration();

	(*pScreen)->GetScriptObject()->Clone(pObject);
	(*pScreen)->OnInit();

	return 1;
}
#undef CHECKATTRIBUTE

//------------------------------------------------------------------------------------------------- 
int CUISystem::ConvertToWString(wstring &szWString, const char *szString)
{
	szWString.clear();

	((CXGame *)m_pSystem->GetIGame())->m_StringTableMgr.Localize(szString, szWString);
	
	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ConvertToWString(wstring &szWString, IFunctionHandler *pH, int iParam)
{
	char *szString;
	char szValue[32];

	if (!pH->GetParam(iParam, szString))
	{
		int iValue;

		pH->GetParam(iParam, iValue);

		itoa(iValue, szValue, 10);
		szString = szValue;
	}

	ConvertToWString(szWString, szString);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ConvertToWString(wstring &szWString, int iStrID)
{
	szWString = ((CXGame *)m_pSystem->GetIGame())->m_StringTableMgr.EnumString(iStrID);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ConvertToString(char *szString, const color4f &pColor)
{
	sprintf(szString, "%d, %d, %d, %d", (int)(pColor.v[0] * 255.0f), (int)(pColor.v[1] * 255.0f), (int)(pColor.v[2] * 255.0f), (int)(pColor.v[3] * 255.0f));

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ConvertToString(char *szString, const UIRect &pRect)
{
	sprintf(szString, "%f, %f, %f, %f", pRect.fLeft, pRect.fTop, pRect.fWidth, pRect.fHeight);

	return 1;
}

int CUISystem::ConvertToString(char *szString, const wstring &szWString, int iMaxSize)
{
	int iSize = szWString.size();

	if (iMaxSize > 0)
	{
		iSize = min(iMaxSize, iSize);
	}

	wchar_t *pChar = (wchar_t *)szWString.c_str();

	szString[iSize] = 0;
	while (iSize--)
	{
		// convert special unicode characters that look like ascii but they aren't
		if (pChar[iSize] > 0xff00 && pChar[iSize] < 0xff5e)
		{
			char cChar = pChar[iSize]-0xfee0;

			if (cChar != 0)
			{
				szString[iSize] = cChar;
			}
			else
			{
				szString[iSize] = '_';
			}
		}
		// add other special cases here
		else
		{
			char cChar = (char)pChar[iSize];

			if (cChar != 0)
			{
				szString[iSize] = cChar;
			}
			else
			{
				szString[iSize] = '_';
			}
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::ConvertToString(string &szString, const wstring &szWString)
{
	szString.clear();

	wchar_t *pChar = (wchar_t *)szWString.c_str();

	while (*pChar)
	{
		szString.push_back((char)*pChar++);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::StripControlCodes(wstring &szOutString, const wstring &szWString)
{
	szOutString.clear();

	wchar_t *pChar = (wchar_t *)szWString.c_str();

	while (*pChar)
	{
		if (*pChar == L'$')
		{
			if (*(pChar + 1))
			{
				++pChar;
			}

			if (*pChar == L'$')
			{
				szOutString.push_back(*pChar);
			}
		}
		else
		{
			szOutString.push_back(*pChar);
		}

		++pChar;
	}

	return 1;
}

int CUISystem::StripControlCodes(string &szOutString, const wstring &szWString)
{
	szOutString.clear();

	wchar_t *pChar = (wchar_t *)szWString.c_str();

	while (*pChar)
	{
		if (*pChar == L'$')
		{
			if (*(pChar + 1))
			{
				++pChar;
			}

			if (*pChar == L'$')
			{
				szOutString.push_back((char)*pChar);
			}
		}
		else
		{
			szOutString.push_back((char)*pChar);
		}

		++pChar;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUISystem::StripControlCodes(string &szOutString, const string &szString)
{
	szOutString.clear();

	char *pChar = (char *)szString.c_str();

	while (*pChar)
	{
		if (*pChar == '$')
		{
			if (*(pChar + 1))
			{
				++pChar;
			}

			if (*pChar == '$')
			{
				szOutString.push_back(*pChar);
			}
		}
		else
		{
			szOutString.push_back(*pChar);
		}

		++pChar;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
void CUISystem::DeleteWidget(CUIWidget *pWidget)
{
	delete pWidget;
}

//------------------------------------------------------------------------------------------------- 
bool CUISystem::IsReserved(const char *szName)
{
	if ((strcmp(szName, "user") == 0) ||
			(strcmp(szName, "skin") == 0))
	{
		return 1;
	}
	return 0;
}

//------------------------------------------------------------------------------------------------- 
bool CUISystem::IsOnFocusScreen(CUIWidget *pWidget)
{
	if (m_pFocusScreen)
	{
		return (pWidget->m_pScreen == m_pFocusScreen);
	}
	else if ((pWidget->m_pScreen && IsScreenActive(pWidget->m_pScreen)) || (!pWidget->m_pScreen))
	{
		return 1;
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
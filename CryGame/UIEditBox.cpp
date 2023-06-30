//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - An EditBox
//
// History:
//  - [20/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#include "StdAfx.h"
#include "UIEditBox.h"
#include "UISystem.h"



_DECLARE_SCRIPTABLEEX(CUIEditBox);


//------------------------------------------------------------------------------------------------- 
// Control and Shift Modifier Checking
//------------------------------------------------------------------------------------------------- 

#define CONTROLDOWN	(m_pUISystem->GetIInput()->KeyDown(XKEY_LCONTROL) || m_pUISystem->GetIInput()->KeyDown(XKEY_RCONTROL))
#define SHIFTDOWN	(m_pUISystem->GetIInput()->KeyDown(XKEY_LSHIFT) || m_pUISystem->GetIInput()->KeyDown(XKEY_RSHIFT))



//------------------------------------------------------------------------------------------------- 
CUIEditBox::CUIEditBox()
: m_iHAlignment(UIALIGN_LEFT),
	m_iVAlignment(UIALIGN_MIDDLE),
  m_cCursorColor(color4f(0.0f, 0.0f, 0.0f, 1.0f)),
	m_iSelectionStart(0),
	m_iSelectionCount(0),
	m_iCursorPos(0),
	m_fLeftSpacing(0),
	m_fRightSpacing(0),
	m_bMouseSelecting(0),
	m_bMouseSelectingAll(0),
	m_iMouseSelectionStart(0),
  m_cSelectionColor(0.0f, 0.0f, 0.8f, 0.8f),
	m_iPathSafe(0),
  m_iNameSafe(0),
	m_iUbiSafe(0),
  m_iNumeric(0),
  m_iMaxLength(0)
{
}

//------------------------------------------------------------------------------------------------- 
CUIEditBox::~CUIEditBox()
{
}

//------------------------------------------------------------------------------------------------- 
string CUIEditBox::GetClassName()
{
	return UICLASSNAME_EDITBOX;
}

//------------------------------------------------------------------------------------------------- 
LRESULT CUIEditBox::Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam)		//AMD Port
{
	switch (iMessage)
	{
	case UIM_LBUTTONDBLCLICK:
		{
			m_bMouseSelectingAll = 1;
			m_iSelectionStart = 0;
			m_iSelectionCount = m_szText.size();
		}
		return 1;
	case UIM_KEYUP:
		if ((lParam == XKEY_RETURN) || (lParam == XKEY_NUMPADENTER))
		{
			OnCommand();
		}
		return 1;
	case UIM_KEYDOWN:
		return ProcessInput(iMessage, lParam, (char *)wParam);
	case UIM_LBUTTONDOWN:
		{
			if (m_bMouseSelectingAll)
			{
				break;
			}

			IFFont *pFont = m_pUISystem->GetIFont(m_pFont);
			UIRect pTextRect = GetTextRect();

			int iCursorPosition = GetCursorPosition(UIM_GET_X_FLOAT(wParam), UIM_GET_Y_FLOAT(wParam), pTextRect, pFont);

			if (iCursorPosition == -1)
			{
					iCursorPosition = m_iCursorPos;
			}
	
			if (!m_iSelectionCount && (m_pUISystem->GetIInput()->KeyDown(XKEY_LSHIFT) || m_pUISystem->GetIInput()->KeyDown(XKEY_RSHIFT)))
			{
				m_iSelectionStart = min(m_iCursorPos, iCursorPosition);
				m_iSelectionCount = abs(iCursorPosition - m_iCursorPos);
			}
			else if (!m_bMouseSelecting)
			{
				m_iMouseSelectionStart = iCursorPosition;
				m_iSelectionStart = iCursorPosition;
				m_iSelectionCount = 0;
				m_bMouseSelecting = 1;
			}
			else
			{
				m_iSelectionCount = abs(m_iMouseSelectionStart - iCursorPosition);

				if (iCursorPosition <= m_iMouseSelectionStart)
				{
					m_iSelectionStart = iCursorPosition;
				}
			}

			m_iCursorPos = iCursorPosition;
		}
		break;
	case UIM_MOUSEUP:
		{
			m_bMouseSelectingAll = 0;
			m_bMouseSelecting = 0;
		}
		break;
	default:
		break;
	}

	return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::Draw(int iPass)
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
		// if we are a textured button, draw the correct texture
		if (m_pTexture.iTextureID > -1)
		{
			m_pUISystem->DrawImage(pAbsoluteRect, m_pTexture, m_cColor);
		}
		// if not textured, just draw the back quad
		else
		{
			m_pUISystem->DrawQuad(pAbsoluteRect, m_cColor);
		}
	}

	// get the font with the correct properties set
	IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

	// get the text rectangle
	UIRect pTextRect = UIRect(pAbsoluteRect.fLeft+m_fLeftSpacing, pAbsoluteRect.fTop, pAbsoluteRect.fWidth-m_fLeftSpacing-m_fRightSpacing, pAbsoluteRect.fHeight);

	m_pUISystem->AdjustRect(&pTextRect, pTextRect, UI_DEFAULT_TEXT_BORDER_SIZE);
	m_pUISystem->SetScissor(&pTextRect);
 
	// if we have text, draw it
	if (!m_szText.empty())
	{
		// if we have selected text, draw the selection background
		if (m_iSelectionCount)
		{
			DrawSelection(m_iSelectionStart, m_iSelectionCount, pFont, pTextRect);
		}

		if (m_iStyle & UISTYLE_PASSWORD)
		{
			wchar_t szPassword[256] = {L'*'};

			int iSize = min(m_szText.size(), 255);
			szPassword[iSize] = 0;

			for (int i = 1; i < iSize; i++)
			{
				szPassword[i] = szPassword[0];
			}

			m_pUISystem->DrawText(pTextRect, m_iHAlignment, m_iVAlignment, pFont, szPassword, 0);
		}
		else
		{
			m_pUISystem->DrawText(pTextRect, m_iHAlignment, m_iVAlignment, pFont, m_szText.c_str(), 0);
		}
	}

	if (GetFlags() & UIFLAG_HAVEFOCUS)
	{
		// draw the blinking cursor
		if (cry_sinf(m_pUISystem->GetISystem()->GetITimer()->GetCurrTime() * 3.0f * UI_DEFAULT_CURSOR_BLINK_SPEED) > 0.0f)
		{
			// get the cursor position
			float fCursorX, fCursorY, fCursorHeight;

			GetCursorCoord(&fCursorX, &fCursorY, &fCursorHeight, pTextRect, pFont);

			if (m_pUISystem->PointInRect(pTextRect, fCursorX, fCursorY))
			{
				DrawCursor(pTextRect, pFont, fCursorX, fCursorY, fCursorHeight);
			}
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
void CUIEditBox::InitializeTemplate(IScriptSystem *pScriptSystem)
{
	_ScriptableEx<CUIEditBox>::InitializeTemplate(pScriptSystem);

	REGISTER_COMMON_MEMBERS(pScriptSystem, CUIEditBox);

	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetText);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetText);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, Clear);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetMaxLength);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetMaxLength);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetTextLength);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetVAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetVAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetHAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetHAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetSelectionStart);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetSelectionStart);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetSelectionCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetSelectionCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SelectAll);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, DeselectAll);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetCursorPosition);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetCursorPosition);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, Cut);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, CopyToClipboard);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, CutToClipboard);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, PasteFromClipboard);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetCursorColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetCursorColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetCursorColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, GetCursorColor);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetNumeric);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetPathSafe);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetNameSafe);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIEditBox, SetPassword);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::Cut()
{
	if (m_iSelectionCount < 1)
	{
		return 0;
	}
	
	m_szText.erase(m_iSelectionStart, m_iSelectionCount);
	m_iCursorPos = m_iSelectionStart;

	m_iSelectionStart = m_iSelectionCount = 0;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::CopyToClipboard()
{
#if !defined(LINUX)
	if (m_szText.empty())
	{
		return 0;
	}
	
	if (!OpenClipboard(0))
	{
		return 0;
	}

	HGLOBAL hGlobal;
	LPVOID	pGlobal;
	int		iTextSize = m_szText.size();

	if (GetSelectedCount())
	{
		iTextSize = GetSelectedCount();
	}

	hGlobal = GlobalAlloc(GHND, (iTextSize + 1) * sizeof(wchar_t));
	pGlobal = GlobalLock (hGlobal);

	memcpy(pGlobal, &m_szText.c_str()[GetSelectionStart()], iTextSize * sizeof(wchar_t));
	((short *)pGlobal)[iTextSize] = 0;

	GlobalUnlock(hGlobal);

	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, hGlobal);
	CloseClipboard();

	OnChanged();
#endif
	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::CutToClipboard()
{
	if (CopyToClipboard())
	{
		Cut();
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::PasteFromClipboard()
{
#if !defined(LINUX)
	HGLOBAL	hGlobal;
	LPVOID	pGlobal;
	bool	bUnicode = true;

	if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		if (!IsClipboardFormatAvailable(CF_TEXT))
		{
			return 0;
		}
	}

	if (!OpenClipboard(0))
	{
		return 0;
	}

	hGlobal = GetClipboardData(CF_UNICODETEXT);

	if (!hGlobal)
	{
		hGlobal = GetClipboardData(CF_TEXT);

		if (!hGlobal)
		{
			return 0;
		}

		bUnicode = false;
	}

	pGlobal = GlobalLock(hGlobal);

	if (bUnicode)
	{
		wchar_t *pString = (wchar_t *)pGlobal;
		wchar_t pWString[2048];
		wchar_t *p = pWString;


		int iLength = 0;
		
		for (int i = 0; (pString[i] != '\n') && (pString[i] != '\r') && (pString[i] != '\0'); i++)
		{
			if (CheckChar(pString[i]))
			{
				++iLength;
				*p++ = pString[i];
			}
		}

		*p = 0;

		if (m_iMaxLength)
		{
			iLength = min(iLength, (m_iMaxLength - (int)m_szText.size()));
		}
		
		m_szText.insert(m_iCursorPos, pWString, iLength);
	
		m_iCursorPos += iLength;
	}
	else
	{
		char *pString = (char *)pGlobal;
		wchar_t pWString[2048];
		wchar_t *p = pWString;

		// convert char to wchar_t

		int i = 0;
		int iLength = 0;
		for (; pString[i] != 0; i++)
		{
			if ((pString[i] == '\n') || (pString[i] == '\r'))
			{
				break;
			}

			if (CheckChar((wchar_t)((unsigned char)pString[i])))
			{
				++iLength;
				*p++ = (unsigned char)pString[i];
			}
		}		

		*p = 0;

		if (m_iMaxLength)
		{
			iLength = min(iLength, (m_iMaxLength - (int)m_szText.size()));
		}

		m_szText.insert(m_iCursorPos, pWString, iLength);

		m_iCursorPos += i;
	}

	GlobalUnlock(hGlobal);
	CloseClipboard();

	OnChanged();
#endif
	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetSelectionStart()
{
	return m_iSelectionStart;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetSelectedCount()
{
	return m_iSelectionCount;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetText(const wstring &szText)
{
	m_szText.resize(0);

	const wchar_t *pString = szText.c_str();

	for (int i = 0; (pString[i] != '\n') && (pString[i] != '\r') && (pString[i] != '\0'); i++)
	{
		if (CheckChar(pString[i]))
		{
			m_szText.push_back(pString[i]);

			if (m_iMaxLength && ((int)m_szText.size() >= m_iMaxLength))
			{
				break;
			}
		}
	}

	m_iCursorPos = m_szText.size();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetText(wstring &szText)
{
	szText = m_szText;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetText(string &szText)
{
	m_pUISystem->ConvertToString(szText, m_szText);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
int CUIEditBox::DrawCursor(const UIRect &pTextRect, IFFont *pFont, float fX, float fY, float fHeight)
{
	m_pUISystem->GetIRenderer()->SetMaterialColor(m_cCursorColor.v[0], m_cCursorColor.v[1], m_cCursorColor.v[2], m_cCursorColor.v[3]);
	m_pUISystem->GetIRenderer()->Draw2dLine(fX, fY, fX, fY + fHeight);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::DrawSelection(int iStart, int iCount, IFFont *pFont, const UIRect &pTextRect)
{
	wstring pSelectedStr;
	wstring pPreSelectionStr;

	float fX, fY;

	if (m_iStyle & UISTYLE_PASSWORD)
	{
		wstring szwPassword;
		int i = m_szText.size();

		szwPassword.reserve(i);
		while(--i >= 0)
			szwPassword.insert(0, L"*");

		m_pUISystem->GetAlignedTextXY(&fX, &fY, pFont, pTextRect, szwPassword.c_str(), m_iHAlignment, m_iVAlignment);

		pSelectedStr = szwPassword.substr(iStart, iCount);
		pPreSelectionStr = szwPassword.substr(0, iStart);
	}
	else
	{
		m_pUISystem->GetAlignedTextXY(&fX, &fY, pFont, pTextRect, m_szText.c_str(), m_iHAlignment, m_iVAlignment);

		pSelectedStr = m_szText.substr(iStart, iCount);
		pPreSelectionStr = m_szText.substr(0, iStart);
	}

	// get text sizes
	vector2f vPreSelectionSize = pFont->GetTextSizeW(pPreSelectionStr.c_str());
	vector2f vSelectionSize = pFont->GetTextSizeW(pSelectedStr.c_str());

	float fRcpScaleX = 1.0f / m_pUISystem->GetIRenderer()->ScaleCoordX(1);
	float fRcpScaleY = 1.0f / m_pUISystem->GetIRenderer()->ScaleCoordY(1);

	vPreSelectionSize.x *= fRcpScaleX;
	vPreSelectionSize.y *= fRcpScaleY;

	vSelectionSize.x *= fRcpScaleX;
	vSelectionSize.y *= fRcpScaleY;

	fX += vPreSelectionSize.x;
		
	UIRect pRect(fX, fY, (float)vSelectionSize.x, (float)vSelectionSize.y);

	m_pUISystem->DrawQuad(pRect, m_cSelectionColor);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SelectLeft()
{
	int iOldCursor = m_iCursorPos;

	Left();

	int iDelta = iOldCursor - m_iCursorPos;

	if (iDelta)
	{
		if (m_iSelectionCount)
		{
			if (m_iSelectionStart < iOldCursor)
			{
				m_iSelectionCount -= iDelta;
			}
			else
			{
				m_iSelectionStart -= iDelta;
				m_iSelectionCount += iDelta;
			}
		}
		else
		{
			m_iSelectionStart = m_iCursorPos;
			m_iSelectionCount = iDelta;
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SelectRight()
{
	int iOldCursor = m_iCursorPos;

	Right();

	int iDelta = m_iCursorPos - iOldCursor;

	if (iDelta)
	{
		if (m_iSelectionCount)
		{
			if (m_iSelectionStart < iOldCursor)
			{
				m_iSelectionCount += iDelta;
			}
			else
			{
				m_iSelectionStart += iDelta;
				m_iSelectionCount -= iDelta;
			}
		}
		else
		{
			m_iSelectionStart = iOldCursor;
			m_iSelectionCount = iDelta;
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::Backspace()
{
	if (m_iSelectionCount)
	{
		Cut();

		return 1;
	}

	if (m_iCursorPos > 0)
	{
		if ((m_iCursorPos - 1 > 0) && ((m_iStyle & UISTYLE_PASSWORD) == 0))
		{
			if (m_szText[m_iCursorPos-2] == '$')
			{
				m_szText.erase(m_iCursorPos-1, 1);
				m_iCursorPos--;
			}
		}
		
		m_szText.erase(m_iCursorPos-1, 1);

		m_iCursorPos--;

		OnChanged();
	}

	return 1;
}

int CUIEditBox::Delete()
{
	if (m_iSelectionCount)
	{
		Cut();
	}
	else
	{
		if (m_iCursorPos < (int)m_szText.size())
		{
			if ((m_szText[m_iCursorPos] == '$') && (m_iCursorPos < (int)m_szText.size()-1) && ((m_iStyle & UISTYLE_PASSWORD) == 0))
			{
				m_szText.erase(m_iCursorPos, 1);				
			}
			m_szText.erase(m_iCursorPos, 1);

			OnChanged();
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::Left()
{
	if ((m_iCursorPos > 1) && ((m_iStyle & UISTYLE_PASSWORD) == 0))
	{
		if (m_szText[m_iCursorPos-2] == '$')
		{
			m_iCursorPos -= 2;
		}
		else
		{
			m_iCursorPos--;
		}
	}
	else if (m_iCursorPos > 0)
	{
		m_iCursorPos--;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::Right()
{
	if ((m_iCursorPos + 1 < (int)m_szText.size()) && ((m_iStyle & UISTYLE_PASSWORD) == 0))
	{
		if (m_szText[m_iCursorPos] == '$')
		{
			m_iCursorPos += 2;
		}
		else
		{
			m_iCursorPos++;
		}
	}
	else if (m_iCursorPos < (int)m_szText.size())
	{
		m_iCursorPos++;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::InsertChar(wchar_t cChar)
{
	if (((m_iMaxLength) && ((int)m_szText.size() < m_iMaxLength)) || (!m_iMaxLength))
	{
		m_szText.insert(m_iCursorPos, &cChar, 1);

		m_iCursorPos++;

		OnChanged();
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::ProcessInput(unsigned int iMessage, int iKeyCode, char *szKeyName)
{
	bool bProcess = 1;
	// selection stuff
	if (SHIFTDOWN || CONTROLDOWN)
	{
		if (SHIFTDOWN)
		{
			switch(iKeyCode)
			{
			case XKEY_LEFT:
				SelectLeft();
				break;
			case XKEY_RIGHT:
				SelectRight();
				break;
			case XKEY_HOME:
				m_iSelectionStart = 0;
				m_iSelectionCount = m_iCursorPos;
				m_iCursorPos = 0;
				break;
			case XKEY_END:
				if (!m_iSelectionCount)
				{
					m_iSelectionStart = m_iCursorPos;
				}

				m_iSelectionCount = m_szText.size() - m_iSelectionStart;
				m_iCursorPos = (int)m_szText.size();
				break;
			case XKEY_INSERT:
				PasteFromClipboard();
				break;
			case XKEY_DELETE:
				CutToClipboard();
				break;
			default:
				break;
			}
		}
		if (CONTROLDOWN)
		{
			switch (iKeyCode)
			{
			case XKEY_C:
				CopyToClipboard();
				bProcess = 0;
				break;
			case XKEY_X:
				CutToClipboard();
				bProcess = 0;
				break;
			case XKEY_V:
				PasteFromClipboard();
				bProcess = 0;
				break;
			case XKEY_INSERT:
				CopyToClipboard();
				break;
			default:
				break;
			}
		}
	}
	else
	{
		switch (iKeyCode)
		{
		case XKEY_LEFT:
			Left();
			m_iSelectionStart = 0;
			m_iSelectionCount = 0;
			m_bMouseSelecting = 0;
			break;
		case XKEY_RIGHT:
			Right();
			m_iSelectionStart = 0;
			m_iSelectionCount = 0;
			m_bMouseSelecting = 0;
			break;
		case XKEY_BACKSPACE:
			Backspace();
			break;
		case XKEY_DELETE:
			Delete();
			break;
		case XKEY_HOME:
			m_iCursorPos = 0;
			m_iSelectionCount = 0;
			m_iSelectionStart = 0;
			m_bMouseSelecting = 0;
			break;
		case XKEY_END:
			m_iCursorPos = (int)m_szText.size();
			m_iSelectionCount = 0;
			m_iSelectionStart = 0;
			m_bMouseSelecting = 0;
			break;
		case XKEY_RETURN:
			break;
		default:
			break;
		}
	}

	if (bProcess && ((szKeyName) && (*szKeyName) && (szKeyName[1] == 0)) || (iKeyCode == XKEY_SPACE))
	{
		if (m_iSelectionCount)
		{
			Delete();
		}

		if (CheckChar((unsigned char)*szKeyName))
		{
			//InsertChar((iKeyCode == XKEY_SPACE ? L' ' : (unsigned char)*szKeyName));
			if (iKeyCode == XKEY_SPACE)
			{
				InsertChar( L' ' );
			}
			else
			{
				wchar_t szwStr[256];
				int iLength = strlen(szKeyName);
#if defined(LINUX)
				mbstowcs(szwStr, szKeyName, iLength);
				szwStr[iLength] = 0;
#else
				MultiByteToWideChar(CP_ACP, 0, szKeyName, -1, szwStr, 256);
#endif
				InsertChar(szwStr[0] );
			}
		}
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::CheckChar(wchar_t cChar)
{
	char cAsciiChar = (char)cChar;

	if (cAsciiChar == 0)
	{
		return 0;
	}

	if (!m_szAllow.empty())
	{
		if (m_szAllow.find(cAsciiChar) == m_szAllow.npos)
		{
			return 0;
		}
	}

	if (!m_szDisallow.empty())
	{
		if (m_szDisallow.find(cAsciiChar) != m_szDisallow.npos)
		{
			return 0;
		}
	}

	if (m_iNumeric)
	{
		if (!isdigit(cChar) && (cChar != '.'))
		{
			return 0;
		}

		if ((cChar == '.') && (m_szText.find('.') != m_szText.npos))
		{
			return 0;
		}
	}

	if (m_iPathSafe)
	{
		switch(cChar)
		{
			// chars that windows does not allow
		case '\\':
		case '/':
		case ':':
		case '*':
		case '?':
		case '\"':
		case '<':
		case '>':
		case '|':
			// chars that may cause problems with our game
		case '@':
		case '#':
		case '%':
			return 0;
		}
	}

	if (m_iNameSafe)
	{
		switch(cChar)
		{
			// chars that may cause problems with our game
			case '@':
			case '#':
			case '%':
				return 0;
		}
	}

	if (m_iUbiSafe)
	{
		if (isalnum(cChar))
		{
			return 1;
		}
		else if (cChar == '.' || cChar == '_' || cChar == '-')
		{
			return 1;
		}

		return 0;
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetCursorCoord(float *fX, float *fY, float *fHeight, const UIRect &pTextRect, IFFont *pFont)
{
	m_pUISystem->GetAlignedTextXY(fX, fY, pFont, pTextRect, m_szText.c_str(), m_iHAlignment, m_iVAlignment);

	vector2f vTextSize;

	if (m_iStyle & UISTYLE_PASSWORD)
	{
		wstring szwPassword;
		int i = m_iCursorPos;

		szwPassword.reserve(i);
		while(--i >= 0)
			szwPassword.insert(0, L"*");

		vTextSize = pFont->GetTextSizeW(szwPassword.c_str());
	}
	else
	{
		wstring pStr(m_szText.substr(0, m_iCursorPos));
		vTextSize = pFont->GetTextSizeW(pStr.c_str());
	}

	
	vTextSize.x /= m_pUISystem->GetIRenderer()->ScaleCoordX(1.0f);
	vTextSize.y /= m_pUISystem->GetIRenderer()->ScaleCoordY(1.0f);

	*fX += vTextSize.x;
	*fHeight = (float)vTextSize.y;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetCursorPosition(float fAtX, float fAtY, const UIRect &pTextRect, IFFont *pFont)
{
	if (m_pUISystem->PointInRect(pTextRect, fAtX, fAtY))
	{
		float fX, fY, fCharWidth;
		float fRcpScaleX = 1.0f / m_pUISystem->GetIRenderer()->ScaleCoordX(1.0);
		wchar_t pChar[2] = {0, 0};

		m_pUISystem->GetAlignedTextXY(&fX, &fY, pFont, pTextRect, m_szText.c_str(), m_iHAlignment, m_iVAlignment);

		for (int i = 0; i < (int)m_szText.size(); i++)
		{
			pChar[0] = m_szText[i];

			if (pChar[0] == L'$')
			{
				++i;
				if (m_szText[i] != '$')
				{
					continue;
				}
			}
			
			fCharWidth = pFont->GetTextSizeW(pChar).x * fRcpScaleX;

			if (fAtX >= fX && fAtX < fX + fCharWidth)
			{
				return i;
			}

			fX += fCharWidth;
		}

		if (fAtX > fX)
		{
			m_iCursorPos = m_szText.size();
		}
	}

	return -1;
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetStringLength(const wchar_t *pString)
{
	int iStringSize = 0;

	for (; pString[iStringSize] != 0; iStringSize++);

	return iStringSize;
}

//------------------------------------------------------------------------------------------------- 
// Script Functions
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetText(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetText, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), SetText, 1, svtString, svtNumber);

	wstring szText;

	m_pUISystem->ConvertToWString(szText, pH, 1);
	
	SetText(szText);

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetText(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetText, 0);

	if (m_szText.empty())
	{
		return pH->EndFunctionNull();
	}

	char	szString[1024] = {0,0};
	m_pUISystem->ConvertToString(szString, m_szText, 1024);

	return pH->EndFunction(szString);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::Clear(IFunctionHandler *pH)
{
	m_szText.clear();

	m_iCursorPos = 0;
	m_iSelectionCount = 0;
	m_iSelectionStart = 0;

	return pH->EndFunctionNull();
}
//------------------------------------------------------------------------------------------------- 
UIRect CUIEditBox::GetTextRect()
{
	UIRect pTextRect(0, 0, m_pRect.fWidth, m_pRect.fHeight);

	if (m_pBorder.fSize > 0.125f)
	{
		m_pUISystem->AdjustRect(&pTextRect, pTextRect, m_pBorder.fSize);
	}

	pTextRect.fLeft += m_fLeftSpacing;
	pTextRect.fWidth -= m_fRightSpacing + m_fLeftSpacing;

	m_pUISystem->AdjustRect(&pTextRect, pTextRect, UI_DEFAULT_TEXT_BORDER_SIZE);

	return pTextRect;
}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetMaxLength(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetMaxLength, m_iMaxLength);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetMaxLength(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetMaxLength, m_iMaxLength);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetTextLength(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetTextLength, int(m_szText.size()));
}

int CUIEditBox::SetVAlign(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetVAlign, m_iVAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetVAlign(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetVAlign, m_iVAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetHAlign(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetHAlign, m_iHAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetHAlign(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetHAlign, m_iHAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetSelectionStart(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), SetSelectionStart, m_iSelectionStart);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetSelectionStart(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), GetSelectionStart, m_iSelectionStart);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetSelectionCount(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), SetSelectionCount, m_iSelectionCount);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetSelectionCount(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), GetSelectionCount, m_iSelectionCount);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SelectAll(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SelectAll, 0);

	m_iSelectionStart = 0;
	m_iSelectionCount = m_szText.size();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::DeselectAll(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), DeselectAll, 0);

	m_iSelectionStart = 0;
	m_iSelectionCount = 0;

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetCursorPosition(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), GetCursorPosition, m_iCursorPos);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetCursorPosition(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetCursorPosition, m_iCursorPos);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::Cut(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Cut, 0);

	Cut();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::CopyToClipboard(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), CopyToClipboard, 0);

	CopyToClipboard();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::CutToClipboard(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), CutToClipboard, 0);

	CutToClipboard();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::PasteFromClipboard(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), PasteFromClipboard, 0);

	PasteFromClipboard();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetTexture, m_pTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetTexture, m_pTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetCursorColor(IFunctionHandler *pH)
{
	RETURN_COLOR_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetCursorColor, m_cCursorColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::GetCursorColor(IFunctionHandler *pH)
{
	RETURN_COLOR_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetCursorColor, m_cCursorColor);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetNumeric(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetNumeric, m_iNumeric);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetPathSafe(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetPathSafe, m_iPathSafe);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetNameSafe(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetNameSafe, m_iNameSafe);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetUbiSafe(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetUbiSafe, m_iUbiSafe);
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetPassword(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetPassword, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetPassword, 1, svtNumber);

	int iPassword = 0;

	pH->GetParam(1, iPassword);

	if (iPassword)
	{
		m_iStyle |= UISTYLE_PASSWORD;
	}
	else
	{
		m_iStyle &= ~UISTYLE_PASSWORD;
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetAllow(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetAllow, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetAllow, 1, svtString);

	char *szAllow = 0;

	if (pH->GetParam(1, szAllow))
	{
		m_szAllow = szAllow;
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUIEditBox::SetDisallow(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetDisallow, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetDisallow, 1, svtString);

	char *szDisallow = 0;

	if (pH->GetParam(1, szDisallow))
	{
		m_szDisallow = szDisallow;
	}

	return pH->EndFunction();
}

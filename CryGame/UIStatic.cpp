//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - A Static Control
//
// History:
//  - [3/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#include "StdAfx.h"
#include "UIStatic.h"
#include "UIScrollBar.h"
#include "UISystem.h"
#include <CryCharAnimationParams.h>



_DECLARE_SCRIPTABLEEX(CUIStatic)


//------------------------------------------------------------------------------------------------- 
CUIStatic::CUIStatic()
: m_fVerticalOffset(0),
	m_fHorizontalOffset(0),
	m_bVerticalScrollBar(0),
	m_bHorizontalScrollBar(0),
  m_pHScroll(0),
	m_pVScroll(0),
	m_iHAlignment(UIALIGN_LEFT),
	m_iVAlignment(UIALIGN_MIDDLE),
  m_fTotalWidth(0.0f),
	m_fTotalHeight(0.0f),
	m_fLineHeight(0),
	m_fLineSpacing(0.0f),
	m_pModel(0),
  m_fAngle(0),
	m_fCameraDistance(4.0f),
	m_fCameraFov(35.0f * gf_PI / 180.0f),
	m_fModelRotation(0.1f),
  m_fLightDistance(50.0f),
	m_fModelRotationAcc(1.0f),
	m_fMouseMultiplier(1.0f),
	m_iMaxLines(100),
  m_fLeftSpacing(0.0f),
	m_fRightSpacing(0.0f)
{
}

//------------------------------------------------------------------------------------------------- 
CUIStatic::~CUIStatic()
{
	ReleaseModel();
}

//------------------------------------------------------------------------------------------------- 
string CUIStatic::GetClassName()
{
	return UICLASSNAME_STATIC;
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetStyle(int iStyle)
{
	return CUIWidget::SetStyle(iStyle);
}

//------------------------------------------------------------------------------------------------- 
LRESULT CUIStatic::Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam)	//AMD Port
{
	switch(iMessage)
	{
	case UIM_DRAW:
		{
			if (m_iStyle & UISTYLE_MULTILINE)
			{
				m_pVScroll = (CUIScrollBar *)GetChild("vscrollbar");
				m_pHScroll = (CUIScrollBar *)GetChild("hscrollbar");

				IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

				// get the rect where we are allowed to draw
				UIRect pTextRect = GetTextRect(1);
				UIRect pRect = GetTextRect(0);

				bool bVScroll = 0, bHScroll = 0;

				// get the total height
				m_fTotalHeight = GetTextHeight();
				m_fTotalWidth = GetTextWidth();

				// check if we need scrollbars
				bVScroll = (m_fTotalHeight > pTextRect.fHeight);
				bHScroll = (m_fTotalWidth > pTextRect.fWidth);

				if (bHScroll && !m_pHScroll)
				{
					m_pUISystem->CreateScrollBar(&m_pHScroll, this, "hscrollbar", UIRect(0, 0, 0, 16.0f), UIFLAG_ENABLED, 0, UISCROLLBARTYPE_HORIZONTAL);
				}

				if (bVScroll && !m_pVScroll)
				{
					m_pUISystem->CreateScrollBar(&m_pVScroll, this, "vscrollbar", UIRect(0, 0, 16.0f, 0.0f), UIFLAG_ENABLED, 0, UISCROLLBARTYPE_VERTICAL);
				}

				// set the bar rects and values
				if (bVScroll && !m_bVerticalScrollBar)
				{
					m_bVerticalScrollBar = 1;

					m_pVScroll->SetFlags(m_pVScroll->GetFlags() | UIFLAG_VISIBLE);

					if (m_iVAlignment == UIALIGN_BOTTOM)
					{
						m_pVScroll->SetValue(1.0f);
					}
					else if (m_iVAlignment == UIALIGN_MIDDLE)
					{
						m_pVScroll->SetValue(0.5f);
					}
					else
					{
						m_pVScroll->SetValue(0.0f);
					}

					GetLineListMetrics();
				}
				else if (!bVScroll && m_bVerticalScrollBar)
				{
					m_bVerticalScrollBar = 0;
				}

				// check if we need an horizontal scrollbar
				if (bHScroll && !m_bHorizontalScrollBar)
				{
					m_bHorizontalScrollBar = 1;

					m_pHScroll->SetFlags(m_pHScroll->GetFlags() | UIFLAG_VISIBLE);

					if (m_iHAlignment == UIALIGN_RIGHT)
					{
						m_pHScroll->SetValue(1.0f);
					}
					else if (m_iHAlignment == UIALIGN_CENTER)
					{
						m_pHScroll->SetValue(0.5f);
					}
					else
					{
						m_pHScroll->SetValue(0.0f);
					}
				}
				else if (!bHScroll && m_bHorizontalScrollBar)
				{
					m_bHorizontalScrollBar = 0;
				}

				// convert pRect to relative coordinates
				m_pUISystem->GetRelativeXY(&pRect.fLeft, &pRect.fTop, pRect.fLeft, pRect.fTop, this);

				// set step sizes and rects
				if (m_bVerticalScrollBar)
				{
					UIRect pVScrollRect;

					pVScrollRect.fLeft = pRect.fLeft + pRect.fWidth - m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;
					pVScrollRect.fTop = pRect.fTop;
					pVScrollRect.fWidth = m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;
					pVScrollRect.fHeight = pRect.fHeight - (m_bHorizontalScrollBar ? m_pUISystem->GetWidgetRect(m_pHScroll).fHeight : 0);

					m_pVScroll->SetRect(pVScrollRect, 1);
					m_pVScroll->SetStep(m_fLineHeight / (m_fTotalHeight - pTextRect.fHeight));
				}
				else
				{
					m_fVerticalOffset = 0;

					if (m_pVScroll)
					{
						m_pVScroll->SetFlags(m_pVScroll->GetFlags() & ~UIFLAG_VISIBLE);
					}
				}

				if (m_bHorizontalScrollBar)
				{
					UIRect pHScrollRect;

					pHScrollRect.fLeft = pRect.fLeft;
					pHScrollRect.fTop = pRect.fTop + pRect.fHeight - m_pUISystem->GetWidgetRect(m_pHScroll).fHeight;
					pHScrollRect.fWidth = pRect.fWidth - (m_bVerticalScrollBar ? m_pUISystem->GetWidgetRect(m_pVScroll).fWidth : 0);
					pHScrollRect.fHeight = m_pUISystem->GetWidgetRect(m_pHScroll).fHeight;

					m_pHScroll->SetRect(pHScrollRect, 1);
					m_pHScroll->SetStep((m_fTotalWidth - pTextRect.fWidth) / m_fTotalWidth);
				}
				else
				{
					m_fHorizontalOffset = 0;

					if (m_pHScroll)
					{
						m_pHScroll->SetFlags(m_pHScroll->GetFlags() & ~UIFLAG_VISIBLE);
					}
				}
			}
		}
		break;
	case UIM_LBUTTONDBLCLICK:
		{
			OnCommand();
		}
		break;
	case UIM_LBUTTONDOWN:
		{
			m_fModelRotationAcc = 0.0f;
		}
		break;
	case UIM_MOUSEUP:
		{
			m_fModelRotationAcc = 1.0f;
		}
	case UIM_MOUSEMOVE:
		{
			if (m_fModelRotationAcc == 0.0f)
			{
				m_fAngle += (UIM_GET_X_FLOAT(lParam) - UIM_GET_X_FLOAT(wParam)) * m_fMouseMultiplier;
			}
		}
		break;
	default:
		break;
	}

	return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::Draw(int iPass)
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
		// if we are a textured static, draw the correct texture
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

	// draw the model if there is one
	if (m_pModel)
	{
		IRenderer *pRenderer = m_pUISystem->GetIRenderer();

		// because we clear the z-buffer here we cannot have more than one
		// hack was done because z-buffer read for flares was not working with transparent menu
		pRenderer->ClearDepthBuffer();

		pRenderer->SetState(GS_NODEPTHTEST);

		int iX = (int)(pRenderer->ScaleCoordX(pAbsoluteRect.fLeft) + 0.5f);
		int iY = (int)(pRenderer->ScaleCoordY(pAbsoluteRect.fTop) + 0.5f);
		int iW = (int)(pRenderer->ScaleCoordX(pAbsoluteRect.fWidth) + 0.5f);
		int iH = (int)(pRenderer->ScaleCoordY(pAbsoluteRect.fHeight) + 0.5f);

		// save old viewport
		int iViewportX, iViewportY, iViewportW, iViewportH;
		pRenderer->GetViewport(&iViewportX, &iViewportY, &iViewportW, &iViewportH);

		// set new viewport
		pRenderer->SetViewport(iX, iY, iW, iH);

		// update angle
		m_fAngle += m_fModelRotationAcc * m_pUISystem->GetISystem()->GetITimer()->GetFrameTime() * m_fModelRotation * 360.0f;

		// setup the camera
		CCamera pCamera;
		pCamera.Init(iW, iH, m_fCameraFov);
		pCamera.SetPos(Vec3d(0.0f, m_fCameraDistance, 0.0f));
		pCamera.SetAngle(Vec3d(0.0f, 0.0f, 0.0f));
		pCamera.Update();
		pRenderer->SetCamera(pCamera);

		// set model rendering params
		SRendParams pRenderParams;
		pRenderParams.vPos = Vec3d(0.0f, 0.0f, -0.9f);
		pRenderParams.vAngles = Vec3d(0.0f, 0.0f, m_fAngle);
		pRenderParams.nDLightMask = 0;
		pRenderParams.vAmbientColor = Vec3d(0.25f, 0.25f, 0.25f);

		// setup a dynamic light
		CDLight pLight;
		memset(&pLight, 0, sizeof(CDLight));

		pLight.m_Color = CFColor(1.0f, 1.0f, 1.0f) * 0.8f;
		pLight.m_SpecColor = CFColor(0.0f, 0.0f, 0.0f);
		pLight.m_Flags = DLF_POINT;
		pLight.m_fRadius = m_fLightDistance * 10.0f;
		pLight.m_fStartRadius = 0.0f;
		pLight.m_fEndRadius = m_fLightDistance * 10.0f;
		pLight.m_fRadius = m_fLightDistance * 10.0f;
		pLight.m_Origin = Vec3d(10.0f, m_fLightDistance, 0.0f);

		// enable the light
		pRenderParams.nDLightMask |= 1 << pLight.m_Id;
//		pRenderParams.nDLightMaskFull |= 1 << pLight.m_Id;

		pRenderer->EF_StartEf();

		pRenderer->EF_ClearLightsList();
		pRenderer->EF_ADDDlight(&pLight);
		pRenderer->EF_UpdateDLight(&pLight);
		
		m_pModel->Update();
		m_pModel->Draw(pRenderParams,Vec3(zero));

		pRenderer->EF_EndEf3D(SHDF_SORT);

		// restore old settings
		pRenderer->SetViewport(iViewportX, iViewportY, iViewportW, iViewportH);	
	}

	// adjust the rect with the scrollbar sizes
	if (m_bHorizontalScrollBar)
	{
		pAbsoluteRect.fHeight -= m_pUISystem->GetWidgetRect(m_pHScroll).fHeight;
	}
	if (m_bVerticalScrollBar)
	{
		pAbsoluteRect.fWidth -= m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;
	}

	// update the offsets
	// vertical
	if (m_bVerticalScrollBar)
	{
		if ((m_iVAlignment == UIALIGN_BOTTOM) && (m_pVScroll->GetFlags() & UIFLAG_VISIBLE))
		{
			m_fVerticalOffset = -(pAbsoluteRect.fHeight - m_fTotalHeight) * (1.0f - m_pVScroll->GetValue());
		}
		else if ((m_iVAlignment == UIALIGN_MIDDLE) && (m_pVScroll->GetFlags() & UIFLAG_VISIBLE))
		{
			m_fVerticalOffset = -(pAbsoluteRect.fHeight - m_fTotalHeight) * 0.5f + (m_fTotalHeight - pAbsoluteRect.fHeight) * -m_pVScroll->GetValue();
		}
		else if (m_pVScroll->GetFlags() & UIFLAG_VISIBLE)
		{
			m_fVerticalOffset = (m_fTotalHeight - pAbsoluteRect.fHeight) * -m_pVScroll->GetValue();
		}
		else
		{
			m_fVerticalOffset = 0.0f;
		}
	}
	else
	{
		m_fVerticalOffset = 0.0f;
	}

	// horizontal
	if (m_bHorizontalScrollBar)
	{
		if ((m_iHAlignment == UIALIGN_RIGHT) && (m_pHScroll->GetFlags() & UIFLAG_VISIBLE))
		{
			m_fHorizontalOffset = -(pAbsoluteRect.fWidth - m_fTotalWidth) * (1.0f - m_pHScroll->GetValue());
		}
		else if ((m_iHAlignment == UIALIGN_MIDDLE) && (m_pHScroll->GetFlags() & UIFLAG_VISIBLE))
		{
			m_fHorizontalOffset = -(pAbsoluteRect.fWidth - m_fTotalWidth) * 0.5f + (m_fTotalWidth - pAbsoluteRect.fWidth) * -m_pHScroll->GetValue();
		}
		else if (m_pHScroll->GetFlags() & UIFLAG_VISIBLE)
		{
			m_fHorizontalOffset = (m_fTotalWidth - pAbsoluteRect.fWidth) * -m_pHScroll->GetValue();
		}
		else
		{
			m_fHorizontalOffset = 0.0f;
		}
	}
	else
	{
		m_fHorizontalOffset = 0.0f;
	}
	
	// get the font
	IFFont *pFont = pFont = m_pUISystem->GetIFont(m_pFont);

	if (!m_vLines.empty())
	{
		IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

		m_pUISystem->SetScissor(&pAbsoluteRect);

		if (m_iStyle & UISTYLE_MULTILINE)
		{
			UIRect pLineRect(pAbsoluteRect.fLeft + m_fLeftSpacing, 0, pAbsoluteRect.fWidth - m_fLeftSpacing - m_fRightSpacing, 0);

			// get the correct start
			switch (m_iVAlignment)
			{
			case UIALIGN_BOTTOM:
				pLineRect.fTop = pAbsoluteRect.fTop + pAbsoluteRect.fHeight - m_fTotalHeight;
				break;
			case UIALIGN_TOP:
				pLineRect.fTop = pAbsoluteRect.fTop;
				break;
			case UIALIGN_MIDDLE:
				pLineRect.fTop = pAbsoluteRect.fTop + (pAbsoluteRect.fHeight - m_fTotalHeight) * 0.5f;
				break;
			}

			m_pUISystem->SetScissor(&pAbsoluteRect);

			pLineRect.fTop += m_fVerticalOffset;
			pLineRect.fLeft += m_fHorizontalOffset;

			for (std::vector<UIStaticLine>::iterator pItor = m_vLines.begin(); pItor != m_vLines.end(); ++pItor)
			{
				UIStaticLine &pLine = *pItor;

				pLineRect.fHeight = m_fLineHeight;

				// check if the line is below the rect
				// if so, we stop drawing, because there is no way the following will be visible
				if (pLineRect.fTop > pAbsoluteRect.fTop + pAbsoluteRect.fHeight)
				{
					break;
				}

				// check if the line is above the rect
				else if (pLineRect.fTop + GetLineHeight(&pLine) < pAbsoluteRect.fTop)
				{
					pLineRect.fTop += GetLineHeight(&pLine);

					continue;
				}

				if (m_iStyle & UISTYLE_WORDWRAP)
				{
					wchar_t *szBaseString = (wchar_t *)pLine.szText.c_str();
					wchar_t *szString = szBaseString;
					wchar_t cSave;
					int		iIndex;

					for (int i = 0; i < pLine.iWrapCount; i++)
					{
						iIndex = pLine.iWrapIndex[i];
						cSave = szBaseString[iIndex];
						szBaseString[iIndex] = 0;

						// draw the text
						m_pUISystem->DrawText(pLineRect, m_iHAlignment, UIALIGN_MIDDLE, pFont, szString);

						szBaseString[iIndex] = cSave;
						szString = szBaseString + iIndex;

						pLineRect.fTop += m_fLineHeight;
					}

					m_pUISystem->DrawText(pLineRect, m_iHAlignment, UIALIGN_MIDDLE, pFont, szString);

					pLineRect.fTop += m_fLineHeight;
				}
				else
				{
					m_pUISystem->DrawText(pLineRect, m_iHAlignment, UIALIGN_MIDDLE, pFont, pLine.szText.c_str());

					pLineRect.fTop += GetLineHeight(&pLine);
				}
			}
		}
		else
		{
			pAbsoluteRect.fLeft += m_fLeftSpacing;
			pAbsoluteRect.fWidth -= m_fLeftSpacing + m_fRightSpacing;

			m_pUISystem->DrawText(pAbsoluteRect, m_iHAlignment, m_iVAlignment, pFont, m_vLines[0].szText.c_str());
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
int CUIStatic::SetText(const wstring &szText)
{
	if (m_vLines.empty())
	{
		UIStaticLine pLine;

		pLine.szText = szText;

		m_vLines.push_back(pLine);
	}
	else
	{
		m_vLines[0].szText = szText;
	}

	IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

	GetLineMetrics(&m_vLines[0], pFont);

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetVAlign(int iAlign)
{
	m_iVAlignment = iAlign;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetVAlign()
{
	return m_iVAlignment;
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetHAlign(int iAlign)
{
	m_iHAlignment = iAlign;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetHAlign()
{
	return m_iHAlignment;
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::LoadModel(const string &szModelName)
{
	if (m_pModel)
	{
		ReleaseModel();
	}
	m_pModel = m_pUISystem->GetISystem()->GetIAnimationSystem()->MakeCharacter(szModelName.c_str());
	m_szModelName = szModelName;

	return (m_pModel ? 1 : 0);
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::ReleaseModel()
{
	if (m_pModel)
	{
		m_pUISystem->GetISystem()->GetIAnimationSystem()->RemoveCharacter(m_pModel);
		m_pModel = 0;
	}
	m_szModelName.clear();

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::StartAnimation(const string &szAnimationName)
{
	if (m_pModel)
	{
		// [PETAR] StartAnimation2 is obsolete - old call provided here as reference
        //if (m_pModel->StartAnimation2(szAnimationName.c_str(), 0.0f, 0.0f, 1, 1, 1))

		CryCharAnimationParams ccap;
		ccap.fBlendInTime = ccap.fBlendOutTime = 0.f;
		ccap.nLayerID = 1;
		ccap.nFlags = ccap.FLAGS_SYNCHRONIZE_WITH_LAYER_0;

		if (m_pModel->StartAnimation(szAnimationName.c_str(),ccap))
		{
			m_pModel->Update();

			return 1;
		}
		else
		{
			m_pModel->ResetAnimations();

			return 0;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------------------------- 
void CUIStatic::InitializeTemplate(IScriptSystem *pScriptSystem)
{
	_ScriptableEx<CUIStatic>::InitializeTemplate(pScriptSystem);

	REGISTER_COMMON_MEMBERS(pScriptSystem, CUIStatic);

	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, SetText);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, GetText);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, Clear);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, SetLine);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, AddLine);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, GetLine);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, GetLineCount);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, SetVAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, GetVAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, SetHAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, GetHAlign);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, SetTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, GetTexture);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, LoadModel);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, ReleaseModel);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, SetView);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, SetAnimation);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, SetShaderFloat);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, SetShader);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIStatic, SetSecondShader);

}

//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetLineMetrics(UIStaticLine *pLine, IFFont *pFont)
{
	// get string metrics
	vector2f vStringSize = pFont->GetTextSizeW(pLine->szText.c_str());

	vStringSize.x /= m_pUISystem->GetIRenderer()->ScaleCoordX(1);
	vStringSize.y /= m_pUISystem->GetIRenderer()->ScaleCoordY(1);

	m_fLineHeight = m_fLineSpacing + vStringSize.y;

	float fAllowedWidth = GetTextRect(1).fWidth - UI_DEFAULT_LINE_WIDTH_ADDITION;
	float fStringWidth = vStringSize.x;

	// if the string is short or wordwrap not enabled, don't do anything
	if ((fStringWidth < fAllowedWidth) || ((m_iStyle & UISTYLE_WORDWRAP) == 0))
	{
		pLine->fWidth = fStringWidth + UI_DEFAULT_LINE_WIDTH_ADDITION;
		pLine->fHeight = m_fLineHeight;
		pLine->fWrapWidth = fStringWidth;
		pLine->iWrapCount = 0;

		return 1;
	}

	// otherwise...
	// clear the wrap points
	memset(pLine->iWrapIndex, 0, sizeof(int) * UI_DEFAULT_MAX_WRAP_INDICES);
	pLine->iWrapCount = 0;
	pLine->fWrapWidth = 0.0f;

	// start with one line height
	pLine->fHeight = m_fLineHeight;

	int		iLastSpace = -1;
	float	fLastSpaceWidth = 0.0f;

	float fCurrentCharWidth = 0.0f;
	float fCurrentLineWidth = 0.0f;
	float fBiggestLineWidth = 0.0f;
	float fWidthSum = 0.0f;

	int			iCurrentChar = 0;
	wchar_t		*pChar = (wchar_t *)pLine->szText.c_str();
	wchar_t		szChar[2] = {0, 0};

	while(szChar[0] = *pChar++)
	{
		// ignore color codes
		if (szChar[0] == L'$')
		{
			if (*pChar)
			{
				++pChar;
				++iCurrentChar;

				if ((*pChar) != L'$')
				{
					++iCurrentChar;

					continue;
				}
				szChar[0] = *pChar;
			}
		}

		// get char width and sum it to the line width
		fCurrentCharWidth = pFont->GetTextSizeW(szChar).x / m_pUISystem->GetIRenderer()->ScaleCoordX(1);;

		// keep track of spaces
		// they are good for spliting the string :D
		if (szChar[0] == L' ')
		{
			iLastSpace = iCurrentChar;
			fLastSpaceWidth = fCurrentLineWidth + fCurrentCharWidth;
		}

		// if line exceed allowed width, split it
		if ((fCurrentLineWidth + fCurrentCharWidth >= fAllowedWidth) && (*pChar))
		{
			if ((iLastSpace > 0) && ((iCurrentChar - iLastSpace) < UI_DEFAULT_WORDWRAP_TRESHOLD) && (iCurrentChar - iLastSpace > 0))
			{
				pLine->iWrapIndex[pLine->iWrapCount++] = iLastSpace + 1;

				if (fLastSpaceWidth > fBiggestLineWidth)
				{
					fBiggestLineWidth = fLastSpaceWidth;
				}

				fCurrentLineWidth = fCurrentLineWidth - fLastSpaceWidth + fCurrentCharWidth;
				assert(fCurrentLineWidth >= 0);
				fWidthSum += fCurrentLineWidth;
			}
			else
			{
				pLine->iWrapIndex[pLine->iWrapCount++] = iCurrentChar;

				if (fCurrentLineWidth > fBiggestLineWidth)
				{
					fBiggestLineWidth = fCurrentLineWidth;
				}

				fWidthSum += fCurrentLineWidth;
				fCurrentLineWidth = fCurrentCharWidth;
			}

			pLine->fHeight += m_fLineHeight;

			// if we don't need any more line breaks, then just stop
			if (fStringWidth - fWidthSum <= fAllowedWidth)
			{
				break;
			}

			fLastSpaceWidth = 0;
			iLastSpace = 0;
		}
		else
		{
			fCurrentLineWidth += fCurrentCharWidth;
		}

		iCurrentChar++;
	}

	pLine->fWrapWidth = fBiggestLineWidth;

	return 1;
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetLineListMetrics()
{
	IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

	m_fLineHeight = m_fLineSpacing + pFont->GetTextSizeW(L"I_").y / m_pUISystem->GetIRenderer()->ScaleCoordY(1);;

	for (std::vector<UIStaticLine>::iterator pItor = m_vLines.begin(); pItor != m_vLines.end(); ++pItor)
	{
		GetLineMetrics(&(*pItor), pFont);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------- 
float CUIStatic::GetLineWidth(UIStaticLine *pLine)
{
	if (m_iStyle & UISTYLE_WORDWRAP)
	{
		return pLine->fWrapWidth;
	}

	return pLine->fWidth;
}

//------------------------------------------------------------------------------------------------- 
float CUIStatic::GetLineHeight(UIStaticLine *pLine)
{
	if (m_iStyle & UISTYLE_WORDWRAP)
	{
		return pLine->fHeight;
	}

	return m_fLineHeight;
}

//------------------------------------------------------------------------------------------------- 
float CUIStatic::GetTextWidth()
{
	float fTextWidth = 0.0f;

	if (m_iStyle & UISTYLE_WORDWRAP)
	{
		for (std::vector<UIStaticLine>::iterator pItor = m_vLines.begin(); pItor != m_vLines.end(); ++pItor)
		{
			if ((*pItor).fWrapWidth > fTextWidth)
			{
				fTextWidth = (*pItor).fWrapWidth;
			}
		}
	}
	else
	{
		for (std::vector<UIStaticLine>::iterator pItor = m_vLines.begin(); pItor != m_vLines.end(); ++pItor)
		{
			if ((*pItor).fWidth > fTextWidth)
			{
				fTextWidth = (*pItor).fWidth;
			}
		}
	}

	return fTextWidth;
}

//------------------------------------------------------------------------------------------------- 
float CUIStatic::GetTextHeight()
{
	float fTextHeight = 0.0f;

	for (std::vector<UIStaticLine>::iterator pItor = m_vLines.begin(); pItor != m_vLines.end(); ++pItor)
	{
		fTextHeight += GetLineHeight(&(*pItor));
	}

	return fTextHeight;
}

//------------------------------------------------------------------------------------------------- 
UIRect CUIStatic::GetTextRect(bool bScrollBars)
{
	UIRect pTextRect(m_pRect);

	// if border is large enough to be visible, remove it from the rect
	if (m_pBorder.fSize > 0.125f)
	{
		m_pUISystem->AdjustRect(&pTextRect, pTextRect, m_pBorder.fSize);
	}

	if ((bScrollBars) && (m_iStyle & UISTYLE_MULTILINE))
	{
		// we have the toolbars, remove them from the rect
		if (m_bHorizontalScrollBar)
		{
			pTextRect.fHeight -= m_pUISystem->GetWidgetRect(m_pHScroll).fHeight;
		}
		else if (m_bVerticalScrollBar)
		{
			pTextRect.fWidth -= m_pUISystem->GetWidgetRect(m_pVScroll).fWidth;
		}
	}

	pTextRect.fLeft += m_fLeftSpacing;
	pTextRect.fWidth -= m_fRightSpacing + m_fLeftSpacing;

	return pTextRect;
}

//------------------------------------------------------------------------------------------------- 
// Script Functions
//------------------------------------------------------------------------------------------------- 

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetFontName(IFunctionHandler *pH)
{
	int iResult = CUIWidget::SetFontName(pH);

	GetLineListMetrics();

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetFontSize(IFunctionHandler *pH)
{
	int iResult = CUIWidget::SetFontSize(pH);

	GetLineListMetrics();

	return iResult;
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetText(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetText, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE2(m_pScriptSystem, GetName().c_str(), SetText, 1, svtString, svtNumber);

	IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

	if (m_vLines.empty())
	{
		UIStaticLine pLine;

		m_pUISystem->ConvertToWString(pLine.szText, pH, 1);

		GetLineMetrics(&pLine, pFont);

		m_vLines.push_back(pLine);
	}
	else
	{
		m_vLines[0].szText.clear();

		m_pUISystem->ConvertToWString(m_vLines[0].szText, pH, 1);

		GetLineMetrics(&m_vLines[0], pFont);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetText(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetText, 0);

	if (m_vLines.empty())
	{
		return pH->EndFunctionNull();
	}

	char	szString[1024] = {0,0};
	int		iStringSize = min(m_vLines[0].szText.size(), 1023);
	wchar_t *pChar = (wchar_t *)m_vLines[0].szText.c_str();

	int i = 0;
	while (*pChar)
	{
		szString[i++] = (char)*pChar++;
	}
	szString[i] = 0;

	return pH->EndFunction(szString);
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::Clear(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Clear, 0);

	m_vLines.clear();

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetLine(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetLine, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetLine, 1, svtNumber);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetLine, 2, svtString);

	char			*szLine;
	int				iLine;

	pH->GetParam(1, iLine);
	pH->GetParam(2, szLine);

	if (iLine < (int)m_vLines.size())
	{
		m_vLines[iLine].szText.clear();

		m_pUISystem->ConvertToWString(m_vLines[iLine].szText, szLine);
	}
	else if (iLine == (int)m_vLines.size())
	{
		UIStaticLine	pLine;

		m_pUISystem->ConvertToWString(pLine.szText, szLine);

		m_vLines.push_back(pLine);
	}

	IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

	GetLineMetrics(&m_vLines[iLine], pFont);

	// don't grow the buffer past our maxlines
	while ((int)m_vLines.size() >= m_iMaxLines)
	{
		m_vLines.erase(m_vLines.begin());
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::AddLine(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), AddLine, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), AddLine, 1, svtString);

	char			*szLine;
	wstring	szWLine;

	pH->GetParam(1, szLine);

	UIStaticLine	pLine;

	m_pUISystem->ConvertToWString(pLine.szText, szLine);

	m_vLines.push_back(pLine);

	IFFont *pFont = m_pUISystem->GetIFont(m_pFont);

	GetLineMetrics(&(*(m_vLines.end()-1)), pFont);

	// don't grow the buffer past our maxlines
	while ((int)m_vLines.size() >= m_iMaxLines)
	{
		m_vLines.erase(m_vLines.begin());
	}
 
	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetLine(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetLine, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), GetLine, 1, svtNumber);

	int iLine = 0;

	pH->GetParam(1, iLine);

	char szString[1024] = {0,0};

	if (iLine < (int)m_vLines.size())
	{
		
		int		iStringSize = min(m_vLines[iLine].szText.size(), 1023);
		wchar_t *pChar = (wchar_t *)m_vLines[iLine].szText.c_str();

		int i = 0;
		while (*pChar)
		{
			szString[i++] = (char)*pChar++;
		}
		szString[i] = 0;
	}

	return pH->EndFunction(szString);
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetLineCount(IFunctionHandler *pH)
{
	if (m_iStyle & UISTYLE_MULTILINE)
	{
		return pH->EndFunction((int)m_vLines.size());
	}
	else
	{
		return pH->EndFunction((int)1);
	}
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetVAlign(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetVAlign, m_iVAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetVAlign(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetVAlign, m_iVAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetHAlign(IFunctionHandler *pH)
{
	RETURN_INT_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetHAlign, m_iHAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetHAlign(IFunctionHandler *pH)
{
	RETURN_INT_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetHAlign, m_iHAlignment);
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_FROM_SCRIPT(m_pScriptSystem, GetName().c_str(), SetTexture, m_pTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetTexture(IFunctionHandler *pH)
{
	RETURN_TEXTURE_TO_SCRIPT(m_pScriptSystem, GetName().c_str(), GetTexture, m_pTexture.iTextureID);
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetHScrollBar(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetHScrollBar, 0);

	if (m_pHScroll)
	{
		return pH->EndFunction(m_pHScroll->GetScriptObject());
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::GetVScrollBar(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), GetVScrollBar, 0);

	if (m_pVScroll)
	{
		return pH->EndFunction(m_pVScroll->GetScriptObject());
	}

	return pH->EndFunctionNull();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::LoadModel(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), LoadModel, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), LoadModel, 1, svtString);

	char *szModelName;

	pH->GetParam(1, szModelName);

	if (m_pModel)
	{
		m_pModel = 0;
	}

	if (LoadModel(szModelName))
	{
		return pH->EndFunction(true);
	}
	else
	{
		return pH->EndFunction(false);
	}
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::ReleaseModel(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), ReleaseModel, 0);

	ReleaseModel();

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetView(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetView, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetView, 1, svtNumber);

	if (pH->GetParamCount() > 3)
	{
		pH->GetParam(4, m_fModelRotation);
	}
	if (pH->GetParamCount() > 2)
	{
		pH->GetParam(3, m_fLightDistance);
	}
	if (pH->GetParamCount() > 1)
	{
		pH->GetParam(2, m_fCameraFov);
		m_fCameraFov = m_fCameraFov * gf_PI / 180.0f;
	}
	if (pH->GetParamCount() > 0)
	{
		pH->GetParam(1, m_fCameraDistance);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetAnimation(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetAnimation, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetAnimation, 1, svtString);

	if (m_pModel)
	{
		char *szAnimName;

		if (pH->GetParam(1, szAnimName))
		{
			StartAnimation(szAnimName);
		}
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetShaderFloat(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetShaderFloat, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetShaderFloat, 1, svtString);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetShaderFloat, 2, svtNumber);

	char *szShaderName = 0;
	float fValue = 0.0f;

	pH->GetParam(1, szShaderName);
	pH->GetParam(2, fValue);

	if (m_pModel)
	{
		m_pModel->SetShaderFloat(szShaderName, fValue);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetShader(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetShader, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetShader, 1, svtString);

	char *szShaderName = 0;

	pH->GetParam(1, szShaderName);

	if (m_pModel)
	{
		m_pModel->SetShaderTemplateName(szShaderName, 0, 0, 0);
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------------------------------- 
int CUIStatic::SetSecondShader(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetSecondShader, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetSecondShader, 1, svtString);

	char *szShaderName = 0;

	pH->GetParam(1, szShaderName);

	if (m_pModel)
	{
		m_pModel->SetShaderTemplateName(szShaderName, 1, 0, 0);
	}

	return pH->EndFunction();
}
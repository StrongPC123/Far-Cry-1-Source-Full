
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//  File: UIVideoPanel.cpp
//  Description: UI Video Panel Manager
//
//  History:
//  - [9/7/2003]: File created by Márcio Martins
//	- February 2005: Modified by Marco Corbetta for SDK release
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "UIVideoPanel.h"
#include "UISystem.h"

#if !defined(NOT_USE_DIVX_SDK)
#include "UIDivX_Video.h"
#endif

#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
#pragma comment(lib, "binkw32.lib")
#endif

_DECLARE_SCRIPTABLEEX(CUIVideoPanel)

//////////////////////////////////////////////////////////////////////
static bool g_bBinkInit = 0;

////////////////////////////////////////////////////////////////////// 
CUIVideoPanel::CUIVideoPanel()
:
#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	m_hBink(0),
#endif
	m_bLooping(1), m_bPlaying(0), m_bPaused(0), m_iTextureID(-1), m_pSwapBuffer(0), m_szVideoFile(""), m_bKeepAspect(1)
{
	m_DivX_Active=0;	
}

////////////////////////////////////////////////////////////////////// 
CUIVideoPanel::~CUIVideoPanel()
{
	ReleaseVideo();
}

////////////////////////////////////////////////////////////////////// 
string CUIVideoPanel::GetClassName()
{
	return UICLASSNAME_VIDEOPANEL;
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::InitBink()
{
#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	if (g_bBinkInit)
	{
		return 1;
	}

	if (!BinkSoundUseDirectSound(0))
	{
		m_pUISystem->GetISystem()->GetILog()->Log("$4Bink Error$1: %s", BinkGetError());
		m_pUISystem->GetISystem()->GetILog()->Log("  Trying WaveOut");

		if (!BinkSoundUseWaveOut())
		{
			char *szError = BinkGetError();
			m_pUISystem->GetISystem()->GetILog()->Log("$4Bink Error$1: %s", szError);
			m_pUISystem->GetISystem()->GetILog()->Log("  No sound will be played!");
		}
	}

#endif
	g_bBinkInit = 1;
	return 1;
}


////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::LoadVideo(const string &szFileName, bool bSound)
{
	
	m_DivX_Active=1; //activate DivX

#if !defined(WIN64) && !defined(NOT_USE_BINK_SDK)
	//check if a BINK-file exists 
	HBINK hBink = BinkOpen(szFileName.c_str(), BINKSNDTRACK);
	if (hBink) {
		m_DivX_Active=0; //deactivate DivX
		BinkClose(hBink);
	}
#endif
#if !defined(NOT_USE_DIVX_SDK)	
	if (m_DivX_Active){
		m_DivX_Active = g_DivXPlayer.Load_DivX( this, szFileName );
		return m_DivX_Active;
	}
#endif
#if !defined(WIN64) && !defined(NOT_USE_BINK_SDK)

	if (m_hBink)
	{
		BinkClose(m_hBink);

		m_hBink = 0;
	}

	if (m_pSwapBuffer)
	{
		delete[] m_pSwapBuffer;
		m_pSwapBuffer = 0;
	}

	if (m_iTextureID > -1)
	{
		m_pUISystem->GetIRenderer()->RemoveTexture(m_iTextureID);
		m_iTextureID = -1;
	}

	if (szFileName.empty())
	{
		return 0;
	}

	if (!InitBink())
	{
		return 0;
	}

	if (!bSound)
	{
		unsigned long dwTrack = 0;

		BinkSetSoundTrack(0, &dwTrack);
	}
	else
	{
		unsigned long dwTrack = 0;

		BinkSetSoundTrack(1, &dwTrack);
	}

	// load bink file
	if (stricmp(szFileName.c_str(), "binklogo") == 0)
	{
		m_hBink = BinkOpen((const char *)BinkLogoAddress(), BINKFROMMEMORY | BINKSNDTRACK);
	}
	else
	{
		// check for a MOD first
		const char *szPrefix=NULL;
		IGameMods *pMods=m_pUISystem->GetISystem()->GetIGame()->GetModsInterface();
		if (pMods)		
			szPrefix=pMods->GetModPath(szFileName.c_str());
				
		if(szPrefix)
		{
			m_hBink = BinkOpen(szPrefix, BINKSNDTRACK);
		}

		// try in the original folder
		if(!m_hBink)
		{
			m_hBink = BinkOpen(szFileName.c_str(), BINKSNDTRACK);
		}
	}

	if (!m_hBink)
	{
		char *szError = BinkGetError();

		OnError(szError);

		return 0;
	}

	// create swap buffer
	m_pSwapBuffer = new int[m_hBink->Width * m_hBink->Height];

	if (!m_pSwapBuffer)
	{
		assert(!"Failed to create video swap buffer for blitting video!");

		BinkClose(m_hBink);
		m_hBink = 0;

		OnError("");

		return 0;
	}

	// create texture for blitting

  // WORKAROUND: NVidia driver bug during playing of video file
  // Solution: Never remove video texture (non-power-of-two)
  if (m_hBink->Width==640 && m_hBink->Height==480)
	  m_iTextureID = 	m_pUISystem->GetIRenderer()->DownLoadToVideoMemory((unsigned char *)m_pSwapBuffer, m_hBink->Width, m_hBink->Height, eTF_0888, eTF_0888, 0, 0, FILTER_LINEAR, 0, "$VideoPanel", FT_DYNAMIC);
  else
    m_iTextureID = 	m_pUISystem->GetIRenderer()->DownLoadToVideoMemory((unsigned char *)m_pSwapBuffer, m_hBink->Width, m_hBink->Height, eTF_0888, eTF_0888, 0, 0, FILTER_LINEAR, 0, NULL, FT_DYNAMIC);

	if (m_iTextureID == -1)
	{
		assert(!"Failed to create video memory surface for blitting video!");

		delete[] m_pSwapBuffer;
		m_pSwapBuffer = 0;

		BinkClose(m_hBink);
		m_hBink = 0;
		
		OnError("");

		return 0;
	}
	return 1;
#else
	OnError("");

	return 0;
#endif

	return 0;
}

////////////////////////////////////////////////////////////////////// 
LRESULT CUIVideoPanel::Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam)	//AMD Port
{

	FUNCTION_PROFILER( m_pUISystem->GetISystem(), PROFILE_GAME );
#if !defined(NOT_USE_DIVX_SDK)
	if (m_DivX_Active){
		g_DivXPlayer.Update_DivX(this);
		return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);
	}
#endif
#if !defined(WIN64) && !defined(NOT_USE_BINK_SDK)

	if ((iMessage == UIM_DRAW) && (wParam == 0))
	{
		// stream the frame here
		if (m_bPlaying && m_hBink && m_pSwapBuffer)
		{
			if (!BinkWait(m_hBink))
			{
				{
					FRAME_PROFILER("CUIVideoPanel::Update:BinkDoFrame", m_pUISystem->GetISystem(), PROFILE_GAME);
					BinkDoFrame(m_hBink);		
				}

				if ((m_iTextureID > -1) && (m_pSwapBuffer))
				{
					{
						FRAME_PROFILER("CUIVideoPanel::Update:BinkCopyToBuffer", m_pUISystem->GetISystem(), PROFILE_GAME);
						BinkCopyToBuffer(m_hBink, m_pSwapBuffer, m_hBink->Width * 4, m_hBink->Height, 0, 0, BINKCOPYALL | BINKSURFACE32);
					}

					{
						FRAME_PROFILER("Renderer::UpdateTextureInVideoMemory", m_pUISystem->GetISystem(), PROFILE_GAME);
						m_pUISystem->GetIRenderer()->UpdateTextureInVideoMemory(m_iTextureID, (unsigned char *)m_pSwapBuffer, 0, 0, m_hBink->Width, m_hBink->Height, eTF_8888);
					}
				}

				if ((m_hBink->FrameNum < m_hBink->Frames) || m_bLooping)
				{
					FRAME_PROFILER("CUIVideoPanel::Update:BinkNextFrame", m_pUISystem->GetISystem(), PROFILE_GAME);
					BinkNextFrame(m_hBink);
				}
			}
		}

		int iResult = CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);

		if (m_hBink && ((m_hBink->FrameNum == m_hBink->Frames) && !m_bLooping))
		{
			Stop();

			OnFinished();
		}

		return iResult;
	}

	return CUISystem::DefaultUpdate(this, iMessage, wParam, lParam);

#endif

	return 0;
}


////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::Play()
{
	if (m_DivX_Active){
		m_bPlaying = 1;
		m_bPaused = 0;
		return 1;
	}	

#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	if (!m_hBink)
	{
		if (m_szVideoFile.empty())
		{
			return 0;
		}

		if (!LoadVideo(m_szVideoFile, 1))
		{
			return 0;
		}
	}

	BinkPause(m_hBink, 0);
 	m_bPlaying = 1;
	m_bPaused = 0;
	return 1;
#else
	return 0;
#endif

	return 0;
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::Stop()
{
#if !defined(NOT_USE_DIVX_SDK)
	if (m_DivX_Active){
		g_DivXPlayer.StopSound();
		m_bPaused = 0;
		m_bPlaying = 0;
		return 1;
	}
#endif
#if !defined(WIN64) && !defined(NOT_USE_BINK_SDK)
	if (!m_hBink)
	{
		return 0;
	}
	BinkPause(m_hBink, 1);
	BinkClose(m_hBink);
	m_hBink = 0;
	m_bPaused = 0;
	m_bPlaying = 0;
	return 1;
#endif

	m_bPaused = 0;
	m_bPlaying = 0;
	return 1;	
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::ReleaseVideo()
{
	if (m_DivX_Active){
		return 1;
	}

#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	if (m_hBink)
	{
		BinkClose(m_hBink);
	}

	if (m_iTextureID > -1)
	{
		m_pUISystem->GetIRenderer()->RemoveTexture(m_iTextureID);
		m_iTextureID = -1;
	}

	m_hBink = 0;
	m_szVideoFile = "";

	if (m_pSwapBuffer)
	{
		delete[] m_pSwapBuffer;
		m_pSwapBuffer = 0;
	}
	return 1;
#else
	return 0;
#endif

	return 1;
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::Pause(bool bPause)
{
	if (m_DivX_Active){
		return 1;
	}


#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	if (!m_hBink)
	{
		return 0;
	}

	if (bPause)
	{
		if (!m_bPaused)
		{
			m_bPaused = 1;
			BinkPause(m_hBink, 1);
		}
	}
	else
	{
		if (m_bPaused)
		{
			m_bPaused = 0;
			BinkPause(m_hBink, 0);
		}
	}
	return 1;
#else
	return 0;
#endif

	return 1;
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::IsPlaying()
{
	if (m_DivX_Active){
		return (m_bPlaying ? 1 : 0);
	}

#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	if (!m_hBink)
	{
		return 0;
	}
	return (m_bPlaying ? 1 : 0);
#else
	return 0;
#endif

	return (0);
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::IsPaused()
{
	if (m_DivX_Active){
		return (m_bPaused ? 1 : 0);
	}
#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	if (!m_hBink)
	{
		return 0;
	}
	return (m_bPaused ? 1 : 0);
#else
	return 0;
#endif

	return 0;
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::SetVolume(int iTrackID, float fVolume)
{

	if (m_DivX_Active){
		return 1;
	}

#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	if (!m_hBink)
	{
		return 0;
	}

	if (fVolume < 0.0f)
	{
		fVolume = 0.0f;
	}

	BinkSetVolume(m_hBink, iTrackID, (int)(fVolume * 32768));

	return 1;
#else
	return 0;
#endif

	return 1;
}

//////////////////////////////////////////////////////////////////////
int CUIVideoPanel::SetPan(int iTrackID, float fPan)
{
	if (m_DivX_Active){
		return 1;
	}

#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	if (!m_hBink)
	{
		return 0;
	}

	if (fPan > 1.0f)
	{
		fPan = 1.0f;
	}
	else if (fPan < -1.0f)
	{
		fPan = -1.0f;
	}

	BinkSetPan(m_hBink, 1, 32768 + (int)(fPan * 32767));
	return 1;
#else
	return 0;
#endif

	return 1;
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::SetFrameRate(int iFrameRate)
{
	if (m_DivX_Active){
		return 1;
	}

#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	if (!m_hBink)	{
		return 0;
	}

	BinkSetFrameRate(iFrameRate, 1);

	return 1;
#else
	return 0;
#endif

	return 1;
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::Draw(int iPass)
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

	// video
	if (m_iTextureID > -1)
	{
		float fWidth = pAbsoluteRect.fWidth;
		float fHeight = pAbsoluteRect.fHeight;

#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
		if (m_bKeepAspect && m_hBink)
		{
			float fAspect = m_hBink->Width / (float)m_hBink->Height;

			if (fAspect < 1.0f)
			{
				fWidth = fHeight * fAspect;
			}
			else
			{
				fHeight = fWidth / fAspect;
			}
		}
#endif

		if (fWidth > pAbsoluteRect.fWidth)
		{
			float fRatio = pAbsoluteRect.fWidth / fWidth;

			fWidth *= fRatio;
			fHeight *= fRatio;
		}
		if (fHeight > pAbsoluteRect.fHeight)
		{
			float fRatio = pAbsoluteRect.fHeight / fHeight;

			fWidth *= fRatio;
			fHeight *= fRatio;
		}

		UIRect pRect;

		pRect.fLeft = pAbsoluteRect.fLeft + (pAbsoluteRect.fWidth - fWidth) * 0.5f;
		pRect.fTop = pAbsoluteRect.fTop + (pAbsoluteRect.fHeight - fHeight) * 0.5f;
		pRect.fWidth = fWidth;
		pRect.fHeight = fHeight;

		if (m_bKeepAspect)
		{
			m_pUISystem->DrawQuad(pAbsoluteRect, m_cColor);
		}

		m_pUISystem->DrawImage(pRect, m_iTextureID, 0, color4f(1.0f, 1.0f, 1.0f, 1.0f));
	}

	// draw overlay
	if (m_pOverlay.iTextureID > -1)
	{
		m_pUISystem->DrawSkin(pAbsoluteRect, m_pOverlay, color4f(1.0f, 1.0f, 1.0f, 1.0f), UISTATE_UP);
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

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::EnableVideo(bool bEnable)
{
#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	if (!m_hBink)
	{
		return 0;
	}

	return BinkSetVideoOnOff(m_hBink, bEnable ? 1 : 0);
#else
	return 0;
#endif

	return 0;
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::EnableAudio(bool bEnable)
{
#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)

	if (!m_hBink)
	{
		return 0;
	}

	return BinkSetSoundOnOff(m_hBink, bEnable ? 1 : 0);
#else
	return 0;
#endif

	return 0;
}

////////////////////////////////////////////////////////////////////// 
void CUIVideoPanel::InitializeTemplate(IScriptSystem *pScriptSystem)
{
	_ScriptableEx<CUIVideoPanel>::InitializeTemplate(pScriptSystem);

	REGISTER_COMMON_MEMBERS(pScriptSystem, CUIVideoPanel);

	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, LoadVideo);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, ReleaseVideo);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, Play);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, Stop);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, Pause);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, IsPlaying);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, IsPaused);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, SetVolume);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, SetPan);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, SetFrameRate);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, EnableVideo);
	REGISTER_SCRIPTOBJECT_MEMBER(pScriptSystem, CUIVideoPanel, EnableAudio);
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::OnError(const char *szError)
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);

	if (!pScriptObject)
	{
		return 1;
	}

	HSCRIPTFUNCTION pScriptFunction = pScriptSystem->GetFunctionPtr(GetName().c_str(), "OnError");

	if (!pScriptFunction)
	{
		if (!pScriptObject->GetValue("OnError", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->PushFuncParam(szError);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::OnFinished()
{
	IScriptSystem *pScriptSystem = m_pUISystem->GetIScriptSystem();
	IScriptObject *pScriptObject = m_pUISystem->GetWidgetScriptObject(this);

	if (!pScriptObject)
	{
		return 1;
	}

	HSCRIPTFUNCTION pScriptFunction = pScriptSystem->GetFunctionPtr(GetName().c_str(), "OnFinished");

	if (!pScriptFunction)
	{
		if (!pScriptObject->GetValue("OnFinished", pScriptFunction))
		{
			return 1;
		}
	}

	int iResult = 1;

	pScriptSystem->BeginCall(pScriptFunction);
	pScriptSystem->PushFuncParam(pScriptObject);
	pScriptSystem->EndCall(iResult);

	pScriptSystem->ReleaseFunc(pScriptFunction);

	return iResult;
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::LoadVideo(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT2(m_pScriptSystem, GetName().c_str(), LoadVideo, 1, 2);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), LoadVideo, 1, svtString);

	if (pH->GetParamCount() == 2)
	{
		CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), LoadVideo, 2, svtNumber);
	}
	
	char *pszFileName;
	int iSound = 0;

	pH->GetParam(1, pszFileName);

	if (pH->GetParamCount() == 2)
	{
		pH->GetParam(2, iSound);
	}

	if (!LoadVideo(pszFileName, iSound != 0))
	{
		return pH->EndFunctionNull();
	}

	return pH->EndFunction(1);
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::ReleaseVideo(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), ReleaseVideo, 0);

	ReleaseVideo();

	return pH->EndFunction(1);
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::Play(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Play, 0);

	Play();

	return pH->EndFunction(1);
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::Stop(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Stop, 0);

	Stop();

	return pH->EndFunction(1);
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::Pause(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), Pause, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), Pause, 1, svtNumber);

	int iPause;

	pH->GetParam(1, iPause);

	Pause(iPause != 0);

	return pH->EndFunction(1);
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::IsPlaying(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), IsPlaying, 0);

	if (m_bPlaying)
	{
		return pH->EndFunction(1);
	}
	else
	{
		return pH->EndFunctionNull();
	}
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::IsPaused(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), IsPaused, 0);

	if (m_bPaused)
	{
		return pH->EndFunction(1);
	}
	else
	{
		return pH->EndFunctionNull();
	}
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::SetVolume(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetVolume, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetVolume, 1, svtNumber);

	float fVolume;

	pH->GetParam(1, fVolume);

	for (int i = 0; i < 16; i++)
	{
		SetVolume(i, fVolume);
	}

	return pH->EndFunction(1);
}

//////////////////////////////////////////////////////////////////////
int CUIVideoPanel::SetPan(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetPan, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetPan, 1, svtNumber);

	float fPan;

	pH->GetParam(1, fPan);

	for (int i = 0; i < 16; i++)
	{
		SetPan(i, fPan);
	}

	return pH->EndFunction(1);
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::SetFrameRate(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), SetFrameRate, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), SetFrameRate, 1, svtNumber);

	int iFrameRate;

	pH->GetParam(1, iFrameRate);

	SetFrameRate(iFrameRate);

	return pH->EndFunction(1);
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::EnableVideo(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), EnableVideo, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), EnableVideo, 1, svtNumber);

	int iEnable;

	pH->GetParam(1, iEnable);

	EnableVideo(iEnable != 0);

	return pH->EndFunction(1);
}

////////////////////////////////////////////////////////////////////// 
int CUIVideoPanel::EnableAudio(IFunctionHandler *pH)
{
	CHECK_SCRIPT_FUNCTION_PARAMCOUNT(m_pScriptSystem, GetName().c_str(), EnableAudio, 1);
	CHECK_SCRIPT_FUNCTION_PARAMTYPE(m_pScriptSystem, GetName().c_str(), EnableAudio, 1, svtNumber);

	int iEnable;

	pH->GetParam(1, iEnable);

	EnableAudio(iEnable != 0);
	
	return pH->EndFunction(1);
}
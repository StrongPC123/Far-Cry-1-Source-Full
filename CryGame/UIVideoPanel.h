//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - A Bink Video Control
//
// History:
//  - [8/8/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#ifndef UIVIDEOPANEL_H
#define UIVIDEOPANEL_H 

#define UICLASSNAME_VIDEOPANEL			"UIVideoPanel"


#include "UIWidget.h"
#include "UISystem.h"

#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
#	include "../binksdk/bink.h"
#endif


class CUISystem;


class CUIVideoPanel : public CUIWidget, public _ScriptableEx<CUIVideoPanel>
{
public:

	UI_WIDGET(CUIVideoPanel)

	CUIVideoPanel();
	~CUIVideoPanel();

	CUISystem* GetUISystem() { return m_pUISystem; }

	string GetClassName();

	LRESULT Update(unsigned int iMessage, WPARAM wParam, LPARAM lParam);	//AMD Port
	int Draw(int iPass);

	int InitBink();

	int LoadVideo(const string &szFileName, bool bSound);
//	int LoadVideo_DivX(const string &szFileName, bool bSound);

	int ReleaseVideo();
	int Play();
	int Stop();
	int Pause(bool bPause = 1);
	int IsPlaying();
	int IsPaused();

	int SetVolume(int iTrackID, float fVolume);
	int SetPan(int iTrackID, float fPan);

	int SetFrameRate(int iFrameRate);

	int EnableVideo(bool bEnable = 1);
	int EnableAudio(bool bEnable = 1);

	int OnError(const char *szError);
	int OnFinished();

	static void InitializeTemplate(IScriptSystem *pScriptSystem);

	//------------------------------------------------------------------------------------------------- 
	// Script Functions
	//------------------------------------------------------------------------------------------------- 
	int LoadVideo(IFunctionHandler *pH);
	int ReleaseVideo(IFunctionHandler *pH);

	int Play(IFunctionHandler *pH);
	int Stop(IFunctionHandler *pH);
	int Pause(IFunctionHandler *pH);
	
	int IsPlaying(IFunctionHandler *pH);
	int IsPaused(IFunctionHandler *pH);

	int SetVolume(IFunctionHandler *pH);
	int SetPan(IFunctionHandler *pH);

	int SetFrameRate(IFunctionHandler *pH);

	int EnableVideo(IFunctionHandler *pH);
	int EnableAudio(IFunctionHandler *pH);

	bool					m_DivX_Active;

	string				m_szVideoFile;
#if !defined(WIN64) && !defined(LINUX) && !defined(NOT_USE_BINK_SDK)
	HBINK					m_hBink;
#endif
	bool					m_bPaused;
	bool					m_bPlaying;
	bool					m_bLooping;
	bool					m_bKeepAspect;
	int						m_iTextureID;
	UISkinTexture m_pOverlay;
	int						*m_pSwapBuffer;
};

#endif
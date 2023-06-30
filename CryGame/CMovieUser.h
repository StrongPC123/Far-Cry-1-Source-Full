
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef MOVIE_USER_H
#define MOVIE_USER_H

//////////////////////////////////////////////////////////////////////////
// Interface for movie-system implemented by user for advanced function-support
class CMovieUser : public IMovieUser, public ISoundEventListener
{
private:
	CXGame *m_pGame;
public: 
	CMovieUser(CXGame *pGame)
	{
		m_InCutSceneCounter = 0;
		m_wPrevClientId = 0;
		m_pGame=pGame;
		m_fPrevMusicVolume=0;
	}

	// interface IMovieUser
	void SetActiveCamera(const SCameraParams &Params);
	void BeginCutScene(unsigned long dwFlags,bool bResetFX);
	void EndCutScene();
	void SendGlobalEvent(const char *pszEvent);
	void PlaySubtitles( ISound *pSound );

	// Implmenents ISoundEventListener.
	void OnSoundEvent( ESoundCallbackEvent event,ISound *pSound );

private:
	void ResetCutSceneParams();

	int m_InCutSceneCounter;
	int m_wPrevClientId;
	Vec3d m_vPrevClientPos;
	bool m_bSoundsPaused;
	float m_fPrevMusicVolume;
};

#endif
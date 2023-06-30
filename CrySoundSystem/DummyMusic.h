#pragma once
#include <isound.h>

class CDummyMusicSystem : public IMusicSystem
{
protected:
	ISystem *m_pSystem;
public:
	CDummyMusicSystem(ISystem *pSystem) { m_pSystem=pSystem; }
	virtual ~CDummyMusicSystem() {}
	void Release() { delete this; }
	ISystem* GetSystem() { return m_pSystem; }
	int GetBytesPerSample() { return 4; }
	IMusicSystemSink* SetSink(IMusicSystemSink *pSink) { return NULL; }
	bool SetData(struct SMusicData *pMusicData,bool bNoRelease=false) {return true;};
	void Unload() {}
	void Pause(bool bPause) {}
	void EnableEventProcessing(bool bEnable) {}
	bool ResetThemeOverride() { return true; }
	bool SetTheme(const char *pszTheme, bool bOverride=false) { return true; }
	const char* GetTheme() { return ""; }
	bool SetMood(const char *pszMood) { return true; }
	bool SetDefaultMood(const char *pszMood) { return true; }
	const char* GetMood() { return ""; }
	IStringItVec* GetThemes() { return NULL; }
	IStringItVec* GetMoods(const char *pszTheme) { return NULL; }
	bool AddMusicMoodEvent(const char *pszMood, float fTimeout) { return true; }
	void Update() {}
	SMusicSystemStatus* GetStatus() { return NULL; }
	void GetMemoryUsage(class ICrySizer* pSizer) {}
	bool LoadMusicDataFromLUA(IScriptSystem* pScriptSystem, const char *pszFilename){return true;}
	bool StreamOGG() {return true;}
	void LogMsg( const char *pszFormat, ... ) {}

	//////////////////////////////////////////////////////////////////////////
	// Editing support.
	//////////////////////////////////////////////////////////////////////////
	virtual void RenamePattern( const char *sOldName,const char *sNewName ) {};
	virtual void UpdateTheme( SMusicTheme *pTheme ) {};
	virtual void UpdateMood( SMusicMood *pMood ) {};
	virtual void UpdatePattern( SPatternDef *pPattern ) {};
	virtual void PlayPattern( const char *sPattern,bool bStopPrevious ) {};
	virtual void DeletePattern( const char *sPattern ) {};
	virtual void Silence() {};
	virtual bool LoadFromXML( const char *sFilename,bool bAddData ) { return true; };
};

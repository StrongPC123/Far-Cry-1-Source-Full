//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: LipSync.h
//  Description: Class to manage and compute all LipSync- and FacialExpression-Features.
//
//  History:
//  - December 6, 2001: Created by Lennert Schneider
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIPSYNC_H__93A3FF8F_9950_4F4E_A719_48B376E471D5__INCLUDED_)
#define AFX_LIPSYNC_H__93A3FF8F_9950_4F4E_A719_48B376E471D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ILipSync.h>
#include <ISound.h>
#include <IStreamEngine.h>

struct ISoundSystem;
struct IScriptSystem;
struct ISound;
struct ICryCharInstance;

/*
#define LIPSYNC_MAXSMOOTH				10000
#define LIPSYNC_PREAMP					1.2f
#define LIPSYNC_RNDEXPRFREQ			5000
#define LIPSYNC_RNDEXPRATTACK		4410
#define LIPSYNC_RNDEXPRSUSTAIN	4410
#define LIPSYNC_RNDEXPRRELEASE	8820
*/

#define MAX_LIPSYNC_TRACKS	3

struct SSyncData
{
	int nOfs;
	int nLen;
	signed char nPat;
	float fAmp;
};

struct SSyncPattern
{
	string sName;
	int nMorphTargetId;
};

struct SExprPattern
{
	SExprPattern()
	{
		fCurrIntervalTime=0.0f;
		fNextIntervalTime=0.0f;
	}
	string sName;
	int nMorphTargetId;
	float fOffset;
	float fInterval;
	float fIntervalRandom;
	float fAmp;
	float fAmpRandom;
	float fBlendIn;
	float fHold;
	float fBlendOut;
	// internal
	float fCurrIntervalTime;
	float fNextIntervalTime;
};

typedef std::map<string,int> TNameToIdMap;
typedef TNameToIdMap::iterator		TNameToIdMapIt;

typedef std::vector<SSyncData>		TSyncDataVec;
typedef std::vector<SSyncPattern>	TSyncPatternVec;
typedef std::vector<SExprPattern>	TExprPatternVec;

class CLipSync : public ILipSync, ISoundEventListener, IStreamCallback
{
private:
	// needed interfaces
	ISystem *m_pSystem;
	ISoundSystem *m_pSoundSystem;
	IScriptSystem *m_pScriptSystem;
	IStreamEngine *m_pStreamEngine;
	ILog *m_pLog;
	ITimer *m_pTimer;
	ICryPak *m_pPak;
	_smart_ptr<ISound> m_pSound;
	IEntity *m_pEntity;
	ICryCharInstance *m_pCharInst;
	IDialogLoadSink *m_pSink;
	IReadStreamPtr m_pReadStream;
	string m_loadedExpression;
	bool m_bSoundFileLoaded;
	bool m_bSyncFileLoaded;
	bool m_bUnload;
	// data
	bool m_bUnloadWhenDone;
	int m_nLipSyncTracks;
	TSyncDataVec m_vecData[MAX_LIPSYNC_TRACKS];
	TSyncPatternVec m_vecPatterns;
	TExprPatternVec m_vecExprPatterns;
	// state
	int m_nLastDataIdx[MAX_LIPSYNC_TRACKS];
	IScriptObject	*m_pAITable;
private:
	virtual ~CLipSync();
	int GetDataIdx(int nChannel, int nSmp, int nLo, int nHi);
	bool UpdateRandomExpressions(float fFrameTime, bool bAnimate);
	bool UpdateLipSync(float fFrameTime, bool bAnimate);
	void RemoveExtension(char *pszFilename);
	void AddExtension(char *pszFilename, const char *pszExtension);
	void CheckIfDialogLoaded();
	void LoadFailed();
	void OnSoundEvent( ESoundCallbackEvent event,ISound *pSound );
	void SyncFileLoaded();
	void SyncFileLoadFailed();
	void AbortLoading();
	virtual void StreamOnComplete(IReadStream *pStream, unsigned nError);
public:
	CLipSync();
	bool Init(ISystem *pSystem, IEntity *pEntity);																		// initializes and prepares the character for lip-synching
	void Release();																																		// releases all resources and deletes itself
	bool LoadRandomExpressions(const char *pszExprScript, bool bRaiseError=true);			// load expressions from script
	bool UnloadRandomExpressions();																										// release expressions
	// loads a dialog for later playback
	bool LoadDialog(const char *pszFilename, int nSoundVolume, float fMinSoundRadius, float fMaxSoundRadius, float fClipDist, int nSoundFlags=0,IScriptObject *pAITable=NULL);																						
	bool UnloadDialog();																															// releases all resources
	bool PlayDialog(bool bUnloadWhenDone=true);																				// plays a loaded dialog
	bool StopDialog();																																// stops (aborts) a dialog
	bool DoExpression(const char *pszMorphTarget, CryCharMorphParams &MorphParams, bool bAnim=true);		// do a specific expression
	bool StopExpression(const char *pszMorphTarget);																	// stop animating the specified expression
	bool Update(bool bAnimate=true);																									// updates animation & stuff
	void SetCallbackSink(IDialogLoadSink *pSink) { m_pSink=pSink; }
};

#endif // !defined(AFX_LIPSYNC_H__93A3FF8F_9950_4F4E_A719_48B376E471D5__INCLUDED_)

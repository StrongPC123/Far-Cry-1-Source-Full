//////////////////////////////////////////////////////////////////////
// 
//  Game Source Code
//
//  File: LipSync.cpp
//  Description: Class to manage and compute all LipSync- and FacialExpression-Features.
//
//  History:
//  - December 6, 2001: Created by Lennert Schneider
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <ISound.h>
#include <ITimer.h>
#include <CryCharMorphParams.h>
#include <ICryPak.h>
#include "LipSync.h"
#include "RandomExprLoadSink.h"
#include <StringUtils.h> // for strnstr
#include <IScriptSystem.h>

#include <IXGame.h>

#define MAGPIE_SUPPORT

#define LIPSYNC_PLAYAHEAD	0
#define LIPSYNC_AMP				0.7f
#define LIPSYNC_FADEIN		0.2f
#define LIPSYNC_FADEOUT		0.2f
#define LIPSYNC_CROSSFADE	0.2f

//////////////////////////////////////////////////////////////////////////
//! Reports a Game Warning to validator with WARNING severity.
inline void GameWarning( const char *format,... )
{
	if (!format)
		return;

	char buffer[MAX_WARNING_LENGTH];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);
	CryWarning( VALIDATOR_MODULE_GAME,VALIDATOR_WARNING,buffer );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLipSync::CLipSync()
{
	m_pSystem=NULL;
	m_pSoundSystem=NULL;
	m_pScriptSystem=NULL;
	m_pStreamEngine=NULL;
	m_pLog=NULL;
	m_pPak=NULL;
	m_pTimer=NULL;
	m_pSound=NULL;
	m_pEntity=NULL;
	m_pCharInst=NULL;
	m_pSink=NULL;
	m_bUnload=false;
	m_pAITable=NULL;
}

CLipSync::~CLipSync()
{

	if (m_pSound)
		m_pSound->RemoveEventListener(this);
	//if(m_pLog)			// if Init wasn't successful m_pLog might be 0
	//	m_pLog->LogToFile("\006~CLipSync(%p)(stream %p)", this, (IReadStream*)m_pReadStream);
}

bool CLipSync::Init(ISystem *pSystem, IEntity *pEntity)
{
	UnloadDialog();
	m_pSystem=pSystem;
	if (!m_pSystem)
		return false;
	m_pSoundSystem=pSystem->GetISoundSystem();
	if (!m_pSoundSystem)
		return false;
	m_pScriptSystem=pSystem->GetIScriptSystem();
	if (!m_pScriptSystem)
		return false;
	m_pStreamEngine=pSystem->GetStreamEngine();
	if (!m_pStreamEngine)
		return false;
	m_pLog=pSystem->GetILog();
	if (!m_pLog)
		return false;
	m_pTimer=pSystem->GetITimer();
	if (!m_pTimer)
		return false;
	m_pPak=pSystem->GetIPak();
	if (!m_pPak)
		return false;
	m_pSound=NULL;
	m_pEntity=pEntity;
	if (!m_pEntity)
		return false;
	IEntityCharacter *pEntChar=m_pEntity->GetCharInterface();
	if (!pEntChar)
		return false;
	m_pCharInst=pEntChar->GetCharacter(0);
	if (!m_pCharInst)
		return false;
	return true;
}

void CLipSync::Release()
{
	UnloadRandomExpressions();
	UnloadDialog();
	delete this;
}

bool CLipSync::LoadRandomExpressions(const char *pszExprScript, bool bRaiseError)
{
	if ((!m_pScriptSystem) || (!m_pSoundSystem) || (!m_pCharInst))
		return false;	// not initialized
	if (!pszExprScript)
		return false;
	// Check if this expression is already loaded.
	if (m_loadedExpression == pszExprScript)
		return true;

	// unloacd all expressions.
	UnloadRandomExpressions();
	if (!strlen(pszExprScript))
		return true;

	ICryCharModel *pModel=m_pCharInst->GetModel();
	assert(pModel);
	IAnimationSet *pAnimSet=pModel->GetAnimationSet();
	assert(pAnimSet);

	// parse random-expressions
	if (m_pScriptSystem->ExecuteFile(pszExprScript, true, false))
	{
		m_loadedExpression = pszExprScript;
		// Get only filename from path. use it as global key to expressions table.
		char filename[_MAX_FNAME];
		_splitpath( pszExprScript,NULL,NULL,filename,NULL );
		
		_SmartScriptObject pObj(m_pScriptSystem, true);
		if (m_pScriptSystem->GetGlobalValue( filename,pObj ))
		{
			CRandomExprLoadSink LoadSink(bRaiseError, m_pScriptSystem, &pObj, pAnimSet, &m_vecExprPatterns);
			pObj->Dump(&LoadSink);
		}
		else
		{
			GameWarning( "LipSync expression script %s does not contain table %s",pszExprScript,filename );
		}
	}
	else
	{
		GameWarning( "LipSync expression script %s filed to load",pszExprScript );
	}
	return true;
}

bool CLipSync::UnloadRandomExpressions()
{
	m_vecExprPatterns.clear();
	return true;
}

void CLipSync::RemoveExtension(char *pszFilename)
{
	char *pDot=strstr(pszFilename, ".");
	if (!pDot)
		return;	// no extension...
	pszFilename[pDot-pszFilename]=0;
}

void CLipSync::AddExtension(char *pszFilename, const char *pszExtension)
{
	strcat(pszFilename, pszExtension);
}

void CLipSync::CheckIfDialogLoaded()
{
	if (m_bSoundFileLoaded && m_bSyncFileLoaded)
	{
		if (m_pSink)
			m_pSink->OnDialogLoaded(this);
		TRACE("DIALOG LOADING SUCEEDED");
	}
}

void CLipSync::LoadFailed()
{
	if (m_pSink)
		m_pSink->OnDialogFailed(this);
	TRACE("DIALOG LOADING FAILED");
}

//////////////////////////////////////////////////////////////////////////
void CLipSync::OnSoundEvent( ESoundCallbackEvent event,ISound *pSound )
{
	switch(event)
	{
	case SOUND_EVENT_ON_LOADED:
		m_pSound->RemoveEventListener(this);
		m_bSoundFileLoaded=true;
		CheckIfDialogLoaded();
		break;
	case SOUND_EVENT_ON_LOAD_FAILED:
		m_pSound->RemoveEventListener(this);
		m_bUnload=true;
		break;
	}
}

void CLipSync::SyncFileLoaded()
{
	//m_pLog->LogToFile("\006CLipSync(%p)::SyncFileLoaded", this);
	m_pReadStream=NULL;
	m_bSyncFileLoaded=true;
	CheckIfDialogLoaded();
}

void CLipSync::SyncFileLoadFailed()
{
	//m_pLog->LogToFile("\006CLipSync(%p)::SyncFileLoadFailed", this);
	m_pReadStream=NULL;
	// for now we also want the dialog to succeed without lip-syncing so regardless if the sync-load fails or not... we proceed
	m_bSyncFileLoaded=true;
	CheckIfDialogLoaded();
	//LoadFailed();
	//UnloadDialog();
}

void CLipSync::AbortLoading()
{
#if !defined(LINUX64)
	if (m_pReadStream!=NULL)
#else
	if (m_pReadStream!=0)
#endif
	{
		//m_pLog->LogToFile("\006CLipSync(%p)::AbortLoading", this);
		m_pReadStream->Abort();
		m_pReadStream=NULL;
		LoadFailed();
	}
}

#define PUSH_DATA(_nIdx) \
{ \
	if (Data[_nIdx].fAmp) \
	{ \
		Data[_nIdx].nLen=(int)fLen[_nIdx]; \
		m_vecData[_nIdx].push_back(Data[_nIdx]); \
		Data[_nIdx].fAmp=0.0f; \
	} \
	fLen[_nIdx]=0.0f; \
}

using namespace CryStringUtils; // for strnstr

//////////////////////////////////////////////////////////////////////////
void CLipSync::StreamOnComplete(IReadStream *pStream, unsigned nError)
{
	//m_pLog->LogToFile("\006 CLypSync stream on complete");
	FUNCTION_PROFILER(m_pSystem, PROFILE_GAME);
	if (nError)
	{
		SyncFileLoadFailed();
		return; 
	}
	ICryCharModel *pModel=m_pCharInst->GetModel();
	assert(pModel);
	IAnimationSet *pAnimSet=pModel->GetAnimationSet();
	assert(pAnimSet);
	const char *pBuffer=(char*)pStream->GetBuffer();	
	int nBufferSize=pStream->GetBytesRead();
	const char *pEndBuffer=pBuffer+nBufferSize;
	if (pBuffer)
	{
#ifdef MAGPIE_SUPPORT
		const float fFramesPerSecond=30.0f;
		const float fFramesPerSecondRecp=1.0f/fFramesPerSecond;
		//m_pPak->FGets(sBuffer, nBufferSize, pFile);	// Frame#,Phonemes
		TNameToIdMap mapNameId;
		m_vecPatterns.clear();
		for (int i=0;i<MAX_LIPSYNC_TRACKS;i++)
			m_vecData[i].clear();
		float fLen[MAX_LIPSYNC_TRACKS];
		memset(fLen, 0, sizeof(fLen));
		SSyncData Data[MAX_LIPSYNC_TRACKS];
		memset(Data, 0, sizeof(Data));
		int nBytesRead=0;
		bool bReadHeader=true;
		m_nLipSyncTracks=0; 

		while ((nBytesRead<nBufferSize) && pBuffer && (pBuffer[0]!=0))	//!m_pPak->FEof(pFile))
		{
			for (int i=0;i<MAX_LIPSYNC_TRACKS;i++)
				fLen[i]+=fFramesPerSecondRecp*1000.0f;
			int nFrame;
			//m_pPak->FGets(sBuffer, nBufferSize, pFile);	//"%d,%s", &nFrame, sBuffer);
			const char *pNewBuffer=strnstr(pBuffer, "\n",pEndBuffer-pBuffer);
			if (pNewBuffer)
			{
				nBytesRead+=(pNewBuffer-pBuffer)+1;
				if (nBytesRead>nBufferSize)
				{
					m_pLog->Log("\001 error during CLypSync:stream on complete (error1)");					
					break;
				}

				if (bReadHeader)
				{
					const char *pCheckBuffer=pBuffer;
					const char *pLastCheckBuffer=pCheckBuffer;
					while (((pCheckBuffer=strnstr(pLastCheckBuffer, ",",pEndBuffer-pLastCheckBuffer))!=NULL) && (pCheckBuffer<strnstr(pLastCheckBuffer, "\n",pEndBuffer-pLastCheckBuffer)))
					{
						m_nLipSyncTracks++;
						pLastCheckBuffer=pCheckBuffer+1;
					}
					bReadHeader=false;
					if (m_nLipSyncTracks>MAX_LIPSYNC_TRACKS)
					{
						m_pLog->Log("Error: LipSync-file has more than %d expression tracks. Skipping.", MAX_LIPSYNC_TRACKS);
						break;
					}
				}
				pBuffer=pNewBuffer+1;
			}
			else
			{
				/*for (int i=0;i<MAX_LIPSYNC_TRACKS;i++)
				{
					PUSH_DATA(i)
				}*/
				break;
			}
			int nRet=0;			//sscanf(pBuffer, "%d,%s\n", &nFrame, sBuffer);
			char sTempBuf[256];
			const char *pScanBuffer;
			const char *pLastScanBuffer=pBuffer;
			const char *pScanBufferComma;
			const char *pScanBufferBreak;
			while (((pScanBufferComma=strnstr(pLastScanBuffer, ",",pEndBuffer-pLastScanBuffer))!=NULL) || ((pScanBufferBreak=strnstr(pLastScanBuffer, "\n",pEndBuffer-pLastScanBuffer))!=NULL))
			{
				pScanBufferComma=strnstr(pLastScanBuffer, ",",pEndBuffer-pLastScanBuffer);
				pScanBufferBreak=strnstr(pLastScanBuffer, "\n",pEndBuffer-pLastScanBuffer);
				if ((!pScanBufferComma) && (!pScanBufferBreak))
					break;
				if (!pScanBufferComma)
					pScanBufferComma=(char*)-1;
				if (!pScanBufferBreak)
					pScanBufferBreak=(char*)-1;
				else
					pScanBufferBreak--;
				bool bLastItem=pScanBufferComma>pScanBufferBreak;
				pScanBuffer=(!bLastItem) ? pScanBufferComma : pScanBufferBreak;
				int nTokenLength=pScanBuffer-pLastScanBuffer;
				if (nTokenLength>=sizeof(sTempBuf))
				{
					nTokenLength=sizeof(sTempBuf)-1;
					m_pLog->Log("Error: LipSync-file has more too long token. Truncating.");
				}
				strncpy(sTempBuf, pLastScanBuffer, nTokenLength);
				sTempBuf[nTokenLength]=0;
				if (nRet==0)	// frame id
				{
					nFrame=atoi(sTempBuf);
				}else
				{
					if (strcmp(sTempBuf, "<none>")!=0)
					{
						if (strnicmp(sTempBuf, "<base", 5)==0)
						{
							PUSH_DATA(nRet-1);
						}else
						{
							int nPatternId=-1;
							TNameToIdMapIt It=mapNameId.find(string(sTempBuf));
							if (It==mapNameId.end())
							{
								SSyncPattern Pattern;
								Pattern.sName=sTempBuf;
								string sAnimName=(Pattern.sName[0]=='#') ? (Pattern.sName) : (string("#phoneme_")+Pattern.sName);
								Pattern.nMorphTargetId=pAnimSet->FindMorphTarget(sAnimName.c_str());
								if (Pattern.nMorphTargetId==-1)
									GameWarning("Morph-Target '%s' not found. Lip-syncing will not or only partially work !", sAnimName.c_str());
								else
								{
									nPatternId=m_vecPatterns.size();
									mapNameId.insert(TNameToIdMapIt::value_type(Pattern.sName, nPatternId));
									m_vecPatterns.push_back(Pattern);
								}
							}else
							{
								nPatternId=It->second;
							}
							if (nPatternId!=-1)
							{
								PUSH_DATA(nRet-1)
								Data[nRet-1].fAmp=LIPSYNC_AMP;
								Data[nRet-1].nPat=nPatternId;
								Data[nRet-1].nOfs=(int)((float)nFrame*fFramesPerSecondRecp*1000.0f);
							}
						}
					}
				}
				nRet++;
				pLastScanBuffer=pScanBuffer+1;
				if (bLastItem)
					break;
			}
			if (nRet!=(m_nLipSyncTracks+1))	// we might have reached the end, lets bail...
			{
/*				for (int i=0;i<MAX_LIPSYNC_TRACKS;i++)
				{
					PUSH_DATA(i)
				}*/
				break;
			}
		}
		for (int i=0;i<MAX_LIPSYNC_TRACKS;i++)
		{
			PUSH_DATA(i)
		}
#else
		// reading header
		int nPatterns;
		m_pPak->FRead(&nPatterns, 4, 1, pFile);
		m_vecPatterns.resize(nPatterns);
		for (int i=0;i<nPatterns;i++)
		{
			SSyncPattern &Pattern=m_vecPatterns[i];
			unsigned char c;
			m_pPak->FRead(&c, 1, 1, pFile);
			Pattern.sName.resize(c+1);
			m_pPak->FRead(&(Pattern.sName[0]), c, 1, pFile);
			Pattern.sName[c]=0;
			string sAnimName=string("#phoneme_")+Pattern.sName;
			Pattern.nMorphTargetId=pAnimSet->FindMorphTarget(sAnimName.c_str());
			if (Pattern.nMorphTargetId==-1)
				GameWarning("Morph-Target '%s' not found. Lip-syncing will not or only partially work !", sAnimName.c_str());
		}
		// reading data
		int nDataSize;
		m_pPak->FRead(&nDataSize, 4, 1, pFile);
		if (nDataSize>0)
		{
			m_vecData.resize(nDataSize);
			for (int i=0;i<nDataSize;i++)
			{
				SSyncData &Data=m_vecData[i];
				m_pPak->FRead(&(Data.nOfs), 4, 1, pFile);
				m_pPak->FRead(&(Data.nPat), 4, 1, pFile);
				m_pPak->FRead(&(Data.fAmp), 4, 1, pFile);
			}
		}
#endif
	}
	SyncFileLoaded();
}

bool CLipSync::LoadDialog(const char *pszFilename, int nSoundVolume, float fMinSoundRadius, float fMaxSoundRadius, float fClipDist, int nSoundFlags,IScriptObject *pAITable)
{
	if ((!m_pScriptSystem) || (!m_pSoundSystem) || (!m_pCharInst))
		return false;	// not initialized
	FUNCTION_PROFILER(m_pSystem, PROFILE_GAME);
	{
		FRAME_PROFILER("LoadDialog: Unload previous dialog", m_pSystem, PROFILE_GAME);
		UnloadDialog();
	}

	m_bUnloadWhenDone=false;
	m_pAITable=pAITable;

	char sSoundFilename[MAX_PATH];
	strncpy(sSoundFilename, pszFilename, MAX_PATH);
	sSoundFilename[MAX_PATH-1]=0;
	char sSyncFilename[MAX_PATH];
	strncpy(sSyncFilename, pszFilename, MAX_PATH);
	sSyncFilename[MAX_PATH-1]=0;
	RemoveExtension(sSyncFilename);
#ifdef MAGPIE_SUPPORT
	AddExtension(sSyncFilename, ".txt");
#else
	AddExtension(sSyncFilename, ".lsf");
#endif
	{
		FRAME_PROFILER("LoadDialog: Loading sound", m_pSystem, PROFILE_GAME);
		if (m_pSound)
			m_pSound->RemoveEventListener( this );
		m_pSound=m_pSoundSystem->LoadSound(sSoundFilename, FLAG_SOUND_3D | nSoundFlags); // FLAG_SOUND_STREAM
	}
	if (!m_pSound)
		return false;
	m_pSound->AddEventListener( this );
	m_pSound->SetVolume(nSoundVolume);
	m_pSound->SetMinMaxDistance(fMinSoundRadius, fMaxSoundRadius*0.5f);
	m_pSound->SetLoopMode(false);

	// ATTENTION,PlayDialog may be already called here.
	m_pSound->Preload();	// this will force starting to load ansynchronously
	
	m_pReadStream=m_pStreamEngine->StartRead("LipSync", sSyncFilename, this);
	if (m_pReadStream->IsFinished())
	{
		//m_pLog->LogToFile("\006io:CLipSync(%p)::LoadDialog IsFinished", this);
		m_pReadStream=NULL;
	}
	return true;
}

bool CLipSync::UnloadDialog()
{
	AbortLoading();
	m_bSoundFileLoaded=false;
	m_bSyncFileLoaded=false;
	m_vecPatterns.clear();
	for (int i=0;i<MAX_LIPSYNC_TRACKS;i++)
		m_vecData[i].clear();
	m_nLipSyncTracks=0;
	if (m_pSound)
	{
		TRACE("DIALOG UNLOADED !!!");
		m_pSound->RemoveEventListener(this);
	}
	m_pSound=NULL;
	m_bUnload=false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CLipSync::PlayDialog(bool bUnloadWhenDone)
{
	if (!m_pSound)
		return false;
	if (m_pReadStream)
	{
		TRACE("WARNING: Waiting to complete load of lipsync data !");
		m_pReadStream->Wait();
	}
	for (int i=0;i<MAX_LIPSYNC_TRACKS;i++)
	{
		m_nLastDataIdx[i]=-1;
	}
	m_bUnloadWhenDone=bUnloadWhenDone;
	if (m_pSound->IsPlaying())
		StopDialog();
	m_pSound->SetPosition(m_pEntity->GetPos());
	m_pSound->Play();

	// if there is an AI conversation table, set timer for the next conversation
	if (m_pAITable!=NULL)
	{
		m_pSystem->GetILog()->Log("\002 Now adding a conversation timer at time %d for the duration of %d seconds",(unsigned long)(m_pTimer->GetCurrTime()*1000),(unsigned long)(m_pSound->GetLengthMs()));

		// Far Cry specific code...
#ifndef _ISNOTFARCRY
			GetIXGame( m_pSystem->GetIGame() )->AddTimer(m_pAITable,(unsigned long)(m_pTimer->GetCurrTime()*1000),(unsigned long)(m_pSound->GetLengthMs()),NULL,false);
#endif
		m_pAITable=NULL;
	}

#ifndef PS2	
	srand(GetTickCount());
#endif
	TRACE("PLAYING DIALOG !!!");
	return true;
}

bool CLipSync::StopDialog()
{
	if (!m_pSound)
		return false;
	if (m_pSound->IsPlaying())
		m_pSound->Stop();
	TRACE("STOPPING DIALOG !!!");
	return true;
}

bool CLipSync::DoExpression(const char *pszMorphTarget, CryCharMorphParams &MorphParams, bool bAnim)
{
	if (!m_pCharInst)
		return false; // not initialized
	ICryCharModel *pModel=m_pCharInst->GetModel();
	assert(pModel);
	IAnimationSet *pAnimSet=pModel->GetAnimationSet();
	assert(pAnimSet);
	int nMorphTargetId=pAnimSet->FindMorphTarget(pszMorphTarget);
	if (nMorphTargetId==-1)
		return false;	// no such morph-target
	// try to set time first in case it is already playing
//	if (!m_pCharInst->SetMorphTime(nMorphTargetId, MorphParams.fStartTime))
		m_pCharInst->StartMorph(nMorphTargetId, MorphParams);
	if (!bAnim)
		m_pCharInst->SetMorphSpeed(nMorphTargetId, 0.0f);
	return true;
}

bool CLipSync::StopExpression(const char *pszMorphTarget)
{
	if (!m_pCharInst)
		return false; // not initialized
	ICryCharModel *pModel=m_pCharInst->GetModel();
	assert(pModel);
	IAnimationSet *pAnimSet=pModel->GetAnimationSet();
	assert(pAnimSet);
	int nMorphTargetId=pAnimSet->FindMorphTarget(pszMorphTarget);
	if (nMorphTargetId==-1)
		return false;	// no such morph-target
	return m_pCharInst->StopMorph(nMorphTargetId);
}

bool CLipSync::UpdateRandomExpressions(float fFrameTime, bool bAnimate)
{
	if (!bAnimate)
		return true;
	if (m_vecExprPatterns.empty())
		return true;	// no expr-patterns loaded
	for (int i=0;i<(int)m_vecExprPatterns.size();i++)
	{
		SExprPattern &Expr=m_vecExprPatterns[i];
		if (!Expr.fNextIntervalTime)
			Expr.fNextIntervalTime=Expr.fOffset+Expr.fInterval+2.0f*(((float)rand()/(float)RAND_MAX)*Expr.fIntervalRandom)-Expr.fIntervalRandom;
		Expr.fCurrIntervalTime+=fFrameTime;
		if (Expr.fCurrIntervalTime<Expr.fNextIntervalTime)
			continue;
		CryCharMorphParams MorphParams;
		MorphParams.fAmplitude=Expr.fAmp+((float)rand()/(float)RAND_MAX)*Expr.fAmpRandom;
		MorphParams.fBlendIn=Expr.fBlendIn;
		MorphParams.fBlendOut=Expr.fBlendOut;
		MorphParams.fLength=Expr.fHold;
		MorphParams.fStartTime=Expr.fCurrIntervalTime-Expr.fNextIntervalTime;
		if (Expr.nMorphTargetId!=-1)
		{
			m_pCharInst->StartMorph(Expr.nMorphTargetId, MorphParams);
//			TRACE("Start morph %s (rnd-expr) - Amp: %1.2f; In: %3.2f; Out: %3.2f; Ofs: %1.2f", Expr.sName.c_str(), MorphParams.fAmplitude, MorphParams.fBlendIn, MorphParams.fBlendOut, MorphParams.fStartTime);
		}
		Expr.fCurrIntervalTime=0.0f;
		Expr.fNextIntervalTime=0.0f;
	}
	return true;
}

bool CLipSync::UpdateLipSync(float fFrameTime, bool bAnimate)
{
	if (m_bUnload)
	{
		UnloadDialog();
		return true;
	}
	if ((!m_pCharInst) || (!m_pSound))
		return true;	// sound not playing
	if (!m_pSound->IsPlaying())
	{
		if (m_bUnloadWhenDone)
			UnloadDialog();
		return true;
	}
	// sync sound-pos
	m_pSound->SetPosition(m_pEntity->GetPos());
//	TRACE("SoundPos: %4.1f, %4.1f, %4.1f", m_pEntity->GetPos().x, m_pEntity->GetPos().y, m_pEntity->GetPos().z);
	if (!bAnimate)
		return true;
	if (!m_nLipSyncTracks)
		return true;	// no data loaded
	if (m_vecPatterns.empty())
		return true;	// no patterns loaded
	// get offset in speech
#ifdef MAGPIE_SUPPORT
	int nCurrSmp=(int)m_pSound->GetCurrentSamplePos(true);
#else
	int nCurrSmp=(int)m_pSound->GetCurrentSamplePos()-LIPSYNC_PLAYAHEAD;		// this adjustment is needed for compensating the smoothing, so we are not too much ahead..
#endif
	if (nCurrSmp<0)
		return true;
	for (int i=0;i<m_nLipSyncTracks;i++)
	{
		if (m_vecData[i].empty())
			continue;
		// get current and next (for interpolation) syncing-data...
		int nThisDataIdx=GetDataIdx(i, nCurrSmp, 0, m_vecData[i].size()-1);
		if (nThisDataIdx<=m_nLastDataIdx[i])
			continue;	// no new event so far...
	//	int nNextDataIdx=nThisDataIdx+1;
	//	if (nNextDataIdx==m_vecData.size())
	//		nNextDataIdx=nThisDataIdx;
		SSyncData &ThisData=m_vecData[i][nThisDataIdx];
		if (nCurrSmp<ThisData.nOfs)
			continue;
	//	SSyncData &NextData=m_vecData[nNextDataIdx];
		if ((ThisData.nPat>=0) && (ThisData.nPat<(int)m_vecPatterns.size()))
		{
	#ifdef MAGPIE_SUPPORT
			float fOfsDiffTotal=(float)ThisData.nLen*(1.0f/1000.0f);// (float)(NextData.nOfs-ThisData.nOfs)*(1.0f/1000.0f);
	#else
			float fOfsDiffTotal=(float)(NextData.nOfs-ThisData.nOfs)*(1.0f/44100.0f);
	#endif
			SSyncPattern &ThisPattern=m_vecPatterns[ThisData.nPat];
			CryCharMorphParams MorphParams;
			MorphParams.fAmplitude=ThisData.fAmp;
			MorphParams.fBlendIn=fOfsDiffTotal*LIPSYNC_FADEIN;//fOfsDiffTotal*0.5f;
			MorphParams.fBlendOut=fOfsDiffTotal*LIPSYNC_FADEOUT+LIPSYNC_CROSSFADE;//fOfsDiffTotal*0.5f;
			MorphParams.fLength=fOfsDiffTotal*(1.0f-LIPSYNC_FADEIN-LIPSYNC_FADEOUT);//0.0f;
	#ifdef MAGPIE_SUPPORT
			MorphParams.fStartTime=(float)(nCurrSmp-ThisData.nOfs)*(1.0f/1000.0f);
	#else
			MorphParams.fStartTime=(float)(nCurrSmp-ThisData.nOfs)*(1.0f/44100.0f);
	#endif
#if !defined(LINUX)
			TRACE("Morping: %s (%d of %d)", ThisPattern.sName.c_str(), nThisDataIdx, m_vecData[i].size());
#endif
			if (ThisPattern.nMorphTargetId!=-1)
				m_pCharInst->StartMorph(ThisPattern.nMorphTargetId, MorphParams);
 			//TRACE("Start morph %s (lip-sync) - Amp: %1.2f; In: %3.2f; Out: %3.2f; Ofs: %1.2f", ThisPattern.sName.c_str(), MorphParams.fAmplitude, MorphParams.fBlendIn, MorphParams.fBlendOut, MorphParams.fStartTime);
		}
		m_nLastDataIdx[i]=nThisDataIdx;
	}
	return true;
}

bool CLipSync::Update(bool bAnimate)
{
	if (!m_pCharInst)
		return true;	// not loaded
	float fFrameTime=m_pTimer->GetFrameTime();
	UpdateRandomExpressions(fFrameTime, bAnimate);
	UpdateLipSync(fFrameTime, bAnimate);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Binary search for the current sync-data.
int CLipSync::GetDataIdx(int nChannel, int nSmp, int nLo, int nHi)
{
	int nMid=((nHi-nLo)>>1)+nLo;
	if (nMid==nLo)
		return nMid;
	if (m_vecData[nChannel][nMid].nOfs>nSmp)
		return GetDataIdx(nChannel, nSmp, nLo, nMid);
	else
		return GetDataIdx(nChannel, nSmp, nMid, nHi);
}

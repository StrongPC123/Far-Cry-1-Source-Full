#pragma once

#include <ISound.h>
#include <IStreamEngine.h>
#include <smartptr.h>

class CSoundSystem;

enum BufferType
{
	btNONE,
	btSAMPLE,
	btSTREAM
};

struct SSoundBufferProps
{
	SSoundBufferProps(const char *_pszName, int _nFlags) : sName(_pszName)
	{
		nFlags=_nFlags & SOUNDBUFFER_FLAG_MASK;
	}
	SSoundBufferProps(const SSoundBufferProps &Props) : sName(Props.sName)
	{
		nFlags=Props.nFlags;
	}
	bool operator<(const SSoundBufferProps &Props) const
	{
		if (nFlags<Props.nFlags)
			return true;
		if (nFlags!=Props.nFlags)
			return false;
		return (stricmp(sName.c_str(), Props.sName.c_str())<0);
	}
	string sName;
	int nFlags;
};

class CSound;

typedef std::vector<CSound*>				TBufferLoadReqVec;
typedef TBufferLoadReqVec::iterator	TBufferLoadReqVecIt;

class CSoundBuffer : IStreamCallback
{
private:
	union tagData
	{
		void *m_pData;
		CS_SAMPLE *m_pSample;
		CS_STREAM *m_pStream;
	};
protected:
	CSoundSystem *m_pSoundSystem;
	int m_nRef;
	BufferType m_Type;
	tagData m_Data;
	SSoundBufferProps m_Props;
	int	m_nBaseFreq;
	float m_fLength;
	IReadStreamPtr m_pReadStream;
	bool m_bLoadFailure;
	bool m_bLooping;
	bool m_bCallbackIteration;
	TBufferLoadReqVec m_vecLoadReq;
	TBufferLoadReqVec m_vecLoadReqToRemove;
	TBufferLoadReqVec m_vecLoadReqToAdd;
protected:
	virtual ~CSoundBuffer();
	virtual void StreamOnComplete(IReadStream *pStream, unsigned nError);
	void SoundLoaded();
	void LoadFailed();
	void UpdateCallbacks();	
public:
	int GetFModFlags(bool bLooping);
  CSoundBuffer(CSoundSystem *pSoundSystem, SSoundBufferProps &Props);
	const char *GetName() { return(m_Props.sName.c_str());}
	int AddRef();
	int Release();
	void RemoveFromLoadReqList(CSound *pSound);
	bool Load(bool bLooping, CSound *pSound);
	bool WaitForLoad();
	void AbortLoading();
	void DestroyData();
	void SetSample(CS_SAMPLE *pPtr);
	void SetStream(CS_STREAM *pPtr);
	SSoundBufferProps GetProps() { return (m_Props); }
	CS_SAMPLE* GetSample() { return (m_Type==btSAMPLE) ? m_Data.m_pSample : NULL; }
	CS_STREAM* GetStream() { return (m_Type==btSTREAM) ? m_Data.m_pStream : NULL; }
	BufferType GetType() { return m_Type; }
	float GetLengthInSeconds() { return m_fLength; }
	int GetBaseFreq() { return m_nBaseFreq; }
	//void AddFlags(int nFlags) { m_Props.nFlags|=nFlags; }
	//void RemoveFlags(int nFlags) { m_Props.nFlags&=~nFlags; }
	bool NotLoaded() { return (m_Data.m_pData==NULL) && (m_pReadStream==NULL); }
	bool Loaded() { return (m_Data.m_pData!=NULL) && (m_pReadStream==NULL); }
	bool Loading() { return (m_Data.m_pData==NULL) && (m_pReadStream!=NULL); }
	bool LoadFailure() { return m_bLoadFailure; }
};

typedef _smart_ptr<CSoundBuffer>	CSoundBufferPtr;
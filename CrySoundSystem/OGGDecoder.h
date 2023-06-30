#pragma once

#include "PatternDecoder.h"
#include <vorbis/vorbisfile.h>

struct IMusicSystem;
class COGGDecoderInstance;

struct SFileAccess
{
	SFileAccess()
	{
		pPak=NULL;
		pFile=NULL;
	}
	SFileAccess(ICryPak *_pPak, FILE *_pFile)
	{
		pPak=_pPak;
		pFile=_pFile;
	}
	ICryPak *pPak;
	FILE *pFile;
};

class COGGDecoder :	public IMusicPatternDecoder
{
protected:
	IMusicSystem *m_pMusicSystem;
	SFileAccess m_FileAccess;
	SMusicPatternFileInfo m_FileInfo;
	friend class COGGDecoderInstance;
protected:
	virtual ~COGGDecoder();
public:
	COGGDecoder(IMusicSystem *pMusicSystem);
	void Release() { delete this; }
	//! Open a music-pattern-file.
	bool Open(const char *pszFilename);
	//! Close a music-pattern-file.
	bool Close();
	//! Retrieve file-info
	bool GetFileInfo(SMusicPatternFileInfo &FileInfo);
	//! Retrieve mem-info
	void GetMemoryUsage(class ICrySizer* pSizer);
	//! Create instance.
	IMusicPatternDecoderInstance* CreateInstance();
};

class COGGDecoderInstance : public IMusicPatternDecoderInstance
{
protected:
	COGGDecoder *m_pDecoder;
	int m_nPos;	// position in samples
	int m_nPosBytes;
	OggVorbis_File m_OVFile;
private:
	bool FillPCMBuffer(signed long *pBuffer, int nSamples);
public:
	static size_t ReadFile(void *ptr, size_t size, size_t nmemb, void *datasource);
	static int SeekFile(void *datasource, ogg_int64_t offset, int whence);
	static int CloseFile(void *datasource);
	static long TellFile(void *datasource);
protected:
	virtual ~COGGDecoderInstance();
public:
	COGGDecoderInstance(COGGDecoder *pDecoder);
	void Release() { delete this; }
	//! Seek to beginning of pattern (if nDelay is set it will wait nDelay-samples before starting playback).
	bool Seek0(int nDelay=0);
	//! Retrieve the current position in the file (in samples)
	int GetPos();
	//! Decode and retrieve pcm-data (stereo/16bit).
	bool GetPCMData(signed long *pDataOut, int nSamples, bool bLoop=true);
};
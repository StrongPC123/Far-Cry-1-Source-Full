#include "StdAfx.h"
#include <CrySizer.h>
#include <ISound.h>
#include <ISystem.h>
#include <ICryPak.h>
#include "oggdecoder.h"
#include "MusicSystem.h"

COGGDecoder::COGGDecoder(IMusicSystem *pMusicSystem)
{
	m_pMusicSystem=pMusicSystem;
	ISystem *pSystem=m_pMusicSystem->GetSystem();
	ASSERT(pSystem);
	m_FileAccess.pPak=pSystem->GetIPak();
	ASSERT(m_FileAccess.pPak);
}

COGGDecoder::~COGGDecoder()
{
	Close();
}

void getTicks(int64* pnTime)
{
#ifdef WIN64
	*pnTime = __rdtsc();
#else
	__asm {
		mov ebx, pnTime
		rdtsc
		mov [ebx], eax
		mov [ebx+4], edx
	}
#endif
}

bool COGGDecoder::Open(const char *pszFilename)
{
	if (m_FileAccess.pFile)
		return false;
	m_FileAccess.pFile=m_FileAccess.pPak->FOpen(pszFilename, "rb");
	if (!m_FileAccess.pFile)
		return false;
	OggVorbis_File TestOVFile;
	ov_callbacks Callbacks;
	Callbacks.read_func=COGGDecoderInstance::ReadFile;
	Callbacks.seek_func=COGGDecoderInstance::SeekFile;
	Callbacks.close_func=COGGDecoderInstance::CloseFile;
	Callbacks.tell_func=COGGDecoderInstance::TellFile;
	if (ov_open_callbacks(&m_FileAccess, &TestOVFile, NULL, 0, Callbacks)!=0)
		return false;
	int nSeekable=ov_seekable(&TestOVFile);
	m_FileInfo.sFilename=pszFilename;
	m_FileInfo.nHeaderSize=0;
	m_FileInfo.nSamples=(int)ov_pcm_total(&TestOVFile, -1);
	// Throw the comments plus a few lines about the bitstream we're decoding
	MTRACE("File-information of %s...", m_FileInfo.sFilename.c_str());
	vorbis_comment *pComment=ov_comment(&TestOVFile, -1);
	vorbis_info *pInfo=ov_info(&TestOVFile, -1);
	char **pPtr=pComment->user_comments;
	while (*pPtr)
	{
		MTRACE("  %s", *pPtr);
		pPtr++;
	}
	MTRACE("  Bitstream is %d channel, %ld Hz, %d kbit/s", pInfo->channels, pInfo->rate, pInfo->bitrate_nominal/1000);
	MTRACE("  Encoded by: %s", pComment->vendor);
	if (pInfo->rate!=44100)
	{
		MTRACE("Warning: [COGGDecoder::Open()] Cannot load file %s, due to unsupported samplerate (%d Hz found; 44100 Hz needed) !", m_FileInfo.sFilename.c_str(), pInfo->rate);
		return false;
	}
	if (pInfo->channels!=2)
	{
		MTRACE("Warning: [COGGDecoder::Open()] Cannot load file %s, due to unsupported channel-number (%d channels found; 2 channels needed) !", m_FileInfo.sFilename.c_str(), pInfo->channels);
		return false;
	}
	if (ov_clear(&TestOVFile)!=0)
		return false;
	if (!nSeekable)
		return false;
	return true;
}

bool COGGDecoder::Close()
{
	if (!m_FileAccess.pFile)
		return false;
	m_FileAccess.pPak->FClose(m_FileAccess.pFile);
	m_FileAccess.pFile=NULL;
	return true;
}

bool COGGDecoder::GetFileInfo(SMusicPatternFileInfo &FileInfo)
{
	if (!m_FileAccess.pFile)
		return false;
	FileInfo=m_FileInfo;
	return true;
}

void COGGDecoder::GetMemoryUsage(class ICrySizer* pSizer)
{
	if (!pSizer->Add(*this))
		return;
}

IMusicPatternDecoderInstance* COGGDecoder::CreateInstance()
{
	return new COGGDecoderInstance(this);
}

//////////////////////////////////////////////////////////////////////////
/*** INSTANCE ***/
//////////////////////////////////////////////////////////////////////////

size_t COGGDecoderInstance::ReadFile(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	SFileAccess *pFileAccess=(SFileAccess*)datasource;
	return pFileAccess->pPak->FRead(ptr, size, nmemb, pFileAccess->pFile);
}

int COGGDecoderInstance::SeekFile(void *datasource, ogg_int64_t offset, int whence)
{
	SFileAccess *pFileAccess=(SFileAccess*)datasource;
	return pFileAccess->pPak->FSeek(pFileAccess->pFile, (long)offset, whence);
}

int COGGDecoderInstance::CloseFile(void *datasource)
{
	return 0;
}

long COGGDecoderInstance::TellFile(void *datasource)
{
	SFileAccess *pFileAccess=(SFileAccess*)datasource;
	return pFileAccess->pPak->FTell(pFileAccess->pFile);
}

COGGDecoderInstance::COGGDecoderInstance(COGGDecoder *pDecoder)
{
//	int64 nStartTime, nEndTime;
//	getTicks(&nStartTime);
	ASSERT(pDecoder);
	m_pDecoder=pDecoder;
	m_pDecoder->m_FileAccess.pPak->FSeek(m_pDecoder->m_FileAccess.pFile, 0, SEEK_SET);
	ov_callbacks Callbacks;
	Callbacks.read_func=ReadFile;
	Callbacks.seek_func=SeekFile;
	Callbacks.close_func=CloseFile;
	Callbacks.tell_func=TellFile;
	if (ov_open_callbacks(&(m_pDecoder->m_FileAccess), &m_OVFile, NULL, 0, Callbacks)!=0)
		MTRACE("Warning: [COGGDecoderInstance::c_tor()] ov_open() failed !");
	// Seek0 is quite expensive for OGGs so we do a simpler version here instead...
	m_nPos=0;
	m_nPosBytes=m_pDecoder->m_FileAccess.pPak->FTell(m_pDecoder->m_FileAccess.pFile);	// store current file-cursor
//	getTicks(&nEndTime);
//	MTRACE("Info: [COGGDecoderInstance::c_tor()] It took %d cycles to initialize ogg-decoder-instance of %s.", (long)(nEndTime-nStartTime), m_pDecoder->m_FileInfo.sFilename.c_str());
}

COGGDecoderInstance::~COGGDecoderInstance()
{
	if (ov_clear(&m_OVFile)!=0)
		MTRACE("Warning: [COGGDecoderInstance::d_tor()] ov_clear() failed !");
}

bool COGGDecoderInstance::Seek0(int nDelay)
{
	int nOldPos=m_nPos;
	m_nPos=-nDelay;
	if (nOldPos>0)	// only seek if it is really necessary, since it takes some time on OGGs...
	{
		if (ov_pcm_seek(&m_OVFile, 0)!=0)
			return false;
		m_nPosBytes=m_pDecoder->m_FileAccess.pPak->FTell(m_pDecoder->m_FileAccess.pFile);	// store current file-cursor
	}
	return true;
}

int COGGDecoderInstance::GetPos()
{
	if (!m_pDecoder->m_FileAccess.pFile)
		return -1;
	return m_nPos;
}

bool COGGDecoderInstance::GetPCMData(signed long *pDataOut, int nSamples, bool bLoop)
{
	if (!m_pDecoder->m_FileAccess.pFile)
		return false;
	int nOfs=0;
	if (m_nPos<0)
	{
		if (-m_nPos>=nSamples)
		{
			memset(pDataOut, 0, nSamples*m_pDecoder->m_pMusicSystem->GetBytesPerSample());
			m_nPos+=nSamples;
			return true;
		}else
		{
			nOfs=-m_nPos;
			m_nPos=0;
			nSamples-=nOfs;
			memset(pDataOut, 0, nOfs*m_pDecoder->m_pMusicSystem->GetBytesPerSample());
		}
	}
	// prepare file-cursor
	m_pDecoder->m_FileAccess.pPak->FSeek(m_pDecoder->m_FileAccess.pFile, m_nPosBytes, SEEK_SET);
	int nSamplesToRead;
	for (;;)
	{
		if ((m_nPos+nSamples)>m_pDecoder->m_FileInfo.nSamples)
			nSamplesToRead=m_pDecoder->m_FileInfo.nSamples-m_nPos;
		else
			nSamplesToRead=nSamples;
		if (!FillPCMBuffer(&(pDataOut[nOfs]), nSamplesToRead))
			return false;
		m_nPos+=nSamplesToRead;
		if (nSamplesToRead==nSamples)
			break;
		nOfs+=nSamplesToRead;
		if (!bLoop)
		{
			memset(&(pDataOut[nOfs]), 0, (nSamples-nSamplesToRead)*m_pDecoder->m_pMusicSystem->GetBytesPerSample());
			break;
		}
		nSamples-=nSamplesToRead;
		Seek0();
	}
	// store file-cursor
	m_nPosBytes=m_pDecoder->m_FileAccess.pPak->FTell(m_pDecoder->m_FileAccess.pFile);
	return true;
}

bool COGGDecoderInstance::FillPCMBuffer(signed long *pBuffer, int nSamples)
{
	int nBytesToRead=nSamples*m_pDecoder->m_pMusicSystem->GetBytesPerSample();
	int nBytesRead=0;
	while (nBytesToRead>nBytesRead)
	{
		int nCurrStream=0;
		long nRet=ov_read(&m_OVFile, &((char*)pBuffer)[nBytesRead], nBytesToRead-nBytesRead, 0, 2, 1, &nCurrStream);
		if (nRet==OV_HOLE)
		{
			MTRACE("Warning: [COGGDecoderInstance::FillPCMBuffer] Hole found in stream %s !", m_pDecoder->m_FileInfo.sFilename.c_str());
			return false;
		}else
		if (nRet==OV_EBADLINK)
		{
			MTRACE("Warning: [COGGDecoderInstance::FillPCMBuffer] Bad link found in stream %s !", m_pDecoder->m_FileInfo.sFilename.c_str());
			return false;
		}else
		if (nRet==0)
		{
			MTRACE("Warning: [COGGDecoderInstance::FillPCMBuffer] Unexpected end of stream %s !", m_pDecoder->m_FileInfo.sFilename.c_str());
			return false;
		}else
		{
			nBytesRead+=nRet;
		}
	}
	return true;
}
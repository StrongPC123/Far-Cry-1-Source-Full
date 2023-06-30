#include "StdAfx.h"
#include <CrySizer.h>
#include <ISound.h>
#include <ISystem.h>
#include <ICryPak.h>
#include "pcmdecoder.h"

CPCMDecoder::CPCMDecoder(IMusicSystem *pMusicSystem) : m_b44KHz(true)
{
	m_pMusicSystem=pMusicSystem;
	ISystem *pSystem=m_pMusicSystem->GetSystem();
	ASSERT(pSystem);
	m_pPak=pSystem->GetIPak();
	ASSERT(m_pPak);
	m_pFile=NULL;
}

CPCMDecoder::~CPCMDecoder()
{
	Close();
}

int CPCMDecoder::ReadFile(void *pBuf, int nSize, int nCount, int *pNewFilePos)
{
	if (!m_pFile) 
		return false;
	int nBytesRead=m_pPak->FRead(pBuf, nSize, nCount, m_pFile);
	if (nBytesRead!=nCount)
	{
		m_nPosBytes=m_pPak->FTell(m_pFile);
		if (pNewFilePos)
			*pNewFilePos=m_nPosBytes;
		return 0;
	}
	m_nPosBytes+=nSize*nCount;
	if (pNewFilePos)
		*pNewFilePos=m_nPosBytes;
	return nBytesRead;
}

bool CPCMDecoder::InitStreamWAV()
{
	if (!m_pFile)
		return false;
	SWaveHdr WaveHeader;
	if (!ReadFile(&WaveHeader, sizeof(SWaveHdr), 1))
		return false;
	if (WaveHeader.dwDSize==0)
	{
		TRACE("Warning: [CPCMDecoder::InitStreamWAV()] Cannot load file %s, due to zero-data-size !", m_FileInfo.sFilename.c_str());
		return false;
	}
/*	if (WaveHeader.dwSRate!=44100)
	{
		TRACE("Warning: [CPCMDecoder::InitStreamWAV()] Cannot load file %s, due to unsupported samplerate (%d Hz found; 44100 Hz needed) !", m_FileInfo.sFilename.c_str(), WaveHeader.dwSRate);
		return false;
	}
*/	if (WaveHeader.wChnls!=2)
	{
		TRACE("Warning: [CPCMDecoder::InitStreamWAV()] Cannot load file %s, due to unsupported channel-number (%d channels found; 2 channels needed) !", m_FileInfo.sFilename.c_str(), WaveHeader.wChnls);
		return false;
	}
	if (WaveHeader.BitsPerSample!=16)
	{
		TRACE("Warning: [CPCMDecoder::InitStreamWAV()] Cannot load file %s, due to unsupported resolution (%d bits/sample found; 16 bits/sample needed) !", m_FileInfo.sFilename.c_str(), WaveHeader.BitsPerSample);
		return false;
	}
	m_PCMFileInfo.nBytesPerSample=WaveHeader.BitsPerSample/8*WaveHeader.wChnls;
	m_FileInfo.nHeaderSize=sizeof(SWaveHdr);
	m_FileInfo.nSamples=WaveHeader.dwDSize/m_PCMFileInfo.nBytesPerSample;
	if(WaveHeader.dwSRate == 22050)
	{
		m_FileInfo.nSamples *= 2;	//correct number if different in 22 KHz mode due to different file size
		m_b44KHz = false;
	}
	else
		m_b44KHz = true;
	return true;
}

bool CPCMDecoder::Open(const char *pszFilename)
{
	if (m_pFile)
		return false;
	m_pFile=m_pPak->FOpen(pszFilename, "rb");
	if (!m_pFile)
		return false;
	m_nPosBytes=0;
	m_FileInfo.sFilename=pszFilename;
	if (!InitStreamWAV())
	{
		Close();
		return false;
	}
	return true;
}

bool CPCMDecoder::Close()
{
	if (!m_pFile)
		return false;
	m_pPak->FClose(m_pFile);
	m_pFile=NULL;
	return true;
}

bool CPCMDecoder::GetFileInfo(SMusicPatternFileInfo &FileInfo)
{
	if (!m_pFile)
		return false;
	FileInfo=m_FileInfo;
	return true;
}

void CPCMDecoder::GetMemoryUsage(class ICrySizer* pSizer)
{
	if (!pSizer->Add(*this))
		return;
}

IMusicPatternDecoderInstance* CPCMDecoder::CreateInstance()
{
	return new CPCMDecoderInstance(this);
}

//////////////////////////////////////////////////////////////////////////
/*** INSTANCE ***/
//////////////////////////////////////////////////////////////////////////

CPCMDecoderInstance::CPCMDecoderInstance(CPCMDecoder *pDecoder) : m_bCopyFromLastFrame(false)
{
	m_pDecoder=pDecoder;
	Seek0();
}

CPCMDecoderInstance::~CPCMDecoderInstance()
{
}

bool CPCMDecoderInstance::SeekBytes(int nBytes)
{
	if (!m_pDecoder->m_pFile)
		return false;
	m_nPosBytes=nBytes;
	if (m_pDecoder->m_nPosBytes==nBytes)
		return true;
	if (m_pDecoder->m_pPak->FSeek(m_pDecoder->m_pFile, nBytes, SEEK_SET)!=0)
		return false;
	m_pDecoder->m_nPosBytes=nBytes;
	return true;
}

bool CPCMDecoderInstance::Seek0(int nDelay)
{
	m_nPos=-nDelay;
	m_bCopyFromLastFrame = false;
	return SeekBytes(m_pDecoder->m_FileInfo.nHeaderSize);
}

int CPCMDecoderInstance::GetPos()
{
	if (!m_pDecoder->m_pFile)
		return -1;
	return m_nPos;
}

bool CPCMDecoderInstance::GetPCMData(signed long *pDataOut, int nSamples, bool bLoop)
{
	if (!m_pDecoder->m_pFile)
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
	if (!SeekBytes(m_nPosBytes))
		return false;
	int nSamplesToRead;
	for (;;)
	{
		if ((m_nPos+nSamples)>m_pDecoder->m_FileInfo.nSamples)
			nSamplesToRead=m_pDecoder->m_FileInfo.nSamples-m_nPos;
		else
			nSamplesToRead=nSamples;
		if(m_pDecoder->Is44KHz())
		{
			if (!m_pDecoder->ReadFile(&(pDataOut[nOfs]), m_pDecoder->m_PCMFileInfo.nBytesPerSample, nSamplesToRead, &m_nPosBytes))
				return false;
		}
		else
		{
			//decode for 44 KHz
			if (!FillPCMBuffer22KHz(&(pDataOut[nOfs]), nSamplesToRead))
				return false;
		}
		// FIXME: conversation of samplerate might be needed here
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
	return true;
}

	//! fills a dest buffer with uncompressed data
const bool CPCMDecoderInstance::FillPCMBuffer22KHz(signed long *pBuffer, int nSamples)
{
	//first check for copying some data from last frame (if not at starting position)
	if(m_bCopyFromLastFrame && m_nPos != 0)
	{
		*pBuffer++ = m_lLastSample;//contains sample to start with from last encoding
		nSamples -= 1;
		m_bCopyFromLastFrame = false;
	}
	const unsigned int cuiSamplesPerBlock = scuiEncodedBlockSize / m_pDecoder->m_PCMFileInfo.nBytesPerSample;//number of samples to read per block (according to allocated static array)
	const unsigned int cuiBlocks = nSamples / (cuiSamplesPerBlock*2);	//blocks to read, remember samples will be doubled
	for(unsigned int i=0; i<cuiBlocks;i++)
	{
		if (!m_pDecoder->ReadFile(m_aEncodedBlock, m_pDecoder->m_PCMFileInfo.nBytesPerSample, cuiSamplesPerBlock, &m_nPosBytes))
			return false;
		//now double up interleaved samples
		const signed long *pEncodedData = reinterpret_cast<signed long*>(&m_aEncodedBlock[0]);
		for(unsigned int j=0; j<cuiSamplesPerBlock;j++)
		{
			*pBuffer++ = *pEncodedData;		//double up samples
			*pBuffer++ = *pEncodedData++;
		}
		nSamples -= (2*cuiSamplesPerBlock);
	}
	if(nSamples > 0)
	{
		//differentiate between a left odd and even sample count
		if(nSamples & 0x1)
		{
			m_bCopyFromLastFrame = true;
			//read one sample more
			if (!m_pDecoder->ReadFile(m_aEncodedBlock, m_pDecoder->m_PCMFileInfo.nBytesPerSample, (nSamples/2)+1, &m_nPosBytes))
				return false;
			const signed long *pEncodedData = reinterpret_cast<signed long*>(&m_aEncodedBlock[0]);
			for(int j=0; j<nSamples/2;j++)
			{
				*pBuffer++ = *pEncodedData;		//double up samples
				*pBuffer++ = *pEncodedData++;
			}
			*pBuffer++ = m_lLastSample = *pEncodedData;	//remember last sample, has to start with in next request
		}
		else
		{
			//even count, no special treatment
			m_bCopyFromLastFrame = false;
			if (!m_pDecoder->ReadFile(m_aEncodedBlock, m_pDecoder->m_PCMFileInfo.nBytesPerSample, (nSamples/2), &m_nPosBytes))
				return false;
			const signed long *pEncodedData = reinterpret_cast<signed long*>(&m_aEncodedBlock[0]);
			for(int j=0; j<nSamples/2;j++)
			{
				*pBuffer++ = *pEncodedData;		//double up samples
				*pBuffer++ = *pEncodedData++;
			}
		}
	}
	return true;
}
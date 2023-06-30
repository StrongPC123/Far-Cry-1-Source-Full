#pragma once

#define LOG_MUSICFILES

struct SMusicPatternFileInfo
{
	SMusicPatternFileInfo()
	{
		sFilename="";
		nHeaderSize=0;
		nSamples=0;
	}
	string sFilename;
	int nHeaderSize;
	int nSamples;
};

struct SWaveHdr
{
	char RIFF[4];									// "RIFF" tag
	unsigned long dwSize;					// Size of data to follow
	char WAVE[4];									// "WAVE" tag
	char fmt_[4];									// "fmt " tag
	unsigned long dw16;						// 16
	unsigned short wOne_0;				// 1
	unsigned short wChnls;				// Number of Channels
	unsigned long dwSRate;				// Sample Rate
	unsigned long BytesPerSec;		// Bytes per second
	unsigned short wBlkAlign;			// Block align
	unsigned short BitsPerSample; // Sample size
	char DATA[4];									// "DATA"
	unsigned long dwDSize;				// Number of Samples
};

enum EWaveFormatIDs
{
	WAVE_FORMAT_PCM					= 0x0001, 
	WAVE_FORMAT_ADPCM				= 0x0002,
};

struct IMusicPatternDecoderInstance
{
	virtual void Release()=0;
	//! Seek to beginning of pattern (if nDelay is set it will wait nDelay-samples before starting playback).
	virtual bool Seek0(int nDelay=0)=0;
	//! Retrieve the current position in the file (in samples).
	virtual int GetPos()=0;
	//! Decode and retrieve pcm-data (stereo/16bit).
	virtual bool GetPCMData(signed long *pDataOut, int nSamples, bool bLoop=true)=0;
};

struct IMusicPatternDecoder
{
	virtual void Release()=0;
	//! Open a music-pattern-file.
	virtual bool Open(const char *pszFilename)=0;
	//! Close a music-pattern-file.
	virtual bool Close()=0;
	//! Retrieve file-info.
	virtual bool GetFileInfo(SMusicPatternFileInfo &FileInfo)=0;
	//! Retrieve mem-info.
	virtual void GetMemoryUsage(class ICrySizer* pSizer)=0;
	//! Create instance.
	virtual IMusicPatternDecoderInstance* CreateInstance()=0;
};
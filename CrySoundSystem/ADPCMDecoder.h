#pragma once

#include "PatternDecoder.h"
#include <string>

struct	IMusicSystem;
class	CADPCMDecoderInstance;
struct	ICryPak;
 
//specific header struct for ADPCM, data stored in a slidely different manner than usual wav
typedef struct SADPCMWaveHdr
{
	char RIFF[4];							// "RIFF" tag, identifies it as WAV
	unsigned long dwSize;					// Size of data to follow (filesize-8)
	char WAVE[4];							// "WAVE" tag
	char fmt_[4];							// "fmt " tag
	unsigned long  dw16;					// 16
	unsigned short FormatTag;				// should be WAVE_FORMAT_ADPCM=2
	unsigned short wChnls;					// Number of Channels (1 for mono, 2 for stereo), should be always 2
	unsigned long  dwSRate;					// Sample Rate
	unsigned long  BytesPerSec;				// Bytes per second
	unsigned short wBlkAlign;				// Block align,  usually 2048
	unsigned short BitsPerSample;			// Bits Per Sample, usually 4
	unsigned short wExtSize;				// not important
	unsigned short SamplesPerBlock;			// samples per block, usually 2036   
	//followed by number of coeficients and coeficients themselves
} SADPCMWaveHdr;

struct MsState 
{
    int step;  
	short sCoef0;
	short sCoef1;
};	


//////////////////////////////////////////////////////////////////////////
class CADPCMDecoder :	public IMusicPatternDecoder
{
public:
	static const unsigned int scuiSizeOfWavHeader = sizeof(SWaveHdr);	//need to hardcode here since SADPCMWaveHdr is not complined with usual header

	CADPCMDecoder(IMusicSystem *pMusicSystem);
	void Release() { delete this; }
	//! Open a music-pattern-file, does actually just store filename
	bool Open(const char *pszFilename); 
	//! Close a music-pattern-file.
	bool Close();
	//! Retrieve file-info
	bool GetFileInfo(SMusicPatternFileInfo &FileInfo);
	//! Retrieve mem-info
	void GetMemoryUsage(class ICrySizer* pSizer);
	//! Create instance.
	IMusicPatternDecoderInstance* CreateInstance();
	//! returns new file handle
	FILE* GetNewFileHandle();
	//! returns new file handle
	void CloseFileHandle(FILE*& pFile);
	//! retreieves file name
	const char* FileName() const;
	//! decode function for one block
	static void AdpcmBlockExpandI( int nCoef, const short* iCoef, const unsigned char* ibuff, short* obuff, int n );
	const bool IsOpen()const;
	//! retrieves frequency encoding
	const bool Is44KHz() const {return m_b44KHz;}

protected:
	bool m_b44KHz;	//keeps track of encoded frequency

	//! decode function for one sample, called exclusively by AdpcmBlockExpandI
	static int CADPCMDecoder::AdpcmDecode( int c, MsState& rState, int sample1, int sample2 );	
	void SetFileInfo(const unsigned int cuiSampleCount, const bool cbIs44KHz = true);

	virtual ~CADPCMDecoder();

	IMusicSystem			*m_pMusicSystem;
	ICryPak					*m_pPak;
	SMusicPatternFileInfo	m_FileInfo;	
	string				    m_szFilename;	
	bool					m_bOpen;	//keeps track for an open command

	friend class CADPCMDecoderInstance;
};

//////////////////////////////////////////////////////////////////////////
class CADPCMDecoderInstance : public IMusicPatternDecoderInstance
{
public:
	//hardcoded constants,  can be changed when using a different format
	static const unsigned int scuiBlockSize			= 2048;	//constant value in 44 KHz Microsoft format, half the size for 22KHz
	static const unsigned int scuiSamplesPerBlock	= 2036; //constant value in 44 KHz Microsoft format, half the size for 22KHz
	static const unsigned int scuiNumberOfCoefs		= 7;	//constant number of coefs

	CADPCMDecoderInstance(CADPCMDecoder *pDecoder);	//initializes as well
	void Release() { Close(); delete this; }
	//! Seek to beginning of pattern (if nDelay is set it will wait nDelay-samples before starting playback).
	bool Seek0(int nDelay=0);
	//! Retrieve the current position in the file (in samples)
	int GetPos();
	//! Decode and retrieve pcm-data (stereo/16bit).
	bool GetPCMData(signed long *pDataOut, int nSamples, bool bLoop=true);
	//! closes all
	void Close();
	//! retrieves number of total contained samples
	const unsigned int Samples();

protected:
	CADPCMDecoder	*m_pDecoder;			// one decoder per file
	int				m_nPos;					// current position in samples
	FILE			*m_pFile;				// own File handle for now
	int				m_iFilePos;				// current file pos corresponding to right file seek pos (counted from m_uiDataStartPos)
	bool			m_bInitialized;			// functionality not available if false
	bool			m_bCopyFromLastFrame;	// indicates some copying from last frame (due to odd sample count request)
	signed long		m_lLastSample;			// sample to reuse for next frame 

	unsigned int	m_uiCurrentBlockSize;	// block size in file, always less than static allocated buffer
	unsigned int	m_uiCurrentSamplesPerBlock;// sampels per block in file(usually m_uiCurrentBlockSize-12), always less than static allocated buffer

	//ADPCM specific stuff
	unsigned char   m_aEncodedBlock[scuiBlockSize];			//static allocated encoded block
	short			m_aDecodedBlock[scuiSamplesPerBlock*2];	//static allocated decoded block, 2 per channel (1:4)
	short			m_aCoefs[scuiNumberOfCoefs * 2/*2 per channel, stereo assumed*/];				//static allocated coeficients

	unsigned int	m_uiDataStartPos;			// usually at position 90, set by InitStreamWav
	unsigned int	m_uiNumSamples;				// total sample count
	unsigned short	m_sCoefs;					// ADPCM: number of coef sets
	short*			m_psSamplePtr;				// Pointer to current sample
	int				m_iState[ 16 ];				// step-size info for *ADPCM writes

protected:
	//! fills a dest buffer with uncompressed data
	const bool FillPCMBuffer(signed long *pBuffer, int nSamples);
	//! fills a dest buffer from 22KHz compressed data
	const bool FillPCMBuffer22KHz(signed long *pBuffer, int nSamples);

	const bool InitStreamWAV();					// for now has every instance its own file handle
	unsigned short AdpcmReadBlock(short* pDest = NULL);		// decoded one block, if dest not specified, default static buffer is chosen

	virtual ~CADPCMDecoderInstance();			// to hit right destructor
};


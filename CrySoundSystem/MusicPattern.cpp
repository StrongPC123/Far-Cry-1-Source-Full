#include "StdAfx.h"
#include <algorithm>
#include <CrySizer.h>
#include <ICryPak.h>
#include "musicpattern.h"
#include "PCMDecoder.h"
#include "OGGDecoder.h"
#include "ADPCMDecoder.h"

//#define TEST

CMusicPattern::CMusicPattern(IMusicSystem *pMusicSystem, const char *pszName,const char *pszFilename)
{
	m_pMusicSystem=pMusicSystem;
	m_pDecoder=NULL;
	m_vecFadePoints.clear();
	m_nLayeringVolume=255;
	m_sName=pszName;
	m_sFilename = pszFilename;
	m_numPatternInstances = 0;
	m_nSamples = 0;
}

CMusicPattern::~CMusicPattern()
{
	Close(); 
}

static bool inline 
FindFile( ICryPak* pPak, const char* pszFilename )
{
	bool bRet( false );
	FILE* pFile( pPak->FOpen( pszFilename, "rb" ) ) ;
	if( 0 != pFile )
	{
		bRet = true;
		pPak->FClose( pFile );
	}
	return( bRet );
}

bool CMusicPattern::Open( const char* pszFilename )
{
	// no valid filename
	if( 0 == pszFilename || strlen( pszFilename ) < 5 )
	{
		return( false );
	}

	if( 0 != m_pDecoder )
	{
		Close();
	}

	ISystem* pSystem( m_pMusicSystem->GetSystem() );
	ICryPak* pPak( pSystem->GetIPak() );
	assert( 0 != pPak );

	string strFilename( pszFilename );
	if( 0 != stricmp( strFilename.c_str() + strFilename.size() - 4, ".ogg" ) )		
	{
		strFilename = strFilename.substr( 0, strFilename.size() - 4 ) + ".ogg";
	}

	bool bWaveFallback( false != m_pMusicSystem->StreamOGG() && false == FindFile( pPak, strFilename.c_str() ) );
	if( false == m_pMusicSystem->StreamOGG() || false != bWaveFallback )
	{
		if( false != bWaveFallback )
		{
#ifdef LOG_MUSICFILES
			m_pMusicSystem->LogMsg( "OGG file %s not found... trying .wav", strFilename.c_str() );
#endif
		}

		// use (AD)PCM files for music streaming
		strFilename = strFilename.substr( 0, strFilename.size() - 4 ) + ".wav";

		FILE* pFile( pPak->FOpen( strFilename.c_str(), "rb" ) );
		if( 0 != pFile )
		{
			// check encoding and spawn PCM or ADPCM decoder accordingly
			SWaveHdr header;
			pPak->FRead( &header, sizeof( SWaveHdr ), 1, pFile );
			if( header.dwSize < ( 90 - 8 + 12 ) )
			{
				// not enough data, not even one sample
				return( false );
			}
			if( WAVE_FORMAT_ADPCM == header.wOne_0 )
			{
				m_pDecoder = new CADPCMDecoder( m_pMusicSystem );	
				m_pDecoder->Open( strFilename.c_str() );// returns always true, just use here to really open it
				// since we actually do not know in advance how many samples are contained, we need to create a temp instance
				IMusicPatternDecoderInstance* pTempInstance( m_pDecoder->CreateInstance() );
				pTempInstance->Release();
#ifdef LOG_MUSICFILES
				m_pMusicSystem->LogMsg( "Initialized ADPCM Decoder for %s", strFilename.c_str() );
#endif
			}
			else
			if( WAVE_FORMAT_PCM == header.wOne_0 )
			{
				m_pDecoder = new CPCMDecoder( m_pMusicSystem );	
#ifdef LOG_MUSICFILES
				m_pMusicSystem->LogMsg( "Initialized PCM Decoder for %s", strFilename.c_str() );
#endif
			}
			else
			{
#ifdef LOG_MUSICFILES
				m_pMusicSystem->LogMsg( "AD(PCM) expected... failed to initialize %s", strFilename.c_str() );
#endif
				return( false );
			}
			pPak->FClose( pFile );
		}
		else
		{
			return( false );
		}
	}
	else
	{
		// use OGG files for music streaming
		if( 0 != stricmp( strFilename.c_str() + strFilename.size() - 4, ".ogg" ) )		
		{	
			strFilename = strFilename.substr( 0, strFilename.size() - 4 ) + ".ogg";
		}

		m_pDecoder = new COGGDecoder( m_pMusicSystem );	
#ifdef LOG_MUSICFILES
		m_pMusicSystem->LogMsg( "Initialized OGG Decoder for %s", strFilename.c_str() );
#endif
	}

	if( 0 != m_pDecoder )
	{
		if( false == m_pDecoder->Open( strFilename.c_str() ) )
		{
			m_pDecoder->Release();
			m_pDecoder=NULL;
			return( false );
		}
	}
	else
	{
		return( false );
	}

	SMusicPatternFileInfo fileInfo;
	if( false != m_pDecoder->GetFileInfo( fileInfo ) )
	{
		m_nSamples = fileInfo.nSamples;
#ifdef TEST
		signed long *pDat = new signed long[m_nSamples];
		signed long *pDataLength = new signed long[m_nSamples];
		signed long *pData = pDat;
		IMusicPatternDecoderInstance* pTempInstance = m_pDecoder->CreateInstance();
		unsigned int c = m_nSamples;
		pTempInstance->GetPCMData(pDataLength,m_nSamples);
		pTempInstance->Seek0();

		while(c >= 11025)
		{
			pTempInstance->GetPCMData(pData,11025);
			pData += 11025;
			c -= 11025;
		}
		if(c > 0)
		{
			pTempInstance->GetPCMData(pData,c);
		}
		SWaveHdr header;
		FILE *pOrig = fopen("MUSIC/Original.wav","rb");
		FILE *pTest = fopen("MUSIC/Test.wav","wb");
		fread(&header, sizeof(SWaveHdr), 1, pOrig);
		fwrite(&header, 1, sizeof(SWaveHdr), pTest);
		fwrite(pDat, 1, m_nSamples*4, pTest);
		fclose(pTest);
		fclose(pOrig);
		pTempInstance->Release();
		for(int i=0;i<m_nSamples;i++)
		{
			if(pDat[i] != pDataLength[i])
			{
				signed long sl0 = pDat[i];
				signed long sl1 = pDataLength[i];
				int ivo=0;
			}
		}
		delete [] pDat;
		delete [] pDataLength;
#endif
	}

	m_sFilename = strFilename;
	return true;
}

bool CMusicPattern::Close()
{
	if (!m_pDecoder)
		return false;
	m_pDecoder->Close();
	m_pDecoder->Release();
	m_pDecoder=NULL;
	return true;
}

bool CMusicPattern::AddFadePoint(int nFadePos)
{
	/*
	SMusicPatternFileInfo FileInfo;
	if (nFadePos<=0)
	{
		if (!GetFileInfo(FileInfo))
			return false;
		nFadePos = FileInfo.nSamples+nFadePos;
	}
	*/
	TMarkerVecIt It=std::find(m_vecFadePoints.begin(), m_vecFadePoints.end(), nFadePos);
	if (It!=m_vecFadePoints.end())	// already in list ?
		return true;
	m_vecFadePoints.push_back(nFadePos);
	std::sort(m_vecFadePoints.begin(), m_vecFadePoints.end());
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CMusicPattern::ClearFadePoints()
{
	m_vecFadePoints.clear();
}

//////////////////////////////////////////////////////////////////////////
IMusicPatternDecoderInstance* CMusicPattern::CreateDecoderInstance()
{
	if (!m_pDecoder)
		return NULL;
	return m_pDecoder->CreateInstance();
}

CMusicPatternInstance* CMusicPattern::CreateInstance()
{
	if (!m_pDecoder)
	{
		// Make new decoder.
		if (!Open( m_sFilename.c_str() ))
		{
			m_pMusicSystem->LogMsg( "ERROR: Music file %s failed to load",m_sFilename.c_str() );
			return 0;
		}
	}
	if (!m_pDecoder)
		return NULL;
	m_numPatternInstances++;
	return new CMusicPatternInstance(this);
}

void CMusicPattern::ReleaseInstance( CMusicPatternInstance* pInstance )
{
	m_numPatternInstances--;
	if (m_numPatternInstances <= 0)
	{
		// Can release decoder.
		Close();
	}
}

bool CMusicPattern::GetFileInfo(SMusicPatternFileInfo &FileInfo)
{
	if (!m_pDecoder)
	{
		// Make new decoder.
		if (!Open( m_sFilename.c_str() ))
		{
			m_pMusicSystem->LogMsg( "ERROR: Music file %s failed to load",m_sFilename.c_str() );
			return false;
		}
		bool bRes = false;
		if (m_pDecoder)
			bRes = m_pDecoder->GetFileInfo(FileInfo);
		Close();
		return bRes;
	}
	else
		return m_pDecoder->GetFileInfo(FileInfo);
}

void CMusicPattern::GetMemoryUsage(class ICrySizer* pSizer)
{
	if (!pSizer->Add(*this))
		return;
	if (m_pDecoder)
		m_pDecoder->GetMemoryUsage(pSizer);
}
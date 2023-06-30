#include "StdAfx.h"
#include "musicpatterninstance.h"
#include "MusicPattern.h"
#include "PatternDecoder.h"

CMusicPatternInstance::CMusicPatternInstance(CMusicPattern *pPattern)
{
	m_nRefs=0;
	m_pPattern=pPattern;
	if (m_pPattern)
		m_pDecoderInstance=m_pPattern->CreateDecoderInstance();
	else
		m_pDecoderInstance=NULL;
}

CMusicPatternInstance::~CMusicPatternInstance()
{
	if (m_pDecoderInstance)
		m_pDecoderInstance->Release();
	m_pPattern->ReleaseInstance( this );
}

bool CMusicPatternInstance::Seek0(int nDelay)
{
	if (!m_pDecoderInstance)
		return false;
	return m_pDecoderInstance->Seek0(nDelay);
}

bool CMusicPatternInstance::GetPCMData(signed long *pDataOut, int nSamples, bool bLoop)
{
	if (!m_pDecoderInstance)
		return false;
	return m_pDecoderInstance->GetPCMData(pDataOut, nSamples, bLoop);
}

int CMusicPatternInstance::GetSamplesToNextFadePoint()
{
	if (!m_pDecoderInstance)
		return false;
	int nPos=m_pDecoderInstance->GetPos();
	for (TMarkerVecIt It=m_pPattern->m_vecFadePoints.begin();It!=m_pPattern->m_vecFadePoints.end();++It)
	{
		int nFadePos=(*It);
		if (nFadePos <= 0)
		{
			nFadePos = m_pPattern->m_nSamples + nFadePos;
		}
		if (nFadePos>nPos)
			return nFadePos-nPos;
	}
	return GetSamplesToEnd();
}

int CMusicPatternInstance::GetSamplesToLastFadePoint()
{
	if (!m_pDecoderInstance)
		return false;
	int nPos=m_pDecoderInstance->GetPos();
	if (m_pPattern->m_vecFadePoints.empty())
		return GetSamplesToEnd();
	
	int nFadePos = (*(m_pPattern->m_vecFadePoints.end()-1));
	if (nFadePos <= 0)
	{
		nFadePos = m_pPattern->m_nSamples + nFadePos;
	}
	return nFadePos - nPos;
}

int CMusicPatternInstance::GetSamplesToEnd()
{
	if (m_pDecoderInstance != NULL && m_pPattern != NULL)
		return m_pPattern->m_nSamples - m_pDecoderInstance->GetPos();
	return -1;
}

#pragma once

#include <SmartPtr.h>

class CMusicPattern;
struct IMusicPatternDecoderInstance;

class CMusicPatternInstance
{
protected:
	int m_nRefs;
	CMusicPattern *m_pPattern;
	IMusicPatternDecoderInstance *m_pDecoderInstance;
public:
	CMusicPatternInstance(CMusicPattern *pPattern);
	virtual ~CMusicPatternInstance();
	void AddRef()
	{
		m_nRefs++;
	}
	void Release()
	{
		m_nRefs--;
		if (m_nRefs<=0)
			delete this;
	}
	CMusicPattern* GetPattern() { return m_pPattern; }
	//! Seek to beginning of pattern (if nDelay is set it will wait nDelay-samples before starting playback).
	bool Seek0(int nDelay=0);
	bool GetPCMData(signed long *pDataOut, int nSamples, bool bLoop=true);
	int GetSamplesToNextFadePoint();
	int GetSamplesToLastFadePoint();
	int GetSamplesToEnd();
};